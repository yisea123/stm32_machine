/*
  Filename: settings.c
  Author: shidawei			Date: 20130221
  Copyright: HuNan GreatWall Information Financial Equipment Co.,Ltd.
  Description: 参数保存
 */
#include "s_def.h"
#include "string.h"
#include "settings.h"
#include "flash_nvm.h"
#include "i2c_EE.h"

t_settings settings;

const t_pos_info box_info_init_data =				//格子初始信息
{
	.pos_valid = 1,
	.pos_large = 0,
	.pos_filled = 0,
	.pos_box_is_take_away = 0,
	.pos_abnomal_recycle = 0,
	.reserve = {'\0'},
};

const t_barcode_info barcode_info_init_data =		//二维码初始信息
{
	.last_barcode = "",
	.current_barcode = "",
};

const t_machine_info machine_info_init_data =		//设备初始信息
{
	.x_last_take_away = TAKE_AWAY_NONE,
	.y_last_take_away = TAKE_AWAY_NONE,
	.pos_cnt_vaild = 36,
	.pos_cnt_invaild = 6,
	.pos_cnt_all = MAX_POS_NUM,

	.box_cnt_filled = 36,
	.package = 0,
	.box_cnt_filled_with_data = 0,
	.x_pos_adjust = 0,
	.y_pos_adjust = 0
};

/* 
  表示当前格子是否有效的配置数组
  回收箱初始状态为不可用，装入回收盒后方为可用，回收位置为1C,2C,1D,2D
*/
const unsigned char pos_valid_array[X_POS_MAX][Y_POS_MAX] =
{
/*	y0,	1,	2	3	4	5	6	7	8	9	A	B	C	D	*/
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	0,	//x0
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	0,	//x1
	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	0,	//x2
};

/**
  * @brief  获得效验和
  * @param  *ptr为选定数据内容
  			len为选定数据长度
  * @retval 效验和
  */
static uint16_t GetChecksum(uint8_t *ptr, uint16_t len)
{
	uint16_t cksum = 0;
	
	for ( ; len-- ; )
		cksum += *ptr++;

	return(cksum);
}

void Settings_Init(void)
{
	unsigned char x = 0, y = 0;
	unsigned char i;
	memset(&settings.settings_tmp.actions_data, 0, sizeof(t_actions_data));
	for(i = 0; i < BOX_NUM_MAX; i++)
	{
		settings.settings_tmp.actions_data.box_reinstall[i] = 1;		//重新上电默认为重新装入
	}
	if(!Settings_Get())
	{
		for(x = 0; x < X_POS_MAX; x++)
		{
			for(y = 0; y < Y_POS_MAX; y++)
			{
				memcpy(&settings.settings_saved.pos_info[x][y], &box_info_init_data, sizeof(t_pos_info));
				settings.settings_saved.pos_info[x][y].pos_valid = pos_valid_array[x][y];
				if(settings.settings_saved.pos_info[x][y].pos_valid)
				{
					if((y < (Y_POS_MAX - 1)) && (pos_valid_array[x][y+1] == 0))		//若其上一个格子不可用表示本身是扩展的
					{
						settings.settings_saved.pos_info[x][y].pos_large = 1;
					}
				}
			}
		}
		memcpy(&settings.settings_saved.machine_info, &machine_info_init_data, sizeof(t_machine_info));
		Settings_Save();
	}
	if(!Settings_BarcodeGet())
	{
		for(x = 0; x < X_POS_MAX; x++)
		{
			for(y = 0; y < Y_POS_MAX; y++)
			{
				memcpy(&settings.barcodes_saved.barcode_info[x][y], &barcode_info_init_data, sizeof(t_barcode_info));
			}
		}
		Settings_BarcodeSaveAll();
	}
}

uint8_t Settings_Save(void)
{
	uint16_t crc;

	settings.settings_saved.data_check = DATA_CHECK_COREECT;
	crc = GetChecksum((uint8_t *)(&settings.settings_saved), (sizeof(settings.settings_saved) - 2));
	settings.settings_saved.crc = crc;
#if USING_EEPROM_TO_SAVE
	return Eeprom_WriteBuffer((uint8_t *)(&settings.settings_saved), EEPROM_START_ADDR, sizeof(settings.settings_saved));
#else
	return Flash_Data_Write((uint8_t *)(&settings.settings_saved), FLASH_SETTINGS_START_ADDR, sizeof(settings.settings_saved));
#endif
}

uint8_t Settings_Get(void)
{
	uint16_t crc;

#if USING_EEPROM_TO_SAVE
	if(!Eeprom_ReadBuffer((uint8_t *)(&settings.settings_saved), EEPROM_START_ADDR, (uint16_t)sizeof(settings.settings_saved)))
#else
	if(!Flash_Data_Read((uint8_t *)(&settings.settings_saved), FLASH_SETTINGS_START_ADDR, (uint16_t)sizeof(settings.settings_saved)))
#endif
	{
		return 0;
	}
	if(settings.settings_saved.data_check != DATA_CHECK_COREECT)
	{
		return 0;
	}
	crc = GetChecksum((uint8_t *)(&settings.settings_saved), (sizeof(settings.settings_saved) - 2));
	if(crc != settings.settings_saved.crc)
	{
		return 0;
	}
	return 1;
}

uint8_t Settings_BarcodeSaveAll(void)
{
#if USING_EEPROM_TO_SAVE
	return Eeprom_WriteBuffer((uint8_t *)(&settings.barcodes_saved), EEPROM_BARCODES_ADDR, sizeof(t_barcodes_saved));
#else
	return Flash_Data_Write((uint8_t *)(&settings.barcodes_saved), FLASH_BARCODE_START_ADDR, sizeof(t_barcodes_saved));
#endif
}

uint8_t Settings_BarcodeSaveXY(unsigned char x, unsigned char y)
{
	/* Flash写入函数的起始地址必须为整页起始地址，除最后一页外整页写入 */
	uint32_t offset;
	uint16_t size;
	uint32_t wr_addr;
	uint8_t * buf_addr;
	uint32_t page_size;
	uint32_t start_addr;

#if USING_EEPROM_TO_SAVE
	page_size = EE_PAGE_SIZE;
	start_addr = EEPROM_BARCODES_ADDR;
#else
	page_size = FLASH_PAGE_SIZE;
	start_addr = FLASH_BARCODE_START_ADDR;
#endif

	offset = (uint32_t)(&settings.barcodes_saved.barcode_info[x][y]) - (uint32_t)(&settings.barcodes_saved.barcode_info[0][0]);
	offset = (offset / page_size) * page_size;		//地址规整
	buf_addr = (uint8_t *)(&settings.barcodes_saved.barcode_info[0][0]) + offset;
	wr_addr = start_addr + offset;
	if((offset + page_size) > sizeof(t_barcodes_saved))
	{
		size = sizeof(t_barcodes_saved) - offset;
	}
	else
	{
		size = page_size;
	}

#if USING_EEPROM_TO_SAVE
	return Eeprom_WriteBuffer(buf_addr, wr_addr,size);
#else
	return Flash_Data_Write(buf_addr, wr_addr, size);
#endif
}

uint8_t Settings_BarcodeGet(void)
{
#if USING_EEPROM_TO_SAVE
	if(!Eeprom_ReadBuffer((uint8_t *)(&settings.barcodes_saved), EEPROM_BARCODES_ADDR, (uint16_t)sizeof(settings.barcodes_saved)))
#else
	if(!Flash_Data_Read((uint8_t *)(&settings.barcodes_saved), FLASH_BARCODE_START_ADDR, (uint16_t)sizeof(settings.barcodes_saved)))
#endif
	{
		return 0;
	}

	return 1;
}

/* 恢复所有数据到原始状态 */
uint8_t Settings_ClearReset(void)
{
	unsigned char x = 0, y = 0;
	char x_adj, y_adj;

	for(y = 0; y < Y_POS_MAX; y++)
	{
		for(x = 0; x < X_POS_MAX; x++)
		{
			memcpy(&settings.settings_saved.pos_info[x][y], &box_info_init_data, sizeof(t_pos_info));
			memcpy(&settings.barcodes_saved.barcode_info[x][y], &barcode_info_init_data, sizeof(t_barcode_info));
			settings.settings_saved.pos_info[x][y].pos_valid = pos_valid_array[x][y];
			if(settings.settings_saved.pos_info[x][y].pos_valid)
			{
				if((y < (Y_POS_MAX - 1)) && (pos_valid_array[x][y+1] == 0))		//若其上一个格子不可用表示本身是扩展的
				{
					settings.settings_saved.pos_info[x][y].pos_large = 1;
				}
			}
		}
	}
	x_adj = settings.settings_saved.machine_info.x_pos_adjust;	//位置校准不清零
	y_adj = settings.settings_saved.machine_info.y_pos_adjust;
	memcpy(&settings.settings_saved.machine_info, &machine_info_init_data, sizeof(t_machine_info));
	settings.settings_saved.machine_info.x_pos_adjust = x_adj;
	settings.settings_saved.machine_info.y_pos_adjust = y_adj;
	if(Settings_BarcodeSaveAll())
	{
		return 0;
		
	}
	if(!Settings_Save())
	{
		return 0;
	}
	return 1;
}

void Settings_SetBoxValidInfo(unsigned char str[X_POS_MAX][Y_POS_MAX])
{
	unsigned char x = 0, y = 0;

	for(x = 0; x < X_POS_MAX; x++)
	{
		for(y = 0; y < Y_POS_MAX; y++)
		{
			settings.settings_saved.pos_info[x][y].pos_valid = str[x][y]&0xF;
			if(settings.settings_saved.pos_info[x][y].pos_valid)
			{
				if((y < (Y_POS_MAX - 1)) && (pos_valid_array[x][y+1] == 0))		//若其上一个格子不可用表示本身是扩展的
				{
					settings.settings_saved.pos_info[x][y].pos_large = 1;
				}
			}
		}
	}
	Settings_Save();
}


