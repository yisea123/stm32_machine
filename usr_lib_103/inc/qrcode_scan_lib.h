/*
	二维码扫描仪驱动库头文件
*/

#ifndef __QRCODE_SCAN_LIB_H__
#define __QRCODE_SCAN_LIB_H__

#define DISABLE_ALL_PREFIX_SUFFIX		1

extern void delay_ms(__IO uint32_t ms);
#define QRCODESCAN_DELAY_MS(n)		delay_ms(n)

typedef enum
{
	BARCODE_TYPE_ALL_1D,
	BARCODE_TYPE_ALL_2D,
	BARCODE_TYPE_QRCODE,
	BARCODE_TYPE_ALL,
	BARCODE_NUM
}barcode_type_t;

typedef enum{
	SECURITY_LEVEL_1 = 0x01,
	SECURITY_LEVEL_2 = 0x02,
	SECURITY_LEVEL_3 = 0x03,
	SECURITY_LEVEL_4 = 0x04
}t_qrcode_scan_security_level;


typedef enum
{
	TRIGGER_SW = 0,
	TRIGGER_HW,
}t_qrcode_scan_trigger_mode;

typedef struct _qrcode_scan_conf_t
{
	t_qrcode_scan_trigger_mode tirgger_mode;
	uint8_t  			security_level;
}qrcode_scan_conf_t;

typedef struct _qrcode_scan_t
{
	void * resource;
	qrcode_scan_conf_t qrcode_scan_conf;
	int (*install)(struct _qrcode_scan_t * qrcode_scan);
	struct _qrcode_scan_ops_t * ops;
}qrcode_scan_t;

typedef struct _qrcode_scan_ops_t
{
	int (*config)(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config);
	int (*get_config)(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config);
	int (*enable_code_type)(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type);
	int (*disable_code_type)(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type);
	/* 一步到位的读取二维码，读取到后才返回 */
	int (*read)(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms);
	/* 启动读二维码，之后可以进行其余动作，再调用get_scan_result获取扫描结果 */
	int (*start_scan)(struct _qrcode_scan_t * qrcode_scan);
	/* 获取二维码结果 */
	int (*get_scan_result)(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms);
}qrcode_scan_ops_t;

#endif
