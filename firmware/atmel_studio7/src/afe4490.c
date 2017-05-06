
#include <asf.h>
#include "afe4490.h"


volatile uint8_t	AFE4490_SPI_TX_Buff[10];

volatile uint8_t	AFE4490_SPI_RX_Buff[3]={0,0,0};



volatile uint8_t AFE4490_SPI_Dummy_Buff[28];
volatile uint8_t AFE4490_SPI_Read_Reg[3];



volatile uint32_t AFE4490resultTemp = 0;
volatile S32 AFE4490sresultTempIR,AFE4490sresultTempRED;
volatile float AFE4490fresultTemp = 0;
volatile S32 AFE4490iresultTemp = 0;
volatile S32 AFE4490voltageTemp = 0;
volatile bool afe4490DataReceived = false;

void afe4490_configure_spi_master_callbacks(void);
void afe4490_configure_spi_master(void);

void afe4490_configure_extint_channel(void);
void afe4490_configure_extint_callbacks(void);
void afe4490_extint_detection_callback(void);
void AFE4490Init (void);
void AFE4490Write (uint8_t address, uint32_t data);

struct spi_module afe4490spi_master_instance;
struct spi_slave_inst AFE4490slave;

uint8_t afe4490data[1] = {0x00};


void afe4490_extint_detection_callback(void)
{
	 //   extint_chan_disable_callback(DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
	 
	 spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, true);
	 
	 AFE4490Write(CONTROL0,0x000001);  
	
	 AFE4490_SPI_TX_Buff[0] =  0x2C;

	 spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, true);
	
	 spi_write_buffer_wait(&afe4490spi_master_instance, AFE4490_SPI_TX_Buff,1);
	
	 spi_read_buffer_wait(&afe4490spi_master_instance,AFE4490_SPI_RX_Buff,3,AFE4490_SPI_Read_Reg);  

	 spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, false);
	 
	 AFE4490resultTemp = (uint32_t)( ((0x00 << 24) | AFE4490_SPI_RX_Buff[0] << 16)| AFE4490_SPI_RX_Buff[1] << 8 | AFE4490_SPI_RX_Buff[2]);
     AFE4490resultTemp = (uint32_t)(AFE4490resultTemp << 10);
		
	 AFE4490sresultTempIR = (S32)(AFE4490resultTemp );
	 AFE4490sresultTempIR = (AFE4490sresultTempIR >> 10);// * 100;
	 
	 AFE4490_SPI_TX_Buff[0] =  0x2A;

	 spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, true);
	 
	 spi_write_buffer_wait(&afe4490spi_master_instance, AFE4490_SPI_TX_Buff,1);
	 
	 spi_read_buffer_wait(&afe4490spi_master_instance,AFE4490_SPI_RX_Buff,3,AFE4490_SPI_Read_Reg);

	 spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, false);	 
	 
	 AFE4490resultTemp = (uint32_t)( ((0x00 << 24) | AFE4490_SPI_RX_Buff[0] << 16)| AFE4490_SPI_RX_Buff[1] << 8 | AFE4490_SPI_RX_Buff[2]);
	 AFE4490resultTemp = (uint32_t)(AFE4490resultTemp << 10);
	 
	 AFE4490sresultTempRED = (S32)(AFE4490resultTemp );
	 AFE4490sresultTempRED = (AFE4490sresultTempRED >> 10);// * 100;	 
		
	afe4490DataReceived = true;
	
}


void AFE4490_Init_Resources(void)
{
	struct spi_config config_spi_master;
	struct spi_slave_inst_config slave_dev_config;

	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = AFE4490_CS;
	spi_attach_slave(&AFE4490slave, &slave_dev_config);
	spi_get_config_defaults(&config_spi_master);
	config_spi_master.mux_setting = SPI_SERCOM_MUX_SETTING;
	config_spi_master.pinmux_pad0 = AFE4490_MISO;
	//config_spi_master.pinmux_pad1 = AFE4490_UNUSED;
	config_spi_master.pinmux_pad2 = AFE4490_MOSI;
	config_spi_master.pinmux_pad3 = AFE4490_SCK;
	spi_init(&afe4490spi_master_instance, AFE4490_SPI_MODULE	, &config_spi_master);
	spi_enable(&afe4490spi_master_instance);

}


void AFE4490_Init_DRDY_Interrupt (void)
{		
	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = AFE4490_DRDY_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = AFE4490_DRDY_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_DOWN;
	config_extint_chan.detection_criteria = EXTINT_DETECT_FALLING;
	extint_chan_set_config(AFE4490_DRDY_EIC_LINE, &config_extint_chan);
	//eic_line_set_callback(EIC, AFE4490_DRDY_LINE, AFE4490_Data_Recieve_handler, AFE4490_DRDY_LINE_IRQ, 1);
	//eic_line_enable(EIC, AFE4490_DRDY_LINE);
}

void AFE4490_Enable_DRDY_Interrupt (void)
{	
	extint_register_callback(afe4490_extint_detection_callback,	AFE4490_DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(AFE4490_DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
}

void AFE4490_Disable_DRDY_Interrupt (void)
{
	extint_chan_disable_callback(AFE4490_DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
}


void AFE4490_Init(void)
{
	AFE4490_Init_Resources();	//Init GPIO and SPI
	delay_ms(10);

   // AFE4490_Reset();
	delay_ms(100);

	AFE4490_Init_DRDY_Interrupt();				// SDATAC command
	delay_ms(300);
	
//	AFE4490_Disable_Start();
//	delay_ms(100);
	
	//for(uint16_t i=0;i<65000;i++)
	{
		//AFE4490_Reg_Read(0x00)	;
		AFE4490Init ();
		delay_ms(10000);
	}

	extint_register_callback(afe4490_extint_detection_callback,	AFE4490_DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(AFE4490_DRDY_EIC_LINE,	EXTINT_CALLBACK_TYPE_DETECT);

	//ADS1292_Disable_Start();
	//delay_ms(1000);
}


void AFE4490_Disable_Start(void)
{
	port_pin_set_output_level(AFE4490_START, IOPORT_PIN_LEVEL_LOW);
	delay_ms(20);
}

void AFE4490_Enable_Start(void)
{
	port_pin_set_output_level(AFE4490_START, IOPORT_PIN_LEVEL_HIGH);
	delay_ms(20);
}

void AFE4490_Hard_Stop (void)
{
	port_pin_set_output_level(AFE4490_START, IOPORT_PIN_LEVEL_LOW);
	delay_ms(100);
}






void AFE4490_Reset(void)
{
	port_pin_set_output_level(AFE4490_START, IOPORT_PIN_LEVEL_HIGH);
	delay_ms(20);
	port_pin_set_output_level(AFE4490_START, IOPORT_PIN_LEVEL_LOW);
	delay_ms(20);
	port_pin_set_output_level(AFE4490_START, IOPORT_PIN_LEVEL_HIGH);
	delay_ms(20);		
	
}

uint8_t* AFE4490_Reg_Read(uint8_t Reg_address)
{
	uint16_t temp_byte=0;
	
	AFE4490_SPI_TX_Buff[0] = 0x20;//0b00100000;//Reg_address | RREG;
	AFE4490_SPI_TX_Buff[1] = 0;
	AFE4490_SPI_TX_Buff[2] = 0;

	spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, true);
	
	spi_write_buffer_wait(&afe4490spi_master_instance, AFE4490_SPI_TX_Buff,2);
	
	//spi_read_buffer_wait(&spi_master_instance,SPI_Read_Reg,2,SPI_TX_Buff);

    spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, false);

	return AFE4490_SPI_Read_Reg;
}



void AFE4490_SPI_Command_Data(unsigned char data_in)
{
	afe4490data[0] = data_in;
	spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, true);
	
	spi_write_buffer_wait(&afe4490spi_master_instance, afe4490data, 1);

	spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, false);
}


void AFE4490Init (void)
{ 

  AFE4490Write(CONTROL0,0x000000);

  AFE4490Write(TIAGAIN,0x000000); // CF = 5pF, RF = 500kR
  AFE4490Write(TIA_AMB_GAIN,0x000005);  // Timers ON, average 3 samples 
  AFE4490Write(LEDCNTRL,0x0022020);   
  AFE4490Write(CONTROL2,0x002000);  // LED_RANGE=100mA, LED=50mA 
  AFE4490Write(CONTROL1,0x010707);  // Timers ON, average 3 samples  

  AFE4490Write(PRPCOUNT, 0X001F3F);
  /*AFE4490Write(CONTROL1, 0x000101);
   AFE4490Write(CONTROL2, 0x000000);  
   AFE4490Write(PRPCOUNT, 0X001F3F);*/

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

  delay_ms(1000);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AFE4490Write (uint8_t address, uint32_t data)
{
  
	AFE4490_SPI_TX_Buff[0] =  address;
  	AFE4490_SPI_TX_Buff[1] = ((data >> 16) & 0xFF) ;
  	AFE4490_SPI_TX_Buff[2] = ((data >> 8) & 0xFF);						// Write Single byte
  	AFE4490_SPI_TX_Buff[3] = (data & 0xFF);					// Write Single byte			   // Write Single byte

  	spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, true);
  	spi_write_buffer_wait(&afe4490spi_master_instance,AFE4490_SPI_TX_Buff, 4);
  	spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, false);
}


uint32_t AFE4490Read (uint8_t address)
{
	uint32_t data=0;
AFE4490_SPI_TX_Buff[0] =  address;

	spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, true);
	
	spi_write_buffer_wait(&afe4490spi_master_instance, AFE4490_SPI_TX_Buff,1);
	
	spi_read_buffer_wait(&afe4490spi_master_instance,AFE4490_SPI_Read_Reg,3,AFE4490_SPI_RX_Buff);

	spi_select_slave(&afe4490spi_master_instance, &AFE4490slave, false);

	return data; // return with 24 bits of read data
}


