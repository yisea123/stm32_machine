/*
  Filename: motor.h
  Author: shidawei			Date: 2014-06-18
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 电机驱动
*/

#ifndef __MOTOR_H__
#define __MOTOR_H__
#include "step_motor_basic.h"

#define HOLDING_TIME		2900

typedef enum
{
	MOTOR_X = 0,
	MOTOR_Y,
	MOTOR_Z,
	MOTOR_HOOK,
	MOTOR_SCANNER,
	MOTOR_MAX
}t_motor_no;

enum
{
	MOTOR_PIN_EN = 0,
	MOTOR_PIN_PLUSE,
	MOTOR_PIN_DIR,
	MOTOR_PIN_SLEEP,
	MOTOR_PIN_MAX
};

void Motor_Init(void);
void Motor_Move(t_motor_no motor_no, unsigned int step, unsigned int micro_step,
	t_motor_dir dir, uint16_t speed);
void Motor_Stop(t_motor_no motor_no, unsigned int stop_step, unsigned int micro_stop_step);
void Motor_WaitStoped(t_motor_no motor_no);
uint8_t Motor_IsStoped(t_motor_no motor_no);
void Motor_IntHandler(t_motor_no motor_no);
void Motor_Hold(t_motor_no motor_no, FunctionalState enable);
void Motor_Speed(t_motor_no motor_no, uint16_t speed);
unsigned int Motor_GetRuningSteps(t_motor_no motor_no);
void Motor_Tim2IntHandler(void);
void Motor_Tim3IntHandler(void);
void Motor_Tim5IntHandler(void);


#endif

