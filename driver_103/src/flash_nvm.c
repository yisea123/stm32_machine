/*
  Filename: flash_nvm.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: FLASH驱动程序
 */
//#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "flash_nvm.h"
#include <string.h>

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
//data define

/**
  * @brief  flash设备初始化
  * @param  
  * @retval 
  */
static void Flash_Nvm_Init(void)
{
	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Clear pending flags (if any) */  
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
}

/**
  * @brief  flash擦除特定页
  * @param  addr为选定的页地址
  * @retval 
  */
static unsigned char Flash_Nvm_Erase(uint32_t addr)
{
	unsigned char ret_val = 1;

	if ((FLASH_ErasePage(addr) != FLASH_COMPLETE))
	{
		ret_val = 0;
	}
	return ret_val;
}

/**
  * @brief  flash设备锁住，保护flash信息
  * @param  
  * @retval 
  */
static void Flash_Nvm_Lock(void)
{
	/* Lock the Flash to disable the flash control register access (recommended
	   to protect the FLASH memory against possible unwanted operation) *********/
	FLASH_Lock();
}

/**
  * @brief  flash设备写
  * @param  addr为选定的开始地址
  			buf为选定的数据
  			len为选定的数据长度
  * @retval 
  */
static uint8_t Flash_Nvm_Write(uint16_t * addr, uint16_t * buf, uint16_t len)
{
	unsigned int i;
	uint16_t * flash_addr = addr;

	for(i = 0; i < (len/2); i++)
	{
		if(FLASH_ProgramHalfWord((uint32_t)flash_addr, *(uint16_t*)(buf + i)) == FLASH_COMPLETE)
		{
			if(*flash_addr != * (buf + i))		//写入数据与原数据不匹配
			{
				return 0;
			}
			flash_addr++;
		}
		else	//write fail
		{
			return 0;
		}
	}

	return 1;
}

/**
  * @brief  读flash，读取数据必须16bit对齐
  * @param  
  * @retval 
  */
uint8_t Flash_Data_Read(uint8_t * buf, uint32_t address, uint16_t len)
{
	uint16_t i;
	uint32_t startAddress;
	uint16_t * buf_tmp;
	__IO uint16_t * addr;

	startAddress = address;
	addr = (uint16_t *)startAddress;
	buf_tmp = (uint16_t *)buf;
	for(i = 0; i < (len/2); i++)		//按16bit读取
	{
		*(buf_tmp + i) = *(addr + i);
	}
	return 1;
}

/**
  * @brief  写flash，写入数据必须16bit对齐
  * @param  
  * @retval 
  */
uint8_t Flash_Data_Write(uint8_t * buf, uint32_t address, uint16_t len)
{
	unsigned char i = 0;
	unsigned char sector_num;
	uint16_t* startaddr;
	uint16_t* addr;
	uint16_t len_once, len_left;
	uint16_t * buf_tmp;

	startaddr = (uint16_t *)address;
	addr = startaddr;
	len_left = len;
	buf_tmp = (uint16_t *)buf;
	sector_num = len / FLASH_PAGE_SIZE;
	if(len % FLASH_PAGE_SIZE)
	{
		sector_num++;
	}
	for(i = 0; i < sector_num; i++)
	{
		if(len_left > FLASH_PAGE_SIZE)		//剩余长度大于一页，写入一页
		{
			len_once = FLASH_PAGE_SIZE;
		}
		else								//剩余长度小于等于一页，写入剩余长度
		{
			len_once = len_left;
		}
		//write
		Flash_Nvm_Init();
		if(!Flash_Nvm_Erase((uint32_t )addr))
		{
			return 0;
		}
		if(!Flash_Nvm_Write(addr, buf_tmp, len_once))		//按16bit写入
		{
			return 0;
		}
		Flash_Nvm_Lock();
		buf_tmp += (len_once/2);
		addr += (len_once/2);
		len_left -= len_once;
	}
	return 1;
}


