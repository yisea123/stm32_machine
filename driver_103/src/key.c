/*
  Filename: key.c
  Copyright: �е糤��(��ɳ)��Ϣ�������޹�˾
  Description: ��������
*/
#include "pin.h"
#include "key.h"

/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
static t_pin_source key_port[KEY_NUM] =
{
	{GPIOG, RCC_APB2Periph_GPIOG,	GPIO_Pin_13, 	GPIO_Mode_IN_FLOATING,		GPIO_Speed_50MHz,	NULL},	//KEY3
	{GPIOB, RCC_APB2Periph_GPIOB,	GPIO_Pin_6, 	GPIO_Mode_IN_FLOATING,		GPIO_Speed_50MHz,	NULL},	//KEY2
	{GPIOB,	RCC_APB2Periph_GPIOB,	GPIO_Pin_7,		GPIO_Mode_IN_FLOATING,		GPIO_Speed_50MHz,	NULL},	//KEY1
};

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
/*����ȷ������*/
#define KEY_CONFIRM_CNT			2

static uint8_t key_cnt[KEY_NUM];

/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  ������ʼ��
  * @param  
  * @retval 
  */
void Key_Init(void)
{
	uint8_t i;
	t_pin_source * p_pin;

	for(i = 0; i < KEY_NUM; i++)
	{
		p_pin = &key_port[i];
		Pin_Init(p_pin);
	}
	for(i = 0; i < KEY_NUM; i++)
	{
		key_cnt[i] = 0;
	}
}

/**
  * @brief  ����״̬��ȡ
  * @param  
  * @retval ����״̬
  */
uint8_t Key_GetSt(void)
{
	uint8_t ret_val = 0;

	if(key_cnt[KEY1] >= KEY_CONFIRM_CNT)
	{
		ret_val |= ST_KEY1;
	}
	if(key_cnt[KEY2] >= KEY_CONFIRM_CNT)
	{
		ret_val |= ST_KEY2;
	}
	if(key_cnt[KEY3] >= KEY_CONFIRM_CNT)
	{
		ret_val |= ST_KEY3;
	}
	
	return ret_val;
}

/**
  * @brief  �ȴ������ͷ�
  * @param  
  * @retval 
  */
void Key_WaitRelease(void)
{
	while(key_cnt[KEY1] || key_cnt[KEY2] || key_cnt[KEY3]);
}

/**
  * @brief  �����жϺ���
  * @param  
  * @retval 
  */
void Key_Timer_Handler(void)
{
	if(!Pin_ReadBit(&key_port[KEY1]))
	{
		if(key_cnt[KEY1] < KEY_CONFIRM_CNT)
		{
			key_cnt[KEY1] += 1;
		}
	}
	else
	{
		if(key_cnt[KEY1] > 0)
		{
			key_cnt[KEY1] -= 1;
		}
	}
	
	if(!Pin_ReadBit(&key_port[KEY2]))
	{
		if(key_cnt[KEY2] < KEY_CONFIRM_CNT)
		{
			key_cnt[KEY2] += 1;
		}
	}
	else
	{
		if(key_cnt[KEY2] > 0)
		{
			key_cnt[KEY2] -= 1;
		}
	}
	
	if(!Pin_ReadBit(&key_port[KEY3]))
	{
		if(key_cnt[KEY3] < KEY_CONFIRM_CNT)
		{
			key_cnt[KEY3] += 1;
		}
	}
	else
	{
		if(key_cnt[KEY3] > 0)
		{
			key_cnt[KEY3] -= 1;
		}
	}
}

