/*
  Filename: maincom.h
  Author: xj			Date: 2015-05-21
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 串口通讯硬件协议仿真
*/


#ifndef __MAINCOM_H__
#define __MAINCOM_H__

typedef enum{
	MCOM_SLAVE,
	MCOM_MASTER	
}t_m_com_type;

typedef enum{
	S_DTR,
	S_CTS,
	S_SIG_NUM
}t_m_slave_signal;

typedef enum{
	M_DSR,
	M_RTS,
	M_SIG_NUM
}t_m_master_signal;

typedef enum{
	S_CTS_ON=0x01,
	S_DTR_ON=0x02,
	M_DSR_ON=0x10,
	M_RTS_ON=0x20
}t_m_status;

typedef struct{
	uint16_t		pin;
	GPIO_TypeDef *	gpio_port;
	uint32_t		gpio_clk;
	uint32_t		exti_line;
	uint8_t			exti_port_source;
	uint8_t			exti_pin_source;
	uint8_t			exti_irq;
	t_m_status		status;
}t_mcom_status_pin;
//-------------------------------------
extern void MCom_Init_All(void);

extern void MCom1_Ready(uint8_t ready);
extern uint8_t MCom1_Send_Allowed(void);
extern uint8_t MCom2_Ready(void);
extern void MCom2_Receive_Allow(uint8_t allow);

//-------------------------------------
extern uint8_t MCom1_Send(uint8_t ch);
extern uint8_t MCom1_Receive(uint8_t *pch);
void MCom1_SendBuf(char *ptx_buff, u32  tx_size);
extern uint8_t MCom2_Send(uint8_t ch);
extern uint8_t MCom2_Receive(uint8_t *pch);




#endif
