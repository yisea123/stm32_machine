/*
  保存回应码与返回回应码
*/
#include "s_def.h"
#include "string.h"
#include "respcode.h"
#include "digtube.h"

static t_resp_code_discription resp_code;

const t_resp_code_discription code_disc_list[RESP_CODE_MAX+1] =
{
	/************ 正常回应 ************/
	{RESP_SUCCESS,				"00",	"SUCCESS",				"成功"},

	/******** 正常与异常分解符 ********/
	{RESP_SUCC_FAIL_BOUND,		"XX",	"SUCC_FAIL_BOUND",		"正常与异常分解符"},

	/************ 异常回应 ************/
	{RESP_CMD_UNDEF,			"10",	"Undefine CMD",			"未定义的指令"},
	{RESP_CMD_PARA_ERR,			"11",	"CMD parament err",		"命令参数错误"},
	{RESP_CMD_UNINIT,			"12",	"Not Init",				"没有初始化"},
	{RESP_READ_FLASH_ERR,		"13",	"Read flash err",		"读取FLASH失败"},
	{RESP_WRITE_FLASH_ERR,		"14",	"Write flash err",		"写入FLASH失败"},

	{RESP_X_RESET_ERR,			"20",	"X reset failed",		"X复位失败"},
	{RESP_X_LOCATE_ERR,			"21",	"X locate failed",		"X定位失败"},

	{RESP_Y_RESET_ERR,			"30",	"Y reset failed",		"Y复位失败"},
	{RESP_Y_LOCATE_ERR,			"31",	"Y locate failed",		"Y定位失败"},

	{RESP_Z_RESET_ERR,			"40",	"Z reset failed",		"Z复位失败"},
	{RESP_Z_LOCATE_ERR,			"41",	"Z locate failed",		"Z定位失败"},

	{RESP_BOX_RESET_ERR,		"50",	"Box reset failed",		"盒子复位失败"},
	{RESP_BOX_PULL_ERR,			"51",	"Box pull failed",		"盒子拉出失败"},
	{RESP_BOX_SEND_ERR,			"52",	"Box send failed",		"盒子送出失败"},
	{RESP_BOX_PUSH_ERR, 		"53",	"Box push failed",		"盒子推回失败"},
	{RESP_BOX_NOT_IN_HOME,		"54",	"Box is not in home",	"托盘未放好，伸出被检到"},
	{RESP_BOX_ALREADY_IN_CAR,	"55",	"Box alreay in car",	"小车内已经有盒子"},
	{RESP_BOX_NOT_IN_CAR,		"56",	"Box is not in car",	"小车内没有盒子"},
	{RESP_PACKAGE_NOT_IN_BOX,	"57",	"Package in not in box","盒子内没有包裹"},

	{RESP_SCAN_FAIL,			"60",	"Scan fail",			"二维码扫描失败"},
	{RESP_BARCODE_NEGATIVE,		"61",	"Barcode negative",		"该二维码不存在"},		
	{RESP_BARCODE_SCAN_DIFF,	"62",	"Barcode is different with the pos",		"接收到的二维码与取出的物品二维码不一致"},
	{RESP_SCAN_COMM_FAIL,		"63",	"Scan communication fail",	"二维码模块通讯失败"},

	{RESP_BOX_EMPTY,			"70",	"All boxes empty",		"所有格子已空"},
	{RESP_BOX_FULL,				"71",	"All boxes full",		"所有格子已满"},
	{RESP_BOX_INVALID,			"72",	"The box invalid",		"该格子不可用"},
	{RESP_BOX_IS_PULLED,		"73",	"The box is pulled",	"该格子已经被拉出"},

	{RESP_GATE_OPEN_FAIL,		"80",	"Opening Gate is failed",	"开门失败"},
	{RESP_GATE_CLOSE_FAIL,		"81",	"Closing Gate is failed",	"关门失败"},
	{RESP_GATE_ANTIPINCH_DETECT,"82",	"Anti-pinch is detected in gate",	"检测到防夹手"},
	{RESP_GATE_SENSOR_ERR,		"82",	"Sensor of Gate error",	"闸门传感器异常"},
	{RESP_GATE_COM_ERR,			"83",	"Gate communication timeout or error",	"与闸门通讯超时或异常"},

	{RESP_GATE_IN_OPEN_FAIL,	"85",	"Opening Gate IN is failed",	"开内门失败"},
	{RESP_GATE_IN_CLOSE_FAIL,	"86",	"Closing Gate IN is failed",	"关内门失败"},
	{RESP_GATE_IN_ANTIPINCH_DETECT,"87",	"Anti-pinch IN is detected in gate",	"检测到内防夹手"},
	{RESP_GATE_IN_SENSOR_ERR,	"88",	"Sensor of Gate IN error",	"内闸门传感器异常"},

	{RESP_Z_MOVE_FRONT_ERR,		"90",	"z move front error.",	"Z运行到前端失败"},
	{RESP_Z_MOVE_REAR_ERR,		"91",	"z move rear error.",	"Z运行到后端失败"},
	{RESP_Z_SENSOR_ERR,			"92",	"z sensor error.",		"Z传感器错误"},
	{RESP_HOOK_UP_ERR,			"93",	"hook up error.",		"钩子抬起失败"},
	{RESP_HOOK_DOWN_ERR,		"94",	"hook down error.",		"钩子落下失败"},
	{RESP_SCANNER_UP_ERR,		"95",	"scanner up error.",	"扫描头抬起失败"},
	{RESP_SCANNER_DOWN_ERR,		"96",	"scanner down error.",	"扫描头落下失败"},

	{RESP_MOTORY_ALARM,			"97",	"y motor alarm.",		"Y轴电机报警"},
	{RESP_RECLYE_BOX_FULL,		"98",	"reclye box full.",		"回收箱已满"},

	{RESP_CODE_MAX,				"FF",	"Response code did not find",		"回应代码未找到"}
};

/*
	* @brief  设置回应代码，并返回正常或异常
	* @param  对应的回应代码
	* @retval 正常回应返回1
			  异常回应返回0
*/
unsigned char SetRespCode(t_resp_code_enum code)
{
	unsigned char ret = 0;
	
	resp_code.code = code;
	resp_code.code_str[0] = code_disc_list[code].code_str[0];
	resp_code.code_str[1] = code_disc_list[code].code_str[1];
	resp_code.code_str[2] = code_disc_list[code].code_str[2];
	if(resp_code.code <= RESP_SUCC_FAIL_BOUND)
	{
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	return ret;
}

//返回值为code索引，参数为错误代码字符串
t_resp_code_enum GetRespCode(unsigned char * code_str)
{
	code_str[0]= resp_code.code_str[0];
	code_str[1]= resp_code.code_str[1];
	return resp_code.code;
}

unsigned char GetRespCodeDisc(char * code_str, char * code_disc_str)
{
	unsigned int i;
	unsigned int code;
	unsigned int len;

	for(i = 0; i < RESP_CODE_MAX; i++)
	{
		if(memcmp(code_str, code_disc_list[i].code_str, 2) == 0)
		{
			code = i;
			memset(code_disc_str, 0, CODE_DISC_STR_LEN);
			strcpy(code_disc_str, code_disc_list[i].code_disc_str_en);
			len = strlen(code_disc_list[i].code_disc_str_en);
			*(code_disc_str + len) = '|';
			len++;
			strcpy(code_disc_str + len, code_disc_list[i].code_disc_str_cn);
			break;
		}
	}
	if(i == RESP_CODE_MAX)
	{
		code = i;
		memset(code_disc_str, 0, CODE_DISC_STR_LEN);
		strcpy(code_disc_str, code_disc_list[i].code_disc_str_en);
		len = strlen(code_disc_list[i].code_disc_str_en);
		*(code_disc_str + len) = '|';
		len++;
		strcpy(code_disc_str + len, code_disc_list[i].code_disc_str_cn);
	}
	
	return code;
}

