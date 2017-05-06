/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

#if defined(__GNUC__)
void board_init(void) WEAK __attribute__((alias("system_board_init")));
#elif defined(__ICCARM__)
void board_init(void);
#  pragma weak board_init=system_board_init
#endif

void system_board_init(void)
{
		struct port_config pin_conf;

			/* Set buttons as inputs */
		pin_conf.direction  = PORT_PIN_DIR_INPUT;
		pin_conf.input_pull = PORT_PIN_PULL_UP;
		port_pin_set_config(DRDY_PIN, &pin_conf);

		
		pin_conf.direction  = PORT_PIN_DIR_INPUT;
		pin_conf.input_pull = PORT_PIN_PULL_UP;
		port_pin_set_config(AFE4490_DRDY_PIN , &pin_conf);		
		
#if 1
		pin_conf.direction = PORT_PIN_DIR_OUTPUT;
		port_pin_set_config(ADS1292_PWDN_PIN, &pin_conf);	
		port_pin_set_output_level(ADS1292_PWDN_PIN,0)	;
		delay_ms(100);
		port_pin_set_output_level(ADS1292_PWDN_PIN,1)	;
		
		pin_conf.direction = PORT_PIN_DIR_OUTPUT;
		port_pin_set_config(ADS1292_START_PIN, &pin_conf);		
		port_pin_set_output_level(ADS1292_START_PIN,1)	;
#endif

		pin_conf.direction = PORT_PIN_DIR_OUTPUT;
		port_pin_set_config(AFE4490_START, &pin_conf);
		port_pin_set_output_level(AFE4490_START,1)	;
	
		pin_conf.direction = PORT_PIN_DIR_OUTPUT;
		port_pin_set_config(AFE4490_CS, &pin_conf);
		port_pin_set_output_level(AFE4490_CS,1)	;	
		
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
//-------------LED	-----------------------
    PORT->Group[0].DIRSET.bit.DIRSET = (1 << 27);
    port_pin_set_output_level(LED0_PIN, 0);
	
//-------------LED	-----------------------
	
//-------------Buzzer-----------------------	
	PORT->Group[1].DIRSET.bit.DIRSET = (1 << 3);
	port_pin_set_output_level(BUZZER, 0);
	
	PORT->Group[0].DIRSET.bit.DIRSET = (1 << 9);
	port_pin_set_output_level(PIN_PA09, 1);
	
	PORT->Group[1].DIRSET.bit.DIRSET = (1 << 9);
	port_pin_set_output_level(PIN_PB09, 1);
		
	PORT->Group[1].DIRSET.bit.DIRSET = (1 << 8);
	port_pin_set_output_level(PIN_PB08, 1);		
}

