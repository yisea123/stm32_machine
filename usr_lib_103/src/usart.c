/*
  Filename: usart.c
  Author: shidawei			Date: 20140113
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: STM32F10X 串口驱动程序Lib
 */

#include "usart.h"

//functions define
/**
  * @brief  串口初始化
  * @param  p_usart_source: 指向串口对象指针
  * @retval 
  */
void Usart_Init(t_usart_source * p_usart_source)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	void (*USARTx_Clk_Init)(uint32_t RCC_APB1Periph, FunctionalState NewState);

	//RCC Conf
	if(p_usart_source->usart_clk == RCC_APB2Periph_USART1)
	{
		USARTx_Clk_Init = RCC_APB2PeriphClockCmd;
	}
	else
	{
		USARTx_Clk_Init = RCC_APB1PeriphClockCmd;
	}
	/* DMA clock enable */
	if(p_usart_source->dma_clk)
		RCC_AHBPeriphClockCmd(p_usart_source->dma_clk, ENABLE);
	/* Enable GPIOA and USART clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	USARTx_Clk_Init(p_usart_source->usart_clk, ENABLE);

	//gpio Conf
	Pin_Init(p_usart_source->tx_pin);
	Pin_Init(p_usart_source->rx_pin);
	if(p_usart_source->cts_pin != NULL)
	{
		Pin_Init(p_usart_source->cts_pin);
	}
	if(p_usart_source->rts_pin != NULL)
	{
		Pin_Init(p_usart_source->rts_pin);
	}

	//NVIC
	NVIC_PriorityGroupConfig(p_usart_source->NVIC_PriorityGroup);
	NVIC_InitStructure.NVIC_IRQChannel = p_usart_source->NVIC_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = p_usart_source->NVIC_IRQChannelPreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = p_usart_source->NVIC_IRQChannelSubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* USART1 configuration ------------------------------------------------------*/
	USART_InitStructure.USART_BaudRate = p_usart_source->baud_rate;
	USART_InitStructure.USART_WordLength = p_usart_source->word_length;
	USART_InitStructure.USART_StopBits = p_usart_source->stop_bits;
	USART_InitStructure.USART_Parity = p_usart_source->parity;
	USART_InitStructure.USART_HardwareFlowControl = p_usart_source->hw_flow;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(p_usart_source->usart_type, &USART_InitStructure);

	/* Interrupt Enable */
	USART_ITConfig(p_usart_source->usart_type, USART_IT_RXNE, ENABLE);

	/* Enable USART */
	USART_Cmd(p_usart_source->usart_type, ENABLE);
}

/**
  * @brief  串口使能
  * @param  p_usart_source: 指向串口对象指针
  *         enable: 1使能，0失能
  * @retval 
  */
void Usart_Enable(t_usart_source * p_usart_source, FunctionalState enable)
{
	if(!enable)
	{
		USART_ITConfig(p_usart_source->usart_type, USART_IT_RXNE, DISABLE);
		/* Disable USART */
		USART_Cmd(p_usart_source->usart_type, DISABLE);
	}
	else
	{
		USART_ITConfig(p_usart_source->usart_type, USART_IT_RXNE, ENABLE);
		/* Disable USART */
		USART_Cmd(p_usart_source->usart_type, ENABLE);
	}
}

/**
  * @brief  串口中断函数
  * @param  p_usart_source: 指向串口对象指针
  * @retval 
  */
void Usart_IRQ_Handler(t_usart_source * p_usart_source)
{
	t_usart_buf * p_rx_buf;
	t_usart_buf * p_tx_buf;
	USART_TypeDef* usart_x;

	p_tx_buf = p_usart_source->tx_buf;
	p_rx_buf = p_usart_source->rx_buf;
	usart_x = p_usart_source->usart_type;
	if(USART_GetITStatus(usart_x, USART_IT_RXNE) != RESET)	//中断接收
  	{
		/* Clear the USART1 Receive interrupt */
		USART_ClearITPendingBit(usart_x, USART_IT_RXNE);
		*(p_rx_buf->p_buf + p_rx_buf->wr_index) = USART_ReceiveData(usart_x);
		p_rx_buf->wr_index = (p_rx_buf->wr_index + 1) % p_rx_buf->buf_size;
        if(((p_rx_buf->wr_index + 1) % p_rx_buf->buf_size) == p_rx_buf->rd_index)	//buffer full, disable rx
		{
  			/* Disable the USART Receive interrupt */
  			USART_ITConfig(usart_x, USART_IT_RXNE, DISABLE);
		}
  	}
	if(USART_GetITStatus(usart_x, USART_IT_TXE) != RESET)	//中断发送
	{
		/* Clear the USART1 transmit interrupt */
		USART_ClearITPendingBit(usart_x, USART_IT_TXE);
		if(p_tx_buf->rd_index == p_tx_buf->wr_index)	//buffer empty, disable tx
		{
			/* Disable the USART1 Transmit interrupt */
			USART_ITConfig(usart_x, USART_IT_TXE, DISABLE);
		}
		/* Write one byte to the transmit data register */
		USART_SendData(usart_x, *(p_tx_buf->p_buf + p_tx_buf->rd_index));
		p_tx_buf->rd_index = (p_tx_buf->rd_index + 1) % p_tx_buf->buf_size;
		if(p_tx_buf->rd_index == p_tx_buf->wr_index)	//buffer empty, disable tx
		{
			/* Disable the USART1 Transmit interrupt */
			USART_ITConfig(usart_x, USART_IT_TXE, DISABLE);
		}
	}
}

/**
  * @brief  向串口发送一个字节数据
  * @param  p_usart_source: 指向串口对象指针
  *         ch: 发送的字节数据
  * @retval 1成功，0失败，失败原因为缓冲区满
  */
uint8_t Usart_SendCh(t_usart_source * p_usart_source, uint8_t ch)
{
	t_usart_buf * p_tx_buf;
	USART_TypeDef* usart_x;

	p_tx_buf = p_usart_source->tx_buf;
	usart_x = p_usart_source->usart_type;
	if(((p_tx_buf->wr_index + 1) % p_tx_buf->buf_size)== p_tx_buf->rd_index)	//buffer full
	{
		return FALSE;
	}

	*(p_tx_buf->p_buf + p_tx_buf->wr_index) = ch;	//write data
	if(p_tx_buf->wr_index == p_tx_buf->rd_index)	//resume from buffer empty
	{
		p_tx_buf->wr_index = (p_tx_buf->wr_index + 1) % p_tx_buf->buf_size;
		USART_ITConfig(usart_x, USART_IT_TXE, ENABLE);
	}
	else
	{
		p_tx_buf->wr_index = (p_tx_buf->wr_index + 1) % p_tx_buf->buf_size;
		USART_ITConfig(usart_x, USART_IT_TXE, ENABLE);
	}

	return TRUE;
}

/**
  * @brief  从串口获得一个字节数据
  * @param  p_usart_source: 指向串口对象指针
  *         ch: 获得字节数据引用
  * @retval 1成功获得一个数据，0缓冲区为空
  */
uint8_t Usart_GetCh(t_usart_source * p_usart_source, uint8_t * ch)
{
	t_usart_buf * p_rx_buf;
	USART_TypeDef* usart_x;

	p_rx_buf = p_usart_source->rx_buf;
	usart_x = p_usart_source->usart_type;
	if(p_rx_buf->rd_index == p_rx_buf->wr_index)	//buffer empty
	{
		return FALSE;
	}

	*ch = *(p_rx_buf->p_buf + p_rx_buf->rd_index);
	if(((p_rx_buf->wr_index + 1) % p_rx_buf->buf_size) == p_rx_buf->rd_index)	//resume from buffer full
	{
		p_rx_buf->rd_index = (p_rx_buf->rd_index + 1) % p_rx_buf->buf_size;
		/* Enable the USART Receive interrupt */
		USART_ITConfig(usart_x, USART_IT_RXNE, ENABLE);
	}
	else
	{
		p_rx_buf->rd_index = (p_rx_buf->rd_index + 1) % p_rx_buf->buf_size;
	}

	return TRUE;
}

/**
  * @brief  往串口缓冲区回退一个字节数据
  * @param  p_usart_source: 指向串口对象指针
  *         ch: 回退字节数据
  * @retval 1成功，0失败，失败原因为缓冲区满
  */
uint8_t Usart_UngetCh(t_usart_source * p_usart_source, uint8_t ch)
{
	t_usart_buf * p_rx_buf;

	p_rx_buf = p_usart_source->rx_buf;

	if(((p_rx_buf->wr_index + 1) % p_rx_buf->buf_size) == p_rx_buf->rd_index)//buffer is full
		return 0;
		
	*(p_rx_buf->p_buf + p_rx_buf->wr_index) = ch;
	p_rx_buf->wr_index = (p_rx_buf->wr_index + 1) % p_rx_buf->buf_size;
		
	return 1;
}

/**
  * @brief  往串口发送数据
  * @param  p_usart_source: 指向串口对象指针
  *         p_buf: 发送数据指针
  *         size: 发送数据长度
  * @retval 
  */
void Usart_SendBuf(t_usart_source * p_usart_source, uint8_t * p_buf, unsigned int size)
{
	DMA_InitTypeDef  DMA_InitStructure;
	uint16_t i;

	if(!size)
		return;

	if(p_usart_source->dma_clk)	//有可用DMA
	{
		DMA_DeInit(p_usart_source->dma_channel);
		DMA_InitStructure.DMA_PeripheralBaseAddr = p_usart_source->dma_base_addr;
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)p_buf;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_InitStructure.DMA_BufferSize = size;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(p_usart_source->dma_channel, &DMA_InitStructure);

		/* Enable DMA TX request*/
		USART_DMACmd(p_usart_source->usart_type, USART_DMAReq_Tx, ENABLE);

		/* Enable USARTy DMA TX Channel */
		DMA_Cmd(p_usart_source->dma_channel, ENABLE);

		USART_Cmd(p_usart_source->usart_type, ENABLE);

		/* Wait until DMA_Channel Transfer Complete */
		while (DMA_GetFlagStatus(p_usart_source->dma_flag_tcif) == RESET)
		{
		}
		/* Enable USARTy DMA TX request */
		USART_DMACmd(p_usart_source->usart_type, USART_DMAReq_Tx, DISABLE);
	}
	else	//无可用DMA
	{
		for(i = 0; i < size; i++)
		{
			Usart_SendCh(p_usart_source, p_buf[i]);
		}
	}
}

/**
  * @brief  等待发送完成
  * @param  
  * @retval 
  */
void Usart_WaitSendCompleted(t_usart_source * p_usart_source)
{
	t_usart_buf * p_tx_buf;
	USART_TypeDef* usart_x;
	
	p_tx_buf = p_usart_source->tx_buf;
	usart_x = p_usart_source->usart_type;
	while(p_tx_buf->wr_index != p_tx_buf->rd_index);
	//等待USART的发送寄存器空。
	while(USART_GetFlagStatus(usart_x, USART_FLAG_TC) == RESET);
}

/**
  * @brief  设置串口CTS，即设置本端是否准备好接收
  * @param  p_usart_source: 指向串口对象指针
  *         enable: 设置CTS高低电平
  * @retval 
  */
void Usart_SetCTS(t_usart_source * p_usart_source, FunctionalState enable)
{
	if(p_usart_source->cts_pin != NULL)
	{
		if(enable)
		{
			Pin_WriteBit(p_usart_source->cts_pin, Bit_SET);
		}
		else
		{
			Pin_WriteBit(p_usart_source->cts_pin, Bit_RESET);
		}
	}
}

/**
  * @brief  取得串口RTS，即彼端是否准备好接受，本端需要发送了
  * @param  p_usart_source: 指向串口对象指针
  * @retval 1表示RTS高电平准备好，0表示RTS低电平
  */
uint8_t Usart_GetRTS(t_usart_source * p_usart_source)
{
	uint8_t ret_val = 1;
	if(p_usart_source->rts_pin != NULL)
	{
		ret_val = Pin_ReadBit(p_usart_source->rts_pin);
	}

	return ret_val;
}

/**
  * @brief  设置串口参数
  * @param  p_usart_source: 指向串口对象指针
  *         baud_rate: 串口波特率
  *         word_length: 字长
  *         stop_bits: 停止位
  *         parity: 校验位
  *         hw_flow: hardware flow control硬件协议
  * @retval 
  */
void Usart_SetBaudRate(t_usart_source * p_usart_source, uint32_t baud_rate, uint16_t word_length,
	uint16_t stop_bits, uint16_t parity, uint16_t hw_flow)
{
	USART_InitTypeDef USART_InitStructure;
	
	p_usart_source->baud_rate = baud_rate;
	p_usart_source->word_length = word_length;
	p_usart_source->stop_bits = stop_bits;
	p_usart_source->parity = parity;
	p_usart_source->hw_flow = hw_flow;
	/* USART1 configuration ------------------------------------------------------*/
	USART_InitStructure.USART_BaudRate = p_usart_source->baud_rate;
	USART_InitStructure.USART_WordLength = p_usart_source->word_length;
	USART_InitStructure.USART_StopBits = p_usart_source->stop_bits;
	USART_InitStructure.USART_Parity = p_usart_source->parity;
	USART_InitStructure.USART_HardwareFlowControl = p_usart_source->hw_flow;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(p_usart_source->usart_type, &USART_InitStructure);
}

