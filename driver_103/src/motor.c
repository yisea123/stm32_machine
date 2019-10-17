/*
  Filename: motor.c
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: �������
*/
#include "lib_common.h"
#include "pin.h"
#include "ramp2.h"
#include "motor.h"
#include "main.h"
#include "s_def.h"

/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
/*��ʱ��*/
static uint16_t tmp_time;

static t_motor_no tim2_motor_no = MOTOR_MAX;
static t_motor_no tim3_motor_no = MOTOR_MAX;
static t_motor_no tim5_motor_no = MOTOR_MAX;


static TIM_TimeBaseInitTypeDef time2_struct =
	{71, TIM_CounterMode_Down, HOLDING_TIME, TIM_CKD_DIV1, NULL};
static TIM_TimeBaseInitTypeDef time3_struct =
	{71, TIM_CounterMode_Down, HOLDING_TIME, TIM_CKD_DIV1, NULL};
static TIM_TimeBaseInitTypeDef time5_struct =
	{71, TIM_CounterMode_Down, HOLDING_TIME, TIM_CKD_DIV1, NULL};


static NVIC_InitTypeDef timer2_nvic_struct =
	{TIM2_IRQn, 1, 1, ENABLE};
static NVIC_InitTypeDef timer3_nvic_struct =
	{TIM3_IRQn, 1, 1, ENABLE};
static NVIC_InitTypeDef timer5_nvic_struct =
	{TIM5_IRQn, 1, 1, ENABLE};


static t_timer_source motor_timer2 = //timer2
	{TIM2, RCC_APB1Periph_TIM2, &time2_struct, NVIC_PriorityGroup_1, &timer2_nvic_struct};

static t_timer_source motor_timer3 = //timer3
	{TIM3, RCC_APB1Periph_TIM3, &time3_struct, NVIC_PriorityGroup_1, &timer3_nvic_struct};

static t_timer_source motor_timer5= //timer5
	{TIM5, RCC_APB1Periph_TIM5, &time5_struct, NVIC_PriorityGroup_1, &timer5_nvic_struct};


static t_pin_source motor_x_pin[MOTOR_PIN_MAX]=
{
	{GPIOB,	RCC_APB2Periph_GPIOB,	GPIO_Pin_0,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOC, RCC_APB2Periph_GPIOC,	GPIO_Pin_5, 	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOC, RCC_APB2Periph_GPIOC,	GPIO_Pin_4, 	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOB, RCC_APB2Periph_GPIOB,	GPIO_Pin_1, 	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
};

static t_pin_source motor_y_pin[MOTOR_PIN_MAX]=
{
	{GPIOF,	RCC_APB2Periph_GPIOF,	GPIO_Pin_14,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOF,	RCC_APB2Periph_GPIOF,	GPIO_Pin_13,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOF,	RCC_APB2Periph_GPIOF,	GPIO_Pin_12,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOF,	RCC_APB2Periph_GPIOF,	GPIO_Pin_14,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},		//�ŷ����û��sleep�ź�
};

static t_pin_source motor_z_pin[MOTOR_PIN_MAX]=
{
	{GPIOE,	RCC_APB2Periph_GPIOE,	GPIO_Pin_9,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOE,	RCC_APB2Periph_GPIOE,	GPIO_Pin_8,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOE,	RCC_APB2Periph_GPIOE,	GPIO_Pin_7,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOE,	RCC_APB2Periph_GPIOE,	GPIO_Pin_10,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
};

static t_pin_source motor_hook_pin[MOTOR_PIN_MAX]=
{
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_2,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_1,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_0,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_3,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
};

static t_pin_source motor_scanner_pin[MOTOR_PIN_MAX]=
{
	{GPIOG,	RCC_APB2Periph_GPIOG,	GPIO_Pin_2,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOG,	RCC_APB2Periph_GPIOG,	GPIO_Pin_4,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOG,	RCC_APB2Periph_GPIOG,	GPIO_Pin_3,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_14,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
};

/* ���ѡͨ�źţ����ڶ��������ò��ֹܽ� */
/*static t_pin_source motor_choice[1]=
{
	{GPIOC, RCC_APB2Periph_GPIOC,	GPIO_Pin_15,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//MOTOR Z1
}; 
*/


/*�������Ĭ��ֵ*/
static t_step_motor_ctrl motor_x_ctrl =
	{MOTOR_DIR_FORWARD,	MOTOR_ST_INIT,	0,	0,	Bit_RESET,	0,	0,	0,	0,	&tmp_time,	&tmp_time,	1};
static t_step_motor_ctrl motor_y_ctrl =
	{MOTOR_DIR_FORWARD,	MOTOR_ST_INIT,	0,	0,	Bit_RESET,	0,	0,	0,	0,	&tmp_time,	&tmp_time,	1};
static t_step_motor_ctrl motor_z_ctrl =
	{MOTOR_DIR_FORWARD,	MOTOR_ST_INIT,	0,	0,	Bit_RESET,	0,	0,	0,	0,	&tmp_time,	&tmp_time,	1};
static t_step_motor_ctrl motor_hook_ctrl =
	{MOTOR_DIR_FORWARD,	MOTOR_ST_INIT,	0,	0,	Bit_RESET,	0,	0,	0,	0,	&tmp_time,	&tmp_time,	1};
static t_step_motor_ctrl motor_scanner_ctrl =
	{MOTOR_DIR_FORWARD,	MOTOR_ST_INIT,	0,	0,	Bit_RESET,	0,	0,	0,	0,	&tmp_time,	&tmp_time,	1};

/*
  �������: X����
*/
static t_step_motor_source motor_x_source =
{
	MOTOR_MODE_SIXTEEN,
	ramp_sixteen_2500_acc,
	ramp_sixteen_2500_dec,
	&motor_x_pin[MOTOR_PIN_EN],
	&motor_x_pin[MOTOR_PIN_PLUSE],
	&motor_x_pin[MOTOR_PIN_DIR],
	&motor_x_pin[MOTOR_PIN_SLEEP],
	NULL,
	&motor_timer2,
	&motor_x_ctrl
};

/*
  �������: Y����
*/
static t_step_motor_source motor_y_source =
{
	MOTOR_MODE_QUARTER,
	ramp_sixteen_5000_acc,
	ramp_sixteen_5000_dec,
	&motor_y_pin[MOTOR_PIN_EN],
	&motor_y_pin[MOTOR_PIN_PLUSE],
	&motor_y_pin[MOTOR_PIN_DIR],
	//&motor_y_pin[MOTOR_PIN_SLEEP],
	NULL,		//Y��Ϊ�ŷ����û��sleep
	NULL,
	&motor_timer3,
	&motor_y_ctrl
};

/*
  �������: Z����
*/
static t_step_motor_source motor_z_source =
{
	MOTOR_MODE_SIXTEEN,
	ramp_sixteen_1200_acc,
	ramp_sixteen_1200_dec,
	&motor_z_pin[MOTOR_PIN_EN],
	&motor_z_pin[MOTOR_PIN_PLUSE],
	&motor_z_pin[MOTOR_PIN_DIR],
	&motor_z_pin[MOTOR_PIN_SLEEP],
	NULL,
	&motor_timer5,
	&motor_z_ctrl
};

/*
  �������: Z��hook���
*/
static t_step_motor_source motor_hook_source =
{
	MOTOR_MODE_SIXTEEN,
	ramp_sixteen_1050_acc,
	ramp_sixteen_1050_dec,
	&motor_hook_pin[MOTOR_PIN_EN],
	&motor_hook_pin[MOTOR_PIN_PLUSE],
	&motor_hook_pin[MOTOR_PIN_DIR],
	&motor_hook_pin[MOTOR_PIN_SLEEP],
	NULL,
	&motor_timer5,
	&motor_hook_ctrl
};

/*
  �������: ɨ��ͷ��ת���
*/
static t_step_motor_source motor_scanner_source =
{
	MOTOR_MODE_SIXTEEN,
	ramp_sixteen_1050_acc,
	ramp_sixteen_1050_dec,
	&motor_scanner_pin[MOTOR_PIN_EN],
	&motor_scanner_pin[MOTOR_PIN_PLUSE],
	&motor_scanner_pin[MOTOR_PIN_DIR],
	&motor_scanner_pin[MOTOR_PIN_SLEEP],
	NULL,
	&motor_timer5,
	&motor_scanner_ctrl
};

/* ����л�С����pin, ����hold */
static t_pin_source motor_hold_sw_pin[4] =
{
	//motor x
	{GPIOA,	RCC_APB2Periph_GPIOA,	GPIO_Pin_4,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	//motor z
	{GPIOA,	RCC_APB2Periph_GPIOA,	GPIO_Pin_5,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	//motor z_hook
	{GPIOA,	RCC_APB2Periph_GPIOA,	GPIO_Pin_1,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
	//motor scanner
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_15,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},
};


/*���������*/
static t_step_motor_source * motor_source_array[MOTOR_MAX] =
{
	&motor_x_source,
	&motor_y_source,
	&motor_z_source,
	&motor_hook_source,
	&motor_scanner_source
};

/* ����л�С����pin, ����hold */
static t_pin_source * motor_hold_sw_source_array[MOTOR_MAX] =
{
	&motor_hold_sw_pin[0],
	NULL,
	&motor_hold_sw_pin[1],
	&motor_hold_sw_pin[2],
	&motor_hold_sw_pin[3]
};

/**
  * @brief  ��õ��Դ�ṹ��ָ��
  * @param  ��
  * @retval ���Դ�ṹ��ָ��
  */
static t_step_motor_source * Motor_GetSource(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	switch(motor_no)
	{
		case MOTOR_X:
			p_motor_source = motor_source_array[MOTOR_X];
			break;
		case MOTOR_Y:
			p_motor_source = motor_source_array[MOTOR_Y];
			break;
		case MOTOR_Z:
			p_motor_source = motor_source_array[MOTOR_Z];
			break;
		case MOTOR_HOOK:
			p_motor_source = motor_source_array[MOTOR_HOOK];
			break;
		case MOTOR_SCANNER:
			p_motor_source = motor_source_array[MOTOR_SCANNER];
			break;
		default:
			p_motor_source = NULL;
			break;
	}
	
	return p_motor_source;
}

/**
  * @brief  ѡ����
  * @param  
  * @retval 
  */
static void Motor_Chioce(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);

	/* ѡ���Ӧ�Ķ�ʱ�� */
	if(p_motor_source->timer_source->timer == TIM2)
	{
		tim2_motor_no = motor_no;
	}
	else if(p_motor_source->timer_source->timer == TIM3)
	{
		tim3_motor_no = motor_no;
	}
	else if(p_motor_source->timer_source->timer == TIM5)
	{
		tim5_motor_no = motor_no;
	}	

}

/*
	��������л�������HOLDʱ���ܵ������󣬷�ֹ����
	�������ʱ�ô����
	set_to_little: 	1����ΪС����
					0����Ϊ�����
*/
static void Motor_EleCurrent(t_motor_no motor_no, unsigned char set_to_little)
{
	t_pin_source * p_pin_source;

	p_pin_source = motor_hold_sw_source_array[motor_no];
	if(p_pin_source != NULL)
	{
		if(set_to_little)
			Pin_WriteBit(p_pin_source, Bit_SET);
		else
			Pin_WriteBit(p_pin_source, Bit_RESET);
	}
}

/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  �����ʼ��
  * @param  
  * @retval 
  */
void Motor_Init(void)
{
	unsigned char i;
	for(i = 0; i < MOTOR_MAX; i++)
	{
		StepMotor_Init(motor_source_array[i]);
	}
	for(i = 0; i < (sizeof(motor_hold_sw_pin)/sizeof(t_pin_source)); i++)
	{
		Pin_Init(&motor_hold_sw_pin[i]);
	}
	for(i = 0; i < MOTOR_MAX; i++)
	{
		Motor_Hold((t_motor_no)i, DISABLE);
	}
}

/**
  * @brief  ����˶�����
  * @param  motor_no: ������
  *         step: �˶�����
  *         micro_step: ΢���˶�����
  *         dir: �˶�����
  *         speed:  ��ʾ����ٶȣ�ʱ����us��ʾ���䵹����Ϊ����ٶ�pps��0��ʾ֧�ֵ�����ٶ�
  * @retval 
  */
void Motor_Move(t_motor_no motor_no, unsigned int step, unsigned int micro_step, t_motor_dir dir, uint16_t speed)
{
	t_step_motor_source * p_motor_source;
	t_motor_dir c_dir;

	c_dir = dir;
	p_motor_source = Motor_GetSource(motor_no);
	Motor_EleCurrent(motor_no, 0);
	Motor_Chioce(motor_no);
	StepMotor_Move(p_motor_source, step, micro_step, c_dir, speed);
}

/**
  * @brief  ���ֹͣ����
  * @param  motor_no: ������
  *         stop_step: ֹͣ����
  *         micro_stop_step: ΢��ֹͣ����
  * @retval 
  */
void Motor_Stop(t_motor_no motor_no, unsigned int stop_step, unsigned int micro_stop_step)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	StepMotor_Stop(p_motor_source, stop_step, micro_stop_step);
}

/**
  * @brief  ����ȴ�ֹͣ����
  * @param  
  * @retval 
  */
void Motor_WaitStoped(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	StepMotor_WaitStoped(p_motor_source);
}

/**
  * @brief  ����Ƿ�ֹͣ
  * @param  
  * @retval 1: ��ֹͣ
  *         0: δֹͣ
  */
uint8_t Motor_IsStoped(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	return StepMotor_IsStoped(p_motor_source);
}


/**
  * @brief  �����������
  * @param  
  * @retval 
  */
void Motor_Hold(t_motor_no motor_no, FunctionalState enable)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
#if 0		//debug y��ʹ�ô��������
	if(motor_no == MOTOR_Y)
		Motor_EleCurrent(motor_no, 0);
	else
		Motor_EleCurrent(motor_no, enable);
#else
	Motor_EleCurrent(motor_no, enable);
#endif
	StepMotor_Hold(p_motor_source, enable);
}

/**
  * @brief  ����жϺ���
  * @param  
  * @retval 
  */
void Motor_IntHandler(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	StepMotor_IntHandler(p_motor_source);
}

/**
  * @brief  ��ʱ��2�жϺ���
  * @param  
  * @retval 
  */
void Motor_Tim2IntHandler(void)
{
	if(tim2_motor_no < MOTOR_MAX)
	{
		Motor_IntHandler(tim2_motor_no);
	}
}

/**
  * @brief  ��ʱ��3�жϺ���
  * @param  
  * @retval 
  */
void Motor_Tim3IntHandler(void)
{
	if(tim3_motor_no < MOTOR_MAX)
	{
		Motor_IntHandler(tim3_motor_no);
	}
}

void Motor_Tim5IntHandler(void)
{
	if(tim5_motor_no < MOTOR_MAX)
	{
		Motor_IntHandler(tim5_motor_no);
	}
}

/**
  * @brief  ����˶��ٶȵ��������ڵ������ǰ�����ٶ����ã����ڵ�������˶������н��е���
  *         ע��: �Ӽ��ٹ������ٶȵ�����Ч�����ù������ٶȵ�����Ч
  * @param  motor_no: ������
  *         speed: �ٶȲ�������ramp���н���ȡֵ
  * @retval 
  */
void Motor_Speed(t_motor_no motor_no, uint16_t speed)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	StepMotor_Speed(p_motor_source, speed);
}

/* ��õ���Ѿ����еĲ�����ȫ�� */
unsigned int Motor_GetRuningSteps(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	return StepMotor_GetRuningSteps(p_motor_source);
}


