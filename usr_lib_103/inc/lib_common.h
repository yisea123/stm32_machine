/*
  Filename: lib_common.h
  Author: shidawei			Date: 20140116
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: Lib公共头文件
*/
#ifndef __LIB_COMMON_H__
#define __LIB_COMMON_H__

/* Uncomment/Comment the line below to enable/disable peripheral header file inclusion */
#include "stm32f10x.h"
//#include "stm32f10x_adc.h"
//#include "stm32f10x_bkp.h"
//#include "stm32f10x_can.h"
//#include "stm32f10x_cec.h"
//#include "stm32f10x_crc.h"
//#include "stm32f10x_dac.h"
//#include "stm32f10x_dbgmcu.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_exti.h"
//#include "stm32f10x_flash.h"
//#include "stm32f10x_fsmc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
//#include "stm32f10x_iwdg.h"
//#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
//#include "stm32f10x_rtc.h"
//#include "stm32f10x_sdio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
//#include "stm32f10x_wwdg.h"
#include "misc.h"

//macro
#define TRUE		1
#define FALSE		0
#define NULL		0

//timer source
typedef struct
{
	TIM_TypeDef * 		timer;
	uint32_t			rcc_Periph;
	TIM_TimeBaseInitTypeDef * TIM_TimeBaseStructure;
	uint32_t 			NVIC_PriorityGroup;
	NVIC_InitTypeDef * 	NVIC_InitStructure;
}t_timer_source;




#endif

