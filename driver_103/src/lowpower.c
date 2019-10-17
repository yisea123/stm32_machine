#include "main.h"
#include "lowpower.h"
#include "sensor.h"
static uint8_t lp_enable;
static uint8_t lp_could_enable;

void LP_Enable(uint8_t enable)
{
	lp_could_enable = enable;
	if(!enable)		//退出节能模式时，将关闭传感器使能
	{
		SleepMode_Exit();
		Sensor_AllEnable(ENABLE);
		delay_ms(2);
	}
}

void LP_Init(void)
{
	lp_enable=DISABLE;
}

void SleepMode_In(void)
{
	if((!lp_enable) && (lp_could_enable))
	{
		lp_enable = ENABLE;
		Sensor_AllEnable(DISABLE);
	}
}

void SleepMode_Exit(void)
{
	if(lp_enable)
	{
		lp_enable = DISABLE;
		Sensor_AllEnable(ENABLE);
	}
}

uint8_t CB_LPCtrlDisable(void)
{
	LP_Enable(0);
	return 0;
}


