/*
	ʵ�ʶ���
*/
#ifndef __ACTIONS_H__
#define __ACTIONS_H__
#include "stm32f10x.h"
#include "s_def.h"

typedef enum
{
	ZMOVE_RESET,			//z��λ
	ZMOVE_PUSH,				//z������
	ZMOVE_PUSH_NO_RETURN,	//z�������С�������أ�ͣ����ǰ��λ��
	ZMOVE_PULL,				//z������
	ZMOVE_PULL_AT_HALFWAY,	//z����δ����λ�����غ��ӣ���ǰ��λ����������
	ZMOVE_SEND,				//�͵��ͻ�ȡ������
	ZMOVE_SEND_BACK,		//�ͳ������
	ZMOVE_TO_DETECT_BOX,	//z���˶��������Ӵ���
	ZMOVE_PUSH_TO_HOOK_POS,			//z�����뵽��ȡλ�ã�����ɨ��
	ZMOVE_PULL_FROM_HOOK_TO_REAR,	//z��ӹ�ȡλ�ûص���ˣ�����ɨ��
}t_zmove_action;

typedef enum
{
	HALF_SPEED = 0,
	FULL_SPEED,
	HIGH_SPEED
}t_speed;

/* 0.05mm/step, 20step/mm */
#define mm_to_step_x(n_mm)				(n_mm*20)
/* 16/(4*600) mm/step, 150step/mm��16΢��/16 */
#define mm_to_step_y(n_mm)				(n_mm*75/2)
/* 15.92*3.1415/200mm/step, 4step/mm */
#define mm_to_step_z(n_mm)				(n_mm*4)
/* 18/45 degree/step, 45/18step/degree */
#define degree_to_step_hook(n_deg)		(n_deg*5/2)
/* 18/45 degree/step, 45/18step/degree */
#define degree_to_step_scanner(n_deg)	(n_deg*5/6)

#define MOTOR_MICRO_STEP_MODE			16						//16΢��

//����˶�������
#define X_FORWARD						MOTOR_DIR_BACKWARD
#define X_BACKWARD						MOTOR_DIR_FORWARD
#define Y_FORWARD						MOTOR_DIR_BACKWARD
#define Y_BACKWARD						MOTOR_DIR_FORWARD
#define Z_FORWARD						MOTOR_DIR_FORWARD
#define Z_BACKWARD						MOTOR_DIR_BACKWARD
#define HOOK_FORWARD					MOTOR_DIR_FORWARD
#define HOOK_BACKWARD					MOTOR_DIR_BACKWARD
#define SCAN_FORWARD					MOTOR_DIR_FORWARD
#define SCAN_BACKWARD					MOTOR_DIR_BACKWARD

/* X���˶����� */
#define MOTORX_FULL_SPEED				500
#define MOTORX_HALF_SPEED				948
#define X_POS_DISTANCE_STEPS			mm_to_step_x(146)		//һ��λ�þ���146mm
#define X_POS_DISTANCE_MICROSTEPS		0
#define X_RESET_DISTANCE_STEPS			(X_POS_DISTANCE_STEPS*3)
#define X_RESET_DISTANCE_MICROSTEPS		0
#define X_LOCATE_PART_WIDTH				mm_to_step_x(20)		//��Ƭ��20mm
#define X_RESET_AT_0_DISTANCE			(X_LOCATE_PART_WIDTH + mm_to_step_x(5))
#define X_LOCATE_STOP_FORWARD_AFTER_STEPS		(mm_to_step_x(10))
#define X_LOCATE_STOP_BACKWARD_AFTER_STEPS		(mm_to_step_x(10))
#define X_RESET_STOP_AFTER_STEPS		(mm_to_step_x(1)/2)	//8.5
#define X_STOP_AFTER_MICRO_STEPS		0
#define X_EDGE_POS_DISTANCE				mm_to_step_x(5)			/*x�����Ե�������*/

/* Y���˶����붨�� */
#define MOTORY_FULL_SPEED				280
#define MOTORY_HALF_SPEED				948
#define Y_POS_DISTANCE_STEPS			mm_to_step_y(38)		//һ��λ�þ���38mm
#define Y_POS_DISTANCE_MICROSTEPS		0
#define Y_POS_DISTANCE_STEPS_PLUS		mm_to_step_y(24)		//ÿ������֮�����߶�24
#define Y_POS_DISTANCE_STEPS_TOP_PLUS	(mm_to_step_y(227)/10)	//����֮�����߶�22.7
#define Y_POS_DISTANCE_STEPS_OUT		(mm_to_step_y(14))		//����������һ���߳�14.8mm
#define Y_RESET_DISTANCE_STEPS			(Y_POS_DISTANCE_STEPS*16)
#define Y_RESET_DISTANCE_MICROSTEPS		0
#define Y_LOCATE_PART_WIDTH				mm_to_step_y(20)		//��Ƭ��20mm
#define Y_RESET_AT_0_DISTANCE			(Y_LOCATE_PART_WIDTH + mm_to_step_y(5))
#define Y_LOCATE_UP_STOP_AFTER_STEPS	(mm_to_step_y(21)/2)
#define Y_LOCATE_DOWN_STOP_AFTER_STEPS	(mm_to_step_y(14)/2)
#define Y_RESET_STOP_AFTER_STEPS		(mm_to_step_y(3))
#define Y_STOP_AFTER_MICRO_STEPS		0
#define Y_EDGE_POS_DISTANCE				mm_to_step_y(7)			/*y�����Ե�������*/

/* Z���˶����� */
#define MOTORZ_HIGH_SPEED				1000
#define MOTORZ_FULL_SPEED				1666
#define MOTORZ_HALF_SPEED				1250
#define Z_DISTANCE_STEPS				mm_to_step_z(268)		//����238mm
#define Z_FRONT_TO_HOOK_STEPS			mm_to_step_z(22)
#define Z_FRONT_TO_UNHOOK_STEPS			mm_to_step_z(23)
#define Z_UNHOOK_BACK_STEPS				mm_to_step_z(1)
#define Z_DISTANCE_MICROSTEPS			0
#define Z_STOP_AFTER_STEPS				mm_to_step_z(2)
#define Z_STOP_AFTER_MICRO_STEPS		0

/* Hook�˶����� */
#define MOTOR_HOOK_FULL_SPEED			4000
#define HOOK_MOVE_DISTANCE_STEPS		degree_to_step_hook(15)	//15��
#define HOOK_RST_DISTANCE_STEPS			(HOOK_MOVE_DISTANCE_STEPS+degree_to_step_hook(5))
#define HOOK_RST_DISTANCE_MICROSTEPS	0
#define HOOK_STOP_AFTER_STEPS			degree_to_step_hook(3)
#define HOOK_STOP_AFTER_MICROSTEPS		0

/* Scanner�˶����� */
#define MOTOR_SCANNER_FULL_SPEED		5000
#define SCANNER_MOVE_DISTANCE_STEPS		(degree_to_step_scanner(113))	//113.3��
#define SCANNER_RST_DISTANCE_STEPS		(SCANNER_MOVE_DISTANCE_STEPS+degree_to_step_scanner(113))
#define SCANNER_RST_DISTANCE_MICROSTEPS	0
#define SCANNER_STOP_AFTER_STEPS		degree_to_step_scanner(5)
#define SCANNER_STOP_AFTER_MICROSTEPS	0

/* ��ά��ɨ��λ�� */
#define X_POS_BARCODE							1				//ɨ���ǲ���С����ʱ��С��ɨ��Xλ��
#define Y_POS_BARCODE							9				//ɨ���ǲ���С����ʱ��С��ɨ��Yλ��
#define Y_POS_BARCODE_MAX_SCANNER_FOLLOW		11				//ɨ������С����ʱ��С�����ɨ��λ��
/* send����λ�� */
#define X_POS_OUT					0
#define Y_POS_OUT					Y_POS_MAX

#define GATE_OPEN_COM				RFID_COM	//բ��ģ�鴮��

#define PackageInBox()		(Sensor_Status(S_PACK1)||Sensor_Status(S_PACK2)||Sensor_Status(S_PACK3)||Sensor_Status(S_PACK4))
#define PackageInTaking()	(Sensor_Status(S_PACK5)||Sensor_Status(S_PACK6)||Sensor_Status(S_PACK7)||Sensor_Status(S_PACK8))

void Actions_XPosInt(void);
void Actions_YPosInt(void);
void Actions_XResetInt(void);
void Actions_YResetInt(void);
void Actions_ZFrontInt(void);
void Actions_ZRearInt(void);
void Actions_HookUpInt(void);
void Actions_ScannerUpInt(void);
void Actions_Init(void);
unsigned char Actions_Reset(unsigned char move_out_reset);
unsigned char Actions_XYMove(uint8_t x_pos, uint8_t y_pos);
unsigned char Actions_ZMove(t_zmove_action action, t_speed speed);
unsigned char Actions_Scan(unsigned char * barcode_srt);
unsigned char Actions_StartScan(void);
unsigned char Actions_ScanStandbyTop(unsigned char * barcode_str);
unsigned char Actions_ScanStandbyCar(unsigned char * barcode_str);
unsigned char Actions_GetScanResult(unsigned char * barcode_str, unsigned int timeout_ms);
unsigned char Actions_GateOpen(unsigned char open);
unsigned char Actions_GateInOpen(unsigned char open);
unsigned char Actions_UpdateBarcode(unsigned char x_pos, unsigned char y_pos, unsigned char is_abnomal_recycle, unsigned char * new_barcode);
unsigned char Actions_PosCanOpration(unsigned char x, unsigned char y);

#endif
