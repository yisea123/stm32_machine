/*
  �����Ӧ���뷵�ػ�Ӧ��
*/

#ifndef __RESP_CODE_H__
#define __RESP_CODE_H__

#define CODE_DISC_STR_LEN		64

/* ��Ӧ������
   ��Ӧ�������respcode.c��code_disc_list[RESP_CODE_MAX+1]�ṹ�� */
typedef enum
{
	/************ ������Ӧ ************/
	RESP_SUCCESS = 0,		//����ִ�гɹ�

	RESP_SUCC_FAIL_BOUND,	//�������쳣��Ӧ�ָ���

	/************ �쳣��Ӧ ************/
	RESP_CMD_UNDEF,			//δ�����ָ��
	RESP_CMD_PARA_ERR,		//�����������
	RESP_CMD_UNINIT,		//�豸δ��ʼ��
	RESP_READ_FLASH_ERR,	//FLASH��ȡʧ��
	RESP_WRITE_FLASH_ERR,	//FLASHд��ʧ��

	RESP_X_RESET_ERR,
	RESP_X_LOCATE_ERR,
	RESP_Y_RESET_ERR,
	RESP_Y_LOCATE_ERR,
	RESP_Z_RESET_ERR,
	
	RESP_Z_LOCATE_ERR,
	RESP_BOX_RESET_ERR,
	RESP_BOX_PULL_ERR,
	RESP_BOX_SEND_ERR,
	RESP_BOX_PUSH_ERR,
	RESP_BOX_NOT_IN_HOME,
	RESP_BOX_ALREADY_IN_CAR,
	RESP_BOX_NOT_IN_CAR,
	RESP_PACKAGE_NOT_IN_BOX,

	RESP_SCAN_FAIL,
	RESP_BARCODE_NEGATIVE,
	RESP_BARCODE_SCAN_DIFF,
	RESP_SCAN_COMM_FAIL,

	RESP_BOX_EMPTY,
	RESP_BOX_FULL,
	RESP_BOX_INVALID,
	RESP_BOX_IS_PULLED,

	RESP_GATE_OPEN_FAIL,
	RESP_GATE_CLOSE_FAIL,
	RESP_GATE_ANTIPINCH_DETECT,
	RESP_GATE_SENSOR_ERR,
	RESP_GATE_COM_ERR,


	RESP_GATE_IN_OPEN_FAIL,
	RESP_GATE_IN_CLOSE_FAIL,
	RESP_GATE_IN_ANTIPINCH_DETECT,
	RESP_GATE_IN_SENSOR_ERR,

	RESP_Z_MOVE_FRONT_ERR,
	RESP_Z_MOVE_REAR_ERR,
	RESP_Z_SENSOR_ERR,
	RESP_HOOK_UP_ERR,
	RESP_HOOK_DOWN_ERR,
	RESP_SCANNER_UP_ERR,
	RESP_SCANNER_DOWN_ERR,
	RESP_MOTORY_ALARM,
	RESP_RECLYE_BOX_FULL,
	
	RESP_CODE_MAX
}t_resp_code_enum;

typedef struct
{
	t_resp_code_enum code;			//��Ӧ��������
	char code_str[3];				//���ֽڻ�Ӧ���룬3����Ϊ��һ��'\0'������
	char code_disc_str_en[CODE_DISC_STR_LEN];		//��Ӧ�����������Ӣ��
	char code_disc_str_cn[CODE_DISC_STR_LEN];		//��Ӧ���������������
}t_resp_code_discription;

unsigned char SetRespCode(t_resp_code_enum code);
t_resp_code_enum GetRespCode(unsigned char * code_str);
unsigned char GetRespCodeDisc(char * code_str, char * code_disc_str);

#endif
