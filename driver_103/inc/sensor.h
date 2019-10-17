/*
  Filename: sensor.h
  Author: xj			Date: 20150513
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 传感器驱动程序头文件
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__
#include "pin.h"
#include "s_def.h"

//传感器枚举值
typedef enum
{
	S_X_LOCATE = 0,		//x定位
	S_X_RESET,			//x复位
	S_Y_LOCATE,			//y定位
	S_Y_RESET,			//y复位
	S_Z_FRONT,			//z向钩子前端
	
	S_Z_REAR,			//z向钩子后端
	S_X0_TEST,			//x0伸出检测
	S_X1_TEST,			//x1伸出检测
	S_X2_TEST,			//x2伸出检测
	S_HOOK_UP,			//钩托盘抬起到位

	S_SCANNER_UP,		//扫描头上升到位
	S_DETECT_BOX,		//检测托盘是否存在
	S_PACK1,			//包存在1
	S_PACK2,			//包存在2
	S_PACK3,			//包存在3

	S_PACK4,			//包存在4
	S_PACK5,			//包存在5
	S_PACK6,			//包存在6
	S_PACK7,			//包存在7
	S_PACK8,			//包存在8

	S_BOX1,				//箱子存在1
	S_BOX2,				//箱子存在2
	S_BOX3,				//箱子存在3

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
	//输出IO，对射传感器使能
	SEN_EN_X0 = 0,		//x0是否伸出检测使能
	SEN_EN_X1,			//x1是否伸出检测使能
	SEN_EN_X2,			//x2是否伸出检测使能
	SEN_EN_DETECT_BOX,	//检测托盘是否存在使能
	SEN_EN_PACK_1_4,	//PACK1 ~ PACK4使能

	SEN_EN_PACK_5_8,	//PACK5 ~ PACK8使能
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

