


#ifndef AFE4490_H_
#define AFE4490_H_


void AFE4490_Init(void);
void AFE4490_Reset(void);
void AFE4490_Init_Resources(void);
void AFE4490_Data_Recieve_handler(void);

uint8_t* AFE4490_Reg_Read(uint8_t Reg_address);
void AFE4490_Reg_Write (unsigned char READ_WRITE_ADDRESS, unsigned char DATA);

void AFE4490_Read_Data(void);

void AFE4490_SPI_Command_Data(unsigned char data_in);
void AFE4490_Init_DRDY_Interrupt (void);

void AFE4490_Enable_DRDY_Interrupt (void);
void AFE4490_Disable_DRDY_Interrupt (void);

void AFE4490_Disable_Start(void);
void AFE4490_Enable_Start(void);
void AFE4490_Hard_Stop (void);
void AFE4490_Start_Data_Conv_Command (void);
void AFE4490_Soft_Stop (void);
void AFE4490_Start_Read_Data_Continuous (void);
void AFE4490_Stop_Read_Data_Continuous (void);

void AFE4490_Self_Calibration(void);

#define CONTROL0    0x00
#define LED2STC     0x01
#define LED2ENDC    0x02
#define LED2LEDSTC    0x03
#define LED2LEDENDC   0x04
#define ALED2STC    0x05
#define ALED2ENDC   0x06
#define LED1STC     0x07
#define LED1ENDC    0x08
#define LED1LEDSTC    0x09
#define LED1LEDENDC   0x0a
#define ALED1STC    0x0b
#define ALED1ENDC   0x0c
#define LED2CONVST    0x0d
#define LED2CONVEND   0x0e
#define ALED2CONVST   0x0f
#define ALED2CONVEND  0x10
#define LED1CONVST    0x11
#define LED1CONVEND   0x12
#define ALED1CONVST   0x13
#define ALED1CONVEND  0x14
#define ADCRSTCNT0    0x15
#define ADCRSTENDCT0  0x16
#define ADCRSTCNT1    0x17
#define ADCRSTENDCT1  0x18
#define ADCRSTCNT2    0x19
#define ADCRSTENDCT2  0x1a
#define ADCRSTCNT3    0x1b
#define ADCRSTENDCT3  0x1c
#define PRPCOUNT    0x1d
#define CONTROL1    0x1e
#define SPARE1      0x1f
#define TIAGAIN     0x20
#define TIA_AMB_GAIN  0x21
#define LEDCNTRL    0x22
#define CONTROL2    0x23
#define SPARE2      0x24
#define SPARE3      0x25
#define SPARE4      0x26
#define SPARE4      0x26
#define RESERVED1   0x27
#define RESERVED2   0x28
#define ALARM     0x29
#define LED2VAL     0x2a
#define ALED2VAL    0x2b
#define LED1VAL     0x2c
#define ALED1VAL    0x2d
#define LED2ABSVAL    0x2e
#define LED1ABSVAL    0x2f
#define DIAG      0x30

#endif /* AFE4490_H_ */
