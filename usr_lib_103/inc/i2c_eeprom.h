/*
  Filename: i2c_eeprom.h
  Author: shidawei			Date: 
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 
*/
#ifndef __I2C_EEPROM_H__
#define __I2C_EEPROM_H__
#include "lib_common.h"
#include "pin.h"

#define I2C_EEPROM_TIMEOUT			0xFFFFFF

typedef struct
{
	t_pin_source * pin_scl;
	t_pin_source * pin_sda;
	
	I2C_TypeDef *  I2Cx;
	uint32_t       I2C_RccClk;
	I2C_InitTypeDef * I2C_InitStruct;

	uint8_t        I2C_Address;
	uint32_t       MemorySize;
	uint16_t       PageSize;
	uint8_t        AddressSize;
	uint8_t        on;
}t_i2c_eeprom;

void I2cEeprom_Init(t_i2c_eeprom * p_i2c_eeprom);
uint8_t  I2cEeprom_ReadBuffer(t_i2c_eeprom * p_i2c_eeprom, uint8_t * pBuffer, uint16_t ReadAddr, uint16_t size);
uint8_t I2cEeprom_WriteBuffer(t_i2c_eeprom * p_i2c_eeprom, uint8_t * pBuffer, uint16_t WriteAddr, uint16_t size);

#endif

