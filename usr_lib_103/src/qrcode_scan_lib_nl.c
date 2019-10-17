/*
	NewLand新大陆二维码扫描仪驱动库
*/
#include "pin.h"
#include "usart.h"
#include "qrcode_scan_lib.h"
#include "qrcode_scan_lib_nl.h"
#include "string.h"

#define MSG_HEAD_LENGTH				6
#define MSG_TAIL_LENGTH				2
#define MSG_ADDITION_LENGTH			(MSG_HEAD_LENGTH+MSG_TAIL_LENGTH)
#define SETTING_RX_TIMEOUT			200
#define DECODE_RX_TIMEOUT			500
const unsigned char msg_head[MSG_HEAD_LENGTH] = {0x7E, 0x01, '0', '0', '0', '0'};
const unsigned char msg_tail[MSG_TAIL_LENGTH] = {';', 0x03};

/* 临时指令与永久指令的差别在与临时指令设置后掉电丢失，永久设置掉电不丢失 */
const unsigned char led_off_forever[] = 	{0x7E,0x01,'0','0','0','0', '@','I','L','L','S','C','N','0', ';',0x03};	//永久关闭LED
const unsigned char led_on_forever[] = 		{0x7E,0x01,'0','0','0','0', '@','I','L','L','S','C','N','1', ';',0x03};	//永久打开LED
const unsigned char led_off_after_scan[] = 	{0x7E,0x01,'0','0','0','0', '@','G','R','L','E','N','A','0', ';',0x03};	//扫描后关闭LED
const unsigned char led_on_after_scan[] = 	{0x7E,0x01,'0','0','0','0', '@','G','R','L','E','N','A','1', ';',0x03};	//扫描后打开LED
const unsigned char led_off[] = 			{0x7E,0x01,'0','0','0','0', '#','I','L','L','S','C','N','0', ';',0x03};	//临时关闭LED
const unsigned char led_on[] = 				{0x7E,0x01,'0','0','0','0', '#','I','L','L','S','C','N','1', ';',0x03};	//临时打开LED

const unsigned char scan_dis_forever[] = 	{0x7E,0x01,'0','0','0','0', '@','S','C','N','E','N','A','0', ';',0x03};	//失能扫描
const unsigned char scan_en_forever[] = 	{0x7E,0x01,'0','0','0','0', '@','S','C','N','E','N','A','1', ';',0x03};	//使能扫描
const unsigned char scan_dis[] = 			{0x7E,0x01,'0','0','0','0', '#','S','C','N','E','N','A','0', ';',0x03};	//临时失能扫描
const unsigned char scan_en[] = 			{0x7E,0x01,'0','0','0','0', '#','S','C','N','E','N','A','1', ';',0x03};	//临时使能扫描

const unsigned char scan_mode_level[] = 	{0x7E,0x01,'0','0','0','0', '@','S','C','N','M','O','D','0', ';',0x03};	//电平触发模式
const unsigned char scan_mode_sensor[] = 	{0x7E,0x01,'0','0','0','0', '@','S','C','N','M','O','D','2', ';',0x03};	//感应模式
const unsigned char scan_mode_continue[] = 	{0x7E,0x01,'0','0','0','0', '@','S','C','N','M','O','D','3', ';',0x03};	//连续读码模式

const unsigned char retry_read_delay_dis[] ={0x7E,0x01,'0','0','0','0', '@','R','R','D','E','N','A','0', ';',0x03};	//重读延迟关闭
const unsigned char retry_read_delay_en[] =	{0x7E,0x01,'0','0','0','0', '@','R','R','D','E','N','A','1', ';',0x03};	//重读延迟开启

const unsigned char retry_read_time_set[] =	{0x7E,0x01,'0','0','0','0', '@','R','R','D','D','U','R','5','0','0', ';',0x03};	//重读延迟设置为500ms

const unsigned char all_prefix_suffix_dis[] ={0x7E,0x01,'0','0','0','0', '@','A','P','S','E','N','A','0', ';',0x03};	//禁止所有前后缀
const unsigned char all_prefix_suffix_en[] ={0x7E,0x01,'0','0','0','0', '@','A','P','S','E','N','A','1', ';',0x03};	//使能所有前后缀

const unsigned char serial_trigger_en[] =	{0x7E,0x01,'0','0','0','0', '@','S','C','N','T','C','E','1', ';',0x03};	//使能串行触发指令
const unsigned char serial_trigger_dis[] =	{0x7E,0x01,'0','0','0','0', '@','S','C','N','T','C','E','0', ';',0x03};	//禁止串行触发指令

const unsigned char serial_trigger_start_code[] =	{0x01,'T',0x04};	//串行触发启动扫描指令
const unsigned char serial_trigger_stop_code[] =	{0x01,'P',0x04};	//串行触发停止扫描指令

const unsigned char all_1d_code_dis[] =	{0x7E,0x01,'0','0','0','0', '@','A','L','L','1','D','C','0', ';',0x03};	//禁止所有一维条码
const unsigned char all_1d_code_en[] =	{0x7E,0x01,'0','0','0','0', '@','A','L','L','1','D','C','1', ';',0x03};	//允许所有一维条码

const unsigned char all_2d_code_dis[] =	{0x7E,0x01,'0','0','0','0', '@','A','L','L','2','D','C','0', ';',0x03};	//禁止所有二维条码
const unsigned char all_2d_code_en[] =	{0x7E,0x01,'0','0','0','0', '@','A','L','L','2','D','C','1', ';',0x03};	//允许所有二维条码

const unsigned char qr_code_dis[] =	{0x7E,0x01,'0','0','0','0', '@','Q','R','C','E','N','A','0', ';',0x03};	//禁止所有二维条码
const unsigned char qr_code_en[] =	{0x7E,0x01,'0','0','0','0', '@','Q','R','C','E','N','A','1', ';',0x03};	//允许所有二维条码

const unsigned char all_code_dis[] ={0x7E,0x01,'0','0','0','0', '@','A','L','L','E','N','A','0', ';',0x03};	//允许所有条码
const unsigned char all_code_en[] =	{0x7E,0x01,'0','0','0','0', '@','A','L','L','E','N','A','1', ';',0x03};	//允许所有条码

const unsigned char en_exposure_forever[] = 	{0x7E,0x01,'0','0','0','0', '@','E','X','P','E','E','N','1', ';',0x03};	//开启曝光时间设置
const unsigned char exposure_time_min_set[] =	{0x7E,0x01,'0','0','0','0', '@','E','X','P','E','M','N','2','0','0','0', ';',0x03};	//最小曝光时间2000us
const unsigned char exposure_time_max_set[] =	{0x7E,0x01,'0','0','0','0', '@','E','X','P','E','M','X','2','0','0','0', ';',0x03};	//最大曝光时间2000us
const unsigned char dis_exposure_forever[] = 	{0x7E,0x01,'0','0','0','0', '@','E','X','P','E','E','N','0', ';',0x03};	//关闭曝光时间设置

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_TxMsg(qrcode_scan_nl_rc * rc, unsigned char const * tx_msg, unsigned int tx_len)
{
	unsigned char ch;

	memcpy(rc->tx_buf, tx_msg, tx_len);		//add msg
	while(Usart_GetCh(rc->com, &ch));	//clean rx before
	Usart_SendBuf(rc->com, rc->tx_buf, tx_len);
	Usart_WaitSendCompleted(rc->com);

	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_RxMsg(qrcode_scan_nl_rc * rc, unsigned char * rx_msg, unsigned int * rx_len, unsigned int timeout_ms)
{
	unsigned int timeout;
	unsigned char ch;
	unsigned int len = 0;

	*rx_len = 0;
	timeout = timeout_ms;
	Usart_WaitSendCompleted(rc->com);
	while(timeout)
	{
		if(Usart_GetCh(rc->com, &ch))
		{			
			timeout = 50;		//如果收到数据则在下50ms内没有收到数据表示此次通讯结束
			rx_msg[len] = ch;
			len++;
		}
		else
		{
			timeout--;
			QRCODESCAN_DELAY_MS(1);
		}
	}
	*rx_len = len;
	return 0;
}

static int QRCodeScanNl_RxResJudge(qrcode_scan_nl_rc * rc, unsigned char * rx_msg, unsigned int rx_len)
{
	if(rx_msg[rx_len - 1 - MSG_TAIL_LENGTH] == 0x06)
		return 0;
	else
		return -1;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_Init(struct _qrcode_scan_t * qrcode_scan)
{
	qrcode_scan_nl_rc * rc;
	unsigned int rx_len;
	
	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	//led off
	if(QRCodeScanNl_TxMsg(rc, led_off_forever, sizeof(led_off_forever)))
	{
		return -1;
	}
	rx_len = sizeof(led_off_forever) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	//set scan mode, 新大陆模块电平触发与软件触发可以同时
	if(QRCodeScanNl_TxMsg(rc, scan_mode_level, sizeof(scan_mode_level)))
	{
		return -1;
	}
	rx_len = sizeof(scan_mode_level) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	//read retry delay enable
	if(QRCodeScanNl_TxMsg(rc, retry_read_delay_en, sizeof(retry_read_delay_en)))
	{
		return -1;
	}
	rx_len = sizeof(retry_read_delay_en) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	//set read retry after time
	if(QRCodeScanNl_TxMsg(rc, retry_read_time_set, sizeof(retry_read_time_set)))
	{
		return -1;
	}
	rx_len = sizeof(retry_read_time_set) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
#if DISABLE_ALL_PREFIX_SUFFIX
	//disable all prefix or suffix
	if(QRCodeScanNl_TxMsg(rc, all_prefix_suffix_dis, sizeof(all_prefix_suffix_dis)))
	{
		return -1;
	}
	rx_len = sizeof(all_prefix_suffix_dis) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
#else
	//enable all prefix or suffix
	if(QRCodeScanNl_TxMsg(rc, all_prefix_suffix_en, sizeof(all_prefix_suffix_en)))
	{
		return -1;
	}
	rx_len = sizeof(all_prefix_suffix_en) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
#endif
#if 1	//设置曝光时间
	if(QRCodeScanNl_TxMsg(rc, en_exposure_forever, sizeof(en_exposure_forever)))	//1
	{
		return -1;
	}
	rx_len = sizeof(en_exposure_forever) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	
	if(QRCodeScanNl_TxMsg(rc, exposure_time_min_set, sizeof(exposure_time_min_set)))	//2
	{
		return -1;
	}
	rx_len = sizeof(exposure_time_min_set) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}

	if(QRCodeScanNl_TxMsg(rc, exposure_time_max_set, sizeof(exposure_time_max_set)))	//3
	{
		return -1;
	}
	rx_len = sizeof(exposure_time_max_set) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	
/*	if(QRCodeScanNl_TxMsg(rc, dis_exposure_forever, sizeof(dis_exposure_forever)))	//4
	{
		return -1;
	}
	rx_len = sizeof(dis_exposure_forever) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
*/
#endif
	//enable serial trigger to scan in level mode
	if(QRCodeScanNl_TxMsg(rc, serial_trigger_en, sizeof(serial_trigger_en)))
	{
		return -1;
	}
	rx_len = sizeof(serial_trigger_en) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_Config(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config)
{
	qrcode_scan_nl_rc * rc;
	
	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	qrcode_scan->qrcode_scan_conf.security_level = config->security_level;
	qrcode_scan->qrcode_scan_conf.tirgger_mode = config->tirgger_mode;
	if(QRCodeScanNl_Init(qrcode_scan))
		return -1;
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_GetConfig(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config)
{
	qrcode_scan_nl_rc * rc;
	
	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	config->security_level = qrcode_scan->qrcode_scan_conf.security_level;
	config->tirgger_mode = qrcode_scan->qrcode_scan_conf.tirgger_mode;
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_EnableCodeType(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type)
{
	qrcode_scan_nl_rc * rc;
	const unsigned char * msg;
	unsigned int rx_len;
	unsigned int tx_len;
	
	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	switch (type)
	{
		case BARCODE_TYPE_ALL_1D:
			msg = all_1d_code_en;
			tx_len = sizeof(all_1d_code_en);
			break;
		case BARCODE_TYPE_ALL_2D:
			msg = all_2d_code_en;
			tx_len = sizeof(all_2d_code_en);
			break;
		case BARCODE_TYPE_QRCODE:
			msg = qr_code_en;
			tx_len = sizeof(qr_code_en);
			break;
		case BARCODE_TYPE_ALL:
			msg = all_code_en;
			tx_len = sizeof(all_code_en);
			break;
		default:
			return -1;
	}
	if(QRCodeScanNl_TxMsg(rc, msg, tx_len))
	{
		return -1;
	}
	rx_len = tx_len + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_DisableCodeType(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type)
{
	qrcode_scan_nl_rc * rc;
	const unsigned char * msg;
	unsigned int rx_len;
	unsigned int tx_len;
	
	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	switch (type)
	{
		case BARCODE_TYPE_ALL_1D:
			msg = all_1d_code_dis;
			tx_len = sizeof(all_1d_code_dis);
			break;
		case BARCODE_TYPE_ALL_2D:
			msg = all_2d_code_dis;
			tx_len = sizeof(all_2d_code_dis);
			break;
		case BARCODE_TYPE_QRCODE:
			msg = qr_code_dis;
			tx_len = sizeof(qr_code_dis);
			break;
		case BARCODE_TYPE_ALL:
			msg = all_code_dis;
			tx_len = sizeof(all_code_dis);
			break;
		default:
			return -1;
	}
	if(QRCodeScanNl_TxMsg(rc, msg, tx_len))
	{
		return -1;
	}
	rx_len = tx_len + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_StartScan(struct _qrcode_scan_t * qrcode_scan)
{
	qrcode_scan_nl_rc * rc;
	unsigned int rx_len;
	
	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	//set led on
	if(QRCodeScanNl_TxMsg(rc, led_on, sizeof(led_on)))
	{
		return -1;
	}
	rx_len = sizeof(led_on) + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	//serial trigger to scan
	if(QRCodeScanNl_TxMsg(rc, serial_trigger_start_code, sizeof(serial_trigger_start_code)))
	{
		return -1;
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_GetScanResult(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms)
{
	qrcode_scan_nl_rc * rc;
	unsigned int rx_len;
	int ret = 0;
	
	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	rx_len = 256;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, timeout_ms))
	{
		return -1;
	}
	if(rx_len)
	{
		memcpy(buff, rc->rx_buf, rx_len);
		*len = rx_len;
		ret = 0;
	}
	else
	{
		ret = -1;
	}
	//stop scan
	if(QRCodeScanNl_TxMsg(rc, serial_trigger_stop_code, sizeof(serial_trigger_stop_code)))
	{
		return -1;
	}
	//set led off
	if(QRCodeScanNl_TxMsg(rc, led_off, sizeof(led_off)))
	{
		return -1;
	}
	rx_len = sizeof(led_off) + MSG_ADDITION_LENGTH + 1;
	if(QRCodeScanNl_RxMsg(rc, rc->rx_buf, &rx_len, SETTING_RX_TIMEOUT))
	{
		return -1;
	}
	if(QRCodeScanNl_RxResJudge(rc, rc->rx_buf, rx_len))
	{
		return -1;
	}
	return ret;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanNl_Read(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms)
{
	if(QRCodeScanNl_StartScan(qrcode_scan))	
		return -1;
	if(QRCodeScanNl_GetScanResult(qrcode_scan, buff, len, timeout_ms))
		return -1;
	return 0;
}

qrcode_scan_ops_t qrcode_scan_nl_ops =
{
	.config = QRCodeScanNl_Config,
	.get_config = QRCodeScanNl_GetConfig,
	.enable_code_type = QRCodeScanNl_EnableCodeType,
	.disable_code_type = QRCodeScanNl_DisableCodeType,
	.read = QRCodeScanNl_Read,
	.start_scan = QRCodeScanNl_StartScan,
	.get_scan_result = QRCodeScanNl_GetScanResult,
};

/* 
success : return 0
else    : return !0
*/
int QRCodeScanNl_Install(qrcode_scan_t * qrcode_scan)
{
	qrcode_scan_nl_rc * rc;
	
	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	qrcode_scan->ops = &qrcode_scan_nl_ops;
	qrcode_scan->install = QRCodeScanNl_Install;

	return 0;
}


