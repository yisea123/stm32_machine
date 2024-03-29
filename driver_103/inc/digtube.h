/*
  Filename: digtube.c
  Author: xj			Date: 2015-05-09
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 数码管驱动
*/

/*****************************************************************************/
#define  MAX_CHAR_TABLE             36//字符表个数

//段位表
#define SEGMENT_A   0x01
#define SEGMENT_B   0x02
#define SEGMENT_C   0x04
#define SEGMENT_D   0x08
#define SEGMENT_E   0x10
#define SEGMENT_F   0x20
#define SEGMENT_G   0x40
#define SEGMENT_DP  0x80

//字母表
#define DIGIT_CHAR_A (SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_B (SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_C (SEGMENT_A | SEGMENT_D | SEGMENT_E | SEGMENT_F)
#define DIGIT_CHAR_D (SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_G)
#define DIGIT_CHAR_E (SEGMENT_A | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_F (SEGMENT_A | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_G (SEGMENT_A | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F)
#define DIGIT_CHAR_H (SEGMENT_B | SEGMENT_C | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_I (SEGMENT_E)
#define DIGIT_CHAR_J (SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E)
#define DIGIT_CHAR_K (SEGMENT_B | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_L (SEGMENT_D | SEGMENT_E | SEGMENT_F)
#define DIGIT_CHAR_M (SEGMENT_A | SEGMENT_C | SEGMENT_E | SEGMENT_G)
#define DIGIT_CHAR_N (SEGMENT_C | SEGMENT_E | SEGMENT_G)
#define DIGIT_CHAR_O (SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_G)
#define DIGIT_CHAR_P (SEGMENT_A | SEGMENT_B | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_Q (SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_R (SEGMENT_E | SEGMENT_G)
#define DIGIT_CHAR_S (SEGMENT_A | SEGMENT_C | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_T (SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_U (SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F)
#define DIGIT_CHAR_V (SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_W (SEGMENT_A | SEGMENT_C | SEGMENT_D | SEGMENT_E)
#define DIGIT_CHAR_X (SEGMENT_B | SEGMENT_D | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_Y (SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_Z (SEGMENT_A | SEGMENT_D | SEGMENT_G)

//数字表
#define DIGIT_CHAR_1 (SEGMENT_B | SEGMENT_C)
#define DIGIT_CHAR_2 (SEGMENT_A | SEGMENT_B | SEGMENT_D | SEGMENT_E | SEGMENT_G)
#define DIGIT_CHAR_3 (SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_G)
#define DIGIT_CHAR_4 (SEGMENT_B | SEGMENT_C | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_5 (SEGMENT_A | SEGMENT_C | SEGMENT_D | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_6 (SEGMENT_A | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_7 (SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_F)
#define DIGIT_CHAR_8 (SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_9 (SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_F | SEGMENT_G)
#define DIGIT_CHAR_0 (SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F)

// -
#define DIGIT_CHAR_MINUS (SEGMENT_G)

//全亮和全灭
#define DIGIT_ALL_ON  (SEGMENT_A | SEGMENT_B | SEGMENT_C | SEGMENT_D | SEGMENT_E | SEGMENT_F | SEGMENT_G | SEGMENT_DP)
#define DIGIT_ALL_OFF  0x00
/*****************************************************************************/

typedef enum{
	LED_A,
	LED_B,
	LED_C,
	LED_D,
	LED_E,
	LED_F,
	LED_G,
	SEGMENT_NUM
}t_segment_port;

typedef enum{
	LED_DIG1,
	LED_DIG2,
	LED_DIG3,	
	LED_DIG_NUM
}t_led_dig;

typedef enum
{
	SET_TIMER_EN1,
	SET_TIMER_EN2,
	SET_TIMER_EN3,	
	SET_TIMER_STATE
}t_set_timchoice;

typedef enum
{
	DIGTUBE_OFF,
	DIGTUBE_ON
}t_on_or_off;

typedef enum
{
	DIGTUBE_MODE_OFF,
	DIGTUBE_MODE_ON,
	DIGTUBE_MODE_BLINK
}t_digtube_mode;

extern void Digtube_All_OnOff(unsigned char on_or_off);
extern void Digtube_Show(unsigned char * code, t_digtube_mode mode);
extern void Digtube_Timer_Handler(void);
extern void Digtube_Init(void);

