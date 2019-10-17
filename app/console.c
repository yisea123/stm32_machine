/*
  简单控制台程序
  shidawei 201708
*/
#include "main.h"
#include "stdio.h"
#include "com.h"
#include "console.h"
#include "string.h"
#include "console_cmd.h"
#include "console.h"

static char cmd_str[32];
static char start_str[] = "\r\nGreatWall Information Co.\r\n";
static char console_start_str[] = "\r\ngwi>";

static int cmd_index = 0;

void help_process(void);
void cmd_undef(void);
void tst_para(void);
void console_quit(void);


PC_CMD PC_CMD_TAB[] =
{
	{"help",			help_process,		"List all of the valid commands."	},
	{"undef_cmd",		cmd_undef,			"Undefined command."				},
	{"quit",			console_quit,		"Console quit."						},
		
	{"xmove",			cmd_xmove,			"X Move, 1: dir, 2: steps(0.1mm)."	},
	{"ymove",			cmd_ymove,			"Y Move, 1: dir, 2: steps(0.1mm)." 	},
	{"zmove",			cmd_zmove,			"Z Move, 1: dir, 2: steps(0.1mm), 3: speed(pps)." 	},
	{"hookmove",		cmd_hookmove,		"Hook Move, 1: dir, 2: steps(0.1mm), 3: speed(pps)." 	},
	{"scanmove",		cmd_scanner_move,	"scan move."						},
	{"hold",			cmd_hold,			"Motor to hold."					},
	{"sensor",			cmd_sensor_st,		"all sensor status."				},
	{"sensor_en",		cmd_sensor_enable,	"enable or disable all sensor."		},
	{"xypos",			cmd_xypos,			"locate to (x, y)."					},
	{"boxget",			cmd_get_put,		"get/put/send/reset the box."		},
	{"scan",			cmd_scan,			"scan qr code."						},
};

void con_puts(char * str)
{
	unsigned int size = 0;
	size = strlen(str);
	con_sendbuf((char *)str, size);
	delay_ms(2);
}

/* 将数据用16进制输出 */
void con_put_hex(char * data, int size)
{
	int i = 0;
	char str[5];
	memset(str, 0, sizeof(str));
	for(i = 0; i < size; i++)
	{
		sprintf(str, " %2x", data[i]);
		con_puts(str);
	}
	con_puts("\r\n");
}

int console_get_parament(void)
{
	uint8_t ch = 0;
	int val = 0;
	con_puts("\tInputs a value: ");
	while (1)
	{
		if(con_getc(&ch))
		{
			if((ch >= '0') && (ch <= '9'))
			{
				con_putc(ch);
				val = val * 10 + ch - '0';
			}
			else if(ch == 0x0D)
			{
					con_puts("\r\n");
					break;
			}
			else if(ch == 0x0A)
			{
					continue;
			}
			else
			{
				con_puts("\r\nInvalid paraments. Pls inputs again : ");
				val = 0;
			}
		}
	}
	return val;
}

static void find_cmd(char *cmd)
{
	unsigned int cnt=0;
	for(cnt=0; cnt < sizeof(PC_CMD_TAB)/sizeof(PC_CMD_TAB[0]); cnt++)
	{
		if(strcmp(PC_CMD_TAB[cnt].cmd_str, (char*)cmd)==0)
		{
			PC_CMD_TAB[cnt].cmd_process();
			return;
		}
	}
	if(cnt >= sizeof(PC_CMD_TAB)/sizeof(PC_CMD_TAB[0]))
	{
		cmd_undef();
	}
}

/************************************************************
	GOLBAL FUNCTIONS
*************************************************************/
void console_init(void)
{
	con_com_enable(ENABLE);
	memset(cmd_str, 0, sizeof(cmd_str));

}

void console_welcome(void)
{
	con_puts(start_str);
	help_process();
	con_puts(console_start_str);
}

void console(void)
{
	uint8_t ch = 0;
//	while(1)
//	{
		if(con_getc(&ch))	//获得一个命令
		{
			con_putc(ch);
			if(ch <= 0x20)
			{
				//con_putc(0x0A);
				if((ch == 0x0D))	//执行命令
				{
					if(cmd_index != 0)
					{
						find_cmd(cmd_str);
						memset(cmd_str, 0, sizeof(cmd_str));
						cmd_index = 0;
						con_puts(console_start_str);
					}
					else
					{
						con_puts(console_start_str);
						return;
					}
				}
				else if(ch == 0x1B)		//ESC 取消命令
				{
					memset(cmd_str, 0, sizeof(cmd_str));
					cmd_index = 0;
					con_puts("\r\nCanneled");
					con_puts(console_start_str);
				}
				else if(ch == 0x08)		//backspace
				{
					if(cmd_index > 0)
					{
						cmd_index--;
						cmd_str[cmd_index] = 0;
					}
				}
			}
			else
			{
				cmd_str[cmd_index++] = ch;
			}
		}
//	}
}

void help_process(void)
{
	unsigned int cnt = 0;
	con_puts("\r\n");
	for(cnt = 0; cnt < sizeof(PC_CMD_TAB)/sizeof(PC_CMD_TAB[0]); cnt++)
	{
		con_puts(PC_CMD_TAB[cnt].cmd_str);
		con_puts("\t\t");
		con_puts(PC_CMD_TAB[cnt].help_str);
		con_puts("\r\n");
	}
	con_puts("ESC: Cannceled the input.\r\n");
}

void cmd_undef(void)
{
	con_puts("\r\nUndefined Command.\r\n");
}

void tst_para(void)
{
	int value;
	char str1[100];
	con_puts("\r\npls input a parament.");
	value = console_get_parament();
	sprintf(str1, "parament is %d.\r\n", value);
	con_puts(str1);
}

void console_quit(void)
{
	quit = 1;
}


