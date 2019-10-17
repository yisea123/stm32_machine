#ifndef __LED1_H__
#define __LED1_H__

typedef enum{
	LED_BREATHING,
	LED_NUM
}t_led_other;

void Led_Init(void);
void Led_BreathingTimerHandler(void);
void Led_BreathingChange(void);

#endif

