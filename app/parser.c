#include "s_def.h"
#include "parser.h"
#include "com.h"
#include "respcode.h"
#include "command.h"
#include "digtube.h"
#include "main.h"

#define   PARSER_SEND_BUF(buf, size)     Com_Send_Buffer(MAIN_COM1, buf, size)
#define   PARSER_GET_CH(ch)              Com_Receive_Ch(MAIN_COM1, ch)

t_cmd			m_cmd;						//从上位机接收的命令
t_rpy			m_rpy;						//返回给上位机的应答
unsigned char   CMD_CMP_Buf[MAX_CMP_LEN];	//命令&应答缓冲
unsigned char   RPY_RDT_Buf[MAX_RDT_LEN];	// 待发送的应答参数缓冲
unsigned char   RPY_Str[MAX_RPY_LEN];		//待发送的应答缓冲。
static unsigned char machine_is_busy = 0;	//设置是否正忙
static unsigned char busy_step = 0;			//设备忙时返回DLE NAK的标记步骤
static unsigned char pre_answer = 0;		//提前回应标记

static unsigned char CMD_Send_ACKOrNAK(unsigned char status);
static unsigned char CMD_BCC_Result(t_cmd  Command);
static unsigned char CMD_Receive_Head(void);
static unsigned char CMD_Receive_Data(void);
static unsigned char CMD_WaitENQ(void);
static void CMD_Reset(void);

void Parser_Init(void)
{
   //此处添加协议解析模块初始化代码
	m_cmd.idn = 0;
	m_cmd.cmd[0] = 0;
	m_cmd.cmd[1] = 0;
	m_cmd.cmd_len[0] = 0;
	m_cmd.cmd_len[1] = 0;
	m_cmd.len_cmp = 0;
	m_cmd.bufIndex = 0;
	m_cmd.p_cmp = CMD_CMP_Buf;
	
	m_rpy.jdg = 'P';
	m_rpy.rcm[0] = '0';
	m_rpy.rcm[1] = '0';
	m_rpy.res = 0;
	m_rpy.len_rdt = 0;
	m_rpy.vailable = 0;
	m_rpy.p_rdt = RPY_RDT_Buf;

	pre_answer = 0;
}

static void CMD_Reset(void)
{
	//此处添加复位命令接收和应答结构体的代码	
	m_cmd.idn = 0;
	m_cmd.cmd[0] = 0;
	m_cmd.cmd[1] = 0;
	m_cmd.cmd_len[0] = 0;
	m_cmd.cmd_len[1] = 0;
	m_cmd.len_cmp = 0;
	m_cmd.bufIndex = 0;
	m_cmd.p_cmp = CMD_CMP_Buf;
	
	m_rpy.vailable=0;
	m_rpy.jdg='P';
	m_rpy.rcm[0]='0';
	m_rpy.rcm[1]='0';
	m_rpy.res=0;
	m_rpy.len_rdt=0;
	m_rpy.p_rdt = RPY_RDT_Buf;
}

/*send ACK or send ENQ*/
static unsigned char CMD_Send_ACKOrNAK(unsigned char status)
{
	unsigned char ret = FALSE;
	char str_ACK[2] = {DLE, ACK};
	char str_NAK[2] = {DLE, NAK};

	if(status == TRUE)
	{
		PARSER_SEND_BUF(str_ACK, 2);
		ret = TRUE;
	}
	else
	{
		PARSER_SEND_BUF(str_NAK, 2);
		ret = FALSE;
	}
	return ret;
}

static unsigned char  CMD_BCC_Result(t_cmd Command)
{
	unsigned char checksum = 0;
	unsigned int i;

	checksum ^= DLE;
	checksum ^= STX;
	checksum ^= Command.idn;
	checksum ^= Command.cmd_len[0];
	checksum ^= Command.cmd_len[1];
	checksum ^= Command.cmd[0];
	checksum ^= Command.cmd[1];

	for(i = 0; i < Command.bufIndex - INDEX_CMP; i++)
	{
		checksum ^= Command.p_cmp[i];
	}

	return checksum;
}

/* 取得报文数据的头
   Return :
   0	数据格式错误或者超时
   1	收到DLE ENQ
   2	收到命令数据头
 */
static unsigned char  CMD_Receive_Head(void)
{
	unsigned char ret = FALSE;
	unsigned char timeout = CMD_TIMEOUT;
	uint8_t ch;
	while(1)
	{
		if(!PARSER_GET_CH(&ch))
		{
			if(timeout)
			{
				delay_ms(1);
				timeout--;
			}
			else
			{
				ret = CMD_RECFAILED;
				break;
			}
		}
		else
		{
			if((m_cmd.bufIndex == INDEX_STX) && (ch == STX))
			{
				CMD_Reset();                 //在此初始化命令以及数据回应结构体
				m_cmd.bufIndex = INDEX_LEN1;
				ret = CMD_RECSUCCESS;
				break;
			}
			else if((m_cmd.bufIndex == INDEX_STX) && (ch == ENQ))
			{
				ret = CMD_RECREPEAT;
				break;
			}
			else
			{
				ret = CMD_RECFAILED;
				break;
			}
		}
	}

	return ret;
}

static unsigned char CMD_Receive_Data(void)
{
   //此处添加命令接收代码
   
	unsigned char  ch, ret = FALSE; 
	unsigned char  dle_cnt = 0, length = 0;
	unsigned short timeout = CMD_TIMEOUT;
	
	while(1)
	{
		if(PARSER_GET_CH(&ch))
		{
			timeout = CMD_TIMEOUT;
			if(m_cmd.bufIndex == INDEX_LEN1)//命令长度的第一个字节
			{
				m_cmd.cmd_len[0] = ch;
				m_cmd.bufIndex = INDEX_LEN2;
			}
			else if(m_cmd.bufIndex == INDEX_LEN2)//命令长度的第二个字节
			{
				m_cmd.cmd_len[1] = ch;
				m_cmd.bufIndex = INDEX_IDN;
				length = ((m_cmd.cmd_len[0] << 8)|(m_cmd.cmd_len[1]));
			}
			else if((m_cmd.bufIndex == INDEX_IDN) && (ch == 'C')) //取得命令标识符
			{
				m_cmd.bufIndex = INDEX_CMD1;
				m_cmd.idn = ch;
			}
			else if(m_cmd.bufIndex == INDEX_CMD1)//接收命令第一个字节
			{
				m_cmd.cmd[0] = ch;
				m_cmd.bufIndex = INDEX_CMD2;
				if(ch == DLE)
				{
					dle_cnt = 1;
				}
				else
				{
					dle_cnt = 0;
				}
			}
			else if(m_cmd.bufIndex == INDEX_CMD2)//接收命令第二个字节
			{
				if(dle_cnt == 1)
				{
					if(ch == DLE)
					{
						dle_cnt = 0;
					}
					else
					{
						dle_cnt = 0;
						m_cmd.cmd[1] = ch;
				        m_cmd.bufIndex = INDEX_CMP;
					}
				}
				else
				{
					if(ch == DLE)
					{
						dle_cnt = 1;
						m_cmd.cmd[1] = ch;
						m_cmd.bufIndex = INDEX_CMP;
					}
					else
					{
						dle_cnt = 0;
						m_cmd.cmd[1] = ch;
						m_cmd.bufIndex = INDEX_CMP;
					}
				}
			}
			else if((m_cmd.bufIndex >= INDEX_CMP) && (m_cmd.bufIndex < length - 1)) //收到命令参数包括(DLE STX)
			{
				if(dle_cnt)//已取得一个DLE
				{
					dle_cnt = 0;
					if(ch == ETX)
					{
						m_cmd.p_cmp[m_cmd.bufIndex++ - INDEX_CMP] = ch;
						m_cmd.len_cmp = m_cmd.bufIndex - INDEX_CMP - 2; //减去DLE ETX
					}
					else if(ch != DLE)//再次收到DLE不计入
					{
						m_cmd.p_cmp[m_cmd.bufIndex++ - INDEX_CMP] = ch;
						m_cmd.len_cmp++;
					}
				}
				else if(ch == DLE)
				{
					m_cmd.p_cmp[m_cmd.bufIndex++ - INDEX_CMP] = ch;
					m_cmd.len_cmp++;
					dle_cnt = 1;
				}
				else
				{
					m_cmd.p_cmp[m_cmd.bufIndex++ - INDEX_CMP] = ch;
					m_cmd.len_cmp++;
					dle_cnt = 0;
				}
			}
			else if(m_cmd.bufIndex == length - 1)
			{
				if(ch == CMD_BCC_Result(m_cmd))
				{
					ret = TRUE;
					break;
				}
				else
				{
					ret = FALSE;
					break;
				}
			}
		}
		else
		{
			if(timeout)
			{
				delay_ms(1);
				timeout--;
			}
			else
			{
				ret = FALSE;
				break;
			}
		}
	}

	return ret;
}
	
/* 取得DLE ENQ
   Return :
   0	数据格式错误或者超时
   1	收到DLE ENQ
   2	收到命令数据头
 */
static unsigned char CMD_WaitENQ(void)
{
	uint8_t Data;
	uint16_t timeout = ENQ_TIMEOUT;
	unsigned char dle_flag = INDEX_DLE;
	unsigned char ret = CMD_RECFAILED;
	
	while(1)
	{
		if(PARSER_GET_CH(&Data))
		{
			timeout = ENQ_TIMEOUT;
			if((dle_flag == INDEX_DLE) && (Data == DLE))
			{
				dle_flag = INDEX_ENQ;
			}
			else if((dle_flag == INDEX_ENQ) && (Data == ENQ))
			{
				ret = CMD_RECSUCCESS;
				break;
			}
			else if((dle_flag == INDEX_ENQ) && (Data == STX))
			{
				CMD_Reset();                 //在此初始化命令以及数据回应结构体
				m_cmd.bufIndex = INDEX_LEN1;
				ret = CMD_RECREPEAT;
				break;
			}
			else
			{
				ret = CMD_RECFAILED;
				break;
			}
		}
		else
		{
			if(timeout)
			{
				delay_ms(1);
				timeout--;
			}
			else
			{
				ret = CMD_RECFAILED;
				break;
			}
		}
	}
	return ret;
}

void CMD_Execute(void)
{
	m_rpy.rcm[0] = m_cmd.cmd[0];
	m_rpy.rcm[1] = m_cmd.cmd[1];
	m_rpy.len_rdt = 0;
	SetRespCode(RESP_SUCCESS);		//default success
	//调用命令执行函数
	switch(m_cmd.cmd[0])
	{
		case 'B':
			CMD_B();
			break;
		case 'D':
			CMD_D();
			break;
		case 'E':
			CMD_E();
			break;
		case 'G':
			CMD_G();
			break;
		case 'Q':
			CMD_Q();
			break;
		case 'R':
			CMD_R();
			break;
		case 'S':
			CMD_S();
			break;
		case 'X':
			CMD_X();
			break;
		case 'd':
			CMD_d();
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}

	m_rpy.vailable = 1;//置应答结构体为有效。
}

void CMD_Answer(void)
{
	uint8_t bcc = 0;
	unsigned short length = 0,i = 0;
	unsigned char *str = RPY_Str;
	unsigned char buf[2] = {0};
	unsigned char str_digtube[3];

	m_rpy.res = GetRespCode(buf);
	if(m_rpy.res <= RESP_SUCC_FAIL_BOUND)
	{
		m_rpy.jdg = 'P';
		str_digtube[0] = 'P';
		str_digtube[1] = buf[0];
		str_digtube[2] = buf[1];
		Digtube_Show(str_digtube, DIGTUBE_MODE_ON);
	}
	else
	{
		m_rpy.jdg = 'N';
		str_digtube[0] = 'N';
		str_digtube[1] = buf[0];
		str_digtube[2] = buf[1];
		Digtube_Show(str_digtube, DIGTUBE_MODE_BLINK);
	}
	str[length++] = DLE;
	str[length++] = STX;
	//回应数据长度
	str[length++] = (((CONST_RPY_LEN + m_rpy.len_rdt)>>8) & 0xFF);
	str[length++] = (CONST_RPY_LEN + m_rpy.len_rdt & 0xFF);

	str[length++] = m_rpy.jdg;
	str[length++] = m_rpy.rcm[0];
	str[length++] = m_rpy.rcm[1];
	
	//填充RES
	str[length++] = buf[0];
	str[length++] = buf[1];
	//填充RDT
	for(i = 0; i < m_rpy.len_rdt; i++)
	{
		str[length++] = m_rpy.p_rdt[i];
		
		if(m_rpy.p_rdt[i] == DLE) 		//DLE转义成DLE DLE
		{
			str[length++] = DLE;
		}
	}
	str[length++] = DLE;
	str[length++] = ETX;

	for(i = 0; i < length; i++)//bcc, form DLE STX to DLE ETX
	{
		bcc ^= str[i];
		if((str[i] == DLE) && (str[i+1] == DLE))  //两个DLE时BCC仅计算一个DLE
		{
			i++;
		}
	}
	str[length++] = bcc;

	//发送回应数据
	PARSER_SEND_BUF((char *)str,length);
}

/* 由于某些过程时间太长，所以需要提前进行回应，然后设备进入忙碌状态 */
void CMD_PreAnswer(void)
{
	pre_answer = 1;
	CMD_Answer();
}

void CMD_Parser(void)
{
	//此处添加命令接收解析，应答打包返回相关代码
	uint8_t ch, ret_enq_val = 0;
	uint8_t ret_head_val = 0;

	if(PARSER_GET_CH(&ch))
	{
		if(ch == DLE)
		{
			m_cmd.bufIndex = INDEX_STX;
			ret_head_val = CMD_Receive_Head();
			if(ret_head_val == CMD_RECSUCCESS)
			{
_get_data_again:
				if(CMD_Receive_Data() == TRUE)
				{
				    CMD_Send_ACKOrNAK(TRUE);
					ret_enq_val = CMD_WaitENQ();
					
					if(ret_enq_val == CMD_RECSUCCESS)
					{
						CMD_Execute();
						if(pre_answer == 1)					//预先已回应不重复回应
						{
							pre_answer = 0;
						}
						else
						{
							CMD_Answer();
						}
					}
					else if(ret_enq_val == CMD_RECREPEAT) //收到报文头(DLE STX)
					{
						goto _get_data_again;
					}
					else    //DLE ENQ接收错误
					{
						CMD_Reset();
					}
				}
				else
				{
					CMD_Send_ACKOrNAK(FALSE);
					CMD_Reset();
				}
			}
			else if(ret_head_val == CMD_RECREPEAT)//设备再次返回应答数据
			{
				if(m_rpy.vailable == 1)
					CMD_Answer();
 			}
			else
			{
				CMD_Reset();
			}
		}
		else
		{
			CMD_Reset();
		} // if(ch == DLE)
		
	} //end of     if(PARSER_GET_CH(&ch))
}

//设置设备为忙状态，当设备为忙时返回忙状态
void CMD_SetBusySt(unsigned char busy)
{
	machine_is_busy = busy;
}

//获取设备为忙状态，当设备为忙时返回忙状态
unsigned char CMD_GetBusySt(void)
{
	return machine_is_busy;
}

void CMD_IfBusyReturn(void)
{
	char str[] = {DLE, SYN};		//设备忙碌返回DLE SYN
	unsigned char ch;
	if(CMD_GetBusySt())
	{
		if(PARSER_GET_CH(&ch))
		{
			if((ch == DLE) && (!busy_step))		//DLE ETX BCC
			{
				busy_step = 1;
			}
			else if((ch == ETX) && (busy_step == 1))
			{
				busy_step = 2;
			}
			else if(busy_step == 2)
			{
				PARSER_SEND_BUF(str, sizeof(str));
			}
			else
			{
				busy_step = 0;
			}
		}
	}
	else
	{
		busy_step = 0;
	}
}


