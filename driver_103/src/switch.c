/*
  Filename: switch.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 拨码开关与跳线
*/
#include "main.h"
#include "pin.h"
#include "switch.h"
/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
t_pin_source jump_port[] =
{
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_7,		GPIO_Mode_IPU,		GPIO_Speed_50MHz,	NULL}	//老化测试
}; 

t_pin_source switch_test_port[] =
{
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_6,		GPIO_Mode_IPU,		GPIO_Speed_50MHz,	NULL}	//自测试
};

t_pin_source switch_mode_port[] =
{
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_2,		GPIO_Mode_IPU,		GPIO_Speed_50MHz,	NULL},
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_3,		GPIO_Mode_IPU,		GPIO_Speed_50MHz,	NULL}
};

/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  获取跳线状态
  * @param  
  * @retval 跳线状态
  */
uint8_t GetJumpSt(void)
{
	return (!Pin_ReadBit(&jump_port[0]));
}

/**
  * @brief  获取测试用拨码开关状态
  * @param  
  * @retval 拨码开关状态
  */
uint8_t GetSwitchSt(void)
{
	return (!Pin_ReadBit(&switch_test_port[0]));
}

/**
  * @brief  获取机型配置用拨码开关状态
  * @param  
  * @retval 拨码开关状态
  */
uint8_t Get_Option_Config(void)
{
	uint8_t ret_val = 0;
	
#if OPTION_CONFIG_MOD
	ret_val = STANDER_CONFIG;
	if((Pin_ReadBit(&switch_mode_port[1])) && (!Pin_ReadBit(&switch_mode_port[0])))
	{
		ret_val = PRINT_SELECTED;
	}
	else if((!Pin_ReadBit(&switch_mode_port[1])) && (Pin_ReadBit(&switch_mode_port[0])))
	{
		ret_val = EMBOSS_SELECTED;
	}
#else
	if((Pin_ReadBit(&switch_mode_port[1])) && (!Pin_ReadBit(&switch_mode_port[0])))
	{
		ret_val = DAUL_RECYCLEBOX;
	}
	else if((!Pin_ReadBit(&switch_mode_port[1])) && (Pin_ReadBit(&switch_mode_port[0])))
	{
		ret_val = (DAUL_RECYCLEBOX|TEMP_MECHANISM);
	}
	else if((!Pin_ReadBit(&switch_mode_port[1])) && (!Pin_ReadBit(&switch_mode_port[0])))
	{
		ret_val = AUTO_EJECT_MECHANISM;
	}
	else
	{
		ret_val = 0;
	}
#endif
	return ret_val;
}

uint8_t Get_Main_Status(void)
{
	uint8_t ret_val = 0;
	if(GetSwitchSt())
	{
		ret_val = 1;
	}
	else
	{
		ret_val = 0;
	}

	return ret_val;
}

/**
  * @brief  拨码开关和跳线初始化
  * @param  
  * @retval 
  */
void Switch_Init(void)
{
	uint8_t i = 0;
	t_pin_source * p_pin;

	Pin_Init(switch_test_port);
	Pin_Init(jump_port);
	for(i = 0; i < sizeof(switch_mode_port)/sizeof(t_pin_source); i++)
	{
		p_pin = &switch_mode_port[i];
		Pin_Init(p_pin);
	}
}

