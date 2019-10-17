/*
  Filename: step_motor_basic.c
  Author: shidawei			Date: 20170415
  Copyright: 
  Description: STM3210X 直流电机
  				驱动Lib
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
	DCMOTOR_ST_RUN,		//运行
	DCMOTOR_ST_DEC,		//减速
	DCMOTOR_ST_STOPED,	//停止
}t_dcmotor_st;

typedef struct
{
	t_dcmotor_dir		dir;		//direction
	unsigned int		steps;		//直流电机需要运动步数, 以ms为单位
	unsigned int		hold_steps;	//强制刹车步数，以ms为单位
	unsigned int		step_cnt;	//直流电机已运动步数，以ms为单位
	t_dcmotor_st		status;		//status
}t_dcmotor_ctrl;


typedef struct
{
	t_pin_source *		pin_en;		//使能
	t_pin_source *		pin_in1;		//输入1
	t_pin_source *		pin_in2;		//输入2
	t_dcmotor_ctrl *	dcmotor_ctrl;
}t_dcmotor_source;

void LibDcmotor_TimerHandler(t_dcmotor_source * p_source);
void LibDcmotor_Init(t_dcmotor_source * p_source);
void LibDcmotor_Move(t_dcmotor_source * p_source, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir);
void LibDcmotor_StartMove(t_dcmotor_source * p_source, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir);
void LibDcmotor_WaitStop(t_dcmotor_source * p_source);
void LibDcmotor_Stop(t_dcmotor_source * p_source, unsigned int stop_steps);

#endif

