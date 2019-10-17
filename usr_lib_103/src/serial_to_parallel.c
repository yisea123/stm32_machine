/*
  Filename: serial_to_parallel.c
  Author: shidawei			Date: 20140115
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: STM32F10X 串转并芯片74HC595PWR
  			   驱动程序Lib
*/
#include "serial_to_parallel.h"
#include <stdlib.h>
#include <string.h>

/****************************************************************
 GLOBAL FUNCTIONS
 ****************************************************************/
/**
  * @brief  串转并初始化
  * @param  
  * @retval 
  */
void SerialToParallel_Init(t_serial_to_parallel * p_serial_to_parallel)
{
	uint8_t * data;
	void (* fun_SpiRccClockCmd)(uint32_t RCC_APB2Periph, FunctionalState NewState);
	if(!p_serial_to_parallel->init_done)
	{
		p_serial_to_parallel->init_done = 1;
		//spi clock enable
		switch(p_serial_to_parallel->SPI_RccClk)
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
		fun_SpiRccClockCmd(p_serial_to_parallel->SPI_RccClk, ENABLE);
		Pin_Init(p_serial_to_parallel->pin_rclk);
		Pin_Init(p_serial_to_parallel->pin_srclr);
		Pin_Init(p_serial_to_parallel->pin_sck);
		Pin_Init(p_serial_to_parallel->pin_mosi);
		//初始化时清零所有输出
		data = malloc(p_serial_to_parallel->link_num);	//根据级联数申请内存
		if(data != NULL)
		{
			memset(data, 1, p_serial_to_parallel->link_num);	//设置所有数据为0
			SerialToParallel_SendData(p_serial_to_parallel, data);
			free(data);
		}
	}
}

/**
  * @brief  串转并发送数据
  * @param  p_serial_to_parallel: 指向串转并对象指针
  *         data: 发送的数据指针，数据长度根据串转并级联数计算无级联为1个字节，级联一次为2个字节一次类推
  * @retval 
  */
void SerialToParallel_SendData(t_serial_to_parallel * p_serial_to_parallel, uint8_t * data)
{
	uint8_t i = 0, j = 0;
	SPI_Cmd(p_serial_to_parallel->SPIx, DISABLE);
	SPI_Init(p_serial_to_parallel->SPIx, p_serial_to_parallel->SPI_InitStruct);
	SPI_Cmd(p_serial_to_parallel->SPIx, ENABLE);

	Pin_WriteBit(p_serial_to_parallel->pin_rclk, Bit_RESET);
	Pin_WriteBit(p_serial_to_parallel->pin_srclr, Bit_RESET);
	Pin_WriteBit(p_serial_to_parallel->pin_srclr, Bit_SET);
	SPI_I2S_ReceiveData(p_serial_to_parallel->SPIx);	//清空接收缓冲
	for(i = 0;i < (p_serial_to_parallel->link_num); i++)
	{													
		while(SPI_I2S_GetFlagStatus(p_serial_to_parallel->SPIx, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(p_serial_to_parallel->SPIx, data[(p_serial_to_parallel->link_num - 1) - i]);  //先输出value数组的最后一个字节，倒序输出最后输出value数组的第一个字节
		while(SPI_I2S_GetFlagStatus(p_serial_to_parallel->SPIx, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_ReceiveData(p_serial_to_parallel->SPIx);
		for(j=0;j<0x08;j++);
	}
	Pin_WriteBit(p_serial_to_parallel->pin_rclk, Bit_SET);
}


