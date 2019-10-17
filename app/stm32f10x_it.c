/*
  Filename: stm32f10x_it.c
  Author: shidawei			Date: 20140113
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 
*/

/* Includes ------------------------------------------------------------------*/
#include "s_def.h"
#include "stm32f10x_it.h"
#include "main.h"
#include "com.h"
#include "motor.h"
#include "digtube.h"
#include "key.h"
#include "led.h"
#include "sensor.h"
#include "console.h"
#include "parser.h"
#include "dcmotor.h"


/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup USART_HalfDuplex
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	TimingDelay_Decrement();
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		Motor_Tim2IntHandler();
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	}
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		Motor_Tim3IntHandler();
		TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	}
}

/* 其余操作共用的timer */
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		Digtube_Timer_Handler();
		Key_Timer_Handler();
		Led_BreathingTimerHandler();
		//DCMotor_TimerHandler(DCMOTOR_Z2);
		TIM_ClearFlag(TIM4, TIM_FLAG_Update);
	}
}

void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		Motor_Tim5IntHandler();
		TIM_ClearFlag(TIM5, TIM_FLAG_Update);
	}
}

void USART1_IRQHandler(void)
{
	Comx_IRQHandler(MAIN_COM1);
	CMD_IfBusyReturn();
}

void USART2_IRQHandler(void)
{
	Comx_IRQHandler(MAIN_COM2);
}

void USART3_IRQHandler(void)
{
	Comx_IRQHandler(QRCODE_COM);
}

void UART4_IRQHandler(void)
{
	Comx_IRQHandler(RFID_COM);	
}

void UART5_IRQHandler(void)
{
	Comx_IRQHandler(EMBS_COM);	
}

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
}

void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		Sensor_IntHandler(S_SCANNER_UP);
	}
	EXTI_ClearITPendingBit(EXTI_Line1);
}

void EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		Sensor_IntHandler(S_Y_RESET);
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
}

void EXTI3_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		Sensor_IntHandler(S_X_RESET);
	}
	EXTI_ClearITPendingBit(EXTI_Line3);
}

void EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		Sensor_IntHandler(S_Y_LOCATE);
	}
	EXTI_ClearITPendingBit(EXTI_Line4);
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		Sensor_IntHandler(S_Z_REAR);
	}
	if(EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		Sensor_IntHandler(S_Z_FRONT);
	}
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		
	}
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		
	}
	if(EXTI_GetITStatus(EXTI_Line9) != RESET)
	{
		Sensor_IntHandler(S_X_LOCATE);
	}
	EXTI_ClearITPendingBit(EXTI_Line5|EXTI_Line6|EXTI_Line7|EXTI_Line8|EXTI_Line9);
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line10) != RESET)
	{
		
	}
	if(EXTI_GetITStatus(EXTI_Line11) != RESET)
	{
		
	}
	if(EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		
	}
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
		Sensor_IntHandler(S_HOOK_UP);
	}
	if(EXTI_GetITStatus(EXTI_Line14) != RESET)
	{
		
	}
	if(EXTI_GetITStatus(EXTI_Line15) != RESET)
	{
		
	}
	EXTI_ClearITPendingBit(EXTI_Line10|EXTI_Line11|EXTI_Line12|EXTI_Line13|EXTI_Line14|EXTI_Line15);
}

