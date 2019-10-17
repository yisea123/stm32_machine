/*
  Filename: settings.h
  Author: shidawei			Date: 20130221
  Copyright: HuNan GreatWall Information Financial Equipment Co.,Ltd.
  Description: 参数保存
 */

#ifndef __SETTINGS_H__
#define __SETTINGS_H__
#include "stm32f10x.h"
#include "string.h"
#include "actions.h"
#include "s_def.h"

#define MAX_POS_NUM		(X_POS_MAX * Y_POS_MAX)

#define DATA_CHECK_COREECT		0x4757	//"GW"
#define TAKE_AWAY_NONE			0xFF

#pragma pack(1)

typedef __packed struct
{
	unsigned char pos_valid;					//位置有效,回收箱初始状态为不可用，装入回收盒后方为可用
	unsigned char pos_large;					//位置是否是扩展的，存放大件
	unsigned char pos_filled;					//位置里有物品
	unsigned char pos_box_is_take_away;			//位置内的盘子是否被取走 0:未被取走，1:被取走
	unsigned char pos_abnomal_recycle;			//是否异常回收
	unsigned char reserve[1];					//保持双字节对齐
}t_pos_info;

typedef __packed struct
{
	unsigned char x_last_take_away;				//上一次取走x位置，TAKE_AWAY_NONE(0xFF)表示没有被取走
	unsigned char y_last_take_away;				//上一次取走y位置，TAKE_AWAY_NONE(0xFF)表示没有被取走
	unsigned char pos_cnt_vaild;				//可用位置数量
	unsigned char pos_cnt_invaild;				//无效位置数量
	unsigned char pos_cnt_all;					//总位置数量，包含不可用的，当两个位置组合成一个大件位置时即产生一个无效

	unsigned char box_cnt_filled;				//有盘子的位置数量
	unsigned char package;						//包裹数量
	unsigned char box_cnt_filled_with_data;		//二维码数据有效的位置数量
	signed char x_pos_adjust;					//x位置校正，单位0.1mm，范围+-6.4mm
	signed char y_pos_adjust;					//y位置校正，单位0.1mm，范围+-6.4mm
}t_machine_info;

typedef __packed struct
{
	unsigned char x_pos_cur;		//当前已到x位置
	unsigned char y_pos_cur;		//当前已到y位置
	unsigned char x_pos_to_stop;	//需要定位到的x位置
	unsigned char y_pos_to_stop;	//需要定位到的y位置
	unsigned int x_run_steps;		//X已跑理论步数
	unsigned int y_run_steps;		//Y已跑理论步数
	unsigned char box_reinstall[BOX_NUM_MAX];	//大盒子被重新装入，用于清机时仅清理抽出重新装入的盒子
}t_actions_data;

typedef __packed struct
{
	t_pos_info pos_info[X_POS_MAX][Y_POS_MAX];	//每个位置的信息
	t_machine_info machine_info;				//整机信息
	unsigned short data_check;					//检查数据是否可用的'G''W'表示可用的0x4757，如不是此值表示为保存过需要初始化
	uint16_t crc;								//校验和，验证数据是否正确
}t_settings_saved;

typedef __packed struct
{
	unsigned char last_barcode[BARCODE_LEN];	//上次二维码表
	unsigned char current_barcode[BARCODE_LEN];	//当次二维码表
}t_barcode_info;

typedef __packed struct
{
	t_barcode_info barcode_info[X_POS_MAX][Y_POS_MAX];
}t_barcodes_saved;

typedef __packed struct
{
	t_actions_data actions_data;				//动作相关结构体
}t_settings_tmp;

typedef __packed struct
{
	t_settings_saved settings_saved;			//掉电保存的数据
	t_barcodes_saved barcodes_saved;			//掉电保存的二维码数据
	t_settings_tmp settings_tmp;				//掉电丢失的临时数据
}t_settings;

#pragma pack()

extern t_settings settings;
#define ACTIONS_DATA		settings.settings_tmp.actions_data
#define SETTING_SAVED		settings.settings_saved

//functions declare
void Settings_Init(void);
uint8_t Settings_Save(void);
uint8_t Settings_Get(void);
uint8_t Settings_BarcodeSaveAll(void);
uint8_t Settings_BarcodeSaveXY(unsigned char x, unsigned char y);
uint8_t Settings_BarcodeGet(void);
uint8_t Settings_ClearReset(void);

#endif

