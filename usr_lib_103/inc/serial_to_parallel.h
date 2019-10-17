/*
  Filename: serial_to_parallel.h
  Author: shidawei			Date: 20140116
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: STM32F10X ��ת��оƬSN74LV165APWR
  			   ��������Lib
*/
#ifndef __SERIAL_TO_PARALLEL_H__
#define __SERIAL_TO_PARALLEL_H__
#include "lib_common.h"
#include "pin.h"

typedef struct
{
	uint8_t		   init_done;
	uint8_t		   link_num;	//������
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

