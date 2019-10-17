/*
  Filename: baud_adapt_lib.c
  Author: shidawei			Date: 2014-06-09
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 串口波特率自适应算法LIB
*/

#include "lib_common.h"
#include "baud_adapt_lib.h"
#include "main.h"

#if IAP_Download
#include "setting.h"
extern t_datas datas;
#endif


/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  根据第一个时间周期计算波特率
  * @param  p_autobaud_source: 波特率自适应算法源对象指针
  *         time: 第一个下降沿到第一个上升沿的时间，即第一个时间周期
  * @retval 
  */
static uint8_t BaudAdapt_BaudrateCalc(t_autobaud_source * p_autobaud_source, uint32_t first_period_time)
{
	uint32_t baud, baud_range;
	int i;
	uint8_t ret_val = FALSE;
	
	baud = TIME_1S/(first_period_time/p_autobaud_source->first_period_bits);
	
	for(i = 0; i < BAUD_NUM; i++)
	{
		baud_range = p_autobaud_source->p_baud_bps[i]>>4;
		if((baud>(p_autobaud_source->p_baud_bps[i]-baud_range)) && 
		   (baud<(p_autobaud_source->p_baud_bps[i]+baud_range)))
		{
			break;
		}
	}
	if(i < BAUD_NUM)	//success
	{
		p_autobaud_source->baudrate = p_autobaud_source->p_baud_bps[i];
		p_autobaud_source->baud_index = i;
		p_autobaud_source->period_unit_array[0] = 
			(first_period_time<<4)/p_autobaud_source->p_baud_1t_uint[p_autobaud_source->baud_index];
		ret_val = TRUE;
	}
	else
	{
		ret_val = FALSE;
	}

	return ret_val;
}

/**
  * @brief  串口协议预识别
  * @param  p_autobaud_source: 波特率自适应算法源对象指针
  * @retval 
  */
static uint8_t BaudAdapt_ProtocolPreRecognition(t_autobaud_source * p_autobaud_source)
{
	int i, j;
	uint32_t range, *ptime_unit;

	range = (p_autobaud_source->baudrate <= BAUD_19200_BPS) ? 8 : 12;
	for(i = p_autobaud_source->cur_protocal; i < p_autobaud_source->protocal_num; i++)
	{
		ptime_unit = p_autobaud_source->protocol_array[i].time_unit;
		
		for(j = p_autobaud_source->recd_array_cnt; j < p_autobaud_source->period_cnt - 1; j++)
		{
			if((p_autobaud_source->period_unit_array[j+1]>(ptime_unit[j]-range)) &&
				(p_autobaud_source->period_unit_array[j+1]<(ptime_unit[j]+range)))
			{
				continue;
			}
			else
			{
				p_autobaud_source->recd_array_cnt = 0;
				break;
			}	
		}
		if(j == (p_autobaud_source->period_cnt - 1))
		{
			p_autobaud_source->cur_protocal = i;
			p_autobaud_source->recd_array_cnt = j;
			return TRUE;
		}
	}
	return FALSE;
}

/**
  * @brief  串口协议识别
  * @param  p_autobaud_source: 波特率自适应算法源对象指针
  * @retval 
  */
static uint8_t BaudAdapt_ProtocolRecognition(t_autobaud_source * p_autobaud_source)
{
	int i, j;
	t_protocol usart_protocol;
	uint32_t range, *ptime_unit, uint_num;
	fun_unget_ch UngetCh;
	fun_set_baudrate SetBaudrate;
	
	range = (p_autobaud_source->baudrate <=BAUD_19200_BPS)?8:12;
	UngetCh = p_autobaud_source->fun_UngetCh;
	SetBaudrate = p_autobaud_source->fun_SetBaudrate;
	
	
	for(i = p_autobaud_source->cur_protocal; i < p_autobaud_source->protocal_num; i++)
	{
		ptime_unit = p_autobaud_source->protocol_array[i].time_unit;
		uint_num = p_autobaud_source->protocol_array[i].uint_num;
		
		for(j = 0; j<uint_num; j++)
		{
			if((p_autobaud_source->period_unit_array[j+1]>(ptime_unit[j]-range)) &&
				(p_autobaud_source->period_unit_array[j+1]<(ptime_unit[j]+range)))
			{
				continue;
			}
			else
			{
				break;
			}
		}
		if(j==uint_num)
		{
			break;
		}
	}

	if(i < p_autobaud_source->protocal_num)
	{
		usart_protocol = p_autobaud_source->protocol_array[i].protocol;
		i = 0;
		while(p_autobaud_source->p_ch[i])
		{
			UngetCh(p_autobaud_source->p_ch[i++]);
		}
		SetBaudrate(p_autobaud_source->baudrate, usart_protocol.wordlength, 
					usart_protocol.stopbits, usart_protocol.parity);
#if IAP_Download
		datas.datas_saved.com_baud=p_autobaud_source->baudrate;
		datas.datas_saved.com_parity=usart_protocol.parity;
		datas.datas_saved.com_stopbits=usart_protocol.stopbits;
		datas.datas_saved.com_wordlenth=usart_protocol.wordlength;;
#endif
		p_autobaud_source->st = AB_OK;
		return TRUE;
	}
	else
	{
		p_autobaud_source->fun_SetComEn(ENABLE);
		p_autobaud_source->st = AB_KO;
		return FALSE;
	}
}

/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  波特率自适应算法初始化
  * @param  p_autobaud_source: 指向波特率自适应算法对象指针
  * @retval 
  */
void BaudAdapt_Init(t_autobaud_source * p_autobaud_source)
{
	t_pin_source		*p_pin_source;
//	t_timer_source		*p_timer_source;

	p_pin_source = p_autobaud_source->pin;
//	p_timer_source = p_autobaud_source->timer;
	Pin_Init(p_pin_source);
//	//timer init
//	/* TIMn Periph clock enable */
//	TIM_DeInit(p_timer_source->timer);
//	if((p_timer_source->timer == TIM1)||(p_timer_source->timer == TIM8)||\
//		(p_timer_source->timer == TIM9)||(p_timer_source->timer == TIM10)||\
//		(p_timer_source->timer == TIM11)||(p_timer_source->timer == TIM15)||\
//		(p_timer_source->timer == TIM16)||(p_timer_source->timer == TIM17))
//	{
//		RCC_APB2PeriphClockCmd(p_timer_source->rcc_Periph, ENABLE);
//	}
//	else
//	{
//		RCC_APB1PeriphClockCmd(p_timer_source->rcc_Periph, ENABLE);
//	}
//	
//	/* Time base configuration */
//	TIM_TimeBaseInit(p_timer_source->timer, p_timer_source->TIM_TimeBaseStructure);
//
//	/* Clear TIMn update pending flag */
//	TIM_ClearFlag(p_timer_source->timer, TIM_FLAG_Update);
//
//	/* Configure two bits for preemption priority */
//	NVIC_PriorityGroupConfig(p_timer_source->NVIC_PriorityGroup);
//
//	/* Enable the TIMn Interrupt */
//	NVIC_Init(p_timer_source->NVIC_InitStructure);
//
//	/* Enable TIMn	Update interrupt */
//	TIM_ITConfig(p_timer_source->timer, TIM_IT_Update, ENABLE);
//	/* auto reload */
//	TIM_ARRPreloadConfig(p_timer_source->timer, ENABLE);
//	/* TIMn enable counter */
//	TIM_Cmd(p_timer_source->timer, DISABLE);
}

/**
  * @brief  波特率自适应算法使能
  * @param  p_autobaud_source: 波特率自适应算法源对象指针
  *         handler: 中断处理函数
  *         enable: 使能或使能
  * @retval 
  */
void BaudAdapt_Enable(t_autobaud_source * p_autobaud_source, fun_exti_handler handler, FunctionalState enable)
{
	t_pin_source		*p_pin_source;
	t_timer_source		*p_timer_source;
	
	p_pin_source = p_autobaud_source->pin;
	p_timer_source = p_autobaud_source->timer;
	if((handler) && (enable))
	{
		/* TIMn Periph clock enable */
		TIM_DeInit(p_timer_source->timer);
		if((p_timer_source->timer == TIM1)||(p_timer_source->timer == TIM8)||\
			(p_timer_source->timer == TIM9)||(p_timer_source->timer == TIM10)||\
			(p_timer_source->timer == TIM11)||(p_timer_source->timer == TIM15)||\
			(p_timer_source->timer == TIM16)||(p_timer_source->timer == TIM17))
		{
			RCC_APB2PeriphClockCmd(p_timer_source->rcc_Periph, ENABLE);
		}
		else
		{
			RCC_APB1PeriphClockCmd(p_timer_source->rcc_Periph, ENABLE);
		}
		/* Time base configuration */
		TIM_TimeBaseInit(p_timer_source->timer, p_timer_source->TIM_TimeBaseStructure);
		/* Clear TIMn update pending flag */
		TIM_ClearFlag(p_timer_source->timer, TIM_FLAG_Update);
		/* Configure two bits for preemption priority */
		NVIC_PriorityGroupConfig(p_timer_source->NVIC_PriorityGroup);
		/* Enable the TIMn Interrupt */
		NVIC_Init(p_timer_source->NVIC_InitStructure);
		/* Enable TIMn	Update interrupt */
		TIM_ITConfig(p_timer_source->timer, TIM_IT_Update, enable);
		/* auto reload */
		TIM_ARRPreloadConfig(p_timer_source->timer, enable);
		/* TIMn enable counter */
		TIM_Cmd(p_timer_source->timer, enable);
		Pin_ExtiEnable(p_pin_source, handler, EXTI_Trigger_Rising_Falling, enable);
		p_autobaud_source->fun_SetComEn((FunctionalState)!enable);
	}
	else
	{
		Pin_ExtiEnable(p_pin_source, handler, EXTI_Trigger_Rising_Falling, DISABLE);
		TIM_Cmd(p_timer_source->timer, DISABLE);
		p_autobaud_source->fun_SetComEn(ENABLE);
	}
}

/**
  * @brief  查询波特率自适应是否完成
  * @param  p_autobaud_source: 波特率自适应算法源对象指针
  * @retval 完成返回1，未完成返回0
  */
uint8_t BaudAdapt_GetDone(t_autobaud_source * p_autobaud_source)
{
	uint8_t ret_val = FALSE;
	if(p_autobaud_source->st == AB_NONE)
	{
		ret_val = FALSE;
	}
	else
	{
		ret_val = TRUE;
	}
	return ret_val;
}

/**
  * @brief  波特率自适应算法中断处理函数
  * @param  p_autobaud_source: 波特率自适应算法源对象指针
  * @retval 
  */
void BaudAdapt_Handler(t_autobaud_source * p_autobaud_source)
{
	uint32_t TimCounter, TimeVal, TIM_Period;
	uint8_t Level;
	t_pin_source * p_pin_src;
	t_timer_source * p_timer_src;

	p_pin_src = p_autobaud_source->pin;
	p_timer_src = p_autobaud_source->timer;
	TIM_Period = p_timer_src->TIM_TimeBaseStructure->TIM_Period;
	Level = Pin_ReadBit(p_pin_src);

	if(p_autobaud_source->phase != PHASE_START)	//第一个边沿定时器未触发，不取值，触发后取值
	{
		TimCounter = TIM_GetCounter(p_timer_src->timer);
		//升序与降序不一致
		if(p_timer_src->TIM_TimeBaseStructure->TIM_CounterMode == TIM_CounterMode_Up)
		{
			TimeVal = (TimCounter > (p_autobaud_source->last_timecnt)) ?\
					  (TimCounter - p_autobaud_source->last_timecnt) :\
					  (TIM_Period + TimCounter - p_autobaud_source->last_timecnt + 1);
		}
		else
		{
			TimeVal = (TimCounter < (p_autobaud_source->last_timecnt)) ?\
				  (p_autobaud_source->last_timecnt - TimCounter) :\
				  (TIM_Period + p_autobaud_source->last_timecnt - TimCounter + 1);
		}
		p_autobaud_source->last_timecnt = TimCounter;
	}

	switch(p_autobaud_source->phase)
	{
		case PHASE_START:				//first trigger, falling
			if(Level == (uint8_t)Bit_RESET)
			{
				TIM_Cmd(p_timer_src->timer, ENABLE);
				p_autobaud_source->last_timecnt = TIM_GetCounter(p_timer_src->timer);
				p_autobaud_source->period_cnt = 0;
				p_autobaud_source->phase = PHASE_5T;
			}
			break;
		case PHASE_5T:
			if(Level == Bit_SET)		//first rising
			{
				p_autobaud_source->period_array[p_autobaud_source->period_cnt++] = TimeVal;
				if(!BaudAdapt_BaudrateCalc(p_autobaud_source, p_autobaud_source->period_array[0]))
				{
					p_autobaud_source->phase = PHASE_START;
					TIM_Cmd(p_timer_src->timer, DISABLE);
					p_autobaud_source->period_cnt--;
				}
				else
				{
					p_autobaud_source->phase = PHASE_N;
				}
			}
			break;
		case PHASE_N:
			p_autobaud_source->period_array[p_autobaud_source->period_cnt] = TimeVal;
			p_autobaud_source->period_unit_array[p_autobaud_source->period_cnt] = \
				       (TimeVal<<4)/p_autobaud_source->p_baud_1t_uint[p_autobaud_source->baud_index];
			p_autobaud_source->period_cnt++;
			if(!BaudAdapt_ProtocolPreRecognition(p_autobaud_source))
			{
				BaudAdapt_Enable(p_autobaud_source, NULL, DISABLE);
				p_autobaud_source->fun_SetComEn(ENABLE);
				p_autobaud_source->st = AB_KO;
				return;
			}
			if(p_autobaud_source->period_cnt == p_autobaud_source->period_num)
			{
				BaudAdapt_ProtocolRecognition(p_autobaud_source);
				BaudAdapt_Enable(p_autobaud_source, NULL, DISABLE);
				p_autobaud_source->fun_SetComEn(ENABLE);
			}
			break;
		default:
			break;
	}

	return;
}


