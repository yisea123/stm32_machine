/*
  Filename: pin.c
  Author: shidawei			Date: 20181102
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: STM32F10X GPIO����Lib
  Update: ���ӿ�ָ����			20181102
*/
#include "pin.h"

/**
  * @brief  GPIO��ʼ��
  * @param  p_pin_source: ָ��pin����ָ��
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
  * @brief  GPIO�ⲿ�ж�ʹ��
  * @param  p_pin_source: ָ��pin����ָ��
  *         handler: �жϺ�������(����ָ��)
  *         trigger: �жϴ�����ʽ
  *         enable: ʹ���жϻ�ʧ���ж�
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
  * @brief  pin�жϴ�������������stm32fxxx_it.c�е��õĺ����ӿ�
  *         ���еľ��庯�����ÿ����أ�ʹ�õĺ���ָ��Pin_ExtiEnable��ָ��
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
  * @brief  ��ȡpin��ӦΪֵ
  * @param  p_pin_source: ָ��pin����ָ��
  * @retval ���ض�Ӧλ��ֵ
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
  * @brief  дpin��Ӧλֵ
  * @param  p_pin_source: ָ��pin����ָ��
  *         BitVal: д��ֵ
  * @retval ����д���Ƿ�ɹ�
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

