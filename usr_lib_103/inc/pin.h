/*
  Filename: pin.h
  Author: shidawei			Date: 20140116
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 管脚配置Lib
*/
#ifndef __PIN_H__
#define __PIN_H__
#include "lib_common.h"

typedef void(*fun_exti_handler)(void);
typedef enum
{
	PIN_RESET=0,
	PIN_SET,
	PIN_ERROR
}t_pin_bit;

typedef struct
{
	uint8_t					EXTI_PortSource;
	uint8_t					EXTI_PinSource;
	uint32_t				EXTI_Line;
	EXTITrigger_TypeDef		EXTI_Trigger;
	uint32_t 				NVIC_PriorityGroup;
	uint8_t					NVIC_IRQChannel;
	uint8_t					NVIC_IRQChannelPreemptionPriority;
	uint8_t					NVIC_IRQChannelSubPriority;
	fun_exti_handler        fun_ExtiHandler;
}t_exti_source;
	
typedef struct	 //用来描述一个GPIO口属性的结构
{
	GPIO_TypeDef*		GPIO_Type;
	uint32_t			GPIO_Rcc;
	uint16_t			GPIO_Pin;
	GPIOMode_TypeDef    GPIO_Mode;
  	GPIOSpeed_TypeDef   GPIO_Speed;
	t_exti_source*      GPIO_Exti;
}t_pin_source;

//functions declare
extern void Pin_Init(t_pin_source* p_pin_source);
extern void Pin_ExtiEnable(t_pin_source* p_pin_source, fun_exti_handler handler,
							EXTITrigger_TypeDef trigger, FunctionalState enable);
extern uint8_t Pin_ReadBit(t_pin_source* p_pin_source);
extern uint8_t Pin_WriteBit(t_pin_source* p_pin_source, BitAction BitVal);
extern void Pin_ExtiHandler(t_pin_source* p_pin_source);

#endif
