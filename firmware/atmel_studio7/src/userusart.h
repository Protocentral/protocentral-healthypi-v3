/*
 * USART.h
 *
 * Created: 5/19/2016 3:41:19 PM
 *  Author: venkatesh
 */ 


#ifndef USERUSART_H_
#define USERUSART_H_

#define MAX_RX_BUFFER_LENGTH   31
#define CESState_Init			0
#define CESState_SOF1_Found		1
#define CESState_SOF2_Found		2
#define	CESState_PktLen_Found	3

#define CESState_EOF_Wait		98
#define	CESState_EOF_Found		99

/*CES CMD IF Packet Format*/
#define	CES_CMDIF_PKT_START_1           0x0A
#define	CES_CMDIF_PKT_START_2           0xFA
#define	CES_CMDIF_PKT_STOP				0x0B

/*CES CMD IF Packet Indices*/
#define	CES_CMDIF_IND_START_1			0
#define	CES_CMDIF_IND_START_2			1
#define	CES_CMDIF_IND_LEN				2
#define	CES_CMDIF_IND_LEN_MSB			3
#define	CES_CMDIF_IND_PKTTYPE			4
#define	CES_CMDIF_IND_DATA0				5
#define	CES_CMDIF_IND_DATA1				6
#define	CES_CMDIF_IND_DATA2				7

/* CES OTA Data Packet Positions */
#define	CES_OTA_DATA_PKT_POS_LENGTH	    0
#define CES_OTA_DATA_PKT_POS_CMD_CAT 	1
#define	CES_OTA_DATA_PKT_POS_DATA_TYPE	2
#define	CES_OTA_DATA_PKT_POS_SENS_TYPE	3
#define	CES_OTA_DATA_PKT_POS_RSVD	    4
#define	CES_OTA_DATA_PKT_POS_SENS_ID	5
#define	CES_OTA_DATA_PKT_POS_DATA	    6

#define	CES_OTA_DATA_PKT_OVERHEAD		6

#define	CES_CMDIF_PKT_OVERHEAD			5

volatile uint8_t rx_buffer[MAX_RX_BUFFER_LENGTH];

struct usart_module usart_instance;

void sercom_usart_init();



#endif /* USART_H_ */