/*
  Filename: led.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: led灯驱动
*/
#include "pin.h"
#include "led.h"
//#include "lib_common.h"

/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
static t_pin_source led_pin[LED_NUM] =
{
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_10,	GPIO_Mode_Out_PP,		GPIO_Speed_50MHz,	NULL},	//heartbeat led
};

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
#define BREATHING_CIRCLE		500
static uint16_t led_breathing_time = BREATHING_CIRCLE;
static uint8_t led_breathing_st = 0;

/**
  * @brief  led灯控制
  * @param  port:灯号
  			enable:使能或禁止
  * @retval 
  */
void Led_On(t_led_other port, uint8_t enable)
{
	t_pin_source * p_pin;
	BitAction bit_set;

	if(port>=LED_NUM)
		return;
	if(enable)//反的
	{
		bit_set = Bit_RESET;
	}
	else
	{
		bit_set = Bit_SET;
	}
	p_pin = &led_pin[port];
	Pin_WriteBit(p_pin, bit_set);
}
/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  led灯初始化
  * @param  
  * @retval 
  */
void Led_Init(void)
{
	uint8_t i;
	t_pin_source * p_pin;

	for(i = 0; i < LED_NUM; i++)
	{
		p_pin = &led_pin[i];
		Pin_Init(p_pin);
	}
}

/**
  * @brief  led灯中断函数
  * @param  
  * @retval 
  */
void Led_BreathingTimerHandler(void)
{
	if(led_breathing_time)
	{
		led_breathing_time--;
	}
	else
	{
		led_breathing_time = BREATHING_CIRCLE;
		Led_BreathingChange();
	}
}

/**
  * @brief  呼吸灯
  * @param  
  * @retval 
  */
void Led_BreathingChange(void)
{
	led_breathing_st = !led_breathing_st;
	Led_On(LED_BREATHING, led_breathing_st);
}

