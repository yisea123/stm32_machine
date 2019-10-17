/*
  Filename: step_motor_basic.c
  Author: shidawei			Date: 20140116
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: STM3210X ���оƬA4982SLP or L6219
  				����Lib
 */
#ifndef __STEP_MOTOR_BASIC_H__
#define __STEP_MOTOR_BASIC_H__
#include "lib_common.h"
#include "pin.h"

//#define L6219			//ʹ��L6219���оƬ
#define MOTOR_LOW_LEVEL_ENABLE	1	//ʹ�õ͵�ƽ��Ϊʹ���ź���8825���ߵ�ƽʹ������0��8880


typedef void(* Fun_StepMotor_Enable)(FunctionalState enable);

//motor direction
typedef enum
{
	MOTOR_DIR_FORWARD = 0,
	MOTOR_DIR_BACKWARD
}t_motor_dir;

typedef enum
{
	MOTOR_MODE_FULL,
	MOTOR_MODE_HALF,
	MOTOR_MODE_QUARTER,
	MOTOR_MODE_SIXTEEN,
	MOTOR_MODE_MAX
}t_motor_mode;

typedef enum
{
	MOTOR_ST_INIT,		//��ʼ��״̬
	MOTOR_ST_HOLD,		//Holding
	MOTOR_ST_ACC,		//����
	MOTOR_ST_CONST,		//����
	MOTOR_ST_DEC,		//����
	MOTOR_ST_POSIT		//ֹͣ����
}t_motor_st;

typedef struct
{
	t_motor_dir			dir;				//direction
	t_motor_st			status;				//status
	unsigned int		step;				//����˶�����
	unsigned int		step_cnt;			//����Ѿ��߹��Ĳ���
#ifdef L6219
	const uint8_t *		pulse_val_base;		//������ֵ����ָ�룬��ʼ��ΪNULL����ʹ��ʱ����ģʽ���ı�
	uint8_t 			pulse_val_len;		//������ֵָ�����鳤�ȣ���ʼ��Ϊ0����ʹ��ʱ���ı�
	uint8_t				pulse_index;		//��ǰָ����������ֵ����
#else
	BitAction 			pulse_val;			//�����ƽ
#endif
	uint16_t    acc_ramp_max_size;	//���ٱ���󳤶�
	uint16_t    dec_ramp_max_size;	//���ٱ���󳤶�
	uint16_t    acc_ramp_rt_size;	//���ٱ�ʵʱ����
	uint16_t    dec_ramp_rt_size;	//���ٱ�ʵʱ����

	const uint16_t *	ptime_ramp;			//ָ��ǰ����ʱ��
	const uint16_t *	ptime_ramp_tbl;		//ָ��ǰ���ָ��
	uint16_t	time_tbl_size;		//��ǰ���С
}t_step_motor_ctrl;

typedef struct
{
	t_motor_mode   mode;
	const uint16_t * ptime_acc_ramp_tbl;	//ָ����ٱ��ָ�룬���ٱ������00���������ڼ������
	const uint16_t * ptime_dec_ramp_tbl;	//ָ����ٱ��ָ�룬���ٱ������00���������ڼ������
#ifdef L6219
	t_pin_source * pin_i01;
	t_pin_source * pin_i11;
	t_pin_source * pin_ph1;
	t_pin_source * pin_i02;
	t_pin_source * pin_i12;
	t_pin_source * pin_ph2;
#else
	t_pin_source * pin_en;
	t_pin_source * pin_pulse;
	t_pin_source * pin_dir;
	t_pin_source * pin_sleep;
	t_pin_source * motor_en_pin;//���ѡͨ�ź�
#ifdef STEP_MOTOR_MODE
	t_pin_source * pin_ms1;
	t_pin_source * pin_ms2;
#endif
#endif
	t_timer_source * timer_source;
	t_step_motor_ctrl * step_motor_ctrl;
}t_step_motor_source;

//functions declare
void StepMotor_Init(t_step_motor_source * p_step_motor_source);
void StepMotor_Move(t_step_motor_source * p_step_motor_source, unsigned int step, unsigned int micro_step,
	t_motor_dir dir, uint16_t speed);
void StepMotor_IntHandler(t_step_motor_source * p_step_motor_source);
void StepMotor_Stop(t_step_motor_source * p_step_motor_source, unsigned int stop_step, unsigned int micro_stop_step);
void StepMotor_WaitStoped(t_step_motor_source * p_step_motor_source);
uint8_t StepMotor_IsStoped(t_step_motor_source * p_step_motor_source);
void StepMotor_Hold(t_step_motor_source * p_step_motor_source, FunctionalState enable);
void StepMotor_Speed(t_step_motor_source * p_step_motor_source, uint16_t speed);
unsigned int StepMotor_GetRuningSteps(t_step_motor_source * p_step_motor_source);

#endif


