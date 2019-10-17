/*
  Filename: uart.h
  Author: xj			Date: 2015-05-08
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 串口驱动程序
*/

#ifndef __COM_H__
#define __COM_H__
#include "lib_common.h"
#include "usart.h"

typedef enum{
	MAIN_COM1,	//USART1, to PC
	MAIN_COM2, 	//USART2, to Gate control
	QRCODE_COM,	//UART3,  to QRcode scanner
	RFID_COM,	//USART4, to RFID
	EMBS_COM,	//UART5,  to EMBOSSER
	COM_NUM,
}t_com_type;

extern void Com_Init(t_com_type com);
extern void Com_Init_All(void);
extern void MCom_Init_All(void);

extern uint8_t Com_Send_Ch(t_com_type com, uint8_t ch);
extern uint8_t Com_Receive_Ch(t_com_type com, uint8_t * pdata);
extern uint8_t Com_UnGet_Ch(t_com_type com, uint8_t data);
extern uint8_t Com_UnGet_Buf(t_com_type com,uint8_t * buf,uint8_t size);

extern void Com_Set_Baud(t_com_type com, uint32_t baud, uint16_t wordlength, uint16_t stopbits, uint16_t parity);
extern void Com_Send_Buffer(t_com_type com, char *ptx_buff, u32  tx_size);
extern void Comx_IRQHandler(t_com_type com);
extern void Com_Enable(t_com_type com, uint8_t enable);


//----------------------------------------------

#define Debug_Send_Ch(ch)	Com_Send_Ch(MAIN_COM1, ch)
#define Debug_Receive_Ch	Com_Receive_Ch(MAIN_COM1)
#define Debug_Send_Buffer(ptx_buff, tx_size)	Com_Send_Buffer(MAIN_COM1, ptx_buff, tx_size)
#define Debug_String(String) {uint8_t XString[20];\
          memcpy(XString, String, (sizeof(String)>20)?20:sizeof(String));\
          Com_Send_Buffer(MAIN_COM1, XString, (sizeof(String)>20)?20:sizeof(String));}

#define MAIN_COM1_IRQHANDLER   		USART1_IRQHandler
#define MAIN_COM2_IRQHANDLER		USART3_IRQHandler
#define RFID_COM_IRQHandler			USART2_IRQHandler



#endif

