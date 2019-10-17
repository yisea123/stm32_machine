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

/*��1.2�����洢�汾��Ϣ*/
//#define FW_info_Addr		0x0100;//�汾�洢��¼���汾���ȴ洢��(���ݹ̼��汾���Ⱥ�У��)
//#define FW_Start_Addr		0x1000;//�汾�洢��ʼ��ַ��2��������ʼд��汾

//��3����
#define FW_flag_Addr		0x40000;//�洢�������ü�APP��תIAP���,��4�����洢���ںͱ����Ϣ

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

