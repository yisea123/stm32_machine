/*
  Filename: key.h
  Author: xj			Date: 2015-05-10
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 按键
*/


#ifndef __KEY_H__
#define __KEY_H__

//传感器枚举值
typedef enum
{
	KEY1 = 0,
	KEY2,
	KEY3,
	KEY_NUM
}t_sw_no;

#define ST_KEY1		(1<<KEY1)
#define ST_KEY2		(1<<KEY2)
#define ST_KEY3		(1<<KEY3)

void Key_Init(void);
uint8_t Key_GetSt(void);
void Key_WaitRelease(void);
void Key_Timer_Handler(void);

#endif

