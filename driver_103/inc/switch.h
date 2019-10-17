/*
  Filename: switch.h
  Author: xj			Date: 2015-05-11
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 拨码开关与跳线
*/


#ifndef __SWITCH_H__
#define __SWITCH_H__

#define MSTATUS_CB_DOWNLOAD    		0x01
#define MSTATUS_DIAGNOSE			0x02
#define MSTATUS_MB_FACTORY_TEST		0x04

#if OPTION_CONFIG_MOD
#define STANDER_CONFIG				0x00	//只有发卡模块
#define PRINT_SELECTED				0x01	//发卡模块+平印机
#define EMBOSS_SELECTED			0x02	//发卡模块+凸字机
#define ALL_SELECTED				0x03	//全配 发卡模块+平印机+凸字机
#else
#define DAUL_RECYCLEBOX				0x01
#define TEMP_MECHANISM				0x02
#define AUTO_EJECT_MECHANISM		0X04	
#endif

uint8_t GetJumpSt(void);
uint8_t GetSwitchSt(void);
uint8_t Get_Option_Config(void);
uint8_t Get_Main_Status(void);
void Switch_Init(void);

#endif

