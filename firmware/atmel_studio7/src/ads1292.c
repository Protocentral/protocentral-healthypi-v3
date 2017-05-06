
#include <asf.h>
#include "ads1292.h"


volatile uint8_t	SPI_TX_Buff[10];

volatile uint8_t	SPI_RX_Buff[100];

uint8_t tdata = 0;
int x = 0;
volatile unsigned int amp_temp = 0;

volatile uint8_t SPI_Dummy_Buff[28];
volatile uint8_t SPI_Read_Reg[1];

volatile uint8_t acc_read = 1;
volatile U8		acc_read_flag = 0;
volatile uint8_t eeg_pkt_ready = false;

volatile uint32_t resultTemp = 0;
volatile S32 sresultTemp,sresultTempResp ;
volatile float fresultTemp = 0;
volatile S32 iresultTemp = 0;
volatile S32 voltageTemp = 0;
volatile bool ads1292DataReceived = false;

void configure_spi_master_callbacks(void);
void configure_spi_master(void);

void configure_extint_channel(void);
void configure_extint_callbacks(void);
void ads1292_detection_callback(void);

#define BUF_LENGTH 9
#define ADS1292_CS_PIN		ADS1292_CS
static uint8_t wr_buffer[BUF_LENGTH] = {
	0x00,0xff,0xff,0xff,0xff,0xff
};
static uint8_t rd_buffer[BUF_LENGTH];
struct spi_module spi_master_instance;
struct spi_slave_inst slave;

uint8_t data[1] = {0x00};
volatile uint16_t ledToggleCount=0;
volatile bool 	ledTogglrFlag =false;


void ads1292_detection_callback(void)
{
	 //   extint_chan_disable_callback(DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
	// port_pin_toggle_output_level(GPLED_0_PIN);
	 
	 spi_select_slave(&spi_master_instance, &slave, true);
	 
	 spi_read_buffer_wait(&spi_master_instance,SPI_Dummy_Buff,BUF_LENGTH,wr_buffer);
	
	 //spi_transceive_buffer_job(&spi_master_instance, wr_buffer,rd_buffer,BUF_LENGTH);
	
	 //	extint_chan_enable_callback(DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);

	  for(x = 3; x < 9; x++)
	  {
		SPI_RX_Buff[x-3] = SPI_Dummy_Buff[x];
	 	
	  }
	
		spi_select_slave(&spi_master_instance, &slave, false);

		resultTemp = (uint32_t)((0x00 << 24) | (SPI_RX_Buff[3] << 16)| SPI_RX_Buff[4] << 8 | SPI_RX_Buff[5]);//6,7,8
		resultTemp = (uint32_t)(resultTemp << 8);

		sresultTemp = (S32)(resultTemp);
		sresultTemp = (sresultTemp >> 8);		//	resultTemp = (uint32_t)(resultTemp << 8);
		

		resultTemp = (uint32_t)((0x00 << 24) | (SPI_RX_Buff[0] << 16)| SPI_RX_Buff[1] << 8 | SPI_RX_Buff[2]);//6,7,8
		resultTemp = (uint32_t)(resultTemp << 8);

		sresultTempResp = (S32)(resultTemp);
		sresultTempResp = (sresultTempResp >> 8);		//	resultTemp = (uint32_t)(resultTemp << 8);
	
		
		ads1292DataReceived = true;
			

			
}


void ADS1292_Init_Resources(void)
{
		struct spi_config config_spi_master;
		struct spi_slave_inst_config slave_dev_config;

		spi_slave_inst_get_config_defaults(&slave_dev_config);
		slave_dev_config.ss_pin = ADS1292_CS_PIN;
		spi_attach_slave(&slave, &slave_dev_config);
		spi_get_config_defaults(&config_spi_master);
		config_spi_master.mux_setting = SPI_SERCOM_MUX_SETTING;
		config_spi_master.transfer_mode = SPI_TRANSFER_MODE_1;
		config_spi_master.pinmux_pad0 = ADS1292_MISO;
		//config_spi_master.pinmux_pad1 = ADS1292_UNUSED;
		config_spi_master.pinmux_pad2 = ADS1292_MOSI;
		config_spi_master.pinmux_pad3 = ADS1292_SCK;
		spi_init(&spi_master_instance, SPI_MODULE, &config_spi_master);
		spi_enable(&spi_master_instance);

}


void ADS1292_Init_DRDY_Interrupt (void)
{		
	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = DRDY_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = DRDY_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_FALLING;
	extint_chan_set_config(DRDY_EIC_LINE, &config_extint_chan);
	//eic_line_set_callback(EIC, ADS1292_DRDY_LINE, ADS1292_Data_Recieve_handler, ADS1292_DRDY_LINE_IRQ, 1);
	//eic_line_enable(EIC, ADS1292_DRDY_LINE);
}

void ADS1292_Enable_DRDY_Interrupt (void)
{	
	extint_register_callback(ads1292_detection_callback,	DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
}

void ADS1292_Disable_DRDY_Interrupt (void)
{
	extint_chan_disable_callback(DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
}


void ADS1292_Init(void)
{
	ADS1292_Init_Resources();	//Init GPIO and SPI
	delay_ms(10);

    ADS1292_Reset();
	delay_ms(100);

	ADS1292_Init_DRDY_Interrupt();				// SDATAC command
	delay_ms(300);
	
	ADS1292_Disable_Start();
	delay_ms(100);
	
	ADS1292_Enable_Start();
	delay_ms(10);
		
/*	for(uint16_t i=0;i<65000;i++)
	{
		ADS1292_Reg_Read(0x00)	;
		delay_ms(100);
	}
*/	


	ADS1292_Hard_Stop();
	ADS1292_Start_Data_Conv_Command();
	ADS1292_Soft_Stop();	
			
	ADS1292_Reg_Write(0x01,0x00); 		//Set sampling rate to 125 SPS
	delay_ms(10);
	ADS1292_Reg_Write(0x01,0x00); 		//Set sampling rate to 125 SPS
	delay_ms(10);
	ADS1292_Reg_Write(0x01,0x00); 		//Set sampling rate to 125 SPS
	delay_ms(10);
	
	ADS1292_Reg_Write(0x02,0b10100000);	//Lead-off comp off, test signal disabled
	delay_ms(10);
	ADS1292_Reg_Write(0x03,0b00010000);	//Lead-off defaults
	delay_ms(10);
	ADS1292_Reg_Write(0x04,0b01000000);		//Ch 1 enabled, gain 6, connected to electrode in
	delay_ms(10);
	ADS1292_Reg_Write(0x05,0b01100000);		//Ch 2 enabled, gain 6, connected to electrode in
	delay_ms(10);
	ADS1292_Reg_Write(0x06,0b00101100);	//RLD settings: fmod/16, RLD enabled, RLD inputs from Ch2 only
	delay_ms(10);
	ADS1292_Reg_Write(0x07,0x00);		//LOFF settings: all disabled
	delay_ms(10);
	//Skip register 8, LOFF Settings default
	
	ADS1292_Reg_Write(0x09,0b11110010);	//Respiration: MOD/DEMOD turned only, phase 0
	delay_ms(10);
	ADS1292_Reg_Write(0x0A,0b00000011);	//Respiration: Calib OFF, respiration freq defaults
	delay_ms(10);

	ADS1292_Enable_Start();
	
	ADS1292_Start_Read_Data_Continuous();
	
	//ADS1292_Enable_DRDY_Interrupt();
	extint_register_callback(ads1292_detection_callback,	DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);

	//ADS1292_Disable_Start();
	//delay_ms(1000);
}


void ADS1292_Disable_Start(void)
{
	port_pin_set_output_level(ADS1292_START_PIN, IOPORT_PIN_LEVEL_LOW);
	delay_ms(20);
}

void ADS1292_Enable_Start(void)
{
	port_pin_set_output_level(ADS1292_START_PIN, IOPORT_PIN_LEVEL_HIGH);
	delay_ms(20);
}

void ADS1292_Hard_Stop (void)
{
	port_pin_set_output_level(ADS1292_START_PIN, IOPORT_PIN_LEVEL_LOW);
	delay_ms(100);
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
	ADS1292_SPI_Command_Data(RDATAC);					// Send 0x10 to the ADS1x9x
}

void ADS1292_Stop_Read_Data_Continuous (void)
{
	ADS1292_SPI_Command_Data(SDATAC);					// Send 0x11 to the ADS1x9x
}


void ADS1292_Reset(void)
{
	port_pin_set_output_level(ADS1292_PWDN_PIN, IOPORT_PIN_LEVEL_HIGH);
	delay_ms(20);
	port_pin_set_output_level(ADS1292_PWDN_PIN, IOPORT_PIN_LEVEL_LOW);
	delay_ms(20);
	port_pin_set_output_level(ADS1292_PWDN_PIN, IOPORT_PIN_LEVEL_HIGH);
	delay_ms(20);		
	ADS1292_SPI_Command_Data(RESET);	
	delay_ms(100);
}

uint8_t* ADS1292_Reg_Read(uint8_t Reg_address)
{
	uint16_t temp_byte=0;
	
	SPI_TX_Buff[0] = 0b00100000;//Reg_address | RREG;
	SPI_TX_Buff[1] = 0;
	SPI_TX_Buff[2] = 0;

	spi_select_slave(&spi_master_instance, &slave, true);
	
	//spi_write_buffer_wait(&spi_master_instance, SPI_TX_Buff,2);
	
	spi_read_buffer_wait(&spi_master_instance,SPI_Read_Reg,2,SPI_TX_Buff);

    spi_select_slave(&spi_master_instance, &slave, false);

	return SPI_Read_Reg;
}



void ADS1292_SPI_Command_Data(unsigned char data_in)
{
	data[0] = data_in;
	spi_select_slave(&spi_master_instance, &slave, true);
	
	spi_write_buffer_wait(&spi_master_instance, data, 1);

	spi_select_slave(&spi_master_instance, &slave, false);
}



void ADS1292_Reg_Write (unsigned char READ_WRITE_ADDRESS, unsigned char DATA)
{
	
	switch (READ_WRITE_ADDRESS)
	{
	
		case 1:	DATA = DATA & 0x87;
				break;
				
		case 2:	DATA = DATA & 0xFB;
				DATA |= 0x80;
				break;
				
		case 3: DATA = DATA & 0xFD;
				DATA |= 0x10;
				break;
				
		case 7:	DATA = DATA & 0x3F;
				break;
				
		case 8:	DATA = DATA & 0x5F;
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
	SPI_TX_Buff[1] = 0;						// Write Single byte
	SPI_TX_Buff[2] = DATA;					// Write Single byte			   // Write Single byte

	spi_select_slave(&spi_master_instance, &slave, true);

	spi_write_buffer_wait(&spi_master_instance,SPI_TX_Buff, 3);

	spi_select_slave(&spi_master_instance, &slave, false);
}

