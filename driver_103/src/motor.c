/*
  Filename: motor.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 电机驱动
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
/*定时器*/
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
	{GPIOF,	RCC_APB2Periph_GPIOF,	GPIO_Pin_14,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},		//伺服电机没有sleep信号
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

/* 电机选通信号，用于多个电机复用部分管脚 */
/*static t_pin_source motor_choice[1]=
{
	{GPIOC, RCC_APB2Periph_GPIOC,	GPIO_Pin_15,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//MOTOR Z1
}; 
*/


/*电机控制默认值*/
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
  电机名称: X轴电机
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
  电机名称: Y轴电机
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
	NULL,		//Y轴为伺服电机没有sleep
	NULL,
	&motor_timer3,
	&motor_y_ctrl
};

/*
  电机名称: Z轴电机
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
  电机名称: Z轴hook电机
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
  电机名称: 扫描头旋转电机
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

/* 电机切换小电流pin, 用于hold */
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


/*各电机配置*/
static t_step_motor_source * motor_source_array[MOTOR_MAX] =
{
	&motor_x_source,
	&motor_y_source,
	&motor_z_source,
	&motor_hook_source,
	&motor_scanner_source
};

/* 电机切换小电流pin, 用于hold */
static t_pin_source * motor_hold_sw_source_array[MOTOR_MAX] =
{
	&motor_hold_sw_pin[0],
	NULL,
	&motor_hold_sw_pin[1],
	&motor_hold_sw_pin[2],
	&motor_hold_sw_pin[3]
};

/**
  * @brief  获得电机源结构体指针
  * @param  无
  * @retval 电机源结构体指针
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
  * @brief  选择电机
  * @param  
  * @retval 
  */
static void Motor_Chioce(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);

	/* 选择对应的定时器 */
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
	电机电流切换，用于HOLD时不能电流过大，防止过热
	电机运行时用大电流
	set_to_little: 	1设置为小电流
					0设置为大电流
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
  * @brief  电机初始化
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
  * @brief  电机运动函数
  * @param  motor_no: 电机序号
  *         step: 运动步数
  *         micro_step: 微分运动步数
  *         dir: 运动方向
  *         speed:  表示电机速度，时间以us表示，其倒数即为电机速度pps，0表示支持的最大速度
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
  * @brief  电机停止函数
  * @param  motor_no: 电机序号
  *         stop_step: 停止步数
  *         micro_stop_step: 微分停止步数
  * @retval 
  */
void Motor_Stop(t_motor_no motor_no, unsigned int stop_step, unsigned int micro_stop_step)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	StepMotor_Stop(p_motor_source, stop_step, micro_stop_step);
}

/**
  * @brief  电机等待停止函数
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
  * @brief  电机是否停止
  * @param  
  * @retval 1: 已停止
  *         0: 未停止
  */
uint8_t Motor_IsStoped(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	return StepMotor_IsStoped(p_motor_source);
}


/**
  * @brief  电机锁死函数
  * @param  
  * @retval 
  */
void Motor_Hold(t_motor_no motor_no, FunctionalState enable)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
#if 0		//debug y轴使用大电流锁死
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
  * @brief  电机中断函数
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
  * @brief  定时器2中断函数
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
  * @brief  定时器3中断函数
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
  * @brief  电机运动速度调整，可在电机启动前进行速度设置，与在电机匀速运动过程中进行调整
  *         注意: 加减速过程中速度调整无效，安置过程中速度调整无效
  * @param  motor_no: 电机序号
  *         speed: 速度参数，在ramp表中进行取值
  * @retval 
  */
void Motor_Speed(t_motor_no motor_no, uint16_t speed)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	StepMotor_Speed(p_motor_source, speed);
}

/* 获得电机已经运行的步数，全步 */
unsigned int Motor_GetRuningSteps(t_motor_no motor_no)
{
	t_step_motor_source * p_motor_source;
	
	p_motor_source = Motor_GetSource(motor_no);
	return StepMotor_GetRuningSteps(p_motor_source);
}


