
#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <i2c_EE.h>
#include <main.h>
#include "console.h"

uint16_t EepromType = AT24C256;

/********************************************************************/
/********************************************************************/
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	
	GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(IIC_SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ;   
	GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);

	Set_IIC_SCL;
	Set_IIC_SDA;

}

void IIC_Start(void)
{
	SDA_OUT();

	Set_IIC_SDA;
	delay_us(2);//4//
	Set_IIC_SCL;
	delay_us(2);
	Clr_IIC_SDA;	 
	delay_us(2);
}	  
void IIC_Stop(void)
{
	SDA_OUT();

	Clr_IIC_SDA;	 
	delay_us(2); //4/  
	Set_IIC_SCL;
	delay_us(2); 
	Set_IIC_SDA;	 
	delay_us(2);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
unsigned char IIC_Wait_Ack(void)
{
	unsigned char ucErrTime=0;
	SDA_IN();      	//SDA����Ϊ����  
	
	Set_IIC_SCL;
	delay_us(2);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}  
	delay_us(4); //--4
	Clr_IIC_SCL;	
	delay_us(2); 
	SDA_OUT();	   
	return 0;  
} 

void IIC_Ack(void)
{
	Clr_IIC_SCL;
	SDA_OUT();
	Clr_IIC_SDA;
	delay_us(2);
	Set_IIC_SCL;
	delay_us(4);//-4
	Clr_IIC_SCL;
}
void IIC_NAck(void)
{
	Clr_IIC_SCL;
	SDA_OUT();
	Set_IIC_SDA;
	delay_us(2);
	Set_IIC_SCL;
	delay_us(4);//--4
	Clr_IIC_SCL;
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(unsigned char txd)
{                        
	unsigned char t;   
	SDA_OUT();
	
	for(t=0;t<8;t++)
	{  
		Clr_IIC_SCL;		//����ʱ�ӿ�ʼ���ݴ���
		delay_us(2);     //
		if ((txd&0x80)>>7) 	
		{
			Set_IIC_SDA;
		}	
		else
		{
			Clr_IIC_SDA;
		} 
		txd<<=1; 	  
		delay_us(2); 	//
		Set_IIC_SCL;
		delay_us(4);  //--4
	}
	Clr_IIC_SCL;	
	delay_us(4); ///--4
	Set_IIC_SDA;	
	delay_us(2); 
} 

//��1���ֽڣ����ض�����һ���ֽ�   
unsigned char IIC_Read_Byte(uint8_t ack)
{
	unsigned char i,receive=0;
	Clr_IIC_SCL;
	delay_us(2);
	Set_IIC_SDA;
	delay_us(2);
	SDA_IN();	//SDA����Ϊ����
	for(i=0;i<8;i++ )
	{
		Set_IIC_SCL;
		delay_us(3);// --3
		receive<<=1;
		if(READ_SDA)receive++; 
		Clr_IIC_SCL;  
		delay_us(2); //--2
	}					 
	SDA_OUT(); 
	return receive;
}



/********************************************************************/
/********************************************************************/

void AT24CXX_Init(void)
{

	IIC_Init();
}

uint8_t AT24CXX_ReadOneByte(uint32_t ReadAddr)
{				  
    uint8_t addH,addL,addP0;
    uint8_t tmp = 0;

    if(EepromType == 0)
        return tmp;
	    
    addH=(ReadAddr&0xFF00)>>8; 
    addL=(ReadAddr&0x00FF);
	
	addP0 = 0xA0 |((ReadAddr / 0x10000) << 1);
	
    IIC_Start();
    if(EepromType > AT24C16)
    {

		if(addP0>0xA0)
		{
			IIC_Send_Byte(addP0);    //write operation
        	IIC_Wait_Ack();
		}
		else
		{
			IIC_Send_Byte(0xA0);    //write operation
        	IIC_Wait_Ack();
		}
        IIC_Send_Byte(addH);    //high address
        IIC_Wait_Ack();
    }
    else
    {
        addH = 0xA0+(addH<<1);  
        
        IIC_Send_Byte(addH);    //high address
        IIC_Wait_Ack();
    }
    
	IIC_Send_Byte(addL);  	//low address
	IIC_Wait_Ack();
		
	IIC_Start();
	
	if(addP0>0xA1)
	IIC_Send_Byte(addP0+1);	//read	 operation
	else
	IIC_Send_Byte(0xA1);	//read	 operation	*/
	IIC_Wait_Ack();

	tmp =IIC_Read_Byte(0); 	//read a byte
	IIC_NAck();
	IIC_Stop();
	return tmp;

}


void AT24CXX_WriteOneByte(uint32_t WriteAddr,uint8_t DataToWrite)
{				   	  	    																 
	
    uint8_t addH,addL,addP0;
    if(EepromType == 0)
        return;
    
    addH=(WriteAddr&0xFF00)>>8; 
    addL=(WriteAddr&0x00FF);
	//addP0 = 0xA0|((ReadAddr & 0xf0000) >>16);
	addP0 = 0xA0 |((WriteAddr / 0x10000) << 1);
	
    IIC_Start();
    if(EepromType > AT24C16)
    {
		if(addP0 > 0xa0)
		{
			IIC_Send_Byte(addP0);    //write operation
        	IIC_Wait_Ack();
		}
		else
		{
			IIC_Send_Byte(0xA0);    //write operation
        	IIC_Wait_Ack();
		}
        
        IIC_Send_Byte(addH);    //high address
        IIC_Wait_Ack();
    }
    else
    {
        addH = 0xA0+(addH<<1);  
        
        IIC_Send_Byte(addH);    //high address
        IIC_Wait_Ack();
    }
	IIC_Send_Byte(addL);	//low address
	IIC_Wait_Ack();	
	
	IIC_Send_Byte(DataToWrite);
	IIC_Wait_Ack();
		 
	IIC_Stop();
	delay_ms(5); 	

}



void AT24CXX_WriteLenByte(uint32_t WriteAddr,uint32_t DataToWrite,uint8_t Len)
{  	
	uint8_t t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
}
uint32_t AT24CXX_ReadLenByte(uint32_t ReadAddr,uint8_t Len)
{  	
	uint8_t t;
	uint32_t temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 	 				   
	}
	return temp;												    
}


uint8_t AT24CXX_Check(void)
{
	uint8_t temp;
	temp=AT24CXX_ReadOneByte(255);//����ÿ�ο�����дAT24CXX			   
	if(temp==0X55)return 0;		   
	else//�ų���һ�γ�ʼ�������
	{
		AT24CXX_WriteOneByte(255,0x55);
	    temp=AT24CXX_ReadOneByte(255);	  
		if(temp==0X55)return 0;
	}
	return 1;											  
}
uint8_t AT24CXX_WritePage(uint32_t WriteAddr,uint8_t *pWrite,uint16_t DataToWrite)
{

    uint8_t addH,addL,addP0;
	uint16_t iLoop=0;
    
    addH=(WriteAddr&0xFF00)>>8; 
    addL=(WriteAddr&0x00FF);
	
	addP0 = 0xA0 |((WriteAddr / 0x10000) << 1);

	//printf("WriteAddr = %08x\r\n",WriteAddr);
	//printf("AddP0 = %02x\r\n",addP0);

    IIC_Start();

    if(EepromType > AT24C16)
    {
		if(addP0 > 0xa0)
		{
			IIC_Send_Byte(addP0);    //write operation
        	IIC_Wait_Ack();
		}
		else
		{
			IIC_Send_Byte(0xA0);    //write operation
        	IIC_Wait_Ack();
		}
        
        IIC_Send_Byte(addH);    //high address
        IIC_Wait_Ack();
    }
    else
    {
        addH = 0xA0+(addH<<1);  
        
        IIC_Send_Byte(addH);    //high address
        IIC_Wait_Ack();
    }
	IIC_Send_Byte(addL);	//low address
	IIC_Wait_Ack();	

	for(iLoop=0;iLoop<DataToWrite;iLoop++){
		IIC_Send_Byte(*(pWrite+iLoop));
		IIC_Wait_Ack();
	} 
	
	IIC_Stop();
	delay_ms(10); 	//д�����ʱ��������10ms	
	
	return 0;
}



/********************************************************************************/
/********************************************************************************/
uint8_t Eeprom_ReadBuffer(uint8_t * pBuffer, uint32_t ReadAddr, uint16_t NumToRead)
{

  	uint8_t addH,addL,addP0;
	uint16_t iLoop=0;
    
  	addH=(ReadAddr&0xFF00)>>8; 
  	addL=(ReadAddr&0x00FF);
	addP0 = 0xA0 |((ReadAddr / 0x10000) << 1);

	//printf("ReadAddr = %08x\r\n",ReadAddr);
	//printf("AddP0 = %02x\r\n",addP0);
	

	IIC_Start();
	
    if(EepromType > AT24C16)
    {
		if(addP0 > 0xa0)
		{
			IIC_Send_Byte(addP0);    //write operation
        	IIC_Wait_Ack();
		}
		else
		{
			IIC_Send_Byte(0xA0);    //write operation
        	IIC_Wait_Ack();
		}
		
        IIC_Send_Byte(addH);    //high address
        IIC_Wait_Ack();
    }
    else
    {
        addH = 0xA0+(addH<<1);  
        
        IIC_Send_Byte(addH);    //high address
        IIC_Wait_Ack();
    }
	
	IIC_Send_Byte(addL);	//low address
	IIC_Wait_Ack();	

	IIC_Start();
	//IIC_Send_Byte(0xA1);	//read	 operation

	if(addP0>0xA1)
	IIC_Send_Byte(addP0+1);	//read	 operation
	else
	IIC_Send_Byte(0xA1);	//read	 operation	*/
	
	IIC_Wait_Ack();
	for(iLoop=0;iLoop<NumToRead;iLoop++){
		*(pBuffer+iLoop) =IIC_Read_Byte(1); //read a byte
		if(iLoop+1<NumToRead)
			IIC_Ack();
	} 
	IIC_NAck();
	IIC_Stop();

	delay_ms(10);

	return 1;

}  

/********************************************************************************/
uint8_t Eeprom_WriteBuffer(uint8_t * pWrite, uint32_t WriteAddr, uint16_t DataToWrite)
{  	
	
	uint16_t i=0;
	uint16_t space=0;	//�����ַ�ڵ�ǰҳ��ʣ�µĿռ�
	uint16_t step=0; 	// n byte ������Ҫռ�õ�ҳ��
	uint16_t increment=0;	// ��ַ����
	uint16_t page_size = 0;
	
	//const uint16_t page_size =256;	//ע��EEPROMоƬ��������С!!

	if(EepromType > AT24C16)
	page_size =256;
	else
 	page_size =32;	
	
	space=page_size-WriteAddr%page_size;
	step=DataToWrite/page_size;

	if(step==0)
	{

		if(DataToWrite<space){//��ҳ����д��
			AT24CXX_WritePage(WriteAddr,pWrite,DataToWrite);		//��дspace ��byte
		}
		else{//��ҳ���Կռ䲻��
			AT24CXX_WritePage(WriteAddr,pWrite,space);		//��дspace ��byte
			if(space<DataToWrite)
			{
				increment=space;//DataToWrite-
				AT24CXX_WritePage(WriteAddr+increment,(pWrite+increment),DataToWrite-space);
			}
		}
	}

	if(step>0)
	{
		
		AT24CXX_WritePage(WriteAddr,pWrite,space);		//��дspace ��byte
		increment=space;
		for(i=0;i<step;i++)
		{
			if((DataToWrite-space-page_size*i)<page_size)	  			//������һҳ������һҳδд��
			{

				AT24CXX_WritePage(WriteAddr+increment,(pWrite+increment),(DataToWrite-space-page_size*i));
			}
			else   			//������һҳ������һҳд��
			{

				AT24CXX_WritePage(WriteAddr+increment,(pWrite+increment),page_size);
				increment=increment+page_size ;
			}
			
		}
		if((DataToWrite-space-page_size*step)>0)	 		//ǰһҳд��������ǰҳûд��
		{
		
			AT24CXX_WritePage(WriteAddr+increment,(pWrite+increment),(DataToWrite-space-page_size*step));
		} 
	}

	return 1;
}


/********************************************************************************/
/********************************************************************************/











