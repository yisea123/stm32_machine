/*
  Filename: baud_adapt_lib.h
  Author: shidawei			Date: 2014-06-09
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: ���ڲ���������Ӧ�㷨LIB
*/
#ifndef __BAUD_ADAPT_LIB_H__
#define __BAUD_ADAPT_LIB_H__
#include "pin.h"

typedef enum
{
	PHASE_START, //����һ���½���
	PHASE_5T,	 //����һ�������� 
	PHASE_N,	 //����2~N���½��ء�������
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

//--------------����λ���ڶ���----------------
#define TIME_1S	1000000 // us

//BAUD_1T:1λ����λ����
#define	T1_4800BPS 		(TIME_1S/BAUD_4800_BPS)
#define	T1_9600BPS 		(TIME_1S/BAUD_9600_BPS)
#define	T1_19200BPS		(TIME_1S/BAUD_19200_BPS)
#define T1_38400BPS		(TIME_1S/BAUD_38400_BPS)
#define T1_57600BPS		(TIME_1S/BAUD_57600_BPS)
#define T1_115200BPS	(TIME_1S/BAUD_115200_BPS)	
//BAUD_5T:5λ����λ����
#define	T5_4800BPS 		(TIME_1S*5/BAUD_4800_BPS)
#define	T5_9600BPS 		(TIME_1S*5/BAUD_9600_BPS)
#define	T5_19200BPS		(TIME_1S*5/BAUD_19200_BPS)
#define T5_38400BPS		(TIME_1S*5/BAUD_38400_BPS)
#define T5_57600BPS		(TIME_1S*5/BAUD_57600_BPS)
#define T5_115200BPS	(TIME_1S*5/BAUD_115200_BPS)	
//BAUD_1_5T:1.5λ����λ����
#define	T1_5_4800BPS 	(TIME_1S*15/10/BAUD_4800_BPS)
#define	T1_5_9600BPS 	(TIME_1S*15/10/BAUD_9600_BPS)
#define	T1_5_19200BPS	(TIME_1S*15/10/BAUD_19200_BPS)
#define T1_5_38400BPS	(TIME_1S*15/10/BAUD_38400_BPS)
#define T1_5_57600BPS	(TIME_1S*15/10/BAUD_57600_BPS)
#define T1_5_115200BPS	(TIME_1S*15/10/BAUD_115200_BPS)	

/*------------------------------------------------------
			ͨѶЭ����ض���
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
			ʶ���㷨��ض���
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
	uint32_t * time_unit; //��λ:(1/10)T
}t_uint_record;

typedef struct
{
	//Ӳ����Դ����
	t_pin_source *		pin;				//���ڼ��㲨��������Ӧ��pin
	t_timer_source *	timer;				//����������Ӧ�㷨ʹ��timer�����뽫timer���ó�1us��period����Ϊ���
	//Ԥ�ṩ���ݣ�����Ҫ����Ŀ�����
	uint32_t *			p_baud_bps;			//ָ������bps����
	uint32_t *			p_baud_1t_uint;		//�����������Ӧ��1λ���ݵ�����ʱ��
	uint8_t				baud_index;			//bps��������
	t_uint_record *		protocol_array;		//Э��ṹ����ָ��
	uint8_t				protocal_num;		//֧�ֵ�Э����������Щ�н���ѡ��
	uint8_t				period_num;			//�����������������Ƚϵ�ʱ���������鳤��
	uint8_t				first_period_bits;	//��һ��ʱ�����ڵ�λ��
	//�㷨��أ���ʱ�洢������м�����
	t_autobaud_phase	phase;				//��λ����
	uint32_t *			period_array;		//����������ʱ������
	uint32_t *			period_unit_array;	//���ź������������ʱ������
	//���������м�����
	uint32_t			baudrate;			//������
	uint16_t			last_timecnt;		//�ϴ�ʱ��������ǵ���ֵ
	uint8_t           	cur_protocal;		//��ǰ�Աȵ���Э��
	uint8_t				period_cnt;			//����������ʱ��������ѽ��յ�������
	uint8_t           	recd_array_cnt;     //��¼�ѱȶ���ȷ�����������λ��ֵ---��ʼ��ֵΪ0
	t_autobaud_st		st;					//�ڼ�������е�״̬
	//�����������
	fun_set_baudrate	fun_SetBaudrate;	//�޸Ķ�Ӧ���ڲ�������ָ��
	fun_set_com_en		fun_SetComEn;		//ʹ�ܴ��ں���ָ��
	uint8_t *			p_ch;				//�����ַ�����ָ�룬������0Ϊ�������������ж���Ҫ�����ַ�����
	fun_unget_ch		fun_UngetCh;		//�����ַ������ڻ���������ָ��
}t_autobaud_source;

void BaudAdapt_Init(t_autobaud_source *p_autobaud_source);	
void BaudAdapt_Enable(t_autobaud_source * p_autobaud_source, fun_exti_handler handler, FunctionalState enable);
uint8_t BaudAdapt_GetDone(t_autobaud_source * p_autobaud_source);
void BaudAdapt_Handler(t_autobaud_source *p_autobaud_source);

#endif

