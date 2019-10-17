/*
  Filename: digtube.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 数码管驱动
*/
#include "pin.h"
#include "digtube.h"

/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
/*定时器*/
#define DIGTUBE_TIM		TIM4
#define DIGTUBE_TIM_RCC	RCC_APB1Periph_TIM4
#define CHG_TIME_CNT	200

static TIM_TimeBaseInitTypeDef digtube_time_struct =
	{71, TIM_CounterMode_Down, 1000, TIM_CKD_DIV1, NULL};		//1ms

static NVIC_InitTypeDef digtube_time_nvic_struct =
	{TIM4_IRQn, 2, 3, ENABLE};

/*数码管段pin*/
static t_pin_source led_port_pin[SEGMENT_NUM] =
{
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_0,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//A
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_1,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//B
	{GPIOG,	RCC_APB2Periph_GPIOG,	GPIO_Pin_10,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//C
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_3,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//D
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_4,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//E
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_5,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//F
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_6,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//G
};
/*数码管位pin*/
static t_pin_source led_dig_pin[LED_DIG_NUM] =
{
	{GPIOG,	RCC_APB2Periph_GPIOG,	GPIO_Pin_9,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//DIG1
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_7,		GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//DIG2
	{GPIOG,	RCC_APB2Periph_GPIOG,	GPIO_Pin_11,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//DIG3	
};

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
/*局部变量*/
static unsigned char  DIGTUBE_CHAR[LED_DIG_NUM];				//显示字符数组
static unsigned short on_time_cnt = 0;						//亮时间
static t_digtube_mode m_digtube_mode = DIGTUBE_MODE_OFF;//默认灭
static unsigned char  m_start_state = SET_TIMER_EN1;  		//定时使能数码管位选管脚状态
/*段位*/
const unsigned char segment_to_show[SEGMENT_NUM] =
{
	SEGMENT_A,
	SEGMENT_B,
	SEGMENT_C,
	SEGMENT_D,
	SEGMENT_E,
	SEGMENT_F,
	SEGMENT_G
};
/*字符*/
const unsigned char code_to_show[MAX_CHAR_TABLE] =
{
	DIGIT_CHAR_0,
	DIGIT_CHAR_1,
	DIGIT_CHAR_2,
	DIGIT_CHAR_3,
	DIGIT_CHAR_4,
	DIGIT_CHAR_5,
	DIGIT_CHAR_6,
	DIGIT_CHAR_7,
	DIGIT_CHAR_8,
	DIGIT_CHAR_9,
	DIGIT_CHAR_A,
	DIGIT_CHAR_B,
	DIGIT_CHAR_C,
	DIGIT_CHAR_D,
	DIGIT_CHAR_E,
	DIGIT_CHAR_F,
	DIGIT_CHAR_G,
	DIGIT_CHAR_H,
	DIGIT_CHAR_I,
	DIGIT_CHAR_J,
	DIGIT_CHAR_K,
	DIGIT_CHAR_L,
	DIGIT_CHAR_M,
	DIGIT_CHAR_N,
	DIGIT_CHAR_O,
	DIGIT_CHAR_P,
	DIGIT_CHAR_Q,
	DIGIT_CHAR_R,
	DIGIT_CHAR_S,
	DIGIT_CHAR_T,
	DIGIT_CHAR_U,
	DIGIT_CHAR_V,
	DIGIT_CHAR_W,
	DIGIT_CHAR_X,
	DIGIT_CHAR_Y,
	DIGIT_CHAR_Z
};

/**
  * @brief  数码管管脚初始化
  * @param  
  * @retval 
  */
void Digtube_Pin_Init(void)
{
	unsigned char i;
	t_pin_source * p_pin;

	on_time_cnt = 0;
	for(i = 0; i < SEGMENT_NUM; i++)
	{
		p_pin = &led_port_pin[i];
		Pin_Init(p_pin);
	}	
	for(i = 0; i < LED_DIG_NUM; i++)
	{
		p_pin = &led_dig_pin[i];
		Pin_Init(p_pin);
	}
}

/**
  * @brief  数码管定时器初始化
  * @param  
  * @retval 
  */
static void Digtube_Timer_Init(void)
{
	TIM_DeInit(DIGTUBE_TIM);

    /* TIM Periph clock enable */
  	RCC_APB1PeriphClockCmd(DIGTUBE_TIM_RCC, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseInit(DIGTUBE_TIM, &digtube_time_struct);

   	/* Clear TIM update pending flag[清除TIM 溢出中断标志] */
  	TIM_ClearFlag(DIGTUBE_TIM, TIM_FLAG_Update);

	/* Configure two bits for preemption priority */
 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Enable the TIM Interrupt */
	NVIC_Init(&digtube_time_nvic_struct);

  	/* Enable TIM Update interrupt [TIM 溢出中断允许]*/
  	TIM_ITConfig(DIGTUBE_TIM, TIM_IT_Update, ENABLE);
  	TIM_ARRPreloadConfig(DIGTUBE_TIM, ENABLE); /*自动重装载预装载*/

	/* TIM enable counter */
  	TIM_Cmd(DIGTUBE_TIM, ENABLE);
}

/**
  * @brief  数码管段控制
  * @param  t_segment_port:段号
  			enable:使能或禁止代表亮或者灭
  * @retval 
  */
void Digtube_Segment_On(t_segment_port port, unsigned char enable)
{
	t_pin_source * p_pin;
	BitAction bit_set;

	if(port>=SEGMENT_NUM)
		return;
	if(enable)
	{
		bit_set = Bit_SET;
	}
	else
	{
		bit_set = Bit_RESET;
	}
	p_pin = &led_port_pin[port];
	Pin_WriteBit(p_pin, bit_set);		
}

/**
  * @brief  数码管位控制
  * @param  port:位号
  			enable:使能或禁止
  * @retval 
  */
void Led_Dig_On(t_led_dig port, unsigned char enable)
{
	t_pin_source * p_pin;
	BitAction bit_set;

	if(port>=LED_DIG_NUM)
		return;
	if(enable)
	{
		bit_set = Bit_SET;
	}
	else
	{
		bit_set = Bit_RESET;
	}
	p_pin = &led_dig_pin[port];
	Pin_WriteBit(p_pin, bit_set);	
}

/**
  * @brief  数码管位字符设置
  * @param  segment:字符
  * @retval 
  */
void Led_Show(unsigned char segment)
{
	t_pin_source * p_pin;
	BitAction bit_set;
	unsigned char i;

	for(i=0; i<SEGMENT_NUM; i++)
	{
		if(segment & segment_to_show[i])
		{
			bit_set = Bit_SET;
		}
		else
		{
			bit_set = Bit_RESET;
		}
		p_pin = &led_port_pin[i];
		Pin_WriteBit(p_pin, bit_set);
	}
}

/**
  * @brief  数码管位字符显示
  * @param  dig:位号
  * @retval 
  */
static void Interface_Digtube_Show(uint8_t dig)
{
	if(dig == LED_DIG1)
	{
	   Led_Dig_On(LED_DIG1,ENABLE);
	   Led_Dig_On(LED_DIG2,DISABLE);
	   Led_Dig_On(LED_DIG3,DISABLE);	   
	   Led_Show(DIGTUBE_CHAR[LED_DIG1]);
	}
	else if(dig == LED_DIG2)
	{
	   Led_Dig_On(LED_DIG2,ENABLE);
	   Led_Dig_On(LED_DIG1,DISABLE);
	   Led_Dig_On(LED_DIG3,DISABLE);	   
	   Led_Show(DIGTUBE_CHAR[LED_DIG2]);
	}
	else if(dig == LED_DIG3)
	{
	   Led_Dig_On(LED_DIG3,ENABLE);
	   Led_Dig_On(LED_DIG1,DISABLE);
	   Led_Dig_On(LED_DIG2,DISABLE);	   
	   Led_Show(DIGTUBE_CHAR[LED_DIG3]);
	}	
}

/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  数码管设置为全亮或者全灭
  * @param  on_or_off:DIGTUBE_ON为全亮，DIGTUBE_OFF为全灭
  * @retval 
  */
void Digtube_All_OnOff(unsigned char on_or_off)
{
	uint8_t i;
	if(on_or_off==DIGTUBE_ON)
	{
 		for(i = 0; i < LED_DIG_NUM; i++)
	    DIGTUBE_CHAR[i] = DIGIT_ALL_ON;
	}
	else if(on_or_off==DIGTUBE_OFF)
	{
 		for(i = 0; i < LED_DIG_NUM; i++)
	    DIGTUBE_CHAR[i] = DIGIT_ALL_OFF;	
	} 
}

/**
  * @brief  数码管显示
  * @param  code:显示内容字符串
  			mode:显示模式，亮或者灭或者闪烁
  * @retval 
  */
void Digtube_Show(unsigned char *code, t_digtube_mode mode)
{
	uint8_t i;
	uint16_t index = 0;
	
	m_digtube_mode = mode;
	for(i = 0; i < LED_DIG_NUM; i++)
	{
		if(code[i] >= '0' && code[i] <= '9')
		{
			index = code[i] - '0';
			DIGTUBE_CHAR[i] = code_to_show[index];
		}
		else if(code[i] >= 'A' && code[i] <= 'Z')
		{
			index = code[i] - 'A' + 0x0A;
			DIGTUBE_CHAR[i] = code_to_show[index];
		}
		else if(code[i] == '-')
		{
			DIGTUBE_CHAR[i] = DIGIT_CHAR_MINUS;
		}
	}
}

/**
  * @brief  数码管初始化
  * @param  
  * @retval 
  */
void Digtube_Init(void)
{
	uint8_t ch[LED_DIG_NUM] = {'8', '8', '8'};
	Digtube_Pin_Init();
	Digtube_Timer_Init();
	Digtube_Show(ch, DIGTUBE_MODE_ON);
}

/**
  * @brief  数码管中断函数
  * @param  
  * @retval 
  */
void Digtube_Timer_Handler(void)
{
	if(((on_time_cnt < (CHG_TIME_CNT/2)) && (m_digtube_mode == DIGTUBE_MODE_BLINK))
		|| (m_digtube_mode == DIGTUBE_MODE_OFF))
	{
		Led_Dig_On(LED_DIG1, DISABLE);
		Led_Dig_On(LED_DIG2, DISABLE);
		Led_Dig_On(LED_DIG3, DISABLE);
	}
	else
	{
		if(m_start_state == SET_TIMER_EN1)
		{
			Interface_Digtube_Show(LED_DIG1); 
			m_start_state = SET_TIMER_EN2;
		}
		else if(m_start_state == SET_TIMER_EN2)
		{
			Interface_Digtube_Show(LED_DIG2); 
			m_start_state = SET_TIMER_EN3;
		}
		else if(m_start_state == SET_TIMER_EN3)
		{
			Interface_Digtube_Show(LED_DIG3); 
			m_start_state = SET_TIMER_EN1;
		}		
	}
	on_time_cnt = (on_time_cnt + 1) % CHG_TIME_CNT;
}

