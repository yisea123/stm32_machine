/*
  Filename: step_motor_basic.c
  Author: shidawei			Date: 20170415
  Copyright: 
  Description: STM3210X ֱ�����
  				����Lib
 */

#ifndef __DCMOTOR_LIB_H__
#define __DCMOTOR_LIB_H__
#include "lib_common.h"
#include "pin.h"

typedef enum
{
	DCMOTOR_DIR_FORWARD = 0,
	DCMOTOR_DIR_BACKWARD
}t_dcmotor_dir;

typedef enum
{
	DCMOTOR_ST_RUN,		//����
	DCMOTOR_ST_DEC,		//����
	DCMOTOR_ST_STOPED,	//ֹͣ
}t_dcmotor_st;

typedef struct
{
	t_dcmotor_dir		dir;		//direction
	unsigned int		steps;		//ֱ�������Ҫ�˶�����, ��msΪ��λ
	unsigned int		hold_steps;	//ǿ��ɲ����������msΪ��λ
	unsigned int		step_cnt;	//ֱ��������˶���������msΪ��λ
	t_dcmotor_st		status;		//status
}t_dcmotor_ctrl;


typedef struct
{
	t_pin_source *		pin_en;		//ʹ��
	t_pin_source *		pin_in1;		//����1
	t_pin_source *		pin_in2;		//����2
	t_dcmotor_ctrl *	dcmotor_ctrl;
}t_dcmotor_source;

void LibDcmotor_TimerHandler(t_dcmotor_source * p_source);
void LibDcmotor_Init(t_dcmotor_source * p_source);
void LibDcmotor_Move(t_dcmotor_source * p_source, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir);
void LibDcmotor_StartMove(t_dcmotor_source * p_source, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir);
void LibDcmotor_WaitStop(t_dcmotor_source * p_source);
void LibDcmotor_Stop(t_dcmotor_source * p_source, unsigned int stop_steps);

#endif

