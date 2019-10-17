/*
  Filename: flash_nvm.h
  Author: xj			Date: 20150512
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: FLASH驱动程序
 */

#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f10x.h"

#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
	#define FLASH_PAGE_SIZE    ((uint16_t)0x800)
#else
	#define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#endif

//define the address where the nvm loaded from, program size: 100K, settings size: 28K ~ 
#define FLASH_SETTINGS_START_ADDR		((uint32_t)0x08019000)
#define FLASH_BARCODE_START_ADDR		((uint32_t)0x08019800)

/* Exported functions ------------------------------------------------------- */
extern uint8_t Flash_Data_Read(uint8_t* data , uint32_t address, uint16_t datalen);
extern uint8_t Flash_Data_Write(uint8_t * buf, uint32_t address, uint16_t len);

#endif

