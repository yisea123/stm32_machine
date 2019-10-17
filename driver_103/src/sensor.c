/*
  Filename: sensor.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 传感器驱动程序
 */
#include "sensor.h"
#include "parallel_to_serial.h"
#include "main.h"
#include "s_def.h"

#define LEVEL_REVERSAL_MASK		((1<<S_X_RESET)|(1<<S_X_LOCATE)|(1<<S_Y_LOCATE)|(1<<S_Y_RESET)|(1<<S_Z_FRONT)|(1<<S_Z_REAR)|(1<<S_HOOK_UP) \
								|(1<<S_SCANNER_UP)|(1<<S_PACK1)|(1<<S_PACK2)|(1<<S_PACK3)|(1<<S_PACK4)|(1<<S_PACK5)|(1<<S_PACK6)\
								|(1<<S_PACK7)|(1<<S_PACK8)|(1<<S_BOX1)|(1<<S_BOX2)|(1<<S_BOX3))
#define SENSOR_FILTER_TIMES		3

#define LINK_NUM	3

//cpu直连sensor pin配置
typedef enum
{
	SEN_PIN_X_LOCATE = 0,
	SEN_PIN_X_RESET,
	SEN_PIN_Y_LOCATE,
	SEN_PIN_Y_RESET,
	SEN_PIN_Z_FRONT,

	SEN_PIN_Z_REAR,		//5
	SEN_PIN_X0_TEST,
	SEN_PIN_X1_TEST,
	SEN_PIN_X2_TEST,
	SEN_PIN_HOOK_RST,

	SEN_PIN_SCAN_RST,	//10
	SEN_PIN_DETECT_BOX,
	SEN_PIN_PACK1,
	SEN_PIN_PACK2,
	SEN_PIN_PACK3,

	SEN_PIN_PACK4,		//15
	SEN_PIN_PACK5,
	SEN_PIN_PACK6,
	SEN_PIN_PACK7,
	SEN_PIN_PACK8,

	SEN_PIN_BOX1,		//20
	SEN_PIN_BOX2,
	SEN_PIN_BOX3,

	SEN_PIN_MAX,
}t_sen_pin_no;

typedef enum
{
	P_TO_S_PIN_INH,
	P_TO_S_PIN_SHLD,
	P_TO_S_PIN_SCK,
	P_TO_S_PIN_MISO,
	P_TO_S_PIN_MAX
}t_p_to_s_pin;

/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
static t_pin_source pin_sen_en[SEN_EN_MAX] =
{
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_3,  GPIO_Mode_Out_PP,     GPIO_Speed_50MHz, NULL},
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_4,  GPIO_Mode_Out_PP,     GPIO_Speed_50MHz, NULL},
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_5,  GPIO_Mode_Out_PP,     GPIO_Speed_50MHz, NULL},
	{GPIOG, RCC_APB2Periph_GPIOG, GPIO_Pin_12, GPIO_Mode_Out_PP,     GPIO_Speed_50MHz, NULL},
	{GPIOE, RCC_APB2Periph_GPIOE, GPIO_Pin_1, GPIO_Mode_Out_PP,     GPIO_Speed_50MHz, NULL},

	{GPIOE, RCC_APB2Periph_GPIOE, GPIO_Pin_15,  GPIO_Mode_Out_PP,     GPIO_Speed_50MHz, NULL},
};

#define EXTI_NULL {0, 0, 0, (EXTITrigger_TypeDef)0, 0, 0, 0, 0, 0}

static t_exti_source exti_sen[SEN_PIN_MAX] =
{
	{GPIO_PortSourceGPIOF, GPIO_PinSource9, EXTI_Line9, EXTI_TRIGGER_MODE_X_LOCATE, NVIC_PriorityGroup_0, EXTI9_5_IRQn,  0x00, 0X00, NULL},		//0
	{GPIO_PortSourceGPIOE, GPIO_PinSource3, EXTI_Line3, EXTI_TRIGGER_MODE_X_RESET, NVIC_PriorityGroup_0, EXTI3_IRQn,	 0x00, 0X00, NULL},
	{GPIO_PortSourceGPIOE, GPIO_PinSource4, EXTI_Line4, EXTI_TRIGGER_MODE_Y_LOCATE, NVIC_PriorityGroup_0, EXTI4_IRQn,  0x00, 0X00, NULL},
	{GPIO_PortSourceGPIOE, GPIO_PinSource2, EXTI_Line2, EXTI_TRIGGER_MODE_Y_RESET, NVIC_PriorityGroup_0, EXTI2_IRQn,	0x00, 0X00, NULL},
	{GPIO_PortSourceGPIOE, GPIO_PinSource6,	EXTI_Line6,	EXTI_TRIGGER_MODE_Z_FRONT, NVIC_PriorityGroup_0, EXTI9_5_IRQn,  0x00, 0X00, NULL},

	{GPIO_PortSourceGPIOE, GPIO_PinSource5,	EXTI_Line5,EXTI_TRIGGER_MODE_Z_REAR, NVIC_PriorityGroup_0, EXTI9_5_IRQn,  0x00, 0X00, NULL},	//5
	EXTI_NULL,
	EXTI_NULL,
	EXTI_NULL,
	{GPIO_PortSourceGPIOC, GPIO_PinSource13,	EXTI_Line13,	EXTI_Trigger_Rising, NVIC_PriorityGroup_0, EXTI15_10_IRQn,  0x00, 0X00, NULL},

	{GPIO_PortSourceGPIOF, GPIO_PinSource1,		EXTI_Line1,		EXTI_Trigger_Rising, NVIC_PriorityGroup_0, EXTI1_IRQn,  0x00, 0X00, NULL},		//10
	EXTI_NULL,
	EXTI_NULL,
	EXTI_NULL,
	EXTI_NULL,

	EXTI_NULL,																																	//15
	EXTI_NULL,
	EXTI_NULL,
	EXTI_NULL,
	EXTI_NULL,

	EXTI_NULL,																																	//20
	EXTI_NULL,
	EXTI_NULL,
};

/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
t_pin_source pin_sen[SEN_PIN_MAX] =
{
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_9,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &exti_sen[SEN_PIN_X_LOCATE]},  //0
	{GPIOE, RCC_APB2Periph_GPIOE, GPIO_Pin_3, 	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &exti_sen[SEN_PIN_X_RESET]}, 
	{GPIOE, RCC_APB2Periph_GPIOE, GPIO_Pin_4, 	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &exti_sen[SEN_PIN_Y_LOCATE]},  
	{GPIOE, RCC_APB2Periph_GPIOE, GPIO_Pin_2, 	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &exti_sen[SEN_PIN_Y_RESET]},  
	{GPIOE, RCC_APB2Periph_GPIOE, GPIO_Pin_6, 	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &exti_sen[SEN_PIN_Z_FRONT]},

	{GPIOE, RCC_APB2Periph_GPIOE, GPIO_Pin_5, 	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &exti_sen[SEN_PIN_Z_REAR]},	//5
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_6,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_7,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_8,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOC, RCC_APB2Periph_GPIOC, GPIO_Pin_13,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &exti_sen[SEN_PIN_HOOK_RST]},

	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_1,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, &exti_sen[SEN_PIN_SCAN_RST]},		//10
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_10,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOC, RCC_APB2Periph_GPIOC, GPIO_Pin_14,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOC, RCC_APB2Periph_GPIOC, GPIO_Pin_15,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_0,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},

	{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_2,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},		//15
	{GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_12,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_13,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_14,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_15,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},

	{GPIOG, RCC_APB2Periph_GPIOG, GPIO_Pin_15,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},		//20
	{GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_8,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},
	{GPIOB, RCC_APB2Periph_GPIOB, GPIO_Pin_9,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL},

};

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
/**
  * @brief  获得CPU直连sensor pin
  * @param  sen_no: 传感器编号
  * @retval pin枚举
  */
static t_sen_pin_no Sensor_GetPinNo(t_sen_no sen_no)
{
	t_sen_pin_no sen_pin_no;
	switch(sen_no)
	{
		case S_X_LOCATE:					//0
			sen_pin_no = SEN_PIN_X_LOCATE;
			break;
		case S_X_RESET:
			sen_pin_no = SEN_PIN_X_RESET;
			break;
		case S_Y_LOCATE:
			sen_pin_no = SEN_PIN_Y_LOCATE;
			break;
		case S_Y_RESET:
			sen_pin_no = SEN_PIN_Y_RESET;
			break;
		case S_Z_FRONT:
			sen_pin_no = SEN_PIN_Z_FRONT;
			break;

		case S_Z_REAR:							//5
			sen_pin_no = SEN_PIN_Z_REAR;
			break;
		case S_X0_TEST:
			sen_pin_no = SEN_PIN_X0_TEST;
			break;
		case S_X1_TEST:
			sen_pin_no = SEN_PIN_X1_TEST;
			break;
		case S_X2_TEST:
			sen_pin_no = SEN_PIN_X2_TEST;
			break;
		case S_HOOK_UP:
			sen_pin_no = SEN_PIN_HOOK_RST;
			break;

		case S_SCANNER_UP:						//10
			sen_pin_no = SEN_PIN_SCAN_RST;
			break;
		case S_DETECT_BOX:
			sen_pin_no = SEN_PIN_DETECT_BOX;
			break;
		case S_PACK1:
			sen_pin_no = SEN_PIN_PACK1;
			break;
		case S_PACK2:
			sen_pin_no = SEN_PIN_PACK2;
			break;
		case S_PACK3:
			sen_pin_no = SEN_PIN_PACK3;
			break;

		case S_PACK4:						//15
			sen_pin_no = SEN_PIN_PACK4;
			break;
		case S_PACK5:
			sen_pin_no = SEN_PIN_PACK5;
			break;
		case S_PACK6:
			sen_pin_no = SEN_PIN_PACK6;
			break;
		case S_PACK7:
			sen_pin_no = SEN_PIN_PACK7;
			break;
		case S_PACK8:
			sen_pin_no = SEN_PIN_PACK8;
			break;

		case S_BOX1:						//20
			sen_pin_no = SEN_PIN_BOX1;
			break;
		case S_BOX2:
			sen_pin_no = SEN_PIN_BOX2;
			break;
		case S_BOX3:
			sen_pin_no = SEN_PIN_BOX3;
			break;

		default:
			sen_pin_no = SEN_PIN_MAX;
			break;
	}

	return sen_pin_no;
}

/**
  * @brief  将sensor no翻译成MASK数据
  * @param  sen_no: 传感器编号
  * @retval 传感器对应MASK数据
  */
static uint32_t Sensor_SensorNoToSenSt(t_sen_no sen_no)
{
	uint32_t sen_st = 0;
	if(sen_no < S_MAX)
		sen_st = (1 << sen_no);
	return sen_st;
}

/**
  * @brief  将传感器得到的最后结果进行与处理，使之与需要的逻辑关系一直
  * @param  data: 引用值，传感器数据，在函数内进行转换
  * @retval 
  */
static void Sensor_DataPreHandler(uint32_t * p_data)//串联传感器取反逻辑
{
	uint32_t data1;
	uint32_t reversal_mask;
	
	reversal_mask = LEVEL_REVERSAL_MASK;		//取反屏蔽字
	data1 = *p_data;				//原始数据
	data1 = data1 & reversal_mask;	//获得需要取反的数据
	data1 = (~data1);				//取反
	data1 &= reversal_mask;			//数据规整
	*p_data &= (~reversal_mask);	//清空结果数据位
	*p_data |= data1;				//最终结果
}

/**
  * @brief  获得CPU直连传感器状态
  * @param  sen_no: 传感器编号
  * @retval 传感器状态
  */
static uint8_t Sensor_GetPinVal(t_sen_pin_no sen_pin_no)
{
	t_pin_source * p_pin;
	uint8_t ret_val = 0;
	
	if(sen_pin_no < SEN_PIN_MAX)
	{
		p_pin = &pin_sen[sen_pin_no];
		ret_val = Pin_ReadBit(p_pin);
	}

	return ret_val;
}

/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
 /**
  * @brief  传感器初始化
  * @param  
  * @retval 
  */
void Sensor_Init(void)
{
	uint8_t i;
	t_pin_source * p_pin;

	for(i = 0; i < SEN_EN_MAX; i++)
	{
		p_pin = &pin_sen_en[i];
		Pin_Init(p_pin);
	}
	for(i = 0; i < SEN_PIN_MAX; i++)
	{
		p_pin = &pin_sen[i];
		Pin_Init(p_pin);
	}
}
 /**
  * @brief  任一对射传感器使能
  * @param  sen_no: 传感器编号
  			enable:使能或者禁止
  * @retval 
  */
void Sensor_Enable(t_sen_no sen_no, FunctionalState enable)
{
	t_pin_source * p_pin;
	BitAction bit_set;

	if(!enable)
	{
		bit_set = Bit_RESET;
	}
	else
	{
		bit_set = Bit_SET;
	}
	p_pin = NULL;
	switch(sen_no)
	{
		case S_X0_TEST:
			p_pin = &pin_sen_en[SEN_EN_X0];
			break;
		case S_X1_TEST:
			p_pin = &pin_sen_en[SEN_EN_X1];
			break;
		case S_X2_TEST:
			p_pin = &pin_sen_en[SEN_EN_X2];
			break;
		case S_DETECT_BOX:
			p_pin = &pin_sen_en[SEN_EN_DETECT_BOX];
			break;
		case S_PACK1:
		case S_PACK2:
		case S_PACK3:
		case S_PACK4:
			p_pin = &pin_sen_en[SEN_EN_PACK_1_4];
			break;
		case S_PACK5:
		case S_PACK6:
		case S_PACK7:
		case S_PACK8:
			p_pin = &pin_sen_en[SEN_EN_PACK_5_8];
			break;
		default:
			if(enable)
			{
				bit_set = Bit_RESET;
			}
			else
			{
				bit_set =Bit_SET ;
			}
			//p_pin = &pin_sen_en[SEN_EN_1];
			break;
	}
	if(p_pin != NULL)
		Pin_WriteBit(p_pin, bit_set);
}

/**
  * @brief  传感器中断使能
  * @param  
  * @retval 
  */
void Sensor_IntEnable(t_sen_no sen_no, fun_exti_handler handler,
					  EXTITrigger_TypeDef trigger, FunctionalState enable)
{
	t_pin_source * p_pin;
	t_sen_pin_no sen_pin_no;
	
	sen_pin_no = Sensor_GetPinNo(sen_no);
	if(sen_pin_no < SEN_PIN_MAX)
	{
		p_pin = &pin_sen[sen_pin_no];
		Pin_ExtiEnable(p_pin, handler, trigger, enable);
	}
}

/**
  * @brief  传感器中断处理
  * @param  
  * @retval 
  */
void Sensor_IntHandler(t_sen_no sen_no)
{
	t_pin_source * p_pin;
	t_sen_pin_no sen_pin_no;
	
	sen_pin_no = Sensor_GetPinNo(sen_no);
	p_pin = &pin_sen[sen_pin_no];
	if(Sensor_IntFilter(sen_no))
		Pin_ExtiHandler(p_pin);
}

/* 传感器中断滤波函数 */
unsigned char Sensor_IntFilter(t_sen_no sen_no)
{
	t_pin_source * p_pin;
	t_sen_pin_no sen_pin_no;
	uint8_t pin_st, pin_will_st;
	unsigned int i = 0;
	
	sen_pin_no = Sensor_GetPinNo(sen_no);
	p_pin = &pin_sen[sen_pin_no];
	if(p_pin->GPIO_Exti->EXTI_Trigger == EXTI_Trigger_Rising)
	{
		pin_will_st = 1;
	}
	else if(p_pin->GPIO_Exti->EXTI_Trigger == EXTI_Trigger_Falling)
	{
		pin_will_st = 0;
	}
	else
	{
		pin_will_st = Pin_ReadBit(p_pin);
	}
	pin_st = pin_will_st;
	for(i = 0; i < SENSOR_FILTER_TIMES; i++)
	{
		pin_st = Pin_ReadBit(p_pin);
		if(pin_st != pin_will_st)
			break;
	}
	if(pin_st != pin_will_st)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/**
  * @brief  任一传感器状态获取
  * @param  
  * @retval 
  */
uint8_t Sensor_Status(t_sen_no sen_no)
{
	t_sen_pin_no sen_pin_no;
	uint8_t ret_val = 0;
	uint32_t sen_st = 0;


	sen_pin_no = Sensor_GetPinNo(sen_no);
	if(sen_pin_no < SEN_PIN_MAX)
	{
		//if((sen_no == S_X0_TEST) || (sen_no == S_X1_TEST) || (sen_no == S_X2_TEST))
		if((sen_no == S_X0_TEST) || (sen_no == S_X1_TEST) || (sen_no == S_X2_TEST)||(sen_no == S_DETECT_BOX) 
			  ||(sen_no == S_PACK1) || (sen_no == S_PACK2)||(sen_no == S_PACK3) || (sen_no == S_PACK4)
			  ||(sen_no == S_PACK5) || (sen_no == S_PACK6)||(sen_no == S_PACK7) || (sen_no == S_PACK8))
			
		{
			Sensor_Enable(sen_no, ENABLE);
			delay_ms(2);
		}
		if(Sensor_GetPinVal(sen_pin_no))
		{
			sen_st = (1 << sen_no);
		}
		//if((sen_no == S_X0_TEST) || (sen_no == S_X1_TEST) || (sen_no == S_X2_TEST))
		if((sen_no == S_X0_TEST) || (sen_no == S_X1_TEST) || (sen_no == S_X2_TEST)||(sen_no == S_DETECT_BOX) 
			  ||(sen_no == S_PACK1) || (sen_no == S_PACK2)||(sen_no == S_PACK3) || (sen_no == S_PACK4)
			  ||(sen_no == S_PACK5) || (sen_no == S_PACK6)||(sen_no == S_PACK7) || (sen_no == S_PACK8))
		{
			Sensor_Enable(sen_no, DISABLE);
		}
	}
	
	Sensor_DataPreHandler(&sen_st);
	sen_st &= Sensor_SensorNoToSenSt(sen_no);	//过滤其余
	if(sen_st)
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
   * @brief  经过滤波后的传感器状态获取
   * @param  
   * @retval 
   */
uint8_t Sensor_StatusAfterFilter(t_sen_no sen_no)
{
	uint8_t sen_st = 0;
	unsigned int i = 0;

	sen_st = Sensor_Status(sen_no);
	for(i = 0; i < SENSOR_FILTER_TIMES; i++)
	{
		if(sen_st != Sensor_Status(sen_no))
		{
			return 0;
		}
	}

	return sen_st;
}

 /**
  * @brief  所有对射传感器使能
  * @param  sen_no: 传感器编号
  			enable:使能或者禁止
  * @retval 
  */
void Sensor_AllEnable(FunctionalState enable)
{
	uint8_t i;

	for(i = 0; i < S_MAX; i++)
	{
		if((i != S_X0_TEST) && (i != S_X1_TEST) && (i != S_X2_TEST))		//此3传感器必须即开即用
		{
			Sensor_Enable((t_sen_no)i, enable);
		}
	}
}

/**
  * @brief  所有传感器状态获取
  * @param  
  * @retval 返回所有传感器索引值对应位的或 
  *         (1<<S_SENSOR_1)|(1<<S_SENSOR_2)|(1<<S_SENSOR_3)...
  */
uint32_t Sensor_AllStatus(void)
{
	uint32_t ret_val = 0;
	uint8_t  i;
	t_sen_pin_no sen_pin_no;

	//CPU直连sensor
	for(i = 0; i < S_MAX; i++)
	{
		if((i == S_X0_TEST) || (i == S_X1_TEST) || (i == S_X2_TEST)||(i == S_DETECT_BOX) 
		   ||(i == S_PACK1) || (i == S_PACK2)||(i == S_PACK3) || (i == S_PACK4)
		   ||(i == S_PACK5) || (i == S_PACK6)||(i == S_PACK7) || (i == S_PACK8))

		{
			Sensor_Enable((t_sen_no)i, ENABLE);
			delay_ms(2);
		}
		sen_pin_no = Sensor_GetPinNo((t_sen_no)i);
		if(sen_pin_no < SEN_PIN_MAX)
		{
			if(Sensor_GetPinVal(sen_pin_no))
			{
				ret_val |= (1 << i);
			}
		}
		if((i == S_X0_TEST) || (i == S_X1_TEST) || (i == S_X2_TEST)||(i == S_DETECT_BOX) 
		   ||(i == S_PACK1) || (i == S_PACK2)||(i == S_PACK3) || (i == S_PACK4)
		   ||(i == S_PACK5) || (i == S_PACK6)||(i == S_PACK7) || (i == S_PACK8))
		{
			Sensor_Enable((t_sen_no)i, DISABLE);
		}
	}
	//并转串sensor
	Sensor_DataPreHandler(&ret_val);
	return ret_val;
}

