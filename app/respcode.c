/*
  �����Ӧ���뷵�ػ�Ӧ��
*/
#include "s_def.h"
#include "string.h"
#include "respcode.h"
#include "digtube.h"

static t_resp_code_discription resp_code;

const t_resp_code_discription code_disc_list[RESP_CODE_MAX+1] =
{
	/************ ������Ӧ ************/
	{RESP_SUCCESS,				"00",	"SUCCESS",				"�ɹ�"},

	/******** �������쳣�ֽ�� ********/
	{RESP_SUCC_FAIL_BOUND,		"XX",	"SUCC_FAIL_BOUND",		"�������쳣�ֽ��"},

	/************ �쳣��Ӧ ************/
	{RESP_CMD_UNDEF,			"10",	"Undefine CMD",			"δ�����ָ��"},
	{RESP_CMD_PARA_ERR,			"11",	"CMD parament err",		"�����������"},
	{RESP_CMD_UNINIT,			"12",	"Not Init",				"û�г�ʼ��"},
	{RESP_READ_FLASH_ERR,		"13",	"Read flash err",		"��ȡFLASHʧ��"},
	{RESP_WRITE_FLASH_ERR,		"14",	"Write flash err",		"д��FLASHʧ��"},

	{RESP_X_RESET_ERR,			"20",	"X reset failed",		"X��λʧ��"},
	{RESP_X_LOCATE_ERR,			"21",	"X locate failed",		"X��λʧ��"},

	{RESP_Y_RESET_ERR,			"30",	"Y reset failed",		"Y��λʧ��"},
	{RESP_Y_LOCATE_ERR,			"31",	"Y locate failed",		"Y��λʧ��"},

	{RESP_Z_RESET_ERR,			"40",	"Z reset failed",		"Z��λʧ��"},
	{RESP_Z_LOCATE_ERR,			"41",	"Z locate failed",		"Z��λʧ��"},

	{RESP_BOX_RESET_ERR,		"50",	"Box reset failed",		"���Ӹ�λʧ��"},
	{RESP_BOX_PULL_ERR,			"51",	"Box pull failed",		"��������ʧ��"},
	{RESP_BOX_SEND_ERR,			"52",	"Box send failed",		"�����ͳ�ʧ��"},
	{RESP_BOX_PUSH_ERR, 		"53",	"Box push failed",		"�����ƻ�ʧ��"},
	{RESP_BOX_NOT_IN_HOME,		"54",	"Box is not in home",	"����δ�źã�������쵽"},
	{RESP_BOX_ALREADY_IN_CAR,	"55",	"Box alreay in car",	"С�����Ѿ��к���"},
	{RESP_BOX_NOT_IN_CAR,		"56",	"Box is not in car",	"С����û�к���"},
	{RESP_PACKAGE_NOT_IN_BOX,	"57",	"Package in not in box","������û�а���"},

	{RESP_SCAN_FAIL,			"60",	"Scan fail",			"��ά��ɨ��ʧ��"},
	{RESP_BARCODE_NEGATIVE,		"61",	"Barcode negative",		"�ö�ά�벻����"},		
	{RESP_BARCODE_SCAN_DIFF,	"62",	"Barcode is different with the pos",		"���յ��Ķ�ά����ȡ������Ʒ��ά�벻һ��"},
	{RESP_SCAN_COMM_FAIL,		"63",	"Scan communication fail",	"��ά��ģ��ͨѶʧ��"},

	{RESP_BOX_EMPTY,			"70",	"All boxes empty",		"���и����ѿ�"},
	{RESP_BOX_FULL,				"71",	"All boxes full",		"���и�������"},
	{RESP_BOX_INVALID,			"72",	"The box invalid",		"�ø��Ӳ�����"},
	{RESP_BOX_IS_PULLED,		"73",	"The box is pulled",	"�ø����Ѿ�������"},

	{RESP_GATE_OPEN_FAIL,		"80",	"Opening Gate is failed",	"����ʧ��"},
	{RESP_GATE_CLOSE_FAIL,		"81",	"Closing Gate is failed",	"����ʧ��"},
	{RESP_GATE_ANTIPINCH_DETECT,"82",	"Anti-pinch is detected in gate",	"��⵽������"},
	{RESP_GATE_SENSOR_ERR,		"82",	"Sensor of Gate error",	"բ�Ŵ������쳣"},
	{RESP_GATE_COM_ERR,			"83",	"Gate communication timeout or error",	"��բ��ͨѶ��ʱ���쳣"},

	{RESP_GATE_IN_OPEN_FAIL,	"85",	"Opening Gate IN is failed",	"������ʧ��"},
	{RESP_GATE_IN_CLOSE_FAIL,	"86",	"Closing Gate IN is failed",	"������ʧ��"},
	{RESP_GATE_IN_ANTIPINCH_DETECT,"87",	"Anti-pinch IN is detected in gate",	"��⵽�ڷ�����"},
	{RESP_GATE_IN_SENSOR_ERR,	"88",	"Sensor of Gate IN error",	"��բ�Ŵ������쳣"},

	{RESP_Z_MOVE_FRONT_ERR,		"90",	"z move front error.",	"Z���е�ǰ��ʧ��"},
	{RESP_Z_MOVE_REAR_ERR,		"91",	"z move rear error.",	"Z���е����ʧ��"},
	{RESP_Z_SENSOR_ERR,			"92",	"z sensor error.",		"Z����������"},
	{RESP_HOOK_UP_ERR,			"93",	"hook up error.",		"����̧��ʧ��"},
	{RESP_HOOK_DOWN_ERR,		"94",	"hook down error.",		"��������ʧ��"},
	{RESP_SCANNER_UP_ERR,		"95",	"scanner up error.",	"ɨ��ͷ̧��ʧ��"},
	{RESP_SCANNER_DOWN_ERR,		"96",	"scanner down error.",	"ɨ��ͷ����ʧ��"},

	{RESP_MOTORY_ALARM,			"97",	"y motor alarm.",		"Y��������"},
	{RESP_RECLYE_BOX_FULL,		"98",	"reclye box full.",		"����������"},

	{RESP_CODE_MAX,				"FF",	"Response code did not find",		"��Ӧ����δ�ҵ�"}
};

/*
	* @brief  ���û�Ӧ���룬�������������쳣
	* @param  ��Ӧ�Ļ�Ӧ����
	* @retval ������Ӧ����1
			  �쳣��Ӧ����0
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

//����ֵΪcode����������Ϊ��������ַ���
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

