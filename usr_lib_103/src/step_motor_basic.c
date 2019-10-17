/*
  Filename: step_motor_basic.c
  Author: shidawei			Date: 20140714
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: STM3210X ���оƬA4982SLP
  				����Lib
 */
#include "step_motor_basic.h"

#define MOTOR_DEBUG			0

#ifdef L6219
/*
1.	L6219 Input Current Table Definition(I_L=0, I_H=1)
	
		I_0 			I_1 		Output
	-------------------------------------	
		I_L 			I_L 		(3/3)Io
		I_H 			I_L 		(2/3)Io
		I_L 			I_H 		(1/3)Io
		I_H 			I_H 		----
2. 	Motor control pins mapping of GPIOx
              I1(0,1)        |            |           I2(0,1)       |            |
    -------------------------|   PhaseA   |-------------------------|   PhaseB   |
       I_0_1    |   I_1_1    |            |   I_0_2    |   I_1_2    |            |
    ------------|------------|------------|------------|------------|------------|
                |            |            |            |            |            |
                |            |            |            |            |            |

3. L6219 Input
                |           |            |            |            |            |
       I01         I11          PH1          I02          I12          PH2        VAL
 1) Init
       1           1            0            1             1            0         
 2) Holding
       0           1            0            0             1            1         
 3)full
   F1  0           0            0            0             0            0         
   F2  0           0            1            0             0            0         
   F3  0           0            1            0             0            1         
   F4  0           0            0            0             0            1         
 4)half
   H1  1           1            1(0)         1             0            1         
   H2  1           0            1            1             0            1         
   H3  1           0            1            1             1            1(0)      
   H4  1           0            1            1             0            0         
   H5  1           1            1(0)         1             0            0         
   H6  1           0            0            1             0            0         
   H7  1           0            0            1             1            1(0)      
   H8  1           0            0            1             0            1         
*/
const uint8_t l6219_init_val = 0x36;
const uint8_t l6219_hold_val = 0x13;
const uint8_t l6219_full_step_val_seq[4] =
{
	0x00, 0x08, 0x09, 0x01
};
const uint8_t l6219_half_step_val_seq[8] =
{
	0x3D, 0x2D, 0x2F, 0x2A, 0x3A, 0x24, 0x27, 0x25
};
const uint8_t l6219_quarter_step_val_seq[16] =
{
	0x18, 0x38, 0x10, 0x24, 0x02, 0x06, 0x03, 0x25, 0x11, 0x31, 0x19, 0x2D, 0x0B, 0x0F, 0x0A, 0x2C
};
#endif

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  ��ñ���
  * @param  
  * @retval 
  */
static uint16_t StepMotor_GetTblSize(const uint16_t * p_tbl)
{
	const uint16_t * p_data;
	uint16_t len = 0;
	
	p_data = p_tbl;
	while(*p_data)
	{
		len++;
		p_data++;
	}
	return len;
}

/**
  * @brief  motor timer init
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
static void StepMotor_TimerInit(t_step_motor_source * p_step_motor_source)
{
	t_timer_source * p_timer_source;

	p_timer_source = p_step_motor_source->timer_source;
	
	TIM_DeInit(p_timer_source->timer);
	/* TIMn Periph clock enable */
	RCC_APB1PeriphClockCmd(p_timer_source->rcc_Periph, ENABLE);
	/* Time base configuration */
	TIM_TimeBaseInit(p_timer_source->timer, p_timer_source->TIM_TimeBaseStructure);

	/* Clear TIMn update pending flag */
	TIM_ClearFlag(p_timer_source->timer, TIM_FLAG_Update);

	/* Configure two bits for preemption priority */
	NVIC_PriorityGroupConfig(p_timer_source->NVIC_PriorityGroup);

	/* Enable the TIMn Interrupt */
	NVIC_Init(p_timer_source->NVIC_InitStructure);

	/* Enable TIMn	Update interrupt */
	TIM_ITConfig(p_timer_source->timer, TIM_IT_Update, ENABLE);
	/* auto reload */
	TIM_ARRPreloadConfig(p_timer_source->timer, ENABLE);
	 /* TIMn enable counter */
	TIM_Cmd(p_timer_source->timer, DISABLE);
}

#ifdef L6219
/**
  * @brief  L6219����
  * @param  ctrl_data: ���͵Ŀ������ݣ����尴λ��ʾ����
  					   BIT0: I01
  					   BIT1: I11
  					   BIT2: PhaseA or Ph1
  					   BIT3: I02
  					   BIT4: I12
  					   BIT5: PhaseB or Ph2
  * @retval 
  */
static void StepMotor_L6219Ctrl(t_step_motor_source * p_step_motor_source, uint8_t ctrl_data)
{
	//I01
	if(ctrl_data & (1<<5))
	{
		Pin_WriteBit(p_step_motor_source->pin_i01, Bit_SET);
	}
	else
	{
		Pin_WriteBit(p_step_motor_source->pin_i01, Bit_RESET);
	}
	//I11
	if(ctrl_data & (1<<4))
	{
		Pin_WriteBit(p_step_motor_source->pin_i11, Bit_SET);
	}
	else
	{
		Pin_WriteBit(p_step_motor_source->pin_i11, Bit_RESET);
	}
	//PhaseA or Ph1
	if(ctrl_data & (1<<3))
	{
		Pin_WriteBit(p_step_motor_source->pin_ph1, Bit_SET);
	}
	else
	{
		Pin_WriteBit(p_step_motor_source->pin_ph1, Bit_RESET);
	}
	//I02
	if(ctrl_data & (1<<2))
	{
		Pin_WriteBit(p_step_motor_source->pin_i02, Bit_SET);
	}
	else
	{
		Pin_WriteBit(p_step_motor_source->pin_i02, Bit_RESET);
	}
	//I12
	if(ctrl_data & (1<<1))
	{
		Pin_WriteBit(p_step_motor_source->pin_i12, Bit_SET);
	}
	else
	{
		Pin_WriteBit(p_step_motor_source->pin_i12, Bit_RESET);
	}
	//PhaseB or Phase2
	if(ctrl_data & (1<<0))
	{
		Pin_WriteBit(p_step_motor_source->pin_ph2, Bit_SET);
	}
	else
	{
		Pin_WriteBit(p_step_motor_source->pin_ph2, Bit_RESET);
	}
}

/**
  * @brief  motor dirve functions
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
static void StepMotor_Drive(t_step_motor_source * p_step_motor_source)
{
	t_step_motor_ctrl * p_step_motor_ctrl;
	const uint8_t * pulse_val = &l6219_init_val;

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	switch(p_step_motor_ctrl->status)
	{
		case MOTOR_ST_INIT:
			pulse_val = &l6219_init_val;
			break;
		case MOTOR_ST_HOLD:
			pulse_val = &l6219_hold_val;
			break;
		case MOTOR_ST_ACC:
		case MOTOR_ST_CONST:
		case MOTOR_ST_DEC:
		case MOTOR_ST_POSIT:
			if(p_step_motor_ctrl->dir == MOTOR_DIR_FORWARD)
			{
				p_step_motor_ctrl->pulse_index = 
					(p_step_motor_ctrl->pulse_index + 1) % p_step_motor_ctrl->pulse_val_len;
			}
			else
			{
				p_step_motor_ctrl->pulse_index = 
					(p_step_motor_ctrl->pulse_index + p_step_motor_ctrl->pulse_val_len - 1)
					% p_step_motor_ctrl->pulse_val_len;
			}
			pulse_val = p_step_motor_ctrl->pulse_val_base + p_step_motor_ctrl->pulse_index;
			break;
		default:
			pulse_val = &l6219_init_val;
			break;
	}
	StepMotor_L6219Ctrl(p_step_motor_source, (*pulse_val));
}
#else
/**
  * @brief  motor dirve functions
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
static void StepMotor_Drive(t_step_motor_source * p_step_motor_source)
{
	t_step_motor_ctrl * p_step_motor_ctrl;

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	switch(p_step_motor_ctrl->status)
	{
		case MOTOR_ST_INIT:
			p_step_motor_source->step_motor_ctrl->pulse_val = Bit_RESET;
            //p_motor_ctrl->drive_tbl_index = 0;
			break;
		case MOTOR_ST_HOLD:
			p_step_motor_source->step_motor_ctrl->pulse_val = Bit_SET;
			break;
		case MOTOR_ST_ACC:
		case MOTOR_ST_CONST:
		case MOTOR_ST_DEC:
		case MOTOR_ST_POSIT:
			p_step_motor_source->step_motor_ctrl->pulse_val = 
				(BitAction)!p_step_motor_source->step_motor_ctrl->pulse_val;
			break;
		default:
			return;
	}
#if !MOTOR_DEBUG
	Pin_WriteBit(p_step_motor_source->pin_pulse, p_step_motor_source->step_motor_ctrl->pulse_val);
#else
	if(p_step_motor_source->step_motor_ctrl->pulse_val)
	{
		Com_SendCh(COM_MAIN, (((*p_step_motor_source->step_motor_ctrl->ptime_ramp) / 1000) %10 + '0'));
		Com_SendCh(COM_MAIN, (((*p_step_motor_source->step_motor_ctrl->ptime_ramp) / 100) %10+ '0'));
		Com_SendCh(COM_MAIN, (((*p_step_motor_source->step_motor_ctrl->ptime_ramp) / 10) %10+ '0'));
		Com_SendCh(COM_MAIN, (((*p_step_motor_source->step_motor_ctrl->ptime_ramp) / 1) %10+ '0'));
		Com_SendCh(COM_MAIN, '	');
	}
#endif
}
#endif

/**
  * @brief  motor move init. it's must to be called before or after motion
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */

static void StepMotor_Choice(t_step_motor_source * p_step_motor_source,FunctionalState enable)
{
	if(p_step_motor_source->motor_en_pin==NULL)
		return;
	if(enable)
		Pin_WriteBit(p_step_motor_source->motor_en_pin, Bit_RESET);
	else
		Pin_WriteBit(p_step_motor_source->motor_en_pin, Bit_SET);
}
static void StepMotor_MoveInit(t_step_motor_source * p_step_motor_source)
{
	t_step_motor_ctrl * p_step_motor_ctrl;

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	p_step_motor_ctrl->step = 0;
	p_step_motor_ctrl->step_cnt = 0;
	p_step_motor_ctrl->status = MOTOR_ST_INIT;
	p_step_motor_ctrl->ptime_ramp = p_step_motor_source->ptime_acc_ramp_tbl;
	StepMotor_Drive(p_step_motor_source);
}

/**
  * @brief  motor set timer
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
void StepMotor_SetTimer(t_step_motor_source * p_step_motor_source)
{
	uint16_t time;

	time = *p_step_motor_source->step_motor_ctrl->ptime_ramp;
#ifdef L6219
	switch(p_step_motor_source->mode)
	{
		case MOTOR_MODE_FULL:
			break;
		case MOTOR_MODE_HALF:
			time = (time>>1);	//time/2
			break;
		case MOTOR_MODE_QUARTER:
			time = (time>>2);	//time/4
			break;
		case MOTOR_MODE_SIXTEEN:
			time = (time>>4);	//time/16
			break;
	}
#else
	switch(p_step_motor_source->mode)	//A4982Ϊ���ش�����ʱ���С��һ��
	{
		case MOTOR_MODE_FULL:
			time = (time>>1);	//time/2
			break;
		case MOTOR_MODE_HALF:
			time = (time>>2);	//time/4
			break;
		case MOTOR_MODE_QUARTER:
			time = (time>>3);	//time/8
			break;
		case MOTOR_MODE_SIXTEEN:
			time = (time>>5);	//time/32
			break;
	}
#endif
	if((time > 8000) || (time == 0))	//avoid chips been broken
	{
		TIM_Cmd(p_step_motor_source->timer_source->timer, DISABLE);
		StepMotor_MoveInit(p_step_motor_source);
	}
	else
	{
#if MOTOR_DEBUG
		time *= 160;	//����ģʽ�Ŵ�10��ʱ�䣬�������������Ϣ
#endif
		TIM_SetAutoreload(p_step_motor_source->timer_source->timer, time);
	}
}

/**
  * @brief  �������ʹ��
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
static void StepMotor_Enable(t_step_motor_source * p_step_motor_source, FunctionalState enable)
{
#ifdef L6219
#else
	if(p_step_motor_source->pin_en != NULL)
	{
		if(enable)
		{
#if MOTOR_LOW_LEVEL_ENABLE
			Pin_WriteBit(p_step_motor_source->pin_en, Bit_RESET);
			Pin_WriteBit(p_step_motor_source->pin_sleep, Bit_SET);
#else
			Pin_WriteBit(p_step_motor_source->pin_en, Bit_SET);
			Pin_WriteBit(p_step_motor_source->pin_sleep, Bit_RESET);
#endif
		}
		else
		{
#if MOTOR_LOW_LEVEL_ENABLE
			Pin_WriteBit(p_step_motor_source->pin_sleep, Bit_RESET);
			Pin_WriteBit(p_step_motor_source->pin_en, Bit_SET);
#else
			Pin_WriteBit(p_step_motor_source->pin_sleep, Bit_SET);
			Pin_WriteBit(p_step_motor_source->pin_en, Bit_RESET);
#endif
		}
	}
#endif
}

/**
  * @brief  ���ݲ�������˶�ģʽ����ģʽ����
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
static void StepMotor_Mode(t_step_motor_source * p_step_motor_source)
{
#ifdef L6219
	t_step_motor_ctrl * p_step_motor_ctrl;

	//data init
	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	switch(p_step_motor_source->mode)
	{
		case MOTOR_MODE_FULL:
			p_step_motor_ctrl->pulse_val_base = l6219_full_step_val_seq;	
			p_step_motor_ctrl->pulse_val_len = sizeof(l6219_full_step_val_seq);
			p_step_motor_ctrl->pulse_index = 0;
			break;
		case MOTOR_MODE_HALF:
			p_step_motor_ctrl->pulse_val_base = l6219_half_step_val_seq;	
			p_step_motor_ctrl->pulse_val_len = sizeof(l6219_half_step_val_seq);
			p_step_motor_ctrl->pulse_index = 0;
			break;
		case MOTOR_MODE_QUARTER:
			p_step_motor_ctrl->pulse_val_base = l6219_quarter_step_val_seq;	
			p_step_motor_ctrl->pulse_val_len = sizeof(l6219_quarter_step_val_seq);
			p_step_motor_ctrl->pulse_index = 0;
			break;
		default:
			p_step_motor_ctrl->pulse_val_base = l6219_full_step_val_seq;	
			p_step_motor_ctrl->pulse_val_len = sizeof(l6219_full_step_val_seq);
			p_step_motor_ctrl->pulse_index = 0;
			break;
	}
#else
#ifdef STEP_MOTOR_MODE
	t_pin_source * p_pin_ms1;
	t_pin_source * p_pin_ms2;

	p_pin_ms1 = p_step_motor_source->pin_ms1;
	p_pin_ms2 = p_step_motor_source->pin_ms2;
	switch(p_step_motor_source->mode)
	{
		case MOTOR_MODE_FULL:
			Pin_WriteBit(p_pin_ms1, Bit_RESET);
			Pin_WriteBit(p_pin_ms2, Bit_RESET);
			break;
		case MOTOR_MODE_HALF:
			Pin_WriteBit(p_pin_ms1, Bit_SET);
			Pin_WriteBit(p_pin_ms2, Bit_RESET);
			break;
		case MOTOR_MODE_QUARTER:
			Pin_WriteBit(p_pin_ms1, Bit_RESET);
			Pin_WriteBit(p_pin_ms2, Bit_SET);
			break;
		case MOTOR_MODE_SIXTEEN:
			Pin_WriteBit(p_pin_ms1, Bit_SET);
			Pin_WriteBit(p_pin_ms2, Bit_SET);
			break;
		default:
			Pin_WriteBit(p_pin_ms1, Bit_RESET);
			Pin_WriteBit(p_pin_ms2, Bit_RESET);
			break;
	}
#endif
#endif
}

/**
  * @brief  ���ò�������˶�����
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
static void StepMotor_Dir(t_step_motor_source * p_step_motor_source, t_motor_dir dir)
{
	p_step_motor_source->step_motor_ctrl->dir = dir;
#ifndef L6219
	if(p_step_motor_source->step_motor_ctrl->dir == MOTOR_DIR_FORWARD)
	{
//		Pin_WriteBit(p_step_motor_source->pin_dir, Bit_SET);
		Pin_WriteBit(p_step_motor_source->pin_dir, Bit_RESET);//8880�޸�
	}
	else
	{
//		Pin_WriteBit(p_step_motor_source->pin_dir, Bit_RESET);
		Pin_WriteBit(p_step_motor_source->pin_dir, Bit_SET);//8880�޸�
	}
#endif
}

/****************************************************************
 GLOBAL FUNCTIONS
 ****************************************************************/
/**
  * @brief  motor Init
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
void StepMotor_Init(t_step_motor_source * p_step_motor_source)
{
	t_step_motor_ctrl * p_step_motor_ctrl;

	//data init
	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	
	//gpio cofig
#ifdef L6219
	Pin_Init(p_step_motor_source->pin_i01);
	Pin_Init(p_step_motor_source->pin_i02);
	Pin_Init(p_step_motor_source->pin_ph1);
	Pin_Init(p_step_motor_source->pin_i11);
	Pin_Init(p_step_motor_source->pin_i12);
	Pin_Init(p_step_motor_source->pin_ph2);
#else
	Pin_Init(p_step_motor_source->pin_en);
	Pin_Init(p_step_motor_source->pin_sleep);
	Pin_Init(p_step_motor_source->pin_pulse);
	Pin_Init(p_step_motor_source->pin_dir);
#ifdef STEP_MOTOR_MODE
	Pin_Init(p_step_motor_source->pin_ms1);
	Pin_Init(p_step_motor_source->pin_ms2);
#endif
	if(p_step_motor_source->motor_en_pin!=NULL)
	{
		Pin_Init(p_step_motor_source->motor_en_pin);
		Pin_WriteBit(p_step_motor_source->motor_en_pin,Bit_SET);
	}
#endif
	//motor timer init
	StepMotor_TimerInit(p_step_motor_source);
	//motor structure initialization
	p_step_motor_ctrl->dir = MOTOR_DIR_FORWARD;
	p_step_motor_ctrl->status = MOTOR_ST_INIT;
	p_step_motor_ctrl->step = 0;
	p_step_motor_ctrl->step_cnt = 0;
#ifndef L6219
	p_step_motor_ctrl->pulse_val = Bit_RESET;
#endif
	p_step_motor_ctrl->acc_ramp_max_size = StepMotor_GetTblSize(p_step_motor_source->ptime_acc_ramp_tbl);
	p_step_motor_ctrl->dec_ramp_max_size = StepMotor_GetTblSize(p_step_motor_source->ptime_dec_ramp_tbl);
	p_step_motor_ctrl->acc_ramp_rt_size = p_step_motor_ctrl->acc_ramp_max_size;
	p_step_motor_ctrl->dec_ramp_rt_size = p_step_motor_ctrl->dec_ramp_max_size;
	p_step_motor_ctrl->ptime_ramp_tbl = p_step_motor_source->ptime_acc_ramp_tbl;
	p_step_motor_ctrl->ptime_ramp = p_step_motor_ctrl->ptime_ramp_tbl;
	p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->acc_ramp_rt_size;
}

/**
  * @brief  ����˶�����
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
            step: ����˶�����(ȫ��)
            micro_step: ����˶�΢������(����΢�ֽ��м���Ĳ���)
            ���ղ�����ȫ����΢����������
            dir: ����˶�����
            speed: ����˶��ٶȣ���ramp����ȡʱ��ֵ��ʾ
  * @retval 
  */
void StepMotor_Move(t_step_motor_source * p_step_motor_source, unsigned int step, unsigned int micro_step,
					t_motor_dir dir, uint16_t speed)
{
	t_step_motor_ctrl * p_step_motor_ctrl;
	TIM_TypeDef * timer;

	switch(p_step_motor_source->mode)
	{
		case MOTOR_MODE_FULL:
			step = step * 1;
			break;
		case MOTOR_MODE_HALF:
			step = step * 2;
			break;
		case MOTOR_MODE_QUARTER:
			step = step * 4;
			break;
		case MOTOR_MODE_SIXTEEN:
			step = step * 16;
			break;
	}
	step += micro_step;		//����΢�ֲ���
	if(step == 0)
		return;
	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	p_step_motor_ctrl->ptime_ramp_tbl = p_step_motor_source->ptime_acc_ramp_tbl;
	p_step_motor_ctrl->ptime_ramp = p_step_motor_ctrl->ptime_ramp_tbl;
	if(step < p_step_motor_ctrl->acc_ramp_max_size)	//����С���������ٱ���
	{
		speed = p_step_motor_ctrl->ptime_ramp_tbl[(step/2) - 2];
	}
	StepMotor_MoveInit(p_step_motor_source);	//�˺����ڻὫstep��ֵ0����step�ڴ�֮�����¸�ֵ
	timer = p_step_motor_source->timer_source->timer;
	StepMotor_Dir(p_step_motor_source, dir);
	StepMotor_Speed(p_step_motor_source, speed);
	p_step_motor_ctrl->step = step;
	p_step_motor_ctrl->step_cnt = 0;
	p_step_motor_ctrl->status = MOTOR_ST_ACC;
	p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->acc_ramp_rt_size;
	p_step_motor_ctrl->ptime_ramp = p_step_motor_ctrl->ptime_ramp_tbl;
	StepMotor_Choice(p_step_motor_source,ENABLE);
	StepMotor_Mode(p_step_motor_source);
	StepMotor_Enable(p_step_motor_source, ENABLE);
	TIM_SetAutoreload(timer, *p_step_motor_ctrl->ptime_ramp);
	TIM_Cmd(timer, ENABLE);
}

/**
  * @brief  ���ֹͣ�˶�
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
            step: ���ֹͣ�˶�ǰ��Ҫ�˶�����(ȫ��)
            micro_step: ���ֹͣ�˶�ǰ��Ҫ�˶�΢������(����΢�ֽ��м���Ĳ���)
            ���ղ����ɼ�΢����������
  * @retval 
  */
void StepMotor_Stop(t_step_motor_source * p_step_motor_source, unsigned int stop_step,
	unsigned int micro_stop_step)
{
	t_step_motor_ctrl * p_step_motor_ctrl;
	uint16_t size_ramp_tbl;		//size of time ramp table
	uint16_t dist_stop_ramp;		//distance to stop

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	size_ramp_tbl = p_step_motor_ctrl->dec_ramp_rt_size;//p_step_motor_ctrl->time_tbl_size;//xj add
	switch(p_step_motor_source->mode)
	{
		case MOTOR_MODE_FULL:
			stop_step = stop_step;
			break;
		case MOTOR_MODE_HALF:
			stop_step = stop_step * 2;
			break;
		case MOTOR_MODE_QUARTER:
			stop_step = stop_step * 4;
			break;
		case MOTOR_MODE_SIXTEEN:
			stop_step = stop_step * 16;
			break;
	}
	stop_step += micro_stop_step;		//����΢�ֲ���
	if(stop_step < size_ramp_tbl)
	{
		dist_stop_ramp = size_ramp_tbl + stop_step;//xj add
	}
	else
	{
		dist_stop_ramp = stop_step;
	}
	//if it didn't slow down, slow down to stop
	if(p_step_motor_ctrl->step_cnt < (p_step_motor_ctrl->step - dist_stop_ramp))
	{
		p_step_motor_ctrl->step = p_step_motor_ctrl->step_cnt + dist_stop_ramp;
	}
}

/**
  * @brief  ��������ȴ�ֹͣ
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
void StepMotor_WaitStoped(t_step_motor_source * p_step_motor_source)
{
	t_step_motor_ctrl * p_step_motor_ctrl;

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	//wait for motor stoped
	while(p_step_motor_ctrl->status != MOTOR_ST_INIT);
}

/**
  * @brief  ��鲽������Ƿ��Ѿ�ֹͣ
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 1: ��ʾ�Ѿ�ֹͣ
  *         0: ��ʾ��������
  */
uint8_t StepMotor_IsStoped(t_step_motor_source * p_step_motor_source)
{
	t_step_motor_ctrl * p_step_motor_ctrl;

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	return ((p_step_motor_ctrl->status == MOTOR_ST_INIT) ? 1 : 0);
}

/**
  * @brief  ���������������
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
void StepMotor_Hold(t_step_motor_source * p_step_motor_source, FunctionalState enable)
{
#ifdef L6219
	t_step_motor_ctrl * p_step_motor_ctrl;

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	p_step_motor_ctrl->status = MOTOR_ST_HOLD;
	StepMotor_Drive(p_step_motor_source);
#else
	StepMotor_Choice(p_step_motor_source, enable);
	StepMotor_Enable(p_step_motor_source, enable);
#endif
}

/**
  * @brief  ��������жϺ���
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  * @retval 
  */
void StepMotor_IntHandler(t_step_motor_source * p_step_motor_source)
{
	const uint16_t * p_const_ramp;	//pulse time
	const uint16_t * p_stop_ramp;	//stop pulse time
	uint16_t dist_stop_ramp;	//distance to stop
	t_step_motor_ctrl * p_step_motor_ctrl;

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	StepMotor_Drive(p_step_motor_source);
#ifndef L6219
	if((p_step_motor_ctrl->pulse_val) && (p_step_motor_ctrl->status != MOTOR_ST_HOLD) &&
		(p_step_motor_ctrl->status != MOTOR_ST_INIT))	//A4982���������߲�
	{
		return;
	}
#endif

	p_step_motor_ctrl->step_cnt++;
	if(p_step_motor_ctrl->step_cnt >= p_step_motor_ctrl->step)
	{
		p_step_motor_ctrl->status = MOTOR_ST_POSIT;
	}
	//get acc and dec steps
	p_const_ramp = p_step_motor_ctrl->ptime_ramp_tbl + p_step_motor_ctrl->time_tbl_size - 1;	//��������rampָ��
	p_stop_ramp = p_step_motor_source->ptime_dec_ramp_tbl + p_step_motor_ctrl->dec_ramp_max_size - 1;;	//���ٵ�ֹͣrampָ��
	dist_stop_ramp = p_step_motor_ctrl->dec_ramp_rt_size;	//������Ҫ�ľ���
	//set the pulse
	switch(p_step_motor_ctrl->status)
	{
		case MOTOR_ST_ACC:
			StepMotor_SetTimer(p_step_motor_source);
			if(p_step_motor_ctrl->ptime_ramp < p_const_ramp)
			{
				p_step_motor_ctrl->ptime_ramp++;
			}
			else
			{
				p_step_motor_ctrl->ptime_ramp = p_stop_ramp;
				p_step_motor_ctrl->status = MOTOR_ST_POSIT;
				break;
			}
			if(p_step_motor_ctrl->ptime_ramp == p_const_ramp)
			{
				p_step_motor_ctrl->status = MOTOR_ST_CONST;
			}
			break;
		case MOTOR_ST_CONST:
			if(p_step_motor_ctrl->step_cnt >= (p_step_motor_ctrl->step - dist_stop_ramp))
			{
				p_step_motor_ctrl->status = MOTOR_ST_DEC;
				//����ָ���л������ٱ�
				p_step_motor_ctrl->ptime_ramp_tbl = p_step_motor_source->ptime_dec_ramp_tbl;
				p_step_motor_ctrl->ptime_ramp = p_step_motor_ctrl->ptime_ramp_tbl + p_step_motor_ctrl->dec_ramp_max_size - p_step_motor_ctrl->dec_ramp_rt_size - 1;
				p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->dec_ramp_rt_size;
				StepMotor_SetTimer(p_step_motor_source);
			}
			else	//���ٹ����н����ٶȵ���
			{
				if(p_step_motor_ctrl->ptime_ramp < p_const_ramp)	//����
				{
					p_step_motor_ctrl->ptime_ramp++;
					StepMotor_SetTimer(p_step_motor_source);
				}
				else if(p_step_motor_ctrl->ptime_ramp > p_const_ramp)	//����
				{
					p_step_motor_ctrl->ptime_ramp--;
					StepMotor_SetTimer(p_step_motor_source);
				}
			}
			break;
		case MOTOR_ST_DEC:
			StepMotor_SetTimer(p_step_motor_source);
			if((p_step_motor_ctrl->ptime_ramp < p_stop_ramp))	//δ����ֹͣramp
			{
				p_step_motor_ctrl->ptime_ramp++;
			}
			else
			{
				p_step_motor_ctrl->ptime_ramp = p_stop_ramp;
				p_step_motor_ctrl->status = MOTOR_ST_POSIT;
			}
			if(p_step_motor_ctrl->ptime_ramp == p_stop_ramp)
			{
				p_step_motor_ctrl->status = MOTOR_ST_POSIT;
			}
			break;
		case MOTOR_ST_POSIT:
			TIM_Cmd(p_step_motor_source->timer_source->timer, DISABLE);
			StepMotor_MoveInit(p_step_motor_source);
			StepMotor_Enable(p_step_motor_source, DISABLE);
			StepMotor_Choice(p_step_motor_source,DISABLE);			
			break;
		default:
			StepMotor_Choice(p_step_motor_source,DISABLE);
			TIM_Cmd(p_step_motor_source->timer_source->timer, DISABLE);
			StepMotor_MoveInit(p_step_motor_source);
			StepMotor_Enable(p_step_motor_source, DISABLE);
			break;
	}
}

/**
  * @brief  ����˶��ٶȵ��������ڵ������ǰ�����ٶ����ã����ڵ�������˶������н��е���
  *         ע��: �Ӽ��ٹ������ٶȵ�����Ч�����ù������ٶȵ�����Ч
  * @param  p_step_motor_source: �������Դ�ṹ��ָ��
  *         speed: �ٶȲ�������ramp���н���ȡֵ
  * @retval 
  */
void StepMotor_Speed(t_step_motor_source * p_step_motor_source, uint16_t speed)
{
	t_step_motor_ctrl * p_step_motor_ctrl;
	uint16_t len;
	uint16_t len_missed;
	
	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	if(speed == 0)		//����ٶ�
	{
		p_step_motor_ctrl->acc_ramp_rt_size = p_step_motor_ctrl->acc_ramp_max_size;
		p_step_motor_ctrl->dec_ramp_rt_size = p_step_motor_ctrl->dec_ramp_max_size;
		if(p_step_motor_ctrl->ptime_ramp_tbl == p_step_motor_source->ptime_acc_ramp_tbl)
		{
			p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->acc_ramp_rt_size;
		}
		else
		{
			p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->dec_ramp_rt_size;
		}
	}
	//��С��2,��С�ٶ�,��С���ı�����ǴӴ�С(����)���С����(����)����
	else if(speed >= *(p_step_motor_source->ptime_acc_ramp_tbl))
	{
		p_step_motor_ctrl->acc_ramp_rt_size = 2;
		p_step_motor_ctrl->dec_ramp_rt_size = 2;
		if(p_step_motor_ctrl->ptime_ramp_tbl == p_step_motor_source->ptime_acc_ramp_tbl)
		{
			p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->acc_ramp_rt_size;
		}
		else
		{
			p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->dec_ramp_rt_size;
		}
	}
	else if((p_step_motor_ctrl->status == MOTOR_ST_INIT)		//����ǰ���ٶȵ���
		|| (p_step_motor_ctrl->status == MOTOR_ST_HOLD))
	{
		//�������ٱ���
		for(len = 0; len < p_step_motor_ctrl->acc_ramp_max_size; len++)
		{
			if(speed > p_step_motor_source->ptime_acc_ramp_tbl[len])
				break;
		}
		p_step_motor_ctrl->acc_ramp_rt_size = len;
		//�������ٱ���
		len_missed = 0;
		for(len = 0; len < p_step_motor_ctrl->dec_ramp_max_size; len++)
		{
			//���ٱ��ٶȴ��ڼ��ٱ��ٶ�ʱ�������볤��
			if(p_step_motor_source->ptime_dec_ramp_tbl[len] < speed)
			{
				len_missed++;
			}
		}
		p_step_motor_ctrl->dec_ramp_rt_size = len - len_missed;
		p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->acc_ramp_rt_size;
	}
	else if(p_step_motor_ctrl->status == MOTOR_ST_CONST)	//���ٹ������ٶȵ���
	{
		if(speed != *p_step_motor_ctrl->ptime_ramp)
		{
			//�������ٱ���
			for(len = 0; len < p_step_motor_ctrl->acc_ramp_max_size; len++)
			{
				if(speed > p_step_motor_source->ptime_acc_ramp_tbl[len])
					break;
			}
			p_step_motor_ctrl->acc_ramp_rt_size = len;
			//�������ٱ���
			len_missed = 0;
			for(len = 0; len < p_step_motor_ctrl->dec_ramp_max_size; len++)
			{
				//���ٱ��ٶȴ��ڼ��ٱ��ٶ�ʱ�������볤��
				if(p_step_motor_source->ptime_dec_ramp_tbl[len] < speed)
				{
					len_missed++;
				}
			}
			p_step_motor_ctrl->dec_ramp_rt_size = len - len_missed;
			p_step_motor_ctrl->time_tbl_size = p_step_motor_ctrl->acc_ramp_rt_size;
		}
	}
}

/* ��õ�ǰ�����в�������ȫ������ */
unsigned int StepMotor_GetRuningSteps(t_step_motor_source * p_step_motor_source)
{
	t_step_motor_ctrl * p_step_motor_ctrl;
	unsigned int run_steps;

	p_step_motor_ctrl = p_step_motor_source->step_motor_ctrl;
	run_steps = p_step_motor_ctrl->step_cnt;
	switch(p_step_motor_source->mode)
	{
		case MOTOR_MODE_FULL:
			run_steps = run_steps;
			break;
		case MOTOR_MODE_HALF:
			run_steps = run_steps / 2;
			break;
		case MOTOR_MODE_QUARTER:
			run_steps = run_steps / 4;
			break;
		case MOTOR_MODE_SIXTEEN:
			run_steps = run_steps / 16;
			break;
	}
	return run_steps;
}


