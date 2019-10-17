#ifndef __S_DEF_H__
#define __S_DEF_H__
#include "stm32f10x.h"

#define PRODUCT_STR				"DPM530 - Cash Provide"
#define VER_MACHINE				200
#define VER_FW					300
#define DATE_STR				"20190710"		//year month day

#define DBG						0


#define MAIN_BOARD_VER_001		001			//��һ��Ƥ����������
#define MAIN_BOARD_VER_102		102			//�ڶ���˿�˻���
#define MAIN_BOARD_VER_103		103			//������˿�˻���

#define MAIN_BOARD_VER			MAIN_BOARD_VER_103


#define FALSE					0
#define TRUE					1
#define NULL					0
#define DATA_VALIDED			0x88

#define BARCODE_LEN				64
#define X_POS_MAX				3
#define Y_POS_MAX				14
#define Y_POS_ALL_SCAN_MAX		12		//���ʱY������
#define Y_POS_RECYCLE_BOX		Y_POS_ALL_SCAN_MAX

#define LAYER_IN_BOX			4		//һ������Ӳ���
#define BOX_NUM_MAX				3		//��༸�������

#define USING_EEPROM_TO_SAVE	1
#define SCAN_STANDBY_CASE_CAR	0		//����һ��ɨ��ͷ��С��������
#define SCAN_STANDBY_CASE_TOP	0		//���ö���ɨ��ͷ������

#define EEPROM_START_ADDR			0x0
#define EEPROM_BARCODES_ADDR		0x400

#define SCAN_MOVE_TIMEOUT_MS		4000
#define GET_SCAN_RESULT_TIMEOUT_MS	200
#define SCAN_TIMEOUT_MS				500

enum
{
	SOH	= 0x01,
	STX	= 0x02,
	ETX	= 0x03,
	EOT = 0x04,
	ENQ = 0x05,
	ACK = 0x06,
	DLE = 0x10,
	NAK = 0x15,
	SYN = 0x16,
};

/***************************************************
 * Golbal Data
 */

#endif
