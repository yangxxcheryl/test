

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"


extern  unsigned char _SampSW;         
unsigned char _LEDSTATE = 0;   //0��ʾ���Թ����Ǻ�ƣ�1��ʾ���Թ������̵�


typedef struct _MOTOR_POSITION{
	unsigned char defPosNum;	// Ԥ����λ��
	int stepPos;		// ������λ��
}MOTOR_POSITION;


extern unsigned char ControlModel;
extern unsigned char WorkProcessStep;		// �������̺�


unsigned char (*EvenPosChangeProcess)(INFO_EVENT * pInfoEvent);
unsigned char (*EvenLiquidProcess)(INFO_EVENT * pInfoEvent);
unsigned char (*EvenCardStoreProcess)(INFO_EVENT * pInfoEvent);

MOTOR_POSITION MotorPosition[SLAVE_NUM];
unsigned char MotModulePhoSta[SLAVE_NUM][2];	// ���ģ���ź״̬��Ϣ
unsigned char LiquidState[4][2];	// Һ·״̬�Ͳ���
unsigned char StoreHumi;			// Ƭ��ʪ��
unsigned char StoreTemp;			// Ƭ���¶�

unsigned char _RingPieceState[RING_QUEUE_NUM];	// ת�̸�Ƭ״̬, 0:��, 1:����, 255:��Ч


// ���ݳ�ʼ��
void InitControlLayerData(void){
	unsigned char i;
	for(i=0; i<SLAVE_NUM; i++){
		MotorPosition[i].defPosNum = 0xf0;
		MotorPosition[i].stepPos = 0x7fff;
		}
	EvenPosChangeProcess = 0;
	EvenLiquidProcess = 0;
	EvenCardStoreProcess = 0;
}

unsigned int time;
unsigned int uart1ReceiveOutTime;


unsigned char GetRingPieceState(unsigned char n){
	if(n>=RING_QUEUE_NUM)
		return 0xff;
	return _RingPieceState[n];
}

// ��ʪ��
unsigned char GetStoreHumi(void){
	return StoreHumi;
}
unsigned char GetStoreTemp(void){
	return StoreTemp;
}

// ���λ��
void SetMotPosIdle(unsigned char slaveNum)
{
	MotorPosition[slaveNum].defPosNum = 0xf0;
//	MotorPosition[slaveNum].stepPos = 0x7fff;
}
unsigned char GetMotPositionOfSite(unsigned char slaveNum){
	return MotorPosition[slaveNum].defPosNum;
}
int GetMotPositionOfStep(unsigned char slaveNum){
	return MotorPosition[slaveNum].stepPos;
}

// ��ȡҺ·״̬
unsigned char GetLiquidMonitorState(unsigned char num){
	if(num>3)
		num = 3;
	return LiquidState[num][0];
}
unsigned char GetLiquidMonitorStatePam(unsigned char num){
	if(num>3)
		num = 3;
	return LiquidState[num][1];
}


// ע��λ�øı��Զ���������
unsigned char RegisterPosChangeEvenProcess(void * proc){
	EvenPosChangeProcess = proc;
}
void CleanPosChangeEvenProcess(void){
	EvenPosChangeProcess = 0;
}
// ע��Һ·�¼���������
unsigned char RegisterLiquidEvenProcess(void * proc){
	EvenLiquidProcess = proc;
}
void CleanLiquidEvenProcess(void){
	EvenLiquidProcess = 0;
}
// ע��Ƭ���¼���������
unsigned char RegisterCardStoreEvenProcess(void * proc)
{
	EvenCardStoreProcess = proc;
}
void CleanPosCardStoreProcess(void)
{
	EvenCardStoreProcess = 0;
}

// �ϴ�ָ��������״̬��Ϣ
extern unsigned char CardSurplusState[6];	// ��Ƭʣ��״̬
extern unsigned char CardStoretate[6];		// Ƭ��״̬
void UpLoadingModuleSensorState(unsigned char slaveNum, unsigned char num){
	char s[2][2] = {"0","1"};
	unsigned char i;
	switch(slaveNum){
		case 0:		// �����ư��ϵ��ź�
			switch(num){
				case 0:		// J4 ȡƬ��⿪���ź�
					i = (PINL & 0x04);
					if(i!=0)
						i = 1;
					Uart0ReUnable;
					uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
					Uart0ReEnable;
					break;
				case 1:		// J7 ת�̸�Ƭ����ź�ź�
					i = PINK & 0x01;
					if(i!=0)
						i = 1;
					Uart0ReUnable;
					uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
					Uart0ReEnable;
					break;
				case 2:		// J8 ��Ƭ�п����ź�
					if(GetwasteCardState() == 0)// ��Ƭ�ֹ��ܿ���
					{
						i = PINK & 0x02;
						if(i!=0)
							i = 1;
						Uart0ReUnable;
						uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
						Uart0ReEnable;
					}
					break;
				case 3:		// J10 Һ�洫�����ź�
					Uart0ReUnable;
					uart_Printf("%s $%2d $%2d $%4d\r\n",strM4201, slaveNum, num, getLiqDetADC(NeedleChannel));
					Uart0ReEnable;
					break;
				case 4:		// J12 ���������ź�
					i = PINJ & 0x40;
					if(i!=0)
						i = 1;
					Uart0ReUnable;
					uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
					Uart0ReEnable;
					break;
				}
			break;
		case MOT_TURN_PLATE:
		case MOT_SAMP_TRUN:
		case MOT_SAMP_NEEDLE:
		case MOT_CARD_LOAD:
		case MOT_CARD_UNLOAD:
		case MOT_STORE_CARD_MOVE:
		case MOT_SAMP_PUMP:
			if(num>1)
				num = 1;
			i = MotModulePhoSta[slaveNum][num];
			if(i!=0)
				i = 1;
			// ���͵��ģ���ź�ź�
			Uart0ReUnable;
			uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
			Uart0ReEnable;
			break;
		case LIQUID_CONTROL:
			if(num>2)
				num = 2;
			// ����Һ·״̬�ź�
			if(LiquidState[num][0] == INFO_LIQ_FULL)
				i = 1;
			else
				i = 0;
			Uart0ReUnable;
			uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
			Uart0ReEnable;
			break;
		case STORE_MONITOR:
			if(num>4)
				num = 4;
			if(CardStoretate[num] == INFO_STORE_OPEN)
			{
				Uart0ReUnable;
				uart_Printf("%s $%2d $%2d $%4d\r\n",strM4201, slaveNum, num, CardStoretate[num]);
				Uart0ReEnable;
			}
			else
			{
				Uart0ReUnable;
				uart_Printf("%s $%2d $%2d $%4d\r\n",strM4201, slaveNum, num, CardSurplusState[num]);
				Uart0ReEnable;
			}
			break;
		default:
			break;
		}
}
/*
unsigned char GetTurnPlateMonitorState(void)
{	
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3175,MotModulePhoSta[MOT_TURN_PLATE][1]);
	Uart0ReEnable;
	return MotModulePhoSta[MOT_TURN_PLATE][1];
}
*/

signed char GetMotorMonitorState(unsigned char slave,unsigned char num)
{
	if(slave == 0 || slave == 6 || slave == 7)	
		return -1;
	if(num > 1)
		num = 1;
	Uart0ReUnable;
	uart_Printf("%s",strM3175);
	switch(slave)
	{
		case 1:uart_Printf(":MOT_TURN_PLATE");break;
		case 2:uart_Printf(":MOT_SAMP_TRUN");break;
		case 3:uart_Printf(":MOT_SAMP_NEEDLE");break;
		case 4:uart_Printf(":MOT_CARD_LOAD");break;
		case 5:uart_Printf(":MOT_CARD_UNLOAD");break;
		case 8:uart_Printf(":MOT_STORE_CARD_MOVE");break;
		case 9:uart_Printf(":MOT_DILUENT");break;
		case 10:uart_Printf(":MOT_FLUID");break;
		case 11:uart_Printf(":MOT_EFFLUENT");break;
		case 12:uart_Printf(":MOT_SAMP_PUMP");break;
		case 13:uart_Printf(":LIQUID_CONTROL");break;
		case 14:uart_Printf(":STORE_MONITOR");break;
		default:break;
	}
	uart_Printf("$%2d $%2d\r\n",num,MotModulePhoSta[slave][num]);
	//uart_Printf("%s:%s.%s $%2d\r\n",strM3175,SlaveID[slave],Monitor[num],MotModulePhoSta[MOT_TURN_PLATE][num]);
	Uart0ReEnable;
	return MotModulePhoSta[slave][num];
}

// �ϴ����д�������Ϣ
void UpLoadingAllSensorState(void){
	char s[2][2] = {"0","1"};
	unsigned char i,n, f;
	Uart0ReUnable;
	uart_Printf("%s",strM4202);	// ���Ϳ�ʼ
	Uart0ReEnable;
	// ���͵��ģ���ź��Ϣ
	for(i=1; i<=12; i++)
	{
		if(i==6 || i==7)
			continue;
		for(n=0; n<2; n++)
		{
			f = MotModulePhoSta[i][n];
			if(f!=0)
				f = 1;
			// ���͵��ģ���ź�ź�
			Uart0ReUnable;
			uart_Printf(" $%c",s[f][0]);
			Uart0ReEnable;
		}
	}
	// ����Һ·ģ����Ϣ
	i = 13;
	for(n=0; n<3; n++)
	{
		if(LiquidState[n][0] == INFO_LIQ_FULL)
			f = 1;
		else
			f = 0;
		Uart0ReUnable;
		uart_Printf(" $%c",s[f][0]);
		Uart0ReEnable;
	}
	// ����Ƭ��ģ����Ϣ
	i = 14;
	for(n=0; n<5; n++){
		if(CardStoretate[n] == INFO_STORE_OPEN)
		{
			Uart0ReUnable;
			uart_Printf(" $%2d",CardStoretate[n]);
			Uart0ReEnable;
		}
		else
		{
			Uart0ReUnable;
			uart_Printf(" $%2d",CardSurplusState[n]);
			Uart0ReEnable;
		}
	}
	//��������ģ����Ϣ
	i = 0;
	for(n=0; n<5; n++){
		switch(n){
			case 0:		// J4 ȡƬ��⿪���ź�
				f = (PINL & 0x04);
				if(f!=0)
					f = 1;
				Uart0ReUnable;
				uart_Printf(" $%c",s[f][0]);
				Uart0ReEnable;
				break;
			case 1:		// J7 ת�̸�Ƭ����ź�ź�
				f = PINK & 0x01;
				if(f!=0)
					f = 1;
				Uart0ReUnable;
				uart_Printf(" $%c",s[f][0]);
				Uart0ReEnable;
				break;
			case 2:		// J8 ��Ƭ�п����ź�
				if(GetwasteCardState() == 0)
				{
					f = PINK & 0x02;
					if(f!=0)
						f = 1;
					Uart0ReUnable;
					uart_Printf(" $%c",s[f][0]);
					Uart0ReEnable;
				}
				break;
			case 3:		// J10 Һ�洫�����ź�
				Uart0ReUnable;
				uart_Printf(" $%4d",getLiqDetADC(NeedleChannel));
				Uart0ReEnable;
				break;
			case 4:		// J12 ���������ź�
				f = PINJ & 0x40;
				if(f!=0)
					f = 1;
				Uart0ReUnable;
				uart_Printf(" $%c",s[f][0]);
				Uart0ReEnable;
				break;
			}
		}
	uart_Printf("\r\n");	// ���ͽ���
}



//git test comment for branch "old"


// �ӻ��¼������ͷ���
void SlaveEventAssignProcess(INFO_EVENT * pInfoEvent){
	// �¼����䴦��
	unsigned char curSlave, infoLen;
	unsigned char * pInfo, *p;
	int pos;
	unsigned char even;
	unsigned char i, j;
	static unsigned char pieceState;

	curSlave = pInfoEvent->slaveNum;
	even = pInfoEvent->event;
	pInfo = &(pInfoEvent->info[0]);
	
	switch(even)
	{
		case STA_CHANGE_POS:
		case STA_INFO_POS:
			pos = *(pInfo+1);
			pos = pos << 8;
			pos += *(pInfo+2);
			MotorPosition[curSlave].defPosNum = *(pInfo);
			MotorPosition[curSlave].stepPos = pos;
			if(EvenPosChangeProcess){
				i = EvenPosChangeProcess(pInfoEvent);
				if(i)
					EvenPosChangeProcess = 0;
				}
			break;
		case STA_MOT_PHO:	// ���ģ���ź״̬�ı���Ϣ
			MotModulePhoSta[curSlave][*(pInfo)] = *(pInfo+1);
			break;

		case INFO_LIQ_EMPTY:		// Һ·��
		case INFO_LIQ_FULL:			// Һ·��
			i = *pInfo;
			if(i<4){
				LiquidState[i][0] = even;
				LiquidState[i][1] = 0;
				}
			break;
		case INFO_LIQ_BUBBLE:		// ������
		case INFO_LIQ_FLOW:			// ��Һ��
			i = *pInfo;
			if(i<4){
				LiquidState[i][0] = even;
				LiquidState[i][1] = *(pInfo+1);
				}
			break;
		case INFO_LIQ_PHO_ON:		// Һ·�������⵽Һ��
		case INFO_LIQ_PHO_OFF:		// Һ·�������⵽��
		case INFO_LIQ_PHO_VAL:		// Һ·��������ź�ֵ
		case INFO_LIQ_PHO_ADJ:		// Һ·����������
			switch(WorkProcessStep){
				case 0:		// ����
					break;
				case 1:		// ��е�Լ�
					break;
				case 2:		// Һ·�Լ�
					DiluteStartCheck(pInfoEvent);
					break;
				case 3:		// ��������
					DiluteProcess(pInfoEvent);
					break;
				case 4:		// ����ά��
					break;
				}
			if(EvenLiquidProcess){
				i = EvenLiquidProcess(pInfoEvent);
				if(i)
					EvenLiquidProcess = 0;
				}
			break;
		case INFO_STORE_OPEN:		// Ƭ�ִ�
		case INFO_STORE_CLOSE:		// Ƭ�ֹر�
		case INFO_STORE_FULL:		// Ƭ����
		case INFO_STORE_LITTLE:		// Ƭ������
		case INFO_STORE_EMPTY:		// Ƭ�ֿ�
		case INFO_STORE_ERROR:		// Ƭ��״̬����
		case INFO_STORE_CAL:		// Ƭ�ֹ�·У׼��Ϣ
		case INFO_STORE_PHO_VOL:
		case INFO_STORE_STATE_ALL:	// ȫ��Ƭ��״̬��Ϣ
		case INFO_STORE_STATE_SPC:	// ָ��Ƭ��״̬��Ϣ
			CardStoreSteteProcess(pInfoEvent);
			if(EvenCardStoreProcess)
			{
				i = EvenCardStoreProcess(pInfoEvent);
				if(i)
					EvenCardStoreProcess = 0;
			}
			break;
		case INFO_STORE_HUMITURE:	// Ƭ����ʪ��
			StoreHumi = *pInfo;
			StoreTemp = *(pInfo+1);
			Uart0ReUnable;
			uart_Printf("%s $%4d $%4d\r\n",strM0111, StoreHumi, StoreTemp);
			Uart0ReEnable;
			break;
		case INFO_STORE_OPEN_ERR:	// Ƭ�ֿ�����ʱ����
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strE0910, *pInfo);
			Uart0ReEnable;
			break;
		default:
			break;
		}

}

/*********************************************************************************************/



/****************************************************************************************************/
// ��е���г�ʼ��
/*
unsigned char MachinePositionInit(void){
	// ��еλ�ó�ʼ��
	static unsigned char mainStep;		
	static unsigned char waitMotSampTurn,waitMotSampNeedle, waitMotSampPump;
	static unsigned char waitMotCardTrolley, waitMotCardLoad, waitMotCardUnLoad, waitMotTurnPlate;
	static unsigned char waitMotLifterA, waitMotLifterB;
	static unsigned char i, n, m;


	if(WaitDelayTime(MOT_SAMP_PUMP))		return 0;
	if(WaitDelayTime(MOT_SAMP_TRUN))		return 0;
	if(WaitDelayTime(MOT_SAMP_NEEDLE))		return 0;
	if(WaitDelayTime(MOT_EFFLUENT))			return 0;
		
	if(waitMotSampTurn){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
	if(waitMotSampNeedle){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
	if(waitMotSampPump){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotSampPump = 0;	}
	if(waitMotCardLoad){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)		return 0;	waitMotCardLoad = 0;	}
	if(waitMotCardUnLoad){	if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;	waitMotCardUnLoad = 0;	}
	if(waitMotTurnPlate){	if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)		return 0;	waitMotTurnPlate = 0;	}
	if(waitMotCardTrolley){	if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)	return 0;	waitMotCardTrolley = 0;	}
	
	switch(mainStep){
		case 0:		// �������в���
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			SetMotRunPam(MOT_CARD_LOAD,160,20,CURRENT_CARD_LOAD);
			SetMotRunPam(MOT_CARD_UNLOAD,160,20,CURRENT_CARD_UNLOAD);
			SetMotRunPam(MOT_SAMP_PUMP,180,10,CURRENT_SAMP_PUMP);
			
			MotInitCheck(MOT_SAMP_NEEDLE);
			MotInitCheck(MOT_CARD_UNLOAD);
			MotInitCheck(MOT_CARD_LOAD);
			MotInitCheck(MOT_SAMP_PUMP);
			SetCardTrolleyState(0);
			SetEValve(EV1, EV_OPEN);
			SetMotRunPam(MOT_EFFLUENT, 180, 2, CURRENT_EFFLUENT);
			MotRun(MOT_EFFLUENT, 2500);			// ������Һ��
			waitMotCardUnLoad = 1;
			mainStep = 1;
			break;
		case 1:		// ת��λ�ó�ʼ��
			SetMotRunPam(MOT_TURN_PLATE,240,20,CURRENT_TURN_PLATE);
			MotInitCheck(MOT_TURN_PLATE);
			waitMotTurnPlate = 1;
			mainStep = 2;
			break;
		case 2:		// ת�����е�0λ
			MotRunToSite(MOT_TURN_PLATE,0);
			waitMotTurnPlate = 1;
			waitMotSampNeedle = 1;
			mainStep = 3;
			break;
		case 3:		// Ƭ��С�����е���ʼλ
			SetMotRunPam(MOT_STORE_CARD_MOVE,200,10,CURRENT_STORE_MOVE);
			MotInitCheck(MOT_STORE_CARD_MOVE);
			waitMotSampNeedle = 1;
			mainStep = 100;
			break;
		case 100:		// ȡ������㾭����������תʱ���������ź�, �˴����������Ա������
			MotRunTo(MOT_SAMP_NEEDLE, 0);
			waitMotSampNeedle = 1;
			mainStep = 4;
			break;
		case 4:		// ȡ���ۻص���ʼλ
			SetEValve(EV_ALL, EV_CLOSE);
			SetMotRunPam(MOT_SAMP_TRUN,200,5,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			SetEValve(EV1, EV_CLOSE);
			waitMotSampTurn = 1;
			waitMotSampPump = 1;
			waitMotTurnPlate = 1;
			//mainStep = 5;
			mainStep = 7;
			i = 0;
			break;
		case 5:	// ȡ������ת������
			MotRunTo(MOT_SAMP_TRUN,_POS_SAMPTURN_SAMP+200);
			waitMotSampTurn = 1;
			mainStep = 6;
			break;
		case 6:
			MotRunTo(MOT_SAMP_TRUN,0);
			waitMotSampTurn = 1;
			i++;
			if(i<1){
				mainStep = 5;
				SetDelayTime(MOT_SAMP_TRUN, 10);
				}
			else{	
				mainStep = 7;	
				i = 0;	
				}
			break;
		case 7:		// ɨ��ת����������Ƭ	_RingPieceState
			i++;
			if(i>=RING_QUEUE_NUM)
				i = 0;
			m = PINK & 0x01;
			_RingPieceState[i] = 0;
			MotRunToSite(MOT_TURN_PLATE,i);
			mainStep = 8;
			break;
		case 8:
			n = PINK & 0x01;
			if(m != n)
				_RingPieceState[i] = 1;
			if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	
				break;
			if(i==0)
				mainStep = 9;
			else
				mainStep = 7;
			break;
		case 9:		// ɨ��ת��
			MotRunToSite(MOT_TURN_PLATE,10);	// 25 ��ת�̵�0��ת��жƬλ��
			waitMotTurnPlate = 1;
			i = 0;
			mainStep = 10;
			break;
		case 10:		// ��˳��Ѱ��ʣ���Ƭ
			n = i + 10;
			if(n>=RING_QUEUE_NUM)
				n -= RING_QUEUE_NUM;
			if(GetRingPieceState(i)==1){	// ת������ʣ���Ƭ
				MotRunToSite(MOT_TURN_PLATE,n);	// ת�����е���ǰλ��
				waitMotTurnPlate = 1;
				mainStep = 11;
				m = 0;		// ��Ƭ�ִ򿪼�ʱ
				}
			else
				mainStep = 14;		// ����������һ��
			break;
		case 11:		// ��ʼж��ת���ϵĸ�Ƭ
			if((PINK & 0x02) == 0){
				SetDelayTime(MOT_TURN_PLATE, 10);
				m ++;
				if(m == 2){
					uart_Printf("%s $   1\r\n",strM0200);
					SetBeepWarning();
					}
				if(m == 25){
					SetBeepWarning();
					m = 5;
					}
				
				break;
				}
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);
			waitMotCardUnLoad = 1;
			mainStep = 12;
			break;
		case 12:
			SetDelayTime(MOT_SAMP_TRUN, 10);
			mainStep = 13;
			break;
		case 13:	// жƬС���ص���ʼ��
			MotRunTo(MOT_CARD_UNLOAD,0);
			waitMotCardUnLoad = 1;
			mainStep = 14;
			break;
		case 14:
			i++;
			if(i < RING_QUEUE_NUM)	// δ�����
				mainStep = 10;
			else
				mainStep = 15;	// ������
			break;
		case 15:	// ���Ƭ��С�����Ƿ��и�Ƭ
			SetMotRunPam(MOT_STORE_CARD_MOVE,64,10,CURRENT_STORE_MOVE);
			MotRunTo(MOT_STORE_CARD_MOVE,100);
			waitMotCardTrolley = 1;
			mainStep = 16;
			break;
		case 16:
			i = PINL & 0x04;		// ��ȡȡƬ��⴫��������״̬
			MotRunTo(MOT_STORE_CARD_MOVE,0);	// ȡƬС�����е���λ
			waitMotCardTrolley = 1;
			mainStep = 17;
			break;
		case 17:
			n = PINL & 0x04;		// ��ȡȡƬ��⴫����״̬
			if(i != n){		// ȡƬС������ʣ���Ƭ
				MotRunToSite(MOT_TURN_PLATE,0);	// ת�����е�0��λ��
				waitMotTurnPlate = 1;
				mainStep = 18;
				}
			else
				mainStep = 26;		// ȡƬС����û�и�Ƭ, ����
			break;
		case 18:		// ��Ƭ����ת��
			SetMotRunPam(MOT_CARD_LOAD,160,10,CURRENT_CARD_LOAD);
			MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_RING);		// װƬ�г�94mm/0.08128 = 1156
			waitMotCardLoad = 1;
			mainStep = 20;
			break;
		case 19:		// ��Ƭ����ת�̲���2
		//	SetMotRunPam(MOT_CARD_LOAD,64,2,2);
			MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_RING);		// װƬ�г�94mm/0.08128 = 1156
			waitMotCardLoad = 1;
			mainStep = 20;
			break;
		case 20:
			SetDelayTime(MOT_SAMP_TRUN, 10);
			mainStep = 21;
			break;
		case 21:		// ��Ƭ���븴λ
			SetMotRunPam(MOT_CARD_LOAD,200,10,CURRENT_CARD_LOAD);
			MotRunTo(MOT_CARD_LOAD,0);
			waitMotCardLoad = 1;
			mainStep = 22;
			break;
		case 22:		// ת��0��ת��жƬλ��
		//	SetMotRunPam(MOT_TURN_PLATE,240,10,2);
			MotRunToSite(MOT_TURN_PLATE,25);
			waitMotTurnPlate = 1;
			mainStep = 23;
			break;
		case 23:		// ��ʼжƬ
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);
			waitMotCardUnLoad = 1;
			mainStep = 24;
			break;
		case 24:
			SetDelayTime(MOT_SAMP_TRUN,10);
			mainStep = 25;
			break;
		case 25:		// жƬС���ص���ʼλ
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,2);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_HOME);
			waitMotCardUnLoad = 1;
			mainStep = 26;
			break;
		case 26:		// ת����ת����ʼ״̬
			MotRunToSite(MOT_TURN_PLATE,29);
			waitMotTurnPlate = 1;
			MotRunTo(MOT_STORE_CARD_MOVE,432);
			waitMotCardTrolley = 1;
			mainStep = 27;
			break;
		case 27:
			uart_Printf("%s\r\n", strM1101);
			mainStep = 0;
			return 1;
			break;
		}
	return 0;
}
*/

unsigned char MachinePositionInit(void){
	// ��еλ�ó�ʼ��
	static unsigned char mainStep;		
	static unsigned char waitMotSampTurn,waitMotSampNeedle, waitMotSampPump;
	static unsigned char waitMotCardTrolley, waitMotCardLoad, waitMotCardUnLoad, waitMotTurnPlate;
	static unsigned char waitMotLifterA, waitMotLifterB;
	static unsigned char i, n, m;


	if(WaitDelayTime(MOT_SAMP_PUMP))		return 0;
	if(WaitDelayTime(MOT_SAMP_TRUN))		return 0;
	if(WaitDelayTime(MOT_SAMP_NEEDLE))		return 0;
	if(WaitDelayTime(MOT_EFFLUENT))			return 0;
		
	if(waitMotSampTurn){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
	if(waitMotSampNeedle){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
	if(waitMotSampPump){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotSampPump = 0;	}
	if(waitMotCardLoad){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)		return 0;	waitMotCardLoad = 0;	}
	if(waitMotCardUnLoad){	if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;	waitMotCardUnLoad = 0;	}
	if(waitMotTurnPlate){	if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)		return 0;	waitMotTurnPlate = 0;	}
	if(waitMotCardTrolley){	if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)	return 0;	waitMotCardTrolley = 0;	}
	
	switch(mainStep)
	{
		case 0:		// �������в���
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			SetMotRunPam(MOT_CARD_LOAD,160,20,CURRENT_CARD_LOAD);
			SetMotRunPam(MOT_CARD_UNLOAD,160,20,CURRENT_CARD_UNLOAD);
			SetMotRunPam(MOT_SAMP_PUMP,180,10,CURRENT_SAMP_PUMP);
			
			MotInitCheck(MOT_SAMP_NEEDLE);
			MotInitCheck(MOT_CARD_UNLOAD);
			MotInitCheck(MOT_CARD_LOAD);
			MotInitCheck(MOT_SAMP_PUMP);
			SetCardTrolleyState(0);
			SetEValve(EV1, EV_OPEN);
			SetMotRunPam(MOT_EFFLUENT, 180, 2, CURRENT_EFFLUENT);
			MotRun(MOT_EFFLUENT, 2500);			// ������Һ��
			waitMotCardUnLoad = 1;
			mainStep = 1;
			break;
		case 1:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2103);
			Uart0ReEnable;
			mainStep = 2;
			break;
		case 2:		// ת��λ�ó�ʼ��
			SetMotRunPam(MOT_TURN_PLATE,240,20,CURRENT_TURN_PLATE);
			MotInitCheck(MOT_TURN_PLATE);
			waitMotTurnPlate = 1;
			mainStep = 3;
			break;
		case 3:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2104);
			Uart0ReEnable;
			mainStep = 4;
			break;
		case 4:		// ת�����е�0λ
			MotRunToSite(MOT_TURN_PLATE,0);
			waitMotTurnPlate = 1;
			waitMotSampNeedle = 1;
			mainStep = 5;
			break;
		case 5:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2107);
			Uart0ReEnable;
			mainStep = 6;
			break;
		case 6:		// Ƭ��С�����е���ʼλ
			SetMotRunPam(MOT_STORE_CARD_MOVE,200,10,CURRENT_STORE_MOVE);
			MotInitCheck(MOT_STORE_CARD_MOVE);
			waitMotSampNeedle = 1;
			mainStep = 7;
			break;
		case 7:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2108);
			Uart0ReEnable;
			mainStep = 8;
			break;
		case 8:		// ȡ������㾭����������תʱ���������ź�, �˴����������Ա������
			MotRunTo(MOT_SAMP_NEEDLE, 0);
			waitMotSampNeedle = 1;
			mainStep = 9;
			break;
		case 9:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2109);
			Uart0ReEnable;
			mainStep = 10;
			break;
		case 10:		// ȡ���ۻص���ʼλ
			SetEValve(EV_ALL, EV_CLOSE);
			SetMotRunPam(MOT_SAMP_TRUN,200,5,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			SetEValve(EV1, EV_CLOSE);
			waitMotSampTurn = 1;
			waitMotSampPump = 1;
			waitMotTurnPlate = 1;
			mainStep = 11;
			i = 0;
			break;
		case 11:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2113);
			uart_Printf("%s\r\n",strM2118);
			Uart0ReEnable;
			mainStep = 12;
			break;
		case 12:		// ɨ��ת����������Ƭ	_RingPieceState
			i++;
			if(i >= RING_QUEUE_NUM)
				i = 0;
			m = PINK & 0x01;
			_RingPieceState[i] = 0;
			MotRunToSite(MOT_TURN_PLATE,i);
			mainStep = 13;
			break;
		case 13:
			n = PINK & 0x01;
			if(m != n)
				_RingPieceState[i] = 1;
			if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	
				break;
			if(i == 0)
				mainStep = 14;
			else
				mainStep = 12;
			break;
		case 14:		// ɨ��ת��
			MotRunToSite(MOT_TURN_PLATE,10);	// 25 ��ת�̵�0��ת��жƬλ��
			waitMotTurnPlate = 1;
			i = 0;
			mainStep = 15;
			break;
		case 15:		// ��˳��Ѱ��ʣ���Ƭ
			n = i + 10;
			if(n>=RING_QUEUE_NUM)
				n -= RING_QUEUE_NUM;
			if(GetRingPieceState(i) == 1)	// ת������ʣ���Ƭ
			{
				MotRunToSite(MOT_TURN_PLATE,n);	// ת�����е���ǰλ��
				waitMotTurnPlate = 1;
				mainStep = 16;
				m = 0;		// ��Ƭ�ִ򿪼�ʱ
			}
			else
				mainStep = 19;		// ����������һ��
			break;
		case 16:		// ��ʼж��ת���ϵĸ�Ƭ
			if(GetwasteCardState() == 0)
			{
				if((PINK & 0x02) == 0)
				{
					SetDelayTime(MOT_TURN_PLATE, 10);
					m ++;
					if(m == 2)
					{
						Uart0ReUnable;
						uart_Printf("%s $   1\r\n",strM0200);
						Uart0ReEnable;
						SetBeepWarning();
					}
					if(m == 25)
					{
						SetBeepWarning();
						m = 5;
					}
					break;
				}
			}
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);
			waitMotCardUnLoad = 1;
			mainStep = 17;
			break;
		case 17:
			SetDelayTime(MOT_SAMP_TRUN, 10);
			mainStep = 18;
			break;
		case 18:	// жƬС���ص���ʼ��
			MotRunTo(MOT_CARD_UNLOAD,0);
			waitMotCardUnLoad = 1;
			mainStep = 19;
			break;
		case 19:
			i++;
			if(i < RING_QUEUE_NUM)	// δ�����
				mainStep = 15;
			else
			{
				mainStep = 20;	// ������
				Uart0ReUnable;
				uart_Printf("%s\r\n",strM2114);
				Uart0ReEnable;
			}
			break;
		case 20:	// ���Ƭ��С�����Ƿ��и�Ƭ
			SetMotRunPam(MOT_STORE_CARD_MOVE,64,10,CURRENT_STORE_MOVE);
			MotRunTo(MOT_STORE_CARD_MOVE,100);
			waitMotCardTrolley = 1;
			mainStep = 21;
			break;
		case 21:
			i = PINL & 0x04;					// ��ȡȡƬ��⴫��������״̬
			MotRunTo(MOT_STORE_CARD_MOVE,0);	// ȡƬС�����е���λ
			waitMotCardTrolley = 1;
			mainStep = 22;
			break;
		case 22:
			n = PINL & 0x04;					// ��ȡȡƬ��⴫����״̬
			if(i != n)		// ȡƬС������ʣ���Ƭ
			{
				MotRunToSite(MOT_TURN_PLATE,0);	// ת�����е�0��λ��
				waitMotTurnPlate = 1;
				mainStep = 23;
				Uart0ReUnable;
				uart_Printf("%s\r\n",strM2115);
				Uart0ReEnable;
			}
			else
				mainStep = 31;		// ȡƬС����û�и�Ƭ, ����
			break;
		case 23:		// ��Ƭ����ת��
			SetMotRunPam(MOT_CARD_LOAD,160,10,CURRENT_CARD_LOAD);
			MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_RING);		// װƬ�г�94mm/0.08128 = 1156
			waitMotCardLoad = 1;
			mainStep = 24;
			break;
		case 24:
			SetDelayTime(MOT_SAMP_TRUN, 10);
			mainStep = 25;
			break;
		case 25:		// ��Ƭ���븴λ
			SetMotRunPam(MOT_CARD_LOAD,200,10,CURRENT_CARD_LOAD);
			MotRunTo(MOT_CARD_LOAD,0);
			waitMotCardLoad = 1;
			mainStep = 26;
			break;
		case 26:		// ת��0��ת��жƬλ��
		//	SetMotRunPam(MOT_TURN_PLATE,240,10,2);
			MotRunToSite(MOT_TURN_PLATE,25);
			waitMotTurnPlate = 1;
			mainStep = 27;
			break;
		case 27:		// ��ʼжƬ
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);
			waitMotCardUnLoad = 1;
			mainStep = 28;
			break;
		case 28:
			SetDelayTime(MOT_SAMP_TRUN,10);
			mainStep = 29;
			break;
		case 29:		// жƬС���ص���ʼλ
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,2);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_HOME);
			waitMotCardUnLoad = 1;
			mainStep = 30;
			break;
		case 30:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2116);
			Uart0ReEnable;
			mainStep = 31;
			break;
		case 31:		// ת����ת����ʼ״̬
			MotRunToSite(MOT_TURN_PLATE,29);
			waitMotTurnPlate = 1;
			MotRunTo(MOT_STORE_CARD_MOVE,432);
			waitMotCardTrolley = 1;
			mainStep = 32;
			break;
		case 32:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2117);
			Uart0ReEnable;
			mainStep = 33;
			break;
		case 33:
			Uart0ReUnable;
			uart_Printf("%s\r\n", strM1101);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		}
	return 0;
}

// �ȴ���������
unsigned char WaitStartKey(void){
	// �ȴ�������������
	static unsigned char callCnt;
	unsigned char key;
	
	key = (PINJ & 0x40);
	if(callCnt == 0){
		if(key == 0)	// �������ɿ����������������һֱ������Ϊ�޶���
			callCnt = 1;
		}
	else {
		if(key == 0x40){	// ��������
			callCnt = 0;
			return 1;
			}
		}
	return 0;
}

/********************************************* ������ʾ�� **********************************************/
static unsigned char BeepNum=0;
static unsigned char BeepState=0;
static unsigned int BeepCnt;

void SetBeepBusy(void){
	// 1�� 1��
	BeepState=1;
	BeepNum = 0;
	BeepCnt=0;
}
void SetBeepWarning(void){
	// 1����
	BeepState=2;
	BeepNum = 0;
	BeepCnt=0;
}
void SetBeepAck(void){
	// 1����
	BeepState=3;
	BeepNum = 0;
	BeepCnt=0;
	
}
void SetBeepPrompt(void){
	// 2����
	BeepState=4;
	BeepNum = 0;
	BeepCnt=0;
}
void SetBeepError(void){
	// ��������
	BeepState=5;
	BeepNum = 0;
	BeepCnt=0;
}
void SetBeep(unsigned char n){
	BeepState=0;
	BeepNum = n;
	BeepCnt=0;
}
void BeepStop(void){
	BeepState=0;
	BeepNum = 0;
	BeepCnt=0;
}

void Beep(void){
	BeepCnt ++;
	switch(BeepState){
		case 0:
			if(BeepNum){
				switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 200:	DDRB  &= 0x7f;				break;
					case 500:	BeepCnt = 0;	BeepNum --;	break;
					default:	break;
				}
			}
			break;
		case 1:// ��ʾæ 1�� 1��
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 200:	DDRB  &= 0x7f;				break;
					case 500:	DDRB  |= 0x80;				break;
					case 2000:	DDRB  &= 0x7f;				break;
					case 2500:	BeepCnt = 0; BeepState = 0;	break;
					default:	break;
				}
			break;
		case 2:// ��ʾ����  1����
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 2500:	DDRB  &= 0x7f;				break;
					case 3000:	BeepCnt = 0; BeepState = 0;	break;
					default:	break;
				}
			break;
		case 3:	// ����Ӧ��  1����
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 200:	DDRB  &= 0x7f;				break;
					case 500:	BeepCnt = 0; BeepState = 0;	break;
					default:	break;
				}
			break;
		case 4:	// �����ʾ  2����
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 200:	DDRB  &= 0x7f;				break;
					case 500:	DDRB  |= 0x80;				break;
					case 700:	DDRB  &= 0x7f;				break;
					case 1000:	BeepCnt = 0; BeepState = 0;	break;
					default:	break;
				}
			break;
		case 5:	// ���� ��������
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 2500:	DDRB  &= 0x7f;				break;
					case 3000:	BeepCnt = 0; 				break;
					default:	break;
				}
			break;
		default:
			break;
		}
}

// ����״ָ̬ʾ��
void SetStateLedBusy(void)
{
	DDRE |= 0x30;
	PORTE |= 0x10;
	PORTE &= 0xdf;
	if(_SampSW == 1)		// δ��������,LED���
		_LEDSTATE = 0;
}
void SetStateLedFree(void)
{
	DDRE |= 0x30;
	PORTE |= 0x20;
	PORTE &= 0xef;
	if(_SampSW == 1)		// δ��������,LED����
		_LEDSTATE = 1;
}
/****************************************************************************************************/


/********************************************* File end **********************************************/