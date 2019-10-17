/*
  Filename: baud_adapt_lib.h
  Author: shidawei			Date: 2014-06-09
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 串口波特率自适应算法LIB
*/
#ifndef __BAUD_ADAPT_LIB_H__
#define __BAUD_ADAPT_LIB_H__
#include "pin.h"

typedef enum
{
	PHASE_START, //检测第一个下降沿
	PHASE_5T,	 //检测第一个上升沿 
	PHASE_N,	 //检测第2~N个下降沿、上升沿
}t_autobaud_phase;

typedef enum
{
	AB_NONE,
	AB_KO,		//fail
	AB_OK,		//success
}t_autobaud_st;

typedef enum
{
	BAUD_4800,
	BAUD_9600,
	BAUD_19200,
	BAUD_38400,
	BAUD_57600,
	//BAUD_115200,
	BAUD_NUM
}t_com_baud;

typedef void (* fun_set_baudrate)(uint32_t baudrate,uint16_t wordlength,uint16_t stopbits,uint16_t parity);
typedef void (* fun_unget_ch)(uint8_t ch);
typedef void (* fun_set_com_en)(FunctionalState en);

#define BAUD_4800_BPS	4800
#define BAUD_9600_BPS	9600
#define BAUD_19200_BPS	19200
#define BAUD_38400_BPS	38400
#define	BAUD_57600_BPS	57600
#define BAUD_115200_BPS	115200

//--------------数据位周期定义----------------
#define TIME_1S	1000000 // us

//BAUD_1T:1位数据位周期
#define	T1_4800BPS 		(TIME_1S/BAUD_4800_BPS)
#define	T1_9600BPS 		(TIME_1S/BAUD_9600_BPS)
#define	T1_19200BPS		(TIME_1S/BAUD_19200_BPS)
#define T1_38400BPS		(TIME_1S/BAUD_38400_BPS)
#define T1_57600BPS		(TIME_1S/BAUD_57600_BPS)
#define T1_115200BPS	(TIME_1S/BAUD_115200_BPS)	
//BAUD_5T:5位数据位周期
#define	T5_4800BPS 		(TIME_1S*5/BAUD_4800_BPS)
#define	T5_9600BPS 		(TIME_1S*5/BAUD_9600_BPS)
#define	T5_19200BPS		(TIME_1S*5/BAUD_19200_BPS)
#define T5_38400BPS		(TIME_1S*5/BAUD_38400_BPS)
#define T5_57600BPS		(TIME_1S*5/BAUD_57600_BPS)
#define T5_115200BPS	(TIME_1S*5/BAUD_115200_BPS)	
//BAUD_1_5T:1.5位数据位周期
#define	T1_5_4800BPS 	(TIME_1S*15/10/BAUD_4800_BPS)
#define	T1_5_9600BPS 	(TIME_1S*15/10/BAUD_9600_BPS)
#define	T1_5_19200BPS	(TIME_1S*15/10/BAUD_19200_BPS)
#define T1_5_38400BPS	(TIME_1S*15/10/BAUD_38400_BPS)
#define T1_5_57600BPS	(TIME_1S*15/10/BAUD_57600_BPS)
#define T1_5_115200BPS	(TIME_1S*15/10/BAUD_115200_BPS)	

/*------------------------------------------------------
			通讯协议相关定义
--------------------------------------------------------*/
#define DATA_5BIT 		0x100
#define	DATA_6BIT		0x200
#define	DATA_7BIT		0x400
#define	DATA_8BIT		0x800
#define DATA_BIT_MASK	0xF00

#define	STOP_1BIT		0x010
#define STOP_1_5BIT 	0x020
#define	STOP_2BIT		0x040
#define STOP_BIT_MASK	0x0F0

#define	PARITY_NONE		0x000
#define	PARITY_ODD		0x001
#define	PARITY_EVEN		0x002
#define PARITY_MASK		0x00F


/*------------------------------------------------------
			识别算法相关定义
--------------------------------------------------------*/

typedef struct
{
	uint16_t wordlength;
	uint16_t stopbits;
	uint16_t parity; 
}t_protocol;

typedef struct
{
	t_protocol protocol;
	uint8_t    uint_num;
	uint32_t * time_unit; //单位:(1/10)T
}t_uint_record;

typedef struct
{
	//硬件资源配置
	t_pin_source *		pin;				//用于计算波特率自适应的pin
	t_timer_source *	timer;				//波特率自适应算法使用timer，必须将timer设置成1us，period设置为最大
	//预提供数据，与需要计算目标相关
	uint32_t *			p_baud_bps;			//指向波特率bps数组
	uint32_t *			p_baud_1t_uint;		//波特率数组对应的1位数据的周期时间
	uint8_t				baud_index;			//bps数组索引
	t_uint_record *		protocol_array;		//协议结构数组指针
	uint8_t				protocal_num;		//支持的协议数，在这些中进行选择
	uint8_t				period_num;			//跳变沿周期数，即比较的时间周期数组长度
	uint8_t				first_period_bits;	//第一个时间周期的位数
	//算法相关，临时存储与计算中间数据
	t_autobaud_phase	phase;				//相位序列
	uint32_t *			period_array;		//跳变沿周期时间数组
	uint32_t *			period_unit_array;	//缩放后的跳变沿周期时间数组
	//计算结果与中间数据
	uint32_t			baudrate;			//波特率
	uint16_t			last_timecnt;		//上次时间计数器记到的值
	uint8_t           	cur_protocal;		//当前对比到的协议
	uint8_t				period_cnt;			//跳变沿周期时间计数，已接收的周期数
	uint8_t           	recd_array_cnt;     //记录已比对正确后的在数组中位置值---初始化值为0
	t_autobaud_st		st;					//在计算过程中的状态
	//串口相关配置
	fun_set_baudrate	fun_SetBaudrate;	//修改对应串口参数函数指针
	fun_set_com_en		fun_SetComEn;		//使能串口函数指针
	uint8_t *			p_ch;				//回退字符数组指针，必须以0为结束符，用于判断需要回退字符长度
	fun_unget_ch		fun_UngetCh;		//回退字符到串口缓冲区函数指针
}t_autobaud_source;

void BaudAdapt_Init(t_autobaud_source *p_autobaud_source);	
void BaudAdapt_Enable(t_autobaud_source * p_autobaud_source, fun_exti_handler handler, FunctionalState enable);
uint8_t BaudAdapt_GetDone(t_autobaud_source * p_autobaud_source);
void BaudAdapt_Handler(t_autobaud_source *p_autobaud_source);

#endif

