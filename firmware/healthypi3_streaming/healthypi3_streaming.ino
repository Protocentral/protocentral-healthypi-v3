//////////////////////////////////////////////////////////////////////////////////////////
//
//   HealthyPi HAT v3 Arduino Firmware
//
//   Copyright (c) 2016 ProtoCentral
//   
//   Heartrate and respiration computation based on original code from Texas Instruments
//
//   SpO2 computation based on original code from Maxim Integrated 
//
//   This software is licensed under the MIT License(http://opensource.org/licenses/MIT). 
//   
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT 
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//   For information on how to use the HealthyPi, visit https://github.com/protocentral/protocentral-healthypi-v3
/////////////////////////////////////////////////////////////////////////////////////////

#include <SPI.h>
#include <Wire.h>
#include "wiring_private.h" // pinPeripheral() function
#include "ads1292r.h"
#include "max30205.h"

#define TEMPERATURE 0
#define FILTERORDER         161
/* DC Removal Numerator Coeff*/
//#define NRCOEFF (0.992)
#define WAVE_SIZE  1
#define NRCOEFF (0.992)

#define IOPORT_PIN_LEVEL_HIGH 1
#define IOPORT_PIN_LEVEL_LOW 0

#define ADS1292R_CS_PIN   3
#define ADS1292R_START_PIN 10
#define ADS1292R_PWDN_PIN 12
#define ADS1292R_DRDY_PIN A4

#define AFE4490_START_PIN  16//A2
#define AFE4490_CS_PIN     15//A1
#define AFE4490_DRDY_PIN   A0 

//******* ecg filter *********
#define MAX_PEAK_TO_SEARCH         5
#define MAXIMA_SEARCH_WINDOW      40
#define MINIMUM_SKIP_WINDOW       50
#define SAMPLING_RATE             125
#define TWO_SEC_SAMPLES           2 * SAMPLING_RATE
#define QRS_THRESHOLD_FRACTION    0.4
#define TRUE 1
#define FALSE 0

//SPI class(MISO
SPIClass ads1292rSPI (&sercom2, 4, 0, 1, SPI_PAD_2_SCK_3, SERCOM_RX_PAD_0);
//SPI class(MISO
SPIClass afe4490SPI (&sercom0, 17, 9 , 8, SPI_PAD_2_SCK_3, SERCOM_RX_PAD_0);    
//SPI class(MISO
Uart  RpiSerial(&sercom4, 38, 22 , SERCOM_RX_PAD_1, UART_TX_PAD_0);    

TwoWire max30205(&sercom1, 11, 13);

int8_t NewDataAvailable;
uint8_t data_len = 20;

volatile byte MSB;
volatile byte data;
volatile byte LSB;

volatile byte *SPI_RX_Buff_Ptr;
unsigned long resultTemp = 0;
signed long sresultTemp,sresultTempResp ;
volatile float fresultTemp = 0;
volatile long iresultTemp = 0;
volatile long voltageTemp = 0;
volatile bool ads1292DataReceived = false;
volatile uint8_t  SPI_Dummy_Buff[10];
volatile long count=0;

uint8_t AFE4490_SPI_TX_Buff[10];
volatile uint8_t  AFE4490_SPI_RX_Buff[3]={0,0,0};
volatile uint32_t AFE4490resultTemp = 0;
volatile int32_t AFE4490sresultTempIR=0,AFE4490sresultTempRED=0;
volatile float AFE4490fresultTemp = 0;
volatile int32_t AFE4490iresultTemp = 0;
volatile int32_t AFE4490voltageTemp = 0;
volatile bool afe4490DataReceived = false;
long v[9];
long   sresultTempECG;
uint8_t heartRate_ads1292=0,heartRate_BP=0,spo2=0;

//************ filter ***************
uint8_t Respiration_Rate = 0 ;

int RESP_Second_Prev_Sample = 0 ;
int RESP_Prev_Sample = 0 ;
int RESP_Current_Sample = 0 ;
int RESP_Next_Sample = 0 ;
int RESP_Second_Next_Sample = 0 ;
uint16_t iprocess=0;

int16_t RESP_WorkingBuff[2 * FILTERORDER];
int16_t Pvev_DC_Sample=0, Pvev_Sample=0;
int16_t ECG_Pvev_DC_Sample, ECG_Pvev_Sample;

int16_t RespCoeffBuf[FILTERORDER] = 
{             
      /* Coeff for lowpass Fc=2Hz @ 125 SPS*/
      120,    124,    126,    127,    127,    125,    122,    118,    113,
      106,     97,     88,     77,     65,     52,     38,     24,      8,
       -8,    -25,    -42,    -59,    -76,    -93,   -110,   -126,   -142,
     -156,   -170,   -183,   -194,   -203,   -211,   -217,   -221,   -223,
     -223,   -220,   -215,   -208,   -198,   -185,   -170,   -152,   -132,
     -108,    -83,    -55,    -24,      8,     43,     80,    119,    159,
      201,    244,    288,    333,    378,    424,    470,    516,    561,
      606,    650,    693,    734,    773,    811,    847,    880,    911,
      939,    964,    986,   1005,   1020,   1033,   1041,   1047,   1049,
     1047,   1041,   1033,   1020,   1005,    986,    964,    939,    911,
      880,    847,    811,    773,    734,    693,    650,    606,    561,
      516,    470,    424,    378,    333,    288,    244,    201,    159,
      119,     80,     43,      8,    -24,    -55,    -83,   -108,   -132,
     -152,   -170,   -185,   -198,   -208,   -215,   -220,   -223,   -223,
     -221,   -217,   -211,   -203,   -194,   -183,   -170,   -156,   -142,
     -126,   -110,    -93,    -76,    -59,    -42,    -25,     -8,      8,
       24,     38,     52,     65,     77,     88,     97,    106,    113,
      118,    122,    125,    127,    127,    126,    124,    120      
};

int16_t CoeffBuf_40Hz_LowPass[FILTERORDER] = 
{
      -72,    122,    -31,    -99,    117,      0,   -121,    105,     34,
     -137,     84,     70,   -146,     55,    104,   -147,     20,    135,
     -137,    -21,    160,   -117,    -64,    177,    -87,   -108,    185,
      -48,   -151,    181,      0,   -188,    164,     54,   -218,    134,
      112,   -238,     90,    171,   -244,     33,    229,   -235,    -36,
      280,   -208,   -115,    322,   -161,   -203,    350,    -92,   -296,
      361,      0,   -391,    348,    117,   -486,    305,    264,   -577,
      225,    445,   -660,     93,    676,   -733,   -119,    991,   -793,
     -480,   1486,   -837,  -1226,   2561,   -865,  -4018,   9438,  20972,
     9438,  -4018,   -865,   2561,  -1226,   -837,   1486,   -480,   -793,
      991,   -119,   -733,    676,     93,   -660,    445,    225,   -577,
      264,    305,   -486,    117,    348,   -391,      0,    361,   -296,
      -92,    350,   -203,   -161,    322,   -115,   -208,    280,    -36,
     -235,    229,     33,   -244,    171,     90,   -238,    112,    134,
     -218,     54,    164,   -188,      0,    181,   -151,    -48,    185,
     -108,    -87,    177,    -64,   -117,    160,    -21,   -137,    135,
       20,   -147,    104,     55,   -146,     70,     84,   -137,     34,
      105,   -121,      0,    117,    -99,    -31,    122,    -72
};

volatile int16_t i16respbuff;
uint16_t resp_buff_indx=0;
int16_t res_wave_buff;
int16_t resp_filterout;
uint8_t    DataPacket[100];

uint16_t k=0;
uint8_t i = 0;

#define FS            25    //sampling frequency
#define BUFFER_SIZE  (FS*4) 
#define MA4_SIZE  4 // DONOT CHANGE
#define min(x,y) ((x) < (y) ? (x) : (y))

const uint8_t uch_spo2_table[184]={ 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99, 
              99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
              100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97, 
              97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 
              90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81, 
              80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 
              66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 
              49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29, 
              28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5, 
              3, 2, 1 } ;
              
static  int32_t an_x[ BUFFER_SIZE]; 
static  int32_t an_y[ BUFFER_SIZE]; 

int16_t ECG_WorkingBuff[2 * FILTERORDER];
unsigned char Start_Sample_Count_Flag = 0;
unsigned char first_peak_detect = FALSE ;
unsigned int sample_index[MAX_PEAK_TO_SEARCH+2] ;
uint16_t scaled_result_display[150];
uint8_t indx =0;

int cnt =0;
volatile uint8_t flag =0;

int QRS_Second_Prev_Sample = 0 ;
int QRS_Prev_Sample = 0 ;
int QRS_Current_Sample = 0 ;
int QRS_Next_Sample = 0 ;
int QRS_Second_Next_Sample = 0 ;

static uint16_t QRS_B4_Buffer_ptr = 0 ;
int16_t QRS_Threshold_Old = 0;
int16_t QRS_Threshold_New = 0;
unsigned int sample_count = 0 ;

uint16_t QRS_Heart_Rate = 0 ;
int16_t ecg_wave_buff,ecg_filterout;

volatile uint8_t global_HeartRate=0;
volatile uint8_t global_RespirationRate=0;

uint16_t aun_ir_buffer[600]; //infrared LED sensor data
uint16_t aun_red_buffer[600];  //red LED sensor data

int32_t n_buffer_count; //data length

int32_t n_spo2;  //SPO2 value
int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
int32_t n_heart_rate; //heart rate value
int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid

long status_byte=0;
uint8_t LeadStatus=0;
boolean leadoff_deteted = true;
uint8_t spo2_probe_open = false;
long time_elapsed =0;

void ads1292_detection_callback(void)
{
    digitalWrite(ADS1292R_CS_PIN,LOW);

    for (int x = 0; x < 9 ; x++)
    {
      SPI_Dummy_Buff[x] = ads1292rSPI.transfer(0xff);
      // SerialUSB.print(SPI_Dummy_Buff[x],HEX); 
    }
    
    status_byte = (long)((long)SPI_Dummy_Buff[2] | ((long) SPI_Dummy_Buff[1]) <<8 | ((long) SPI_Dummy_Buff[0])<<16);
    status_byte  = (status_byte & 0x0f8000) >> 15;
    LeadStatus = (unsigned char ) status_byte ; 

    if(!((LeadStatus & 0x1f) == 0 ))
    leadoff_deteted  = TRUE;
    else leadoff_deteted  = FALSE;

    digitalWrite(ADS1292R_CS_PIN,HIGH);
    resultTemp = (unsigned long)((0x00 << 24) | (SPI_Dummy_Buff[6] << 16)| SPI_Dummy_Buff[7] << 8 | SPI_Dummy_Buff[8]);//6,7,8
    resultTemp = (unsigned long)(resultTemp << 8);

    sresultTemp = (signed long)(resultTemp);
    ecg_wave_buff = (int16_t)  (sresultTemp>>16);   // store for TI's algorithm
      
    resultTemp = (uint32_t)((0 << 24) | (SPI_Dummy_Buff[3] << 16)| SPI_Dummy_Buff[4] << 8 | SPI_Dummy_Buff[5]);//6,7,8
    resultTemp = (uint32_t)(resultTemp << 8);

    sresultTempResp = (long)(resultTemp);
    res_wave_buff = (int16_t)(sresultTempResp>>8) ; 
    
    ads1292DataReceived = true;
  
}
   
void afe4490_detection_callback(void)
{
   afe4490DataReceived = true;
   count++;
}

void setup() 
{   
    int cnt;
    delay(2000);
    
    SerialUSB.begin(115200);
    RpiSerial.begin(57600);
    
    max30205begin();
    
    ads1292Rbegin();
    delay(100);
    afe4490begin();   
    delay(500);
    AFE4490Write(CONTROL0,0x000001);
    n_buffer_count=0;


    for (cnt =0 ; cnt < FILTERORDER; cnt++)
    {
        RESP_WorkingBuff[cnt] = 0;
        ECG_WorkingBuff[cnt] = 0;
    }

    Pvev_DC_Sample = 0;
    Pvev_Sample = 0;
    ECG_Pvev_DC_Sample = 0;
    ECG_Pvev_Sample = 0;
}

int dec=0;

void loop() 
{   
   if(ads1292DataReceived == true)
   {
    if(leadoff_deteted == true)
     {
       if(millis() > time_elapsed )
       leadoff_deteted = true;
       else leadoff_deteted = false;
      
     }
     else
     {
        time_elapsed = millis() + 2000;
     }

     float temp = getTemperature()*100; // read temperature for every 100ms
     int temperature =  (uint16_t) temp;
       
      resp_filterout = Resp_ProcessCurrSample(res_wave_buff);
      //resp_filterout=res_wave_buff;
      RESP_Algorithm_Interface(resp_filterout);

      //ecg_filterout=ecg_wave_buff;
      ecg_filterout = ECG_ProcessCurrSample(ecg_wave_buff);
      QRS_Algorithm_Interface(ecg_filterout);
       
      DataPacket[0] = 0x0A;  // sync0
      DataPacket[1] = 0xFA;
      DataPacket[2] = (uint8_t) (data_len);
      DataPacket[3] = (uint8_t) (data_len>>8);
      DataPacket[4] = 0x02;
      
      if(leadoff_deteted == true)
      {
            DataPacket[5] = 0;
          DataPacket[6] =  0;
    
          DataPacket[7] =  0;
          DataPacket[8] =  0;
      }
      else
      {
          DataPacket[5] =  ecg_filterout;
          DataPacket[6] =  ecg_filterout>>8;
    
          DataPacket[7] =  resp_filterout;
          DataPacket[8] =  resp_filterout>>8;   
      }

      if(1) // if(afe4490DataReceived == true)
      {     
          AFE4490_SPI_TX_Buff[0] =  0x2C;
          
          digitalWrite(AFE4490_CS_PIN,LOW);
          afe4490SPI.transfer(AFE4490_SPI_TX_Buff[0]);
          
          AFE4490_SPI_RX_Buff[0] =  afe4490SPI.transfer(0xff);
          AFE4490_SPI_RX_Buff[1] =  afe4490SPI.transfer(0xff);
          AFE4490_SPI_RX_Buff[2] =  afe4490SPI.transfer(0xff);
          
          AFE4490resultTemp = (uint32_t)( ((0x00 << 24) | AFE4490_SPI_RX_Buff[0] << 16)| AFE4490_SPI_RX_Buff[1] << 8 | AFE4490_SPI_RX_Buff[2]);
          AFE4490resultTemp = (uint32_t)(AFE4490resultTemp << 10);

          if(dec==5)
          {
            aun_ir_buffer[n_buffer_count]=(uint16_t) (AFE4490resultTemp>>16);
          }
          
          AFE4490sresultTempIR = (int32_t)(AFE4490resultTemp );
          //AFE4490sresultTempIR = (AFE4490sresultTempIR);// * 100;
          
          AFE4490_SPI_TX_Buff[0] =  0x2A;
          
          afe4490SPI.transfer(AFE4490_SPI_TX_Buff[0]);
          
          AFE4490_SPI_RX_Buff[0] =  afe4490SPI.transfer(0xff);
          AFE4490_SPI_RX_Buff[1] =  afe4490SPI.transfer(0xff);
          AFE4490_SPI_RX_Buff[2] =  afe4490SPI.transfer(0xff);
          
          AFE4490resultTemp = (uint32_t)( ((0x00 << 24) | AFE4490_SPI_RX_Buff[0] << 16)| AFE4490_SPI_RX_Buff[1] << 8 | AFE4490_SPI_RX_Buff[2]);
          AFE4490resultTemp = (uint32_t)(AFE4490resultTemp << 10);

          if(dec==5)
          {
           aun_red_buffer[n_buffer_count]=(uint16_t) (AFE4490resultTemp>>16);
           dec=0;
            n_buffer_count++;
          }
          
          dec++;
           
          AFE4490sresultTempRED = (int32_t)(AFE4490resultTemp );
          AFE4490sresultTempRED = (AFE4490sresultTempRED >> 10);// * 100;   
          
          digitalWrite(AFE4490_CS_PIN,HIGH);
 
          if(n_buffer_count>99)
          {
            
            estimate_spo2(aun_ir_buffer, 100, aun_red_buffer, &n_spo2, &ch_spo2_valid); 
            if(n_spo2 == -999)
            spo2_probe_open = true;
            else
            spo2_probe_open = false;
            
            n_buffer_count=0;
          }         
      }  
      
      DataPacket[9] =  AFE4490sresultTempIR;  // spo2 ir
      DataPacket[10] =  AFE4490sresultTempIR>>8;
      DataPacket[11] =  AFE4490sresultTempIR>>16;
      DataPacket[12] =  AFE4490sresultTempIR>>24;

      DataPacket[13] =  AFE4490sresultTempRED;  // spo2 ir
      DataPacket[14] =  AFE4490sresultTempRED>>8;
      DataPacket[15] =  AFE4490sresultTempRED>>16;
      DataPacket[16] =  AFE4490sresultTempRED>>24;

      DataPacket[17] =  (uint8_t) temperature;      // temperature
      DataPacket[18] =  (uint8_t) (temperature >> 8);
        
      DataPacket[19] =  global_RespirationRate; //QRS_Heart_Rate;   // calculated from ads1292r
      DataPacket[20] =  n_spo2;
  
      DataPacket[21] =  global_HeartRate;   // systolic pressure
      DataPacket[22] =  80;     //diastolic pressure
      DataPacket[23] =  heartRate_BP;  // from BP module

      DataPacket[24] =    spo2_probe_open<< 1 |leadoff_deteted;  // leadstatus
        
      DataPacket[25] = 0x00;
      DataPacket[26] = 0x0b;
    
      for(int i=0; i<27; i++) // transmit the data
      {
          SerialUSB.write(DataPacket[i]);
          RpiSerial.write(DataPacket[i]);        
      } 
      ads1292DataReceived = false;
  }
}

void ads1292Rbegin()
{

  ads1292rSPI.begin();
  

  pinMode(ADS1292R_START_PIN, OUTPUT);
  pinMode(ADS1292R_DRDY_PIN, INPUT);
  pinMode(ADS1292R_DRDY_PIN, INPUT_PULLUP);
  attachInterrupt(ADS1292R_DRDY_PIN, ads1292_detection_callback , FALLING);
  pinMode(ADS1292R_PWDN_PIN, OUTPUT);

  
  pinMode(ADS1292R_CS_PIN, OUTPUT);
 
   pinPeripheral(4, PIO_SERCOM_ALT);
   pinPeripheral(0, PIO_SERCOM_ALT);
   pinPeripheral(1, PIO_SERCOM_ALT);
  
   ads1292rSPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
  
  SPI_Reset();                                            
  delay(100);       

  ADS1292_Disable_Start();
  delay(100);                                               

  ADS1292_Enable_Start();
  delay(10);  

 
  ADS1292_Hard_Stop();
  ADS1292_Start_Data_Conv_Command();
  ADS1292_Soft_Stop(); 

  digitalWrite(ADS1292R_CS_PIN,LOW);

  ADS1292_Reg_Write(0x01,0x00);     //Set sampling rate to 125 SPS
  delay(10);
  
  ADS1292_Reg_Write(0x02,0b11100000); //Lead-off comp off, test signal disabled
  delay(10);
  ADS1292_Reg_Write(0x03,0b11110000); //Lead-off defaults
  delay(10);
  ADS1292_Reg_Write(0x04,0b01000000);   //Ch 1 enabled, gain 6, connected to electrode in
  delay(10);
  ADS1292_Reg_Write(0x05,0b01100000);   //Ch 2 enabled, gain 6, connected to electrode in
  delay(10);
  ADS1292_Reg_Write(0x06,0b00101100); //RLD settings: fmod/16, RLD enabled, RLD inputs from Ch2 only
  delay(10);
  ADS1292_Reg_Write(0x07,0x00);   //LOFF settings: all disabled
  delay(10);
  //Skip register 8, LOFF Settings default
  
  ADS1292_Reg_Write(0x09,0b11110010); //Respiration: MOD/DEMOD turned only, phase 0
  delay(10);
  ADS1292_Reg_Write(0x0A,0b00000011); //Respiration: Calib OFF, respiration freq defaults
  delay(10);

  ADS1292_Enable_Start();
  
  ADS1292_Start_Read_Data_Continuous();
  
  digitalWrite(ADS1292R_CS_PIN,HIGH); //release chip, signal end transfer

  SPI_Start();
  delay(100);
}

void writeRegister(uint8_t address, uint8_t value)
{
  digitalWrite(ADS1292R_CS_PIN,LOW);
  delay(5);
  ads1292rSPI.transfer(WREG|(address<<2));        
  ads1292rSPI.transfer(value); 
  delay(5);
  digitalWrite(ADS1292R_CS_PIN,HIGH);
}  


uint8_t readRegister(uint8_t address)
{
  uint8_t data;

  digitalWrite(ADS1292R_CS_PIN,LOW);
  delay(5);
  ads1292rSPI.transfer(RREG|(address<<2));        
//  data = ads1292rSPI.transfer(SPI_MASTER_DUMMY); 
  delay(5);
  digitalWrite(ADS1292R_CS_PIN,HIGH);

  return data;
}  

uint8_t * Read_Data()
{
  static byte SPI_Buff[3];

  if((digitalRead(ADS1292R_DRDY_PIN)) == LOW)                //        Wait for DRDY to transition low
  {
    digitalWrite(ADS1292R_CS_PIN,LOW);                          //Take CS low
    delayMicroseconds(100);
    for (int i = 0; i < 3; i++)
    { 
     // SPI_Buff[i] = ads1292rSPI.transfer(SPI_MASTER_DUMMY);
    }
    delayMicroseconds(100);
    digitalWrite(ADS1292R_CS_PIN,HIGH);                      //  Clear CS to high
    NewDataAvailable = true;
  }
    
  return SPI_Buff;
}

void SPI_Reset()
{
  digitalWrite(ADS1292R_PWDN_PIN, IOPORT_PIN_LEVEL_HIGH);
  delay(20);
  digitalWrite(ADS1292R_PWDN_PIN, IOPORT_PIN_LEVEL_LOW);
  delay(20);
  digitalWrite(ADS1292R_PWDN_PIN, IOPORT_PIN_LEVEL_HIGH);
  delay(20); 
  ADS1292_SPI_Command_Data(RESET);                              
}

void SPI_Start()
{
  ADS1292_SPI_Command_Data(START);
}

void SPI_Stop()
{
  ADS1292_SPI_Command_Data(STOP);
}

void ADS1292_Reg_Write (unsigned char READ_WRITE_ADDRESS, unsigned char DATA)
{
  uint8_t  SPI_TX_Buff[10];
  
  switch (READ_WRITE_ADDRESS)
  {
  
    case 1: DATA = DATA & 0x87;
        break;
        
    case 2: DATA = DATA & 0xFB;
        DATA |= 0x80;
        break;
        
    case 3: DATA = DATA & 0xFD;
        DATA |= 0x10;
        break;
        
    case 7: DATA = DATA & 0x3F;
        break;
        
    case 8: DATA = DATA & 0x5F;
        break;
        
    case 9:DATA |= 0x02;
        break;
        
    case 10:DATA = DATA & 0x87;
        DATA |= 0x01;
        break;
        
    case 11:DATA = DATA & 0x0F;
        break;
        
    default:break;
  }   
  
  SPI_TX_Buff[0] = READ_WRITE_ADDRESS | WREG;
  SPI_TX_Buff[1] = 0;           // Write Single byte
  SPI_TX_Buff[2] = DATA;          // Write Single byte         // Write Single byte

  digitalWrite(ADS1292R_CS_PIN,LOW);
  delay(5);
  ads1292rSPI.transfer( SPI_TX_Buff[0]);        
  ads1292rSPI.transfer(SPI_TX_Buff[1]); 
  ads1292rSPI.transfer(SPI_TX_Buff[2]); 
  delay(5);
  digitalWrite(ADS1292R_CS_PIN,HIGH);
}

void ADS1292_Enable_Start(void)
{
  digitalWrite(ADS1292R_START_PIN, IOPORT_PIN_LEVEL_HIGH);
  delay(20);
}

void ADS1292_Disable_Start(void)
{
  digitalWrite(ADS1292R_START_PIN, IOPORT_PIN_LEVEL_LOW);
  delay(20);
}

void ADS1292_Hard_Stop (void)
{
  digitalWrite(ADS1292R_START_PIN, IOPORT_PIN_LEVEL_LOW);
  delay(100);
}

void ADS1292_Start_Data_Conv_Command (void)
{
  ADS1292_SPI_Command_Data(START);
}

void ADS1292_Soft_Stop (void)
{
  ADS1292_SPI_Command_Data(STOP);  
}

void ADS1292_Start_Read_Data_Continuous (void)
{
  ADS1292_SPI_Command_Data(RDATAC);         // Send 0x10 to the ADS1x9x
}


void ADS1292_SPI_Command_Data(unsigned char data_in)
{
  digitalWrite(ADS1292R_CS_PIN, LOW);
  delay(2);
  digitalWrite(ADS1292R_CS_PIN, HIGH);
  delay(2);
  digitalWrite(ADS1292R_CS_PIN, LOW);
  delay(2);
  ads1292rSPI.transfer(data_in);
  delay(2);
  digitalWrite(ADS1292R_CS_PIN, HIGH);
}

void afe4490begin()
{
    afe4490SPI.begin();
    delay(100);
    pinMode(AFE4490_START_PIN, OUTPUT);
    digitalWrite(AFE4490_START_PIN, HIGH);
    pinMode(AFE4490_DRDY_PIN, INPUT);
    pinMode(AFE4490_DRDY_PIN, INPUT_PULLUP);
    attachInterrupt(AFE4490_DRDY_PIN, afe4490_detection_callback , FALLING);
    delay(100);
    pinMode(AFE4490_CS_PIN, OUTPUT);
    
    pinPeripheral(17, PIO_SERCOM_ALT);
    pinPeripheral(9, PIO_SERCOM_ALT);
    pinPeripheral(8, PIO_SERCOM_ALT);
    delay(100); 
    afe4490SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));  
    
    AFE4490Write(CONTROL0,0x000000);
    
    AFE4490Write(CONTROL0,0x000008);

   AFE4490Write(TIAGAIN,0x000000); // CF = 5pF, RF = 500kR
   AFE4490Write(TIA_AMB_GAIN,0x000001);
   
   AFE4490Write(LEDCNTRL,0x001414);
   AFE4490Write(CONTROL2,0x000000); // LED_RANGE=100mA, LED=50mA
   AFE4490Write(CONTROL1,0x010707); // Timers ON, average 3 samples
   
   AFE4490Write(PRPCOUNT, 0X001F3F);

   AFE4490Write(LED2STC, 0X001770); //timer control
   AFE4490Write(LED2ENDC,0X001F3E); //timer control
   AFE4490Write(LED2LEDSTC,0X001770); //timer control
   AFE4490Write(LED2LEDENDC,0X001F3F); //timer control
   AFE4490Write(ALED2STC, 0X000000); //timer control
   AFE4490Write(ALED2ENDC, 0X0007CE); //timer control
   AFE4490Write(LED2CONVST,0X000002); //timer control
   AFE4490Write(LED2CONVEND, 0X0007CF); //timer control
   AFE4490Write(ALED2CONVST, 0X0007D2); //timer control
   AFE4490Write(ALED2CONVEND,0X000F9F); //timer control

   AFE4490Write(LED1STC, 0X0007D0); //timer control
   AFE4490Write(LED1ENDC, 0X000F9E); //timer control
   AFE4490Write(LED1LEDSTC, 0X0007D0); //timer control
   AFE4490Write(LED1LEDENDC, 0X000F9F); //timer control
   AFE4490Write(ALED1STC, 0X000FA0); //timer control
   AFE4490Write(ALED1ENDC, 0X00176E); //timer control
   AFE4490Write(LED1CONVST, 0X000FA2); //timer control
   AFE4490Write(LED1CONVEND, 0X00176F); //timer control
   AFE4490Write(ALED1CONVST, 0X001772); //timer control
   AFE4490Write(ALED1CONVEND, 0X001F3F); //timer control

   AFE4490Write(ADCRSTCNT0, 0X000000); //timer control
   AFE4490Write(ADCRSTENDCT0,0X000000); //timer control
   AFE4490Write(ADCRSTCNT1, 0X0007D0); //timer control
   AFE4490Write(ADCRSTENDCT1, 0X0007D0); //timer control
   AFE4490Write(ADCRSTCNT2, 0X000FA0); //timer control
   AFE4490Write(ADCRSTENDCT2, 0X000FA0); //timer control
   AFE4490Write(ADCRSTCNT3, 0X001770); //timer control
   AFE4490Write(ADCRSTENDCT3, 0X001770);
}

void AFE4490Write (uint8_t address, uint32_t data)
{
    AFE4490_SPI_TX_Buff[0] =  address;
    AFE4490_SPI_TX_Buff[1] = ((data >> 16) & 0xFF) ;
    AFE4490_SPI_TX_Buff[2] = ((data >> 8) & 0xFF);            // Write Single byte
    AFE4490_SPI_TX_Buff[3] = (data & 0xFF);         // Write Single byte         // Write Single byte
    
    digitalWrite(AFE4490_CS_PIN,LOW);
    delay(5);
    afe4490SPI.transfer(AFE4490_SPI_TX_Buff[0]);        
    afe4490SPI.transfer(AFE4490_SPI_TX_Buff[1]); 
    afe4490SPI.transfer(AFE4490_SPI_TX_Buff[2]); 
    afe4490SPI.transfer(AFE4490_SPI_TX_Buff[3]);
    delay(5);
    digitalWrite(AFE4490_CS_PIN,HIGH);
    }

void max30205begin()
{
    max30205.begin();
    pinPeripheral(11, PIO_SERCOM);
    pinPeripheral(13, PIO_SERCOM);
    
    I2CwriteByte(MAX30205_ADDRESS, MAX30205_CONFIGURATION, 0x00); //mode config
    I2CwriteByte(MAX30205_ADDRESS, MAX30205_THYST ,      0x00); // set threshold
    I2CwriteByte(MAX30205_ADDRESS, MAX30205_TOS,       0x00); //  
}

// Wire.h read and write protocols
void I2CwriteByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
    max30205.beginTransmission(address);  // Initialize the Tx buffer
    max30205.write(subAddress);           // Put slave register address in Tx buffer
    max30205.write(data);                 // Put data in Tx buffer
    max30205.endTransmission();           // Send the Tx buffer
}

float getTemperature(void){
  uint8_t readRaw[2] = {0}; 
  float temperature; 
    I2CreadBytes(MAX30205_ADDRESS,MAX30205_TEMPERATURE, &readRaw[0] ,2); // read two bytes
  int16_t raw = readRaw[0] << 8 | readRaw[1];  //combine two bytes
    temperature = raw  * 0.00390625;     // convert to temperature
  return  temperature;
}

void I2CreadBytes(uint8_t address, uint8_t subAddress, uint8_t * dest, uint8_t count)
{
  max30205.beginTransmission(address);   // Initialize the Tx buffer
  // Next send the register to be read. OR with 0x80 to indicate multi-read.
  max30205.write(subAddress);     
  max30205.endTransmission(false);
  uint8_t i = 0;
  max30205.requestFrom(address, count);  // Read bytes from slave register address
  while (max30205.available())
  {
    dest[i++] = max30205.read(); 
  }
}

void Resp_FilterProcess(int16_t * RESP_WorkingBuff, int16_t * CoeffBuf, int16_t* FilterOut)
{
  int32_t acc=0;     // accumulator for MACs
  int  k;

// perform the multiply-accumulate
  for ( k = 0; k < 161; k++ )
  {
      acc += (int32_t)(*CoeffBuf++) * (int32_t)(*RESP_WorkingBuff--);
  }
  // saturate the result
  if ( acc > 0x3fffffff )
  {
      acc = 0x3fffffff;
  } else if ( acc < -0x40000000 )
  {
      acc = -0x40000000;
  }
  // convert from Q30 to Q15
  *FilterOut = (int16_t)(acc >> 15);
  
}

int16_t Resp_ProcessCurrSample(int16_t CurrAqsSample)
{
    static uint16_t bufStart=0, bufCur = FILTERORDER-1, FirstFlag = 1;
    
    int16_t temp1, temp2;//, RESPData;

    int16_t RESPData;
    /* Count variable*/
    uint16_t Cur_Chan;
    int16_t FiltOut;
 
    temp1 = NRCOEFF * Pvev_DC_Sample;
    Pvev_DC_Sample = (CurrAqsSample  - Pvev_Sample) + temp1;
    Pvev_Sample = CurrAqsSample;
    temp2 = Pvev_DC_Sample;
    RESPData = (int16_t) temp2;
    
    RESPData = CurrAqsSample;
    
    /* Store the DC removed value in RESP_WorkingBuff buffer in millivolts range*/
    
    //RESP_WorkingBuff[bufCur]=(CurrAqsSample[0]);
    RESP_WorkingBuff[bufCur] = RESPData;
    //FiltOut = RESPData;
    Resp_FilterProcess(&RESP_WorkingBuff[bufCur],RespCoeffBuf,(int16_t*)&FiltOut);
    /* Store the DC removed value in Working buffer in millivolts range*/
    RESP_WorkingBuff[bufStart] = RESPData;

    //int x = *((int*)(&arg));
    /* Store the filtered out sample to the LeadInfo buffer*/
    //FilteredOut = &FiltOut ;//(CurrOut);
    //FilteredOut[0] = CurrAqsSample[0];
    //FilteredOut = CurrAqsSample;
  
    bufCur++;
    bufStart++;
    if ( bufStart  >= (FILTERORDER-1))
    {
      bufStart=0; 
      bufCur = FILTERORDER-1;
    }
  
    return FiltOut;
}
  
void Respiration_Rate_Detection(int16_t Resp_wave)
{ 
    static uint16_t skipCount = 0, SampleCount = 0,TimeCnt=0, SampleCountNtve=0, PtiveCnt =0,NtiveCnt=0 ;
    static int16_t MinThreshold = 0x7FFF, MaxThreshold = 0x8000, PrevSample = 0, PrevPrevSample = 0, PrevPrevPrevSample =0;
    static int16_t MinThresholdNew = 0x7FFF, MaxThresholdNew = 0x8000, AvgThreshold = 0;
    static unsigned char startCalc=0, PtiveEdgeDetected=0, NtiveEdgeDetected=0, peakCount = 0;
    static uint16_t PeakCount[8];
    
    SampleCount++;
    SampleCountNtve++;
    TimeCnt++; 
    if (Resp_wave < MinThresholdNew) MinThresholdNew = Resp_wave;
    if (Resp_wave > MaxThresholdNew) MaxThresholdNew = Resp_wave;
    
    if (SampleCount > 1000)
  {
    SampleCount =0;
  }
  if (SampleCountNtve > 1000)
  {
    SampleCountNtve =0;
  }
  

  if ( startCalc == 1)
  {
    if (TimeCnt >= 500)
    {
      TimeCnt =0;
      if ( (MaxThresholdNew - MinThresholdNew) > 400)
      {
        MaxThreshold = MaxThresholdNew; 
        MinThreshold =  MinThresholdNew;
        AvgThreshold = MaxThreshold + MinThreshold;
        AvgThreshold = AvgThreshold >> 1;
      }
      else
      {
        startCalc = 0;
        Respiration_Rate = 0;
      }
    }

    PrevPrevPrevSample = PrevPrevSample;
    PrevPrevSample = PrevSample;
    PrevSample = Resp_wave;
    if ( skipCount == 0)
    {
      if (PrevPrevPrevSample < AvgThreshold && Resp_wave > AvgThreshold)
      {
        if ( SampleCount > 40 &&  SampleCount < 700)
        {
//            Respiration_Rate = 6000/SampleCount;  // 60 * 100/SampleCount;
          PtiveEdgeDetected = 1;
          PtiveCnt = SampleCount;
          skipCount = 4;
        }
        SampleCount = 0;
      }
      if (PrevPrevPrevSample < AvgThreshold && Resp_wave > AvgThreshold)
      {
        if ( SampleCountNtve > 40 &&  SampleCountNtve < 700)
        {
          NtiveEdgeDetected = 1;
          NtiveCnt = SampleCountNtve;
          skipCount = 4;
        }
        SampleCountNtve = 0;
      }
      
      if (PtiveEdgeDetected ==1 && NtiveEdgeDetected ==1)
      {
        PtiveEdgeDetected = 0;
        NtiveEdgeDetected =0;
        
        if (abs(PtiveCnt - NtiveCnt) < 5)
        {
          PeakCount[peakCount++] = PtiveCnt;
          PeakCount[peakCount++] = NtiveCnt;
          if( peakCount == 8)
          {
            peakCount = 0;
            PtiveCnt = PeakCount[0] + PeakCount[1] + PeakCount[2] + PeakCount[3] + 
                PeakCount[4] + PeakCount[5] + PeakCount[6] + PeakCount[7];
            PtiveCnt = PtiveCnt >> 3;
            Respiration_Rate = 6000/PtiveCnt; // 60 * 125/SampleCount;
          }
        }
      }
    }
    else
    {
      skipCount--;
    }
    
  }
  else
  {
    TimeCnt++;
    if (TimeCnt >= 500)
    {
      TimeCnt = 0;
      if ( (MaxThresholdNew - MinThresholdNew) > 400)
      {
        startCalc = 1;
        MaxThreshold = MaxThresholdNew; 
        MinThreshold =  MinThresholdNew;
        AvgThreshold = MaxThreshold + MinThreshold;
        AvgThreshold = AvgThreshold >> 1;
        PrevPrevPrevSample = Resp_wave;
        PrevPrevSample = Resp_wave;
        PrevSample = Resp_wave;

      }
    }
  }
    //Sanity Check
    
    //if((Respiration_Rate>0)&&(abs(global_RespirationRate-Respiration_Rate)<50))
    //{
      global_RespirationRate=(uint8_t)Respiration_Rate;
    //}
}
  
void RESP_Algorithm_Interface(int16_t CurrSample)
{
    static int16_t prev_data[64] ={0};
    //static unsigned char Decimeter = 0;
    char i;
    long Mac=0;
    prev_data[0] = CurrSample;
    for ( i=63; i > 0; i--)
    {
      Mac += prev_data[i];
      prev_data[i] = prev_data[i-1];
  
    }
    Mac += CurrSample;
  //  Mac = Mac;
    CurrSample = (int16_t) Mac >> 1;
    RESP_Second_Prev_Sample = RESP_Prev_Sample ;
    RESP_Prev_Sample = RESP_Current_Sample ;
    RESP_Current_Sample = RESP_Next_Sample ;
    RESP_Next_Sample = RESP_Second_Next_Sample ;
    RESP_Second_Next_Sample = CurrSample;// << 3 ;
  //  fprintf(fp,"%d\n", CurrSample);
    //Decimeter++;
    //Resp_Rr_val = RESP_Second_Next_Sample;
    //if ( Decimeter == 5)
    //{
    //  Decimeter = 0;
  //    RESP_process_buffer();
      Respiration_Rate_Detection(RESP_Second_Next_Sample);
    //}
}

void ECG_FilterProcess(int16_t * WorkingBuff, int16_t * CoeffBuf, int16_t* FilterOut)
{

    int32_t acc=0;     // accumulator for MACs
    int  k;

 // perform the multiply-accumulate
    for ( k = 0; k < 161; k++ )
    {
        acc += (int32_t)(*CoeffBuf++) * (int32_t)(*WorkingBuff--);
    }
    // saturate the result
    if ( acc > 0x3fffffff )
    {
        acc = 0x3fffffff;
    } else if ( acc < -0x40000000 )
    {
        acc = -0x40000000;
    }
    // convert from Q30 to Q15
    *FilterOut = (int16_t)(acc >> 15);
}

int16_t ECG_ProcessCurrSample(int16_t CurrAqsSample)
{
  
    static uint16_t ECG_bufStart=0, ECG_bufCur = FILTERORDER-1, ECGFirstFlag = 1;
    /* Working Buffer Used for Filtering*/
    //  static short ECG_WorkingBuff[2 * FILTERORDER];
    int16_t *CoeffBuf;
    int16_t temp1, temp2, ECGData;

    /* Count variable*/
    uint16_t Cur_Chan;
    int16_t FiltOut=0;
    //  short FilterOut[2];
    CoeffBuf = CoeffBuf_40Hz_LowPass;         // Default filter option is 40Hz LowPass

    if  ( ECGFirstFlag )                // First Time initialize static variables.
    {


        ECGFirstFlag = 0;
    }

    temp1 = NRCOEFF * ECG_Pvev_DC_Sample;       //First order IIR
    ECG_Pvev_DC_Sample = (CurrAqsSample  - ECG_Pvev_Sample) + temp1;
    ECG_Pvev_Sample = CurrAqsSample;
    temp2 = ECG_Pvev_DC_Sample>>2 ;
    ECGData = (int16_t) temp2;

    /* Store the DC removed value in Working buffer in millivolts range*/
    ECG_WorkingBuff[ECG_bufCur] = ECGData;
    ECG_FilterProcess(&ECG_WorkingBuff[ECG_bufCur],CoeffBuf,(int16_t*)&FiltOut);
    /* Store the DC removed value in ECG_WorkingBuff buffer in millivolts range*/
    ECG_WorkingBuff[ECG_bufStart] = ECGData;

    /* Store the filtered out sample to the LeadInfo buffer*/
    //FiltOut = CurrAqsSample;//(CurrOut);
    // FilteredOut[0] = CurrAqsSample[0];

    ECG_bufCur++;
    ECG_bufStart++;

    if ( ECG_bufStart  == (FILTERORDER-1))
    {
        ECG_bufStart=0;
        ECG_bufCur = FILTERORDER-1;
    }
    return FiltOut;
}


void QRS_Algorithm_Interface(int16_t CurrSample)
{
    static int16_t prev_data[32] ={0};
    int16_t i;
    long Mac=0;
    prev_data[0] = CurrSample;
    for ( i=31; i > 0; i--)
    {
      Mac += prev_data[i];
      prev_data[i] = prev_data[i-1];  
    }
    Mac += CurrSample;
    Mac = Mac >> 2;
    CurrSample = (int16_t) Mac;
    QRS_Second_Prev_Sample = QRS_Prev_Sample ;
    QRS_Prev_Sample = QRS_Current_Sample ;
    QRS_Current_Sample = QRS_Next_Sample ;
    QRS_Next_Sample = QRS_Second_Next_Sample ;
    QRS_Second_Next_Sample = CurrSample ;
      
    QRS_process_buffer();
}

static void QRS_process_buffer( void )
{

  int16_t first_derivative = 0 ;
  int16_t scaled_result = 0 ;

  static int16_t max = 0 ;

  /* calculating first derivative*/
  first_derivative = QRS_Next_Sample - QRS_Prev_Sample  ;
  
  /*taking the absolute value*/
  if(first_derivative < 0)
  {
    first_derivative = -(first_derivative);
  }

  scaled_result = first_derivative;

  if( scaled_result > max )
  {
    max = scaled_result ;
  }

  QRS_B4_Buffer_ptr++;
  if (QRS_B4_Buffer_ptr ==  TWO_SEC_SAMPLES)
  {
    QRS_Threshold_Old = ((max *7) /10 ) ;
    QRS_Threshold_New = QRS_Threshold_Old ;
    //if(max > 70)
    first_peak_detect = TRUE ;
    max = 0;
    QRS_B4_Buffer_ptr = 0;
  }


  if( TRUE == first_peak_detect )
  {
    QRS_check_sample_crossing_threshold( scaled_result ) ;
  }
  //scaled_result_display[indx++] =  scaled_result ;  

}

static void QRS_check_sample_crossing_threshold( uint16_t scaled_result )
{
  /* array to hold the sample indexes S1,S2,S3 etc */
  
  static uint16_t s_array_index = 0 ;
  static uint16_t m_array_index = 0 ;
  
  static unsigned char threshold_crossed = FALSE ;
  static uint16_t maxima_search = 0 ;
  static unsigned char peak_detected = FALSE ;
  static uint16_t skip_window = 0 ;
  static long maxima_sum = 0 ;
  static unsigned int peak = 0;
  static unsigned int sample_sum = 0;
  static unsigned int nopeak=0;
  uint16_t max = 0 ;
  uint16_t HRAvg;
  uint16_t  RRinterval =0;

  if( TRUE == threshold_crossed  )
  {
    /*
    Once the sample value crosses the threshold check for the
    maxima value till MAXIMA_SEARCH_WINDOW samples are received
    */
    sample_count ++ ;
    maxima_search ++ ;

    if( scaled_result > peak )
    {
      peak = scaled_result ;
    }

    if( maxima_search >= MAXIMA_SEARCH_WINDOW )
    {
      // Store the maxima values for each peak
      maxima_sum += peak ;
      maxima_search = 0 ;

  
      threshold_crossed = FALSE ;
      peak_detected = TRUE ;
    }

  }
  else if( TRUE == peak_detected )
  {
    /*
    Once the sample value goes below the threshold
    skip the samples untill the SKIP WINDOW criteria is meet
    */
    sample_count ++ ;
    skip_window ++ ;

    if( skip_window >= MINIMUM_SKIP_WINDOW )
    {
      skip_window = 0 ;
      peak_detected = FALSE ;
    }

    if( m_array_index == MAX_PEAK_TO_SEARCH )
    {
      sample_sum = sample_sum / (MAX_PEAK_TO_SEARCH - 1);
      HRAvg =  (uint16_t) sample_sum  ;

      // Compute HR without checking LeadOffStatus
      QRS_Heart_Rate = (uint16_t) 60 *  SAMPLING_RATE;
      QRS_Heart_Rate =  QRS_Heart_Rate/ HRAvg ;
      if(QRS_Heart_Rate > 250)
        QRS_Heart_Rate = 250 ;

      /* Setting the Current HR value in the ECG_Info structure*/
      maxima_sum =  maxima_sum / MAX_PEAK_TO_SEARCH;
      max = (int16_t) maxima_sum ;
      /*  calculating the new QRS_Threshold based on the maxima obtained in 4 peaks */
      maxima_sum = max * 7;
      maxima_sum = maxima_sum/10;
      QRS_Threshold_New = (int16_t)maxima_sum;

      /* Limiting the QRS Threshold to be in the permissible range*/
      if(QRS_Threshold_New > (4 * QRS_Threshold_Old))
      {
        QRS_Threshold_New = QRS_Threshold_Old;
      }

      sample_count = 0 ;
      s_array_index = 0 ;
      m_array_index = 0 ;
      maxima_sum = 0 ;
      sample_index[0] = 0 ;
      sample_index[1] = 0 ;
      sample_index[2] = 0 ;
      sample_index[3] = 0 ;
      Start_Sample_Count_Flag = 0;

      sample_sum = 0;
    }
  }
  else if( scaled_result > QRS_Threshold_New )
  {
    /*
      If the sample value crosses the threshold then store the sample index
    */
    Start_Sample_Count_Flag = 1;
    sample_count ++ ;
    m_array_index++;
    threshold_crossed = TRUE ;
    peak = scaled_result ;
    nopeak = 0;

    /*  storing sample index*/
      sample_index[ s_array_index ] = sample_count ;
    if( s_array_index >= 1 )
    {
      sample_sum += sample_index[ s_array_index ] - sample_index[ s_array_index - 1 ] ;
    }
    s_array_index ++ ;
  }

  else if(( scaled_result < QRS_Threshold_New ) && (Start_Sample_Count_Flag == 1))
  {
    sample_count ++ ;
        nopeak++; 
        if (nopeak > (3 * SAMPLING_RATE))
        { 
          sample_count = 0 ;
      s_array_index = 0 ;
      m_array_index = 0 ;
      maxima_sum = 0 ;
      sample_index[0] = 0 ;
      sample_index[1] = 0 ;
      sample_index[2] = 0 ;
      sample_index[3] = 0 ;
      Start_Sample_Count_Flag = 0;
      peak_detected = FALSE ;
      sample_sum = 0;
                
          first_peak_detect = FALSE;
          nopeak=0;

      QRS_Heart_Rate = 0;

        }
  }
   else
   {
    nopeak++; 
    if (nopeak > (3 * SAMPLING_RATE))
     { 
    /* Reset heart rate computation sate variable in case of no peak found in 3 seconds */
    sample_count = 0 ;
    s_array_index = 0 ;
    m_array_index = 0 ;
    maxima_sum = 0 ;
    sample_index[0] = 0 ;
    sample_index[1] = 0 ;
    sample_index[2] = 0 ;
    sample_index[3] = 0 ;
    Start_Sample_Count_Flag = 0;
    peak_detected = FALSE ;
    sample_sum = 0;
      first_peak_detect = FALSE;
    nopeak = 0;
    QRS_Heart_Rate = 0;
     }
   }
   global_HeartRate= (uint8_t)(QRS_Heart_Rate);
   //SerialUSB.println(global_HeartRate);
}

void estimate_spo2(uint16_t *pun_ir_buffer, int32_t n_ir_buffer_length, uint16_t *pun_red_buffer, int32_t *pn_spo2, int8_t *pch_spo2_valid)
{
  uint32_t un_ir_mean,un_only_once ;
  int32_t k, n_i_ratio_count;
  int32_t i, s, m, n_exact_ir_valley_locs_count, n_middle_idx;
  int32_t n_th1, n_npks, n_c_min;   
  int32_t an_ir_valley_locs[15] ;
  int32_t n_peak_interval_sum;
  
  int32_t n_y_ac, n_x_ac;
  int32_t n_spo2_calc; 
  int32_t n_y_dc_max, n_x_dc_max; 
  int32_t n_y_dc_max_idx, n_x_dc_max_idx; 
  int32_t an_ratio[5], n_ratio_average; 
  int32_t n_nume, n_denom ;

  // calculates DC mean and subtract DC from ir
  un_ir_mean =0; 
  for (k=0 ; k<n_ir_buffer_length ; k++ ) un_ir_mean += pun_ir_buffer[k] ;
  un_ir_mean =un_ir_mean/n_ir_buffer_length ;
    
  // remove DC and invert signal so that we can use peak detector as valley detector
  for (k=0 ; k<n_ir_buffer_length ; k++ )  
    an_x[k] = -1*(pun_ir_buffer[k] - un_ir_mean) ; 
    
  // 4 pt Moving Average
  for(k=0; k< BUFFER_SIZE-MA4_SIZE; k++){
    an_x[k]=( an_x[k]+an_x[k+1]+ an_x[k+2]+ an_x[k+3])/(int)4;        
  }
  // calculate threshold  
  n_th1=0; 
  for ( k=0 ; k<BUFFER_SIZE ;k++){
    n_th1 +=  an_x[k];
  }
  n_th1=  n_th1/ ( BUFFER_SIZE);
  if( n_th1<30) n_th1=30; // min allowed
  if( n_th1>60) n_th1=60; // max allowed

  for ( k=0 ; k<15;k++) an_ir_valley_locs[k]=0;
  // since we flipped signal, we use peak detector as valley detector
  find_peak( an_ir_valley_locs, &n_npks, an_x, BUFFER_SIZE, n_th1, 4, 15 );//peak_height, peak_distance, max_num_peaks 
  n_peak_interval_sum =0;
  if (n_npks>=2){
    for (k=1; k<n_npks; k++) n_peak_interval_sum += (an_ir_valley_locs[k] -an_ir_valley_locs[k -1] ) ;
    n_peak_interval_sum =n_peak_interval_sum/(n_npks-1);
    //*pn_heart_rate =(int32_t)( (FS*60)/ n_peak_interval_sum );
    //*pch_hr_valid  = 1;
  }
  else  { 
    //*pn_heart_rate = -999; // unable to calculate because # of peaks are too small
    //*pch_hr_valid  = 0;
  }

  //  load raw value again for SPO2 calculation : RED(=y) and IR(=X)
  for (k=0 ; k<n_ir_buffer_length ; k++ )  {
      an_x[k] =  pun_ir_buffer[k] ; 
      an_y[k] =  pun_red_buffer[k] ; 
  }

  // find precise min near an_ir_valley_locs
  n_exact_ir_valley_locs_count =n_npks; 
  
  //using exact_ir_valley_locs , find ir-red DC andir-red AC for SPO2 calibration an_ratio
  //finding AC/DC maximum of raw

  n_ratio_average =0; 
  n_i_ratio_count = 0; 
  for(k=0; k< 5; k++) an_ratio[k]=0;
  for (k=0; k< n_exact_ir_valley_locs_count; k++){
    if (an_ir_valley_locs[k] > BUFFER_SIZE ){
      *pn_spo2 =  -999 ; // do not use SPO2 since valley loc is out of range
      *pch_spo2_valid  = 0; 
      return;
    }
  }
  // find max between two valley locations 
  // and use an_ratio betwen AC compoent of Ir & Red and DC compoent of Ir & Red for SPO2 
  for (k=0; k< n_exact_ir_valley_locs_count-1; k++){
    n_y_dc_max= -16777216 ; 
    n_x_dc_max= -16777216; 
    if (an_ir_valley_locs[k+1]-an_ir_valley_locs[k] >3){
        for (i=an_ir_valley_locs[k]; i< an_ir_valley_locs[k+1]; i++){
          if (an_x[i]> n_x_dc_max) {n_x_dc_max =an_x[i]; n_x_dc_max_idx=i;}
          if (an_y[i]> n_y_dc_max) {n_y_dc_max =an_y[i]; n_y_dc_max_idx=i;}
      }
      n_y_ac= (an_y[an_ir_valley_locs[k+1]] - an_y[an_ir_valley_locs[k] ] )*(n_y_dc_max_idx -an_ir_valley_locs[k]); //red
      n_y_ac=  an_y[an_ir_valley_locs[k]] + n_y_ac/ (an_ir_valley_locs[k+1] - an_ir_valley_locs[k])  ; 
      n_y_ac=  an_y[n_y_dc_max_idx] - n_y_ac;    // subracting linear DC compoenents from raw 
      n_x_ac= (an_x[an_ir_valley_locs[k+1]] - an_x[an_ir_valley_locs[k] ] )*(n_x_dc_max_idx -an_ir_valley_locs[k]); // ir
      n_x_ac=  an_x[an_ir_valley_locs[k]] + n_x_ac/ (an_ir_valley_locs[k+1] - an_ir_valley_locs[k]); 
      n_x_ac=  an_x[n_y_dc_max_idx] - n_x_ac;      // subracting linear DC compoenents from raw 
      n_nume=( n_y_ac *n_x_dc_max)>>7 ; //prepare X100 to preserve floating value
      n_denom= ( n_x_ac *n_y_dc_max)>>7;
      if (n_denom>0  && n_i_ratio_count <5 &&  n_nume != 0)
      {   
        an_ratio[n_i_ratio_count]= (n_nume*100)/n_denom ; //formular is ( n_y_ac *n_x_dc_max) / ( n_x_ac *n_y_dc_max) ;
        n_i_ratio_count++;
      }
    }
  }
  // choose median value since PPG signal may varies from beat to beat
  sort_ascend(an_ratio, n_i_ratio_count);
  n_middle_idx= n_i_ratio_count/2;

  if (n_middle_idx >1)
    n_ratio_average =( an_ratio[n_middle_idx-1] +an_ratio[n_middle_idx])/2; // use median
  else
    n_ratio_average = an_ratio[n_middle_idx ];

  if( n_ratio_average>2 && n_ratio_average <184){
    n_spo2_calc= uch_spo2_table[n_ratio_average] ;
    *pn_spo2 = n_spo2_calc ;
    *pch_spo2_valid  = 1;//  float_SPO2 =  -45.060*n_ratio_average* n_ratio_average/10000 + 30.354 *n_ratio_average/100 + 94.845 ;  // for comparison with table
  }
  else{
    *pn_spo2 =  -999 ; // do not use SPO2 since signal an_ratio is out of range
    *pch_spo2_valid  = 0; 
  }
}

void find_peak( int32_t *pn_locs, int32_t *n_npks,  int32_t  *pn_x, int32_t n_size, int32_t n_min_height, int32_t n_min_distance, int32_t n_max_num )
/**
* \brief        Find peaks
* \par          Details
*               Find at most MAX_NUM peaks above MIN_HEIGHT separated by at least MIN_DISTANCE
*
* \retval       None
*/
{
    find_peak_above( pn_locs, n_npks, pn_x, n_size, n_min_height );
    remove_close_peaks( pn_locs, n_npks, pn_x, n_min_distance );
    *n_npks = min( *n_npks, n_max_num );
}

void find_peak_above( int32_t *pn_locs, int32_t *n_npks,  int32_t  *pn_x, int32_t n_size, int32_t n_min_height )
/**
* \brief        Find peaks above n_min_height
* \par          Details
*               Find all peaks above MIN_HEIGHT
*
* \retval       None
*/
{
  int32_t i = 1, n_width;
  *n_npks = 0;
  
  while (i < n_size-1){
    if (pn_x[i] > n_min_height && pn_x[i] > pn_x[i-1]){      // find left edge of potential peaks
      n_width = 1;
      while (i+n_width < n_size && pn_x[i] == pn_x[i+n_width])  // find flat peaks
        n_width++;
      if (pn_x[i] > pn_x[i+n_width] && (*n_npks) < 15 ){      // find right edge of peaks
        pn_locs[(*n_npks)++] = i;    
        // for flat peaks, peak location is left edge
        i += n_width+1;
      }
      else
        i += n_width;
    }
    else
      i++;
    //  Serial.println("beat");
  }
}

void remove_close_peaks(int32_t *pn_locs, int32_t *pn_npks, int32_t *pn_x, int32_t n_min_distance)
/**
* \brief        Remove peaks
* \par          Details
*               Remove peaks separated by less than MIN_DISTANCE
*
* \retval       None
*/
{
    
  int32_t i, j, n_old_npks, n_dist;
    
  /* Order peaks from large to small */
  sort_indices_descend( pn_x, pn_locs, *pn_npks );

  for ( i = -1; i < *pn_npks; i++ ){
    n_old_npks = *pn_npks;
    *pn_npks = i+1;
    for ( j = i+1; j < n_old_npks; j++ ){
      n_dist =  pn_locs[j] - ( i == -1 ? -1 : pn_locs[i] ); // lag-zero peak of autocorr is at index -1
      if ( n_dist > n_min_distance || n_dist < -n_min_distance )
        pn_locs[(*pn_npks)++] = pn_locs[j];
    }
  }

  // Resort indices int32_to ascending order
  sort_ascend( pn_locs, *pn_npks );
}

void sort_ascend(int32_t  *pn_x, int32_t n_size) 
/**
* \brief        Sort array
* \par          Details
*               Sort array in ascending order (insertion sort algorithm)
*
* \retval       None
*/
{
  int32_t i, j, n_temp;
  for (i = 1; i < n_size; i++) {
    n_temp = pn_x[i];
    for (j = i; j > 0 && n_temp < pn_x[j-1]; j--)
        pn_x[j] = pn_x[j-1];
    pn_x[j] = n_temp;
  }
}

void sort_indices_descend(  int32_t  *pn_x, int32_t *pn_indx, int32_t n_size)
/**
* \brief        Sort indices
* \par          Details
*               Sort indices according to descending order (insertion sort algorithm)
*
* \retval       None
*/ 
{
  int32_t i, j, n_temp;
  for (i = 1; i < n_size; i++) {
    n_temp = pn_indx[i];
    for (j = i; j > 0 && pn_x[n_temp] > pn_x[pn_indx[j-1]]; j--)
      pn_indx[j] = pn_indx[j-1];
    pn_indx[j] = n_temp;
  }
}

