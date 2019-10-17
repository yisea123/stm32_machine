/*
	二维码扫描驱动
*/
#ifndef __QRCODE_SCAN_H__
#define __QRCODE_SCAN_H__

#include "qrcode_scan_lib.h"

extern qrcode_scan_t qrcode_dev[];
#define QRCODE_SCAN_DEV_0			(&qrcode_dev[0])	//HollyWell qrcode scanner
#define QRCODE_SCAN_DEV_STANDBY_TOP	(&qrcode_dev[1])	//NewLand备用扫描头，失败后用此扫描头再扫描
#if SCAN_STANDBY_CASE_CAR
#define QRCODE_SCAN_DEV_STANDBY_CAR	(&qrcode_dev[2])
#endif
#define QRCODE_SCAN_DEV				QRCODE_SCAN_DEV_0

int QRCode_Init(void);
int QRCodeScan_Config(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config);
int QRCodeScan_GetConfig(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config);
int QRCodeScan_EnableCodeType(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type);
int QRCodeScan_DisableCodeType(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type);
int QRCodeScan_Read(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms);
int QRCodeScan_StartScan(struct _qrcode_scan_t * qrcode_scan);
int QRCodeScan_GetScanResult(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms);

#endif
