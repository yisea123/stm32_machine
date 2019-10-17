/*
  Filename: pin.c
  Author: shidawei			Date: 20181102
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: STM32F10X GPIO配置Lib
  Update: 增加空指针检查			20181102
*/
#include "pin.h"

/**
  * @brief  GPIO初始化
  * @param  p_pin_source: 指向pin对象指针
  * @retval 
  */
void Pin_Init(t_pin_source* p_pin_source)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if(p_pin_source == 0)
		return;
	//rcc init
	RCC_APB2PeriphClockCmd(p_pin_source->GPIO_Rcc, ENABLE);
	//gpio init
	GPIO_InitStructure.GPIO_Mode = p_pin_source->GPIO_Mode;
	GPIO_InitStructure.GPIO_Speed = p_pin_source->GPIO_Speed;
	GPIO_InitStructure.GPIO_Pin = p_pin_source->GPIO_Pin;
	GPIO_Init(p_pin_source->GPIO_Type, &GPIO_InitStructure);
}

/**
  * @brief  GPIO外部中断使能
  * @param  p_pin_source: 指向pin对象指针
  *         handler: 中断函数名称(函数指针)
  *         trigger: 中断触发方式
  *         enable: 使能中断或失能中断
  * @retval 
  */
void Pin_ExtiEnable(t_pin_source* p_pin_source, fun_exti_handler handler,
					EXTITrigger_TypeDef trigger, FunctionalState enable)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	t_exti_source * p_exti_source;

	if(p_pin_source == 0)
		return;
	p_exti_source= p_pin_source->GPIO_Exti;
	if(p_exti_source == NULL)
	{
		return;
	}
	
	if((p_pin_source->GPIO_Mode == GPIO_Mode_IPD) ||
	   (p_pin_source->GPIO_Mode == GPIO_Mode_IPU) ||
	   (p_pin_source->GPIO_Mode == GPIO_Mode_AIN) ||
	   (p_pin_source->GPIO_Mode == GPIO_Mode_IN_FLOATING)
		)
	{
		if(enable)
		{
			p_exti_source->fun_ExtiHandler = handler;
			p_exti_source->EXTI_Trigger = trigger;
		}
		else
		{
			p_exti_source->fun_ExtiHandler = NULL;
		}
		RCC_APB2PeriphClockCmd((p_pin_source->GPIO_Rcc | RCC_APB2Periph_AFIO), ENABLE);
		GPIO_EXTILineConfig(p_exti_source->EXTI_PortSource, p_exti_source->EXTI_PinSource);
		/* Configure EXTI line */
		EXTI_InitStructure.EXTI_Line = p_exti_source->EXTI_Line;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = trigger;
		EXTI_InitStructure.EXTI_LineCmd = enable;
		EXTI_Init(&EXTI_InitStructure);
		
		/* Enable and set EXTI Interrupt to the lowest priority */
		NVIC_PriorityGroupConfig(p_exti_source->NVIC_PriorityGroup);
		NVIC_InitStructure.NVIC_IRQChannel = p_exti_source->NVIC_IRQChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = p_exti_source->NVIC_IRQChannelPreemptionPriority;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = p_exti_source->NVIC_IRQChannelSubPriority;
		NVIC_InitStructure.NVIC_IRQChannelCmd = enable;
		NVIC_Init(&NVIC_InitStructure);
	}
}

/**
  * @brief  pin中断处理函数，用于在stm32fxxx_it.c中调用的函数接口
  *         其中的具体函数调用可重载，使用的函数指针Pin_ExtiEnable中指定
  * @param  
  * @retval 
  */
void Pin_ExtiHandler(t_pin_source* p_pin_source)
{
	t_exti_source * p_exti_source;

	if(p_pin_source == 0)
		return;
	p_exti_source= p_pin_source->GPIO_Exti;
	if((p_exti_source != NULL) && (p_exti_source->fun_ExtiHandler != NULL))
	{
		p_exti_source->fun_ExtiHandler();
	}
}

/**
  * @brief  读取pin对应为值
  * @param  p_pin_source: 指向pin对象指针
  * @retval 返回对应位的值
  */
uint8_t Pin_ReadBit(t_pin_source* p_pin_source)
{
	uint8_t ret_val = 0;
	if(p_pin_source == 0)
		return 0;
	if((p_pin_source->GPIO_Mode == GPIO_Mode_IN_FLOATING) ||
	   (p_pin_source->GPIO_Mode == GPIO_Mode_AIN) ||
	   (p_pin_source->GPIO_Mode == GPIO_Mode_IPD) ||
	   (p_pin_source->GPIO_Mode == GPIO_Mode_IPU))
	{
		ret_val = GPIO_ReadInputDataBit(p_pin_source->GPIO_Type, p_pin_source->GPIO_Pin);
	}
	else if((p_pin_source->GPIO_Mode == GPIO_Mode_Out_OD) ||
		    (p_pin_source->GPIO_Mode == GPIO_Mode_Out_PP))
	{
		ret_val = GPIO_ReadOutputDataBit(p_pin_source->GPIO_Type, p_pin_source->GPIO_Pin);
	}

	return ret_val;
}

/**
  * @brief  写pin对应位值
  * @param  p_pin_source: 指向pin对象指针
  *         BitVal: 写入值
  * @retval 返回写入是否成功
  */
uint8_t Pin_WriteBit(t_pin_source* p_pin_source, BitAction BitVal)
{
	uint8_t ret_val = 0;
	if(p_pin_source == 0)
		return 0;
	if((p_pin_source->GPIO_Mode == GPIO_Mode_Out_OD) ||
		    (p_pin_source->GPIO_Mode == GPIO_Mode_Out_PP))
	{
		GPIO_WriteBit(p_pin_source->GPIO_Type, p_pin_source->GPIO_Pin, BitVal);
		ret_val = 1;
	}

	return ret_val;
}

