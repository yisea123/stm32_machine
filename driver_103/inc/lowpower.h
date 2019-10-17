#ifndef __LOWPOWER_H__
#define __LOWPOWER_H__

#include "stm32f10x.h"

extern void LP_Enable(uint8_t enable);
extern void LP_Init(void);
extern void SleepMode_In(void);
extern void SleepMode_Exit(void);
extern uint8_t CB_LPCtrlDisable(void);

#endif

