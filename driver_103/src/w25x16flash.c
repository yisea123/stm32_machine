#include "w25x16flash.h"
#include "main.h"

uint8_t drv_w25x16_ReadWriteByte(t_W25x16_soursce* w25x16,uint8_t data )
{
	uint8_t ch;
	while(SPI_I2S_GetFlagStatus(w25x16->SPIX,SPI_I2S_FLAG_TXE)==RESET);
		SPI_I2S_SendData(w25x16->SPIX,data);
	while(SPI_I2S_GetFlagStatus(w25x16->SPIX,SPI_I2S_FLAG_RXNE)==RESET); 
		ch=SPI_I2S_ReceiveData(w25x16->SPIX); 	
	return ch;
}


void drv_w25x16_Release_PowerDown(t_W25x16_soursce* w25x16)
{
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadWriteByte(w25x16,w25x_PowerDown);//发送数据
	Pin_WriteBit(w25x16->CS,Bit_SET);
	SPI_Cmd(w25x16->SPIX,DISABLE);	
}
void drv_w25x16_init(t_W25x16_soursce* w25x16)
{
	Pin_Init(w25x16->CS);
	Pin_Init(w25x16->MISO);
	Pin_Init(w25x16->MOSI);
	Pin_Init(w25x16->SCK);
	SPI_Init(w25x16->SPIX,w25x16->SPI_InitStruct);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadID(w25x16);
}




uint8_t drv_w25x16_GetStatus(t_W25x16_soursce* w25x16)
{
	uint8_t status;
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadWriteByte(w25x16,W25x_ReadStatus);//发送数据
	status=drv_w25x16_ReadWriteByte(w25x16,0xff);//接收字节
	Pin_WriteBit(w25x16->CS,Bit_SET);
	SPI_Cmd(w25x16->SPIX,DISABLE);	
	return status;
}


uint8_t drv_w25x16_IsBusy(t_W25x16_soursce* w25x16)
{
	uint8_t rs;
	rs=drv_w25x16_GetStatus(w25x16);
	if(rs&0x01)
		return TRUE;//忙
	else
		return FALSE;
}
uint16_t drv_w25x16_ReadID(t_W25x16_soursce* w25x16)
{
	uint16_t rs=0;
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadWriteByte(w25x16,W25x_ReadID);//发送数据
	drv_w25x16_ReadWriteByte(w25x16,0);//发送数据
	drv_w25x16_ReadWriteByte(w25x16,0);//发送数据
	drv_w25x16_ReadWriteByte(w25x16,0);//发送数据	
	rs=drv_w25x16_ReadWriteByte(w25x16,0xff);//接收字节
	rs=(rs<<8)|drv_w25x16_ReadWriteByte(w25x16,0xff);//接收字节
	Pin_WriteBit(w25x16->CS,Bit_SET);
	return rs;	
}
uint8_t drv_w25x16_Erase_Chip(t_W25x16_soursce* w25x16)
{
	uint8_t rs;
	uint16_t timeout;
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadWriteByte(w25x16,W25x_FullErase);//发送数据
	Pin_WriteBit(w25x16->CS,Bit_SET);
	SPI_Cmd(w25x16->SPIX,DISABLE);
	while(1)
	{
		delay_ms(100);
		timeout++;
		if(drv_w25x16_IsBusy(w25x16))//整片擦除25S
		{
			if(timeout>300)
			{
				rs=FALSE;
				break;
			}
		}
		else
		{
			rs=TRUE;
			break;
		}		
	}
	return rs;
}

void drv_w25x16_Write_Enable(t_W25x16_soursce* w25x16)
{
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadWriteByte(w25x16,W25x_WriteEnable);//发送数据
	Pin_WriteBit(w25x16->CS,Bit_SET);
	SPI_Cmd(w25x16->SPIX,DISABLE);		
}
void drv_w23x16_Write_Disable(t_W25x16_soursce* w25x16)
{
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadWriteByte(w25x16,W25x_WriteDisable);//发送数据
	Pin_WriteBit(w25x16->CS,Bit_SET);
	SPI_Cmd(w25x16->SPIX,DISABLE);		
}

//擦除一个扇区
//Addr:扇区地址 0~511 for w25x16
//擦除一个山区的最少时间:150ms

uint8_t drv_w25x16_Erase_Sector(t_W25x16_soursce* w25x16,uint32_t Addr)
{
	uint8_t rs,timeout;
	Addr*=4096;
	drv_w25x16_Write_Enable(w25x16);
	while(drv_w25x16_IsBusy(w25x16));
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);	
	drv_w25x16_ReadWriteByte(w25x16,W25x_SectorErase);//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)(Addr>>16));//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)(Addr>>8));//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)Addr);//发送数据
	Pin_WriteBit(w25x16->CS,Bit_SET);
	while(1)
	{
		delay_ms(10);
		timeout++;
		if(drv_w25x16_IsBusy(w25x16))
		{
			if(timeout>100)
			{
				rs=FALSE;
				break;
			}
		}
		else
		{
			rs=TRUE;
			break;
		}		
	}
	return rs;		
}


//擦除一个块区
//Addr:块区地址 0~32 for w25x16
//擦除一个块区的最少时间:10s

uint8_t drv_w25x16_Erase_Block(t_W25x16_soursce* w25x16,uint32_t Addr)
{
	uint8_t rs,timeout;
	Addr*=65536;
	drv_w25x16_Write_Enable(w25x16);
	while(drv_w25x16_IsBusy(w25x16));
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);	
	drv_w25x16_ReadWriteByte(w25x16,W25x_BlockErase);//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)(Addr>>16));//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)(Addr>>8));//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)Addr);//发送数据
	Pin_WriteBit(w25x16->CS,Bit_SET);
	while(1)
	{
		delay_ms(10);
		timeout++;
		if(drv_w25x16_IsBusy(w25x16))
		{
			if(timeout>200)
			{
				rs=FALSE;
				break;
			}
		}
		else
		{
			rs=TRUE;
			break;
		}		
	}
	return rs;	
}



void drv_w25x16_WritePage(t_W25x16_soursce* w25x16,uint32_t Addr,uint32_t Numbyte)
{
	uint16_t i;
	drv_w25x16_Write_Enable(w25x16);
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadWriteByte(w25x16,W25x_PageProgram);//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)(Addr>>16));//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)(Addr>>8));//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)Addr);//发送数据
	for(i=0;i<Numbyte;i++)
		drv_w25x16_ReadWriteByte(w25x16,w25x16->pBuffer[i]);
	Pin_WriteBit(w25x16->CS,Bit_SET);
	while(drv_w25x16_IsBusy(w25x16));//等待空闲
}

void drv_w25x16_Write_NoCheck(t_W25x16_soursce* w25x16,uint32_t Addr,uint32_t Numbyte)
{
	uint16_t pageremain;	   
	pageremain=256-Addr%256; //单页剩余的字节数
	if(Numbyte<pageremain)
		pageremain=Numbyte;
	while(1)
	{
		drv_w25x16_WritePage(w25x16,Addr,pageremain);
		if(pageremain==Numbyte)
			break;
		else
		{
			w25x16->pBuffer+=pageremain;
			Addr+=pageremain;
			Numbyte-=pageremain;
			if(Numbyte>256)
				pageremain=256;
			else
				pageremain=Numbyte;
		}
	}
}


void drv_w25x16_Read(t_W25x16_soursce* w25x16,uint32_t Addr,uint32_t Numbyte)
{
	uint16_t i;
	Pin_WriteBit(w25x16->CS,Bit_RESET);
	SPI_Cmd(w25x16->SPIX,ENABLE);
	drv_w25x16_ReadWriteByte(w25x16,W25x_ReadEnable);//发送数据
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)(Addr>>16));
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)(Addr>>8));
	drv_w25x16_ReadWriteByte(w25x16,(uint8_t)Addr);
	for(i=0;i<Numbyte;i++)
		w25x16->pBuffer[i]=drv_w25x16_ReadWriteByte(w25x16,0xff);
	Pin_WriteBit(w25x16->CS,Bit_SET);
}
void drv_w25x16_Write(t_W25x16_soursce* w25x16,uint32_t Addr,uint32_t Numbyte)
{
	u32 secpos;
	u16 secoff;
	u16 secremain;	     
	secpos=Addr/4096;//扇区地址 0~511 for w25x16
	secoff=Addr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小
	if(Numbyte<=secremain)
		secremain=Numbyte;//不大于4096个字节
	while(1)
	{
		drv_w25x16_Write_NoCheck(w25x16,Addr,secremain);
		if(Numbyte==secremain)
			break;//写入结束了
		else
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	
			w25x16->pBuffer+=secremain;
			Addr+=secremain;//写地址偏移	   
			Numbyte-=secremain;				//字节数递减
			if(Numbyte>4096)
				secremain=4096;	//下一个扇区还是写不完
			else 
				secremain=Numbyte;			//下一个扇区可以写完了	
		}
	}
}

