#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"
#include "eeprom.h"
//*************************************************
extern unsigned char CardNoneUseful;	// �ϻ�����,����Ҫ��������Լ�Ƭ
unsigned char CardStoreTestFlag;		// Ƭ��ȡƬ����
extern unsigned int _DropMode;			// ����ģʽ,�����Ƿ���Ҫ������
extern unsigned char _MixtureMode;		// 1:1ģʽ��,�Ƿ���Ҫ������
extern unsigned char InsertRingFlag;
extern unsigned char insertflag[30];	// ����ת�̱�־
extern unsigned char workSwitch;			// ֹͣ��������
extern RING_QUEUE	RingQueue;
static unsigned char WaitPhotoFlag = 0;
static unsigned char ReScanfFlag = 0;
static unsigned int  CardScanfPos = 0;
extern unsigned long SecondCount;
extern TEST_QUEUE TestQueueA;
extern TEST_QUEUE TestQueueB;
//*************************************************
extern unsigned char GetNewTestCard;
extern unsigned char CurInsertRingNum;
extern unsigned char TurnPlateUsedLock;
extern unsigned char WorkProcessStep;
unsigned char CardStoreOpenLook = 0;
extern unsigned int _AutoTestCycleNum;

//
unsigned char preCardStoreNum = 1;		// ����ȡƬ������,Ƭ�ֵ��л�
static unsigned char stopFlag = 0;		//������ͣ����ȡƬ���������ʶ
static unsigned char preGetCardNum[5];	// ����ȡƬ������,����ÿ��Ƭ��ȡƬ����
extern unsigned char WithoutPuncture;	// Ĭ��Ϊ0 ����   1  �����̲�Ȼ�G   2 �����̲��ϻ�
unsigned char LastCardGetState;			// ��һ��ȡƬ�����Ƿ����

void SetGetCardTestMode(unsigned char m)
{
	if(m > 1)
		m = 1;
	CardStoreTestFlag = m;
	if(1 == CardStoreTestFlag)
	{
		CardNoneUseful = 0;		// ���������,�Զ��ر��ϻ�����
		_AutoTestCycleNum = 124; 	// 124
		LastCardGetState = 1;	
		preCardStoreNum = 1;	// Ĭ�ϴ�1��Ƭ�ֿ�ʼ
		SetDiluentRatio(1);		// ����ϡ�ͱ���Ϊ1:1
#ifdef Puncture
		WithoutPuncture = 2;	// ������		
#endif
		Uart0ReUnable;
		uart_Printf("%s $ 1\r\n",strM3125);
		Uart0ReEnable;
	}
	else
	{
		_AutoTestCycleNum = 0;
		Uart0ReUnable;
		uart_Printf("%s $ 0\r\n",strM3129);
		Uart0ReEnable;
	}
}

static void ReStartCardGetTest(void)
{
	unsigned char i;
	for(i = 0;i < 5;i++)
		preGetCardNum[i] = 0;
	preCardStoreNum = 1;
	LastCardGetState = 1;
	_AutoTestCycleNum = 124; 	// 124
}

void clearstopFlag(void)
{
	if(stopFlag == 1)
		stopFlag = 0;
}

static void CardGetTestDone(void)
{
	static unsigned char num;
	if(1 == stopFlag)	return;
	stopFlag = 0;
	LastCardGetState = 1;
	if(preCardStoreNum >= 1)
	{
		preGetCardNum[preCardStoreNum - 1]++;
		if(preGetCardNum[preCardStoreNum - 1] >= 25)  // 25
		{
			preCardStoreNum++;
			if(preCardStoreNum > 5)
				preCardStoreNum = 5;
		}
	}
	num = preGetCardNum[0] +  preGetCardNum[1] + preGetCardNum[2] + preGetCardNum[3] +  preGetCardNum[4];
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3128,num);
	Uart0ReEnable;
	if(num >= 125)	 // 125
	{
		ReStartCardGetTest();
		CardStoreTestFlag = 0;	 // �˳�ȡƬ����ģʽ
		LastCardGetState = 0;	 // ��һ��ȡƬ�������
		SetDiluentRatio(9);
#ifdef Puncture
		WithoutPuncture = 0;	// �ָ�����		
#endif
		Uart0ReUnable;
		uart_Printf("%s\r\n", strM3127);
		Uart0ReEnable;
	}
}

unsigned int CalCardStorePos(unsigned char num)
{
	unsigned int i;
	switch(num)
	{
		case 1:		i = 975-25;	break;			// 79mm / 0.081 = 975;
		case 2:		i = 1383-25;	break;		// (79+33)mm / 0.081 = 1383
		case 3:		i = 1790-25;	break;		// (79+66)mm / 0.081 = 1790
		case 4:		i = 2198-25;	break;		// (79+99)mm / 0.081 = 2198
		case 5:		i = 2605-45;	break;		// (79+132)mm / 0.081 = 2605
		default:	i = 975-25;	break;
	}
	return i;
}

unsigned char CardTrolleySta = 255;		// С����ʼ״̬ 0:С��ƽ��, 1:С������, 255:״̬δ�ȶ�
unsigned char CardTrolleySet=0;//С����ִ��״̬
void SetCardTrolleyState(unsigned char state)
{
	// state==0 �ر�; state==1 ����; state==2 ��ƽ
	CardTrolleySet = state;//
	switch(state)
	{
		case 0:		// �ر�
			PORTF &= 0xfc;		// Forward OFF 01111100
			break;
		case 1:		// ����
			PORTF &= 0xfc;
			PORTF |= 0x01;
			break;
		case 2:		// ��ƽ
			PORTF &= 0xfc;
			PORTF |= 0x02;
			break;
	}
	if(CardTrolleySet == 0)
	{
	//	PORTF &= 0xfe;		// Forward OFF 01111111
	//	PORTF |= 0x02;		// Reverse ON  00000010
		PORTF &= 0xfc;		// Forward OFF 01111100
	}
	else 
	{
		PORTF |= 0x01;		// Forward ON 11111100
	//	PORTF &= 0xfd;		// Reverse OFF 11111101
	}
}
unsigned char GetCardTrolleyState(void)
{
	return CardTrolleySta;
}
void CardTrolleyTurnProcess(void){		// ��ƬС����ת����
	unsigned char sw;
	if(PINL & 0x04){//�˿ڶ��� ��Ϊ0
	//	sw = 1;		// С��ƽ��
		CardTrolleySta = 1;
		}
	else{
	//	sw = 0;		// С������
		CardTrolleySta = 0;
		}
}


/********************************  Ƭ��״̬��������  ***************************************/
unsigned char CardStoreOpenState;	// Ƭ�ֿ���״̬
unsigned char CardSurplusState[6];		// ��Ƭʣ��״̬
unsigned char CardStoretate[6];		// Ƭ��״̬

void _SenCardStoreState(unsigned char num)
{
	switch(CardSurplusState[num])
	{
		case INFO_STORE_FULL:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n", strM3132, num+1);
			Uart0ReEnable;
			break;
		case INFO_STORE_LITTLE:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n", strM3133, num+1);
			Uart0ReEnable;
			break;
		case INFO_STORE_EMPTY:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n", strM3134, num+1);
			Uart0ReEnable;
			break;
		case INFO_STORE_ERROR:
		default:
			break;
	}
}
void ReportCardStoreState(unsigned char num)
{
	// ����λ�����浱ǰƬ��״̬
	unsigned char i;
	if(num == 0)
	{
		for(i=0; i<5; i++)
		{
			_SenCardStoreState(i);
		}
	}
	else
	{
		num --;
		_SenCardStoreState(num);
	}
}

_CONST unsigned char Num2Bit[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char CardStoreSteteProcess(INFO_EVENT * pInfoEvent)
{
	unsigned char num;
	unsigned char * pInfo;
	if(pInfoEvent->event != INFO_STORE_STATE_ALL)
	{
		num = pInfoEvent->info[0];
		if(num>5)
			return 0;
	}
	//num = 5-num;	// �ߵ�˳��
	switch(pInfoEvent->event)
	{
		case INFO_STORE_OPEN:		// Ƭ�ִ�
			CardStoreOpenState |= Num2Bit[num];
			CardStoretate[num] = INFO_STORE_OPEN;
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strM3130, num+1);
			Uart0ReEnable;
			break;
		case INFO_STORE_CLOSE:		// Ƭ�ֹر�
			CardStoreOpenState &= (0xff-Num2Bit[num]);
			CardStoretate[num] = INFO_STORE_CLOSE;
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n", strM3131, num+1);
			Uart0ReEnable;
			_SenCardStoreState(num);	// Ƭ�ֹرպ���Ƭ�ִ洢״̬
			break;
		case INFO_STORE_FULL:		// Ƭ����
			CardSurplusState[num] = pInfoEvent->event;
			CardStoretate[num] = INFO_STORE_FULL;
			break;
		case INFO_STORE_LITTLE:		// Ƭ������
			CardSurplusState[num] = pInfoEvent->event;
			CardStoretate[num] = INFO_STORE_LITTLE;
			break;
		case INFO_STORE_EMPTY:		// Ƭ�ֿ�
			CardSurplusState[num] = pInfoEvent->event;
			CardStoretate[num] = INFO_STORE_EMPTY;
			break;
		case INFO_STORE_ERROR:		// Ƭ��״̬����
			CardSurplusState[num] = pInfoEvent->event;
			CardStoretate[num] = INFO_STORE_ERROR;
			break;
		case INFO_STORE_STATE_ALL:	// ȫ��Ƭ��״̬��Ϣ
			pInfo = &(pInfoEvent->info[0]);
			for(num=0; num<6; num++)
			{
				if(*pInfo >= INFO_STORE_FULL && *pInfo <= INFO_STORE_ERROR)
				{
					CardSurplusState[num] = *pInfo;
				}
				pInfo ++;
			}
			break;
		case INFO_STORE_STATE_SPC:	// ָ��Ƭ��״̬��Ϣ
			pInfo = &(pInfoEvent->info[1]);
			if(*pInfo >= INFO_STORE_FULL && *pInfo <= INFO_STORE_ERROR)
			{
				CardSurplusState[num] = *pInfo;
			}
		default:
			break;
		}
	return 0;
}


/*********************************************************************************************/

static unsigned char CardstoreProcess_workStep,CardstoreProcess_mainStep;

void printf_CardstoreProcess_StepState(void)
{
	uart_Printf("*3203 CardStoreStepState $%2d $%2d\r\n",CardstoreProcess_mainStep,CardstoreProcess_workStep);
}


/********************************  ���Կ���ȡ��������  ***************************************/
extern unsigned char _GetCardMainStep, _GetCardWorkStep;
extern SAMP_INFO NewTestInfo;

unsigned char GetNewPieceFromStoreProcess(INFO_EVENT * pInfoEvent){
	static unsigned char mainStep;		
	static unsigned char workStep;
	static signed int storePos;
	static unsigned char inWork,i,j;
	static unsigned char waitMotCardTrolley, waitMotCardLoad,waitMotCardUnload, waitMotTurnPlate;
	static unsigned char oldStoreState;
	static unsigned char oldGetCardState;
	static unsigned char curCardStoreNum;
	static unsigned char checkReDoCnt, takeRedoCnt;
	signed int siTmp;
	unsigned char pos;
	unsigned char ucTmp;
	CardstoreProcess_mainStep = mainStep;
	CardstoreProcess_workStep = workStep;
	if(inWork)
	{
		if(waitMotCardTrolley)
		{
			if(WaitDelayTime(MOT_STORE_CARD_MOVE))
			{
				if(GetMotState(MOT_STORE_CARD_MOVE) != STA_SLAVE_FREE)	
					return 0;
				inWork = 0;
				waitMotCardTrolley = 0;
				SetDelayTime(MOT_STORE_CARD_MOVE,0);
			}
			else	// 5����ʱ��,ȡƬ��Ƭ
			{
				MotStop(MOT_STORE_CARD_MOVE);
				SetBeepWarning();
#ifndef UartSendLong
				Uart0ReUnable;
				uart_Printf("!3528 $%8d\r\n", NewTestInfo.testSerial);
				Uart0ReEnable;
#else
				Uart0ReUnable;
				uart_Printf("!3528 $ ");
				uart0SendInt(NewTestInfo.testSerial);
				uart_Printf("\r\n");
				Uart0ReEnable;
#endif
				inWork = 0;
				waitMotCardTrolley = 0;
				workSwitch = 0;
				Uart0ReUnable;
				uart_Printf("%s\r\n",strM0106);
				Uart0ReEnable;
				if(CardStoreTestFlag != 0)
				{
					ReStartCardGetTest();// ���¿�ʼ
					stopFlag = 1;
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3126);
					Uart0ReEnable;
					
				}
			}
		}
		
		if(waitMotCardLoad)
		{
			if(WaitDelayTime(MOT_CARD_LOAD))
			{
				if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)
					return 0;
				inWork = 0;
				waitMotCardLoad = 0;
				SetDelayTime(MOT_CARD_LOAD,0);
			}
			else	// 5����ʱ������Ƭ����λ��Ƭ
			{
				MotStop(MOT_CARD_LOAD);
				SetBeepWarning();	
#ifndef UartSendLong
				Uart0ReUnable;
				uart_Printf("!3529 $%8d\r\n", NewTestInfo.testSerial);
				Uart0ReEnable;
#else
				Uart0ReUnable;
				uart_Printf("!3529 $ ");
				uart0SendInt(NewTestInfo.testSerial);
				uart_Printf("\r\n");
				Uart0ReEnable;
#endif
				inWork = 0;
				waitMotCardLoad = 0;
				workSwitch = 0;
				Uart0ReUnable;
				uart_Printf("%s\r\n",strM0106);
				Uart0ReEnable;
				if(CardStoreTestFlag != 0)
				{
					ReStartCardGetTest();// ���¿�ʼ
					stopFlag = 1;
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3126);
					Uart0ReEnable;
				}
			}
		}
	}
	if(workSwitch == 0)							return 0;
	if(WaitDelayTime(MOT_STORE_CARD_MOVE))		return 0;
	if(WaitDelayTime(MOT_CARD_LOAD))			return 0;
	if(WaitDelayTime(MOT_CARD_UNLOAD))			return 0;
	if(waitMotCardTrolley){	if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)	return 0;waitMotCardTrolley = 0;}
	if(waitMotCardLoad){if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)	return 0;waitMotCardLoad = 0;}
	if(waitMotCardUnload){if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;waitMotCardUnload = 0;}
	if(waitMotTurnPlate){if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	return 0;waitMotTurnPlate = 0;}
	
	switch(mainStep)
	{
		case 0:	// ׼���������ж��Ƿ����µĲ��Կ�ʼ
			if(GetNewTestCard != 0 && GetNewTestCard < 6)
			{
				mainStep = 100;
				workStep = 0;
				curCardStoreNum = GetNewTestCard;
				MotInitCheck(MOT_STORE_CARD_MOVE);
				SetCardTrolleyState(0);	
				SetDelayTime(MOT_STORE_CARD_MOVE,10);
				waitMotCardTrolley = 1; 
				checkReDoCnt = 0;
				takeRedoCnt = 0;
				if(0 != CardStoreTestFlag)
					LastCardGetState = 0;
			}
			else
				return 1;	
			break;
		case 100:
			ucTmp = CardSurplusState[GetNewTestCard - 1];
			if(CardNoneUseful == 0)
			{
				if(ucTmp == INFO_STORE_EMPTY)
				{
					SetBeepWarning();
					Uart0ReUnable;
					uart_Printf("!3520 $%4d\r\n", GetNewTestCard);
					Uart0ReEnable;
					mainStep = 101;
				}
				else if(ucTmp == INFO_STORE_ERROR)
				{
					Uart0ReUnable;
					uart_Printf("!3522 $%4d\r\n", GetNewTestCard);
					Uart0ReEnable;
					mainStep = 101;
				}
				else
				{
					mainStep = 1;
					CardStoreOpenLook = 1;	
				}
			}
			else
			{
				mainStep = 1;
				CardStoreOpenLook = 1;
			}
			oldStoreState = ucTmp;
			break;
		case 101:
			ucTmp = CardSurplusState[GetNewTestCard-1];
			if(ucTmp != oldStoreState)
				mainStep = 100;
			break;
		case 1:		// С���Ƶ�Ƭ��ǰ
			switch(workStep)
			{
				case 0:
					storePos = 494;		// 40mm / 0.081 = 494
					SetMotRunPam(MOT_STORE_CARD_MOVE,200,20,CURRENT_STORE_MOVE);
					if(CardStoreOpenState & 0x3f)	// ���Ƭ�ֿ���
					{
						Uart0ReUnable;
						uart_Printf("!3525\r\n");
						Uart0ReEnable;
						workStep = 2;
						break;
					}
					MotRunTo(MOT_STORE_CARD_MOVE,storePos);			// 	
					workStep = 1;
					break;
				case 1:
					if(CardStoreOpenState & 0x3f)	// ���Ƭ�ֿ���
					{
						MotStop(MOT_STORE_CARD_MOVE);
						Uart0ReUnable;
						uart_Printf("!3525\r\n");
						Uart0ReEnable;
						workStep = 2;
					}
					else	// Ƭ�ִ��ڹر�״̬���ȴ�Ƭ��С��ֹͣ
					{
						if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)
							break;
						else
							workStep = 3;
					}
					break;
				case 2:		// Ƭ�ִ��ڿ���״̬���ȴ�Ƭ�ֹرպ��������Ƭ�ֵ��
					if((CardStoreOpenState & 0x3f) == 0)
					{
						siTmp = GetMotPositionOfStep(MOT_STORE_CARD_MOVE) + 5;
						if(siTmp<storePos)
						{
							MotRunTo(MOT_STORE_CARD_MOVE,storePos);
							SetDelayTime(MOT_STORE_CARD_MOVE,1);
							workStep = 1;
						}
						else
						{
							workStep = 3;
						}
					}
					break;
				case 3:	// Ƭ��ȡƬС�������е�Ƭ����ʼ��
					SetDelayTime(MOT_STORE_CARD_MOVE,10);
				//	CardStoreOpenLook = 1;
					workStep = 0;
					mainStep = 2;
					break;
				}
			break;
		case 2:		// С���ƶ���ȡƬ��ʼλ��
			switch(workStep)
			{
				case 0:
					if(CardStoreOpenState & 0x3f)
					{
						Uart0ReUnable;
						uart_Printf("!3526\r\n");
						Uart0ReEnable;
						workStep = 2;
						break;
					}
					storePos = CalCardStorePos(GetNewTestCard);	
					SetMotRunPam(MOT_STORE_CARD_MOVE,200,20,CURRENT_STORE_MOVE);
					MotRunTo(MOT_STORE_CARD_MOVE,storePos);			// 	
					workStep = 1;
					break;
				case 1:
					if(CardStoreOpenState & 0x3f)
					{
						MotStop(MOT_STORE_CARD_MOVE);
						Uart0ReUnable;
						uart_Printf("!3526\r\n");
						Uart0ReEnable;
						workStep = 2;
					}
					else
					{
						if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)
							break;
						else
							workStep = 3;
					}
					break;
				case 2:		// Ƭ�ִ��ڿ���״̬���ȴ�Ƭ�ֹرպ��������Ƭ�ֵ��
					if((CardStoreOpenState & 0x3f) == 0)
					{
						siTmp = GetMotPositionOfStep(MOT_STORE_CARD_MOVE) + 5;
						if(siTmp<storePos)
						{
							MotRunTo(MOT_STORE_CARD_MOVE,storePos);
							SetDelayTime(MOT_STORE_CARD_MOVE,1);
							workStep = 1;
						}
						else
						{
							workStep = 3;
						}
					}
					break;
				case 3:	// Ƭ��ȡƬС�������е�Ŀ�ĵ�
					MotInitCheck(MOT_CARD_LOAD);
					waitMotCardLoad = 1;
					SetDelayTime(MOT_STORE_CARD_MOVE,5);
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_CARD_LOAD,-25);
					waitMotCardLoad = 1;
					workStep = 0;
					mainStep = 3;
					break;
				}
			break;
		case 3:// ��Ƭ̧��С����Ƭǰ����ʹ֮���빳Ƭ����
			switch(workStep)
			{
				/*
				case 0:	
					SetCardTrolleyState(1);
					SetMotRunPam(MOT_STORE_CARD_MOVE,16,10,CURRENT_STORE_MOVE);
					MotRun(MOT_STORE_CARD_MOVE,-98);
					waitMotCardTrolley = 1;
					workStep = 1;
					break;
				*/
				case 0:
					SetCardTrolleyState(1);
					SetDelayTime(MOT_STORE_CARD_MOVE,2);	// ��ʱ200ms
					workStep = 10;
				case 10:
					SetMotRunPam(MOT_STORE_CARD_MOVE,16,10,CURRENT_STORE_MOVE);
					MotRun(MOT_STORE_CARD_MOVE,-98);
					SetDelayTime(MOT_STORE_CARD_MOVE,2);	// ��ʱ200ms
					waitMotCardTrolley = 1;
					workStep = 1;
					break;
				case 1:
					SetCardTrolleyState(0);
					SetBeepAck();
					workStep = 2;
					break;
				case 2:		
					SetMotRunPam(MOT_STORE_CARD_MOVE,100,10,CURRENT_STORE_MOVE);
					MotRun(MOT_STORE_CARD_MOVE,-(235 + 70));	
					waitMotCardTrolley = 1;
				//	workStep = 3;
					workStep = 7;
					break;
				case 3:
					MotRun(MOT_STORE_CARD_MOVE,86);		
					waitMotCardTrolley = 1;
					workStep = 4;
					break;
				case 4:
					MotRun(MOT_STORE_CARD_MOVE,-86);			
					waitMotCardTrolley = 1;
					workStep = 5;
					break;
				case 5:
					MotRun(MOT_STORE_CARD_MOVE,99);	
					waitMotCardTrolley = 1;
					workStep = 6;
					break;
				case 6:
					MotRun(MOT_STORE_CARD_MOVE,-150);
					waitMotCardTrolley = 1;
					workStep = 7;
					break;
				case 7:
					SetDelayTime(MOT_STORE_CARD_MOVE,10);
					workStep = 8;
					break;
				case 8:
				//	SetCardTrolleyState(2);
					MotRun(MOT_STORE_CARD_MOVE,320);
					waitMotCardTrolley = 1;
					workStep = 9;
					break;
				case 9:
				//	SetCardTrolleyState(0);
					workStep = 0;
					mainStep = 5;
					break;
				default:
					break;
				}
			break;
		case 5:	// ���е��뿪Ƭ��λ��
			switch(workStep){
				case 0:
					SetMotRunPam(MOT_STORE_CARD_MOVE,200,20,CURRENT_STORE_MOVE);
					if(CardStoreOpenState & 0x3f)
					{	
						Uart0ReUnable;
						uart_Printf("!3527\r\n");
						Uart0ReEnable;
						workStep = 2;
						break;
					}
					if(WaitPhotoFlag == 0)
						MotRunTo(MOT_STORE_CARD_MOVE,494);	
					else
						MotRunTo(MOT_STORE_CARD_MOVE,0);	
					workStep = 1;
					break;
				case 1:
					if(CardStoreOpenState & 0x3f)
					{
						MotStop(MOT_STORE_CARD_MOVE);
						Uart0ReUnable;
						uart_Printf("!3527\r\n");
						Uart0ReEnable;
						workStep = 2;
					}
					else
					{	
						if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)
							break;
						else
						{
							if(WaitPhotoFlag == 0)
								workStep = 3;
							else
							{
								//workStep = 20;
								//SetMotRunPam(MOT_STORE_CARD_MOVE,50,5,CURRENT_STORE_MOVE);
								//workStep = 26;	// 2016-11-18
								workStep = 20;		// 2016-11-18
							}
						}
					}
					break;
				case 2:		
					if((CardStoreOpenState & 0x3f) == 0)
					{	
						siTmp = GetMotPositionOfStep(MOT_STORE_CARD_MOVE);
						if(WaitPhotoFlag == 0)
						{
							if(siTmp>(494+20))
							{
								MotRunTo(MOT_STORE_CARD_MOVE,494);
								SetDelayTime(MOT_STORE_CARD_MOVE,1);
								workStep = 1;
							}
							else
							{
								workStep = 3;
								SetMotRunPam(MOT_STORE_CARD_MOVE,100,20,CURRENT_STORE_MOVE);
							}
						}
						else
						{
							if(siTmp > 0)
							{
								MotRunTo(MOT_STORE_CARD_MOVE,0);
								SetDelayTime(MOT_STORE_CARD_MOVE,1);
								workStep = 1;
							}
							else
							{
								//workStep = 20;
								//SetMotRunPam(MOT_STORE_CARD_MOVE,50,5,CURRENT_STORE_MOVE);
								//workStep = 26;	// 2016-11-18
								workStep = 20;		// 2016-11-18
							}
						}
					}
					break;
				case 20:
					SetMotRunPam(MOT_STORE_CARD_MOVE,50,5,CURRENT_STORE_MOVE);
					MotRunTo(MOT_STORE_CARD_MOVE,70);
					waitMotCardTrolley = 1;
					workStep = 21;
					break;
				case 21:
					SetMotRunPam(MOT_CARD_LOAD,30,5,CURRENT_CARD_LOAD);
					//MotRunTo(MOT_CARD_LOAD,55);
					MotRunTo(MOT_CARD_LOAD,30);
					waitMotCardLoad = 1;
					workStep = 22;
					break;
				case 22:
					SetDelayTime(MOT_CARD_LOAD,5);
					workStep = 23;
					break;
				case 23:
					MotRunTo(MOT_CARD_LOAD,20);
					waitMotCardLoad = 1;
					SetDelayTime(MOT_CARD_LOAD,5);
					workStep = 24;
					break;
				case 24:
					siTmp = GetCardScanfPos(); 
					SetMotRunPam(MOT_STORE_CARD_MOVE,250,20,CURRENT_STORE_MOVE);
					MotRunTo(MOT_STORE_CARD_MOVE,(unsigned int)siTmp);
				//	MotRunTo(MOT_STORE_CARD_MOVE,305);
					waitMotCardTrolley = 1;
					workStep = 25;
					break;
				case 25:
					Uart0ReUnable;
					uart_Printf("%s $%4d\r\n",strM3146,NewTestInfo.cardStoreNum);
					Uart0ReEnable;
					SetMotRunPam(MOT_CARD_LOAD,140,10,CURRENT_CARD_LOAD);
					MotRunTo(MOT_CARD_LOAD,0);
					SetDelayTime(waitCardBarcode,50);	// 5s�ȴ�����
					workStep = 3;
					break;
				case 26:
					SetDelayTime(MOT_STORE_CARD_MOVE,5);
					workStep = 27;
				case 27:
					SetMotRunPam(MOT_CARD_LOAD,140,10,CURRENT_CARD_LOAD);
					MotRunTo(MOT_CARD_LOAD,300);	// 15/0.08012 = 184
					waitMotCardLoad = 1;
					workStep = 28;
					break;
				case 28:
					MotRunTo(MOT_CARD_LOAD,0);
					waitMotCardLoad = 1;
					workStep = 29;
					break;
				case 29:
					SetDelayTime(MOT_STORE_CARD_MOVE,2);
					workStep = 20;
					break;
				case 3:
					if(WaitPhotoFlag)
					{
						if(WaitDelayTime(waitCardBarcode))
						{
							if(GetReCardScanf())	// ��Ҫ���¼��
							{
								SetDelayTime(waitCardBarcode,0);
								SetReCardScanf(0);
								MotRunTo(MOT_STORE_CARD_MOVE,0);
								waitMotCardTrolley = 1;
								// workStep = 26;	// 2016-11-18
								workStep = 20;	// 2016-11-18
							}
							break;
						}
					}
					oldGetCardState = PINL & 0x04;	
					MotRunTo(MOT_STORE_CARD_MOVE,0);	
					waitMotCardTrolley = 1;
					if(inWork == 0)
					{
						inWork = 1;							
						SetDelayTime(MOT_STORE_CARD_MOVE,50);
					}
					workStep = 4;
					break;
				case 4:	// Ƭ��ȡƬС�������е���ʼλ
					SetDelayTime(MOT_STORE_CARD_MOVE,10);
					workStep = 5;
					break;
				case 5:	// �����Կ��Ƿ�ɹ�ȡ��
					ucTmp = PINL & 0x04;		
					//*************************************************
					if(CardNoneUseful == 0)
					{
						if(ucTmp == 0 && oldGetCardState == 0)
						{
							if(checkReDoCnt < 2)
							{
								MotRunTo(MOT_STORE_CARD_MOVE,100);	
								waitMotCardTrolley = 1;
								checkReDoCnt ++;
								workStep = 3;
								break;
							}
							if(takeRedoCnt < 1)
							{
								takeRedoCnt ++;
								workStep = 0;
								mainStep = 1;
								break;
							}
							// ȡƬʧ��
							SetBeepWarning();
							if(CardStoreTestFlag != 0)
							{
								ReStartCardGetTest();// ���¿�ʼ
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3126);
								Uart0ReEnable;
							}
#ifndef UartSendLong
							Uart0ReUnable;
							uart_Printf("!3540 $%8d", NewTestInfo.testSerial);
							uart_Printf(" $%4d $%4d\r\n",oldGetCardState,ucTmp);
							Uart0ReEnable;
#else
							Uart0ReUnable;
							uart_Printf("!3540 $ ");
							uart0SendInt(NewTestInfo.testSerial);
							uart_Printf(" $%4d $%4d\r\n",oldGetCardState,ucTmp);
							uart_Printf("\r\n");
							Uart0ReEnable;
#endif
						
							GetNewTestCard = 250;	
							CardStoreOpenLook = 0;	
							InsertRingFlag = 1;
							workStep = 0;
							mainStep = 0;
							break;
						}
						else if(ucTmp == 4 && oldGetCardState == 0)
						{
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n",strM3105,oldGetCardState,ucTmp); 
							Uart0ReEnable;
							workStep = 6;
							oldGetCardState = ucTmp;
							break;
						}
					}
					else
						workStep = 6;
					//*********************************************************	
					//if(ucTmp == 4 && oldGetCardState == 0)
					//	oldGetCardState = ucTmp;
					//workStep = 6;
					break;
				case 6:	// ���Կ��ƶ�����Һλ��
					SetMotRunPam(MOT_CARD_LOAD,140,10,CURRENT_CARD_LOAD);
					MotRunTo(MOT_CARD_LOAD,300);	// 15/0.08012 = 184
					workStep = 7;
					break;
				case 7:	// ����׼���ٴ��ƶ�����Һλ�ã���ֹ���Կ�Ƭ����б��
					MotRunTo(MOT_CARD_LOAD,0);
					workStep = 8;
					break;
				case 8:	
					_SenCardStoreState(GetNewTestCard-1);
					SetMotRunPam(MOT_CARD_LOAD,140,10,CURRENT_CARD_LOAD);
					if(_DropMode == 1)	
					{
						if(0 == _MixtureMode)// ���Կ��ƶ����鵽����λ��
						{
							MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_MIX);		// 115
							waitMotCardLoad = 1;
							workStep = 9;
						}
						else// ���Կ��ƶ�������λ��
						{
							CardStoreOpenLook = 0;		 
							workStep = 10;
						}
					}
					else	// ���Կ��ٴ��ƶ�����Һλ��
					{
						MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_DROP);
						waitMotCardLoad = 1;
						workStep = 0;
						mainStep = 6;
					}
					break;
				case 9:
					Uart0ReUnable;
					uart_Printf("%s $%4d\r\n",strM3109,curCardStoreNum);	
					Uart0ReEnable;
					GetNewTestCard = 222;		// ����ȡ�����Կ���ʶ,���Կ�ʼ������
					CardStoreOpenLook = 0;		 
					workStep = 10;
					break;
				case 10:
					if(GetNewTestCard == 233)	// �����Ƚ���,׼������
					{
						MotRunTo(MOT_CARD_LOAD,0);
						waitMotCardLoad = 1;
					//	mainStep = 6;
					//	workStep = 0;
						workStep = 11;
					}
					break;
				// 2016-12-06 ���ߵ� ӫ�����λ��
				case 11:
					MotRunTo(MOT_CARD_LOAD,-25);
					waitMotCardLoad = 1;
					mainStep = 6;
					workStep = 0;
					break;
				}
			break;
		case 6:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strM3106,curCardStoreNum);	
			Uart0ReEnable;
			GetNewTestCard = 254;			// ����ȡ�����Կ���ʶ
			CardStoreOpenLook = 0;		// 
			mainStep = 7;
			break;
		case 7:		// �ȴ�������Ϻ󽫸�Ƭ����ת��
			if(GetNewTestCard == 255)
			{
				SetDelayTime(MOT_CARD_LOAD, 50);
				mainStep = 8;
				workStep = 0;
			}break;
		case 8:		// ������걾�ĸ�Ƭװ��ת��
			switch(workStep)
			{
				case 0:	
					if(TurnPlateUsedLock == 0)
					{
						TurnPlateUsedLock = 1;		// ռ��ת�̱�ʶ
						workStep = 1;
					}
					break;
				case 1:		// ����ת��ת����λ
					SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);
					MotRunToSite(MOT_TURN_PLATE,0);		// ת�����е���λ
					workStep = 20;
					waitMotTurnPlate = 1;
					break;
				case 20:
					SetDelayTime(MOT_TURN_PLATE,5);
					workStep = 2;
					break;
				case 2:		// ����ת��ת����ǰλ��
					//SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);	
					MotRunToSite(MOT_TURN_PLATE,CurInsertRingNum);
					waitMotTurnPlate = 1;
#ifndef LoadCheck
					workStep = 3;  // ��Ƭ���ʹ��
#else
					workStep = 30;	 // ��Ƭ����ʹ��
#endif
					break;
				case 30:
					SetDelayTime(MOT_TURN_PLATE,5);
					workStep = 31;
					break;
				case 31:
					siTmp = getLiqDetADC(LoadChannel);
					if(siTmp < CardLocationAD)		// ���Լ�Ƭ
					{
						workStep = 3;
						Uart0ReUnable;
						uart_Printf("%s $ %d\r\n",strM3195,(unsigned int)siTmp);
						Uart0ReEnable;
					}
					else 							// ���Լ�Ƭ
					{
						workStep = 32;
						Uart0ReUnable;
						uart_Printf("%s $ %d\r\n",strE3934,(unsigned int)siTmp);
						Uart0ReEnable;
						i++;
						if(i > 2)
						{
							i = 0;
							workStep = 3;	// ��Ƭ�����𻵻�жƬ������
							Uart0ReUnable;
							uart_Printf("%s $%4d\r\n",strE4910,CurInsertRingNum);		// ����3�ν�Ƭ�������
							Uart0ReEnable;
						}
					}
					break;
				case 32:
					// ����λ���ƶ�����Ƭλ
					siTmp = CurInsertRingNum + 25;
					if(siTmp >= RING_QUEUE_NUM)
						siTmp -= RING_QUEUE_NUM;
					MotRunToSite(MOT_TURN_PLATE,siTmp);		// ת��ת����ǰλ��
					waitMotTurnPlate = 1;
					workStep = 33;
					break;
				case 33:
					SetDelayTime(MOT_TURN_PLATE,5);
				//	workStep = 34;
					workStep = 35;
					break;
				/*
				case 34:
					siTmp = getLiqDetADC(UnloadChannel);
					if(siTmp < CardLocationAD)	// ��ǰλ��û���Լ�Ƭ
					{
						MotRunToSite(MOT_TURN_PLATE,CurInsertRingNum);
						waitMotTurnPlate = 1;
						//workStep = 3;
						workStep = 30;
					}
					else 			// ���Լ�Ƭ
					{
						workStep = 35;
					}		
					break;
				*/
				case 35:
					if(GetwasteCardState() == 0)// ��Ƭ�ֹ��ܿ���
					{
						if((PINK & 0x02) == 0)
						{		// ��Ƭ�ִ�
							TurnPlateUsedLock = 0;
							mainStep = 8;
							workStep = 0;
							break;
						}
					}
					SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
					MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);		// жƬ�г�67mm/0.08128 = 824
					waitMotCardUnload = 1;
					workStep = 36;
					break;
				case 36:
					SetMotRunPam(MOT_CARD_UNLOAD,100,20,2);
					MotRunTo(MOT_CARD_UNLOAD,0);
					waitMotCardUnload = 1;
					workStep = 37;
					break;
				case 37:
					SetDelayTime(MOT_CARD_UNLOAD,5);
					workStep = 38; // �����Ƭ
					break;
				case 38:	// ��Ƭ�������֮��,����Ƿ����Լ�Ƭ
					siTmp = getLiqDetADC(UnloadChannel);
					if(siTmp < CardLocationAD)	// ��ǰλ��û���Լ�Ƭ
					{
						MotRunToSite(MOT_TURN_PLATE,CurInsertRingNum);
						waitMotTurnPlate = 1;
						//workStep = 3;
						workStep = 30;
					}
					else 						// ���Լ�Ƭ
					{
						//workStep = 1;
						MotRunToSite(MOT_TURN_PLATE,0);		// ת�����е���λ
						waitMotTurnPlate = 1;
						workStep = 32;
						Uart0ReUnable;
						uart_Printf("%s $%4d $%4d\r\n",strM3117,CurInsertRingNum,j + 2);		// ��Ӧλ���ٴ���Ƭ
						Uart0ReEnable;
						j++;
						if(j > 2)
						{
							j = 0;
							workStep = 39;	// ��Ƭ�������Ƭ������
							Uart0ReUnable;
							uart_Printf("%s $%4d\r\n",strE4909,CurInsertRingNum);		// ��Ӧλ������3����Ƭʧ��
							Uart0ReEnable;
						}
					}
					break;	
				case 39:	// ����3����Ƭ�����ԣ������ж��ǽ�Ƭ����Ƭ������,��Ȼִ�н�Ƭ����
					MotRunToSite(MOT_TURN_PLATE,CurInsertRingNum);
					waitMotTurnPlate = 1;
					workStep = 3;	
					break;								
				case 3:		
					i = 0;
					j = 0;
					SetDelayTime(MOT_CARD_LOAD,5);
					workStep = 111;
					break;
					
				case 111:	
					//if(GetTurnPlateMonitorState() == 0)//0��ʾ����ͨ��1��ʾ�����ڵ� 
					if(GetMotorMonitorState(MOT_TURN_PLATE,StopMonitor) == 0)
						workStep = 4;
					else 
						workStep = 1;
					break;
				case 4:	// ��Ƭ����ת��
				//	SetMotRunPam(MOT_CARD_LOAD,120,2,3);
					MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_RING);		// װƬ�г�94mm/0.08128 = 1156
					Uart0ReUnable;
					uart_Printf("*3918 CardLoadBegain\r\n");	
					Uart0ReEnable;
					waitMotCardLoad = 1;
					workStep = 7;
					break;
				case 7:		// ��Ƭװ��ת����ɣ���Ƭ����С���ص���ʼλ
				//	SetMotRunPam(MOT_CARD_LOAD,160,10,4);
					SetMotRunPam(MOT_CARD_LOAD,220,10,CURRENT_CARD_LOAD);
					Uart0ReUnable;
					uart_Printf("*3919 CardLoadBack\r\n");	
					Uart0ReEnable;
					MotRunTo(MOT_CARD_LOAD,0);
					waitMotCardLoad = 1;
					if(inWork == 0)
					{
						inWork = 1;
						SetDelayTime(MOT_CARD_LOAD,50);
					}
					workStep = 12;
					break;
				case 12:
					MotRunTo(MOT_CARD_LOAD,0);
					SetDelayTime(MOT_CARD_LOAD,1);
					waitMotCardLoad = 1;
					workStep = 13;
					break;
				case 13:
					MotRunTo(MOT_CARD_LOAD,0);
					SetDelayTime(MOT_CARD_LOAD,1);
					waitMotCardLoad = 1;
					workStep = 11;
					break;
				case 11:
					siTmp = GetMotPositionOfStep(MOT_CARD_LOAD);
					if(siTmp != 0)
					{
						MotRunTo(MOT_CARD_LOAD,0);
						waitMotCardLoad = 1;
						break;
					}
					if(CardNoneUseful == 0)
						workStep = 80;
					else
						workStep = 8;
					Uart0ReUnable;
					uart_Printf("*3920 CardLoadBackDone\r\n");
					Uart0ReEnable;
					break;
				//***************************************************
				//2016-06-12 ���ӻ���֮�����¼�⣬�Լ�Ƭ�Ƿ�����ת��
				case 80:
					ucTmp = PINL & 0x04;	
					Uart0ReUnable;
					uart_Printf("%s $%4d $%4d\r\n",strM3149,oldGetCardState,ucTmp);	
					Uart0ReEnable;
					//if(ucTmp == oldGetCardState)	//�Լ�Ƭ����
					if(ucTmp == 4 && oldGetCardState == 4)	//�Լ�Ƭ����
					{
						//TurnPlateUseLock = 0;
						MotRunTo(MOT_TURN_PLATE,0);
						waitMotTurnPlate = 1;
						workStep = 1;
					}
					else
						workStep = 8;
					break;					
				//***************************************************
				case 8:		// ���
					workStep = 9;
					TurnPlateUsedLock = 0;			
					SetRingQueueUnitUsed(CurInsertRingNum);		// ���ø�Ƭ�Ѿ�װ��ת�̱�ʶ
					InsertRingFlag = 1;
					if(insertflag[CurInsertRingNum] == 0)
						insertflag[CurInsertRingNum] = 255;
#ifndef UartSendLong					
					Uart0ReUnable;
					//uart_Printf("%s $%8d",strM3147,NewTestInfo.testSerial); //2016-07-07
					uart_Printf("%s $%8d",strM3147,RingQueue.sampInfo[CurInsertRingNum].testSerial); //2016-09-13
#else
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3147);
					uart0SendInt(RingQueue.sampInfo[CurInsertRingNum].testSerial);
#endif
					uart_Printf(" $%4d $%4d\r\n",CurInsertRingNum,insertflag[CurInsertRingNum]); //2016-07-07
					Uart0ReEnable;
					SetMotRunPam(MOT_STORE_CARD_MOVE,200,20,CURRENT_STORE_MOVE);
					break;
				case 9:		// ȡƬС�����е�Ƭ�ֿ�
					MotRunTo(MOT_STORE_CARD_MOVE,432);
					waitMotCardTrolley = 1;
					workStep = 10;
					break;
				case 10:
					if(0 != CardStoreTestFlag)
					{
						CardGetTestDone();	// �������
					}
					workStep = 0;
					mainStep = 0;
					inWork = 0;
					return 1;
					break;
				}
			break;
		default:
			break;
		}
	return 0;
}

unsigned char GetStoreProcess(void)
{
	static unsigned char mainStep;		
	static unsigned char workStep;
	static signed int storePos;
	static unsigned char inWork;
	static unsigned char waitMotCardTrolley, waitMotCardLoad, waitMotTurnPlate;
	static unsigned char oldStoreState;
	static unsigned char oldGetCardState;
	static unsigned char curCardStoreNum;
	static unsigned char checkReDoCnt, takeRedoCnt;
	signed int siTmp;
	unsigned char pos;
	unsigned char ucTmp;
	if(WaitDelayTime(MOT_STORE_CARD_MOVE))		return 0;
	if(WaitDelayTime(MOT_CARD_LOAD))		return 0;
	if(waitMotCardTrolley){	if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)	return 0;waitMotCardTrolley = 0;}
	if(waitMotCardLoad){if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)	return 0;waitMotCardLoad = 0;}
	if(waitMotTurnPlate){if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	return 0;waitMotTurnPlate = 0;}
	
	switch(mainStep)
	{
		case 0:	
			MotInitCheck(MOT_CARD_LOAD);
			waitMotCardLoad = 1;
			mainStep = 99;
			break;
		case 99:
			mainStep = 100;
			MotInitCheck(MOT_STORE_CARD_MOVE);
			SetDelayTime(MOT_STORE_CARD_MOVE,10);
			waitMotCardTrolley = 1; 
			break;
		case 100:
			ucTmp = CardSurplusState[GetNewTestCard-1];
			if(CardNoneUseful == 0)
			{
				if(ucTmp == INFO_STORE_EMPTY)
				{
					SetBeepWarning();
					Uart0ReUnable;
					uart_Printf("!3520 $%4d\r\n", GetNewTestCard);
					Uart0ReEnable;
					mainStep = 101;
				}
				else if(ucTmp == INFO_STORE_ERROR)
				{
					Uart0ReUnable;
					uart_Printf("!3522 $%4d\r\n", GetNewTestCard);
					Uart0ReEnable;
					mainStep = 101;
				}
				else
				{
					mainStep = 1;
					CardStoreOpenLook = 1;	
				}
			}
			else
			{
				mainStep = 1;
				CardStoreOpenLook = 1;
			}
			oldStoreState = ucTmp;
			break;
		case 101:
			ucTmp = CardSurplusState[GetNewTestCard-1];
			if(ucTmp != oldStoreState)
				mainStep = 100;
			break;
		case 1: 	// С���Ƶ�Ƭ��ǰ
			switch(workStep)
			{
				case 0:
					storePos = 494;		// 40mm / 0.081 = 494
					SetMotRunPam(MOT_STORE_CARD_MOVE,200,20,CURRENT_STORE_MOVE);
					if(CardStoreOpenState & 0x3f) // ���Ƭ�ֿ���
					{	
						Uart0ReUnable;
						uart_Printf("!3525\r\n");
						Uart0ReEnable;
						workStep = 2;
						break;
					}
					MotRunTo(MOT_STORE_CARD_MOVE,storePos);			// 	
					workStep = 1;
					break;
				case 1:
					if(CardStoreOpenState & 0x3f)	// ���Ƭ�ֿ���
					{	
						MotStop(MOT_STORE_CARD_MOVE);
						Uart0ReUnable;
						uart_Printf("!3525\r\n");
						Uart0ReEnable;
						workStep = 2;
					}
					else
					{	// Ƭ�ִ��ڹر�״̬���ȴ�Ƭ��С��ֹͣ
						if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)
							break;
						else
							workStep = 3;
					}
					break;
				case 2:		// Ƭ�ִ��ڿ���״̬���ȴ�Ƭ�ֹرպ��������Ƭ�ֵ��
					if((CardStoreOpenState & 0x3f) == 0)
					{
						siTmp = GetMotPositionOfStep(MOT_STORE_CARD_MOVE) + 5;
						if(siTmp<storePos)
						{
							MotRunTo(MOT_STORE_CARD_MOVE,storePos);
							SetDelayTime(MOT_STORE_CARD_MOVE,1);
							workStep = 1;
						}
						else
						{
							workStep = 3;
						}
					}
					break;
				case 3:	// Ƭ��ȡƬС�������е�Ƭ����ʼ��
					SetDelayTime(MOT_STORE_CARD_MOVE,10);
					workStep = 0;
					mainStep = 2;
					break;
			}
			break;
		case 2:		// С���ƶ���ȡƬ��ʼλ��
			switch(workStep)
			{
				case 0:
					if(CardStoreOpenState & 0x3f)
					{
						Uart0ReUnable;
						uart_Printf("!3526\r\n");
						Uart0ReEnable;
						workStep = 2;
						break;
					}
					storePos = CalCardStorePos(GetNewTestCard);	
					SetMotRunPam(MOT_STORE_CARD_MOVE,200,20,CURRENT_STORE_MOVE);
					MotRunTo(MOT_STORE_CARD_MOVE,storePos);			// 	
					workStep = 1;
					break;
				case 1:
					if(CardStoreOpenState & 0x3f)
					{
						Uart0ReUnable;
						MotStop(MOT_STORE_CARD_MOVE);
						uart_Printf("!3526\r\n");
						Uart0ReEnable;
						workStep = 2;
					}
					else
					{
						if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)
							break;
						else
							workStep = 3;
					}
					break;
				case 2:		// Ƭ�ִ��ڿ���״̬���ȴ�Ƭ�ֹرպ��������Ƭ�ֵ��
					if((CardStoreOpenState & 0x3f) == 0)
					{
						siTmp = GetMotPositionOfStep(MOT_STORE_CARD_MOVE) + 5;
						if(siTmp<storePos)
						{
							MotRunTo(MOT_STORE_CARD_MOVE,storePos);
							SetDelayTime(MOT_STORE_CARD_MOVE,1);
							workStep = 1;
						}
						else
						{
							workStep = 3;
						}
					}
					break;
				case 3:	// Ƭ��ȡƬС�������е�Ŀ�ĵ�
					MotInitCheck(MOT_CARD_LOAD);
					SetDelayTime(MOT_STORE_CARD_MOVE,5);
					workStep = 0;
					mainStep = 3;
					break;
			}
			break;
		case 3:// ��Ƭ̧��С����Ƭǰ����ʹ֮���빳Ƭ����
			switch(workStep)
			{
				case 0:	
					SetCardTrolleyState(1);
					SetMotRunPam(MOT_STORE_CARD_MOVE,16,10,CURRENT_STORE_MOVE);
					MotRun(MOT_STORE_CARD_MOVE,-98);
					waitMotCardTrolley = 1;
					workStep = 1;
					break;
				case 1:
					SetCardTrolleyState(0);
					workStep = 2;
					break;
				case 2:		
					SetMotRunPam(MOT_STORE_CARD_MOVE,100,10,CURRENT_STORE_MOVE);
					MotRun(MOT_STORE_CARD_MOVE,-(235+70));	
					waitMotCardTrolley = 1;
				//	workStep = 3;
					workStep = 7;
					break;
				case 3:
					MotRun(MOT_STORE_CARD_MOVE,86);		
					waitMotCardTrolley = 1;
					workStep = 4;
					break;
				case 4:
					MotRun(MOT_STORE_CARD_MOVE,-86);			
					waitMotCardTrolley = 1;
					workStep = 5;
					break;
				case 5:
					MotRun(MOT_STORE_CARD_MOVE,99);	
					waitMotCardTrolley = 1;
					workStep = 6;
					break;
				case 6:
					MotRun(MOT_STORE_CARD_MOVE,-150);
					waitMotCardTrolley = 1;
					workStep = 7;
					break;
				case 7:
					SetDelayTime(MOT_STORE_CARD_MOVE,10);
					workStep = 8;
					break;
				case 8:
					SetCardTrolleyState(2);
					MotRun(MOT_STORE_CARD_MOVE,320);
					waitMotCardTrolley = 1;
					workStep = 9;
					break;
				case 9:
					SetCardTrolleyState(0);
					workStep = 0;
					mainStep = 5;
					break;
				default:
					break;
			}
			break;
		case 5:	// ���е��뿪Ƭ��λ��
			switch(workStep)
			{
				case 0:
					SetMotRunPam(MOT_STORE_CARD_MOVE,200,20,CURRENT_STORE_MOVE);
					if(CardStoreOpenState & 0x3f)
					{	
						Uart0ReUnable;
						uart_Printf("!3527\r\n");
						Uart0ReEnable;
						workStep = 2;
						break;
					}
					MotRunTo(MOT_STORE_CARD_MOVE,0);	
					workStep = 1;
					break;
				case 1:
					if(CardStoreOpenState & 0x3f)
					{
						MotStop(MOT_STORE_CARD_MOVE);
						Uart0ReUnable;
						uart_Printf("!3527\r\n");
						Uart0ReEnable;
						workStep = 2;
					}
					else
					{	
						if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)
							break;
						else
						{
							workStep = 4;
							SetMotRunPam(MOT_STORE_CARD_MOVE,50,5,CURRENT_STORE_MOVE);
						}
					}
					break;
				case 2:		
					if((CardStoreOpenState & 0x3f) == 0)
					{	
						siTmp = GetMotPositionOfStep(MOT_STORE_CARD_MOVE);
						if(siTmp > 0)
						{
							MotRunTo(MOT_STORE_CARD_MOVE,0);
							SetDelayTime(MOT_STORE_CARD_MOVE,1);
							workStep = 1;
						}
						else
						{
							workStep = 4;
						}
					}
					break;
				case 4:
						SetMotRunPam(MOT_STORE_CARD_MOVE,50,5,CURRENT_STORE_MOVE);
						MotRunTo(MOT_STORE_CARD_MOVE,70);
						waitMotCardTrolley = 1;
						workStep = 5;
						break;
				case 5:
						SetMotRunPam(MOT_CARD_LOAD,30,5,CURRENT_CARD_LOAD);
						MotRunTo(MOT_CARD_LOAD,55); 
						waitMotCardLoad = 1;
						workStep = 6;
						SetDelayTime(MOT_STORE_CARD_MOVE,3);
						break;
				case 6:
						MotRunTo(MOT_CARD_LOAD,20);
						waitMotCardLoad = 1;
						//workStep = 17;
						workStep = 7;
						SetDelayTime(MOT_STORE_CARD_MOVE,3);
						break;
				/*
				case 17:
						MotRunTo(MOT_STORE_CARD_MOVE,0);
						workStep = 18;
						break;
				case 18:
						SetDelayTime(MOT_STORE_CARD_MOVE,3);
						workStep = 7;
						break;
				*/
				case 7:
						siTmp = GetCardScanfPos(); 
						MotRunTo(MOT_STORE_CARD_MOVE,(unsigned int)siTmp);
				//		MotRunTo(MOT_STORE_CARD_MOVE,305);
						waitMotCardTrolley = 1;
						workStep = 8;
						break;
				case 8:
						SetMotRunPam(MOT_CARD_LOAD,140,10,CURRENT_CARD_LOAD);
						MotInitCheck(MOT_CARD_LOAD);
						workStep = 9;
						break;
				case 9:
						Uart0ReUnable;
						uart_Printf("%s $%4d\r\n",strM3146,NewTestInfo.cardStoreNum);
						Uart0ReEnable;
						SetDelayTime(MOT_STORE_CARD_MOVE,50);
						workStep = 10;
						break;
				case 10:
						workStep = 0;
						mainStep = 0;
						inWork = 0;
						CardStoreOpenLook = 0;
						Uart0ReUnable;
						uart_Printf("%s $%4d\r\n",strM4199,NewTestInfo.cardStoreNum);
						Uart0ReEnable;
						return 1;
				}
		}
		return 0;
}

void SetCardScanf(unsigned char num)
{
	if(num == 0)
		WaitPhotoFlag = 0;
	else
		WaitPhotoFlag = 1;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3123,WaitPhotoFlag);
	Uart0ReEnable;
}

void SetReCardScanf(const unsigned char num)
{
	ReScanfFlag = num;
}

unsigned char GetReCardScanf(void)
{
	return ReScanfFlag;
}


unsigned int SetCardScanfPos(signed char n)
{
	signed char i;
	if(n == 0)
	{
		EEPROM_READ(EEP_ADD_CARDSCANF_POS, i);		// ���¶�ȡд�������ֵ
		return (_POS_CARD_SCANF + (signed int)i);
	}
	else
	{
		if(n <= 100 && n >= -50)	// ������Χ�ж�
		{
			EEPROM_WRITE(EEP_ADD_CARDSCANF_POS, n);
		}
		EEPROM_READ(EEP_ADD_CARDSCANF_POS, i);		// ���¶�ȡд�������ֵ
		if(i > 100 || i < -50)		// �����ֵ���쳣
		{
			i = 0;
			EEPROM_WRITE(EEP_ADD_CARDSCANF_POS, i);	// ��ձ���ֵ
		}
		return (_POS_CARD_SCANF + (signed int)i);
	}
}

unsigned int GetCardScanfPos(void)
{
	signed char i;
	EEPROM_READ(EEP_ADD_CARDSCANF_POS, i);		// ���¶�ȡд�������ֵ
	if(i > 100 || i < -50)		// �����ֵ���쳣
	{
		i = 0;
		EEPROM_WRITE(EEP_ADD_CARDSCANF_POS, i);
	}
	return (_POS_CARD_SCANF + (signed char)i);
}

unsigned char CardScanfPosCheck(void)
{
	static unsigned char workStep;
	static unsigned char waitMotCardTrolley;
	unsigned int Pos;
	if(WaitDelayTime(MOT_STORE_CARD_MOVE))		return 0;
	if(waitMotCardTrolley){	if(GetMotState(MOT_STORE_CARD_MOVE) != STA_SLAVE_FREE)	return 0;waitMotCardTrolley = 0;}
	switch(workStep)
	{
		case 0:
				SetMotRunPam(MOT_STORE_CARD_MOVE,200,20,CURRENT_STORE_MOVE);
				MotRunTo(MOT_STORE_CARD_MOVE,0);	
				waitMotCardTrolley = 1;
				workStep = 1;
				SetDelayTime(MOT_STORE_CARD_MOVE,5);
				break;
		case 1:
				Pos = GetCardScanfPos();
				//SetMotRunPam(MOT_STORE_CARD_MOVE,50,5,CURRENT_STORE_MOVE);
				SetMotRunPam(MOT_STORE_CARD_MOVE,250,20,CURRENT_STORE_MOVE);
				MotRunTo(MOT_STORE_CARD_MOVE,Pos);	
				waitMotCardTrolley = 1;
				workStep = 2;
				break;
		case 2:
				SetDelayTime(MOT_STORE_CARD_MOVE,5);
				workStep = 3;
				break;
		case 3:
				workStep = 0;
				return 1;
				break;
		default:break;
	}
	return 0;
}

/*********************************************************************************************/



/****************************************File end***********************************************/
