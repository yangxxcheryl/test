

#include <iom1280v.h>
#include <macros.h>
#include "B1404_LIB.h"
#include "LibCommon.h"


/*
�ӻ�ģ������ӿڣ�Ϊ�ϲ��ṩ���ƺ���
*/

/************************************** ������� *********************************************/
unsigned char  GetMotState(unsigned char slaveNum)
{
	return GetSlaveState(slaveNum);
}

unsigned char MotStop(unsigned char slaveNum)
{
	COMMAND_STRING command;
	if(slaveNum>=SLAVE_NUM)
		return 1;
//	if(GetSlaveState(slaveNum)==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
	command.cmd = (slaveNum << 3)  +  1;
	command.cmdLen =  1 + 1;
	command.pam[0] = CMD_MOT_STOP;
	InsertUrgentCommand(slaveNum, &command);
//		}
	return 0;
}

unsigned char MotRun(unsigned char slaveNum, signed int n){
	COMMAND_STRING command;
	unsigned char *pChar;
	signed int num;
	num = n;
	pChar = (unsigned char *)(&n);
	if(slaveNum>=SLAVE_NUM)
		return 1;
	SetMotPosIdle(slaveNum);
//	if(GetSlaveState(slaveNum)==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum<<3) + 3;
		command.cmdLen = 1+3;
		num = (unsigned int)n;
		command.pam[0] = CMD_MOT_RUN;
		command.pam[2] = *pChar++;
		command.pam[1] = *pChar;
		
		InsertCommand(slaveNum, &command);
//		}
/*	if(slaveNum == MOT_DILUENT)
		StartLiquidMonitor(0);
	else if(slaveNum == MOT_FLUID)
		StartLiquidMonitor(1);
	else if(slaveNum == MOT_EFFLUENT)
		StartLiquidMonitor(2);*/
	return 0;
}
unsigned char MotRunTo(unsigned char slaveNum, signed int x){
	COMMAND_STRING command;
	unsigned int num;
	if(slaveNum>=SLAVE_NUM)
		return 1;
	SetMotPosIdle(slaveNum);
//	if(GetSlaveState(slaveNum)==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum << 3) + 3;
		command.cmdLen = 1+3;
		num = (unsigned int)x;
		command.pam[0] = CMD_MOT_RUN_TO;
		command.pam[1] = (unsigned char)(num>>8);
		command.pam[2] = (unsigned char)num;
		InsertCommand(slaveNum, &command);
//		}
	return 0;
}
unsigned char MotRunToSite(unsigned char slaveNum, unsigned char definePos){
	COMMAND_STRING command;
	if(slaveNum>=SLAVE_NUM)
		return 1;
	SetMotPosIdle(slaveNum);
//	if(GetSlaveState(slaveNum)==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum << 3) + 2;
		command.cmdLen = 1+2;
		command.pam[0] = CMD_MOT_RUN_TO_SITE;
		command.pam[1] = definePos;
		InsertCommand(slaveNum, &command);
//		}
	return 0;
}
unsigned char SetMotRunPam(unsigned char slaveNum, unsigned char maxVel, unsigned char accel, unsigned char current){
	COMMAND_STRING command;
	if(slaveNum>=SLAVE_NUM)
		return 1;
//	if(GetSlaveState(slaveNum)==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum<<3) + 4;
		command.pam[0] = CMD_MOT_SET_PAM;
		command.pam[1] = maxVel;
		command.pam[2] = accel;
		command.pam[3] = current;
		command.cmdLen = 5;
		InsertCommand(slaveNum, &command);
//		}
	return 0;
}
unsigned char MotSetLock(unsigned char slaveNum, unsigned char lock){
	COMMAND_STRING command;
	if(slaveNum>=SLAVE_NUM)
		return 1;
//	if(GetSlaveState(slaveNum)==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum << 3) + 2;
		command.cmdLen = 1 + 2;
		command.pam[0] = CMD_MOT_ENABLE;
		command.pam[1] = lock;
		InsertCommand(slaveNum, &command);
//		}
	return 0;
}
unsigned char MotSetEnable(unsigned char slaveNum, unsigned char enable){
/*	COMMAND_STRING command;
	command.cmd = (slaveNum<<3) | CMD_MOT_ENABLE;
	command.cmdLen = 1+1;
	command.pam[0] = enable;
	InsertCommand(slaveNum, &command);*/
	return 0;
}
unsigned char GetMotPositionFromSlave(unsigned char slaveNum){
	COMMAND_STRING command;
	if(slaveNum>=SLAVE_NUM)
		return 1;
//	if(GetSlaveState(slaveNum)==(STA_SLAVE_FREE)){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum<<3) + 1;
		command.cmdLen = 1+1;
		command.pam[0] = CMD_MOT_GET_POS;
		InsertCommand(slaveNum, &command);
//		}
	return 0;
}

unsigned char MotInitCheck(unsigned char slaveNum){
	COMMAND_STRING command;
	unsigned char state;
	if(slaveNum>=SLAVE_NUM)
		return 1;
	state = GetSlaveState(slaveNum);
	SetMotPosIdle(slaveNum);
//	if(state==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum<<3) + 1;
		command.cmdLen = 1+1;
		command.pam[0] = CMD_MOT_INIT;
		InsertCommand(slaveNum, &command);	
//		}
	return 0;
}
unsigned char MotAdjustPosition(unsigned char slaveNum, unsigned posNum){
	COMMAND_STRING command;
	unsigned char state;
	if(slaveNum>=SLAVE_NUM)
		return 1;
	state = GetSlaveState(slaveNum);
	SetMotPosIdle(slaveNum);
//	if(state==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum<<3) + 2;
		command.cmdLen = 1+2;
		command.pam[0] = CMD_MOT_ADJUST;
		command.pam[1] = posNum;
		
		InsertCommand(slaveNum, &command);
//		}
	return 0;
}
unsigned char MotSetPam(unsigned char slaveNum, unsigned char pam0, unsigned char pam1){
	COMMAND_STRING command;
	unsigned char state;
	if(slaveNum>=SLAVE_NUM)
		return 1;
	state = GetSlaveState(slaveNum);
//	if(state==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (slaveNum<<3) + 3;
		command.cmdLen = 1+3;
		command.pam[0] = CMD_MOT_SET_BASE_PAM;
		command.pam[1] = pam0;
		command.pam[2] = pam1;
		InsertCommand(slaveNum, &command);
//		}
	return 0;
}
unsigned char SlaveSetAddress(unsigned char address){
	COMMAND_STRING command;
	if(address>SLAVE_NUM)
		return 1;
	command.cmd = COMMON_ADDRESS + 3;
	command.cmdLen = 1+3;
	command.pam[0] = CMD_MOT_SET_ADD;
	command.pam[1] = address;
	command.pam[2] = 0xff - address;
	//InsertCommand(COMMON_ADDRESS, &command);
	InsertCommand(COMMON_ADDRESS, &command);
	//InsertUrgentCommand(address, &command);
	return 0;
}
/*********************************************************************************************/

/************************************* Һ·���� **********************************************/
unsigned char SetEValve(unsigned char evNum, unsigned char sw){
	// evNum:[1:5]����ţ�[0]Ϊ���з�;
	// sw:����״̬��0�رգ�1��
	COMMAND_STRING command;
	unsigned char state;
	state = GetSlaveState(LIQUID_CONTROL);
//	if(state==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (LIQUID_CONTROL << 3) + 3;
		command.pam[0] = CMD_LIQ_SET_VALVE;
		command.pam[1] = evNum;
		command.pam[2] = sw;
		command.cmdLen = 4;
		
		InsertCommand(LIQUID_CONTROL, &command);
//		}
	return 0;
}

unsigned char GetLiquidState(unsigned char phNum){
	// ��ȡҺ·������״̬
	COMMAND_STRING command;
	unsigned char state;
	state = GetSlaveState(LIQUID_CONTROL);
//	if(state==STA_SLAVE_FREE){	// �ڵ��Ϊ����״̬�·���
		command.cmd = (LIQUID_CONTROL<<3) + 2;
		command.pam[0] = CMD_LIQ_GET_STATE;
		command.pam[1] = phNum;
		command.cmdLen = 3;
		InsertCommand(LIQUID_CONTROL, &command);
//		}
	return 0;
}

unsigned char SetLiquidMonitor(unsigned char phNum, unsigned char mode){
	// ����Һ·���������ģʽ
	COMMAND_STRING command;
	unsigned char state;
	state = GetSlaveState(LIQUID_CONTROL);
	command.cmd = (LIQUID_CONTROL<<3) + 3;
	command.pam[0] = CMD_LIQ_SET_MONITOR;
	command.pam[1] = phNum;
	command.pam[2] = mode;		// ����������ģʽ 0����Ч״̬��1����ͨģʽ��2�����ܼ��ģʽ
	command.cmdLen = 4;
	InsertCommand(LIQUID_CONTROL, &command);
	return 0;
}

unsigned char StartLiquidMonitor(unsigned char phNum){
	// ����Һ·���������
	COMMAND_STRING command;
	unsigned char state;
	state = GetSlaveState(LIQUID_CONTROL);
	command.cmd = (LIQUID_CONTROL<<3) + 2;
	command.pam[0] = CMD_LIQ_START_MONITOR;
	command.pam[1] = phNum;
	command.cmdLen = 3;
	InsertCommand(LIQUID_CONTROL, &command);
	return 0;
}

unsigned char ReadLiquidMonitorResult(unsigned char phNum){
	// ��ȡҺ·�����������
	COMMAND_STRING command;
	unsigned char state;
	state = GetSlaveState(LIQUID_CONTROL);
	command.cmd = (LIQUID_CONTROL<<3) + 2;
	command.pam[0] = CMD_LIQ_READ_MONITOR;
	command.pam[1] = phNum;
	command.cmdLen = 3;
	InsertCommand(LIQUID_CONTROL, &command);
	return 0;
}

unsigned char SetLiquidPhotoAdjust(unsigned char phNum){
	// У׼Һ·����
	COMMAND_STRING command;
	
	command.cmd = (LIQUID_CONTROL<<3) + 2;
	command.pam[0] = CMD_LIQ_CAL_PHOTO;
	command.pam[1] = phNum;
	command.cmdLen = 3;
	InsertCommand(LIQUID_CONTROL, &command);
	return 0;
}

unsigned char GetLiquidPhotoInfo(void){
	// ��ȡҺ·���������ֵ
	COMMAND_STRING command;
	
	command.cmd = (LIQUID_CONTROL<<3) + 1;
	command.pam[0] = CMD_LIQ_GET_PHOTO;
	command.cmdLen = 2;
	InsertCommand(LIQUID_CONTROL, &command);
	return 0;
}

unsigned char LiquidFlowCheck(unsigned char liqNum){
	// Һ·�������
	COMMAND_STRING command;

	command.cmd = (LIQUID_CONTROL<<3) + 2;
	command.pam[0] = CMD_LIQ_FLOW_CHECK;
	command.pam[1] = liqNum;
	command.cmdLen = 3;
	InsertCommand(LIQUID_CONTROL, &command);
	return 0;
}
/*********************************************************************************************/

/************************************* Ƭ�ֿ��� **********************************************/

unsigned char GetStoreState(unsigned  char chNum){
	// ��ȡƬ��״̬,0:����Ƭ��״̬;1:ָ��Ƭ��״̬.
	// 0xff:Ƭ�ִ�,״̬��Ч
	// 0/
	// 0~3:Ƭ��״̬
	COMMAND_STRING command;
	if(chNum>6)
		chNum = 6;
	command.cmd = (STORE_MONITOR<<3) + 2;
	command.pam[0] = CMD_STORE_GET_STATE;
	command.pam[1] = chNum;
	command.cmdLen = 3;
	InsertCommand(STORE_MONITOR, &command);
	return 0;
}

unsigned char SetStoreCAL(unsigned char num){
	// Ƭ�ֹ�·У׼
	COMMAND_STRING command;

	command.cmd = (STORE_MONITOR<<3) + 2;
	command.pam[0] = CMD_STORE_CAL;
	command.pam[1] = num;
	command.cmdLen = 3;
	InsertCommand(STORE_MONITOR, &command);
	return 0;
}

unsigned char GetStorePhoVol(unsigned char num){
	// ��ȡ��·��ѹֵ
	COMMAND_STRING command;
//	if(num == 0)
//		num = 1;
//	if(num>6)
//		num = 6;
	command.cmd = (STORE_MONITOR<<3) + 2;
	command.pam[0] = CMD_STORE_GET_VOL;
	command.pam[1] = num;
	command.cmdLen = 3;
	InsertCommand(STORE_MONITOR, &command);
	return 0;
}

unsigned char SetStoreDoorOpen(unsigned char num){
	// ����Ƭ����
	COMMAND_STRING command;
//	if(num == 0)
//		num = 1;
//	if(num>6)
//		num = 6;
	command.cmd = (STORE_MONITOR<<3) + 2;
	command.pam[0] = CMD_STORE_OPEN;
	command.pam[1] = num;
	command.cmdLen = 3;
	InsertCommand(STORE_MONITOR, &command);
	return 0;
}
