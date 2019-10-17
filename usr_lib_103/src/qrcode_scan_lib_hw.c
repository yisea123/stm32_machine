/*
	Honeywell二维码扫描驱动
*/
#include "pin.h"
#include "qrcode_scan_lib.h"
#include "qrcode_scan_lib_hw.h"
#include "string.h"

typedef enum
{
	//1D Code
	UPC_A = 0,
	UPC_E,
	UPC_E1,
	EAN_8,
	EAN_13,
	BOOKLAND_EAN,
	CODE_128,
	GS1_128,
	ISBT_128,
	CODE_39,
	TRIOPIC_CODE_39,
	CODE_93,
	CODE_11,
	INTERLENVED_2_OF_5,
	DISCRETE_2_OF_5,
	CODEBAR,
	MSI,

	//2d code
	QR_CODE,
	PDF417,
	DATA_MATRIX,
	AZTEC,
	MICRO_PDF417,
	MAXI_CODE,
	BARCODE_INDEX_NUM
}barcode_index_hw_t;

//N4313条码类型设置命令集
uint8_t barcodetype[BARCODE_INDEX_NUM][6]=
{
	//1D CODE
	{'U', 'P', 'B','E','N','A'},//UPC_A
	{'U', 'P', 'E','E','N','0'},//UPC_E
	{'U', 'P', 'E','E','N','0'},//UPC_E0
	{'E', 'A', '8','E','N','A'},//EAN_8
	{'E', '1', '3','E','N','A'},//EAN_13
	{'E', '1', '3','I','S','S'},//BOOKLAND_EAN
	{'1', '2', '8','E','N','A'},//code128
	{'G', 'S', '1','E','N','A'},//GS1_128
	{'I', 'S', 'B','E','N','A'},//ISBT_128	
	{'C', '3', '9','E','N','A'},//CODE_39	
	{'T', 'R', 'I','E','N','A'},//TRIOPIC_CODE_39	
	{'C', '9', '3','E','N','A'},//CODE_93
	{'C', '1', '1','E','N','A'},//CODE_11
	{'I', '2', '5','E','N','A'},//INTERLEAVED_2_OF_5,
	{'R', '2', '5','E','N','A'},//DISCRETE_2_OF_5
	{'C', 'B', 'R','E','N','A'},//CODABAR
	{'M', 'S', 'I','E','N','A'},//MSI

	//2D CODE
	{'Q', 'R', 'C','E','N','A'},//QR Code
	{'P', 'D', 'F','E','N','A'},//PDF417
	{'I', 'D', 'M','E','N','A'},//Data Matrix
	{'A', 'Z', 'T','E','N','A'},//Aztec Code
	{'M', 'P', 'D','E','N','A'},//Micro PDF417
	{'M', 'A', 'X','E','N','A'},//Maxi Code
};

/*---------------------------------------------------------------------*\
|	@brief	Set or Reset CPU's RTS pin LOW to request N4313 for		|
|			transmitting data.											|
|	@param	*N4313:	Checking which N4313.							|
|			*ENABLE:	Pull RTS pin LOW. (activate)					|
|			*DISABLE:	Pull RTS pin HIGH. (deactivate)					|
|	@retval None.														|
\*---------------------------------------------------------------------*/
static void QRCodeScanHw_MCURequestToSend(qrcode_scan_hw_rc_t * rc,
									FunctionalState enable)
{
	volatile uint8_t test_value = PIN_SET;
	if(enable)
		Pin_WriteBit(rc->com->rts_pin, LOW);
	else
		Pin_WriteBit(rc->com->rts_pin, HIGH);
	test_value = Pin_ReadBit(rc->com->cts_pin);
}


/*---------------------------------------------------------------------*\
|	@brief	Check that if the N4313 is ready to receive message or not|
|	@param	N4313:Checking which N4313.								|
|	@retval *FALSE:	The N4313 is not ready to receive message.		|
|			*TRUE:	The N4313 is ready to receive message.			|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_IsN4313ReadyToReceive(qrcode_scan_hw_rc_t * rc)
{
#if 1
	return 1;
#else
	uint16_t waitingTimeRemained = 1000;
	uint8_t result = FALSE;
	
	QRCodeScanHw_MCURequestToSend(rc, ENABLE);
	while(Pin_ReadBit(rc->com->cts_pin) && \
		waitingTimeRemained)
	{
		waitingTimeRemained--;
		QRCODESCAN_DELAY_MS(1);
	}
	if(waitingTimeRemained)
		result = TRUE;
	
	return result;
#endif
}

/*---------------------------------------------------------------------*\
|	@brief	Check that if the N4313 has completed receiving message	|
|			or not.														|
|	@param	N4313:Checking which N4313.								|
|	@retval *FALSE:	The N4313 has not completed the data reception.	|
|			*TRUE:	The N4313 has completed the data reception.		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_IsN4313ReceiveCompleted(qrcode_scan_hw_rc_t * rc)
{
#if 1
	return 1;
#else
	uint16_t waitingTimeRemained = 50;//等待超时设定50ms
	uint8_t result = FALSE;
	
	QRCodeScanHw_MCURequestToSend(rc, DISABLE);
	while(!Pin_ReadBit(rc->com->cts_pin) && \
		waitingTimeRemained){
		waitingTimeRemained--;
		QRCODESCAN_DELAY_MS(1);
	}
	if(waitingTimeRemained)
		result = TRUE;
	return result;
#endif
}

/*---------------------------------------------------------------------*\
|	@brief	Send a SSI command to the N4313 and check the N4313 has	|
|			taken back the transmission permission or not.				|
|	@param	N4313:		Sending from which N4313.					|
|	@retval *COMM_FAILED:	The  has not taken back the			|
|							transmission permission yet.				|
|			*COMM_SUCCESSED:The N4313 has taken back the transmission	|
|							permission already.							|
\*---------------------------------------------------------------------*/

static int8_t QRCodeScanHw_TxMsg(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED;
	unsigned char ch;

	if(QRCodeScanHw_IsN4313ReadyToReceive(rc))
	{
		//N4313准备接受串口数据
		while(Usart_GetCh(rc->com, &ch));		//清理可能存在的多余数据，防止二维码串码
		Usart_SendBuf(rc->com, rc->tx_msg, rc->msg_length);//串口发送数据
		Usart_WaitSendCompleted(rc->com);//等待发送完毕

		if(QRCodeScanHw_IsN4313ReceiveCompleted(rc))
			result = COMM_SUCCESSED;
		else
			result = COMM_FAILED;
	}
	return result;
}

/*---------------------------------------------------------------------*\
|	@brief	Receive message from the N4313.							|
|	@param	N4313:Receiving message from which N4313.				|
|	@retval length:	The length of the message that N4313 received.	|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_RxMsg(qrcode_scan_hw_rc_t * rc)
{
	
	unsigned short waitingTimeRemained = 200;//等待超时设定200ms
	unsigned char ch;
	uint8_t length = 0;

	// start to receive decoded data
	while(waitingTimeRemained)
	{
		if(Usart_GetCh(rc->com, &ch))//从串口缓冲区中取出一个字节
		{			
			waitingTimeRemained = 10;
			rc->rx_msg[length] = ch;//将取出的字节保存在N4313接受缓冲区中
			length++;
		}
		else
		{
			waitingTimeRemained--;
			QRCODESCAN_DELAY_MS(1);//延时1ms
		}
	}
	return length;//返回接收到得字符串长度，没接收到数据返回0
}

/*---------------------------------------------------------------------*\
|	@brief	判断扫描仪类型是一维N4313还是二维N5680.							|
|	@param	N4313: 获取扫描仪类型参数.				|
|	@retval  n4313_scan :	N4313扫描仪.	
			n5680_scan:  N5680扫描仪
			unknow_scan:未知扫描仪		|
\*---------------------------------------------------------------------*/
static scan_device_type_t QRCodeScanHw_ReadProductid(qrcode_scan_hw_rc_t * rc)
{
	scan_device_type_t result = unknow_scan;
	uint8_t i,length;
	uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, 'p', '_', 'n','a','m','e','?',0x21};
	for(i=0;i<11;i++)
		rc->tx_msg[i]=scan_TxMsg[i];//将指令保存到N4313发送缓冲区中
	rc->msg_length=i;//保存发送命令长度
	QRCodeScanHw_TxMsg(rc); //发送命令
	length=QRCodeScanHw_RxMsg(rc);//获取扫描仪返回数据，函数返回接收到得数据长度
	if(length)//根据返回字符串中某些字符判断扫描仪类型
	{
		if(((rc->rx_msg[20])==0x34)&&((rc->rx_msg[21])==0x33)&&((rc->rx_msg[22])==0x30)&&((rc->rx_msg[23])==0x30))
			result=n4313_scan;
		else if(((rc->rx_msg[17])==0x35)&&((rc->rx_msg[18])==0x36)&&((rc->rx_msg[19])==0x30)&&((rc->rx_msg[20])==0x30))
		 	result = n5680_scan;
		else if(((rc->rx_msg[8])==0x35)&&((rc->rx_msg[9])==0x31)&&((rc->rx_msg[10])==0x38)&&((rc->rx_msg[11])==0x30))
			result=n5180_scan;
	}
	else
		 result = unknow_scan;
	return result;
}

/*---------------------------------------------------------------------*\
|	@brief	设置扫描仪出厂参数							|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static int8_t QRCodeScanHw_SetAllParamDefaults(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[10] = {0x16, 0x4d, 0x0d, 'D', 'E', 'F','A','L','T',0x21};
	for(i=0;i<10;i++)
	{
		rc->tx_msg[i]=scan_TxMsg[i];
	}
	rc->msg_length=i;
	QRCodeScanHw_TxMsg(rc); //发送命令
	length = QRCodeScanHw_RxMsg(rc);//获取扫描仪返回数据，函数返回接收到得数据长度
	if(length)
	{
		if((rc->rx_msg[6]) == 0x06)//扫描仪返回数据校检
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;
}
/*---------------------------------------------------------------------*\
|	@brief	关闭扫描仪所有条码类型识别							|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_DisableAllSymbologies(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, 'A', 'L', 'L','E','N','A','0',0x21};	
	for(i=0;i<11;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[7])==0x06)//扫描仪返回数据校检
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;	
}

/*---------------------------------------------------------------------*\
|	@brief	允许扫描仪所有条码类型识别							|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_EnableAllSymbologies(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, 'A', 'L', 'L','E','N','A','1',0x21};	
	for(i=0;i<11;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[7])==0x06)//扫描仪返回数据校检
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;	
}

/*---------------------------------------------------------------------*\
|	@brief	设置默认CODE128类型条码识别							|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_SetDefaultSymbologies(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, '1', '2', '8','E','N','A','1',0x21};	
	for(i=0;i<11;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[7])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;		
}
/*---------------------------------------------------------------------*\
|	@brief	清除扫描条码返回的后缀							|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_ClearAllSuffix(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, 'S', 'U', 'F','C','A','2',0x21};	
	for(i=0;i<11;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[6])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;		
}

/*---------------------------------------------------------------------*\
|	@brief	清除扫描条码返回的后缀							|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_AddSuffix(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	//uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, 'S', 'U', 'F','C','A','2',0x21};
	uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, 'V', 'S', 'U','F','C','R',0x21};
	for(i=0;i<11;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[6])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;		
}

/*---------------------------------------------------------------------*\
|	@brief	清除扫描条码返回的前缀ID						|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_CleanAllPrefix(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[] = {0x16, 0x4d, 0x0d, 'P', 'R', 'E','C','A','2',0x21};
	for(i=0;i<sizeof(scan_TxMsg);i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[20])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;		
}

/*---------------------------------------------------------------------*\
|	@brief	设置扫描条码返回的前缀ID						|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_AddIDPrefix(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[20] = {0x16, 0x4d, 0x0d, 'P', 'R', 'E','C','A','2',',','B','K','2','9','9','5','C','8','0',0x21};	
	for(i=0;i<20;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[20])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;		
}
/*---------------------------------------------------------------------*\
|	@brief	设置扫描仪安全级别					|
|	|@param	*N4313:			设置N4313安全级别.|
|			*SECURITY_LEVEL_1:	Set security level to 1.				|
|			*SECURITY_LEVEL_2:	Set security level to 2.				|
|			*SECURITY_LEVEL_3:	Set security level to 3.				|
|			*SECURITY_LEVEL_4:	Set security level to 4.				|		|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static int8_t QRCodeScanHw_SetSecurityLevel(qrcode_scan_hw_rc_t * rc, uint8_t securityLevel)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[10] = {0x16, 0x4d, 0x0d, 'P', 'A', 'P','L','S','1',0x21};	
	for(i=0;i<10;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	switch(securityLevel)//设置安全扫描级别
	{
		case 0x01:
			rc->tx_msg[8]='1';
			break;
		case 0x02:
			rc->tx_msg[8]='2';
			break;
		case 0x03:
			rc->tx_msg[8]='3';
			break;
		case 0x04:
			rc->tx_msg[8]='4';
			break;
		default:
			return COMM_FAILED;
	}
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[6])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;		
}
/*---------------------------------------------------------------------*\
|	@brief	设置扫描失败超时timeout时间					|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static int8_t QRCodeScanHw_ScanTimeout(qrcode_scan_hw_rc_t * rc, unsigned int timeout_ms)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[14] = {0x16, 0x4d, 0x0d, 'T', 'R', 'G','S','T','O','5','0','0','0',0x21};

	i = 9;
	scan_TxMsg[i++] = ((timeout_ms / 1000) % 10) + '0';
	scan_TxMsg[i++] = ((timeout_ms / 100) % 10) + '0';
	scan_TxMsg[i++] = ((timeout_ms / 10) % 10) + '0';
	scan_TxMsg[i++] = ((timeout_ms / 1) % 10) + '0';
	for(i=0;i<14;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[10])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;		
}
/*---------------------------------------------------------------------*\
|	@brief	关闭扫描条码返回类型UPCA自动转EAN13	功能			|
|	@param	无			|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static int8_t QRCodeScanHw_CloseUPCA_TO_EAN13(qrcode_scan_hw_rc_t * rc)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, 'U', 'P', 'A','E','N','A','1',0x21};	
	for(i=0;i<14;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[7])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;		
}
/*---------------------------------------------------------------------*\
|	@brief	开启硬件触发扫描					|
|	@param	无			|
|	@retval  无				|
\*---------------------------------------------------------------------*/
static void QRCodeScanHw_ActivateTrigger(qrcode_scan_hw_rc_t * rc)
{
	if(rc->trigger_pin)
		Pin_WriteBit(rc->trigger_pin, LOW);
}
/*---------------------------------------------------------------------*\
|	@brief	关闭硬件触发扫描					|
|	@param	无			|
|	@retval  无				|
\*---------------------------------------------------------------------*/
static void QRCodeScanHw_DeactivateTrigger(qrcode_scan_hw_rc_t * rc)
{
	if(rc->trigger_pin)
		Pin_WriteBit(rc->trigger_pin, HIGH);
}
/*---------------------------------------------------------------------*\
|	@brief	获取N4313串口返回数据					|
|	@param	无			|
|	@retval  length:获取字符串长度
		|
\*---------------------------------------------------------------------*/
static uint8_t QRCodeScanHw_RxDeocdedData(qrcode_scan_hw_rc_t * rc, unsigned int timeout_ms)
{
	uint16_t waitingTimeRemained = timeout_ms;
	uint8_t ch;
	uint8_t length = 0;

	// start to receive decoded data
	while(waitingTimeRemained)
	{
		if(Usart_GetCh(rc->com, &ch))
		{
			waitingTimeRemained = 20;
			rc->rx_msg[length] = ch;
			length++;
		}
		else
		{
			waitingTimeRemained--;
			QRCODESCAN_DELAY_MS(1);
		}
	}
	return length;
}


/*---------------------------------------------------------------------*\
|	@brief	硬件触发扫描条码			|
|	@param	无			|
|	@retval  length:获取字符串长度
		|
\*---------------------------------------------------------------------*/
static int8_t QRCodeScanHw_HWDecodeData(qrcode_scan_hw_rc_t * rc, unsigned int timeout_ms)
{
	uint8_t length = 0;
	QRCodeScanHw_ActivateTrigger(rc);//开启硬件扫描
	length = QRCodeScanHw_RxDeocdedData(rc, timeout_ms);
	QRCodeScanHw_DeactivateTrigger(rc);//关闭硬件扫描
	QRCODESCAN_DELAY_MS(10);
	return length;
}
/*---------------------------------------------------------------------*\
|	@brief	软件触发扫描条码			|
|	@param	无			|
|	@retval  length:获取字符串长度
		|
\*---------------------------------------------------------------------*/
static int8_t QRCodeScanHw_SSIDecodeData(qrcode_scan_hw_rc_t * rc, unsigned int timeout_ms)
{
	int8_t i,length=0;
	uint8_t scan_TxMsg[3] = {0x16, 0x54, 0x0d};	
	uint8_t scan_TxMsg1[3] = {0x16, 0x55, 0x0d};
	for(i=0;i<3;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;
	QRCodeScanHw_TxMsg(rc); //开启软件扫描
	length = QRCodeScanHw_RxDeocdedData(rc, timeout_ms);//获取返回条码值
	for(i=0;i<3;i++)
		rc->tx_msg[i]=scan_TxMsg1[i];
	rc->msg_length=i;
	QRCodeScanHw_TxMsg(rc); //关闭软件扫描
	return length;
}

/*---------------------------------------------------------------------*\
|	@brief	条码类型识别使能		|
|	@param	* N4313:
			 codeType:条码类型参数	
			 enable:  0  关闭
			 		1 开启				|
|	@retval  COMM_SUCCESSED: 设置成功
			COMM_FAILED:  设置失败
		|
\*---------------------------------------------------------------------*/
static int QRCodeScanHw_SetCodeType(qrcode_scan_hw_rc_t * rc, barcode_index_hw_t barcode_index, uint8_t enable)
{
	int8_t result = COMM_FAILED,i=0,length=0;

	if(!rc)
		return -1;
	rc->tx_msg[0]=0x16;
	rc->tx_msg[1]=0x4d;
	rc->tx_msg[2]=0x0d;
	if(barcode_index == UPC_A)
	{
		QRCodeScanHw_CloseUPCA_TO_EAN13(rc);//关闭UPC_A条码自动转换成EAN13
	}
	for(i=0;i<6;i++)
	{
		rc->tx_msg[i+3]=barcodetype[barcode_index][i];//将条码类型设置指令保存到N4313发送缓冲区
	}
	if(enable)
		rc->tx_msg[i+3]='1';//开启设置
	else
		rc->tx_msg[i+3]='0';//关闭设置
	i++;
	rc->tx_msg[i+3]=0x21;//保存到扫描仪ram中，掉电丢失
	rc->msg_length=i+4;	
	QRCodeScanHw_TxMsg(rc); 
	length=QRCodeScanHw_RxMsg(rc);
	if(length)
	{
		if((rc->rx_msg[7])==0x06)
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;				
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanHw_Config(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config)
{
	qrcode_scan_hw_rc_t * rc;

	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	qrcode_scan->qrcode_scan_conf.tirgger_mode = config->tirgger_mode;
	qrcode_scan->qrcode_scan_conf.security_level = config->security_level;
	rc->scan_device_type=QRCodeScanHw_ReadProductid(rc);//获取扫描仪类型
	if(rc->scan_device_type==n4313_scan)//如果是4313则恢复出厂参数，关闭所有条码识别，开启CODE 128条码识别
	{
		QRCodeScanHw_SetAllParamDefaults(rc);
		QRCodeScanHw_DisableAllSymbologies(rc);
		QRCodeScanHw_SetDefaultSymbologies(rc);
	}
	QRCodeScanHw_ClearAllSuffix(rc);
	QRCodeScanHw_AddSuffix(rc);
	QRCodeScanHw_ScanTimeout(rc, 5000);
#if DISABLE_ALL_PREFIX_SUFFIX
	QRCodeScanHw_CleanAllPrefix(rc);
#else
	QRCodeScanHw_AddIDPrefix(rc);
#endif
	QRCodeScanHw_SetSecurityLevel(rc, qrcode_scan->qrcode_scan_conf.security_level);
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanHw_GetConfig(struct _qrcode_scan_t * qrcode_scan, qrcode_scan_conf_t * config)
{
	qrcode_scan_hw_rc_t * rc;

	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	config->tirgger_mode = qrcode_scan->qrcode_scan_conf.tirgger_mode;
	config->security_level = qrcode_scan->qrcode_scan_conf.security_level;
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanHw_EnableCodeType(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type)
{
	qrcode_scan_hw_rc_t * rc;
	barcode_index_hw_t i;

	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	switch(type)
	{
		case BARCODE_TYPE_ALL_1D:
			for(i = (barcode_index_hw_t)0; i < QR_CODE; i++)
			{
				if(QRCodeScanHw_SetCodeType(rc, i, 1))
				{
					return -1;
				}
			}
			break;
		case BARCODE_TYPE_ALL_2D:
			for(i = QR_CODE; i < BARCODE_INDEX_NUM; i++)
			{
				if(QRCodeScanHw_SetCodeType(rc, i, 1))
				{
					return -1;
				}
			}
			break;
		case BARCODE_TYPE_QRCODE:
			if(QRCodeScanHw_SetCodeType(rc, QR_CODE, 1))
			{
				return -1;
			}
			break;
		case BARCODE_TYPE_ALL:
			if(QRCodeScanHw_EnableAllSymbologies(rc) != COMM_SUCCESSED)
			{
				return -1;
			}
			break;
		default:
			return -1;
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanHw_DisableCodeType(struct _qrcode_scan_t * qrcode_scan, barcode_type_t type)
{
	qrcode_scan_hw_rc_t * rc;
	barcode_index_hw_t i;

	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	switch(type)
	{
		case BARCODE_TYPE_ALL_1D:
			//disable all symbologies
			if(QRCodeScanHw_DisableAllSymbologies(rc) != COMM_SUCCESSED)
				return -1;
			//enable 2d code
			for(i = QR_CODE; i < BARCODE_INDEX_NUM; i++)
			{
				if(QRCodeScanHw_SetCodeType(rc, i, 1))
				{
					return -1;
				}
			}
			break;
		case BARCODE_TYPE_ALL_2D:
			for(i = QR_CODE; i < BARCODE_INDEX_NUM; i++)
			{
				if(QRCodeScanHw_SetCodeType(rc, i, 0))
				{
					return -1;
				}
			}
			break;
		case BARCODE_TYPE_QRCODE:
			if(QRCodeScanHw_SetCodeType(rc, QR_CODE, 0))
			{
				return -1;
			}
			break;
		case BARCODE_TYPE_ALL:
			if(QRCodeScanHw_DisableAllSymbologies(rc) != COMM_SUCCESSED)
			{
				return -1;
			}
			break;
		default:
			return -1;
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanHw_StartScan(struct _qrcode_scan_t * qrcode_scan)
{
	qrcode_scan_hw_rc_t * rc;

	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	if(qrcode_scan->qrcode_scan_conf.tirgger_mode == TRIGGER_HW)//根据参数设置，判断软件触发或者硬件触发
	{
		QRCodeScanHw_ActivateTrigger(rc);//开启硬件扫描
	}
	else
	{
		int8_t i;
		uint8_t scan_TxMsg[3] = {0x16, 0x54, 0x0d};	
		for(i = 0; i < 3; i++)
			rc->tx_msg[i] = scan_TxMsg[i];
		rc->msg_length = i;
		QRCodeScanHw_TxMsg(rc); //开启软件扫描
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanHw_GetScanResult(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms)
{
	qrcode_scan_hw_rc_t * rc;
	int8_t length = 0;
	unsigned short i = 0;

	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	
	length = QRCodeScanHw_RxDeocdedData(rc, timeout_ms);
	if(qrcode_scan->qrcode_scan_conf.tirgger_mode == TRIGGER_HW)
	{
		QRCodeScanHw_DeactivateTrigger(rc);//关闭硬件扫描
		QRCODESCAN_DELAY_MS(10);
	}
	else
	{
		int8_t i;
		uint8_t scan_TxMsg1[3] = {0x16, 0x55, 0x0d};
		for(i = 0; i < 3; i++)
			rc->tx_msg[i] = scan_TxMsg1[i];
		rc->msg_length = i;
		QRCodeScanHw_TxMsg(rc); //关闭软件扫描
	}
	if(length)//如果扫描成功，将N4313定义的条码类型ID转换成SE955兼容的ID号
	{
#if (!DISABLE_ALL_PREFIX_SUFFIX)
		switch(rc->rx_msg[0])//霍尼韦尔返回条码第一字节为条码类型ID
		{
			case 'j':
				rc->rx_msg[0]=0x03;//CODE 128
				break;
			case 'D':
				rc->rx_msg[0]=0x0a;//EAN 8
				break;
			case 'c':
				rc->rx_msg[0]=0x08;//UPC_A
				break;
			case 'E':
				rc->rx_msg[0]=0x09;//UPC_E
				break;
			case 'd':
				rc->rx_msg[0]=0x0b;//EAN_A3
				break;	
			case 'I':
				rc->rx_msg[0]=0x0f;//GS1_128
				break;	
			case 'b':
				rc->rx_msg[0]=0x01;//CODE_39
				break;	
			case 'i':
				rc->rx_msg[0]=0x07;//CODE_93
				break;	
			case 'e':
				rc->rx_msg[0]=0x06;//INTERLEAVED_2_OF_5
				break;		
			case 'a':
				rc->rx_msg[0]=0x02;//CODEBAR
				break;	
			case 'g':
				rc->rx_msg[0]=0x0e;//MSI
				break;					
			default:
				break;
		}
#endif
		for(i = 0; i < length; i++)		//利用后缀0x0D截断区分两条二维码
		{
			if(rc->rx_msg[i] < 0x20)
			{
				length = i;
				break;
			}
		}
		memcpy(buff, rc->rx_msg, length);
		*len = length;
	}
	else
	{
		return -1;
	}
	return 0;
}

/* 
success : return 0
else    : return !0
*/
static int QRCodeScanHw_Read(struct _qrcode_scan_t * qrcode_scan, unsigned char * buff, unsigned int * len, unsigned int timeout_ms)
{
	qrcode_scan_hw_rc_t * rc;
	int8_t length = 0;
	unsigned short i = 0;

	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	if(qrcode_scan->qrcode_scan_conf.tirgger_mode == TRIGGER_HW)//根据参数设置，判断软件触发或者硬件触发
		length=QRCodeScanHw_HWDecodeData(rc, timeout_ms);
	else
		length= QRCodeScanHw_SSIDecodeData(rc, timeout_ms);
	if(length)//如果扫描成功，将N4313定义的条码类型ID转换成SE955兼容的ID号
	{
#if (!DISABLE_ALL_PREFIX_SUFFIX)
		switch(rc->rx_msg[0])//霍尼韦尔返回条码第一字节为条码类型ID
		{
			case 'j':
				rc->rx_msg[0]=0x03;//CODE 128
				break;
			case 'D':
				rc->rx_msg[0]=0x0a;//EAN 8
				break;
			case 'c':
				rc->rx_msg[0]=0x08;//UPC_A
				break;
			case 'E':
				rc->rx_msg[0]=0x09;//UPC_E
				break;
			case 'd':
				rc->rx_msg[0]=0x0b;//EAN_A3
				break;	
			case 'I':
				rc->rx_msg[0]=0x0f;//GS1_128
				break;	
			case 'b':
				rc->rx_msg[0]=0x01;//CODE_39
				break;	
			case 'i':
				rc->rx_msg[0]=0x07;//CODE_93
				break;	
			case 'e':
				rc->rx_msg[0]=0x06;//INTERLEAVED_2_OF_5
				break;		
			case 'a':
				rc->rx_msg[0]=0x02;//CODEBAR
				break;	
			case 'g':
				rc->rx_msg[0]=0x0e;//MSI
				break;					
			default:
				break;
		}
#endif
		for(i = 0; i < length; i++)		//利用后缀0x0D截断区分两条二维码
		{
			if(rc->rx_msg[i] < 0x20)
			{
				length = i;
				break;
			}
		}
		memcpy(buff, rc->rx_msg, length);
		*len = length;
	}
	return 0;
}

qrcode_scan_ops_t qrcode_scan_hw_ops =
{
	.config = QRCodeScanHw_Config,
	.get_config = QRCodeScanHw_GetConfig,
	.enable_code_type = QRCodeScanHw_EnableCodeType,
	.disable_code_type = QRCodeScanHw_DisableCodeType,
	.read = QRCodeScanHw_Read,
	.start_scan = QRCodeScanHw_StartScan,
	.get_scan_result = QRCodeScanHw_GetScanResult,
};

/* 
success : return 0
else    : return !0
*/
int QRCodeScanHw_Install(qrcode_scan_t * qrcode_scan)
{
	qrcode_scan_hw_rc_t * rc;

	rc = qrcode_scan->resource;
	if((!qrcode_scan) || (!rc))
		return -1;
	qrcode_scan->install = QRCodeScanHw_Install;
	qrcode_scan->ops = &qrcode_scan_hw_ops;
	return 0;
}


