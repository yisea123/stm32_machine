/*
  Filename: autobaud.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 波特率自适应驱动
*/
#include "lib_common.h"
#include "autobaud.h"
#include "baud_adapt_lib.h"
#include "com.h"
/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
static t_exti_source autobaud_exit =
	{GPIO_PortSourceGPIOA, GPIO_PinSource8, EXTI_Line8, EXTI_Trigger_Rising_Falling, NVIC_PriorityGroup_0, EXTI9_5_IRQn, 0x01, 0X01, NULL};

static t_pin_source autobaud_pin =		//PG1
	{GPIOA, RCC_APB2Periph_GPIOA, GPIO_Pin_8, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &autobaud_exit};

static TIM_TimeBaseInitTypeDef autobaud_timer_struct =
	{71, TIM_CounterMode_Up, 0xFFFF, TIM_CKD_DIV1, NULL};

/*static NVIC_InitTypeDef autobaud_timer_nvic_struct =
	{TIM5_IRQn, 1, 1, ENABLE};
								   */
static t_timer_source autobaud_timer =
	{TIM1, RCC_APB2Periph_TIM1, &autobaud_timer_struct, NVIC_PriorityGroup_1,NULL};

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
#define PROTOCOL_NUM		3		//支持不同的协议数
#define PERIOD_NUM	     	7		//需要对比的时间周期个数

uint32_t period_array[PERIOD_NUM];
uint32_t period_uint_array[PERIOD_NUM];
uint8_t ch[3] = {0x10, 0x02, 0x00};

uint32_t com_baud_bps[BAUD_NUM] =
{
	BAUD_4800_BPS,
	BAUD_9600_BPS,
	BAUD_19200_BPS,
	BAUD_38400_BPS,
	BAUD_57600_BPS,
	//BAUD_115200_BPS //115200bps的波特率自适应切换到串口驱动时丢数据
};

uint32_t com_baud_period_uint[BAUD_NUM] =
{
	T1_4800BPS, 		
	T1_9600BPS, 		
	T1_19200BPS,		
 	T1_38400BPS,		
 	T1_57600BPS,		
	//T1_115200BPS	
};

uint32_t time_unit1[PERIOD_NUM] = {16, 48, 16, 32, 16, 96, 16};
uint32_t time_unit2[PERIOD_NUM] = {16, 64, 16, 32, 16, 112, 16};
uint32_t time_unit3[PERIOD_NUM] = {16, 48, 32, 32, 16, 96, 32};

t_uint_record com_protocol_array[PROTOCOL_NUM] =
{
	{{USART_WordLength_8b, USART_StopBits_1, USART_Parity_No}, 3, 	time_unit1},
	{{USART_WordLength_9b, USART_StopBits_1, USART_Parity_Odd}, 3,  time_unit2},
	{{USART_WordLength_9b, USART_StopBits_1, USART_Parity_Even}, 3,	time_unit3}
};

t_autobaud_source m_autobaud_src =
{
	&autobaud_pin,
	&autobaud_timer,

	com_baud_bps,
	com_baud_period_uint,
	0,
	com_protocol_array,
	PROTOCOL_NUM,
	PERIOD_NUM,
	5,

	PHASE_START,
	period_array,
	period_uint_array,

	19200,
	0,
	0,
	0,
	0,
	AB_NONE,

	AutoBaud_SetBaudrate,
	AutoBaud_SetComEn,
	ch,
	AutoBaud_UnGetCh
};

/**
  * @brief  设置波特率自适应对应串口
  * @param  
  * @retval 
  */
void AutoBaud_SetBaudrate(uint32_t baudrate,uint16_t wordlength,uint16_t stopbits,uint16_t parity)
{
	Com_Set_Baud(MAIN_COM1, baudrate, wordlength, stopbits, parity);
}

/**
  * @brief  波特率自适应对应串口使能
  * @param  en: 使能或禁止
  * @retval 
  */
void AutoBaud_SetComEn(FunctionalState en)
{
	Com_Enable(MAIN_COM1, en);
}

/**
  * @brief  波特率自适应对应串口回退一个字符
  * @param  
  * @retval 
  */
void AutoBaud_UnGetCh(uint8_t ch)
{
	Com_UnGet_Ch(MAIN_COM1, ch);
}

/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  波特率自适应控制
  * @param  enable:使能或禁止
  * @retval 
  */
void Autobaud_Enable(uint8_t enable)
{
	FunctionalState en;
	if(enable)
	{
		en = ENABLE;
	}
	else
	{
		en = DISABLE;
	}
	BaudAdapt_Enable(&m_autobaud_src, Autobaud_Handler, en);
}

/**
  * @brief  波特率自适应初始化
  * @param  
  * @retval 
  */
void Autobaud_Init(void)
{
	BaudAdapt_Init(&m_autobaud_src);
}

/**
  * @brief  波特率自适应中断函数
  * @param  
  * @retval 
  */
void Autobaud_Handler(void)
{
	BaudAdapt_Handler(&m_autobaud_src);
}

/**
  * @brief  查询波特率自适应是否完成
  * @param  
  * @retval 
  */
uint8_t Autobaud_GetDone(void)
{
	return BaudAdapt_GetDone(&m_autobaud_src);
}

void Autobaud_Set_Done(void)
{
	t_autobaud_source * p_autobaud_source;
	p_autobaud_source=&m_autobaud_src;
	p_autobaud_source->st=AB_OK;
}

void Autobaud_Set_Done_Init(void)
{
	t_autobaud_source * p_autobaud_source;
	p_autobaud_source=&m_autobaud_src;
	p_autobaud_source->st=AB_NONE;
}


