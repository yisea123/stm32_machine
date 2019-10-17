/*
	二维码扫描驱动
*/
#include "s_def.h"
#include "qrcode_scan.h"
#include "qrcode_scan_lib.h"
#include "qrcode_scan_lib_hw.h"
#include "qrcode_scan_lib_nl.h"
#include "string.h"

#include "com.h"

t_usart_source scan_usart0;		//在初始化函数中赋值
t_usart_source scan_usart1;		//在初始化函数中赋值

extern t_usart_source com_qrcode_scan_source2;
extern t_usart_source com_qrcode_scan_source3;
extern t_usart_source com_embs_source;

qrcode_scan_hw_rc_t hw_rc =
{
	.trigger_pin = NULL,
	.com = &com_qrcode_scan_source2,
	.scan_device_type = unknow_scan,
};

qrcode_scan_hw_rc_t hw_rc2 =
{
	.trigger_pin = NULL,
	.com = &com_qrcode_scan_source3,
	.scan_device_type = unknow_scan,
};

qrcode_scan_hw_rc_t hw_rc3 =
{
	.trigger_pin = NULL,
	.com = &com_embs_source,
	.scan_device_type = unknow_scan,
};

qrcode_scan_nl_rc nl_rc =
{
	.com = &com_qrcode_scan_source2,
};

qrcode_scan_nl_rc nl_rc2 =
{
	.com = &com_qrcode_scan_source3,
};

qrcode_scan_nl_rc nl_rc3 =
{
	.com = &com_embs_source,
};

qrcode_scan_t qrcode_dev[] =
{

	{
		.resource = &hw_rc,
		.qrcode_scan_conf =
		{
			.tirgger_mode = TRIGGER_SW,
			.security_level = SECURITY_LEVEL_1,
		},
		.install = QRCodeScanHw_Install,
	},
/*
	{
		.resource = &nl_rc,
		.qrcode_scan_conf =
		{
			.tirgger_mode = TRIGGER_SW,
			.security_level = SECURITY_LEVEL_1,
		},
		.install = QRCodeScanNl_Install,
    },
*/
#if SCAN_STANDBY_CASE_TOP
	{
		.resource = &hw_rc2,
		.qrcode_scan_conf =
		{
			.tirgger_mode = TRIGGER_SW,
			.security_level = SECURITY_LEVEL_1,
		},
		.install = QRCodeScanHw_Install,
	},
/*
	{
		.resource = &nl_rc2,
		.qrcode_scan_conf =
		{
			.tirgger_mode = TRIGGER_SW,
			.security_level = SECURITY_LEVEL_1,
		},
		.install = QRCodeScanNl_Install,
	},
*/
#endif
#if SCAN_STANDBY_CASE_CAR
/*	{
		.resource = &nl_rc3,
		.qrcode_scan_conf =
		{
			.tirgger_mode = TRIGGER_SW,
			.security_level = SECURITY_LEVEL_1,
		},
		.install = QRCodeScanNl_Install,
	},
*/
	{
		.resource = &hw_rc3,
		.qrcode_scan_conf =
		{
			.tirgger_mode = TRIGGER_SW,
			.security_level = SECURITY_LEVEL_1,
		},
		.install = QRCodeScanHw_Install,
	},

#endif

};

#define qrcode_dev_num	(sizeof(qrcode_dev)/sizeof(qrcode_scan_t))

/* 
success : return 0
else    : return !0
*/
int QRCode_Init(void)
{
	unsigned int i;

	for(i = 0; i < qrcode_dev_num; i++)
	{
		if(qrcode_dev[i].install(&qrcode_dev[i]))
			return -1;
		if(QRCodeScan_Config(&qrcode_dev[i], &qrcode_dev[i].qrcode_scan_conf))
		{
			//return -1;	//保证下一个扫描仪能正常使用，不退出，不然下一个没有装载，会出现空指针
		}
		if(QRCodeScan_EnableCodeType(&qrcode_dev[i], BARCODE_TYPE_ALL))
		{
			//return -1;
		}
		if(QRCodeScan_DisableCodeType(&qrcode_dev[i], BARCODE_TYPE_ALL_1D))
		{
			//return -1;
		}
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
int QRCodeScan_Config(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config)
{
	if(qrcode_scan)
		if(qrcode_scan->ops)
			return qrcode_scan->ops->config(qrcode_scan, config);
	return -1;
}

/* 
success : return 0
else    : return !0
*/
int QRCodeScan_GetConfig(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config)
{
	if(qrcode_scan)
		if(qrcode_scan->ops)
			return qrcode_scan->ops->get_config(qrcode_scan, config);
	return -1;
}

/* 
success : return 0
else    : return !0
*/
int QRCodeScan_EnableCodeType(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type)
{
	if(qrcode_scan)
		if(qrcode_scan->ops)
			return qrcode_scan->ops->enable_code_type(qrcode_scan, type);
	return -1;
}

/* 
success : return 0
else    : return !0
*/
int QRCodeScan_DisableCodeType(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type)
{
	if(qrcode_scan)
		if(qrcode_scan->ops)
			return qrcode_scan->ops->disable_code_type(qrcode_scan, type);
	return -1;
}

extern void StringTailCutUnseeChar(unsigned char * str);
/* 
success : return 0
else    : return !0
*/
int QRCodeScan_Read(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms)
{
	if(!qrcode_scan)
		return -1;
	if(!qrcode_scan->ops)
		return -1;
	if(qrcode_scan->ops->read(qrcode_scan, buff, len, timeout_ms))
	{
		return -1;
	}
	else
	{
		StringTailCutUnseeChar(buff);
		*len = strlen((char *)buff);
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
int QRCodeScan_StartScan(struct _qrcode_scan_t * qrcode_scan)
{
	if(qrcode_scan)
		if(qrcode_scan->ops)
			return qrcode_scan->ops->start_scan(qrcode_scan);
	return -1;
}

/* 
success : return 0
else    : return !0
*/
int QRCodeScan_GetScanResult(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms)
{
	if(!qrcode_scan)
		return -1;
	if(!qrcode_scan->ops)
		return -1;
	if(qrcode_scan->ops->get_scan_result(qrcode_scan, buff, len, timeout_ms))
	{
		return -1;
	}
	else
	{
		StringTailCutUnseeChar(buff);
		*len = strlen((char *)buff);
	}
	return 0;
}


