#ifndef __DCMOTOR_H__
#define __DCMOTOR_H__

#include "dcmotor_lib.h"

enum
{
	DCMOTOR_PIN_EN,
	DCMOTOR_PIN_IN1,
	DCMOTOR_PIN_IN2,
	DCMOTOR_PIN_MAX
};

typedef enum
{
	DCMOTOR_Z2,
	DCMOTOR_MAX
}t_dcmotor_no;

void DCMotor_TimerHandler(t_dcmotor_no dcmotor_no);
void DCMotor_Init(void);
void DCMotor_Move(t_dcmotor_no dcmotor_no, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir);
void DCMotor_StartMove(t_dcmotor_no dcmotor_no, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir);
void DCMotor_WaitStop(t_dcmotor_no dcmotor_no);
void DCMotor_Stop(t_dcmotor_no dcmotor_no, unsigned int stop_steps);

#endif

