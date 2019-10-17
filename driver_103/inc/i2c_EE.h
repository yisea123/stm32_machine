#ifndef __I2C_EE_H
#define __I2C_EE_H
#include "settings.h" 


#define AT24C16		2*1024-1	//0x7FF
#define AT24C256	32767 
#define AT24C2048	256*1024-1  //0X40000

#define EE_PAGE_SIZE 256

#define CHECKCHAR 0xAA
#define CHECKEEPROMSIZE 2035


#define IIC_SCL_PORT            GPIOB
#define IIC_SCL_CLK             RCC_APB2Periph_GPIOB
#define IIC_SCL_PIN             GPIO_Pin_10

#define IIC_SDA_PORT            GPIOB
#define IIC_SDA_CLK             RCC_APB2Periph_GPIOB
#define IIC_SDA_PIN             GPIO_Pin_11


#define SDA_IN()  {GPIOB->CRH&=0XFFFF8FFF;GPIOB->CRH|=8<<12;}     
#define SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=3<<12;}


//IO操作函数	 
#define Set_IIC_SCL  {GPIO_SetBits(IIC_SCL_PORT,IIC_SCL_PIN);}
#define Clr_IIC_SCL  {GPIO_ResetBits(IIC_SCL_PORT,IIC_SCL_PIN);} 
#define Set_IIC_SDA  {GPIO_SetBits(IIC_SDA_PORT,IIC_SDA_PIN);}
#define Clr_IIC_SDA  {GPIO_ResetBits(IIC_SDA_PORT,IIC_SDA_PIN);} 
#define READ_SDA    (GPIO_ReadInputDataBit(IIC_SDA_PORT, IIC_SDA_PIN))

//#define Set_IIC_WP  {GPIO_SetBits(IIC_WP_PORT,IIC_WP);}
//#define Clr_IIC_WP  {GPIO_ResetBits(IIC_WP_PORT,IIC_WP);} 
	 
/********************************************************************************/

//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);			//IIC发送一个字节
uint8_t IIC_Read_Byte(uint8_t ack);//IIC读取一个字节
uint8_t IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

//void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
//uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	  

/********************************************************************************/
/********************************************************************************/		  
uint8_t AT24CXX_ReadOneByte(uint32_t ReadAddr);							//指定地址读取一个字节
void AT24CXX_WriteOneByte(uint32_t WriteAddr,uint8_t DataToWrite);		//指定地址写入一个字节
void AT24CXX_WriteLenByte(uint32_t WriteAddr,uint32_t DataToWrite,uint8_t Len);//指定地址开始写入指定长度的数据
uint32_t AT24CXX_ReadLenByte(uint32_t ReadAddr,uint8_t Len);					//指定地址开始读取指定长度数据
//void AT24CXX_Write(uint16_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite);	//从指定地址开始写入指定长度的数据
void AT24CXX_Read(uint32_t ReadAddr,uint8_t *pBuffer,uint16_t NumToRead);   	//从指定地址开始读出指定长度的数据

uint8_t AT24CXX_Check(void);  //检查器件
void AT24CXX_Init(void); //初始化IIC
void AT24CXX_Configuration(void);
uint16_t CheckEepromSize(void);

uint8_t AT24CXX_WritePage(uint32_t WriteAddr,uint8_t *pWrite,uint16_t DataToWrite);
void AT24CXX_Write(uint32_t WriteAddr,uint8_t *pWrite,uint16_t DataToWrite);

uint8_t Eeprom_ReadBuffer(uint8_t * pBuffer, uint32_t ReadAddr, uint16_t NumToRead);
uint8_t Eeprom_WriteBuffer(uint8_t * pWrite, uint32_t WriteAddr, uint16_t DataToWrite);



/********************************************************************************/
/********************************************************************************/



#endif
















