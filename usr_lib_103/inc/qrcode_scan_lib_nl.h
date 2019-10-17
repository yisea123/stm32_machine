/*
	NewLand QR Code scanner
*/

#ifndef __QRCODE_SCAN_LIB_NL_H__
#define __QRCODE_SCAN_LIB_NL_H__

#define RX_BUFF_SIZE_NL		128
#define TX_BUFF_SIZE_NL		64

typedef struct _qrcode_scan_nl_rc
{
	t_usart_source * com;
	unsigned char rx_buf[RX_BUFF_SIZE_NL];
	unsigned char tx_buf[TX_BUFF_SIZE_NL];
}qrcode_scan_nl_rc;

int QRCodeScanNl_Install(qrcode_scan_t * qrcode_scan);

#endif

