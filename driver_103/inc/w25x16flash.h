#ifndef W25X16FLASH_H_
#define W25X16FLASH_H_
#include"pin.h"
//#include "config.h"
#include <stdint.h>



#define FLASH_ID 			0xEF15
#define W25x_WriteEnable 	0x06
#define W25x_WriteDisable	0x04
#define W25x_ReadEnable		0x03
#define W25x_ReadID   		0x90
#define W25x_ReadStatus 	0x05
#define W25x_BlockErase 	0xD8
#define W25x_SectorErase    0x20 
#define W25x_FullErase 		0xC7
#define W25x_PageProgram    0x02 
#define w25x_PowerDown		0xAB

/*第1.2块区存储版本信息*/
//#define FW_info_Addr		0x0100;//版本存储记录及版本长度存储区(备份固件版本长度和校检)
//#define FW_Start_Addr		0x1000;//版本存储起始地址第2个扇区开始写入版本

//第3块区
#define FW_flag_Addr		0x40000;//存储串口设置及APP跳转IAP标记,第4块区存储串口和标记信息

typedef struct 
{
	SPI_TypeDef* SPIX;
	SPI_InitTypeDef* SPI_InitStruct;
	t_pin_source*  MISO;
	t_pin_source*  SCK;
	t_pin_source*  MOSI;
	t_pin_source*  CS;
	uint8_t * pBuffer;
}t_W25x16_soursce;

void drv_w25x16_init(t_W25x16_soursce* w25x16);
uint16_t drv_w25x16_ReadID(t_W25x16_soursce* w25x16);

void drv_w25x16_Write(t_W25x16_soursce* w25x16,uint32_t Addr,uint32_t Numbyte);
void drv_w25x16_Read(t_W25x16_soursce* w25x16,uint32_t Addr,uint32_t Numbyte);
uint8_t drv_w25x16_Erase_Block(t_W25x16_soursce* w25x16,uint32_t Addr);
uint8_t drv_w25x16_Erase_Sector(t_W25x16_soursce* w25x16,uint32_t Addr);


#endif

