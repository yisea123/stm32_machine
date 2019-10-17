/*
  Filename: flash_nvm.c
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: FLASH��������
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
  * @brief  flash�豸��ʼ��
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
  * @brief  flash�����ض�ҳ
  * @param  addrΪѡ����ҳ��ַ
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
  * @brief  flash�豸��ס������flash��Ϣ
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
  * @brief  flash�豸д
  * @param  addrΪѡ���Ŀ�ʼ��ַ
  			bufΪѡ��������
  			lenΪѡ�������ݳ���
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
			if(*flash_addr != * (buf + i))		//д��������ԭ���ݲ�ƥ��
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
  * @brief  ��flash����ȡ���ݱ���16bit����
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
	for(i = 0; i < (len/2); i++)		//��16bit��ȡ
	{
		*(buf_tmp + i) = *(addr + i);
	}
	return 1;
}

/**
  * @brief  дflash��д�����ݱ���16bit����
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
		if(len_left > FLASH_PAGE_SIZE)		//ʣ�೤�ȴ���һҳ��д��һҳ
		{
			len_once = FLASH_PAGE_SIZE;
		}
		else								//ʣ�೤��С�ڵ���һҳ��д��ʣ�೤��
		{
			len_once = len_left;
		}
		//write
		Flash_Nvm_Init();
		if(!Flash_Nvm_Erase((uint32_t )addr))
		{
			return 0;
		}
		if(!Flash_Nvm_Write(addr, buf_tmp, len_once))		//��16bitд��
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


