/*
  Filename: step_motor_basic.c
  Author: shidawei			Date: 20170415
  Copyright: 
  Description: STM3210X ֱ�����
  				����Lib
 */
#include "lib_common.h"
#include "pin.h"
#include "dcmotor_lib.h"

extern void delay_ms(__IO uint32_t ms);

/* ����1ms�Ķ�ʱ���ж��� */
void LibDcmotor_TimerHandler(t_dcmotor_source * p_source)
{
	t_dcmotor_ctrl * p_dcmotor_ctrl;

	p_dcmotor_ctrl = p_source->dcmotor_ctrl;
	if(p_dcmotor_ctrl->step_cnt < p_dcmotor_ctrl->steps)	//����
	{
		p_dcmotor_ctrl->step_cnt++;
		if(p_dcmotor_ctrl->step_cnt >= p_dcmotor_ctrl->steps)	//ֹͣ
		{
			//stoped
			Pin_WriteBit(p_source->pin_en, Bit_RESET);
			Pin_WriteBit(p_source->pin_in1, Bit_RESET);
			Pin_WriteBit(p_source->pin_in2, Bit_RESET);
			p_dcmotor_ctrl->status = DCMOTOR_ST_STOPED;
		}
		else if(p_dcmotor_ctrl->step_cnt == (p_dcmotor_ctrl->steps - p_dcmotor_ctrl->hold_steps))	//ɲ��
		{
			p_dcmotor_ctrl->status = DCMOTOR_ST_DEC;
			Pin_WriteBit(p_source->pin_in1, Bit_SET);
			Pin_WriteBit(p_source->pin_in2, Bit_SET);
		}
	}
	else
	{
		if(p_dcmotor_ctrl->status != DCMOTOR_ST_STOPED)		//ֹͣ
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
 * ֱ��������к���������ֹͣ�󷵻�
 * steps: ���о��룬������ʱ��Ϊ���뵥λms
 * hold_steps: ǿ��ɲ������
 * dir:   ���з���
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
	//�ȴ�ֹͣ����Ϊ��ǿ��ɲ�����룬����������Ҫ��ȥ
	while(p_dcmotor_ctrl->step_cnt <= (p_dcmotor_ctrl->steps - p_dcmotor_ctrl->hold_steps))
	{
		p_dcmotor_ctrl->step_cnt++;
		delay_ms(1);
	}
	//ǿ��ɲ��
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
 * ֱ��������к�����ֱ�ӷ��أ������wait_stopֹͣ
 * steps: ���о��룬������ʱ��Ϊ���뵥λms������ʱ�䲻�������������������ද��
 * hold_steps: ǿ��ɲ������
 * dir:   ���з���
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
 * ֱ������ȴ�ֹͣ����
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
 * ֱ�����ֹͣ����
 * ����ֱ������ȴ�ֹͣ�����Դ˺��������жϺ�����ʹ��
*/
void LibDcmotor_Stop(t_dcmotor_source * p_source, unsigned int stop_steps)
{
	t_dcmotor_ctrl * p_dcmotor_ctrl;

	p_dcmotor_ctrl = p_source->dcmotor_ctrl;
	//��û�н��������
	if(p_dcmotor_ctrl->step_cnt < p_dcmotor_ctrl->steps - stop_steps)
		p_dcmotor_ctrl->step_cnt = p_dcmotor_ctrl->steps - stop_steps;
}

