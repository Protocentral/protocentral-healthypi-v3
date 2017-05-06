


#ifndef ADS1292_H_
#define ADS1292_H_


#define REFERENCE_POINT_LOW   55 //70
#define REFERENCE_POINT_HIGH  50  //60
#define OFFSET_CALCU           0
#define DUMMY			1
#define ADS1292_GAIN_1			0
#define ADS1292_GAIN_32			0x50

void ADS1292_Init(void);
void ADS1292_Reset(void);
void ADS1292_Init_Resources(void);
void ADS1292_Data_Recieve_handler(void);

uint8_t* ADS1292_Reg_Read(uint8_t Reg_address);
void ADS1292_Reg_Write (unsigned char READ_WRITE_ADDRESS, unsigned char DATA);

void ADS1292_Read_Data(void);

void ADS1292_SPI_Command_Data(unsigned char data_in);
void ADS1292_Init_DRDY_Interrupt (void);

void ADS1292_Enable_DRDY_Interrupt (void);
void ADS1292_Disable_DRDY_Interrupt (void);

void ADS1292_Disable_Start(void);
void ADS1292_Enable_Start(void);
void ADS1292_Hard_Stop (void);
void ADS1292_Start_Data_Conv_Command (void);
void ADS1292_Soft_Stop (void);
void ADS1292_Start_Read_Data_Continuous (void);
void ADS1292_Stop_Read_Data_Continuous (void);

void ecs_switch_range(int value);
void sendIVData(S32 s_current, S32 s_voltage);
void sendIVPeakData(S32 s_current_peak, S32 s_voltage_peak);
void SendPeakDataPacket(U8* data, U16 data_len);
void SendDataPacket(U8* data, U16 data_len);
void ADS1292_Self_Calibration(void);
//void sendIVData(U8 temp1, U8 temp2, U8 temp3, U8 temp4,float s_voltage);
//static volatile bool ecs_switch_enable_flag = 0;


// Register Read Commands
#define RREG		0x20		//Read n nnnn registers starting at address r rrrr
//first byte 001r rrrr (2xh)(2) - second byte 000n nnnn(2)
#define WREG		0x40		//Write n nnnn registers starting at address r rrrr
//first byte 010r rrrr (2xh)(2) - second byte 000n nnnn(2)

// System Commands
#define WAKEUP		0x02		//Wake-up from standby mode
#define STANDBY	0x04		//Enter standby mode
#define RESET		0x06		//Reset the device
#define START		0x08		//Start/restart (synchronize) conversions
#define STOP		0x0A		//Stop conversion

// Data Read Commands
#define RDATAC		0x10		//Enable Read Data Continuous mode.
//This mode is the default mode at power-up.
#define SDATAC		0x11		//Stop Read Data Continuously mode
#define RDATA		0x12		//Read data by command; supports multiple read back.

#define ADS1292_REG_ID			0x00
#define ADS1292_REG_CONFIG1		0x01
#define ADS1292_REG_CONFIG2		0x02
#define ADS1292_REG_LOFF		0x03
#define ADS1292_REG_CH1SET		0x04
#define ADS1292_REG_CH2SET		0x05
#define ADS1292_REG_RLDSENS		0x06
#define ADS1292_REG_LOFFSENS	0x07
#define ADS1292_REG_LOFFSTAT	0x08


#endif /* ADS1292_H_ */
