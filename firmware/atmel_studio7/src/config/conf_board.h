/**
 * \file
 *
 * \brief User board configuration template
 *
 * Copyright (C) 2013-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H


#define AFE4490 1
#define ADS1292 0

#define IOPORT_PIN_LEVEL_LOW 0
#define IOPORT_PIN_LEVEL_HIGH 1

#define ON 1
#define OFF 0


#define EXT1_PIN_3                PIN_PB00
#define EXT1_PIN_4                PIN_PB01
#define EXT1_PIN_5                PIN_PB06
#define EXT1_PIN_6                PIN_PB07
#define EXT1_PIN_7                PIN_PB02
#define EXT1_PIN_8                PIN_PB03
#define EXT1_PIN_9                PIN_PB04
#define EXT1_PIN_10               PIN_PB05
#define EXT1_PIN_11               PIN_PA08
#define EXT1_PIN_12               PIN_PA09
#define EXT1_PIN_13               PIN_PB09

#define EXT1_PIN_15               PIN_PA05
#define EXT1_PIN_16               PIN_PA06
#define EXT1_PIN_17               PIN_PA04
#define EXT1_PIN_18               PIN_PA07

#define EXT1_PIN_ADC_0            EXT1_PIN_3
#define EXT1_PIN_ADC_1            EXT1_PIN_4
#define EXT1_PIN_GPIO_0           EXT1_PIN_5
#define EXT1_PIN_GPIO_1           EXT1_PIN_6
#define EXT1_PIN_PWM_0            EXT1_PIN_7
#define EXT1_PIN_PWM_1            EXT1_PIN_8
#define EXT1_PIN_IRQ              EXT1_PIN_9
#define EXT1_PIN_I2C_SDA          EXT1_PIN_11
#define EXT1_PIN_I2C_SCL          EXT1_PIN_12
#define EXT1_PIN_UART_RX          EXT1_PIN_13
#define EXT1_PIN_UART_TX          EXT1_PIN_14
#define EXT1_PIN_SPI_SS_1         EXT1_PIN_10

#define EXT1_PIN_SPI_MOSI         EXT1_PIN_16
#define EXT1_PIN_SPI_MISO         EXT1_PIN_17
#define EXT1_PIN_SPI_SCK          EXT1_PIN_18

//*****************************LED*************************
#define LED0_PIN                  PIN_PA27
#define LED0_ACTIVE               false
#define LED0_INACTIVE             !LED0_ACTIVE

#define GPLED_0_NAME                "LED0 (yellow)"
#define GPLED_0_PIN                 LED0_PIN
#define GPLED_0_ACTIVE              LED0_ACTIVE
#define GPLED_0_INACTIVE            LED0_INACTIVE
#define GPLED0_GPIO                 LED0_PIN
#define GPLED0                      LED0_PIN

//******************** Buzzer**********************
#define BUZZER                 PIN_PB03



//********************* ADS1292*********************

#define SPI_MODULE					SERCOM2
#define SPI_SERCOM_MUX_SETTING		SPI_SIGNAL_MUX_SETTING_E
#define ADS1292_MISO				PINMUX_PA08D_SERCOM2_PAD0
#define ADS1292_UNUSED				PINMUX_PA09D_SERCOM2_PAD1
#define ADS1292_MOSI				PINMUX_PA10D_SERCOM2_PAD2
#define ADS1292_SCK					PINMUX_PA11D_SERCOM2_PAD3
#define ADS1292_DMAC_ID_TX			SERCOM2_DMAC_ID_TX
#define ADS1292_DMAC_ID_RX			SERCOM2_DMAC_ID_RX
#define ADS1292_CS                PIN_PA09

//******************** DRDY *****************************
#define DRDY_PIN                   PIN_PA05
#define DRDY_ACTIVE                false
#define DRDY_INACTIVE              !DRDY_ACTIVE
#define DRDY_EIC_PIN               PIN_PA05A_EIC_EXTINT5
#define DRDY_EIC_MUX               MUX_PA05A_EIC_EXTINT5
#define DRDY_EIC_PINMUX            PINMUX_PA05A_EIC_EXTINT5
#define DRDY_EIC_LINE              05

#define ADS1292_DRDYNAME             "DRDY"
#define ADS1292_DRDY_PIN              DRDY_PIN
#define ADS1292_DRDY_ACTIVE           DRDY_ACTIVE
#define ADS1292_DRDY_INACTIVE         DRDY_INACTIVE
#define ADS1292_DRDY_EIC_PIN          DRDY_EIC_PIN
#define ADS1292_DRDY_EIC_MUX          DRDY_EIC_MUX
#define ADS1292_DRDY_EIC_PINMUX       DRDY_EIC_PINMUX
#define ADS1292_DRDY_EIC_LINE         DRDY_EIC_LINE


#define ADS1292_START_PIN	PIN_PA01
#define ADS1292_PWDN_PIN	PIN_PA00

#define LED0_PIN			PIN_PA27

//AFE4490 Interface lines
#define AFE4490_SPI_MODULE					SERCOM0
#define SPI_SERCOM_MUX_SETTING		SPI_SIGNAL_MUX_SETTING_E
#define AFE4490_MISO				PINMUX_PA04D_SERCOM0_PAD0
#define AFE4490_UNUSED				PINMUX_PA05D_SERCOM0_PAD1
#define AFE4490_MOSI				PINMUX_PA06D_SERCOM0_PAD2
#define AFE4490_SCK					PINMUX_PA07D_SERCOM0_PAD3
#define AFE4490_DMAC_ID_TX			SERCOM0_DMAC_ID_TX
#define AFE4490_DMAC_ID_RX			SERCOM0_DMAC_ID_RX
#define AFE4490_CS					PIN_PB08

#define AFE4490_START				PIN_PB09

//******************** AFE4490 DRDY ****************
#define AFE4490_DRDY_PIN                   PIN_PA02
#define AFE4490_DRDY_ACTIVE                false
#define AFE4490_DRDY_INACTIVE              !DRDY_ACTIVE
#define AFE4490_DRDY_EIC_PIN               PIN_PA02A_EIC_EXTINT2
#define AFE4490_DRDY_EIC_MUX               MUX_PA02A_EIC_EXTINT2
#define AFE4490_DRDY_EIC_PINMUX            PINMUX_PA02A_EIC_EXTINT2
#define AFE4490_DRDY_EIC_LINE              02

//*********** UART*****************
/*#define EDBG_CDC_MODULE              SERCOM1
#define EDBG_CDC_SERCOM_MUX_SETTING  USART_RX_1_TX_0_XCK_1
#define EDBG_CDC_SERCOM_PINMUX_PAD0  PINMUX_PA16C_SERCOM1_PAD0
#define EDBG_CDC_SERCOM_PINMUX_PAD1  PINMUX_PA17C_SERCOM1_PAD1
#define EDBG_CDC_SERCOM_PINMUX_PAD2  PINMUX_UNUSED
#define EDBG_CDC_SERCOM_PINMUX_PAD3  PINMUX_UNUSED*/

//*********** UART testin*****************
#define EDBG_CDC_MODULE              SERCOM4
#define EDBG_CDC_SERCOM_MUX_SETTING  USART_RX_1_TX_0_XCK_1
#define EDBG_CDC_SERCOM_PINMUX_PAD0  PINMUX_PA12D_SERCOM4_PAD0
#define EDBG_CDC_SERCOM_PINMUX_PAD1  PINMUX_PA13D_SERCOM4_PAD1
#define EDBG_CDC_SERCOM_PINMUX_PAD2  PINMUX_UNUSED
#define EDBG_CDC_SERCOM_PINMUX_PAD3  PINMUX_UNUSED

struct ICPAlarm
{
	volatile U32  AlarmMinVal;
	volatile U32  AlarmMaxVal;
	volatile bool  AlaramMode;
	volatile bool AlarmFlag;
	
	volatile bool AlarmSetflag;
	volatile bool AlarmStatus;
};

struct ICPCalibrationPara
{
	volatile U32 Calib_Pt_0;
	volatile U32 Calib_Pt_1;
	volatile U32 Calib_Pt_5;
	volatile U32 Calib_Pt_10;
	volatile U32 Calib_Pt_40;
	volatile U32 Calib_Pt_100;
	
	volatile bool requestData;
	volatile bool writedata;
};

struct ICPFactoryRST
{
	volatile bool resetflag;
	volatile bool resetVal;
};

struct ICPOffset
{
	volatile bool offsetflag;
	
};

struct CalibratedPara
{
	U32 	Gb0calibration   ;
	U32     Gb1calibration  ;
	U32     Gb5calibration   ;
	U32     Gb10calibration  ;
	U32     Gb40calibration  ;
	U32		Gb100calibration ;
	
	bool AlarmMode;
	U32 AlarmMax ;
	U32 AlarmMin ;
	 	
	bool PosCalib;
	bool NegCalib;
	
	
};

struct ICPcmdmode
{
	bool stop ;
	uint8_t mode;
	bool start;
	bool startgui;
}	;

#endif // CONF_BOARD_H
