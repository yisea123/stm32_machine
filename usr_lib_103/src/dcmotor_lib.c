/*
  Filename: step_motor_basic.c
  Author: shidawei			Date: 20170415
  Copyright: 
  Description: STM3210X 直流电机
  				驱动Lib
 */
#include "lib_common.h"
#include "pin.h"
#include "dcmotor_lib.h"

extern void delay_ms(__IO uint32_t ms);

/* 放入1ms的定时器中断中 */
void LibDcmotor_TimerHandler(t_dcmotor_source * p_source)
{
	t_dcmotor_ctrl * p_dcmotor_ctrl;

	p_dcmotor_ctrl = p_source->dcmotor_ctrl;
	if(p_dcmotor_ctrl->step_cnt < p_dcmotor_ctrl->steps)	//运行
	{
		p_dcmotor_ctrl->step_cnt++;
		if(p_dcmotor_ctrl->step_cnt >= p_dcmotor_ctrl->steps)	//停止
		{
			//stoped
			Pin_WriteBit(p_source->pin_en, Bit_RESET);
			Pin_WriteBit(p_source->pin_in1, Bit_RESET);
			Pin_WriteBit(p_source->pin_in2, Bit_RESET);
			p_dcmotor_ctrl->status = DCMOTOR_ST_STOPED;
		}
		else if(p_dcmotor_ctrl->step_cnt == (p_dcmotor_ctrl->steps - p_dcmotor_ctrl->hold_steps))	//刹车
		{
			p_dcmotor_ctrl->status = DCMOTOR_ST_DEC;
			Pin_WriteBit(p_source->pin_in1, Bit_SET);
			Pin_WriteBit(p_source->pin_in2, Bit_SET);
		}
	}
	else
	{
		if(p_dcmotor_ctrl->status != DCMOTOR_ST_STOPED)		//停止
		{
			//stoped
			Pin_WriteBit(p_source->pin_en, Bit_RESET);
			Pin_WriteBit(p_source->pin_in1, Bit_RESET);
			Pin_WriteBit(p_source->pin_in2, Bit_RESET);
			p_dcmotor_ctrl->status = DCMOTOR_ST_STOPED;
		}
	}
}

void LibDcmotor_Init(t_dcmotor_source * p_source)
{
	t_dcmotor_ctrl * p_dcmotor_ctrl;

	p_dcmotor_ctrl = p_source->dcmotor_ctrl;

	//gpio config
	Pin_Init(p_source->pin_en);
	Pin_Init(p_source->pin_in1);
	Pin_Init(p_source->pin_in2);
	p_dcmotor_ctrl->dir = DCMOTOR_DIR_FORWARD;
	p_dcmotor_ctrl->hold_steps = 0;
	p_dcmotor_ctrl->status = DCMOTOR_ST_STOPED;
	p_dcmotor_ctrl->steps = 0;
	p_dcmotor_ctrl->step_cnt = 0;
}

/*
 * 直流电机运行函数，运行停止后返回
 * steps: 运行距离，以运行时间为距离单位ms
 * hold_steps: 强制刹车距离
 * dir:   运行方向
*/
void LibDcmotor_Move(t_dcmotor_source * p_source, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir)
{
#if 0
	LibDcmotor_StartMove(p_source, steps, hold_steps, dir);
	LibDcmotor_WaitStop(p_source);
#else
	t_dcmotor_ctrl * p_dcmotor_ctrl;

	p_dcmotor_ctrl = p_source->dcmotor_ctrl;
	p_dcmotor_ctrl->steps = steps;
	p_dcmotor_ctrl->step_cnt = 0;
	p_dcmotor_ctrl->status = DCMOTOR_ST_RUN;
	p_dcmotor_ctrl->hold_steps = hold_steps;
	if(dir)
	{
		Pin_WriteBit(p_source->pin_in1, Bit_SET);
		Pin_WriteBit(p_source->pin_in2, Bit_RESET);
	}
	else
	{
		Pin_WriteBit(p_source->pin_in1, Bit_RESET);
		Pin_WriteBit(p_source->pin_in2, Bit_SET);
	}
	Pin_WriteBit(p_source->pin_en, Bit_SET);
	//等待停止，因为有强制刹车距离，所以这里需要减去
	while(p_dcmotor_ctrl->step_cnt <= (p_dcmotor_ctrl->steps - p_dcmotor_ctrl->hold_steps))
	{
		p_dcmotor_ctrl->step_cnt++;
		delay_ms(1);
	}
	//强制刹车
	p_dcmotor_ctrl->status = DCMOTOR_ST_DEC;
	Pin_WriteBit(p_source->pin_in1, Bit_SET);
	Pin_WriteBit(p_source->pin_in2, Bit_SET);
	while(p_dcmotor_ctrl->step_cnt <= p_dcmotor_ctrl->steps)
	{
		p_dcmotor_ctrl->step_cnt++;
		delay_ms(1);
	}
	//stoped
	Pin_WriteBit(p_source->pin_en, Bit_RESET);
	Pin_WriteBit(p_source->pin_in1, Bit_RESET);
	Pin_WriteBit(p_source->pin_in2, Bit_RESET);
	p_dcmotor_ctrl->status = DCMOTOR_ST_STOPED;
#endif
}

/*
 * 直流电机运行函数，直接返回，需调用wait_stop停止
 * steps: 运行距离，以运行时间为距离单位ms，运行时间不包含启动后做出的其余动作
 * hold_steps: 强制刹车距离
 * dir:   运行方向
*/
void LibDcmotor_StartMove(t_dcmotor_source * p_source, unsigned int steps, unsigned int hold_steps, t_dcmotor_dir dir)
{
	t_dcmotor_ctrl * p_dcmotor_ctrl;

	p_dcmotor_ctrl = p_source->dcmotor_ctrl;
	p_dcmotor_ctrl->steps = steps;
	p_dcmotor_ctrl->step_cnt = 0;
	p_dcmotor_ctrl->status = DCMOTOR_ST_RUN;
	p_dcmotor_ctrl->hold_steps = hold_steps;
	if(dir)
	{
		Pin_WriteBit(p_source->pin_in1, Bit_SET);
		Pin_WriteBit(p_source->pin_in2, Bit_RESET);
	}
	else
	{
		Pin_WriteBit(p_source->pin_in1, Bit_RESET);
		Pin_WriteBit(p_source->pin_in2, Bit_SET);
	}
	Pin_WriteBit(p_source->pin_en, Bit_SET);
}

/*
 * 直流电机等待停止函数
*/
void LibDcmotor_WaitStop(t_dcmotor_source * p_source)
{
	t_dcmotor_ctrl * p_dcmotor_ctrl;

	p_dcmotor_ctrl = p_source->dcmotor_ctrl;
	while(p_dcmotor_ctrl->step_cnt < p_dcmotor_ctrl->steps);
	//stoped
	Pin_WriteBit(p_source->pin_en, Bit_RESET);
	Pin_WriteBit(p_source->pin_in1, Bit_RESET);
	Pin_WriteBit(p_source->pin_in2, Bit_RESET);
	p_dcmotor_ctrl->status = DCMOTOR_ST_STOPED;
}

/*
 * 直流电机停止函数
 * 由于直流电机等待停止，所以此函数用于中断函数中使用
*/
void LibDcmotor_Stop(t_dcmotor_source * p_source, unsigned int stop_steps)
{
	t_dcmotor_ctrl * p_dcmotor_ctrl;

	p_dcmotor_ctrl = p_source->dcmotor_ctrl;
	//还没有进入减速区
	if(p_dcmotor_ctrl->step_cnt < p_dcmotor_ctrl->steps - stop_steps)
		p_dcmotor_ctrl->step_cnt = p_dcmotor_ctrl->steps - stop_steps;
}

