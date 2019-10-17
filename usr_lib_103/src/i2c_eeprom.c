/*
  Filename: i2c_eeprom.c
  Author: shidawei			Date:
  Copyright: 中电长城(长沙)信息技术有限公司
  Description: STM32F10X EEPROM芯片AT24C16
               驱动程序Lib
*/
#include "i2c_eeprom.h"

/****************************************************************
 LOCAL FUNCTIONS
 ****************************************************************/
/**
  * @brief  eeprom页写
  * @param  p_i2c_eeprom: eeprom结构体指针
  *         buf: 写入的数据
  *         addr: 写入地址
  *         size: 写入数据大小
  * @retval 成功返回1，失败返回0
  */
static uint8_t I2cEeprom_WritePage(t_i2c_eeprom* p_i2c_eeprom, uint8_t* buf, uint16_t addr, uint8_t size)
{
    uint8_t tmp_address, ret_val = 0;
	uint32_t timeout = I2C_EEPROM_TIMEOUT;
    //I2C_AcknowledgeConfig(p_i2c_eeprom->I2Cx, ENABLE);
    if(addr >= p_i2c_eeprom->MemorySize) //地址越界
	{
		ret_val = 0;
		return ret_val;
	}
    tmp_address = (addr >> 8);
    tmp_address = tmp_address & 0x07;
    tmp_address = tmp_address << 1;
    tmp_address = (p_i2c_eeprom->I2C_Address) | tmp_address;
    ////////////////////////
    timeout = I2C_EEPROM_TIMEOUT;
    while(I2C_GetFlagStatus(p_i2c_eeprom->I2Cx, I2C_FLAG_BUSY)) // Added by Najoua 27/08/2008
    {
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* Send START condition */
    I2C_GenerateSTART(p_i2c_eeprom->I2Cx, ENABLE);

    /* Test on EV5 and clear it */
	timeout = I2C_EEPROM_TIMEOUT;
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(p_i2c_eeprom->I2Cx, tmp_address, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
	timeout = I2C_EEPROM_TIMEOUT;
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* Send the EEPROM's internal address to write to */
    I2C_SendData(p_i2c_eeprom->I2Cx, (uint8_t)(addr & 0x00FF));

    /* Test on EV8 and clear it */
	timeout = I2C_EEPROM_TIMEOUT;
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* While there is data to be written */
    while(size--)
    {
        /* Send the current byte */
        I2C_SendData(p_i2c_eeprom->I2Cx, *buf);

        /* Point to the next byte to be written */
        buf++;

        /* Test on EV8 and clear it */
        while (!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }

    /* Send STOP condition */
    I2C_GenerateSTOP(p_i2c_eeprom->I2Cx, ENABLE);
    // I2C_AcknowledgeConfig(p_i2c_eeprom->I2Cx, DISABLE);
    ret_val = 1;

	return ret_val;
}

/**
  * @brief  Wait for EEPROM Standby state
  * @param  
  * @retval 
  */
static void I2cEeprom_WaitStandbyState(t_i2c_eeprom * p_i2c_eeprom)
{
    __IO uint16_t SR1_Tmp = 1;

    do
    {
        /* Send START condition */
        I2C_GenerateSTART(p_i2c_eeprom->I2Cx, ENABLE);
        /* Read p_i2c_eeprom->I2Cx SR1 register */
        SR1_Tmp = I2C_ReadRegister(p_i2c_eeprom->I2Cx, I2C_Register_SR1);
        /* Send EEPROM address for write */
        I2C_Send7bitAddress(p_i2c_eeprom->I2Cx, p_i2c_eeprom->I2C_Address, I2C_Direction_Transmitter);
    }
    while(!(I2C_ReadRegister(p_i2c_eeprom->I2Cx, I2C_Register_SR1) & 0x0002));

    /* Clear AF flag */
    I2C_ClearFlag(p_i2c_eeprom->I2Cx, I2C_FLAG_AF);
    /* STOP condition */
    I2C_GenerateSTOP(p_i2c_eeprom->I2Cx, ENABLE);
}


/****************************************************************
 GLOBAL FUNCTIONS
 ****************************************************************/
/**
  * @brief  初始化I2C EEPROM
  * @param  p_i2c_eeprom: eeprom结构体指针
  * @retval 
  */
void I2cEeprom_Init(t_i2c_eeprom * p_i2c_eeprom)
{
    uint16_t i = 0;

    //rcc
    RCC_APB1PeriphClockCmd(p_i2c_eeprom->I2C_RccClk, ENABLE);
    //PIN
    Pin_Init(p_i2c_eeprom->pin_scl);
    Pin_Init(p_i2c_eeprom->pin_sda);
    I2C_Init(p_i2c_eeprom->I2Cx, p_i2c_eeprom->I2C_InitStruct);

    while(I2C_GetFlagStatus(p_i2c_eeprom->I2Cx, I2C_FLAG_BUSY)); // Added by Najoua 27/08/2008

    /* Send START condition */
    I2C_GenerateSTART(p_i2c_eeprom->I2Cx, ENABLE);

    /* Test on EV5 and clear it */
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(p_i2c_eeprom->I2Cx, p_i2c_eeprom->I2C_Address, I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && (i++<0xFF00));
    if(i < 0xFF00)
    {
        p_i2c_eeprom->on = 1;
    }
    else
    {
        p_i2c_eeprom->on = 0;
    }
    I2C_GenerateSTOP(p_i2c_eeprom->I2Cx, ENABLE);
}

/**
  * @brief  从EEPROM里面读取一块数据
  * @param  p_i2c_eeprom: 指向EEPROM对象
  *         pBuffer: 存放从EEPROM读取的数据的缓冲区指针
  *         ReadAddr: 接收数据的EEPROM的地址
  *         size: 要从EEPROM读取的字节数
  * @retval 成功返回1，失败返回0
  */
uint8_t I2cEeprom_ReadBuffer(t_i2c_eeprom * p_i2c_eeprom, uint8_t* pBuffer, uint16_t ReadAddr, uint16_t size)
{
    uint8_t tmp_address;
	uint32_t timeout = I2C_EEPROM_TIMEOUT;
	uint8_t ret_val = 0;
    if(p_i2c_eeprom->on == 0)
    {
		ret_val = 0;
		return ret_val;
    }
    I2C_AcknowledgeConfig(p_i2c_eeprom->I2Cx, ENABLE);
    if(ReadAddr>=p_i2c_eeprom->MemorySize) //地址越界
	{
		ret_val = 0;
		return ret_val;
	}

	tmp_address=(ReadAddr>>8);
    tmp_address=tmp_address &0x07;
    tmp_address=tmp_address<<1;
    tmp_address=(p_i2c_eeprom->I2C_Address)|tmp_address;

    ////////////////////////////////////////////////
	timeout = I2C_EEPROM_TIMEOUT;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) // Added by Najoua 27/08/2008
    {
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }


    /* Send START condition */
    I2C_GenerateSTART(I2C1, ENABLE);
    //*((u8 *)0x4001080c) &=~0x80;

    /* Test on EV5 and clear it */
	timeout = I2C_EEPROM_TIMEOUT;
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* Send EEPROM address for write */
    I2C_Send7bitAddress(p_i2c_eeprom->I2Cx,tmp_address , I2C_Direction_Transmitter);

    /* Test on EV6 and clear it */
	timeout = I2C_EEPROM_TIMEOUT;
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	{
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* Clear EV6 by setting again the PE bit */
    I2C_Cmd(p_i2c_eeprom->I2Cx, ENABLE);


    ///////////////////////////////////////////////////


    I2C_SendData(p_i2c_eeprom->I2Cx, (uint8_t)(ReadAddr & 0x00FF));

    /* Test on EV8 and clear it */
	timeout = I2C_EEPROM_TIMEOUT;
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* Send STRAT condition a second time */
    I2C_GenerateSTART(p_i2c_eeprom->I2Cx, ENABLE);

    /* Test on EV5 and clear it */
	timeout = I2C_EEPROM_TIMEOUT;
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* Send EEPROM address for read */
    I2C_Send7bitAddress(p_i2c_eeprom->I2Cx, p_i2c_eeprom->I2C_Address, I2C_Direction_Receiver);

    /* Test on EV6 and clear it */
	timeout = I2C_EEPROM_TIMEOUT;
    while(!I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
		timeout--;
		if(!timeout)
		{
			ret_val = 0;
			return ret_val;
		}
    }

    /* While there is data to be read */
    while(size)
    {
        if(size == 1)
        {
            /* Disable Acknowledgement */
            I2C_AcknowledgeConfig(p_i2c_eeprom->I2Cx, DISABLE);

            /* Send STOP Condition */
            I2C_GenerateSTOP(p_i2c_eeprom->I2Cx, ENABLE);
        }

        /* Test on EV7 and clear it */
        if(I2C_CheckEvent(p_i2c_eeprom->I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            /* Read a byte from the EEPROM */
            *pBuffer = I2C_ReceiveData(p_i2c_eeprom->I2Cx);

            /* Point to the next location where the byte read will be saved */
            pBuffer++;

            /* Decrement the read bytes counter */
            size--;
        }
    }

    /* I2Cx Acknowledgement to be ready for another reception */
//	I2C_AcknowledgeConfig(p_i2c_eeprom->I2Cx, ENABLE);
	ret_val = 1;
	return ret_val;

}

/**
  * @brief  I2C EEPROM写数据
  * @param  p_i2c_eeprom: 指向EEPROM对象
  *         pBuffer: 需要写入的缓冲区指针
  *         WriteAddr: 写入到地址
  *         size: 写入数据长度
  * @retval 成功返回1，失败返回0
  */
uint8_t I2cEeprom_WriteBuffer(t_i2c_eeprom * p_i2c_eeprom, uint8_t* pBuffer, uint16_t WriteAddr, uint16_t size)
{
    uint16_t addr;  //当前写入地址
    uint8_t page;  //需要多少个完整页
    uint8_t head; //开头非页对齐部分有多少个字节
    uint8_t rear;  //结尾非页对齐部分由多少个字节
    uint8_t i;
    if(p_i2c_eeprom->on == 0)
    {
        return 0;
    }
    I2C_AcknowledgeConfig(p_i2c_eeprom->I2Cx, ENABLE);
    if(WriteAddr>=p_i2c_eeprom->MemorySize)//地址越界
    {
        return 0;
    }
    addr= WriteAddr%(p_i2c_eeprom->PageSize);	 //计数写入操作从页中的哪个字节开始 为0时则为页对齐
    head=(p_i2c_eeprom->PageSize)-addr;	     //第一页最多可写入多少个字节。
    if(size<=head)                        //如果第一页就能写完所有数据
    {
        head=size;
        page=0;
        rear=0;
    }
    else	 //如果需要跨页则计算共需要多少个完整页，以及最后一页需要写多少个字节。
    {
        page=(size-head)/(p_i2c_eeprom->PageSize);
        rear=(size-head)%(p_i2c_eeprom->PageSize);
    }
    addr=WriteAddr;

    //写第一页的数据
    if(head)   //加这个判断是防止NUM传入0的情况。
    {
        I2cEeprom_WritePage(p_i2c_eeprom,pBuffer,addr,head);
        I2cEeprom_WaitStandbyState(p_i2c_eeprom);
        addr+=head;   //更新下次写入地址
        pBuffer+=head;	//更新下次写入的起始数据
    }

    //中间的整页写入
    for(i=0; i<page; i++)
    {
        I2cEeprom_WritePage(p_i2c_eeprom,pBuffer,addr,head);
        I2cEeprom_WaitStandbyState(p_i2c_eeprom);
        addr+=p_i2c_eeprom->PageSize;	//更新下次写入地址
        pBuffer+=p_i2c_eeprom->PageSize; //更新下次写入的起始数据
    }
    //结尾的剩余字节写入。
    if(rear)
    {
        I2cEeprom_WritePage(p_i2c_eeprom,pBuffer,addr,rear);
        I2cEeprom_WaitStandbyState(p_i2c_eeprom);
    }
    return 1;
}



