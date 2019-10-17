/*
  Filename: serial_to_parallel.h
  Author: shidawei			Date: 20140116
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: STM32F10X 串转并芯片SN74LV165APWR
  			   驱动程序Lib
*/
#ifndef __SERIAL_TO_PARALLEL_H__
#define __SERIAL_TO_PARALLEL_H__
#include "lib_common.h"
#include "pin.h"

typedef struct
{
	uint8_t		   init_done;
	uint8_t		   link_num;	//级连数
	t_pin_source * pin_rclk;
	t_pin_source * pin_srclr;
	t_pin_source * pin_sck;
	t_pin_source * pin_mosi;

	SPI_TypeDef * 	SPIx;
	uint32_t		SPI_RccClk;
	SPI_InitTypeDef * SPI_InitStruct;
}t_serial_to_parallel;

void SerialToParallel_Init(t_serial_to_parallel * p_serial_to_parallel);
void SerialToParallel_SendData(t_serial_to_parallel * p_serial_to_parallel, uint8_t * data);

#endif

