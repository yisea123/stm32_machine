/*
  Filename: parallel_to_serial.c
  Author: shidawei			Date: 20140114
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: STM32F10X 串转并芯片SN74LV165APWR
               驱动程序Lib
*/
#include "parallel_to_serial.h"

/****************************************************************
 GLOBAL FUNCTIONS
 ****************************************************************/
/**
  * @brief  并转串初始化
  * @param  p_parallel_to_serial: 指向并转串对象指针
  * @retval 
  */
void ParallelToSerial_Init(t_parallel_to_serial * p_parallel_to_serial)
{
	void (* fun_SpiRccClockCmd)(uint32_t RCC_APB2Periph, FunctionalState NewState);
	if(!p_parallel_to_serial->init_done)
	{
		p_parallel_to_serial->init_done = 1;
		//spi clock enable
		switch(p_parallel_to_serial->SPI_RccClk)
		{
			case RCC_APB2Periph_SPI1:
				fun_SpiRccClockCmd = RCC_APB2PeriphClockCmd;
				break;
			case RCC_APB1Periph_SPI2:
			case RCC_APB1Periph_SPI3:
				fun_SpiRccClockCmd = RCC_APB1PeriphClockCmd;
				break;
			default:
				break;
		}
		fun_SpiRccClockCmd(p_parallel_to_serial->SPI_RccClk, ENABLE);
		Pin_Init(p_parallel_to_serial->pin_inh);
		Pin_Init(p_parallel_to_serial->pin_shld);
		Pin_Init(p_parallel_to_serial->pin_sck);
		Pin_Init(p_parallel_to_serial->pin_miso);
		Pin_WriteBit(p_parallel_to_serial->pin_inh, Bit_SET);
		Pin_WriteBit(p_parallel_to_serial->pin_shld, Bit_SET);
	}
}

/**
  * @brief  并转串读取数据
  * @param  p_parallel_to_serial: 指向并转串对象指针
  *         data: 读取的数据引用
  * @retval 读取数据的长度
  */
uint8_t ParallelToSerial_GetData(t_parallel_to_serial * p_parallel_to_serial, uint8_t * data)
{
	uint16_t len = 0, i;

	//spi config
	SPI_Cmd(p_parallel_to_serial->SPIx, DISABLE);
	SPI_Init(p_parallel_to_serial->SPIx, p_parallel_to_serial->SPI_InitStruct);
	SPI_Cmd(p_parallel_to_serial->SPIx, ENABLE);
	
	Pin_WriteBit(p_parallel_to_serial->pin_shld, Bit_SET);
	Pin_WriteBit(p_parallel_to_serial->pin_shld, Bit_RESET);
	for(i=0; i<0x2F; i++);
	Pin_WriteBit(p_parallel_to_serial->pin_inh, Bit_RESET);
	for(i=0; i<0x2F; i++);
	Pin_WriteBit(p_parallel_to_serial->pin_shld, Bit_SET);
	SPI_I2S_ReceiveData(p_parallel_to_serial->SPIx); //此处的接收不能省略，否则可能导致下面的接收过程意外的提前完成，导致接受不到预期的电平值。
	//级联一次多一个byte数据
	for(len = 0; len < p_parallel_to_serial->link_num; len++)
	{
		while(SPI_I2S_GetFlagStatus(p_parallel_to_serial->SPIx, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(p_parallel_to_serial->SPIx, 0xFF);
		while(SPI_I2S_GetFlagStatus(p_parallel_to_serial->SPIx, SPI_I2S_FLAG_RXNE) == RESET);
		data[len] = SPI_I2S_ReceiveData(p_parallel_to_serial->SPIx);
		for(i = 0; i < 0x08; i++);
	}		
	Pin_WriteBit(p_parallel_to_serial->pin_inh, Bit_SET);
	Pin_WriteBit(p_parallel_to_serial->pin_shld, Bit_SET);

	return len;
}


