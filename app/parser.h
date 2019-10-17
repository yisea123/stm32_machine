/*
  Filename: parser.h
  Author: shidawei			Date: 20130221
  Copyright: HuNan GreatWall Information Financial Equipment Co.,Ltd.
  Description: 通讯协议与命令解析头文件
 */

#ifndef __PARSER_H__
#define __PARSER_H__
#include "s_def.h"


//macro
#define MAX_CMP_LEN		256
#define MAX_CMD_LEN		(MAX_CMP_LEN + 6)
#define MAX_RDT_LEN		(10*1024)
#define MAX_RPY_LEN		(MAX_RDT_LEN + 8)

#define CONST_RPY_LEN   12   //(DLE STX JDG + TWO Byte Len + TWO Byte RCM + TWO Byte RES + DLE ETX BCC)

//等待时限定义

#define  CMD_TIMEOUT  	5
#define  ENQ_TIMEOUT  	500

#define INDEX_STX		1
#define INDEX_LEN1      2
#define INDEX_LEN2      3
#define INDEX_IDN		4
#define INDEX_CMD1		5
#define INDEX_CMD2		6
#define INDEX_CMP		7

#define INDEX_DLE       1
#define INDEX_ENQ       2

#define CMD_RECFAILED   0
#define CMD_RECSUCCESS  1
#define CMD_RECREPEAT   2

typedef struct
{
	unsigned char	idn;			//command identification
	unsigned char	cmd[2];			//command
	unsigned char   cmd_len[2];     //command  length
	unsigned char *	p_cmp;			//command parameters
	unsigned char   bufIndex;       //当前字节所在缓冲区位置
	unsigned short	len_cmp;		//command parameter length
}t_cmd;

typedef struct
{
	unsigned char   vailable;       //is rpy  validate;
	unsigned char	jdg;			//judge, reply is nagative or positive
	unsigned char	rcm[2];			//reply command
	unsigned char	res;			//result
	unsigned char *	p_rdt;			//reply data table	
	unsigned short	len_rdt;		//rdt length
}t_rpy;

extern t_cmd    m_cmd;
extern t_rpy    m_rpy;
extern unsigned char CMD_CMP_Buf[MAX_CMP_LEN];
extern unsigned char RPY_RDT_Buf[MAX_RDT_LEN];

//functions declare
extern void Parser_Init(void);
extern void CMD_Parser(void);
extern void CMD_Answer(void);
extern void CMD_PreAnswer(void);
extern void CMD_ShowErrCode(uint8_t rpy_res);
void CMD_SetBusySt(unsigned char busy);
unsigned char CMD_GetBusySt(void);
void CMD_IfBusyReturn(void);

#endif


