/*
  Filename: autobaud.h
  Author: shidawei			Date: 2014-06-24
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 波特率自适应
*/

#ifndef __AUTO_BAUD_H__
#define __AUTO_BAUD_H__



extern void Autobaud_Init(void);	
extern void Autobaud_Handler(void);
extern void Autobaud_Enable(uint8_t enable);
extern void AutoBaud_SetBaudrate(uint32_t baudrate,uint16_t wordlength,uint16_t stopbits,uint16_t parity);
extern void AutoBaud_SetComEn(FunctionalState en);
extern void AutoBaud_UnGetCh(uint8_t ch);
extern uint8_t Autobaud_GetDone(void);
extern void Autobaud_Set_Done(void);
extern void Autobaud_Set_Done_Init(void);


#endif

