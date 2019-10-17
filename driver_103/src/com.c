/*
  Filename: uart.c
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: 串口驱动程序
*/
#include "lib_common.h"
#include "usart.h"
#include "com.h"
#include "main.h"
#include "string.h"

/****************************************************************
 PIN CONFIGURE
 ****************************************************************/
/*缓冲区定义*/
#define MAIN_COM1_RX_LENGTH	256
#define MAIN_COM1_TX_LENGTH	256

#define CDRD_COM2_RX_LENGTH	256
#define CDRD_COM2_TX_LENGTH	256

#define ACT_COM3_RX_LENGTH	256
#define ACT_COM3_TX_LENGTH	256

#define RFID_COM4_RX_LENGTH 256
#define RFID_COM4_TX_LENGTH 256

#define EMBS_COM5_RX_LENGTH 256
#define EMBS_COM5_TX_LENGTH 256


uint8_t rx_buf1[MAIN_COM1_RX_LENGTH];
uint8_t tx_buf1[MAIN_COM1_TX_LENGTH];
uint8_t rx_buf2[CDRD_COM2_RX_LENGTH];
uint8_t tx_buf2[CDRD_COM2_TX_LENGTH];
uint8_t rx_buf3[ACT_COM3_RX_LENGTH];
uint8_t tx_buf3[ACT_COM3_TX_LENGTH];
uint8_t rx_buf4[RFID_COM4_RX_LENGTH];
uint8_t tx_buf4[RFID_COM4_TX_LENGTH];
uint8_t rx_buf5[EMBS_COM5_RX_LENGTH];
uint8_t tx_buf5[EMBS_COM5_TX_LENGTH];


/*pin---usart1*/
static t_pin_source com_main_tx_pin =	
	{GPIOA,	RCC_APB2Periph_GPIOA,	GPIO_Pin_9,		GPIO_Mode_AF_PP,		GPIO_Speed_50MHz,	NULL};
static t_pin_source com_main_rx_pin =	
	{GPIOA,	RCC_APB2Periph_GPIOA,	GPIO_Pin_10,	GPIO_Mode_IN_FLOATING,	GPIO_Speed_50MHz,	NULL};

/*pin---usart2*/
static t_pin_source com_cdrd_tx_pin =
	{GPIOA,	RCC_APB2Periph_GPIOA,	GPIO_Pin_2,		GPIO_Mode_AF_PP,		GPIO_Speed_50MHz,	NULL};
static t_pin_source com_cdrd_rx_pin=
	{GPIOA, RCC_APB2Periph_GPIOA,	GPIO_Pin_3, 	GPIO_Mode_IN_FLOATING,	GPIO_Speed_50MHz,	NULL};

/*PIN---usart3*/
static t_pin_source com_diso_tx_pin=
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_8, GPIO_Mode_AF_PP, GPIO_Speed_50MHz,	NULL};
static t_pin_source com_diso_rx_pin=
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_9, GPIO_Mode_IN_FLOATING,	GPIO_Speed_50MHz,	NULL};

/*pin---usart4*/
static t_pin_source com_rfid_rx_pin=
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_11, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz,	NULL};
static t_pin_source com_rfid_tx_pin=
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_10, GPIO_Mode_AF_PP,	GPIO_Speed_50MHz,	NULL};

/*pin---uart5*/
static t_pin_source com_embs_tx_pin=
	{GPIOC,	RCC_APB2Periph_GPIOC,	GPIO_Pin_12,	GPIO_Mode_AF_PP,		GPIO_Speed_50MHz,	NULL};
static t_pin_source com_embs_rx_pin=
	{GPIOD,	RCC_APB2Periph_GPIOD,	GPIO_Pin_2,	GPIO_Mode_IN_FLOATING,	GPIO_Speed_50MHz,	NULL};
	
static t_usart_buf com_main_tx_buf =	//main com tx buffer
	{tx_buf1,	0,	0,	MAIN_COM1_TX_LENGTH};
static t_usart_buf com_main_rx_buf =	//main com rx buffer
	{rx_buf1,	0,	0,	MAIN_COM1_RX_LENGTH};
static t_usart_buf com_cdrd_tx_buf =	//cdrd com tx buffer
	{tx_buf2,	0,	0,	CDRD_COM2_RX_LENGTH};
static t_usart_buf com_cdrd_rx_buf =	//cdrd com rx buffer
	{rx_buf2,	0,	0,	CDRD_COM2_TX_LENGTH};
static t_usart_buf com_diso_tx_buf =	//cdrd com tx buffer
	{tx_buf3,	0,	0,	ACT_COM3_RX_LENGTH};
static t_usart_buf com_diso_rx_buf =	//cdrd com rx buffer
	{rx_buf3,	0,	0,	ACT_COM3_TX_LENGTH};
static t_usart_buf com_rfid_tx_buf =	//rfid  com tx buffer
	{tx_buf4,	0,	0,	RFID_COM4_TX_LENGTH};
static t_usart_buf com_rfid_rx_buf =	//rfid  com rx buffer
	{rx_buf4,	0,	0,	RFID_COM4_RX_LENGTH};
static t_usart_buf com_embs_tx_buf =	//embs  com tx buffer
	{tx_buf5,	0,	0,	EMBS_COM5_TX_LENGTH};
static t_usart_buf com_embs_rx_buf =	//embs  com rx buffer
	{rx_buf5,	0,	0,	EMBS_COM5_RX_LENGTH};

/*usart1*/
static t_usart_source com_main_source =	
{
	&com_main_tx_pin,			//tx pin
	&com_main_rx_pin,			//rx pin
//	&com_main_cts_pin,			//cts pin
	NULL,
	NULL,						//rts pin

	&com_main_tx_buf,			//tx buf
	&com_main_rx_buf,			//rx buf

	RCC_APB2Periph_USART1,		//clock
	USART1,
	115200,
	USART_WordLength_8b,
	USART_StopBits_1,
	USART_Parity_No,
	USART_HardwareFlowControl_None,		//hareware flow control

	NVIC_PriorityGroup_1,
	USART1_IRQn,
	0x0,
	0x1,

	DMA1_Channel4,				//tx dma
	RCC_AHBPeriph_DMA1,
	DMA1_FLAG_TC4,
	(USART1_BASE + 0x04)
};

/*usart2*/
t_usart_source com_qrcode_scan_source2 =	
{
	&com_cdrd_tx_pin,			//tx pin
	&com_cdrd_rx_pin,			//rx pin
	NULL,						//cts pin
	NULL,						//rts pin
	&com_cdrd_tx_buf,			//tx buf
	&com_cdrd_rx_buf,			//rx buf

	RCC_APB1Periph_USART2,		//clock
	USART2,
	9600,
	USART_WordLength_8b,
	USART_StopBits_1,
	USART_Parity_No,
	USART_HardwareFlowControl_None,		//hareware flow control
	
	NVIC_PriorityGroup_1,
	USART2_IRQn,
	0x1,
	0x0,

/*	DMA1_Channel7,				//tx dma
	RCC_AHBPeriph_DMA1,
	DMA1_FLAG_TC7,
	(USART2_BASE + 0x04)
*/
	NULL,
	NULL,
	NULL,
	NULL,

/*	DMA1_Channel2,				//tx dma
	RCC_AHBPeriph_DMA1,
	DMA1_FLAG_TC2,
	(USART3_BASE + 0x04)*/
};

/*-usart3*/
t_usart_source com_qrcode_scan_source3=
{
		&com_diso_tx_pin,			//tx pin
		&com_diso_rx_pin,			//rx pin
		NULL,						//cts pin
		NULL,						//rts pin
	
		&com_diso_tx_buf,			//tx buf
		&com_diso_rx_buf,			//rx buf
	
		
		RCC_APB1Periph_USART3,		//clock
		USART3,
		9600,  
		USART_WordLength_8b,
		USART_StopBits_1,
		USART_Parity_No,
		USART_HardwareFlowControl_None, 	//hareware flow control
		
		NVIC_PriorityGroup_1,
		USART3_IRQn,
		0x1,
		0x0,
	
/*		DMA1_Channel2,				//tx dma
		RCC_AHBPeriph_DMA1,
		DMA1_FLAG_TC2,
		(USART3_BASE + 0x04)*/
		NULL,
		NULL,
		NULL,
		NULL,
};

/*usart4*/
t_usart_source com_rfid_source =	
{
	&com_rfid_tx_pin,			//tx pin
	&com_rfid_rx_pin,			//rx pin
	NULL,						//cts pin
	NULL,						//rts pin

	&com_rfid_tx_buf,			//tx buf
	&com_rfid_rx_buf,			//rx buf

	
	RCC_APB1Periph_UART4,		//clock
	UART4,
	9600,	 
	USART_WordLength_8b,
	USART_StopBits_1,
	USART_Parity_No,
	USART_HardwareFlowControl_None,		//hareware flow control
	
	NVIC_PriorityGroup_1,
	UART4_IRQn,
	0x1,
	0x0,

	NULL,
	NULL,
	NULL,
	NULL,
};

/*uart5*/
t_usart_source com_embs_source =	
{
	&com_embs_tx_pin,			//tx pin
	&com_embs_rx_pin,			//rx pin
	NULL,						//cts pin
	NULL,						//rts pin

	&com_embs_tx_buf,			//tx buf
	&com_embs_rx_buf,			//rx buf

	
	RCC_APB1Periph_UART5,		//clock
	UART5, 
	9600,	
	USART_WordLength_8b,
	USART_StopBits_1,
	USART_Parity_No,
	USART_HardwareFlowControl_None,		//hareware flow control
	
	NVIC_PriorityGroup_1,
	UART5_IRQn,
	0x1,
	0x0,

	NULL,
	NULL,
	NULL,
	NULL,
	
	//DMA2_Channel5,				//tx dma
	//RCC_AHBPeriph_DMA2,
	//DMA2_FLAG_TC5,
	//(UART4_BASE + 0x04)
};

t_usart_source * usart_array[COM_NUM] =
{
	&com_main_source,
	&com_qrcode_scan_source2,
	&com_qrcode_scan_source3,
	&com_rfid_source,
	&com_embs_source,
};

/****************************************************************
 LOCAL FUNCTIONS 
 ****************************************************************/
static t_usart_source * Com_StructGet(t_com_type com_no)
{
	t_usart_source * p_usart_source;
	if(com_no < COM_NUM)
	{
		p_usart_source = usart_array[com_no];
	}
	else
	{
		p_usart_source = NULL;
	}
	
	return p_usart_source;
}


/****************************************************************
 GLOBAL FUNCTIONS 
 ****************************************************************/
void Com_Init_All(void)
{
	uint8_t i;
	t_usart_source * p_usart_source;
	
	for(i = 0; i < COM_NUM; i++)
	{
		p_usart_source = Com_StructGet((t_com_type)i);
		if(p_usart_source)
		{
			Usart_Init(p_usart_source);
		}
	}
	GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
}

uint8_t Com_Send_Ch(t_com_type com, uint8_t ch)
{
	t_usart_source * p_usart_source;
	uint8_t ret_val;

	p_usart_source = Com_StructGet(com);
	if(p_usart_source)
	{
		ret_val = Usart_SendCh(p_usart_source, ch);
	}
	else
	{
		ret_val = FALSE;
	}

	return ret_val;
}

uint8_t Com_Receive_Ch(t_com_type com, uint8_t *pdata)
{
	t_usart_source * p_usart_source;
	uint8_t ret_val;
	
	p_usart_source = Com_StructGet(com);
	if(p_usart_source)
	{
		ret_val = Usart_GetCh(p_usart_source, pdata);
	}
	else
	{
		ret_val = FALSE;
	}

	return ret_val;
}

uint8_t Com_UnGet_Ch(t_com_type com, uint8_t data)
{
	t_usart_source * p_usart_source;
	uint8_t ret_val;
	
	p_usart_source = Com_StructGet(com);
	if(p_usart_source)
	{
		ret_val = Usart_UngetCh(p_usart_source, data);
	}
	else
	{
		ret_val = FALSE;
	}

	return ret_val;
}

uint8_t Com_UnGet_Buf(t_com_type com, uint8_t * buf, uint8_t size)
{
	t_usart_source * p_usart_source;
	uint8_t ret_val, i;
	uint8_t * p_data;
	
	p_usart_source = Com_StructGet(com);
	p_data = buf;
	if(p_usart_source)
	{
		for(i = 0; i < size; i++)
		{
			p_data = buf + i;
			if(!Usart_UngetCh(p_usart_source, (*p_data)))
			{
				ret_val = FALSE;
				break;
			}
		}
		ret_val = TRUE;
	}
	else
	{
		ret_val = FALSE;
	}

	return ret_val;
}

void Com_Send_Buffer(t_com_type com, char *ptx_buff, u32  tx_size)
{
	t_usart_source * p_usart_source;
	
	p_usart_source = Com_StructGet(com);
	
	if(p_usart_source)
	{
		Usart_SendBuf(p_usart_source, (uint8_t *)ptx_buff, tx_size);
	}
}

void Com_Enable(t_com_type com, uint8_t enable)
{
	t_usart_source * p_usart_source;
	
	p_usart_source = Com_StructGet(com);
	if(p_usart_source)
	{
		if(enable)
		{
			Usart_Enable(p_usart_source, ENABLE);
		}
		else
		{
			Usart_Enable(p_usart_source, DISABLE);
		}
	}
}


void Com_Set_Baud(t_com_type com, uint32_t baud, uint16_t wordlength, uint16_t stopbits, uint16_t parity)
{
	t_usart_source * p_usart_source;
	
	p_usart_source = Com_StructGet(com);
	if(p_usart_source)
	{
		Usart_SetBaudRate(p_usart_source, baud, wordlength, stopbits, parity, USART_HardwareFlowControl_None);
	}
}

void Comx_IRQHandler(t_com_type com)
{
	t_usart_source * p_usart_source;
	
	p_usart_source = Com_StructGet(com);
	if(p_usart_source)
	{
		Usart_IRQ_Handler(p_usart_source);
	}
}

void Get_QRCodeScan_Usart2(t_usart_source * p_usart_source)
{
	memcpy(p_usart_source, &com_qrcode_scan_source2, sizeof(t_usart_source));
}

void Get_QRCodeScan_Usart3(t_usart_source * p_usart_source)
{
	memcpy(p_usart_source, &com_qrcode_scan_source3, sizeof(t_usart_source));
}




