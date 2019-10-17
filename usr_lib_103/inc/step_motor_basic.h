/*
  Filename: step_motor_basic.c
  Author: shidawei			Date: 20140116
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: STM3210X 电机芯片A4982SLP or L6219
  				驱动Lib
 */
#ifndef __STEP_MOTOR_BASIC_H__
#define __STEP_MOTOR_BASIC_H__
#include "lib_common.h"
#include "pin.h"

//#define L6219			//使用L6219电机芯片
#define MOTOR_LOW_LEVEL_ENABLE	1	//使用低电平作为使能信号如8825，高电平使能需置0如8880


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
	MOTOR_ST_INIT,		//初始化状态
	MOTOR_ST_HOLD,		//Holding
	MOTOR_ST_ACC,		//加速
	MOTOR_ST_CONST,		//匀速
	MOTOR_ST_DEC,		//减速
	MOTOR_ST_POSIT		//停止安置
}t_motor_st;

typedef struct
{
	t_motor_dir			dir;				//direction
	t_motor_st			status;				//status
	unsigned int		step;				//电机运动步数
	unsigned int		step_cnt;			//电机已经走过的步数
#ifdef L6219
	const uint8_t *		pulse_val_base;		//步序列值常量指针，初始化为NULL，在使用时根据模式将改变
	uint8_t 			pulse_val_len;		//步序列值指针数组长度，初始化为0，在使用时将改变
	uint8_t				pulse_index;		//当前指定到不序列值索引
#else
	BitAction 			pulse_val;			//脉冲电平
#endif
	uint16_t    acc_ramp_max_size;	//加速表最大长度
	uint16_t    dec_ramp_max_size;	//减速表最大长度
	uint16_t    acc_ramp_rt_size;	//加速表实时长度
	uint16_t    dec_ramp_rt_size;	//减速表实时长度

	const uint16_t *	ptime_ramp;			//指向当前加速时间
	const uint16_t *	ptime_ramp_tbl;		//指向当前表的指针
	uint16_t	time_tbl_size;		//当前表大小
}t_step_motor_ctrl;

typedef struct
{
	t_motor_mode   mode;
	const uint16_t * ptime_acc_ramp_tbl;	//指向加速表的指针，加速表必须以00结束，用于计算表长度
	const uint16_t * ptime_dec_ramp_tbl;	//指向减速表的指针，减速表必须以00结束，用于计算表长度
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
	t_pin_source * motor_en_pin;//电机选通信号
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


