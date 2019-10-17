/*
  Filename: maincom.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 串口通讯硬件协议仿真
*/
#include "lib_common.h"
#include "maincom.h"
#include "pin.h"
#include "com.h"

/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
static uint8_t mcom1_status = 0;
static uint8_t mcom2_status = 0;

enum
{
	PIN_DTR,
	PIN_CTS,
	PIN_MCOM1_MAX
};

enum
{
	PIN_RTS,
	PIN_DSR,
	PIN_MCOM2_MAX
};

t_pin_source mcom1_ctrl_pin[2] =
{
	{GPIOC, RCC_APB2Periph_GPIOC, GPIO_Pin_8, 	GPIO_Mode_Out_PP,      GPIO_Speed_50MHz, NULL},	//DTR OUT
	{GPIOA, RCC_APB2Periph_GPIOA, GPIO_Pin_11,  GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL}	//CTS IN
};

t_pin_source mcom2_ctrl_pin[2] =
{
	{GPIOA, RCC_APB2Periph_GPIOA, GPIO_Pin_1,  GPIO_Mode_Out_PP,      GPIO_Speed_50MHz, NULL},	//RTS OUT
	{GPIOA, RCC_APB2Periph_GPIOA, GPIO_Pin_0,  GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL}	//DSR IN
};

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
void MCom_Init_All(void)
{
	Pin_Init(&mcom1_ctrl_pin[0]);
	Pin_Init(&mcom1_ctrl_pin[1]);
	Pin_Init(&mcom2_ctrl_pin[0]);
	Pin_Init(&mcom2_ctrl_pin[1]);

	MCom1_Ready(1);
	MCom2_Receive_Allow(1);
}

void MCom1_Ready(uint8_t ready)
{
	t_pin_source * p_pin = &mcom1_ctrl_pin[0];
	if(ready)
	{
		Pin_WriteBit(p_pin, Bit_RESET);
		mcom1_status |= S_DTR_ON;
	}
	else
	{
		Pin_WriteBit(p_pin, Bit_SET);
		mcom1_status &= ~S_DTR_ON;
	}
}

uint8_t MCom1_Send_Allowed(void)
{
	t_pin_source * p_pin = &mcom1_ctrl_pin[1];
	uint8_t ret_val = 0;
	
	if(Pin_ReadBit(p_pin))
	{
		mcom1_status &= ~S_CTS_ON;
		ret_val = 1;
	}
	else
	{
		mcom1_status |= S_CTS_ON;
		ret_val = 0;
	}
	return ret_val;
}

uint8_t MCom2_Ready(void)
{
	t_pin_source * p_pin = &mcom2_ctrl_pin[1];
	uint8_t ret_val = 0;
	if(Pin_ReadBit(p_pin))
	{
		mcom2_status &= ~M_DSR_ON;
		ret_val = 0;
	}
	else
	{
		mcom2_status |= M_DSR_ON;
		ret_val = 1;
	}
	return ret_val;
}

void MCom2_Receive_Allow(uint8_t allow)
{
	t_pin_source * p_pin = &mcom2_ctrl_pin[0];
	if(allow)
	{
		Pin_WriteBit(p_pin, Bit_RESET);
		mcom2_status |= S_DTR_ON;
	}
	else
	{
		Pin_WriteBit(p_pin, Bit_SET);
		mcom2_status &= ~S_DTR_ON;
	}
}

uint8_t MCom1_Send(uint8_t ch)
{
	if(!(mcom1_status & S_DTR_ON))
		return 0;

	if(!MCom1_Send_Allowed())
		return 0;

	return Com_Send_Ch(MAIN_COM1, ch);
}

void MCom1_SendBuf(char *ptx_buff, u32  tx_size)
{
	u32 cnt = 0;
	char * prt;

	prt = ptx_buff;
	if(tx_size == 0)
	{
		return;
	}
	for(cnt = 0; cnt < tx_size; cnt++)
	{
		while(!MCom1_Send(*prt));
		prt++;
	}
}

uint8_t MCom1_Receive(uint8_t *pch)
{
	if(!(mcom1_status & S_DTR_ON))
		return 0;
	
	return Com_Receive_Ch(MAIN_COM1, pch);
}

uint8_t MCom2_Send(uint8_t ch)
{
#if 0 //del for test
	if(!MCom2_Ready())
		return 0;
#endif	
	return Com_Send_Ch(MAIN_COM2, ch);
}

uint8_t MCom2_Receive(uint8_t *pch)
{
	uint8_t ch, ret;
#if 0	//del for test
	if(!MCom2_Ready())
		return 0;
	
	if(!(mcom2_status & M_RTS_ON))
		return 0;
#endif 	
	//MCom2_Receive_Allow(1);
	ret = Com_Receive_Ch(MAIN_COM2, &ch);
	//MCom2_Receive_Allow(0);
	*pch = ch;
	return ret;
}

