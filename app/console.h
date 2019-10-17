/*
  简单控制台程序
*/
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define COM_CONSOLE			MAIN_COM1

#define con_com_enable(en)		Com_Enable(COM_CONSOLE, en)
#define con_getc(ch)			Com_Receive_Ch(COM_CONSOLE, ch)
#define con_putc(ch)			Com_Send_Ch(COM_CONSOLE, ch)
#define con_sendbuf(ptx_buff, tx_size)	Com_Send_Buffer(COM_CONSOLE, ptx_buff, tx_size)

typedef void (*process_cmd)(void);

typedef struct
{
        char * cmd_str;
        process_cmd  cmd_process;
        char * help_str;
} PC_CMD;

void console_init(void);
void console_welcome(void);
void console(void);
int console_get_parament(void);
void con_puts(char * str);
void con_put_hex(char * data, int size);


#endif

