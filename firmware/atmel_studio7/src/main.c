
#include <asf.h>
#include "ads1292.h"
#include "afe4490.h"
#include <stdio.h>
#include "string.h"
#include "userusart.h"

#define READ_MODE			0
#define SEND_MODE			1
#define CALIBRATION_MODE			2

//Calibration
#define CALIB_SAMPLE 100
#define MAX_LENGTH 10
#define MIN_LENGTH 1

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);

extern volatile uint8_t	SPI_RX_Buff[100];
extern volatile uint8_t AFE4490_SPI_RX_Buff[100];
extern volatile S32 sresultTemp,sresultTempResp;
extern volatile bool ads1292DataReceived;
extern volatile bool afe4490DataReceived;
char result[50];
volatile uint8_t ads1292_rx_Data[10];
volatile int32_t uads1292temp = 0;
volatile int32_t sads1292temp=0;
float volt=0,FCalPres=0;
volatile int i;
int count=0;
uint8_t data_len = 19;
uint8_t DataPacketHeader[40];
uint32_t volt_int32,CalPressure_int32 ;
uint32_t CalPressure;
volatile uint32_t ConvFactor = 0;
volatile uint32_t packet_counter=0;
uint8_t indx;
int16_t dly=0;
volatile uint8_t mode=10;
volatile uint8_t CalibStep=0;

uint8_t ofcal0,ofcal1,ofcal2;
volatile uint32_t FirstCalib=0, dryCalib=0, FullTankCalib=0;
float fvolt=0;
int c=0;

volatile U8		DataPacket[44];
extern volatile S32 AFE4490sresultTempIR,AFE4490sresultTempRED;

void SendAckData();
void SendNegAckData();
float readChannel();
void  getCalibrateData();
void resetAlldata();
void SendCalibDatatoApplication();
void ResetFlag();
void SendSelfTest();




int main(void)
{
	irq_initialize_vectors();
	cpu_irq_enable();
	delay_init();
	system_init();

	sercom_usart_init();

	AFE4490_Init();
	ADS1292_Init();

	
	system_interrupt_enable_global();

while (true)
 {

//	 usart_write_buffer_wait(&usart_instance,"Protocentral", 15);
//	 delay_ms(10000);
#if 0 // brainbay
		if(ads1292DataReceived == true)
		{
	
				packet_counter++; //if(Pkt_Counter > 0) Pkt_Counter = 0x00;
			
				DataPacket[0] = 0xA0;  // sync0
				DataPacket[1] = 36;   //sync1
			
				DataPacket[2] = (unsigned char)(packet_counter >> 24);
				DataPacket[3] = (unsigned char)(packet_counter >> 16);
				DataPacket[4] = (unsigned char)(packet_counter >> 8);
				DataPacket[5] = (unsigned char)(packet_counter );
			

			
				DataPacket[6] = (unsigned char) (sresultTemp);
				DataPacket[7] = (unsigned char) (sresultTemp>>8);   //>>8
				DataPacket[8] = (unsigned char) (sresultTemp >> 16); //>>16
				DataPacket[9] = (unsigned char) (sresultTemp >>24);
			
				for(indx=0;indx<28;indx++)  // fill channel 1 buff
				{
					//ueegtemp = (uint16_t) ( (EEG_Ch1_Data[pckt+1]<<8)| EEG_Ch1_Data[pckt]);
					//seegtemp = (int16_t) (ueegtemp);
					//pckt++;
					//DataPacket[ ] = EEG_Ch1_Data[pckt++] ;
					DataPacket[10+indx] = 0x00 ;
				}
			
				DataPacket[38] = 0xc0;//(uint8_t)seegtemp ;
			
				//usart_putchar(BLUETOOTH_UART,DataPacket[indx]);
				usart_write_buffer_wait(&usart_instance,DataPacket, 39);
					//delay_us(50);

	
			ads1292DataReceived =  false;
	
	
		}
#endif	

#if 0// processing
		if(ads1292DataReceived == true)
		{
			
			packet_counter++; //if(Pkt_Counter > 0) Pkt_Counter = 0x00;
			
			DataPacket[0] = 0x0A;  // sync0
			DataPacket[1] = 0xFA;
			DataPacket[2] = (uint8_t) (8);
			DataPacket[3] = (uint8_t) (8>>8);
			DataPacket[4] = 0x02;

			DataPacket[9] = (unsigned char) (sresultTemp);
			DataPacket[10] = (unsigned char) (sresultTemp>>8); 
			DataPacket[11] = (unsigned char) (sresultTemp >> 16);
			DataPacket[12] = (unsigned char) (sresultTemp >>24);
			
			DataPacket[5] = (unsigned char) (sresultTempResp);
			DataPacket[6] = (unsigned char) (sresultTempResp>>8);
			DataPacket[7] = (unsigned char) (sresultTempResp >> 16);
			DataPacket[8] = (unsigned char) (sresultTempResp >>24);
			
			DataPacket[13] = 0x00;
			DataPacket[14] = 0x0b;
			
			//usart_putchar(BLUETOOTH_UART,DataPacket[indx]);
			usart_write_buffer_wait(&usart_instance,DataPacket, 15);
						
			ads1292DataReceived =  false;
			
			
		}
#endif

#if 1
	if(afe4490DataReceived == true)
	{
		packet_counter++; //if(Pkt_Counter > 0) Pkt_Counter = 0x00;
	
		DataPacket[0] = 0x0A;  // sync0
		DataPacket[1] = 0xFA;
		DataPacket[2] = (uint8_t) (data_len);
		DataPacket[3] = (uint8_t) (data_len>>8);
		DataPacket[4] = 0x02;

		DataPacket[5] = (unsigned char) (sresultTemp);
		DataPacket[6] = (unsigned char) (sresultTemp>>8);
		DataPacket[7] = (unsigned char) (sresultTemp >> 16);
		DataPacket[8] = (unsigned char) (sresultTemp >>24);
		
		DataPacket[9] = (unsigned char) (sresultTempResp);
		DataPacket[10] = (unsigned char) (sresultTempResp >> 8);
		DataPacket[11] = (unsigned char) (sresultTempResp >> 16);
		DataPacket[12] = (unsigned char) (sresultTempResp >>24);		

		DataPacket[13] = (unsigned char)  (AFE4490sresultTempIR);
		DataPacket[14] = (unsigned char) (AFE4490sresultTempIR>>8);
		DataPacket[15] = (unsigned char) (AFE4490sresultTempIR >> 16);
		DataPacket[16] = (unsigned char) (AFE4490sresultTempIR >>24);
		
		DataPacket[17] = (unsigned char)  (AFE4490sresultTempRED);
		DataPacket[18] = (unsigned char) (AFE4490sresultTempRED>>8);
		DataPacket[19] = (unsigned char) (AFE4490sresultTempRED >> 16);
		DataPacket[20] = (unsigned char) (AFE4490sresultTempRED >>24);
		
		DataPacket[21] =  0x1C;
		DataPacket[22] =  120;
		DataPacket[23] =  80;
			
		DataPacket[24] = 0x00;
		DataPacket[25] = 0x0b;
	
		//usart_putchar(BLUETOOTH_UART,DataPacket[indx]);
		usart_write_buffer_wait(&usart_instance,DataPacket, 26);
	
	/*	sprintf(result,"%d,",sresultTemp);
		//SPI_RX_Buff[0] = 0x29;
		byte = 0;
		for(int i=0;i<20;i++)  if(result[i] != '\0') {byte++;}
		usart_write_buffer_wait(&usart_instance,result, byte);
		for(int i=0;i<20;i++) result[i] = '\0';
		//usart_write_wait(&usart_instance,pwp_state);*/
	
		afe4490DataReceived =  false;
		
	  }
		
#endif		
/*	if(calibration)
	{
		sprintf(result,"read %d,%d,%d \r\n",getCalibration.Calib_Pt_0,getCalibration.Calib_Pt_1,getCalibration.Calib_Pt_5);
		//SPI_RX_Buff[0] = 0x29;
		usart_write_buffer_wait(&usart_instance,result, 20);
		//usart_write_wait(&usart_instance,pwp_state);
		calibration = 0;
		//pwp_state = 0;
		
	}*/



	
	}
}


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void SendAckData()
{
	DataPacketHeader[0] = 0x0A;
	DataPacketHeader[1] = 0xFA;
	DataPacketHeader[2] = 0x01;
	DataPacketHeader[3] = 0x00;
	DataPacketHeader[4] = 0x01;

	DataPacketHeader[5] = 0x00;

	DataPacketHeader[6] = 0x00;
	DataPacketHeader[7] = 0x0b;
	usart_write_buffer_wait(&usart_instance,DataPacketHeader,8);
	delay_us(1000);
}

void SendNegAckData()
{
	DataPacketHeader[0] = 0x0A;
	DataPacketHeader[1] = 0xFA;
	DataPacketHeader[2] = 0x01;
	DataPacketHeader[3] = 0x00;
	DataPacketHeader[4] = 0x01;

	DataPacketHeader[5] = 0x01;

	DataPacketHeader[6] = 0x00;
	DataPacketHeader[7] = 0x0b;
	usart_write_buffer_wait(&usart_instance,DataPacketHeader,8);
	delay_us(1000);
}

/*void offsetCorrection()
{

	offSet= dryCalib/1000;
	write16(0x0d,0x0000);
	delay_ms(10);
	write16(0x0d,(~((0x0000) | offSet<< 11)+1));
	readChannel()	;
}*/

float readChannel()
{
	
	float totalVolt =0;
	
	for(indx = 0;indx<CALIB_SAMPLE ;)
	{
		if(ads1292DataReceived == true)
		{
			ads1292_rx_Data[0]= (unsigned char)SPI_RX_Buff[1];
			ads1292_rx_Data[1]= (unsigned char)SPI_RX_Buff[2];
			ads1292_rx_Data[2]= (unsigned char)SPI_RX_Buff[3];
			ads1292_rx_Data[3]= (unsigned char)SPI_RX_Buff[4];
			
			uads1292temp = (signed long) (((unsigned long)ads1292_rx_Data[0]<<24)|((unsigned long)ads1292_rx_Data[1]<<16)|(ads1292_rx_Data[2]<<8)|ads1292_rx_Data[3]);//
			sads1292temp = (signed long) (uads1292temp);
			// adc_count = (float)2.5/2147483648;//4294967295;
			volt      = (0.0000000011641)*(float)sads1292temp;
			totalVolt += volt;
			indx++;
		}
		
	}
	totalVolt = totalVolt/CALIB_SAMPLE;
	return totalVolt;
}

