/*
  Filename: usart.c
  Author: shidawei			Date: 20130312
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 串口驱动程序头文件
 */

#ifndef __USART_H__
#define __USART_H__
#include "lib_common.h"
#include "pin.h"

//usart buffer
typedef struct
{
	uint8_t *		p_buf;
	uint16_t		rd_index;
	uint16_t		wr_index;
	uint32_t		buf_size;
}t_usart_buf;

//USART结构定义
typedef struct
{
	t_pin_source *	tx_pin;			//tx pin
	t_pin_source *	rx_pin;			//rx pin
	t_pin_source *	cts_pin;		//cts pin
	t_pin_source *	rts_pin;		//rts pin

	t_usart_buf *	tx_buf;
	t_usart_buf *	rx_buf;

	uint32_t		usart_clk;		//clock
	USART_TypeDef * usart_type;
	uint32_t		baud_rate;
	uint16_t		word_length;
	uint16_t		stop_bits;
	uint16_t		parity;
	uint16_t		hw_flow;		//hareware flow control

	uint32_t 		NVIC_PriorityGroup;		//NVIC Priority Group
	IRQn_Type		NVIC_IRQChannel;			//IRQ
	uint8_t			NVIC_IRQChannelPreemptionPriority;
	uint8_t			NVIC_IRQChannelSubPriority;

	//tx dma
	DMA_Channel_TypeDef*	dma_channel;
	uint32_t		dma_clk;
	uint32_t		dma_flag_tcif;
	uint32_t		dma_base_addr;
}t_usart_source;

//functions declare
void Usart_Init(t_usart_source * p_usart_source);
void Usart_Enable(t_usart_source * p_usart_source, FunctionalState enable);
void Usart_IRQ_Handler(t_usart_source * p_usart_source);
uint8_t Usart_SendCh(t_usart_source * p_usart_source, uint8_t ch);
uint8_t Usart_GetCh(t_usart_source * p_usart_source, uint8_t * ch);
uint8_t Usart_UngetCh(t_usart_source * p_usart_source, uint8_t ch);
void Usart_SendBuf(t_usart_source * p_usart_source, uint8_t * p_buf, unsigned int size);
void Usart_WaitSendCompleted(t_usart_source * p_usart_source);
void Usart_SetCTS(t_usart_source * p_usart_source, FunctionalState enable);
uint8_t Usart_GetRTS(t_usart_source * p_usart_source);
void Usart_SetBaudRate(t_usart_source * p_usart_source, uint32_t baud_rate, uint16_t word_length,
	uint16_t stop_bits, uint16_t parity, uint16_t hw_flow);

#endif

