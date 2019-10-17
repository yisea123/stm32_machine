/*
  Filename: usart.c
  Author: shidawei			Date: 20140113
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: STM32F10X ������������Lib
 */

#include "usart.h"

//functions define
/**
  * @brief  ���ڳ�ʼ��
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
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
  * @brief  ����ʹ��
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
  *         enable: 1ʹ�ܣ�0ʧ��
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
  * @brief  �����жϺ���
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
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
	if(USART_GetITStatus(usart_x, USART_IT_RXNE) != RESET)	//�жϽ���
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
	if(USART_GetITStatus(usart_x, USART_IT_TXE) != RESET)	//�жϷ���
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
  * @brief  �򴮿ڷ���һ���ֽ�����
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
  *         ch: ���͵��ֽ�����
  * @retval 1�ɹ���0ʧ�ܣ�ʧ��ԭ��Ϊ��������
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
  * @brief  �Ӵ��ڻ��һ���ֽ�����
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
  *         ch: ����ֽ���������
  * @retval 1�ɹ����һ�����ݣ�0������Ϊ��
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
  * @brief  �����ڻ���������һ���ֽ�����
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
  *         ch: �����ֽ�����
  * @retval 1�ɹ���0ʧ�ܣ�ʧ��ԭ��Ϊ��������
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
  * @brief  �����ڷ�������
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
  *         p_buf: ��������ָ��
  *         size: �������ݳ���
  * @retval 
  */
void Usart_SendBuf(t_usart_source * p_usart_source, uint8_t * p_buf, unsigned int size)
{
	DMA_InitTypeDef  DMA_InitStructure;
	uint16_t i;

	if(!size)
		return;

	if(p_usart_source->dma_clk)	//�п���DMA
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
	else	//�޿���DMA
	{
		for(i = 0; i < size; i++)
		{
			Usart_SendCh(p_usart_source, p_buf[i]);
		}
	}
}

/**
  * @brief  �ȴ��������
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
	//�ȴ�USART�ķ��ͼĴ����ա�
	while(USART_GetFlagStatus(usart_x, USART_FLAG_TC) == RESET);
}

/**
  * @brief  ���ô���CTS�������ñ����Ƿ�׼���ý���
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
  *         enable: ����CTS�ߵ͵�ƽ
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
  * @brief  ȡ�ô���RTS�����˶��Ƿ�׼���ý��ܣ�������Ҫ������
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
  * @retval 1��ʾRTS�ߵ�ƽ׼���ã�0��ʾRTS�͵�ƽ
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
  * @brief  ���ô��ڲ���
  * @param  p_usart_source: ָ�򴮿ڶ���ָ��
  *         baud_rate: ���ڲ�����
  *         word_length: �ֳ�
  *         stop_bits: ֹͣλ
  *         parity: У��λ
  *         hw_flow: hardware flow controlӲ��Э��
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

