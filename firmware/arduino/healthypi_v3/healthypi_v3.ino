/*
  HealthyPi 3 firmware for the v3 board
  Program with Arduino Zero/M0 bootloader

  More information on https://github.com/Protocentral/protocentral-healthypi-v3

  License: MIT License (https://opensource.org/licenses/MIT)
  
  ProtoCentral (www.protocentral.com) 2017
*/
 
#include <SPI.h>
#include <Wire.h>
#include "wiring_private.h" // pinPeripheral() function
#include "ads1292r.h"
#include "afe4490.h"
#include "max30205.h"

#define IOPORT_PIN_LEVEL_HIGH 1
#define IOPORT_PIN_LEVEL_LOW 0

#define ADS1292R_CS_PIN   3
#define ADS1292R_START_PIN 10
#define ADS1292R_PWDN_PIN 12
#define ADS1292R_DRDY_PIN A4
#define AFE4490_START_PIN  16//A2
#define AFE4490_CS_PIN     15//A1
#define AFE4490_DRDY_PIN   A0 


SPIClass ads1292rSPI (&sercom2, 4, 0, 1, SPI_PAD_2_SCK_3, SERCOM_RX_PAD_0);

SPIClass afe4490SPI (&sercom0, 17, 9 , 8, SPI_PAD_2_SCK_3, SERCOM_RX_PAD_0);    // ads1220 17 -->PA4 9-->  8-->

Uart  RpiSerial(&sercom4, 38, 22 , SERCOM_RX_PAD_1, UART_TX_PAD_0);    // ads1220 17 -->PA4 9-->  8-->

TwoWire max30205(&sercom1, 11, 13);


//Low pass bessel filter order=5 alpha1=0.016 
class filter
{
  public:
    filter()
    {
      for(int i=0; i <= 5; i++)
        v[i]=0;
    }
  private:
    short v[6];
  public:
    short step(short x)
    {
      v[0] = v[1];
      v[1] = v[2];
      v[2] = v[3];
      v[3] = v[4];
      v[4] = v[5];
      long tmp = ((((x * 730349L) >> 21)  //= (   2.6569958266e-6 * x)
        + ((v[0] * 563392L) >> 3) //+(  0.5372920725*v[0])
        + ((v[1] * -793380L) >> 1)  //+( -3.0265040083*v[1])
        + (v[2] * 895973L)  //+(  6.8357313605*v[2])
        + (v[3] * -1014392L)  //+( -7.7391952732*v[3])
        + (v[4] * 575746L)  //+(  4.3925908247*v[4])
        )+65536) >> 17; // round and downshift fixed point /131072

      v[5]= (short)tmp;
      return (short)((/* xpart */
         (((v[0] + v[5]))<<13) /* 524288L (^2)*/
         + ((655360L * (v[1] + v[4]))>>1)
         + /* xi==xj*/(655360L * (v[2] + v[3]))
        )
+32768) >> 16; // round and downshift fixed point

    }
};

filter fbp;// FilterBeHp5();

int8_t NewDataAvailable;
uint8_t data_len = 23;

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
volatile uint8_t    DataPacket[44];
uint8_t AFE4490_SPI_TX_Buff[10];
volatile uint8_t  AFE4490_SPI_RX_Buff[3]={0,0,0};
volatile uint32_t AFE4490resultTemp = 0;
volatile int32_t AFE4490sresultTempIR,AFE4490sresultTempRED;
volatile float AFE4490fresultTemp = 0;
volatile int32_t AFE4490iresultTemp = 0;
volatile int32_t AFE4490voltageTemp = 0;
volatile bool afe4490DataReceived = false;
float  temp ;
uint16_t temperature;
uint8_t heartRate_ads1292=0;
uint8_t spo2=0;
uint8_t heartRate_BP=0;

volatile long v[6];

void ads1292_detection_callback(void)
{
       digitalWrite(ADS1292R_CS_PIN,LOW);

     for (int x = 0; x < 9 ; x++)
    {
      SPI_Dummy_Buff[x] = ads1292rSPI.transfer(0xff);
      // SerialUSB.print(SPI_Dummy_Buff[x],HEX); 
    }

    digitalWrite(ADS1292R_CS_PIN,HIGH);
    resultTemp = (unsigned long)((0x00 << 24) | (SPI_Dummy_Buff[6] << 16)| SPI_Dummy_Buff[7] << 8 | SPI_Dummy_Buff[8]);//6,7,8
    resultTemp = (unsigned long)(resultTemp << 8);

    sresultTemp = (signed long)(resultTemp);
    sresultTemp = (signed long)(sresultTemp >> 8);   //  resultTemp = (uint32_t)(resultTemp << 8);
    
    resultTemp = (uint32_t)((0x00 << 24) | (SPI_Dummy_Buff[3] << 16)| SPI_Dummy_Buff[4] << 8 | SPI_Dummy_Buff[5]);//6,7,8
    resultTemp = (uint32_t)(resultTemp << 8);

    sresultTempResp = (long)(resultTemp);
    sresultTempResp = (sresultTempResp >> 8);   //  resultTemp = (uint32_t)(resultTemp << 8);

    sresultTempResp = Filterstep(sresultTempResp);
    ads1292DataReceived = true; 
}

long Filterstep(long x)
    {
      v[0] = v[1];
      v[1] = v[2];
      long tmp = ((((x *    126L) >>  9)  //= (   3.8374579819e-3 * x)
        + ((v[0] * -116L) >> 1) //+( -0.9074493728*v[0])
        + (v[1] * 121L) //+(  1.8920995408*v[1])
        )+32) >> 6; // round and downshift fixed point /64

      v[2]= (long)tmp;
      return (long)((
         (v[0] + v[2])
        +2 * v[1])); // 2^
    }

void afe4490_detection_callback(void)
{
   //   extint_chan_disable_callback(DRDY_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
   
  /* AFE4490Write(CONTROL0,0x000001);  
  
   AFE4490_SPI_TX_Buff[0] =  0x2C;

   digitalWrite(AFE4490_CS_PIN,LOW);
   afe4490SPI.transfer(AFE4490_SPI_TX_Buff[0]);
  
   AFE4490_SPI_RX_Buff[0] =  afe4490SPI.transfer(0xff);
   AFE4490_SPI_RX_Buff[1] =  afe4490SPI.transfer(0xff);
   AFE4490_SPI_RX_Buff[2] =  afe4490SPI.transfer(0xff);
   
   AFE4490resultTemp = (uint32_t)( ((0x00 << 24) | AFE4490_SPI_RX_Buff[0] << 16)| AFE4490_SPI_RX_Buff[1] << 8 | AFE4490_SPI_RX_Buff[2]);
   AFE4490resultTemp = (uint32_t)(AFE4490resultTemp << 10);
    
   AFE4490sresultTempIR = (int32_t)(AFE4490resultTemp );
   AFE4490sresultTempIR = (AFE4490sresultTempIR >> 10);// * 100;
   
   AFE4490_SPI_TX_Buff[0] =  0x2A;

   afe4490SPI.transfer(AFE4490_SPI_TX_Buff[0]);
  
   AFE4490_SPI_RX_Buff[0] =  afe4490SPI.transfer(0xff);
   AFE4490_SPI_RX_Buff[1] =  afe4490SPI.transfer(0xff);
   AFE4490_SPI_RX_Buff[2] =  afe4490SPI.transfer(0xff);
   
   AFE4490resultTemp = (uint32_t)( ((0x00 << 24) | AFE4490_SPI_RX_Buff[0] << 16)| AFE4490_SPI_RX_Buff[1] << 8 | AFE4490_SPI_RX_Buff[2]);
   AFE4490resultTemp = (uint32_t)(AFE4490resultTemp << 10);
   
   AFE4490sresultTempRED = (int32_t)(AFE4490resultTemp );
   AFE4490sresultTempRED = (AFE4490sresultTempRED >> 10);// * 100;   
   
   digitalWrite(AFE4490_CS_PIN,HIGH); */
   afe4490DataReceived = true;
  
   count++;
}

void setup() {
  SerialUSB.begin(115200);
  RpiSerial.begin(57600);
  // do this first, for Reasons

  max30205begin();
 
  ads1292Rbegin();
  delay(100);
  afe4490begin();   

  for(int i=0; i <= 5; i++)
        v[i]=0;
}

uint8_t i = 0;
void loop() {

#if 1
  if(!(millis()%1000))
  {
       SerialUSB.println(count); // print to check
       //AFE4490Write(CONTROL0,0x000008);
    count = 0;
  }
  
    if(afe4490DataReceived == true)
  {


    AFE4490Write(CONTROL0,0x000001);  
  
   AFE4490_SPI_TX_Buff[0] =  0x2C;

   digitalWrite(AFE4490_CS_PIN,LOW);
   afe4490SPI.transfer(AFE4490_SPI_TX_Buff[0]);
  
   AFE4490_SPI_RX_Buff[0] =  afe4490SPI.transfer(0xff);
   AFE4490_SPI_RX_Buff[1] =  afe4490SPI.transfer(0xff);
   AFE4490_SPI_RX_Buff[2] =  afe4490SPI.transfer(0xff);
   
   AFE4490resultTemp = (uint32_t)( ((0x00 << 24) | AFE4490_SPI_RX_Buff[0] << 16)| AFE4490_SPI_RX_Buff[1] << 8 | AFE4490_SPI_RX_Buff[2]);
   AFE4490resultTemp = (uint32_t)(AFE4490resultTemp << 10);
    
   AFE4490sresultTempIR = (int32_t)(AFE4490resultTemp );
   AFE4490sresultTempIR = (AFE4490sresultTempIR >> 10);// * 100;
   
   AFE4490_SPI_TX_Buff[0] =  0x2A;

   afe4490SPI.transfer(AFE4490_SPI_TX_Buff[0]);
  
   AFE4490_SPI_RX_Buff[0] =  afe4490SPI.transfer(0xff);
   AFE4490_SPI_RX_Buff[1] =  afe4490SPI.transfer(0xff);
   AFE4490_SPI_RX_Buff[2] =  afe4490SPI.transfer(0xff);
   
   AFE4490resultTemp = (uint32_t)( ((0x00 << 24) | AFE4490_SPI_RX_Buff[0] << 16)| AFE4490_SPI_RX_Buff[1] << 8 | AFE4490_SPI_RX_Buff[2]);
   AFE4490resultTemp = (uint32_t)(AFE4490resultTemp << 10);
   
   AFE4490sresultTempRED = (int32_t)(AFE4490resultTemp );
   AFE4490sresultTempRED = (AFE4490sresultTempRED >> 10);// * 100;   
   
   digitalWrite(AFE4490_CS_PIN,HIGH);

   temp = getTemperature()*100; // read temperature for every 100ms
   temperature =  (uint16_t) temp;
  
    DataPacket[0] = 0x0A;  // sync0
    DataPacket[1] = 0xFA;
    DataPacket[2] = (uint8_t) (data_len);
    DataPacket[3] = (uint8_t) (data_len>>8);
    DataPacket[4] = 0x02;

    DataPacket[5] =  (sresultTemp);
    DataPacket[6] = (sresultTemp>>8);
    DataPacket[7] =  (sresultTemp >> 16);
    DataPacket[8] =  (sresultTemp >>24);
    
    DataPacket[9] =  (sresultTempResp);
    DataPacket[10] =  (sresultTempResp >> 8);
    DataPacket[11] =  (sresultTempResp >> 16);
    DataPacket[12] =  (sresultTempResp >>24);    

    DataPacket[13] =  AFE4490sresultTempRED;
    DataPacket[14] =  AFE4490sresultTempRED>>8;
    DataPacket[15] =  AFE4490sresultTempRED>>16;
    DataPacket[16] =  AFE4490sresultTempRED>>24;
    
    DataPacket[17] =  AFE4490sresultTempIR;
    DataPacket[18] =  AFE4490sresultTempIR>>8;
    DataPacket[19] =  AFE4490sresultTempIR>>16;
    DataPacket[20] =  AFE4490sresultTempIR>>24;


//SerialUSB.println(temp);
    DataPacket[21] =  (uint8_t) temperature;
    DataPacket[22] =  (uint8_t) (temperature >> 8);

    
    DataPacket[23] =  heartRate_ads1292;   // calculated from ads1292r
    DataPacket[24] =  spo2;

    DataPacket[25] =  120;   // systolic pressure
    DataPacket[26] =  80;     //diastolic pressure
    DataPacket[27] =  heartRate_BP;  // from BP module
      
    DataPacket[28] = 0x00;
    DataPacket[29] = 0x0b;
  
   for(int i=0; i<30; i++) // transmit the data
   {
      SerialUSB.write(DataPacket[i]);
     //delayMicroseconds(10);
      RpiSerial.write(DataPacket[i]);        
   }
    afe4490DataReceived =  false;
    
    }
#endif


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
  
  ADS1292_Reg_Write(0x02,0b10100000); //Lead-off comp off, test signal disabled
  delay(10);
  ADS1292_Reg_Write(0x03,0b00010000); //Lead-off defaults
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

float getTemperature(void)
{
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
