/*
	执行命令
*/
#include "s_def.h"
#include "string.h"
#include "stdio.h"
#include "command.h"
#include "actions.h"
#include "respcode.h"
#include "parser.h"
#include "qrcode_scan.h"
#include "main.h"
#include "settings.h"
#include "sensor.h"
#include "com.h"

typedef struct
{
	unsigned char x;
	unsigned char y;
}recycle_xy_t;

const recycle_xy_t recycle_box_array[] =
{
	{1, 13},	{1,12},
	{2, 13},	{2,12}
};

void CMD_Q(void)
{
	switch(m_cmd.cmd[1])
	{
		case '0':
			CMD_Q0();
			break;
		case '1':
			CMD_Q1();
			break;
		case '2':
			CMD_Q2();
			break;
		case 'A':
			CMD_QA();
			break;
		case 'V':
			CMD_QV();
			break;
		case 't':		//quit protocal mode, console mode
			quit = 1;
			SetRespCode(RESP_SUCCESS);
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}

void CMD_G(void)
{
	switch(m_cmd.cmd[1])
	{
		case 'C':	//get responce code discription
			GetRespCodeDisc((char *)m_cmd.p_cmp, (char *)m_rpy.p_rdt);
			m_rpy.len_rdt = strlen((char *)m_rpy.p_rdt);
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}

//reset
void CMD_R(void)
{
	switch(m_cmd.cmd[1])
	{
		case '0':
			CMD_R0();
			break;
		case '1':
			CMD_R1();
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}

//box motion
void CMD_B(void)
{
	switch(m_cmd.cmd[1])
	{
		case '0':
			if(!Actions_ZMove(ZMOVE_RESET, HIGH_SPEED))
			{
				return;
			}
			break;
		case '1':
			if(!Actions_ZMove(ZMOVE_PULL, HIGH_SPEED))
			{
				return;
			}
			break;
		case '2':
			if(!Actions_ZMove(ZMOVE_SEND, HIGH_SPEED))
			{
				return;
			}
			break;
		case '3':
			if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
			{
				return;
			}
			break;
		case 'A':		//scan all box
			CMD_BA();
			break;
		case 'B':
			CMD_BB();
			break;
		case 'C':
			CMD_BC();
			break;
		case 'G':
			CMD_BG();
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}

void CMD_D(void)
{
	switch(m_cmd.cmd[1])
	{
		case '0':
			CMD_D0();
			break;
		case '1':
			CMD_D1();
			break;
		case '2':
			CMD_D2();
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}

void CMD_E(void)
{
	switch(m_cmd.cmd[1])
	{
		case 'A':
			CMD_EA();
			break;
		case 'B':
			CMD_EB();
			break;
		case 'C':
			CMD_EC();
			break;
		case 'D':
			CMD_ED();
			break;
		case 'E':
			CMD_EE();
			break;
		case 'F':
			CMD_EF();
			break;
		case 'G':
			CMD_EG();
			break;
		case 'H':
			CMD_EH();
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}

void CMD_X(void)
{
	switch(m_cmd.cmd[1])
	{
		case 'Y':
			CMD_XY();
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}

void CMD_S(void)
{
	switch(m_cmd.cmd[1])
	{
		case 'C':
			CMD_SC();
			break;
		case 'X':
			CMD_SX();
			break;
		case 'Y':
			CMD_SY();
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}

void CMD_d(void)
{
	switch(m_cmd.cmd[1])
	{
		case 'e':	//get responce code discription
			GetRespCodeDisc((char *)m_cmd.p_cmp, (char *)m_rpy.p_rdt);
			m_rpy.len_rdt = strlen((char *)m_rpy.p_rdt);
			break;
		default:
			SetRespCode(RESP_CMD_UNDEF);
			break;
	}
}


/****************二级命令函数*****************    */
/* Reset */
void CMD_R0(void)
{
	Actions_Reset(1);
}

/* Reset and clear all data */
void CMD_R1(void)
{
	Actions_Reset(1);
	Settings_ClearReset();
}

//(x,y) motion
void CMD_XY(void)
{
	unsigned char x, y;
	
	if((m_cmd.p_cmp[0] >= '0') & (m_cmd.p_cmp[0] <= '9'))
	{
		x = m_cmd.p_cmp[0]&0xF;
	}
	else	//a ~ f, A ~ F
	{
		x = 9 + m_cmd.p_cmp[0]&0xF;
	}
	if((m_cmd.p_cmp[1] >= '0') & (m_cmd.p_cmp[1] <= '9'))
	{
		y = m_cmd.p_cmp[1]&0xF;
	}
	else	//a ~ f, A ~ F
	{
		y = 9 + m_cmd.p_cmp[1]&0xF;
	}
//	if(!settings.settings_saved.pos_info[x][y].pos_valid)
//	{
//		SetRespCode(RESP_BOX_INVALID);
//	}
	if(!Actions_XYMove(x, y))
	{
		return;
	}
}

//scan qrcode for debug
void CMD_SC(void)
{
	unsigned int len = 0;

	if(QRCodeScan_Read(QRCODE_SCAN_DEV, m_rpy.p_rdt, &len, SCAN_TIMEOUT_MS))
	{
		SetRespCode(RESP_SCAN_FAIL);
	}
	else
	{
		if(len)
		{
			m_rpy.len_rdt = len;
		}
		else
		{
			m_rpy.len_rdt = 0;
			SetRespCode(RESP_SCAN_FAIL);
		}
	}
#if SCAN_STANDBY_CASE_CAR
	memset(barcode_str, 0, BARCODE_LEN);
	if(QRCodeScan_Read(QRCODE_SCAN_DEV_STANDBY_CAR, m_rpy.p_rdt, &len, SCAN_TIMEOUT_MS))
	{
		SetRespCode(RESP_SCAN_FAIL);
	}
	else
	{
		if(len)
		{
			m_rpy.len_rdt = len;
		}
		else
		{
			m_rpy.len_rdt = 0;
			SetRespCode(RESP_SCAN_FAIL);
		}
	}
#endif
#if SCAN_STANDBY_CASE_TOP
	memset(barcode_str, 0, BARCODE_LEN);
	if(QRCodeScan_Read(QRCODE_SCAN_DEV_STANDBY_TOP, m_rpy.p_rdt, &len, SCAN_TIMEOUT_MS))
	{
		SetRespCode(RESP_SCAN_FAIL);
	}
	else
	{
		if(len)
		{
			m_rpy.len_rdt = len;
		}
		else
		{
			m_rpy.len_rdt = 0;
			SetRespCode(RESP_SCAN_FAIL);
		}
	}
#endif
}

//设置X校准数值，单位0.1mm
void CMD_SX(void)
{
	short x_adj;

	x_adj = (m_cmd.p_cmp[1]&0xF) * 10 + (m_cmd.p_cmp[2]&0xF);
	if(m_cmd.p_cmp[0] == '-')
	{
		x_adj = -x_adj;
	}
	if(((settings.settings_saved.machine_info.x_pos_adjust + x_adj) > -64) &&
		((settings.settings_saved.machine_info.x_pos_adjust + x_adj) < 64))
	{
		settings.settings_saved.machine_info.x_pos_adjust += x_adj;
	}
	else
	{
		SetRespCode(RESP_CMD_PARA_ERR);
	}
}

//设置Y校准数值，单位0.1mm
void CMD_SY(void)
{
	short y_adj;

	y_adj = (m_cmd.p_cmp[1]&0xF) * 10 + (m_cmd.p_cmp[2]&0xF);
	if(m_cmd.p_cmp[0] == '-')
	{
		y_adj = -y_adj;
	}
	if(((settings.settings_saved.machine_info.x_pos_adjust + y_adj) > -64) &&
		((settings.settings_saved.machine_info.x_pos_adjust + y_adj) < 64))
	{
		settings.settings_saved.machine_info.x_pos_adjust += y_adj;
	}
	else
	{
		SetRespCode(RESP_CMD_PARA_ERR);
	}
}

//查询设备状态
void CMD_Q0(void)
{
	unsigned int sen_st = 0;

	sen_st = Sensor_AllStatus();
	if(!(sen_st & ((1<<S_X_LOCATE)|(1<<S_X_RESET))))
	{
		SetRespCode(RESP_X_LOCATE_ERR);
	}
	else if(!(sen_st & ((1<<S_Y_LOCATE)|(1<<S_Y_RESET))))
	{
		SetRespCode(RESP_Y_LOCATE_ERR);
	}
	else if(((sen_st & ((1<<S_Z_FRONT)|(1<<S_Z_REAR))) == 0) &&
			(settings.settings_tmp.actions_data.y_pos_cur != Y_POS_OUT))	//当在最顶时，钩子是伸出状态，不能检测钩子复位
	{
		SetRespCode(RESP_BOX_RESET_ERR);
	}
}

//获取设备信息，包括总计格子数量，二维码有效物品数量
void CMD_Q1(void)
{
	char str[8];

	m_rpy.len_rdt = 0;
	
	memset(str, 0, sizeof(str));
	sprintf(str, "%2d;", settings.settings_saved.machine_info.pos_cnt_vaild);
	memcpy(m_rpy.p_rdt + m_rpy.len_rdt, str, strlen(str));
	m_rpy.len_rdt += strlen(str);

	memset(str, 0, sizeof(str));
	sprintf(str, "%2d;", settings.settings_saved.machine_info.pos_cnt_invaild);
	memcpy(m_rpy.p_rdt + m_rpy.len_rdt, str, strlen(str));
	m_rpy.len_rdt += strlen(str);
	
	memset(str, 0, sizeof(str));
	sprintf(str, "%2d;", settings.settings_saved.machine_info.pos_cnt_all);
	memcpy(m_rpy.p_rdt + m_rpy.len_rdt, str, strlen(str));
	m_rpy.len_rdt += strlen(str);
	
	memset(str, 0, sizeof(str));
	sprintf(str, "%2d;", settings.settings_saved.machine_info.box_cnt_filled);
	memcpy(m_rpy.p_rdt + m_rpy.len_rdt, str, strlen(str));
	m_rpy.len_rdt += strlen(str);
	
	memset(str, 0, sizeof(str));
	sprintf(str, "%2d", settings.settings_saved.machine_info.box_cnt_filled_with_data);
	memcpy(m_rpy.p_rdt + m_rpy.len_rdt, str, strlen(str));
	m_rpy.len_rdt += strlen(str);
	
	CMD_Q0();
}

//查询传感器状态
void CMD_Q2(void)
{
	unsigned int sen_st = 0;
	unsigned char i;

	m_rpy.len_rdt = 0;
	sen_st = Sensor_AllStatus();
	for(i = 0; i < S_MAX; i++)
	{
		if(sen_st & (1 << i))
		{
			*(m_rpy.p_rdt + m_rpy.len_rdt) = '1';
		}
		else
		{
			*(m_rpy.p_rdt + m_rpy.len_rdt) = '0';
		}
		m_rpy.len_rdt++;
	}
}

//获取所有格子信息
void CMD_QA(void)
{
	unsigned char x, y, i;
	char str[16];

	m_rpy.len_rdt = 0;
	for(x = 0; x < X_POS_MAX; x++)
	{
		for(y = 0; y < Y_POS_MAX; y++)
		{
			i = 0;
			str[i++] = x + '0';
			if(y < 10)
			{
				str[i++] = y + '0';
			}
			else
			{
				str[i++] = y + 'A' - 10;
			}
			str[i++] = '-';
			if (settings.settings_saved.pos_info[x][y].pos_valid)
			{
				str[i++] = '1';
			}
			else
			{
				str[i++] = '0';
			}
			if (settings.settings_saved.pos_info[x][y].pos_large)
			{
				str[i++] = '1';
			}
			else
			{
				str[i++] = '0';
			}
			if (settings.settings_saved.pos_info[x][y].pos_filled)
			{
				str[i++] = '1';
			}
			else
			{
				str[i++] = '0';
			}
			if (settings.settings_saved.pos_info[x][y].pos_box_is_take_away)
			{
				str[i++] = '1';
			}
			else
			{
				str[i++] = '0';
			}
			if (settings.settings_saved.pos_info[x][y].pos_abnomal_recycle)
			{
				str[i++] = '1';
			}
			else
			{
				str[i++] = '0';
			}
			str[i++] = ';';
			memcpy(m_rpy.p_rdt + m_rpy.len_rdt, str, i);
			m_rpy.len_rdt += i;
		}
	}
}

//查询版本信息
void CMD_QV(void)
{
	char str[32];
	char product_str[] = PRODUCT_STR;
	char data_str[] = DATE_STR;

	memcpy(m_rpy.p_rdt + m_rpy.len_rdt, product_str, strlen(product_str));
	m_rpy.len_rdt += strlen(product_str);
	memset(str, 0, sizeof(str));
	sprintf(str, " v%1d.%1d%1d Date: ", VER_FW/100, (VER_FW/10)%10, (VER_FW%10));
	memcpy(m_rpy.p_rdt + m_rpy.len_rdt, str, strlen(str));
	m_rpy.len_rdt += strlen(str);
	memcpy(m_rpy.p_rdt + m_rpy.len_rdt, data_str, strlen(data_str));
	m_rpy.len_rdt += strlen(data_str);
}

//扫描设备内所有的二维码
void CMD_BA(void)
{
	unsigned char x = 0;
	int y = 0;
//	unsigned char str_resp[2];
	unsigned char str[BARCODE_LEN];
	unsigned char scan_success = 0;

	for(x = 0; x < X_POS_MAX; x++)
	{
		if((x % 2) == 0)	//当x为0,2时y从0 -> max，当x为1时y从max -> 0，y走之字形
		{
			y = 0;
		}
		else
		{
			y = Y_POS_ALL_SCAN_MAX - 1;
		}
		while((y < Y_POS_ALL_SCAN_MAX) && (y >= 0))
		{
			scan_success = 0;
			settings.settings_saved.pos_info[x][y].pos_valid = 1;
			if(!Actions_PosCanOpration(x,y))
			{
				if((x % 2) == 0)	//当x为0,2时y从0 ~ max，当x为1时y从max ~ 0，y走之字形
				{
					y++;
				}
				else
				{
					y--;
				}
				continue;
			}
			//move to the position
			if(!Actions_XYMove(x, y))
			{
				goto __exit__;
			}
			if(!Actions_ZMove(ZMOVE_TO_DETECT_BOX, HIGH_SPEED))
			{
				goto __exit__;
			}
			if(Sensor_Status(S_DETECT_BOX))
			{
				//判断大小托盘
				if(y > 0)
				{
					settings.settings_saved.pos_info[x][y - 1].pos_large = 0;
				}
				else
				{
					settings.settings_saved.pos_info[x][y].pos_large = 0;
				}
				//start scan
				if(!Actions_StartScan())
				{
					goto __exit__;
				}
				//pull
				if(!Actions_ZMove(ZMOVE_PULL_AT_HALFWAY, FULL_SPEED))
				{
					Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
					goto __exit__;
				}
				//get scan result
				if(PackageInBox())
				{
					settings.settings_saved.pos_info[x][y].pos_filled = 1;
					settings.settings_saved.pos_info[x][y].pos_abnomal_recycle = 0;
					if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
					{
						//扫描成功，当前barcode复制到last_barcode，更新current barcode
						Actions_UpdateBarcode(x, y, 0, str);
						scan_success = 1;
					}
					else
					{
						//扫描失败，当前barcode复制到last_barcode，current barcode置空
						Actions_UpdateBarcode(x, y, 0, NULL);
						scan_success = 0;
					}
				}
				else
				{
					settings.settings_saved.pos_info[x][y].pos_filled = 0;
					Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
				}
				//if(x,y) is moved, move to the position
				if((settings.settings_tmp.actions_data.x_pos_cur != x)
					|| (settings.settings_tmp.actions_data.y_pos_cur != y))
				{
					if(!Actions_XYMove(x, y))
					{
						goto __exit__;
					}
				}
				//push back
				if(PackageInBox())
				{
					if(scan_success == 0)		//拉出扫描未成功
					{
						//start scan
						if(!Actions_StartScan())
						{
							goto __exit__;
						}
						if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, FULL_SPEED))
						{
							settings.settings_saved.pos_info[x][y].pos_filled = 0;
							Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
							goto __exit__;
						}
						if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
						{
							Actions_UpdateBarcode(x, y, 0, str);
							scan_success = 1;
						}
						else
						{
							//扫描失败，重试，重新拉出并重新放回去
							if(!Actions_StartScan())
							{
								goto __exit__;
							}
							//pull
							if(!Actions_ZMove(ZMOVE_PULL_AT_HALFWAY, FULL_SPEED))
							{
								Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
								goto __exit__;
							}
							if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))	//成功，直接推回去
							{
								Actions_UpdateBarcode(x, y, 0, str);
								scan_success = 1;
								//push
								if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, HIGH_SPEED))
								{
									settings.settings_saved.pos_info[x][y].pos_filled = 0;
									goto __exit__;
								}
							}
							else														//失败，推回去并扫描
							{
								//start scan
								if(!Actions_StartScan())
								{
									goto __exit__;
								}
								//push
								if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, FULL_SPEED))
								{
									settings.settings_saved.pos_info[x][y].pos_filled = 0;
									Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
									goto __exit__;
								}
								if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
								{
									Actions_UpdateBarcode(x, y, 0, str);
									scan_success = 1;
								}
								else
								{
									Actions_UpdateBarcode(x, y, 0, NULL);
									scan_success = 0;
								}
							}
						}
					}
					else
					{
						if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, HIGH_SPEED))
						{
							settings.settings_saved.pos_info[x][y].pos_filled = 0;
							goto __exit__;
						}
					}
				}
				else
				{
					if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, HIGH_SPEED))
					{
						settings.settings_saved.pos_info[x][y].pos_filled = 0;
						goto __exit__;
					}
				}
			}
			else
			{
				//判断大小托盘(每个箱子最上一个只能为小托盘)
				if((y > 0) && ((y + 1) % 4 != 0))
				{
					settings.settings_saved.pos_info[x][y - 1].pos_large = 1;
				}
				else
				{
					settings.settings_saved.pos_info[x][y - 1].pos_large = 0;
				}
				settings.settings_saved.pos_info[x][y].pos_valid = 0;
				settings.settings_saved.pos_info[x][y].pos_filled = 0;
				//没有盒子时，清理掉二维码
				Actions_UpdateBarcode(x, y, 0, NULL);
			}
			if((x % 2) == 0)	//当x为0,2时y从0 ~ max，当x为1时y从max ~ 0，y走之字形
			{
				y++;
			}
			else
			{
				y--;
			}
		}
	}

__exit__:
	if(!Settings_Save())
	{
		SetRespCode(RESP_WRITE_FLASH_ERR);
	}
	CMD_SetBusySt(0);	//turn off busy status
}

//获取二维码对应的位置
void CMD_BB(void)
{
	unsigned int x = 0, y = 0;
	char str[BARCODE_LEN];
	unsigned char str_cmp = 1;

	memset(str, 0, sizeof(str));
	memcpy(str, m_cmd.p_cmp, m_cmd.len_cmp);
	m_rpy.len_rdt = 0;
	for(x = 0; x < X_POS_MAX; x++)
	{
		for(y = 0; y < Y_POS_MAX; y++)
		{
			if(!settings.settings_saved.pos_info[x][y].pos_filled)
				continue;
			if(!settings.settings_saved.pos_info[x][y].pos_valid)
				continue;
			str_cmp = strcmp((char *)settings.barcodes_saved.barcode_info[x][y].current_barcode, str);
			if(str_cmp == 0)
			{
				*(m_rpy.p_rdt+m_rpy.len_rdt) = x + '0';
				m_rpy.len_rdt++;
				if(y < 10)
				{
					*(m_rpy.p_rdt+m_rpy.len_rdt) = y + '0';
				}
				else
				{
					*(m_rpy.p_rdt+m_rpy.len_rdt) = y + 'A' - 10;
				}
				m_rpy.len_rdt++;
				break;
			}
		}
		if((y < Y_POS_MAX) && (str_cmp == 0))
		{
			break;
		}
	}
	if(x == X_POS_MAX)	//barcode nagative
	{
		SetRespCode(RESP_BARCODE_NEGATIVE);
		return;
	}
}

//将小车运动到二维码扫描位置，并扫描二维码，不进行其余动作
void CMD_BC(void)
{
	unsigned int x = 0, y = 0;
	unsigned char scan_success = 0;
	unsigned char str1[2];
	unsigned char str[BARCODE_LEN];
	unsigned int str_len;
	memset(str, 0, sizeof(str_len));
	m_rpy.len_rdt = 0;
	if(!PackageInBox())
	{
		SetRespCode(RESP_PACKAGE_NOT_IN_BOX);
		return;
	}
	//move to scan & scan
	if(Actions_Scan(str))
	{
		str_len = strlen((char *)str);
		memcpy((m_rpy.p_rdt+m_rpy.len_rdt), str, str_len);
		m_rpy.len_rdt += str_len;
		scan_success = 1;
	}
	if(GetRespCode(str1) == RESP_SCAN_FAIL)
	{
		SetRespCode(RESP_SUCCESS);
	}
	if(!scan_success)	//固定扫描失败，运动扫描
	{
		//find the empty position
		if((settings.settings_saved.machine_info.x_last_take_away != TAKE_AWAY_NONE) &&
		   (settings.settings_saved.machine_info.y_last_take_away < Y_POS_ALL_SCAN_MAX))
		{
			x = settings.settings_saved.machine_info.x_last_take_away;
			y = settings.settings_saved.machine_info.y_last_take_away;
		}
		else
		{
			for(x = 0; x < X_POS_MAX; x++)
			{
				for(y = 0; y < Y_POS_ALL_SCAN_MAX; y++)
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
				goto __exit__;
			}
		}
		//move to the empty position
		if(!Actions_XYMove(x, y))
		{
			goto __exit__;
		}
		//start scan
		if(!Actions_StartScan())
		{
			return;
		}
		//push
		if(!Actions_ZMove(ZMOVE_PUSH_TO_HOOK_POS, FULL_SPEED))
		{
			//失败时也必须清理掉扫描仪中的数据
			Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
			goto __exit__;
		}
		//get scan result
		if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
		{
			str_len = strlen((char *)str);
			memcpy((m_rpy.p_rdt+m_rpy.len_rdt), str, str_len);
			m_rpy.len_rdt += str_len;
			scan_success = 1;
		}
		else
		{
			scan_success = 0;
		}
		if(!scan_success)
		{
			//start scan
			if(!Actions_StartScan())
			{
				goto __exit__;
			}
			//pull back
			if(!Actions_ZMove(ZMOVE_PULL_FROM_HOOK_TO_REAR, FULL_SPEED))
			{
				//失败时也必须清理掉扫描仪中的数据
				Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
				goto __exit__;
			}
			//get scan result
			if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
			{
				str_len = strlen((char *)str);
				memcpy((m_rpy.p_rdt+m_rpy.len_rdt), str, str_len);
				m_rpy.len_rdt += str_len;
				scan_success = 1;
			}
			else
			{
				//扫描失败，重新推入后拉出
				scan_success = 0;
				//start scan
				if(!Actions_StartScan())
				{
					return;
				}
				//push
				if(!Actions_ZMove(ZMOVE_PUSH_TO_HOOK_POS, FULL_SPEED))
				{
					//失败时也必须清理掉扫描仪中的数据
					Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
					goto __exit__;
				}
				//get scan result
				if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
				{
					str_len = strlen((char *)str);
					memcpy((m_rpy.p_rdt+m_rpy.len_rdt), str, str_len);
					m_rpy.len_rdt += str_len;
					scan_success = 1;
					//pull back
					if(!Actions_ZMove(ZMOVE_PULL_FROM_HOOK_TO_REAR, HIGH_SPEED))
					{
						goto __exit__;
					}
				}
				else
				{
					scan_success = 0;
					//start scan
					if(!Actions_StartScan())
					{
						goto __exit__;
					}
					//pull back
					if(!Actions_ZMove(ZMOVE_PULL_FROM_HOOK_TO_REAR, FULL_SPEED))
					{
						//失败时也必须清理掉扫描仪中的数据
						Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
						goto __exit__;
					}
					//get scan result
					if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
					{
						str_len = strlen((char *)str);
						memcpy((m_rpy.p_rdt+m_rpy.len_rdt), str, str_len);
						m_rpy.len_rdt += str_len;
						scan_success = 1;
					}
					else
					{
						scan_success = 0;
						SetRespCode(RESP_SCAN_FAIL);
					}
				}
			}
		}
		else
		{
			//pull back
			if(!Actions_ZMove(ZMOVE_PULL_FROM_HOOK_TO_REAR, HIGH_SPEED))
			{
				goto __exit__;
			}
		}
	}
__exit__:
	if(!Settings_Save())
	{
		SetRespCode(RESP_WRITE_FLASH_ERR);
		return;
	}
}

extern void StringTailCutUnseeChar(unsigned char * str);

//获取所有二维码信息，每条二维码以CR LF(0x0D 0x0A)分隔
void CMD_BG(void)
{
	unsigned int x = 0, y = 0;
	unsigned int str_len = 0;
	unsigned char i = 0;
	char str[8];

	m_rpy.len_rdt = 0;
	for(x = 0; x < X_POS_MAX; x++)
	{
		for(y = 0; y < Y_POS_MAX; y++)
		{
			if(!settings.settings_saved.pos_info[x][y].pos_valid)
				continue;
			if(settings.settings_saved.pos_info[x][y].pos_filled)
			{
				i = 0;
				str[i++] = x + '0';
				if(y < 10)
				{
					str[i++] = y + '0';
				}
				else
				{
					str[i++] = y + 'A' - 10;
				}
				str[i++] = '-';
				memcpy((m_rpy.p_rdt+m_rpy.len_rdt), str, i);
				m_rpy.len_rdt += i;
				StringTailCutUnseeChar(settings.barcodes_saved.barcode_info[x][y].current_barcode);
				str_len = strlen((char *)settings.barcodes_saved.barcode_info[x][y].current_barcode);
				if(str_len > BARCODE_LEN)
				{
					str_len = 0;
				}
				memcpy((m_rpy.p_rdt+m_rpy.len_rdt), settings.barcodes_saved.barcode_info[x][y].current_barcode, str_len);
				m_rpy.len_rdt += str_len;
				*(m_rpy.p_rdt+m_rpy.len_rdt) = 0x0D;
				m_rpy.len_rdt++;
				*(m_rpy.p_rdt+m_rpy.len_rdt) = 0x0A;
				m_rpy.len_rdt++;
				if(settings.settings_saved.pos_info[x][y].pos_abnomal_recycle)
				{
					StringTailCutUnseeChar(settings.barcodes_saved.barcode_info[x][y].last_barcode);
					str_len = strlen((char *)settings.barcodes_saved.barcode_info[x][y].last_barcode);
					memcpy((m_rpy.p_rdt+m_rpy.len_rdt), settings.barcodes_saved.barcode_info[x][y].last_barcode, str_len);
					m_rpy.len_rdt += str_len;
				}
				*(m_rpy.p_rdt+m_rpy.len_rdt) = 0x0D;
				m_rpy.len_rdt++;
				*(m_rpy.p_rdt+m_rpy.len_rdt) = 0x0A;
				m_rpy.len_rdt++;
			}
		}
	}
}

//参数为二维码，将指定二维码的包取出，到预备区
void CMD_EA(void)
{
	unsigned int x = 0, y = 0;
	unsigned char str[BARCODE_LEN];
	unsigned char ret = 1;
	unsigned char scan_success = 0;

	memset(str, 0, sizeof(str));
	memcpy(str, m_cmd.p_cmp, m_cmd.len_cmp);
	//convert barcode to position
	for(x = 0; x < X_POS_MAX; x++)
	{
		for(y = 0; y < Y_POS_MAX; y++)
		{
			if(!settings.settings_saved.pos_info[x][y].pos_filled)
				continue;
			if(!settings.settings_saved.pos_info[x][y].pos_valid)
				continue;
			ret = memcmp((char *)settings.barcodes_saved.barcode_info[x][y].current_barcode, (char *)str, m_cmd.len_cmp);
			if(ret == 0)
			{
				break;
			}
		}
		if((y < Y_POS_MAX) && (ret == 0))
		{
			break;
		}
	}
	if(x == X_POS_MAX)	//barcode nagative
	{
		SetRespCode(RESP_BARCODE_NEGATIVE);
		return;
	}
	//move to (x,y)
	if(!Actions_XYMove(x, y))
	{
		goto __exit__;
	}
	//start scan
	if(!Actions_StartScan())
	{
		return;
	}
	//pull
	if(!Actions_ZMove(ZMOVE_PULL, FULL_SPEED))
	{
		//失败时也必须清理掉扫描仪中的数据
		Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
		goto __exit__;
	}
	//get scan result
	if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
	{
		scan_success = 1;
		ret = memcmp((char *)m_cmd.p_cmp, (char *)str, m_cmd.len_cmp);
		if(ret != 0)
		{
			SetRespCode(RESP_BARCODE_SCAN_DIFF);
			goto __exit__;
		}
	}
	else
	{
		//start scan
		if(!Actions_StartScan())
		{
			goto __exit__;
		}
		//push back
		if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, FULL_SPEED))
		{
			//失败时也必须清理掉扫描仪中的数据
			Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
			goto __exit__;
		}
		//get scan result
		if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
		{
			scan_success = 1;
			ret = memcmp((char *)m_cmd.p_cmp, (char *)str, m_cmd.len_cmp);
			if(ret != 0)
			{
				SetRespCode(RESP_BARCODE_SCAN_DIFF);
			}
		}
		else
		{
			scan_success = 0;
		}
		if(!scan_success)
		{
			//start scan
			if(!Actions_StartScan())
			{
				goto __exit__;
			}
			//pull back
			if(!Actions_ZMove(ZMOVE_PULL_AT_HALFWAY, FULL_SPEED))
			{
				//失败时也必须清理掉扫描仪中的数据
				Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
				goto __exit__;
			}
			//get scan result
			if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
			{
				scan_success = 1;
				ret = memcmp((char *)m_cmd.p_cmp, (char *)str, m_cmd.len_cmp);
				if(ret != 0)
				{
					SetRespCode(RESP_BARCODE_SCAN_DIFF);
					goto __exit__;
				}
			}
			else
			{
				scan_success = 0;
				SetRespCode(RESP_SCAN_FAIL);
			}
		}
		else
		{
			//pull back
			if(!Actions_ZMove(ZMOVE_PULL_AT_HALFWAY, HIGH_SPEED))
			{
				goto __exit__;
			}
		}
	}
__exit__:
	if(!Settings_Save())
	{
		SetRespCode(RESP_WRITE_FLASH_ERR);
		return;
	}
}

//将小车内的包发出，送到客户取走区
void CMD_EB(void)
{
	Actions_ZMove(ZMOVE_SEND, HIGH_SPEED);
}

//将客户区取走区的包回收到预备区
void CMD_EC(void)
{
	Actions_ZMove(ZMOVE_SEND_BACK, HIGH_SPEED);
}

//将预备区的小车内的包，如果有包，经过扫描后，自动送回到上一次发出的位置
void CMD_ED(void)
{
	unsigned int x = 0, y = 0;
	unsigned char str[BARCODE_LEN];
	unsigned char scan_success = 0;
	
	//find the empty position
	if((settings.settings_saved.machine_info.x_last_take_away != TAKE_AWAY_NONE) &&
	   (settings.settings_saved.machine_info.y_last_take_away != TAKE_AWAY_NONE))
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
			goto __exit__;
		}
	}
	if((settings.settings_tmp.actions_data.y_pos_cur == Y_POS_OUT) && (!Sensor_Status(S_Z_REAR)))
	{
		if(!Actions_ZMove(ZMOVE_SEND_BACK, HIGH_SPEED))
		{
			goto __exit__;
		}
	}
	if(PackageInBox())
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 1;
	}
	else
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
	}
	/* 当无包，不要扫描或者是想要放入的位置满足扫描条件时，直接移动到该位置
	   如果不满足扫描条件，在扫描函数内会移动到满足扫描条件位置 */
	if((!settings.settings_saved.pos_info[x][y].pos_filled) ||
	   (y <= Y_POS_BARCODE_MAX_SCANNER_FOLLOW))
	{
		//move to the empty position
		if(!Actions_XYMove(x, y))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
	}
	if(settings.settings_saved.pos_info[x][y].pos_filled)	//有包
	{
		//move to scan & scan 1
		if(!Actions_StartScan())
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
		//push & not return
		if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, FULL_SPEED))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			//失败时也必须清理掉扫描仪中的数据
			Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
			goto __exit__;
		}
		//move to scan & scan
		if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
		{
			Actions_UpdateBarcode(x, y, 0, str);
			scan_success = 1;
		}
		else
		{
			Actions_UpdateBarcode(x, y, 0, NULL);
			scan_success = 0;
		}
		if(!scan_success)		//扫描未成功
		{
			//move to scan & scan
			if(!Actions_StartScan())
			{
				
			}
			//pull
			if(!Actions_ZMove(ZMOVE_PULL_AT_HALFWAY, FULL_SPEED))
			{
				settings.settings_saved.pos_info[x][y].pos_filled = 0;
				//失败时也必须清理掉扫描仪中的数据
				Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
				goto __exit__;
			}
			//move to scan & scan
			if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
			{
				Actions_UpdateBarcode(x, y, 0, str);
				scan_success = 1;
			}
			else
			{
				Actions_UpdateBarcode(x, y, 0, NULL);
				scan_success = 0;
			}
			if(!scan_success)		//扫描未成功
			{
				//move to scan & scan
				if(!Actions_StartScan())
				{
					SetRespCode(RESP_SCAN_COMM_FAIL);
				}
				if(!Actions_ZMove(ZMOVE_PUSH, FULL_SPEED))
				{
					settings.settings_saved.pos_info[x][y].pos_filled = 0;
					//失败时也必须清理掉扫描仪中的数据
					Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
					goto __exit__;
				}
				//move to scan & scan
				if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
				{
					Actions_UpdateBarcode(x, y, 0, str);
					scan_success = 1;
				}
				else
				{
					Actions_UpdateBarcode(x, y, 0, NULL);
					//SetRespCode(RESP_SCAN_FAIL);
					scan_success = 0;
				}
			}
			else
			{
				//push & not return
				if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
				{
					settings.settings_saved.pos_info[x][y].pos_filled = 0;
					goto __exit__;
				}
			}
		}
		else
		{
			//push & not return
			if(!Actions_ZMove(ZMOVE_RESET, HIGH_SPEED))
			{
				settings.settings_saved.pos_info[x][y].pos_filled = 0;
				goto __exit__;
			}
		}
	}
	else
	{
		//push & not return
		if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
	}
	//放入前确认是否因为扫描移动了位置
	if((settings.settings_tmp.actions_data.x_pos_cur != x) || (settings.settings_tmp.actions_data.y_pos_cur != y))
	{
		//pull
		if(!Actions_ZMove(ZMOVE_PULL, HIGH_SPEED))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
		if(!Actions_XYMove(x, y))
		{
			goto __exit__;
		}
		//push & not return
		if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
	}
	if(settings.settings_saved.machine_info.box_cnt_filled < settings.settings_saved.machine_info.pos_cnt_vaild)
	{
		settings.settings_saved.machine_info.box_cnt_filled++;
	}
	if((settings.settings_saved.pos_info[x][y].pos_filled) && \
		(settings.settings_saved.machine_info.box_cnt_filled_with_data < settings.settings_saved.machine_info.pos_cnt_vaild))
	{
		settings.settings_saved.machine_info.box_cnt_filled_with_data++;
	}
__exit__:
	if(!Settings_Save())
	{
		SetRespCode(RESP_WRITE_FLASH_ERR);
		return;
	}
}

//将坐标(x,y)的包，取出到预备区
void CMD_EE(void)
{
	unsigned char x, y;
	
	if((m_cmd.p_cmp[0] >= '0') & (m_cmd.p_cmp[0] <= '9'))
	{
		x = m_cmd.p_cmp[0]&0xF;
	}
	else	//a ~ f, A ~ F
	{
		x = 9 + m_cmd.p_cmp[0]&0xF;
	}
	if((m_cmd.p_cmp[1] >= '0') & (m_cmd.p_cmp[1] <= '9'))
	{
		y = m_cmd.p_cmp[1]&0xF;
	}
	else	//a ~ f, A ~ F
	{
		y = 9 + m_cmd.p_cmp[1]&0xF;
	}
	if(!settings.settings_saved.pos_info[x][y].pos_valid)
	{
		SetRespCode(RESP_BOX_INVALID);
	}
	if(settings.settings_saved.pos_info[x][y].pos_box_is_take_away)
	{
		SetRespCode(RESP_BOX_INVALID);
	}
	if(!Actions_XYMove(x, y))
	{
		goto __exit__;
	}
	//pull
	if(!Actions_ZMove(ZMOVE_PULL, HIGH_SPEED))
	{
		goto __exit__;
	}
	if(settings.settings_saved.pos_info[x][y].pos_filled)
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
		settings.settings_saved.machine_info.box_cnt_filled_with_data--;
	}
	if(settings.settings_saved.machine_info.box_cnt_filled > 0)
	{
		settings.settings_saved.machine_info.box_cnt_filled--;
	}
__exit__:
	if(!Settings_Save())
	{
		SetRespCode(RESP_WRITE_FLASH_ERR);
		return;
	}
}

//将预备区的小车内的包，自动送回到上一次发出的位置，不经过扫描
void CMD_EF(void)
{
	unsigned int x = 0, y = 0;
	
	//find the empty position
	if((settings.settings_saved.machine_info.x_last_take_away != TAKE_AWAY_NONE) &&
	   (settings.settings_saved.machine_info.y_last_take_away != TAKE_AWAY_NONE))
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
			return;
		}
	}
	if((settings.settings_tmp.actions_data.y_pos_cur == Y_POS_OUT) && (!Sensor_Status(S_Z_REAR)))
	{
		if(!Actions_ZMove(ZMOVE_SEND_BACK, HIGH_SPEED))
		{
			goto __exit__;
		}
	}
	if(PackageInBox())
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 1;
	}
	else
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
	}
	//move to the empty position
	if(!Actions_XYMove(x, y))
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
		goto __exit__;
	}
	//push
	if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
		goto __exit__;
	}
	if(settings.settings_saved.machine_info.box_cnt_filled < settings.settings_saved.machine_info.pos_cnt_vaild)
	{
		settings.settings_saved.machine_info.box_cnt_filled++;
	}
	if((settings.settings_saved.pos_info[x][y].pos_filled) && \
		(settings.settings_saved.machine_info.box_cnt_filled_with_data < settings.settings_saved.machine_info.pos_cnt_vaild))
	{
		settings.settings_saved.machine_info.box_cnt_filled_with_data++;
	}
__exit__:
	if(!Settings_Save())
	{
		SetRespCode(RESP_WRITE_FLASH_ERR);
		return;
	}
}

//将预备区的小车内的包，经过扫描后，异常回收到原位置
void CMD_EG(void)
{
	unsigned int x = 0, y = 0;
	unsigned char str[BARCODE_LEN];
	unsigned char scan_success = 0;
	
	//find the empty position
	if((settings.settings_saved.machine_info.x_last_take_away != TAKE_AWAY_NONE) &&
	   (settings.settings_saved.machine_info.y_last_take_away != TAKE_AWAY_NONE))
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
			goto __exit__;
		}
	}
	if((settings.settings_tmp.actions_data.y_pos_cur == Y_POS_OUT) && (!Sensor_Status(S_Z_REAR)))
	{
		if(!Actions_ZMove(ZMOVE_SEND_BACK, HIGH_SPEED))
		{
			goto __exit__;
		}
	}
	if(PackageInBox())
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 1;
	}
	else
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
	}
	//move to the empty position
	if(!Actions_XYMove(x, y))
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
		goto __exit__;
	}
	if((settings.settings_saved.pos_info[x][y].pos_filled) && (!scan_success))	//有包
	{
		//move to scan & scan 1
		if(!Actions_StartScan())
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
		//push & not return
		if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, FULL_SPEED))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			//失败时也必须清理掉扫描仪中的数据
			Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
			goto __exit__;
		}
		//move to scan & scan
		if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
		{
			Actions_UpdateBarcode(x, y, 1, str);
			scan_success = 1;
		}
		else
		{
			Actions_UpdateBarcode(x, y, 1, NULL);
			scan_success = 0;
		}
		if(!scan_success)		//扫描未成功
		{
			//move to scan & scan
			if(!Actions_StartScan())
			{
				goto __exit__;
			}
			//pull
			if(!Actions_ZMove(ZMOVE_PULL_AT_HALFWAY, FULL_SPEED))
			{
				settings.settings_saved.pos_info[x][y].pos_filled = 0;
				//失败时也必须清理掉扫描仪中的数据
				Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
				goto __exit__;
			}
			//move to scan & scan
			if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
			{
				Actions_UpdateBarcode(x, y, 1, str);
				scan_success = 1;
			}
			else
			{
				Actions_UpdateBarcode(x, y, 1, NULL);
				scan_success = 0;
			}
			if(!scan_success)		//扫描未成功
			{
				//move to scan & scan
				if(!Actions_StartScan())
				{
					
				}
				if(!Actions_ZMove(ZMOVE_PUSH, FULL_SPEED))
				{
					settings.settings_saved.pos_info[x][y].pos_filled = 0;
					//失败时也必须清理掉扫描仪中的数据
					Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
					goto __exit__;
				}
				//move to scan & scan
				if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
				{
					Actions_UpdateBarcode(x, y, 1, str);
					scan_success = 1;
				}
				else
				{
					Actions_UpdateBarcode(x, y, 1, NULL);
					//SetRespCode(RESP_SCAN_FAIL);
					scan_success = 0;
				}
			}
			else
			{
				//push & not return
				if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
				{
					settings.settings_saved.pos_info[x][y].pos_filled = 0;
					goto __exit__;
				}
			}
		}
		else
		{
			//push & not return
			if(!Actions_ZMove(ZMOVE_RESET, HIGH_SPEED))
			{
				settings.settings_saved.pos_info[x][y].pos_filled = 0;
				goto __exit__;
			}
		}
	}
	else
	{
		//push & not return
		if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
	}
	if(settings.settings_saved.machine_info.box_cnt_filled < settings.settings_saved.machine_info.pos_cnt_vaild)
	{
		settings.settings_saved.machine_info.box_cnt_filled++;
	}
	if((settings.settings_saved.pos_info[x][y].pos_filled) && \
		(settings.settings_saved.machine_info.box_cnt_filled_with_data < settings.settings_saved.machine_info.pos_cnt_vaild))
	{
		settings.settings_saved.machine_info.box_cnt_filled_with_data++;
	}
__exit__:
	if(!Settings_Save())
	{
		SetRespCode(RESP_WRITE_FLASH_ERR);
		return;
	}
}

//将预备区的小车内的包，经过扫描后，异常回收到回收位置
void CMD_EH(void)
{
	unsigned char i = 0;
	const recycle_xy_t * rec_pos;
	unsigned int x = 0, y = 0;
	unsigned char str[BARCODE_LEN];
	unsigned char scan_success = 0;
	
	//find the empty position
	if((settings.settings_saved.machine_info.x_last_take_away != TAKE_AWAY_NONE) &&
	   (settings.settings_saved.machine_info.y_last_take_away != TAKE_AWAY_NONE))
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
			goto __exit__;
		}
	}
	if((settings.settings_tmp.actions_data.y_pos_cur == Y_POS_OUT) && (!Sensor_Status(S_Z_REAR)))
	{
		if(!Actions_ZMove(ZMOVE_SEND_BACK, HIGH_SPEED))
		{
			goto __exit__;
		}
	}
	if(PackageInBox())
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 1;
	}
	else
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
	}
	//move to the empty position
	if(!Actions_XYMove(x, y))
	{
		settings.settings_saved.pos_info[x][y].pos_filled = 0;
		goto __exit__;
	}
	if((settings.settings_saved.pos_info[x][y].pos_filled) && (!scan_success))	//有包
	{
		//move to scan & scan 1
		if(!Actions_StartScan())
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
		//push & not return
		if(!Actions_ZMove(ZMOVE_PUSH_NO_RETURN, FULL_SPEED))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			//失败时也必须清理掉扫描仪中的数据
			Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
			goto __exit__;
		}
		//move to scan & scan
		if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
		{
			Actions_UpdateBarcode(x, y, 1, str);
			scan_success = 1;
		}
		else
		{
			Actions_UpdateBarcode(x, y, 1, NULL);
			scan_success = 0;
		}
		if(!scan_success)		//扫描未成功
		{
			//move to scan & scan
			if(!Actions_StartScan())
			{
				goto __exit__;
			}
			//pull
			if(!Actions_ZMove(ZMOVE_PULL_AT_HALFWAY, FULL_SPEED))
			{
				settings.settings_saved.pos_info[x][y].pos_filled = 0;
				//失败时也必须清理掉扫描仪中的数据
				Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
				goto __exit__;
			}
			//move to scan & scan
			if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
			{
				Actions_UpdateBarcode(x, y, 1, str);
				scan_success = 1;
			}
			else
			{
				Actions_UpdateBarcode(x, y, 1, NULL);
				scan_success = 0;
			}
			if(!scan_success)		//扫描未成功
			{
				//move to scan & scan
				if(!Actions_StartScan())
				{
					
				}
				if(!Actions_ZMove(ZMOVE_PUSH, FULL_SPEED))
				{
					settings.settings_saved.pos_info[x][y].pos_filled = 0;
					//失败时也必须清理掉扫描仪中的数据
					Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS);
					goto __exit__;
				}
				//move to scan & scan
				if(Actions_GetScanResult(str, GET_SCAN_RESULT_TIMEOUT_MS))
				{
					Actions_UpdateBarcode(x, y, 1, str);
					scan_success = 1;
				}
				else
				{
					Actions_UpdateBarcode(x, y, 1, NULL);
					scan_success = 0;
				}
			}
			else
			{
				//push & not return
				if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
				{
					settings.settings_saved.pos_info[x][y].pos_filled = 0;
					goto __exit__;
				}
			}
		}
		else
		{
			//push & not return
			if(!Actions_ZMove(ZMOVE_RESET, HIGH_SPEED))
			{
				settings.settings_saved.pos_info[x][y].pos_filled = 0;
				goto __exit__;
			}
		}
	}
	else
	{
		//push & not return
		if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
		{
			settings.settings_saved.pos_info[x][y].pos_filled = 0;
			goto __exit__;
		}
	}
	//放入回收区
	for(i = 0; i < sizeof(recycle_box_array)/sizeof(recycle_xy_t); i++)
	{
		rec_pos = &recycle_box_array[i];
		//回收箱初始状态为不可用，装入回收盒后方为可用
		if(settings.settings_saved.pos_info[x][y].pos_large)
		{
			if( ((rec_pos->y + 1) < Y_POS_MAX) &&			//上面还有一个位置
				(settings.settings_saved.pos_info[rec_pos->x][rec_pos->y].pos_valid == 0) &&		//本位置空
				(settings.settings_saved.pos_info[rec_pos->x][rec_pos->y + 1].pos_valid == 0) )		//高一位置空
			{
				settings.settings_saved.pos_info[rec_pos->x][rec_pos->y].pos_large = 1;
				break;
			}
		}
		else
		{
			if(settings.settings_saved.pos_info[rec_pos->x][rec_pos->y].pos_valid == 0)		//本位置空
			{
				settings.settings_saved.pos_info[rec_pos->x][rec_pos->y].pos_large = 0;
				break;
			}
		}
	}
	if(i == sizeof(recycle_box_array)/sizeof(recycle_xy_t))
	{
		SetRespCode(RESP_RECLYE_BOX_FULL);
		goto __exit__;
	}
	//pull
	if(!Actions_ZMove(ZMOVE_PULL, HIGH_SPEED))
	{
		goto __exit__;
	}
	settings.settings_saved.pos_info[x][y].pos_filled = 0;
	//move to
	if(!Actions_XYMove(rec_pos->x, rec_pos->y))
	{
		goto __exit__;
	}
	if(PackageInBox())
	{
		settings.settings_saved.pos_info[rec_pos->x][rec_pos->y].pos_filled = 1;
	}
	//push
	if(!Actions_ZMove(ZMOVE_PUSH, HIGH_SPEED))
	{
		settings.settings_saved.pos_info[rec_pos->x][rec_pos->y].pos_filled = 0;
		goto __exit__;
	}
	settings.settings_saved.pos_info[rec_pos->x][rec_pos->y].pos_valid = 1;
	if(scan_success)
	{
		Actions_UpdateBarcode(x, y, 0, NULL);
		Actions_UpdateBarcode(rec_pos->x, rec_pos->y, 1, str);
	}
	else
	{
		Actions_UpdateBarcode(x, y, 0, NULL);
		Actions_UpdateBarcode(rec_pos->x, rec_pos->y, 1, NULL);
	}
	if(settings.settings_saved.machine_info.box_cnt_filled < settings.settings_saved.machine_info.pos_cnt_vaild)
	{
		settings.settings_saved.machine_info.box_cnt_filled++;
	}
	if((settings.settings_saved.pos_info[x][y].pos_filled) && \
		(settings.settings_saved.machine_info.box_cnt_filled_with_data < settings.settings_saved.machine_info.pos_cnt_vaild))
	{
		settings.settings_saved.machine_info.box_cnt_filled_with_data++;
	}
__exit__:
	if(!Settings_Save())
	{
		SetRespCode(RESP_WRITE_FLASH_ERR);
		return;
	}
}

//close
void CMD_D0(void)
{
	if(!Actions_GateOpen(0))
		return;
	Actions_GateInOpen(1);
}

//open
void CMD_D1(void)
{
	if(!Actions_GateInOpen(0))
		return;
	Actions_GateOpen(1);
}

//gate status
void CMD_D2(void)
{
	char cmd_sensor[] = {0x02, 'C', 'Q', '1', 0x03};
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
		SetRespCode(RESP_GATE_COM_ERR);
		return;
	}
	if(sen_st.gate_close && sen_st.gate_open)
	{
		SetRespCode(RESP_GATE_SENSOR_ERR);
		return;
	}
	m_rpy.len_rdt = 0;
	if(sen_st.gate_open)
	{
		*(m_rpy.p_rdt + m_rpy.len_rdt) = '1';
	}
	else
	{
		*(m_rpy.p_rdt + m_rpy.len_rdt) = '0';
	}
	m_rpy.len_rdt++;
	if(sen_st.gate_close)
	{
		*(m_rpy.p_rdt + m_rpy.len_rdt) = '1';
	}
	else
	{
		*(m_rpy.p_rdt + m_rpy.len_rdt) = '0';
	}
	m_rpy.len_rdt++;
	if(sen_st.gate_anti_pinch)
	{
		*(m_rpy.p_rdt + m_rpy.len_rdt) = '1';
	}
	else
	{
		*(m_rpy.p_rdt + m_rpy.len_rdt) = '0';
	}
	m_rpy.len_rdt++;
}


