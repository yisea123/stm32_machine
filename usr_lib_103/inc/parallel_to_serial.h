/*
  Filename: parallel_to_serial.h
  Author: shidawei			Date: 20140114
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 串转并芯片驱动程序头文件
*/
#ifndef __PARALLEL_TO_SERIAL_H__
#define __PARALLEL_TO_SERIAL_H__
#include "lib_common.h"
#include "pin.h"

typedef struct
{
	uint8_t			init_done;
	uint8_t			link_num;		//串转并级联数
	t_pin_source * 	pin_inh;
	t_pin_source * 	pin_shld;
	t_pin_source * 	pin_sck;
	t_pin_source * 	pin_miso;

	SPI_TypeDef * 	SPIx;
	uint32_t		SPI_RccClk;
	SPI_InitTypeDef * SPI_InitStruct;
}t_parallel_to_serial;

void ParallelToSerial_Init(t_parallel_to_serial * p_parallel_to_serial);
uint8_t ParallelToSerial_GetData(t_parallel_to_serial * p_parallel_to_serial, uint8_t * data);


#endif

