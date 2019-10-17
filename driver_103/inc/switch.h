/*
  Filename: switch.h
  Author: xj			Date: 2015-05-11
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: ���뿪��������
*/


#ifndef __SWITCH_H__
#define __SWITCH_H__

#define MSTATUS_CB_DOWNLOAD    		0x01
#define MSTATUS_DIAGNOSE			0x02
#define MSTATUS_MB_FACTORY_TEST		0x04

#if OPTION_CONFIG_MOD
#define STANDER_CONFIG				0x00	//ֻ�з���ģ��
#define PRINT_SELECTED				0x01	//����ģ��+ƽӡ��
#define EMBOSS_SELECTED			0x02	//����ģ��+͹�ֻ�
#define ALL_SELECTED				0x03	//ȫ�� ����ģ��+ƽӡ��+͹�ֻ�
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

