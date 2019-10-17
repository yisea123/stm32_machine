/*
	Honeywell¶þÎ¬ÂëÉ¨ÃèÇý¶¯¿â
*/

#ifndef __QRCODE_SCAN_LIB_HW_H__
#define __QRCODE_SCAN_LIB_HW_H__

#include "usart.h"

#define HIGH				Bit_SET
#define LOW					Bit_RESET

#define COMM_FAILED			-1
#define COMM_SUCCESSED		1

#define RX_BUFF_SIZE_HW		128
#define TX_BUFF_SIZE_HW		64

typedef enum
{
	unknow_scan=0,
	se955_scan,
	n4313_scan,
	n5680_scan,
	n5180_scan,
	se3307_scan,
	n3680_scan	
}scan_device_type_t;

typedef struct _qrcode_scan_hw_rc_t
{
	t_pin_source*		trigger_pin;
	t_usart_source* 	com;
	scan_device_type_t	scan_device_type;
	unsigned char rx_msg[RX_BUFF_SIZE_HW];
	unsigned char tx_msg[TX_BUFF_SIZE_HW];
	unsigned int msg_length;
}qrcode_scan_hw_rc_t;

int QRCodeScanHw_Install(qrcode_scan_t * qrcode_scan);

#endif

