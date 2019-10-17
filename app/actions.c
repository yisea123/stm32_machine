/*
	实际动作
*/
#include "s_def.h"
#include "actions.h"
#include "motor.h"
#include "sensor.h"
#include "dcmotor.h"
#include "console.h"
#include "main.h"
#include "settings.h"
#include "respcode.h"
#include "qrcode_scan.h"
#include "com.h"
#include "string.h"

#define WAIT_TIME_FOR_STOP			500
#define SENSOR_INT_TO_STOP_MOTOR	1		//motor stoped using sensor interrupt

t_pin_source motory_alm_pin =
{GPIOF, RCC_APB2Periph_GPIOF, GPIO_Pin_11,	GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz, NULL};

static unsigned char Actions_ZReset(t_speed speed);

/***************************************************************************
 * LOCAL FUNCTIONS
 ***************************************************************************/
//MotorY伺服电机出错警告信号
static void MotorY_AlmInit(void)
{
	Pin_Init(&motory_alm_pin);
}
//MotorY伺服电机出错报警
static unsigned char MotorY_Alarm(void)
{
	return Pin_ReadBit(&motory_alm_pin);
}

static unsigned char Actions_HookUp(void)
{
	if(Sensor_Status(S_HOOK_UP))
	{
		return 1;
	}
	Sensor_IntEnable(S_HOOK_UP, Actions_HookUpInt, EXTI_TRIGGER_MODE_HOOK, ENABLE);
	Motor_Move(MOTOR_HOOK, HOOK_RST_DISTANCE_STEPS, 0, HOOK_FORWARD, MOTOR_HOOK_FULL_SPEED);
	Motor_WaitStoped(MOTOR_HOOK);
	Sensor_IntEnable(S_HOOK_UP, Actions_HookUpInt, EXTI_TRIGGER_MODE_HOOK, DISABLE);
	if(!Sensor_Status(S_HOOK_UP))
	{
		return SetRespCode(RESP_HOOK_UP_ERR);
	}
	return 1;
}

static unsigned char Actions_HookDown(void)
{
	if(!Sensor_Status(S_HOOK_UP))
	{
		return 1;
	}
	Motor_Move(MOTOR_HOOK, HOOK_MOVE_DISTANCE_STEPS, HOOK_RST_DISTANCE_MICROSTEPS, HOOK_BACKWARD, MOTOR_HOOK_FULL_SPEED);
	Motor_WaitStoped(MOTOR_HOOK);
	if(Sensor_Status(S_HOOK_UP))
	{
		return SetRespCode(RESP_HOOK_DOWN_ERR);
	}
	return 1;
}

static unsigned char Actions_HookResetDown(void)
{
	if(!Sensor_Status(S_HOOK_UP))
	{
		if(!Actions_HookUp())
			return 0;
	}
	Motor_Move(MOTOR_HOOK, HOOK_MOVE_DISTANCE_STEPS, HOOK_RST_DISTANCE_MICROSTEPS, HOOK_BACKWARD, MOTOR_HOOK_FULL_SPEED);
	Motor_WaitStoped(MOTOR_HOOK);
	if(Sensor_Status(S_HOOK_UP))
	{
		return SetRespCode(RESP_HOOK_DOWN_ERR);
	}
	return 1;
}

static unsigned char Actions_HookDownAfterUpFailed(void)
{
	Motor_Move(MOTOR_HOOK, HOOK_MOVE_DISTANCE_STEPS/3, HOOK_RST_DISTANCE_MICROSTEPS, HOOK_BACKWARD, MOTOR_HOOK_FULL_SPEED);
	Motor_WaitStoped(MOTOR_HOOK);
	if(Sensor_Status(S_HOOK_UP))
	{
		return SetRespCode(RESP_HOOK_DOWN_ERR);
	}
	return 1;
}

static unsigned char Actions_ZMoveRear(t_speed speed)
{
	uint16_t motor_speed;

	if(speed == HALF_SPEED)
	{
		motor_speed = MOTORZ_HALF_SPEED;
	}
	else if(speed == FULL_SPEED)
	{
		motor_speed = MOTORZ_FULL_SPEED;
	}
	else
	{
		motor_speed = MOTORZ_HIGH_SPEED;
	}
	if(Sensor_Status(S_Z_FRONT) && Sensor_Status(S_Z_REAR))
	{
		return SetRespCode(RESP_Z_SENSOR_ERR);
	}
	/* 当Z在最前端，并且检测到有盒子，并且检测到托盘伸出时，可能是钩子上有一个托盘 */
	if(Sensor_Status(S_Z_FRONT) && Sensor_Status(S_DETECT_BOX))
	{
		if(Sensor_Status(S_X0_TEST) || Sensor_Status(S_X1_TEST) || Sensor_Status(S_X2_TEST))
		{
			if((!Sensor_Status(S_HOOK_UP)))
			{
				if(!Actions_HookUp())
				{
					return 0;
				}
			}
		}
	}
	if(!Sensor_Status(S_Z_REAR))
	{
		Sensor_IntEnable(S_Z_REAR, Actions_ZRearInt, EXTI_TRIGGER_MODE_Z_REAR, ENABLE);
		Motor_Move(MOTOR_Z, Z_DISTANCE_STEPS, Z_DISTANCE_MICROSTEPS, Z_BACKWARD, motor_speed);
		Motor_WaitStoped(MOTOR_Z);
		Motor_Hold(MOTOR_Z, ENABLE);
		Sensor_IntEnable(S_Z_REAR, Actions_ZRearInt, EXTI_TRIGGER_MODE_Z_REAR, DISABLE);
	}
	if(!Sensor_Status(S_Z_REAR))
	{
		return SetRespCode(RESP_Z_MOVE_REAR_ERR);
	}
	return 1;
}

static unsigned char Actions_ZMoveFront(t_speed speed)
{
	uint16_t motor_speed;

	if(speed == HALF_SPEED)
	{
		motor_speed = MOTORZ_HALF_SPEED;
	}
	else if(speed == FULL_SPEED)
	{
		motor_speed = MOTORZ_FULL_SPEED;
	}
	else
	{
		motor_speed = MOTORZ_HIGH_SPEED;
	}
	if(Sensor_Status(S_Z_FRONT) && Sensor_Status(S_Z_REAR))
	{
		return SetRespCode(RESP_Z_SENSOR_ERR);
	}
	if((!Sensor_Status(S_Z_FRONT)) && (!Sensor_Status(S_Z_REAR)))	//missed
	{
		if(!Actions_ZMoveRear(FULL_SPEED))
		{
			return 0;
		}
	}
	if(!Sensor_Status(S_Z_FRONT))
	{
		Sensor_IntEnable(S_Z_FRONT, Actions_ZFrontInt, EXTI_TRIGGER_MODE_Z_FRONT, ENABLE);
		Motor_Move(MOTOR_Z, Z_DISTANCE_STEPS, Z_DISTANCE_MICROSTEPS, Z_FORWARD, motor_speed);
		Motor_WaitStoped(MOTOR_Z);
		Motor_Hold(MOTOR_Z, ENABLE);
		Sensor_IntEnable(S_Z_FRONT, Actions_ZFrontInt, EXTI_TRIGGER_MODE_Z_FRONT, DISABLE);
	}
	if(!Sensor_Status(S_Z_FRONT))
	{
		return SetRespCode(RESP_Z_MOVE_FRONT_ERR);
	}
	return 1;
}

static unsigned char Actions_ZMoveToHook(t_speed speed)
{
	uint16_t motor_speed;

	if(speed == HALF_SPEED)
	{
		motor_speed = MOTORZ_HALF_SPEED;
	}
	else if(speed == FULL_SPEED)
	{
		motor_speed = MOTORZ_FULL_SPEED;
	}
	else
	{
		motor_speed = MOTORZ_HIGH_SPEED;
	}
	if(Sensor_Status(S_Z_FRONT) && Sensor_Status(S_Z_REAR))
	{
		return SetRespCode(RESP_Z_SENSOR_ERR);
	}
	if(!Sensor_Status(S_Z_FRONT))
	{
		if(Actions_ZMoveFront(FULL_SPEED))
			return 0;
	}
	if(!Sensor_Status(S_Z_FRONT))
	{
		return SetRespCode(RESP_Z_MOVE_FRONT_ERR);
	}
	Motor_Move(MOTOR_Z, Z_FRONT_TO_HOOK_STEPS, 0, Z_FORWARD, motor_speed);
	Motor_WaitStoped(MOTOR_Z);
	Motor_Hold(MOTOR_Z, ENABLE);
	if(Sensor_Status(S_Z_FRONT))
	{
		return SetRespCode(RESP_Z_MOVE_FRONT_ERR);
	}
	return 1;
}

static unsigned char Actions_ZMoveHookToFront(t_speed speed)
{
	uint16_t motor_speed;

	if(speed == HALF_SPEED)
	{
		motor_speed = MOTORZ_HALF_SPEED;
	}
	else if(speed == FULL_SPEED)
	{
		motor_speed = MOTORZ_FULL_SPEED;
	}
	else
	{
		motor_speed = MOTORZ_HIGH_SPEED;
	}
	if(Sensor_Status(S_Z_FRONT) && Sensor_Status(S_Z_REAR))
	{
		return SetRespCode(RESP_Z_SENSOR_ERR);
	}
	if(!Sensor_Status(S_Z_FRONT))
	{
		Sensor_IntEnable(S_Z_FRONT, Actions_ZFrontInt, EXTI_TRIGGER_MODE_Z_FRONT, ENABLE);
		Motor_Move(MOTOR_Z, (Z_FRONT_TO_HOOK_STEPS + mm_to_step_z(10)), Z_DISTANCE_MICROSTEPS, Z_BACKWARD, motor_speed);
		Motor_WaitStoped(MOTOR_Z);
		Motor_Hold(MOTOR_Z, ENABLE);
		Sensor_IntEnable(S_Z_FRONT, Actions_ZFrontInt, EXTI_TRIGGER_MODE_Z_FRONT, DISABLE);
	}
	if(!Sensor_Status(S_Z_FRONT))
	{
		return SetRespCode(RESP_Z_MOVE_REAR_ERR);
	}
	return 1;
}

static unsigned char Actions_ZMoveToUnhook(t_speed speed)
{
	uint16_t motor_speed;

	if(speed == HALF_SPEED)
	{
		motor_speed = MOTORZ_HALF_SPEED;
	}
	else if(speed == FULL_SPEED)
	{
		motor_speed = MOTORZ_FULL_SPEED;
	}
	else
	{
		motor_speed = MOTORZ_HIGH_SPEED;
	}
	if(Sensor_Status(S_Z_FRONT) && Sensor_Status(S_Z_REAR))
	{
		return SetRespCode(RESP_Z_SENSOR_ERR);
	}
	if(!Sensor_Status(S_Z_FRONT))
	{
		if(Actions_ZMoveFront(speed))
			return 0;
	}
	if(!Sensor_Status(S_Z_FRONT))
	{
		return SetRespCode(RESP_Z_MOVE_FRONT_ERR);
	}
	Motor_Move(MOTOR_Z, Z_FRONT_TO_UNHOOK_STEPS, 0, Z_FORWARD, motor_speed);
	Motor_WaitStoped(MOTOR_Z);
	Motor_Move(MOTOR_Z, Z_UNHOOK_BACK_STEPS, 0, Z_BACKWARD, motor_speed);
	Motor_WaitStoped(MOTOR_Z);
	Motor_Hold(MOTOR_Z, ENABLE);
	if(Sensor_Status(S_Z_FRONT))
	{
		return SetRespCode(RESP_Z_MOVE_FRONT_ERR);
	}
	return 1;
}

static unsigned char Actions_ZReset(t_speed speed)
{
	if(Sensor_Status(S_DETECT_BOX))		//检测到有盒子
	{
		if(Sensor_Status(S_X0_TEST) || Sensor_Status(S_X1_TEST) || Sensor_Status(S_X2_TEST))	//可能有盒子没有推进去
		{
			if(!Actions_ZMove(ZMOVE_PUSH, speed))
				return 0;
		}
		if(!Actions_HookResetDown())
		{
			return 0;
		}
		if(!Actions_ZMoveRear(speed))
		{
			return 0;
		}
		if(!Actions_HookUp())
		{
			return 0;
		}
	}
	else
	{
		if(!Actions_HookResetDown())
		{
			return 0;
		}
		if(!Actions_ZMoveRear(speed))
		{
			return 0;
		}
		if(!Actions_HookUp())
		{
			return 0;
		}
	}
	
	return 1;
}

static unsigned char Actions_ScannerUp(void)
{
	if(Sensor_Status(S_SCANNER_UP))
	{
		return 1;
	}
	Sensor_IntEnable(S_SCANNER_UP, Actions_ScannerUpInt, EXTI_TRIGGER_MODE_SCANNER, ENABLE);
	Motor_Move(MOTOR_SCANNER, SCANNER_RST_DISTANCE_STEPS, SCANNER_RST_DISTANCE_MICROSTEPS, SCAN_FORWARD, MOTOR_SCANNER_FULL_SPEED);
	Motor_WaitStoped(MOTOR_SCANNER);
	Sensor_IntEnable(S_SCANNER_UP, Actions_ScannerUpInt, EXTI_TRIGGER_MODE_SCANNER, DISABLE);
	Motor_Hold(MOTOR_SCANNER, ENABLE);
	if(!Sensor_Status(S_SCANNER_UP))
	{
		return SetRespCode(RESP_SCANNER_UP_ERR);
	}

	return 1;
}

static unsigned char Actions_ScannerDown(void)
{
	if(!Sensor_Status(S_SCANNER_UP))
	{
		return 1;
	}
	Motor_Move(MOTOR_SCANNER, SCANNER_MOVE_DISTANCE_STEPS, 0, SCAN_BACKWARD, MOTOR_SCANNER_FULL_SPEED);
	Motor_WaitStoped(MOTOR_SCANNER);
	Motor_Hold(MOTOR_SCANNER, ENABLE);
	if(Sensor_Status(S_SCANNER_UP))
	{
		return SetRespCode(RESP_SCANNER_DOWN_ERR);
	}

	return 1;
}

static unsigned int Actions_YStepsCalc(unsigned char next_pos, unsigned char cur_pos)
{
	unsigned int steps = 0;
	unsigned char min_pos, big_pos;
	if(cur_pos < next_pos)
	{
		min_pos = cur_pos;
		big_pos = next_pos;
	}
	else
	{
		min_pos = next_pos;
		big_pos = cur_pos;
	}
	if((min_pos < 4) \
		&& (big_pos >= 4) && (big_pos < 8))
	{
		steps = (big_pos - min_pos) * Y_POS_DISTANCE_STEPS + Y_POS_DISTANCE_STEPS_PLUS;
	}
	else if((min_pos >= 4) && (min_pos < 8) \
		&& (big_pos >= 8) && (big_pos < 12))
	{
		steps = (big_pos - min_pos) * Y_POS_DISTANCE_STEPS + Y_POS_DISTANCE_STEPS_PLUS;
	}
	else if((min_pos < 4) \
		&& (big_pos >= 8) && (big_pos < 12))
	{
		steps = (big_pos - min_pos) * Y_POS_DISTANCE_STEPS + Y_POS_DISTANCE_STEPS_PLUS * 2;
	}
	else if((min_pos < 4) \
		&& (big_pos >= 12) && (big_pos < 16))
	{
		steps = (big_pos - min_pos) * Y_POS_DISTANCE_STEPS + Y_POS_DISTANCE_STEPS_PLUS * 2 + Y_POS_DISTANCE_STEPS_TOP_PLUS;
	}
	else if((min_pos >= 4) && (min_pos < 8) \
		&& (big_pos >= 12) && (big_pos < 16))
	{
		steps = (big_pos - min_pos) * Y_POS_DISTANCE_STEPS + Y_POS_DISTANCE_STEPS_PLUS + Y_POS_DISTANCE_STEPS_TOP_PLUS;
	}
	else if((min_pos >= 8) && (min_pos < 12) \
		&& (big_pos >= 12) && (big_pos < 16))
	{
		steps = (big_pos - min_pos) * Y_POS_DISTANCE_STEPS + Y_POS_DISTANCE_STEPS_TOP_PLUS;
	}
	else
	{
		steps = (big_pos - min_pos) * Y_POS_DISTANCE_STEPS;
	}
	
	return steps;
}

/***************************************************************************
 * GLOBAL FUNCTIONS
 ***************************************************************************/
void StringTailCutUnseeChar(unsigned char * str)
{
	unsigned int str_len = 0;
	int i = 0;
	
	str_len = strlen((char *)str);
	if(str_len)
	{
		for(i = str_len - 1; i > 0; i--)
		{
			if(str[i] < 0x20)
			{
				str[i] = 0x0;
			}
			else
			{
				break;
			}
		}
	}
}

/* x pos中断函数 */
void Actions_XPosInt(void)
{
	unsigned int steps, min_steps, max_steps, stop_steps;
	//如果在范围内的中断在计数，不在范围内中断跳过
	steps = Motor_GetRuningSteps(MOTOR_X);
	if(ACTIONS_DATA.x_run_steps == 0)		//启动第一个需减去半个挡片
	{
		min_steps = ACTIONS_DATA.x_run_steps + X_POS_DISTANCE_STEPS - X_EDGE_POS_DISTANCE - X_LOCATE_PART_WIDTH/2;
		max_steps = ACTIONS_DATA.x_run_steps + X_POS_DISTANCE_STEPS + X_EDGE_POS_DISTANCE - X_LOCATE_PART_WIDTH/2;
	}
	else
	{
		min_steps = ACTIONS_DATA.x_run_steps + X_POS_DISTANCE_STEPS - X_EDGE_POS_DISTANCE;
		max_steps = ACTIONS_DATA.x_run_steps + X_POS_DISTANCE_STEPS + X_EDGE_POS_DISTANCE;
	}
	//计算当前位置
	if(ACTIONS_DATA.x_pos_cur < ACTIONS_DATA.x_pos_to_stop)			//Forward
	{
		ACTIONS_DATA.x_pos_cur++;
		stop_steps = X_LOCATE_STOP_FORWARD_AFTER_STEPS;
	}
	else if(ACTIONS_DATA.x_pos_cur > ACTIONS_DATA.x_pos_to_stop)	//Backward
	{
		ACTIONS_DATA.x_pos_cur--;
		stop_steps = X_LOCATE_STOP_BACKWARD_AFTER_STEPS;
	}
	if((steps < min_steps) || (steps > max_steps))
	{
		return;
	}
	else
	{
		ACTIONS_DATA.x_run_steps = steps;
	}
	if(ACTIONS_DATA.x_pos_cur == ACTIONS_DATA.x_pos_to_stop)		//reached
	{
		Sensor_IntEnable(S_X_LOCATE, NULL, EXTI_TRIGGER_MODE_X_LOCATE, DISABLE);
		Motor_Stop(MOTOR_X, stop_steps, X_STOP_AFTER_MICRO_STEPS);
	}
}

/* Y pos中断函数 */
void Actions_YPosInt(void)
{
	unsigned int steps, min_steps, max_steps, stop_steps;
	//如果在范围内的中断在计数，不在范围内中断跳过
	steps = Motor_GetRuningSteps(MOTOR_Y);
	if(ACTIONS_DATA.y_run_steps == 0)		//启动第一个需减去半个挡片
	{
		min_steps = ACTIONS_DATA.y_run_steps + Y_POS_DISTANCE_STEPS - Y_EDGE_POS_DISTANCE - Y_LOCATE_PART_WIDTH/2;
		max_steps = ACTIONS_DATA.y_run_steps + Y_POS_DISTANCE_STEPS + Y_EDGE_POS_DISTANCE - Y_LOCATE_PART_WIDTH/2;
	}
	else
	{
		min_steps = ACTIONS_DATA.y_run_steps + Y_POS_DISTANCE_STEPS - Y_EDGE_POS_DISTANCE;
		max_steps = ACTIONS_DATA.y_run_steps + Y_POS_DISTANCE_STEPS + Y_EDGE_POS_DISTANCE;
	}
	if(ACTIONS_DATA.y_pos_cur < ACTIONS_DATA.y_pos_to_stop)
	{
		if((ACTIONS_DATA.y_pos_cur == 3) || (ACTIONS_DATA.y_pos_cur == 7))
		{
			min_steps += Y_POS_DISTANCE_STEPS_PLUS;
			max_steps += Y_POS_DISTANCE_STEPS_PLUS;
		}
		else if((ACTIONS_DATA.y_pos_cur == 11))
		{
			min_steps += Y_POS_DISTANCE_STEPS_TOP_PLUS;
			max_steps += Y_POS_DISTANCE_STEPS_TOP_PLUS;
		}
	}
	else
	{
		if((ACTIONS_DATA.y_pos_cur == 4) || (ACTIONS_DATA.y_pos_cur == 8))
		{
			min_steps += Y_POS_DISTANCE_STEPS_PLUS;
			max_steps += Y_POS_DISTANCE_STEPS_PLUS;
		}
		else if((ACTIONS_DATA.y_pos_cur == 12))
		{
			min_steps += Y_POS_DISTANCE_STEPS_TOP_PLUS;
			max_steps += Y_POS_DISTANCE_STEPS_TOP_PLUS;
		}
	}
	//计算当前位置
	if(ACTIONS_DATA.y_pos_cur < ACTIONS_DATA.y_pos_to_stop)			//Forward
	{
		ACTIONS_DATA.y_pos_cur++;
		stop_steps = Y_LOCATE_UP_STOP_AFTER_STEPS;
	}
	else if(ACTIONS_DATA.y_pos_cur > ACTIONS_DATA.y_pos_to_stop)	//Backward
	{
		ACTIONS_DATA.y_pos_cur--;
		stop_steps = Y_LOCATE_DOWN_STOP_AFTER_STEPS;
	}
	if((steps < min_steps) || (steps > max_steps))					//error
	{
		stop_steps = Y_LOCATE_UP_STOP_AFTER_STEPS;
		return;
	}
	else
	{
		ACTIONS_DATA.y_run_steps = steps;
	}
	if(ACTIONS_DATA.y_pos_cur == ACTIONS_DATA.y_pos_to_stop)		//reached
	{
		Sensor_IntEnable(S_Y_LOCATE, NULL, EXTI_TRIGGER_MODE_Y_LOCATE, DISABLE);
		Motor_Stop(MOTOR_Y, stop_steps, Y_STOP_AFTER_MICRO_STEPS);
	}
}

/* x reset中断函数 */
void Actions_XResetInt(void)
{
	unsigned int stop_steps;
	signed short x_adj;
	
	ACTIONS_DATA.x_pos_cur = 0;
	if(settings.settings_saved.machine_info.x_pos_adjust < 0)
	{
		x_adj = mm_to_step_x(-settings.settings_saved.machine_info.x_pos_adjust)/10;
		stop_steps = X_RESET_STOP_AFTER_STEPS - x_adj;
	}
	else
	{
		x_adj = mm_to_step_x(settings.settings_saved.machine_info.x_pos_adjust)/10;
		stop_steps = X_RESET_STOP_AFTER_STEPS + x_adj;
	}
	Sensor_IntEnable(S_X_RESET, NULL, EXTI_TRIGGER_MODE_X_RESET, DISABLE);
	Motor_Stop(MOTOR_X, stop_steps, X_STOP_AFTER_MICRO_STEPS);
}

/* y reset中断函数 */
void Actions_YResetInt(void)
{
	unsigned int stop_steps;
	signed short y_adj;
	
	ACTIONS_DATA.y_pos_cur = 0;
	if(settings.settings_saved.machine_info.y_pos_adjust < 0)
	{
		y_adj = mm_to_step_x(-settings.settings_saved.machine_info.y_pos_adjust)/10;
		stop_steps = Y_RESET_STOP_AFTER_STEPS - y_adj;
	}
	else
	{
		y_adj = mm_to_step_x(settings.settings_saved.machine_info.y_pos_adjust)/10;
		stop_steps = Y_RESET_STOP_AFTER_STEPS + y_adj;
	}
	
	Sensor_IntEnable(S_Y_RESET, NULL, EXTI_TRIGGER_MODE_Y_RESET, DISABLE);
	Motor_Stop(MOTOR_Y, stop_steps, Y_STOP_AFTER_MICRO_STEPS);
}

/* z rear中断函数 */
void Actions_ZRearInt(void)
{
	Sensor_IntEnable(S_Z_REAR, NULL, EXTI_TRIGGER_MODE_Z_REAR, DISABLE);
	Motor_Stop(MOTOR_Z, Z_STOP_AFTER_STEPS, Z_STOP_AFTER_MICRO_STEPS);
}

/* z front中断函数 */
void Actions_ZFrontInt(void)
{
	Sensor_IntEnable(S_Z_FRONT, NULL, EXTI_TRIGGER_MODE_Z_FRONT, DISABLE);
	Motor_Stop(MOTOR_Z, Z_STOP_AFTER_STEPS, Z_STOP_AFTER_MICRO_STEPS);
}

/* hook reset中断函数 */
void Actions_HookUpInt(void)
{
	Sensor_IntEnable(S_HOOK_UP, NULL, EXTI_TRIGGER_MODE_HOOK, DISABLE);
	Motor_Stop(MOTOR_HOOK, HOOK_STOP_AFTER_STEPS, 0);
}

/* scanner reset中断函数 */
void Actions_ScannerUpInt(void)
{
	Sensor_IntEnable(S_SCANNER_UP, NULL, EXTI_TRIGGER_MODE_HOOK, DISABLE);
	Motor_Stop(MOTOR_SCANNER, SCANNER_STOP_AFTER_STEPS, SCANNER_STOP_AFTER_MICROSTEPS);
}

void Actions_Init(void)
{
	MotorY_AlmInit();
}

/*
	move_out_reset == 1,时如果在原点先移出来再回去
*/
unsigned char Actions_Reset(unsigned char move_out_reset)
{
	unsigned char ret = 0;

	if(MotorY_Alarm())
	{
		return SetRespCode(RESP_Y_LOCATE_ERR);
	}
	if(!Actions_ZReset(FULL_SPEED))		/* z reset */
	{
		return 0;
	}
	if(!Sensor_Status(S_DETECT_BOX))		//没有盒子时，钩子进行复位
	{
		if(!Actions_HookDown())		/* hook reset */
		{
			return 0;
		}
	}
	/* 检查盒子是否伸出 */
	if(Sensor_Status(S_X0_TEST) || Sensor_Status(S_X1_TEST) || Sensor_Status(S_X2_TEST))
	{
		return SetRespCode(RESP_BOX_NOT_IN_HOME);
	}
	/* (x,y) is in reset pos, move out */
	if((Sensor_Status(S_Y_RESET) || Sensor_Status(S_X_RESET)) && move_out_reset)
	{
		if(Sensor_Status(S_X_RESET))
		{
			Motor_Move(MOTOR_X, X_RESET_AT_0_DISTANCE, 0, X_FORWARD, MOTORX_FULL_SPEED);
		}
		if(Sensor_Status(S_Y_RESET))
		{
			Motor_Move(MOTOR_Y, Y_RESET_AT_0_DISTANCE, 0, Y_FORWARD, MOTORY_FULL_SPEED);
		}
		while(1)		//如果x先停止了，x先锁死，如果y先停了，y先锁死，防止往下掉
		{
			if(Motor_IsStoped(MOTOR_X))
			{
				Motor_Hold(MOTOR_X, ENABLE);
				break;
			}
			if(Motor_IsStoped(MOTOR_Y))
			{
				Motor_Hold(MOTOR_Y, ENABLE);
				break;
			}
		}
		Motor_WaitStoped(MOTOR_Y);
		Motor_Hold(MOTOR_Y, ENABLE);
		Motor_WaitStoped(MOTOR_X);
		Motor_Hold(MOTOR_X, ENABLE);
		if(Sensor_Status(S_Y_RESET))
		{
			return SetRespCode(RESP_Y_RESET_ERR);
		}
		if(Sensor_Status(S_X_RESET))
		{
			return SetRespCode(RESP_X_RESET_ERR);
		}
	}
	/* (x,y) reset */
	if(!Sensor_Status(S_Y_RESET))
	{
		Sensor_IntEnable(S_Y_RESET, Actions_YResetInt, EXTI_TRIGGER_MODE_Y_RESET, ENABLE);
		Motor_Move(MOTOR_Y, Y_RESET_DISTANCE_STEPS, Y_RESET_DISTANCE_MICROSTEPS, Y_BACKWARD, MOTORY_FULL_SPEED);
	}
	if(!Sensor_Status(S_X_RESET))
	{
		Sensor_IntEnable(S_X_RESET, Actions_XResetInt, EXTI_TRIGGER_MODE_X_RESET, ENABLE);
		Motor_Move(MOTOR_X, X_RESET_DISTANCE_STEPS, X_RESET_DISTANCE_MICROSTEPS, X_BACKWARD, MOTORX_FULL_SPEED);
	}
	while(1)		//如果x先停止了，x先锁死，如果y先停了，y先锁死，防止往下掉
	{
		if(Motor_IsStoped(MOTOR_X))
		{
			Sensor_IntEnable(S_X_RESET, Actions_XResetInt, EXTI_TRIGGER_MODE_X_RESET, DISABLE);
			Motor_Hold(MOTOR_X, ENABLE);
			break;
		}
		if(Motor_IsStoped(MOTOR_Y))
		{
			Sensor_IntEnable(S_Y_RESET, Actions_YResetInt, EXTI_TRIGGER_MODE_Y_RESET, DISABLE);
			Motor_Hold(MOTOR_Y, ENABLE);
			break;
		}
	}
	Motor_WaitStoped(MOTOR_Y);
	Sensor_IntEnable(S_Y_RESET, Actions_YResetInt, EXTI_TRIGGER_MODE_Y_RESET, DISABLE);
	Motor_Hold(MOTOR_Y, ENABLE);
	Motor_WaitStoped(MOTOR_X);
	Sensor_IntEnable(S_X_RESET, Actions_XResetInt, EXTI_TRIGGER_MODE_X_RESET, DISABLE);
	Motor_Hold(MOTOR_X, ENABLE);
	/* confirm (x,y) the result */
	if(!Sensor_Status(S_X_RESET))
	{
		ret = SetRespCode(RESP_X_RESET_ERR);
		return ret;
	}
	else if(!Sensor_Status(S_Y_RESET))
	{
		ret = SetRespCode(RESP_Y_RESET_ERR);
		return ret;
	}
	else
	{
		ret = 1;
		ACTIONS_DATA.x_pos_cur = 0;
		ACTIONS_DATA.y_pos_cur = 0;
	}
	//scanner pos reset
	if(Sensor_Status(S_SCANNER_UP))
	{
		ret = 1;
	}
	else
	{
		ret = Actions_ScannerUp();
	}
	return ret;
}

/* (x,y)按位置定位，如果已经在该位置，进行一次传感器确认 */
static unsigned char Actions_XYMoveDo(uint8_t x_pos, uint8_t y_pos)
{
	unsigned char ret = 0;
	unsigned int x_steps, y_steps;
	unsigned int x_micro_steps = X_POS_DISTANCE_MICROSTEPS;
	unsigned int y_micro_steps = Y_POS_DISTANCE_MICROSTEPS;
	t_motor_dir x_dir, y_dir;

	if(MotorY_Alarm())
	{
		return SetRespCode(RESP_Y_LOCATE_ERR);
	}
	if((x_pos >= X_POS_MAX) || (y_pos >= Y_POS_MAX))		/* para error */
	{
		return SetRespCode(RESP_CMD_PARA_ERR);
	}
	if((!Sensor_Status(S_Z_REAR)) && (!Sensor_Status(S_Z_FRONT)))	/* z is not in pos, reset */
	{
		if(!Actions_ZMoveRear(FULL_SPEED))
		{
			return SetRespCode(RESP_Z_MOVE_REAR_ERR);
		}
	}
	/* 检查盘子是否伸出 */
	if(Sensor_Status(S_X0_TEST) || Sensor_Status(S_X1_TEST) || Sensor_Status(S_X2_TEST))
	{
		return SetRespCode(RESP_BOX_NOT_IN_HOME);
	}
	/* (x,y) is in reset pos, reset again to confirm. */
	if((x_pos == 0) && (y_pos == 0))
	{
		return Actions_Reset(0);
	}
	/* position is lost, reset to (0,0) */
	if( ((Sensor_Status(S_X_LOCATE) && Sensor_Status(S_Y_LOCATE)) == 0) || \
		((settings.settings_tmp.actions_data.x_pos_cur == 0) && (!Sensor_Status(S_X_RESET))) || \
		((settings.settings_tmp.actions_data.y_pos_cur == 0) && (!Sensor_Status(S_Y_RESET))) || \
		((settings.settings_tmp.actions_data.x_pos_cur != 0) && (Sensor_Status(S_X_RESET))) || \
		((settings.settings_tmp.actions_data.y_pos_cur != 0) && (Sensor_Status(S_Y_RESET))) )
	{
		if(!Actions_Reset(1))
			return 0;
	}
	/* calculate the direction and distance */
	ACTIONS_DATA.x_pos_to_stop = x_pos;
	ACTIONS_DATA.y_pos_to_stop = y_pos;
	if(ACTIONS_DATA.x_pos_to_stop > ACTIONS_DATA.x_pos_cur)
	{
		x_steps = (ACTIONS_DATA.x_pos_to_stop - ACTIONS_DATA.x_pos_cur) * X_POS_DISTANCE_STEPS;
		x_dir = X_FORWARD;
	}
	else
	{
		x_steps = (ACTIONS_DATA.x_pos_cur - ACTIONS_DATA.x_pos_to_stop) * X_POS_DISTANCE_STEPS;
		x_dir = X_BACKWARD;
	}
	
	if(ACTIONS_DATA.y_pos_to_stop > ACTIONS_DATA.y_pos_cur)
	{
		y_steps = Actions_YStepsCalc(ACTIONS_DATA.y_pos_to_stop, ACTIONS_DATA.y_pos_cur);
		y_dir = Y_FORWARD;
	}
	else
	{
		y_steps = Actions_YStepsCalc(ACTIONS_DATA.y_pos_cur, ACTIONS_DATA.y_pos_to_stop);
		y_dir = Y_BACKWARD;
	}
	/* actions */
	if(y_steps)		/* y move */
	{
		ACTIONS_DATA.y_run_steps = 0;
		y_steps += mm_to_step_y(15);
		Sensor_IntEnable(S_Y_LOCATE, Actions_YPosInt, EXTI_TRIGGER_MODE_Y_LOCATE, ENABLE);
		Motor_Move(MOTOR_Y, y_steps, y_micro_steps, y_dir, MOTORY_FULL_SPEED);
		if(ACTIONS_DATA.y_pos_cur >= 12)
		{
			delay_ms(200);
		}
	}
	if(x_steps)		/* x move */
	{
		ACTIONS_DATA.x_run_steps = 0;
		x_steps += mm_to_step_x(15);
		Sensor_IntEnable(S_X_LOCATE, Actions_XPosInt, EXTI_TRIGGER_MODE_X_LOCATE, ENABLE);
		Motor_Move(MOTOR_X, x_steps, x_micro_steps, x_dir, MOTORX_FULL_SPEED);
	}
	while(1)		//如果x先停止了，x先锁死，如果y先停了，y先锁死，防止往下掉
	{
		if(Motor_IsStoped(MOTOR_X))
		{
			Sensor_IntEnable(S_X_LOCATE, Actions_XPosInt, EXTI_TRIGGER_MODE_X_LOCATE, DISABLE);
			Motor_Hold(MOTOR_X, ENABLE);
			break;
		}
		if(Motor_IsStoped(MOTOR_Y))
		{
			Sensor_IntEnable(S_Y_LOCATE, Actions_YPosInt, EXTI_TRIGGER_MODE_Y_LOCATE, DISABLE);
			Motor_Hold(MOTOR_Y, ENABLE);
			break;
		}
	}
	Motor_WaitStoped(MOTOR_X);
	Sensor_IntEnable(S_X_LOCATE, Actions_XPosInt, EXTI_TRIGGER_MODE_X_LOCATE, DISABLE);
	Motor_Hold(MOTOR_X, ENABLE);
	Motor_WaitStoped(MOTOR_Y);
	Sensor_IntEnable(S_Y_LOCATE, Actions_YPosInt, EXTI_TRIGGER_MODE_Y_LOCATE, DISABLE);
	Motor_Hold(MOTOR_Y, ENABLE);
	/* confirm the result */
	if(Sensor_Status(S_X_LOCATE) && Sensor_Status(S_Y_LOCATE))
	{
		ACTIONS_DATA.x_pos_cur = ACTIONS_DATA.x_pos_to_stop;
		ACTIONS_DATA.y_pos_cur = ACTIONS_DATA.y_pos_to_stop;
		ret = 1;
	}
	else if(!Sensor_Status(S_X_LOCATE))
	{
		ret = SetRespCode(RESP_X_LOCATE_ERR);
	}
	else
	{
		ret = SetRespCode(RESP_Y_LOCATE_ERR);
	}
	return ret;
}

unsigned char Actions_XYMove(uint8_t x_pos, uint8_t y_pos)
{
	unsigned char ret = 0;

	/* position is lost, reset to (0,0) */
	if( ((Sensor_Status(S_X_LOCATE) && Sensor_Status(S_Y_LOCATE)) == 0) || \
		((settings.settings_tmp.actions_data.x_pos_cur == 0) && (!Sensor_Status(S_X_RESET))) || \
		((settings.settings_tmp.actions_data.y_pos_cur == 0) && (!Sensor_Status(S_Y_RESET))) || \
		((settings.settings_tmp.actions_data.x_pos_cur != 0) && (Sensor_Status(S_X_RESET))) || \
		((settings.settings_tmp.actions_data.y_pos_cur != 0) && (Sensor_Status(S_Y_RESET))) )
	{
		if(!Actions_ScannerDown())
			return 0;
		if(!Actions_Reset(1))
			return 0;
		if((x_pos == 0) && (y_pos == 0))
		{
			return 1;
		}
	}
	if(y_pos > Y_POS_BARCODE_MAX_SCANNER_FOLLOW)				//高于11扫描头下降
	{
		if(!Actions_ScannerDown())
			return 0;
	}
	if((settings.settings_tmp.actions_data.y_pos_cur == Y_POS_OUT) && (y_pos != settings.settings_tmp.actions_data.y_pos_cur))
	{
		Motor_Move(MOTOR_Y, Y_POS_DISTANCE_STEPS_OUT, 0, Y_BACKWARD, MOTORY_FULL_SPEED);		//2. 到(x_out, y_out)
		Motor_WaitStoped(MOTOR_Y);
		Motor_Hold(MOTOR_Y, ENABLE);
		if(Sensor_Status(S_X_LOCATE) && Sensor_Status(S_Y_LOCATE))
		{
			settings.settings_tmp.actions_data.y_pos_cur = 13;
		}
	}

	//不需要移动，也进入do，会进行一次位置确认
	if((settings.settings_tmp.actions_data.x_pos_cur == x_pos) &&
		(settings.settings_tmp.actions_data.x_pos_cur == y_pos))
	{
		ret = Actions_XYMoveDo(x_pos, y_pos);
		if(!ret)
			return ret;
	}
	//送到客户取走区
	else if(y_pos == Y_POS_OUT)
	{
		if((settings.settings_tmp.actions_data.x_pos_cur == 1) &&
			(settings.settings_tmp.actions_data.y_pos_cur == 0))
		{
			ret = Actions_XYMoveDo(1, 1);			//1. 到(1,1)
			if(!ret)
				return ret;
		}
		ret = Actions_XYMoveDo(X_POS_OUT, 11);		//1. 到(x_out, 11)
		if(!ret)
			return ret;
		ret = Actions_XYMoveDo(X_POS_OUT, 13);		//2. 到(x_out, 13)
		if(!ret)
			return ret;
		Motor_Move(MOTOR_Y, Y_POS_DISTANCE_STEPS_OUT, 0, Y_FORWARD, MOTORY_FULL_SPEED);		//3. 到(x_out, y_out)
		Motor_WaitStoped(MOTOR_Y);
		Motor_Hold(MOTOR_Y, ENABLE);
		if(Sensor_Status(S_X_LOCATE) && Sensor_Status(S_Y_LOCATE))
		{
			settings.settings_tmp.actions_data.y_pos_cur = Y_POS_OUT;
		}
	}
	//当前位置为(1,0)时，避开皮带
	else if((settings.settings_tmp.actions_data.x_pos_cur != x_pos) &&
			(settings.settings_tmp.actions_data.x_pos_cur == 1) &&
			(settings.settings_tmp.actions_data.y_pos_cur == 0) &&
			(y_pos != 0))
	{
		ret = Actions_XYMoveDo(1, 1);			//1. 到(1,1)
		if(!ret)
			return ret;
		ret = Actions_XYMoveDo(x_pos, y_pos);	//2. 到(x, y)
		if(!ret)
			return ret;
	}
	//y=0时，第一排的移动需要避开皮带
	else if((settings.settings_tmp.actions_data.x_pos_cur != x_pos) &&
			(settings.settings_tmp.actions_data.y_pos_cur == 0)&&(y_pos == 0))
	{
		ret = Actions_XYMoveDo(settings.settings_tmp.actions_data.x_pos_cur, 1);	//1. 到(cur,1)
		if(!ret)
			return ret;
		ret = Actions_XYMoveDo(x_pos, 1);			//2. 到(x,1)
		if(!ret)
			return ret;
		ret = Actions_XYMoveDo(x_pos, y_pos);	//3. 到(x, y)
		if(!ret)
			return ret;
	}
	//目的位置为(1,0)时，避开皮带
	else if((x_pos == 1)&&(y_pos == 0))
	{
		ret = Actions_XYMoveDo(1, 1);			//1. 到(1,1)
		if(!ret)
			return ret;
		ret = Actions_XYMoveDo(x_pos, y_pos);	//2. 到(x, y)
		if(!ret)
			return ret;
	}
	//从最高位置平移需要避开传感器
	else if((settings.settings_tmp.actions_data.x_pos_cur != x_pos)
			&& (settings.settings_tmp.actions_data.y_pos_cur > 12))
	{
		ret = Actions_XYMoveDo(settings.settings_tmp.actions_data.x_pos_cur, 12);	//1. 到(x_cur_pos,12)
		if(!ret)
			return ret;
		ret = Actions_XYMoveDo(x_pos, y_pos);	//2. 到(x, y)
		if(!ret)
			return ret;
	}
	else
	{
		ret = Actions_XYMoveDo(x_pos, y_pos);
		if(!ret)
			return ret;
	}
	return ret;
}

unsigned char Actions_ZMove(t_zmove_action action, t_speed speed)
{
	unsigned char ret;
	unsigned char x, y;

	x = settings.settings_tmp.actions_data.x_pos_cur;
	y = settings.settings_tmp.actions_data.y_pos_cur;
	ret = 1;
	switch(action)
	{
		case ZMOVE_RESET:		//复位
			ret = Actions_ZMoveRear(speed);
			if(!ret)
			{
				break;
			}
			break;
		case ZMOVE_PUSH:		//推入
			if(!Sensor_Status(S_DETECT_BOX))	//box exsit
			{
				ret = SetRespCode(RESP_BOX_NOT_IN_CAR);
			}
			if(Sensor_Status(S_HOOK_UP))		//hook up
			{
				ret = Actions_HookUp();
				if(!ret)
				{
					break;
				}
			}
			ret = Actions_ZMoveFront(speed);			//z move front
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveToUnhook(HALF_SPEED);
			if(!ret)
			{
				break;
			}
			delay_ms(100);
			ret = Actions_HookResetDown();			//hook reset
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveRear(speed);			//z move back
			if(!ret)
			{
				break;
			}
			if(Sensor_Status(S_DETECT_BOX))		//dectect box
			{
				ret = SetRespCode(RESP_BOX_PUSH_ERR);
				break;
			}
			settings.settings_saved.pos_info[x][y].pos_box_is_take_away = 0;
			settings.settings_saved.machine_info.x_last_take_away = TAKE_AWAY_NONE;
			settings.settings_saved.machine_info.y_last_take_away = TAKE_AWAY_NONE;
			if(!Settings_Save())
			{
				return SetRespCode(RESP_WRITE_FLASH_ERR);
			}
			break;
		case ZMOVE_PUSH_NO_RETURN:		//z向推入后不撤回，钩子在前端
			if(!Sensor_Status(S_DETECT_BOX))	//box exsit
			{
				ret = SetRespCode(RESP_BOX_NOT_IN_CAR);
			}
			if(Sensor_Status(S_HOOK_UP))		//hook up
			{
				ret = Actions_HookUp();
				if(!ret)
				{
					break;
				}
			}
			ret = Actions_ZMoveFront(speed);			//z move front
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveToUnhook(HALF_SPEED);
			if(!ret)
			{
				break;
			}
			ret = Actions_HookResetDown();			//hook reset
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveHookToFront(speed);	//z move back to front position
			if(!ret)
			{
				break;
			}
			settings.settings_saved.pos_info[x][y].pos_box_is_take_away = 0;
			settings.settings_saved.machine_info.x_last_take_away = TAKE_AWAY_NONE;
			settings.settings_saved.machine_info.y_last_take_away = TAKE_AWAY_NONE;
			if(!Settings_Save())
			{
				return SetRespCode(RESP_WRITE_FLASH_ERR);
			}
			break;
		case ZMOVE_PULL:		//取出
			if(Sensor_Status(S_DETECT_BOX) && Sensor_Status(S_Z_REAR))	//box alread in car
			{
				ret = SetRespCode(RESP_BOX_ALREADY_IN_CAR);
				break;
			}
			ret = Actions_HookDown();			//hook reset
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveFront(speed);			//z move front
			if(!ret)
			{
				break;
			}
			if(!Sensor_Status(S_DETECT_BOX))	//dectect box
			{
				ret = SetRespCode(RESP_BOX_INVALID);
				break;
			}
			ret = Actions_ZMoveToHook(speed);
			if(!ret)
			{
				break;
			}
			ret = Actions_HookUp();				//hook up
			if(!ret)							//reset to hook up
			{
				SetRespCode(RESP_SUCCESS);
				ret = Actions_HookDownAfterUpFailed();
				if(!ret)
				{
					break;
				}
				ret = Actions_ZMoveHookToFront(speed);
				if(!ret)
				{
					break;
				}
				ret = Actions_HookResetDown();
				if(!ret)
				{
					break;
				}
				ret = Actions_ZMoveToHook(speed);
				if(!ret)
				{
					break;
				}
				ret = Actions_HookUp();				//hook up
				if(!ret)
				{
					break;
				}
			}
			ret = Actions_ZMoveRear(speed);			//z move rear
			if(!ret)
			{
				break;
			}
			if(!Sensor_Status(S_DETECT_BOX))	//dectect box
			{
				ret = SetRespCode(RESP_BOX_PULL_ERR);
				break;
			}
			settings.settings_saved.pos_info[x][y].pos_box_is_take_away = 1;
			settings.settings_saved.machine_info.x_last_take_away = x;
			settings.settings_saved.machine_info.y_last_take_away = y;
			if(!Settings_Save())
			{
				return SetRespCode(RESP_WRITE_FLASH_ERR);
			}
			break;
		case ZMOVE_PULL_AT_HALFWAY:		//z向半拉出，在未撤回位置拉回盒子
			if(Sensor_Status(S_DETECT_BOX) && Sensor_Status(S_Z_REAR))	//box alread in car
			{
				ret = SetRespCode(RESP_BOX_ALREADY_IN_CAR);
				break;
			}
			ret = Actions_HookDown();			//hook reset
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveFront(speed);			//z move front
			if(!ret)
			{
				break;
			}
			if(!Sensor_Status(S_DETECT_BOX))	//dectect box
			{
				ret = SetRespCode(RESP_BOX_INVALID);
				break;
			}
			ret = Actions_ZMoveToHook(speed);
			if(!ret)
			{
				break;
			}
			ret = Actions_HookUp();				//hook up
			if(!ret)							//reset to hook up
			{
				SetRespCode(RESP_SUCCESS);
				ret = Actions_HookDownAfterUpFailed();
				if(!ret)
				{
					break;
				}
				ret = Actions_ZMoveHookToFront(speed);
				if(!ret)
				{
					break;
				}
				ret = Actions_HookResetDown();
				if(!ret)
				{
					break;
				}
				ret = Actions_ZMoveToHook(speed);
				if(!ret)
				{
					break;
				}
				ret = Actions_HookUp();				//hook up
				if(!ret)
				{
					break;
				}
			}
			ret = Actions_ZMoveRear(speed);			//z move rear
			if(!ret)
			{
				break;
			}
			if(!Sensor_Status(S_DETECT_BOX))	//dectect box
			{
				ret = SetRespCode(RESP_BOX_PULL_ERR);
				break;
			}
			settings.settings_saved.pos_info[x][y].pos_box_is_take_away = 1;
			settings.settings_saved.machine_info.x_last_take_away = x;
			settings.settings_saved.machine_info.y_last_take_away = y;
			if(!Settings_Save())
			{
				return SetRespCode(RESP_WRITE_FLASH_ERR);
			}
			break;
		case ZMOVE_SEND:		//送出
			if(!Sensor_Status(S_DETECT_BOX))
			{
				ret = SetRespCode(RESP_BOX_NOT_IN_CAR);
				break;
			}
			if(!Actions_GateInOpen(1))
			{
				break;
			}
			if(!Sensor_Status(S_Z_REAR))
			{
				ret = Actions_ZMoveRear(speed);
				if(!ret)
				{
					break;
				}
			}
			if(!Sensor_Status(S_DETECT_BOX))
			{
				ret = SetRespCode(RESP_BOX_NOT_IN_CAR);
				break;
			}
			ret = Actions_XYMove(X_POS_OUT, Y_POS_OUT);
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveFront(speed);			//z move front
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveToUnhook(HALF_SPEED);
			if(!ret)
			{
				break;
			}
			break;
		case ZMOVE_SEND_BACK:		//送出后回收
			if(Sensor_Status(S_DETECT_BOX) && Sensor_Status(S_Z_REAR))	//box alread in car
			{
				ret = SetRespCode(RESP_BOX_ALREADY_IN_CAR);
				break;
			}
			if(!Actions_GateInOpen(1))
			{
				break;
			}
			if((x != X_POS_OUT) && (y != Y_POS_OUT))
			{
				ret = Actions_XYMove(X_POS_OUT, Y_POS_OUT);
				if(!ret)
				{
					break;
				}
			}
			if(!Sensor_Status(S_DETECT_BOX))
			{
				ret = Actions_ZMoveRear(speed);
				if(!ret)
				{
					break;
				}
				ret = Actions_ZMove(ZMOVE_PULL, speed);
				if(!ret)
				{
					break;
				}
			}
			else
			{
				if((Sensor_Status(S_Z_FRONT) || Sensor_Status(S_Z_REAR)) == 0)
				{
					ret = Actions_ZMoveHookToFront(speed);
					if(!ret)
					{
						ret = 1;
						ret = Actions_ZMoveRear(speed);
						if(!ret)
						{
							break;
						}
						ret = Actions_ZMove(ZMOVE_PULL, speed);
						if(!ret)
						{
							break;
						}
					}
					else
					{
						ret = Actions_ZMoveRear(speed);			//z move rear
						if(!ret)
						{
							break;
						}
						if(!Sensor_Status(S_DETECT_BOX))	//dectect box
						{
							ret = SetRespCode(RESP_BOX_PULL_ERR);
							break;
						}
					}
				}
				else
				{
					ret = Actions_ZMove(ZMOVE_PULL, speed);
					if(!ret)
					{
						break;
					}
				}
			}
			break;
		case ZMOVE_TO_DETECT_BOX:		//z向运动到检测盒子存在
			if(Sensor_Status(S_DETECT_BOX) && Sensor_Status(S_Z_REAR))	//box alread in car
			{
				ret = SetRespCode(RESP_BOX_ALREADY_IN_CAR);
				break;
			}
			if((!Sensor_Status(S_Z_FRONT)) && (!Sensor_Status(S_Z_REAR)))	//missed
			{
				ret = Actions_ZMoveRear(speed);
				if(!ret)
				{
					break;
				}
			}
			ret = Actions_ZMoveFront(speed);
			if(!ret)
			{
				break;
			}
			break;
		case ZMOVE_PUSH_TO_HOOK_POS:
			if(!Sensor_Status(S_DETECT_BOX))	//box exsit
			{
				ret = SetRespCode(RESP_BOX_NOT_IN_CAR);
			}
			if(Sensor_Status(S_HOOK_UP))		//hook up
			{
				ret = Actions_HookUp();
				if(!ret)
				{
					break;
				}
			}
			ret = Actions_ZMoveFront(speed);			//z move front
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveToUnhook(HALF_SPEED);
			if(!ret)
			{
				break;
			}
			settings.settings_saved.pos_info[x][y].pos_box_is_take_away = 0;
			settings.settings_saved.machine_info.x_last_take_away = TAKE_AWAY_NONE;
			settings.settings_saved.machine_info.y_last_take_away = TAKE_AWAY_NONE;
			if(!Settings_Save())
			{
				return SetRespCode(RESP_WRITE_FLASH_ERR);
			}
			break;
		case ZMOVE_PULL_FROM_HOOK_TO_REAR:
			if(Sensor_Status(S_DETECT_BOX) && Sensor_Status(S_Z_REAR))	//box alread in car
			{
				ret = SetRespCode(RESP_BOX_ALREADY_IN_CAR);
				break;
			}
			ret = Actions_HookUp();				//hook up
			if(!ret)
			{
				break;
			}
			ret = Actions_ZMoveRear(speed);			//z move rear
			if(!ret)
			{
				break;
			}
			if(!Sensor_Status(S_DETECT_BOX))	//dectect box
			{
				ret = SetRespCode(RESP_BOX_PULL_ERR);
				break;
			}
			settings.settings_saved.pos_info[x][y].pos_box_is_take_away = 1;
			settings.settings_saved.machine_info.x_last_take_away = x;
			settings.settings_saved.machine_info.y_last_take_away = y;
			if(!Settings_Save())
			{
				return SetRespCode(RESP_WRITE_FLASH_ERR);
			}
			break;
	}
	return ret;
}

/* 整个扫描动作一次完成，等待扫描结果才返回 */
unsigned char Actions_Scan(unsigned char * barcode_str)
{
	unsigned int len = 0;
	unsigned char ret = 0;

	/* test the package in box */
	if(!PackageInBox())
	{
		return SetRespCode(RESP_PACKAGE_NOT_IN_BOX);
	}
	/* move to scanner position */
	if(settings.settings_tmp.actions_data.y_pos_cur > Y_POS_BARCODE_MAX_SCANNER_FOLLOW)
	{
		if(!Actions_XYMove(settings.settings_tmp.actions_data.x_pos_cur, Y_POS_BARCODE_MAX_SCANNER_FOLLOW))
		{
			return 0;
		}
	}
	if(!Sensor_Status(S_SCANNER_UP))
	{
		if(!Actions_ScannerUp())
		{
			return 0;
		}
	}
	/* start scan */
	memset(barcode_str, 0, BARCODE_LEN);
	if(QRCodeScan_Read(QRCODE_SCAN_DEV, barcode_str, &len, SCAN_TIMEOUT_MS))
	{
		return SetRespCode(RESP_SCAN_FAIL);
	}
	else
	{
		if(len)
		{
			ret = 1;
		}
		else
		{
			ret = SetRespCode(RESP_SCAN_FAIL);
		}
	}
	
	return ret;
}

/* 备用扫描方式，移动到固定位置，利用顶部的扫描头进行扫描 */
unsigned char Actions_ScanStandbyTop(unsigned char * barcode_str)
{
	unsigned int len = 0;
	unsigned char ret = 0;

	/* test the package in box */
	if(!PackageInBox())
	{
		return SetRespCode(RESP_PACKAGE_NOT_IN_BOX);
	}
	/* move to scanner position */
	if((settings.settings_tmp.actions_data.x_pos_cur != X_POS_BARCODE)
		||(settings.settings_tmp.actions_data.y_pos_cur != Y_POS_BARCODE))
	{
		if(!Actions_XYMove(X_POS_BARCODE, Y_POS_BARCODE))
		{
			return 0;
		}
	}
	/* start scan */
	memset(barcode_str, 0, BARCODE_LEN);
	if(QRCodeScan_Read(QRCODE_SCAN_DEV_STANDBY_TOP, barcode_str, &len, SCAN_TIMEOUT_MS))
	{
		return SetRespCode(RESP_SCAN_FAIL);
	}
	else
	{
		if(len)
		{
			ret = 1;
		}
		else
		{
			ret = SetRespCode(RESP_SCAN_FAIL);
		}
	}
	
	return ret;
}

#if SCAN_STANDBY_CASE_CAR
/* 备用扫描方式，小车上的二号扫描头 */
unsigned char Actions_ScanStandbyCar(unsigned char * barcode_str)
{
	unsigned int len = 0;
	unsigned char ret = 0;

	/* test the package in box */
	if(!PackageInBox())
	{
		return SetRespCode(RESP_PACKAGE_NOT_IN_BOX);
	}
	/* start scan */
	memset(barcode_str, 0, BARCODE_LEN);
	if(QRCodeScan_Read(QRCODE_SCAN_DEV_STANDBY_CAR, barcode_str, &len, SCAN_TIMEOUT_MS))
	{
		return SetRespCode(RESP_SCAN_FAIL);
	}
	else
	{
		if(len)
		{
			ret = 1;
		}
		else
		{
			ret = SetRespCode(RESP_SCAN_FAIL);
		}
	}
	
	return ret;
}
#endif


/* 扫描动作分两次完成，中间可以进行其余操作 */
unsigned char Actions_StartScan(void)
{
	unsigned char ret = 0;
	unsigned char x, y;

	/* move to scanner position */
	if(settings.settings_tmp.actions_data.y_pos_cur > Y_POS_BARCODE_MAX_SCANNER_FOLLOW)
	{
		//find the empty position
		if((settings.settings_saved.machine_info.x_last_take_away != TAKE_AWAY_NONE) &&
		   (settings.settings_saved.machine_info.y_last_take_away != TAKE_AWAY_NONE) &&
		   (settings.settings_saved.machine_info.y_last_take_away <= Y_POS_BARCODE_MAX_SCANNER_FOLLOW))
		{
			x = settings.settings_saved.machine_info.x_last_take_away;
			y = settings.settings_saved.machine_info.y_last_take_away;
		}
		else
		{
			for(x = 0; x < X_POS_MAX; x++)
			{
				for(y = 0; y < Y_POS_MAX; y++)
				{
					if(settings.settings_saved.pos_info[x][y].pos_box_is_take_away)
					{
						break;
					}
				}
				if((y < Y_POS_MAX) && (settings.settings_saved.pos_info[x][y].pos_box_is_take_away))
				{
					break;
				}
			}
			if(x == X_POS_MAX)
			{
				SetRespCode(RESP_BOX_FULL);
				return 0;
			}
		}
		if(!Actions_XYMove(x, y))
		{
			return 0;
		}
	}
	if(!Sensor_Status(S_SCANNER_UP))
	{
		if(!Actions_ScannerUp())
		{
			return 0;
		}
	}
	/* start scan */
	if(QRCodeScan_StartScan(QRCODE_SCAN_DEV_0))
	{
		ret = SetRespCode(RESP_SCAN_COMM_FAIL);
	}
	else
	{
		ret = 1;
	}
	
	return ret;
}

/* 扫描动作分两次完成，中间可以进行其余操作，
   注意此函数不改变RESP返回值 */
unsigned char Actions_GetScanResult(unsigned char * barcode_str, unsigned int timeout_ms)
{
	unsigned int len = 0;
	unsigned char ret = 0;

	/* start scan */
	memset(barcode_str, 0, BARCODE_LEN);
	if(QRCodeScan_GetScanResult(QRCODE_SCAN_DEV_0, barcode_str, &len, timeout_ms))
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	
	return ret;
}

/* 1: open, 0: close */
unsigned char Actions_GateOpen(unsigned char open)
{
	char cmd_open[] =		{0x02, 'C', 'D', '1', 0x03};
	char cmd_close[] =		{0x02, 'C', 'D', '2', 0x03};
	char cmd_sensor[] =		{0x02, 'C', 'Q', '1', 0x03};
	char res_str[8];
	char * p_res_ch;
	unsigned int delay_time = 0;
	struct t_sen_st
	{
		unsigned char gate_close;
		unsigned char gate_open;
		unsigned char gate_anti_pinch;
	}sen_st;

	//sensor status
	delay_time = 50;
	while((Com_Receive_Ch(GATE_OPEN_COM, (uint8_t *)res_str))&&(delay_time--));	//clear data before
	Com_Send_Buffer(GATE_OPEN_COM, cmd_sensor, sizeof(cmd_sensor));
	delay_time = 500;
	memset(res_str, 0, sizeof(res_str));
	p_res_ch = res_str;
	while(delay_time)
	{
		if(Com_Receive_Ch(GATE_OPEN_COM, (uint8_t *)p_res_ch))
		{
			if(*p_res_ch == 0x03)
			{
				break;
			}
			p_res_ch++;
		}
		delay_ms(1);
		delay_time--;
	}
	if(delay_time > 0)
	{
		sen_st.gate_close = res_str[2]&(1<<0);
		sen_st.gate_open = res_str[2]&(1<<1);
		sen_st.gate_anti_pinch = ((res_str[2]&(1<<2))||(res_str[2]&(1<<3)));
	}
	else
	{
		return SetRespCode(RESP_GATE_COM_ERR);
	}
	if(sen_st.gate_anti_pinch)
	{
		return SetRespCode(RESP_GATE_ANTIPINCH_DETECT);
	}
	if(sen_st.gate_close && sen_st.gate_open)
	{
		return SetRespCode(RESP_GATE_SENSOR_ERR);
	}
	//open or close
	delay_time = 50;
	while((Com_Receive_Ch(GATE_OPEN_COM, (uint8_t *)res_str))&&(delay_time--));	//clear data before
	if(open)
	{
		Com_Send_Buffer(GATE_OPEN_COM, cmd_open, sizeof(cmd_open));
	}
	else
	{
		Com_Send_Buffer(GATE_OPEN_COM, cmd_close, sizeof(cmd_close));
	}
	delay_time = 20000;
	memset(res_str, 0, sizeof(res_str));
	p_res_ch = res_str;
	while(delay_time)
	{
		if(Com_Receive_Ch(GATE_OPEN_COM, (uint8_t *)p_res_ch))
		{
			if(*p_res_ch == 0x03)
			{
				break;
			}
			p_res_ch++;
		}
		delay_ms(1);
		delay_time--;
	}
	if((delay_time > 0) && (res_str[1] == 'P'))
	{
		return 1;
	}
	else
	{
		if(open)
		{
			return SetRespCode(RESP_GATE_OPEN_FAIL);
		}
		else
		{
			return SetRespCode(RESP_GATE_CLOSE_FAIL);
		}
	}
}

/* 1: open, 0: close */
unsigned char Actions_GateInOpen(unsigned char open)
{
	char cmd_open[] =		{0x02, 'C', 'E', '1', 0x03};
	char cmd_close[] =		{0x02, 'C', 'E', '2', 0x03};
	char cmd_sensor[] =		{0x02, 'C', 'Q', '1', 0x03};
	char res_str[8];
	char * p_res_ch;
	unsigned int delay_time = 0;
	struct t_sen_st
	{
		unsigned char gate_close;
		unsigned char gate_open;
		unsigned char gate_anti_pinch;
	}sen_st;

	//sensor status
	delay_time = 50;
	while((Com_Receive_Ch(GATE_OPEN_COM, (uint8_t *)res_str))&&(delay_time--));	//clear data before
	Com_Send_Buffer(GATE_OPEN_COM, cmd_sensor, sizeof(cmd_sensor));
	delay_time = 500;
	memset(res_str, 0, sizeof(res_str));
	p_res_ch = res_str;
	while(delay_time)
	{
		if(Com_Receive_Ch(GATE_OPEN_COM, (uint8_t *)p_res_ch))
		{
			if(*p_res_ch == 0x03)
			{
				break;
			}
			p_res_ch++;
		}
		delay_ms(1);
		delay_time--;
	}
	if(delay_time > 0)
	{
		sen_st.gate_close = res_str[2]&(1<<4);
		sen_st.gate_open = res_str[2]&(1<<5);
		sen_st.gate_anti_pinch = (res_str[2]&(1<<6));
	}
	else
	{
		return SetRespCode(RESP_GATE_COM_ERR);
	}
	if(sen_st.gate_anti_pinch)
	{
		return SetRespCode(RESP_GATE_ANTIPINCH_DETECT);
	}
	if(sen_st.gate_close && sen_st.gate_open)
	{
		return SetRespCode(RESP_GATE_SENSOR_ERR);
	}
	//open or close
	delay_time = 50;
	while((Com_Receive_Ch(GATE_OPEN_COM, (uint8_t *)res_str))&&(delay_time--));	//clear data before
	if(open)
	{
		Com_Send_Buffer(GATE_OPEN_COM, cmd_open, sizeof(cmd_open));
	}
	else
	{
		Com_Send_Buffer(GATE_OPEN_COM, cmd_close, sizeof(cmd_close));
	}
	delay_time = 20000;
	memset(res_str, 0, sizeof(res_str));
	p_res_ch = res_str;
	while(delay_time)
	{
		if(Com_Receive_Ch(GATE_OPEN_COM, (uint8_t *)p_res_ch))
		{
			if(*p_res_ch == 0x03)
			{
				break;
			}
			p_res_ch++;
		}
		delay_ms(1);
		delay_time--;
	}
	if((delay_time > 0) && (res_str[1] == 'P'))
	{
		return 1;
	}
	else
	{
		if(open)
		{
			return SetRespCode(RESP_GATE_OPEN_FAIL);
		}
		else
		{
			return SetRespCode(RESP_GATE_CLOSE_FAIL);
		}
	}
}

/* 更新二维码数据
  x_pos: x坐标
  y_pos：y坐标
  is_abnomal_recycle：是否异常回收，异常回收置1
  new_barcode：新的二维码
*/
unsigned char Actions_UpdateBarcode(unsigned char x_pos, unsigned char y_pos, unsigned char is_abnomal_recycle, unsigned char * new_barcode)
{
	memset(settings.barcodes_saved.barcode_info[x_pos][y_pos].last_barcode, 0, sizeof(settings.barcodes_saved.barcode_info[x_pos][y_pos].last_barcode));
	StringTailCutUnseeChar(settings.barcodes_saved.barcode_info[x_pos][y_pos].current_barcode);
	strcpy((char *)settings.barcodes_saved.barcode_info[x_pos][y_pos].last_barcode, (char *)settings.barcodes_saved.barcode_info[x_pos][y_pos].current_barcode);
	memset(settings.barcodes_saved.barcode_info[x_pos][y_pos].current_barcode, 0, sizeof(settings.barcodes_saved.barcode_info[x_pos][y_pos].current_barcode));
	if(new_barcode && strlen((char *)new_barcode))
	{
		StringTailCutUnseeChar(new_barcode);
		strcpy((char *)settings.barcodes_saved.barcode_info[x_pos][y_pos].current_barcode, (char *)new_barcode);
	}
	settings.settings_saved.pos_info[x_pos][y_pos].pos_abnomal_recycle = is_abnomal_recycle;
	if(!Settings_Save())
	{
		return SetRespCode(RESP_WRITE_FLASH_ERR);
	}
	if(!Settings_BarcodeSaveXY(x_pos, y_pos))
	//if(!Settings_BarcodeSaveAll())
	{
		return SetRespCode(RESP_WRITE_FLASH_ERR);
	}
	return 1;
}

unsigned char Actions_PosCanOpration(unsigned char x, unsigned char y)
{
	if(settings.settings_saved.pos_info[x][y].pos_valid == 0)
		return 0;
	if(y < 4)
	{
		if(Sensor_Status(S_BOX1))
			return 1;
	}
	else if((y >= 4) && (y < 8))
	{
		if(Sensor_Status(S_BOX2))
			return 1;
	}
	else if((y >= 8) && (y < 12))
	{
		if(Sensor_Status(S_BOX3))
			return 1;
	}
	return 0;
}

