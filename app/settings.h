/*
  Filename: settings.h
  Author: shidawei			Date: 20130221
  Copyright: HuNan GreatWall Information Financial Equipment Co.,Ltd.
  Description: ��������
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
	unsigned char pos_valid;					//λ����Ч,�������ʼ״̬Ϊ�����ã�װ����պк�Ϊ����
	unsigned char pos_large;					//λ���Ƿ�����չ�ģ���Ŵ��
	unsigned char pos_filled;					//λ��������Ʒ
	unsigned char pos_box_is_take_away;			//λ���ڵ������Ƿ�ȡ�� 0:δ��ȡ�ߣ�1:��ȡ��
	unsigned char pos_abnomal_recycle;			//�Ƿ��쳣����
	unsigned char reserve[1];					//����˫�ֽڶ���
}t_pos_info;

typedef __packed struct
{
	unsigned char x_last_take_away;				//��һ��ȡ��xλ�ã�TAKE_AWAY_NONE(0xFF)��ʾû�б�ȡ��
	unsigned char y_last_take_away;				//��һ��ȡ��yλ�ã�TAKE_AWAY_NONE(0xFF)��ʾû�б�ȡ��
	unsigned char pos_cnt_vaild;				//����λ������
	unsigned char pos_cnt_invaild;				//��Чλ������
	unsigned char pos_cnt_all;					//��λ�����������������õģ�������λ����ϳ�һ�����λ��ʱ������һ����Ч

	unsigned char box_cnt_filled;				//�����ӵ�λ������
	unsigned char package;						//��������
	unsigned char box_cnt_filled_with_data;		//��ά��������Ч��λ������
	signed char x_pos_adjust;					//xλ��У������λ0.1mm����Χ+-6.4mm
	signed char y_pos_adjust;					//yλ��У������λ0.1mm����Χ+-6.4mm
}t_machine_info;

typedef __packed struct
{
	unsigned char x_pos_cur;		//��ǰ�ѵ�xλ��
	unsigned char y_pos_cur;		//��ǰ�ѵ�yλ��
	unsigned char x_pos_to_stop;	//��Ҫ��λ����xλ��
	unsigned char y_pos_to_stop;	//��Ҫ��λ����yλ��
	unsigned int x_run_steps;		//X�������۲���
	unsigned int y_run_steps;		//Y�������۲���
	unsigned char box_reinstall[BOX_NUM_MAX];	//����ӱ�����װ�룬�������ʱ������������װ��ĺ���
}t_actions_data;

typedef __packed struct
{
	t_pos_info pos_info[X_POS_MAX][Y_POS_MAX];	//ÿ��λ�õ���Ϣ
	t_machine_info machine_info;				//������Ϣ
	unsigned short data_check;					//��������Ƿ���õ�'G''W'��ʾ���õ�0x4757���粻�Ǵ�ֵ��ʾΪ�������Ҫ��ʼ��
	uint16_t crc;								//У��ͣ���֤�����Ƿ���ȷ
}t_settings_saved;

typedef __packed struct
{
	unsigned char last_barcode[BARCODE_LEN];	//�ϴζ�ά���
	unsigned char current_barcode[BARCODE_LEN];	//���ζ�ά���
}t_barcode_info;

typedef __packed struct
{
	t_barcode_info barcode_info[X_POS_MAX][Y_POS_MAX];
}t_barcodes_saved;

typedef __packed struct
{
	t_actions_data actions_data;				//������ؽṹ��
}t_settings_tmp;

typedef __packed struct
{
	t_settings_saved settings_saved;			//���籣�������
	t_barcodes_saved barcodes_saved;			//���籣��Ķ�ά������
	t_settings_tmp settings_tmp;				//���綪ʧ����ʱ����
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

