#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define TRUE							1
#define FALSE							0

/*-----------------------Command Key----------------------*/
extern unsigned char quit;


/* Exported functions ------------------------------------------------------- */
void TimingDelay_Decrement(void);
void delay_us(__IO uint32_t us);
void delay_ms(__IO uint32_t ms);
void delay_s(__IO uint32_t n_s);
unsigned short GetCRC(unsigned char * data, int len);
short Calc_CRC(unsigned short crc, unsigned short ch);

#if IAP_Download
typedef  void (*pFunction)(void);
#endif



#endif

