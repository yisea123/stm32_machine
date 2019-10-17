/*
  Filename: sensor.h
  Author: xj			Date: 20150513
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: ��������������ͷ�ļ�
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__
#include "pin.h"
#include "s_def.h"

//������ö��ֵ
typedef enum
{
	S_X_LOCATE = 0,		//x��λ
	S_X_RESET,			//x��λ
	S_Y_LOCATE,			//y��λ
	S_Y_RESET,			//y��λ
	S_Z_FRONT,			//z����ǰ��
	
	S_Z_REAR,			//z���Ӻ��
	S_X0_TEST,			//x0������
	S_X1_TEST,			//x1������
	S_X2_TEST,			//x2������
	S_HOOK_UP,			//������̧��λ

	S_SCANNER_UP,		//ɨ��ͷ������λ
	S_DETECT_BOX,		//��������Ƿ����
	S_PACK1,			//������1
	S_PACK2,			//������2
	S_PACK3,			//������3

	S_PACK4,			//������4
	S_PACK5,			//������5
	S_PACK6,			//������6
	S_PACK7,			//������7
	S_PACK8,			//������8

	S_BOX1,				//���Ӵ���1
	S_BOX2,				//���Ӵ���2
	S_BOX3,				//���Ӵ���3

	S_MAX
}t_sen_no;

#define EXTI_TRIGGER_MODE_X_LOCATE			EXTI_Trigger_Falling
#define EXTI_TRIGGER_MODE_X_RESET			EXTI_Trigger_Falling
#define EXTI_TRIGGER_MODE_Y_LOCATE			EXTI_Trigger_Falling
#define EXTI_TRIGGER_MODE_Y_RESET			EXTI_Trigger_Falling
#define EXTI_TRIGGER_MODE_Z_FRONT			EXTI_Trigger_Falling
#define EXTI_TRIGGER_MODE_Z_REAR			EXTI_Trigger_Falling
#define EXTI_TRIGGER_MODE_HOOK				EXTI_Trigger_Falling
#define EXTI_TRIGGER_MODE_SCANNER			EXTI_Trigger_Falling

typedef enum
{
	//���IO�����䴫����ʹ��
	SEN_EN_X0 = 0,		//x0�Ƿ�������ʹ��
	SEN_EN_X1,			//x1�Ƿ�������ʹ��
	SEN_EN_X2,			//x2�Ƿ�������ʹ��
	SEN_EN_DETECT_BOX,	//��������Ƿ����ʹ��
	SEN_EN_PACK_1_4,	//PACK1 ~ PACK4ʹ��

	SEN_EN_PACK_5_8,	//PACK5 ~ PACK8ʹ��
	SEN_EN_MAX
}t_sen_en_no;

//functions declare
void Sensor_Init(void);
uint8_t Sensor_Status(t_sen_no sen_no);
void Sensor_Enable(t_sen_no sen_no, FunctionalState enable);
uint8_t Sensor_StatusAfterFilter(t_sen_no sen_no);
void Sensor_IntEnable(t_sen_no sen_no, fun_exti_handler handler,
					  EXTITrigger_TypeDef trigger, FunctionalState enable);
void Sensor_IntHandler(t_sen_no sen_no);
unsigned char Sensor_IntFilter(t_sen_no sen_no);
uint32_t Sensor_AllStatus(void);
void Sensor_AllEnable(FunctionalState enable);

#endif

