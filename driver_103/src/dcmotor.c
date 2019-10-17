/*
	直流电机驱动
*/
#include "dcmotor_lib.h"
#include "dcmotor.h"
#include "pin.h"

static t_pin_source motor_x_pin[DCMOTOR_PIN_MAX]=
{
	//没有使能脚，不能直接置空，这里随意使用一个定义，在调用该变量时使用空指针
	{GPIOE, RCC_APB2Periph_GPIOE,	GPIO_Pin_13, 	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOE, RCC_APB2Periph_GPIOE,	GPIO_Pin_13, 	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOE, RCC_APB2Periph_GPIOE,	GPIO_Pin_14, 	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
};

static t_dcmotor_ctrl motor_z2_ctrl =
{
	DCMOTOR_DIR_FORWARD,
	0,
	10,
	0,
	DCMOTOR_ST_STOPED
};

static t_dcmotor_source motor_z2_source =
{
	NULL,
	&motor_x_pin[DCMOTOR_PIN_IN1],
	&motor_x_pin[DCMOTOR_PIN_IN2],
	&motor_z2_ctrl
};

static t_dcmotor_source * dcmotor_source_array[DCMOTOR_MAX] =
	{&motor_z2_source};

void DCMotor_Init(void)
{
	LibDcmotor_Init(&motor_z2_source);
}

void DCMotor_TimerHandler(t_dcmotor_no dcmotor_no)
{
	t_dcmotor_source * p_dcmotor_source;
	
	p_dcmotor_source = dcmotor_source_array[dcmotor_no];
	LibDcmotor_TimerHandler(p_dcmotor_source);
}

void DCMotor_Move(t_dcmotor_no dcmotor_no, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir)
{
	t_dcmotor_source * p_dcmotor_source;

	p_dcmotor_source = dcmotor_source_array[dcmotor_no];
	LibDcmotor_Move(p_dcmotor_source, steps, hold_steps, dir);
}

void DCMotor_StartMove(t_dcmotor_no dcmotor_no, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir)
{
	t_dcmotor_source * p_dcmotor_source;

	p_dcmotor_source = dcmotor_source_array[dcmotor_no];
	LibDcmotor_StartMove(p_dcmotor_source, steps, hold_steps, dir);
}

void DCMotor_WaitStop(t_dcmotor_no dcmotor_no)
{
	t_dcmotor_source * p_dcmotor_source;
	
	p_dcmotor_source = dcmotor_source_array[dcmotor_no];
	LibDcmotor_WaitStop(p_dcmotor_source);
}

void DCMotor_Stop(t_dcmotor_no dcmotor_no, unsigned int stop_steps)
{
	t_dcmotor_source * p_dcmotor_source;
	
	p_dcmotor_source = dcmotor_source_array[dcmotor_no];
	LibDcmotor_Stop(p_dcmotor_source, stop_steps);
}


