/*
	Honeywell��ά��ɨ������
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

//N4313���������������
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
	uint16_t waitingTimeRemained = 50;//�ȴ���ʱ�趨50ms
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
		//N4313׼�����ܴ�������
		while(Usart_GetCh(rc->com, &ch));		//������ܴ��ڵĶ������ݣ���ֹ��ά�봮��
		Usart_SendBuf(rc->com, rc->tx_msg, rc->msg_length);//���ڷ�������
		Usart_WaitSendCompleted(rc->com);//�ȴ��������

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
	
	unsigned short waitingTimeRemained = 200;//�ȴ���ʱ�趨200ms
	unsigned char ch;
	uint8_t length = 0;

	// start to receive decoded data
	while(waitingTimeRemained)
	{
		if(Usart_GetCh(rc->com, &ch))//�Ӵ��ڻ�������ȡ��һ���ֽ�
		{			
			waitingTimeRemained = 10;
			rc->rx_msg[length] = ch;//��ȡ�����ֽڱ�����N4313���ܻ�������
			length++;
		}
		else
		{
			waitingTimeRemained--;
			QRCODESCAN_DELAY_MS(1);//��ʱ1ms
		}
	}
	return length;//���ؽ��յ����ַ������ȣ�û���յ����ݷ���0
}

/*---------------------------------------------------------------------*\
|	@brief	�ж�ɨ����������һάN4313���Ƕ�άN5680.							|
|	@param	N4313: ��ȡɨ�������Ͳ���.				|
|	@retval  n4313_scan :	N4313ɨ����.	
			n5680_scan:  N5680ɨ����
			unknow_scan:δ֪ɨ����		|
\*---------------------------------------------------------------------*/
static scan_device_type_t QRCodeScanHw_ReadProductid(qrcode_scan_hw_rc_t * rc)
{
	scan_device_type_t result = unknow_scan;
	uint8_t i,length;
	uint8_t scan_TxMsg[11] = {0x16, 0x4d, 0x0d, 'p', '_', 'n','a','m','e','?',0x21};
	for(i=0;i<11;i++)
		rc->tx_msg[i]=scan_TxMsg[i];//��ָ��浽N4313���ͻ�������
	rc->msg_length=i;//���淢�������
	QRCodeScanHw_TxMsg(rc); //��������
	length=QRCodeScanHw_RxMsg(rc);//��ȡɨ���Ƿ������ݣ��������ؽ��յ������ݳ���
	if(length)//���ݷ����ַ�����ĳЩ�ַ��ж�ɨ��������
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
|	@brief	����ɨ���ǳ�������							|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
	QRCodeScanHw_TxMsg(rc); //��������
	length = QRCodeScanHw_RxMsg(rc);//��ȡɨ���Ƿ������ݣ��������ؽ��յ������ݳ���
	if(length)
	{
		if((rc->rx_msg[6]) == 0x06)//ɨ���Ƿ�������У��
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;
}
/*---------------------------------------------------------------------*\
|	@brief	�ر�ɨ����������������ʶ��							|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
		if((rc->rx_msg[7])==0x06)//ɨ���Ƿ�������У��
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;	
}

/*---------------------------------------------------------------------*\
|	@brief	����ɨ����������������ʶ��							|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
		if((rc->rx_msg[7])==0x06)//ɨ���Ƿ�������У��
			result=COMM_SUCCESSED;
		else
		 	result = COMM_FAILED;			
	}
	else
		 result = COMM_FAILED;
	return result;	
}

/*---------------------------------------------------------------------*\
|	@brief	����Ĭ��CODE128��������ʶ��							|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
|	@brief	���ɨ�����뷵�صĺ�׺							|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
|	@brief	���ɨ�����뷵�صĺ�׺							|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
|	@brief	���ɨ�����뷵�ص�ǰ׺ID						|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
|	@brief	����ɨ�����뷵�ص�ǰ׺ID						|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
|	@brief	����ɨ���ǰ�ȫ����					|
|	|@param	*N4313:			����N4313��ȫ����.|
|			*SECURITY_LEVEL_1:	Set security level to 1.				|
|			*SECURITY_LEVEL_2:	Set security level to 2.				|
|			*SECURITY_LEVEL_3:	Set security level to 3.				|
|			*SECURITY_LEVEL_4:	Set security level to 4.				|		|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
		|
\*---------------------------------------------------------------------*/
static int8_t QRCodeScanHw_SetSecurityLevel(qrcode_scan_hw_rc_t * rc, uint8_t securityLevel)
{
	int8_t result = COMM_FAILED,i,length;
	uint8_t scan_TxMsg[10] = {0x16, 0x4d, 0x0d, 'P', 'A', 'P','L','S','1',0x21};	
	for(i=0;i<10;i++)
		rc->tx_msg[i]=scan_TxMsg[i];
	rc->msg_length=i;	
	switch(securityLevel)//���ð�ȫɨ�輶��
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
|	@brief	����ɨ��ʧ�ܳ�ʱtimeoutʱ��					|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
|	@brief	�ر�ɨ�����뷵������UPCA�Զ�תEAN13	����			|
|	@param	��			|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
|	@brief	����Ӳ������ɨ��					|
|	@param	��			|
|	@retval  ��				|
\*---------------------------------------------------------------------*/
static void QRCodeScanHw_ActivateTrigger(qrcode_scan_hw_rc_t * rc)
{
	if(rc->trigger_pin)
		Pin_WriteBit(rc->trigger_pin, LOW);
}
/*---------------------------------------------------------------------*\
|	@brief	�ر�Ӳ������ɨ��					|
|	@param	��			|
|	@retval  ��				|
\*---------------------------------------------------------------------*/
static void QRCodeScanHw_DeactivateTrigger(qrcode_scan_hw_rc_t * rc)
{
	if(rc->trigger_pin)
		Pin_WriteBit(rc->trigger_pin, HIGH);
}
/*---------------------------------------------------------------------*\
|	@brief	��ȡN4313���ڷ�������					|
|	@param	��			|
|	@retval  length:��ȡ�ַ�������
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
|	@brief	Ӳ������ɨ������			|
|	@param	��			|
|	@retval  length:��ȡ�ַ�������
		|
\*---------------------------------------------------------------------*/
static int8_t QRCodeScanHw_HWDecodeData(qrcode_scan_hw_rc_t * rc, unsigned int timeout_ms)
{
	uint8_t length = 0;
	QRCodeScanHw_ActivateTrigger(rc);//����Ӳ��ɨ��
	length = QRCodeScanHw_RxDeocdedData(rc, timeout_ms);
	QRCodeScanHw_DeactivateTrigger(rc);//�ر�Ӳ��ɨ��
	QRCODESCAN_DELAY_MS(10);
	return length;
}
/*---------------------------------------------------------------------*\
|	@brief	�������ɨ������			|
|	@param	��			|
|	@retval  length:��ȡ�ַ�������
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
	QRCodeScanHw_TxMsg(rc); //�������ɨ��
	length = QRCodeScanHw_RxDeocdedData(rc, timeout_ms);//��ȡ��������ֵ
	for(i=0;i<3;i++)
		rc->tx_msg[i]=scan_TxMsg1[i];
	rc->msg_length=i;
	QRCodeScanHw_TxMsg(rc); //�ر����ɨ��
	return length;
}

/*---------------------------------------------------------------------*\
|	@brief	��������ʶ��ʹ��		|
|	@param	* N4313:
			 codeType:�������Ͳ���	
			 enable:  0  �ر�
			 		1 ����				|
|	@retval  COMM_SUCCESSED: ���óɹ�
			COMM_FAILED:  ����ʧ��
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
		QRCodeScanHw_CloseUPCA_TO_EAN13(rc);//�ر�UPC_A�����Զ�ת����EAN13
	}
	for(i=0;i<6;i++)
	{
		rc->tx_msg[i+3]=barcodetype[barcode_index][i];//��������������ָ��浽N4313���ͻ�����
	}
	if(enable)
		rc->tx_msg[i+3]='1';//��������
	else
		rc->tx_msg[i+3]='0';//�ر�����
	i++;
	rc->tx_msg[i+3]=0x21;//���浽ɨ����ram�У����綪ʧ
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
	rc->scan_device_type=QRCodeScanHw_ReadProductid(rc);//��ȡɨ��������
	if(rc->scan_device_type==n4313_scan)//�����4313��ָ������������ر���������ʶ�𣬿���CODE 128����ʶ��
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
	if(qrcode_scan->qrcode_scan_conf.tirgger_mode == TRIGGER_HW)//���ݲ������ã��ж������������Ӳ������
	{
		QRCodeScanHw_ActivateTrigger(rc);//����Ӳ��ɨ��
	}
	else
	{
		int8_t i;
		uint8_t scan_TxMsg[3] = {0x16, 0x54, 0x0d};	
		for(i = 0; i < 3; i++)
			rc->tx_msg[i] = scan_TxMsg[i];
		rc->msg_length = i;
		QRCodeScanHw_TxMsg(rc); //�������ɨ��
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
		QRCodeScanHw_DeactivateTrigger(rc);//�ر�Ӳ��ɨ��
		QRCODESCAN_DELAY_MS(10);
	}
	else
	{
		int8_t i;
		uint8_t scan_TxMsg1[3] = {0x16, 0x55, 0x0d};
		for(i = 0; i < 3; i++)
			rc->tx_msg[i] = scan_TxMsg1[i];
		rc->msg_length = i;
		QRCodeScanHw_TxMsg(rc); //�ر����ɨ��
	}
	if(length)//���ɨ��ɹ�����N4313�������������IDת����SE955���ݵ�ID��
	{
#if (!DISABLE_ALL_PREFIX_SUFFIX)
		switch(rc->rx_msg[0])//����Τ�����������һ�ֽ�Ϊ��������ID
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
		for(i = 0; i < length; i++)		//���ú�׺0x0D�ض�����������ά��
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
	if(qrcode_scan->qrcode_scan_conf.tirgger_mode == TRIGGER_HW)//���ݲ������ã��ж������������Ӳ������
		length=QRCodeScanHw_HWDecodeData(rc, timeout_ms);
	else
		length= QRCodeScanHw_SSIDecodeData(rc, timeout_ms);
	if(length)//���ɨ��ɹ�����N4313�������������IDת����SE955���ݵ�ID��
	{
#if (!DISABLE_ALL_PREFIX_SUFFIX)
		switch(rc->rx_msg[0])//����Τ�����������һ�ֽ�Ϊ��������ID
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
		for(i = 0; i < length; i++)		//���ú�׺0x0D�ض�����������ά��
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


