#include "main.h"
#include "com.h"
#include "led.h"
#include "sensor.h"
#include "motor.h"
#include "digtube.h"
#include "switch.h"
#include "dcmotor.h"
#include "key.h"
#if IAP_Download
#include "setting.h"
#endif
#include "console.h"
#include "qrcode_scan.h"
#include "actions.h"
#include "parser.h"
#include "settings.h"
#include "s_def.h"
#include "i2c_EE.h"

unsigned char quit = 0;

#define POLINOMIAL								0x1021 //”√”⁄º∆À„crc

__IO uint32_t TimingDelay;

void NVIC_Config(void)
{
#ifndef STM32F10X_HD
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
#endif
}

static void SysTickConfig(void)
{
	
	RCC_ClocksTypeDef RCC_Clocks;

	/* SysTick end of count event each 10ms */
	RCC_GetClocksFreq(&RCC_Clocks);
	//SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
	SysTick_Config(RCC_Clocks.HCLK_Frequency /1000000); //1us --john

	/* Configure the SysTick handler priority */
	NVIC_SetPriority(SysTick_IRQn, 0x0);
}

static void Target_Init(void)
{
	Digtube_Init();
	Led_Init();
	Key_Init();
	Motor_Init();
	DCMotor_Init();
	Sensor_Init();
	Com_Init_All();
#if USING_EEPROM_TO_SAVE
	AT24CXX_Init();
#endif

	Sensor_AllEnable(ENABLE);
	QRCode_Init();
	Settings_Init();
	console_init();
	Actions_Init();
}

int main(void)
{
	RCC_ClocksTypeDef RCC_Clocks;

	/*!< At this stage the microcontroller clock setting is already configured, 
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f2xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f2xx.c file
	 */  

	/* NVIC configuration */
	NVIC_Config();

	/* SysTick end of count event each 10ms */
	RCC_GetClocksFreq(&RCC_Clocks);
	//SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);
	SysTickConfig();
	/* Add your application code here
	 */

	Target_Init();
	while(1)
	{
		quit = 0;
		while(!quit)
		{
			CMD_Parser();
		}

		quit = 0;
		console_welcome();
		while(!quit)
		{
			console();
		}
	}
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 1 ms.
  * @retval None
  */

void delay_us(__IO uint32_t us)
{

	TimingDelay = us;
	while(TimingDelay != 0);
}

void delay_ms(__IO uint32_t ms)
{

	TimingDelay = ms*1000;
	while(TimingDelay != 0);
}

void delay_s(__IO uint32_t n_s)
{

	TimingDelay = n_s*1000*1000;
	while(TimingDelay != 0);
}
/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
void TimeOut_UserCallback(void)
{
  /* User can add his own implementation to manage TimeOut Communication failure */
  /* Block communication and all processes */
  while (1)
  {   
  }
}

// CRC calc
short Calc_CRC(unsigned short crc, unsigned short ch)
{
	unsigned short i;
	
	ch <<= 8;
	for(i = 8;i > 0;i--)
	{
		if(((ch^crc) & 0x8000))
		{
			crc = (crc<<1)^POLINOMIAL;
		}
		else
		{
			crc <<= 1;
		}
		ch <<= 1;
	}
	return crc;
}

/* CRCC Cyclic redundancy code check(2 bytes) */
unsigned short GetCRC(unsigned char * data, int len)
{
	unsigned char ch;
	unsigned short i;
	unsigned short crc = 0x00;
	
	for(i = 0; i < len; i++)
	{
		ch = (*data++);
		crc = Calc_CRC(crc, (unsigned short)ch);
	}
	return crc;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

