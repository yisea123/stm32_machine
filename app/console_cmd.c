/*
	¿ØÖÆÌ¨ÃüÁî
*/
#include "s_def.h"
#include "console.h"
#include "stdio.h"
#include "string.h"
#include "com.h"
#include "motor.h"
#include "dcmotor.h"
#include "sensor.h"
#include "actions.h"
#include "qrcode_scan.h"
#include "console_cmd.h"

#define MOVE_SPEED			948

void cmd_xmove(void)
{
	unsigned int val1 = 0, val2 = 0;
	unsigned int steps, m_steps, speed;
	t_motor_dir dir;
	con_puts("pls input the direction.\r\n");
	val1 = console_get_parament();
	if(val1)
	{
		dir = X_FORWARD;
	}
	else
	{
		dir = X_BACKWARD;
	}
	con_puts("pls input steps(0.1mm).\r\n");
	val2 = console_get_parament();
	steps = mm_to_step_x(val2)/10;
	m_steps = (mm_to_step_x(val2)%10)*MOTOR_MICRO_STEP_MODE/10;
	con_puts("pls input speed(pps).\r\n");
	val2 = console_get_parament();
	speed = 1000000/val2;
	Motor_Move(MOTOR_X, steps, m_steps, dir, speed);
	Motor_WaitStoped(MOTOR_X);
	Motor_Hold(MOTOR_X, ENABLE);
}

void cmd_ymove(void)
{
	unsigned int val1 = 0, val2 = 0;
	unsigned int steps, speed;
	t_motor_dir dir;
	con_puts("pls input the direction.\r\n");
	val1 = console_get_parament();
	if(val1)
	{
		dir = Y_FORWARD;
	}
	else
	{
		dir = Y_BACKWARD;
	}
	con_puts("pls input steps(0.1mm).\r\n");
	val2 = console_get_parament();
	steps = mm_to_step_y(val2)/10;
	con_puts("pls input speed(pps).\r\n");
	val2 = console_get_parament();
	speed = 1000000/val2;
	Motor_Move(MOTOR_Y, steps, 0, dir, speed);
	Motor_WaitStoped(MOTOR_Y);
	Motor_Hold(MOTOR_Y, ENABLE);
}

void cmd_zmove(void)
{
	unsigned int val1 = 0, val2 = 0;
	unsigned int steps, speed;
	t_motor_dir dir;
	con_puts("pls input the direction.\r\n");
	val1 = console_get_parament();
	if(val1)
	{
		dir = Z_FORWARD;
	}
	else
	{
		dir = Z_BACKWARD;
	}
	con_puts("pls input steps(0.1mm).\r\n");
	val2 = console_get_parament();
	steps = mm_to_step_z(val2)/10;
	con_puts("pls input speed(pps).\r\n");
	val2 = console_get_parament();
	speed = 1000000/val2;
	Motor_Move(MOTOR_Z, steps, 0, dir, speed);
	Motor_WaitStoped(MOTOR_Z);
	//Motor_Hold(MOTOR_Z, ENABLE);
}

void cmd_hookmove(void)
{
	unsigned int val1 = 0, val2 = 0;
	unsigned int steps, m_steps, speed;
	t_motor_dir dir;
	con_puts("pls input the direction.\r\n");
	val1 = console_get_parament();
	if(val1)
	{
		dir = HOOK_FORWARD;
	}
	else
	{
		dir = HOOK_BACKWARD;
	}
	con_puts("pls input steps(0.1degree).\r\n");
	val2 = console_get_parament();
	steps = degree_to_step_hook(val2)/10;
	m_steps = (degree_to_step_hook(val2)%10)*MOTOR_MICRO_STEP_MODE/10;
	con_puts("pls input speed(pps).\r\n");
	val2 = console_get_parament();
	speed = 1000000/val2;
	Motor_Move(MOTOR_HOOK, steps, m_steps, dir, speed);
	Motor_WaitStoped(MOTOR_HOOK);
}

void cmd_scanner_move(void)
{
	unsigned int val1 = 0, val2 = 0;
	unsigned int steps, m_steps, speed;
	t_motor_dir dir;
	con_puts("pls input the direction.\r\n");
	val1 = console_get_parament();
	if(val1)
	{
		dir = SCAN_FORWARD;
	}
	else
	{
		dir = SCAN_BACKWARD;
	}
	con_puts("pls input steps(0.1mm).\r\n");
	val2 = console_get_parament();
	steps = degree_to_step_scanner(val2)/10;
	m_steps = (degree_to_step_scanner(val2)%10)*MOTOR_MICRO_STEP_MODE/10;
	con_puts("pls input speed(pps).\r\n");
	val2 = console_get_parament();
	speed = 1000000/val2;
	Motor_Move(MOTOR_SCANNER, steps, m_steps, dir, speed);
	Motor_WaitStoped(MOTOR_SCANNER);
	Motor_Hold(MOTOR_SCANNER, ENABLE);
}

void cmd_hold(void)
{
	unsigned char val, enable;
	t_motor_no motor_no;
	con_puts("pls input hold motor.\r\n");
	val = console_get_parament();
	if((val > 3) || val == 0)
	{
		con_puts("\r\nparament error.");
		return;
	}
	motor_no = (t_motor_no)(val - 1);
	con_puts("pls input hold or not.\r\n");
	enable = console_get_parament();
	if(enable)
		Motor_Hold(motor_no, ENABLE);
	else
		Motor_Hold(motor_no, DISABLE);
}


void cmd_sensor_st(void)
{
	uint32_t sen_st = 0, mask = 0;
	unsigned int i = 0;
	char str[128];
	char * ptr;
	sen_st = Sensor_AllStatus();
	mask = 1;
	ptr = str;
	con_puts("\r\nSensor Status:\r\n");
	for(i = 0; i < S_MAX; i++)
	{
		if((i != 0) && (i % 5 == 0))
		{
			*ptr++ = ' ';
			*ptr++ = ' ';
		}
		if(sen_st & (mask << i))
		{
			*ptr++ = '1';
		}
		else
		{
			*ptr++ = '0';
		}
	}
	*ptr++ = '\0';
	con_puts(str);
}

void cmd_sensor_enable(void)
{
	unsigned int i = 0;
	FunctionalState enable = DISABLE;
	con_puts("\r\nSensor enable:\r\n");
	con_puts("0: disable, 1: enable.\r\n");
	if(console_get_parament())
		enable = ENABLE;
	for(i = 0; i < S_MAX; i++)
	{
		Sensor_Enable((t_sen_no)i, enable);
	}
}

void cmd_xypos(void)
{
	unsigned char x_pos, y_pos;
	con_puts("pls input x pos.\r\n");
	x_pos = console_get_parament();
	con_puts("pls input y pos.\r\n");
	y_pos = console_get_parament();
	if(!Actions_XYMove(x_pos, y_pos))
	{
		con_puts("xy position move failed.");
	}
}

void cmd_scan(void)
{
	uint8_t str[128];
	unsigned int len = 0;

	memset(str, 0, sizeof(str));
	if(QRCodeScan_Read(QRCODE_SCAN_DEV, str, &len, SCAN_TIMEOUT_MS))
	{
		con_puts("scanner error.\r\n");
	}
	else
	{
		if(len)
		{
			con_puts("\r\n");
			con_puts((char *)str);
		}
		else
		{
			con_puts("\r\nscanner error.");
		}
	}
#if SCAN_STANDBY_CASE_TOP
	memset(str, 0, sizeof(str));
	if(QRCodeScan_Read(QRCODE_SCAN_DEV_STANDBY_TOP, str, &len, SCAN_TIMEOUT_MS))
	{
		con_puts("scanner error.\r\n");
	}
	else
	{
		if(len)
		{
			con_puts("\r\n");
			con_puts((char *)str);
		}
		else
		{
			con_puts("\r\nscanner error.");
		}
	}
#endif
}

void cmd_get_put(void)
{
	unsigned char val;
	t_zmove_action action;
	con_puts("pls input 0: reset, 1: get, 2: send, 3: push,\r\n 4: push not return, 5: pull at halfway.\r\n");
	val = console_get_parament();
	switch(val)
	{
		case 1:
			action = ZMOVE_PULL;
			break;
		case 2:
			action = ZMOVE_SEND;
			break;
		case 3:
			action = ZMOVE_PUSH;
			break;
		case 4:
			action = ZMOVE_PUSH_NO_RETURN;
			break;
		case 5:
			action = ZMOVE_PULL_AT_HALFWAY;
			break;
		default:
			action = ZMOVE_RESET;
			break;
	}
	
	if(!Actions_ZMove(action, HIGH_SPEED))
	{
		con_puts("box move error.\r\n");
	}
}

