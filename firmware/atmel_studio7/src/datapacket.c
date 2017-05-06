#include <datapacket.h>
#include <asf.h>
#include <stdio.h>
static uint8_t	pwp_rx_state = CESState_Init;

static volatile uint16_t	CES_Pkt_Len = 0;		//Data Payload length
static volatile uint16_t	CES_Pkt_EndSeq;
uint8_t		CES_Pkt_PktType = 0;
uint16_t	CES_Pkt_DestAddr = 0;
uint8_t		CES_Pkt_LQI;

volatile uint16_t	CES_Pkt_Pos_Counter  = 0;
volatile uint16_t	CES_Pkt_Data_Counter = 0;
volatile    uint8_t  pwp_state   = 0;
volatile    uint8_t  calibration = 0;
volatile    uint8_t startCalibration = 0;

volatile	uint8_t		DataRcvPacket[125];
volatile unsigned char packet = 0;
 void pwpProcessData(uint8_t *fdcRxch);
 void icpParsePacket(void);
 
 struct ICPCalibrationPara ICPgetCalibration;
 struct ICPAlarm ICPAlaramSettings;
 struct ICPFactoryRST ICPFactoryReset ;
 struct ICPOffset  ICPoffsetSetting;
 struct ICPcmdmode  ICPCmd;


void pwpProcessData(uint8_t *fdcRxch)
{
	
	for(volatile uint8_t q=0;q<PACKET_SIZE;q++)
	{
		
		int rxch = *(fdcRxch+q);
		switch(pwp_rx_state)
		{
			
			case CESState_Init:	if(rxch==CES_CMDIF_PKT_START_1)
									{
										pwp_rx_state = CESState_SOF1_Found;
					
									}
									break;
								
			case CESState_SOF1_Found: if(rxch==CES_CMDIF_PKT_START_2)
									{
										pwp_rx_state=CESState_SOF2_Found;
									}
									else
									{
										//Invalid Packet, reset state to init
										pwp_rx_state=CESState_Init;
									}
									break;
									
			case CESState_SOF2_Found:pwp_rx_state = CESState_PktLen_Found;
									 CES_Pkt_Len = (U8) rxch;
									 CES_Pkt_Data_Counter = 0;
									 CES_Pkt_Pos_Counter = CES_CMDIF_IND_LEN;
									 
									 //pwp_state=0;
									 
									 break;
									 
			case CESState_PktLen_Found:CES_Pkt_Pos_Counter++;
									 if(CES_Pkt_Pos_Counter<(CES_CMDIF_PKT_OVERHEAD))  //Read Header
									 {
										 if(CES_Pkt_Pos_Counter==CES_CMDIF_IND_LEN_MSB)
										 {
										   CES_Pkt_Len = (U16) ((rxch<<8)|CES_Pkt_Len);
										 }
										 else
										 {
					 			         }
										 if(CES_Pkt_Pos_Counter==CES_CMDIF_IND_PKTTYPE)
										 {
											DataRcvPacket[CES_Pkt_Data_Counter++]= (U8) rxch;
										}
									}
									else if((CES_Pkt_Pos_Counter>=(CES_CMDIF_PKT_OVERHEAD))&&(CES_Pkt_Pos_Counter<(CES_CMDIF_PKT_OVERHEAD+CES_Pkt_Len+1)))  //Read Data
									{
										DataRcvPacket[CES_Pkt_Data_Counter]= (U8) rxch;
										//pwp_state = (uint8_t) DataRcvPacket[0];
										//pwp_state++;
										CES_Pkt_Data_Counter++;
										
									}
									else	//All header and data received
									{
										//Check for EOF Byte
										if(rxch==CES_CMDIF_PKT_STOP)
										{
										//Complete valid packet received
					
										//udi_cdc_putc(5);
										//ecsParsePacket();
					
											
											icpParsePacket();
											if(!calibration)calibration = 1;
											pwp_rx_state=CESState_Init;
										    startCalibration = 1;

									    }
									    else
									    {
										//Packet end not found, drop packet
											pwp_rx_state=CESState_Init;
									    }
								    }
									break;
									
									default: break;
					     //Invalid state
						//TODO: Error handling for undefined state in packet parser
							
		}
	}
	
}





void icpParsePacket(void)
{

	uint8_t b[4];

	
	switch(DataRcvPacket[0])
	{
		case ICP_START_CMD :ICPgetCalibration.requestData=true;
			
							break;
		
		case ICP_START_DATA:
						b[3] = DataRcvPacket[1];
						b[2] = DataRcvPacket[2];
						b[1] = DataRcvPacket[3];
						b[0] = DataRcvPacket[4];
			           
						ICPgetCalibration.Calib_Pt_0= ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]));
			
						//ecs_current_state.startPotential = (float)(ecs_current_state.startPotential / 1000.00 );
			
						b[3] = DataRcvPacket[5];
						b[2] = DataRcvPacket[6];
						b[1] = DataRcvPacket[7];
						b[0] = DataRcvPacket[8];
			
						ICPgetCalibration.Calib_Pt_1 = ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]));
			
						//ecs_current_state.vertexPotential = (float)(ecs_current_state.vertexPotential / 1000.00 );
			
						b[3] = DataRcvPacket[9];
						b[2] = DataRcvPacket[10];
						b[1] = DataRcvPacket[11];
						b[0] = DataRcvPacket[12];
			
						ICPgetCalibration.Calib_Pt_5 = ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]));
			
						//ecs_current_state.endPotential = (float)(ecs_current_state.endPotential / 1000.00 );

						b[3] = DataRcvPacket[13];
						b[2] = DataRcvPacket[14];
						b[1] = DataRcvPacket[15];
						b[0] = DataRcvPacket[16];
			
						ICPgetCalibration.Calib_Pt_10 =  ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]));
			
						b[3] = DataRcvPacket[17];
						b[2] = DataRcvPacket[18];
						b[1] = DataRcvPacket[19];
						b[0] = DataRcvPacket[20];
			
						ICPgetCalibration.Calib_Pt_40 =  ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]));
			
						b[3] = DataRcvPacket[21];
						b[2] = DataRcvPacket[22];
						b[1] = DataRcvPacket[23];
						b[0] = DataRcvPacket[24];
			
						ICPgetCalibration.Calib_Pt_100 =  ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]));		
						
						ICPgetCalibration.writedata = true; 		
						break;
		
case ICP_ALARM_SETTING :ICPAlaramSettings.AlaramMode =  DataRcvPacket[1];
						ICPAlaramSettings.AlarmFlag  = true;
						b[3] = DataRcvPacket[1];
						b[2] = DataRcvPacket[2];
						b[1] = DataRcvPacket[3];
						b[0] = DataRcvPacket[4];
						ICPAlaramSettings.AlarmMaxVal = ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]));	
								
						b[3] = DataRcvPacket[5];
						b[2] = DataRcvPacket[6];
						b[1] = DataRcvPacket[7];
						b[0] = DataRcvPacket[8];							
						ICPAlaramSettings.AlarmMinVal = ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3]));	
						break;
							
case ICP_FACTORY_RESET: ICPFactoryReset.resetflag = true;
						ICPFactoryReset.resetVal =  DataRcvPacket[1];
						break;
							
case ICP_OFFSET_SETTING: ICPoffsetSetting.offsetflag = true;
						break;		
						
case ICP_ALARM_ON	  : ICPAlaramSettings.AlarmSetflag = true;
                        ICPAlaramSettings.AlarmStatus = DataRcvPacket[1];
						break;	
						
case ICP_STOP         : ICPCmd.stop  = true;
						ICPCmd.mode = DataRcvPacket[1];
						break;	
					
case ICP_START       :  ICPCmd.start  = true; 																				
							break;
							
case ICP_STARTGUI   : ICPCmd.startgui = true;
						break;							
						
		default:			break;
	}
}




