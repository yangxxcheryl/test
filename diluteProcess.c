

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"
#include "eeprom.h"

/* ��ģ��������¹���:
1.Һ·�Լ�
2.�ö���
3.ϡ�ͱ걾
4.�Ӹ�Ƭ����ȡ�¸�Ƭ
5.�����������ĸ�Ƭ����ת��
6.��������ģ�齫������������������
*/


/*		ϡ�ͱ���
����	�걾(uL)		ϡ��Һ(uL)		����Һ(uL)
				[һ�λ���]
1		100
2		90				90	(4)			180
5		90				360	(12)		450
10		60				540	(18)		600

20		30				570	(19)		600
50		9.8				480	(16)		489.8		(��Ϊ���λ��ȵ�һ��ϡ��)
100		10				990	(33)		1000		
200		4.97			990	(33)		994.7
				[���λ���]
10		60				540	(18)		600			X 50 = 500
20		30				570	(19)		600			X 50 = 1000
40		20				780	(26)		800			X 50 = 2000
50		18.98			930	(31)		948.99		X 100 = 5000
100		10				990	(33)		1000		X 100 = 10000
����ϡ����100����һ��ϡ��ҺΪ�걾�ٴ�ϡ��


ϡ�ͱ�������λ�ǣ�30uL����ÿת��һ����С�ֵĹܵ�����1
�����ò����г̣�16 * 2.032mm = 32.512mm / 400 = 0.08128mm
������ÿ��ע������0.08128mm * ��1*1*3.1416��mm2 = 0.255349248mm3 = 0.25535mm3
*/


void _FluidMotRun(signed int n,unsigned char vel);
void _EffluentMotRun(signed int n,unsigned char vel);
void _DiluentMotRun(signed int n,unsigned char vel);
void _NewDiluentMotRun(signed int n,unsigned char vel);
void _NeedleMotRun(signed int n,unsigned char vel);
void _NeedleMotRunTo(signed int n,unsigned char vel);
void _SampPumpMotRun(signed int n,unsigned char vel);
void _SampPumpMotRunTo(signed int n,unsigned char vel);

//********************************************
extern unsigned char CardNoneUseful;
unsigned char InsertRingFlag = 0;
extern unsigned char insertflag[30];
unsigned char JudgeFlag;					// �������������,��ͨ���˱�������ϡ�ͱ���
unsigned char WithoutPuncture = 0;			// Ĭ��Ϊ0 ����   1  ������
extern unsigned char preCardStoreNum;
extern unsigned char CardStoreTestFlag;	
extern unsigned char LastCardGetState;
extern unsigned char primeProcessSW;		
extern unsigned char WorkProcessStep;		// �������̺�
//********************************************

SAMP_INFO NewTestInfo;		// �²�����Ϣ

// ��������
unsigned char CurInsertRingNum;			// �²��Կ�Ҫ����ת�̵�λ�ã�0xff��ʾ��Ч
unsigned char GetNewTestCard;			// ȡ�µĲ��Կ���0:��, 1:ȡ���뿨, 2:ȡ��1��, 3:ȡ��2��, 4:ȡ��3��, 5:ȡ��4��, 6:ȡ��5����
										// 254:ȡ����ɵȴ�����, 255:��Ƭ������ɵȴ�����ת��, 250:��ȡ��Ƭ����
										// 222:ȡƬ׼��������, 233:�����Ƚ���,׼������254ȡƬ���׼������
unsigned long SecondCount = 0;			// ��ʱ�Ӽ������ڶ�ʱ�жϼ���
extern unsigned char TurnPlateUsedLock;	// ת��ʹ����


static unsigned char _DiluentQuitFlag = 0;
static unsigned int liqDetBaseAdc;
static unsigned char runNum;
static unsigned char waitMotSampTurn,waitMotSampNeedle, waitMotSampPump,waitMotFluid,waitMotDiluent,waitEffluent;
signed int _DropVolume;					// ������
static signed int _DropVolumeFactor;	// ��������������
signed int _SamplingVolume;				// ������
unsigned int _DropMode;					// ����ģʽ 0 ������Լ�Ƭ	1 ӫ���Լ�Ƭ
unsigned char _MixtureMode;				// 1:1�Ƿ���Ҫ���� 0 ��Ҫ  1  ����Ҫ
unsigned int _ReMixNum = 5;				// ������
unsigned int _SleepTime;				// ����ʱ��
unsigned char _SleepSwitch;				// ���߿���
unsigned int _AutoTestCycleNum = 0;		// �Զ����Դ���


static signed int NeedleOnMixSidePos;	// ȡ���뵽���ȳ��ұ���λ��
static signed int NeedleOnMixCenterPos;	// ȡ���뵽���ȳ��м�λ��
static signed int DropHeight;			// �����߶�
static signed int MixHeight;			// �����ȸ߶�
static unsigned char CleanMode = 0;		// ��ϴģʽ  0:����ϴ��1:��ͨ��ϴ�� 2:��Ũ����ϴ
static unsigned char TestDebugMode = 0;	// ���Ȳ���ģʽ 0:��������  1�����Ȳ���
unsigned char _SampSW = 1;		// �����ж�
static unsigned char _WaitStartKey;		// �ȴ�����

// �����ò���
unsigned char _NewCardStoreNum;			// Ƭ�ֺ�
unsigned char _NewMultipNum;			// ϡ�ͱ������
static unsigned int _NewReadTime0;		// ��һ����ʱ��
static unsigned int _NewReadTime1;		// �ڶ�����ʱ��
static unsigned char _NewTestType;		// ��������


unsigned char JumpMode = 0;		// ֱ�Ӳ���ģʽ  0 ��Ҫ��е�Լ��Һ·�Լ�   1 ֱ�ӽ������  2 ���߽���Һ·�Լ� 3 ����ģʽ���3322Һ·�Լ�
unsigned char stopTestFlag = 0; // ֹͣ����,��Ƭ����������ʱ�䳬��20����

static unsigned char DiluteProcess_workStep;	
static unsigned char DiluteProcess_mainStep;	

/**********************************  ϡ�ʹ�������  *****************************************/

unsigned int CalMixingHeight(unsigned char multipNum, unsigned char diluteTime);
void _SetCurWorkParamter(void);
void _SetNewCardGet(unsigned char num);
unsigned char _CheckFluidSupply(void);
unsigned char _CheckDiluentSupply(void);


void SetDropVolume(unsigned int vol);
signed int CalSampVolume(unsigned char multipNum, unsigned char diluteTime);
signed int CalDiluteVolume(unsigned char multipNum, unsigned char diluteTime);
signed int CalDilute2Volume(unsigned char multipNum, unsigned char dilutetype);
unsigned char CalSampSyringSpeed(unsigned char multipNum, unsigned char diluteTime);
unsigned char CalDiluentInjectSpeed(unsigned char multipNum, unsigned char diluteTime);	



void SetTestDebugMode(unsigned char m)
{
	// ���ò���״̬�µĵ���ģʽ	0:������1:����Һ����
	if(m>1)
		m = 0;
	TestDebugMode = m;
}

unsigned char SamplingSwitch(unsigned char  sw)
{
	_SampSW = sw;
}

extern unsigned char _DiluteMainStep, _DiluteWorkStep;
extern unsigned char CardSurplusState[];		// ��Ƭʣ��״̬
extern unsigned char CardStoretate[];
extern RING_QUEUE	RingQueue;





void printf_DiluteProcess_StepState(void)
{
	Uart0ReUnable;
	uart_Printf("%s $%2d $%2d\r\n",strM3201,DiluteProcess_mainStep,DiluteProcess_workStep);
	Uart0ReEnable;
}

void SetPunctureState(unsigned char m)
{
	if(m > 2)
		m = 2;
	WithoutPuncture = m;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3119,WithoutPuncture);
	Uart0ReEnable;
}

unsigned char DiluteProcess(INFO_EVENT * pInfoEvent)
{
	// ϡ�����д�������
	static unsigned char mainStep;		
	static unsigned char workStep;
	static unsigned char startKey;
	static unsigned char  checkDiluent, checkFluid;
	static unsigned char pos;
	static unsigned char ucTmp;
	static unsigned char Num;
	signed int siTmp,siTmp1;
	static signed char sc;
	static unsigned int i;
	static unsigned int theDifferenceOfStep,theDownPointAdc;
	static long lastSampTime = 0;// ���һ��ȡ��ʱ��,���ڴ����ж�
	static unsigned char FindNum = 0;// ����ת�̴���
	static unsigned char temp1,temp2;	


#ifdef Puncture
	static unsigned char detRetry ;			// ����ʹ��
	static unsigned char ErrorNum = 0;		// ���ݴ���ģʽ�µĴ���,���ݲ�ͬ�Ĵ����Ͳ�ͬ����Ϣ
#endif
	
	DiluteProcess_mainStep = mainStep;
	DiluteProcess_workStep = workStep;
	
	if(1 == checkDiluent)					// ���ϡ��Һ
	{	
		if(_CheckDiluentSupply() == 1)
		{	
			mainStep = 13;	
			workStep = 0;	
			checkDiluent = 0;
		}	
	}
	if(1 == checkFluid)						// �����ϴҺ
	{		
		if(_CheckFluidSupply()==1)
		{	
			mainStep = 12;	
			workStep = 0;	
			checkFluid = 0;
		}
	}
	
	if(JumpMode == 3)		// ����ģʽ���ֶ�3322Һ·�Լ����֮��,��ת������
	{
		JumpMode = 0;
		mainStep = 1;
		workStep = 1;
		if(_SampSW == 0)	// �����ʱ����������,��������
			_SampSW = 1;
	}
	
	if(WaitDelayTime(MOT_SAMP_PUMP))		return 0;
	if(WaitDelayTime(MOT_SAMP_TRUN))		return 0;
	if(WaitDelayTime(MOT_SAMP_NEEDLE))		return 0;
	if(WaitDelayTime(MOT_EFFLUENT))			return 0;
	if(WaitDelayTime(MOT_FLUID))			return 0;
	if(WaitDelayTime(MOT_DILUENT))			return 0;
	
	if(waitMotSampTurn){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
	if(waitMotSampNeedle){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
	if(waitMotSampPump){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotSampPump = 0;	}
	if(waitMotFluid){		if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)			return 0;	waitMotFluid = 0;	}
	if(waitMotDiluent){		if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE)		return 0;	waitMotDiluent = 0;	}
	if(waitEffluent){		if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;	}

	if(_WaitStartKey)
	{
		if(_DiluentQuitFlag != 0)	// ϡ�ͳ����˳�����
		{
			_DiluentQuitFlag = 0;
			_WaitStartKey = 0;
			mainStep = 10;		// �����˳�����
			workStep = 0;
			return 1;
		}
		if(WaitStartKey() == 0)
		{		// �����¼�����
			if(mainStep == 1 && workStep == 1)
			{		// ���Կ���״̬�£����������¼�
				if(_SleepTime == 1)
				{		// ����ʱ����Чʱ
					// ��������
					TestALampClose();	// �رչ�Դ
					_WaitStartKey = 0;
					_SleepTime = 0;
					mainStep = 20;		// �����������
					workStep = 0;
					return 0;
				}
			}
			return 1;
		}

		if(_SampSW)
		{
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM3200);	// 2016-10-10 ���Ͱ�����Ϣ
			Uart0ReEnable;
			_WaitStartKey = 0;
		}
		else
			return 1;
	}
	
	switch(mainStep)
	{
		case 0:
		// ��ʼ��׼��
			switch(workStep)
			{
				case 0:		// �ȴ��û�������������ȡ�������е�����λ��
					if(JumpMode == 1)	
					{
						_NeedleMotRunTo(0, 180);
						waitMotSampNeedle = 1;
						workStep = 24;
						JumpMode = 0;
						break;
					}
					if(MachinePositionInit())
					{
						_DiluentQuitFlag = 0;
						Uart0ReUnable;
//						uart_Printf("%s\r\n",strM3191);		// ȡ������״̬
						uart_Printf("%s\r\n",strM3101);		// �밴��������ʼ
						Uart0ReEnable;
						workStep = 1;
						TestDebugMode = 0;
					}
					break;
				case 1:	// �ȴ��û���������
					_WaitStartKey = 1;	
					workStep = 2;
					if(CardNoneUseful == 0)
						TestALampOpen();		
					break;
				case 2:
					if(JumpMode == 2)			// ��������,��ʱ����������,����Һ·�Լ�
					{
						primeProcessSW = 3;
						mainStep = 1;
						workStep = 0;
						JumpMode = 0;
						break;
					}
					SetEValve(EV_ALL, EV_CLOSE);
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3100);		// ����
					Uart0ReEnable;
					lastSampTime = SecondCount;		// �������ʱ����
					// ��ȡȡ����λ����Ϣ
					NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
					NeedleOnMixSidePos = GetNeedleOnMixSidePos();
					DropHeight = GetDropHeight();
					SetMotRunPam(MOT_SAMP_TRUN,240,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 3;
					break;
				case 3:		// ��ע��ϴҺ
					ucTmp = _PrimingFluid();
					if(ucTmp == 1)	workStep = 4;		// ����ϡ��Һ��ע
					else if(ucTmp == 0xff)	
					{	// ���¹�ע
						workStep = 1;	
						Uart0ReUnable;
						uart_Printf("%s\r\n",strE3902);	
						Uart0ReEnable;
					}
					break;
				case 4:		// ��עϡ��Һ
					ucTmp = _PrimingDiluent();
					if(ucTmp == 1)	workStep = 5;		// ȫ����ע���
					else if(ucTmp == 0xff)	
					{	// ���¹�ע
						Uart0ReUnable;
						uart_Printf("%s\r\n", strE3904);
						Uart0ReEnable;
						workStep = 1;	
					}
					break;
				case 5:	// ��ϴ��ϴͷ��ȡ����ͨ��
					// ������ѹ
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(100, 200);
					SetDelayTime(MOT_EFFLUENT, 5);
					workStep = 6;
				case 6:	// ��ϴ
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(30, 30);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetDelayTime(MOT_EFFLUENT, 10);
					waitMotSampNeedle = 1;
					workStep = 100;
					break;
				case 100:
					MotStop(MOT_FLUID);
					waitMotFluid = 1;
					SetDelayTime(MOT_EFFLUENT, 10);
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampNeedle = 1;
					workStep = 7;
					break;
				case 7:	// �ȹر���ϴҺ
					_FluidMotRun(-2, 80);
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotFluid = 1;
					waitMotSampNeedle = 1;
					workStep = 8;
					break;
				case 8:	// �رո�ѹ
					SetEValve(EV_ALL, EV_CLOSE);
					MotStop(MOT_EFFLUENT);
					_NeedleMotRunTo(_POS_MIX_BUTTOM, 180);//240
					waitMotSampNeedle = 1;
					sc = 3;		// ������ϴ����
					workStep = 9;
					break;
				case 9:	// ��ϴ�͹�עȡ����ͨ��
					sc --;
					SetEValve(EV2, EV_OPEN);
					MotInitCheck(MOT_SAMP_PUMP);
					_FluidMotRun(12, 64);		// ע��1mL��ϴҺ
					waitMotFluid = 1;
					SetDelayTime(MOT_FLUID, 40);
					workStep = 10;
					break;
				case 10:
					_EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					workStep = 11;
					break;
				case 11:
					if(sc != 0){
						workStep = 9;
						}
					else{
						workStep = 15;
						}
					break;
				case 15:	// ��ϴ��ϣ�����ϴͷ����ˮ
					SetEValve(EV_ALL, EV_CLOSE);
					SetEValve(EV3, EV_OPEN);
					_NeedleMotRunTo(0, 180);//240
					_EffluentMotRun(30, 200);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					workStep = 16;
					break;
				case 16:	// �Ÿɻ��ȳ�ˮ
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(20, 220);
					waitEffluent = 1;
					workStep = 17;
					break;
				case 17:			// ȡ���۲�������λ�ã��������е�����λ
					SetMotRunPam(MOT_SAMP_NEEDLE, 200, 5, CURRENT_SAMP_NEEDLE);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetMotRunPam(MOT_SAMP_PUMP,120,5,CURRENT_SAMP_PUMP);
					MotInitCheck(MOT_SAMP_PUMP);
					waitMotSampNeedle = 1;
					workStep = 18;
					SetBeepAck();
					break;
				case 18:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotInitCheck(MOT_SAMP_TRUN);
					waitMotSampTurn = 1;
					workStep = 19;
					break;
				case 19:
					MotRunTo(MOT_SAMP_TRUN,_POS_SAMPTURN_SAMP);
					waitMotSampTurn = 1;
					workStep = 20;
					break;
				case 20:
#ifndef Puncture				
					_NeedleMotRunTo(_POS_SAMP_DOWN, 240);	//��������
					waitMotSampNeedle = 1;
#endif					
					_EffluentMotRun(100, 200);
					SetEValve(EV3, EV_OPEN);
					SetDelayTime(MOT_SAMP_TRUN, 30);
					workStep = 21;
					break;
				case 21:		// ȡ����������λ�ã�������������
					SetBeepPrompt();
					MotStop(MOT_EFFLUENT);
					SetEValve(EV_ALL,EV_CLOSE);
					workStep = 22;	
					break;
				case 22:
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_INTERVAL+_SAMP_PUMP_AIR_ISOLATE);
					waitMotSampPump = 1;
					workStep = 23;
					break;
				case 23:
					SetStateLedFree();
					mainStep = 1;
					workStep = 0;
					if(CleanMode != 0)
					{
						if(1 == CleanMode)
						{
							Uart0ReUnable;
							uart_Printf("%s\r\n", strM3167);
							Uart0ReEnable;
						}
						else if(2 == CleanMode)
						{
							Uart0ReUnable;
							uart_Printf("%s\r\n", strM3168);
							Uart0ReEnable;
						}
						else if(250 == CleanMode)
						{
							Uart0ReUnable;
							uart_Printf("%s\r\n", strM3169);
							Uart0ReEnable;
						}
						else if(255 == CleanMode)
						{
							Uart0ReUnable;
							uart_Printf("%s\r\n", strM3170);
							Uart0ReEnable;
						}
						CleanMode = 0;
						checkFluid = 0;
					}
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3102);
					Uart0ReEnable;
					SetStateLedFree();
					break;
				case 24:
					SetDelayTime(MOT_SAMP_NEEDLE,10);
					workStep = 25;
					break;
				case 25:
					if(GetMotorMonitorState(MOT_SAMP_NEEDLE,ZeroMonitor) == 1)		// �ж��������Ƿ�ص���λ
					{
						MotRunTo(MOT_SAMP_TRUN, 0);
						waitMotSampTurn = 1;
						workStep = 26;
					}
					else
					{
						workStep = 0;
						JumpMode = 1;
					}
					break;
				case 26:
					SetDelayTime(MOT_SAMP_TRUN,10);
					workStep = 27;
					break;
				case 27:
					if(GetMotorMonitorState(MOT_SAMP_TRUN,ZeroMonitor) == 1)		// �ж���ת���Ƿ�ص���λ
					{
						MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
						waitMotSampTurn = 1;
						// ��ȡȡ����λ����Ϣ �����ȳ�����  ��Ե  �����߶�
						NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
						NeedleOnMixSidePos = GetNeedleOnMixSidePos();
						DropHeight = GetDropHeight();
#ifndef Puncture
						workStep = 28;
#else
						workStep = 21;
#endif
					}
					else
					{
						workStep = 25;
					}
					break;
				case 28:
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);
					waitMotSampNeedle = 1;
					workStep = 21;
					break;
			}
			break;
		case 1:	
// ��ȡ��������ϴ������
			switch(workStep)
			{
				case 0:	// �ȴ��û���������
					if(_AutoTestCycleNum == 0)
						_WaitStartKey = 1;
					else
					{
						_AutoTestCycleNum --;
						Uart0ReUnable;
						uart_Printf("// AutoTestCyc: %d\r\n", _AutoTestCycleNum);
						Uart0ReEnable;
					}
					lastSampTime = SecondCount;		// ��������ȡ��ʱ��
					GetStoreState(0);				// �鿴Ƭ����Ϣ
					waitMotSampPump = 1;
					workStep = 1;	
					break;
				case 1:		
					// ���ֹͣ����
					if(1 == stopTestFlag)
					{
						// �ϱ�������Ϣ,��ʾ����رշ�Ƭ��
						Uart0ReUnable;
						uart_Printf("%s\r\n", strE3906);
						Uart0ReEnable;
						SetBeepWarning();
						workStep = 0;
						return 1;
					}
					
					if(CleanMode == 1)
					{	// ��ͨ��ϴģʽ
						Uart0ReUnable;
						uart_Printf("%s\r\n", strM3157);
						Uart0ReEnable;
						mainStep = 105;
						workStep = 0;
						SetStateLedBusy();
						break;
					}
					else if(2 == CleanMode)
					{	// ǿ����ϴģʽ
						Uart0ReUnable;
						uart_Printf("%s\r\n", strM3158);
						Uart0ReEnable;
						mainStep = 100;
						workStep = 0;
						SetStateLedBusy();
						break;
					}
					// �����ȡƬ����
					if(0 != CardStoreTestFlag)
					{	
						if(0 == LastCardGetState)					// ȡƬ����ģʽ��,ǰһ��û�����,�޷�ִ����һ��ȡƬ����
							return;
						//LastCardGetState = 0;
						if(_NewCardStoreNum != preCardStoreNum)
						{
							SetWorkStoreNum(preCardStoreNum);
						}
					}
					// �ȼ��Ƭ���Ƿ񱻿���
					if(CardStoretate[_NewCardStoreNum - 1] == INFO_STORE_OPEN)
					{
						Uart0ReUnable;
						uart_Printf("!3521 $%4d\r\n", _NewCardStoreNum);
						Uart0ReEnable;
						_WaitStartKey = 1;
						SetDelayTime(MOT_SAMP_NEEDLE, 10);
						SetBeepWarning();
						break;
					}
					// ���Ƭ���Ƿ��и�Ƭ
					if(CardNoneUseful == 0)
					{
						
						if(CardSurplusState[_NewCardStoreNum - 1] == INFO_STORE_EMPTY)
						{
							Uart0ReUnable;
							uart_Printf("!3520 $%4d\r\n", _NewCardStoreNum);
							Uart0ReEnable;
							_WaitStartKey = 1;
							SetDelayTime(MOT_SAMP_NEEDLE, 10);
							SetBeepWarning();
							break;
						}
						if(CardSurplusState[_NewCardStoreNum - 1] == INFO_STORE_ERROR)
						{
							Uart0ReUnable;
							uart_Printf("!3522 $%4d\r\n", _NewCardStoreNum);
							Uart0ReEnable;
							_WaitStartKey = 1;
							SetDelayTime(MOT_SAMP_NEEDLE, 10);
							SetBeepWarning();
							break;
						}
					}
#ifndef Puncture
					// ��ʼ���ԣ����汾�β��ԵĲ��Կ����ͺ�ϡ�ͱ���	
					_SetCurWorkParamter();
					_SetNewCardGet(NewTestInfo.cardStoreNum);
#endif
					SetBeepAck();
					SetEValve(EV_ALL,EV_CLOSE);
					SetDelayTime(MOT_SAMP_NEEDLE, 2);  // 2017-05-22 5 -> 2 
					workStep = 2;
					break;
				case 2:
#ifdef Puncture		
					if(0 == WithoutPuncture)
					{		
						MotRunTo(MOT_SAMP_TRUN,_POS_SAMPTURN_SAMP);
						waitMotSampTurn = 1;
						workStep = 30;
					}
					else
					{
						if(1 == WithoutPuncture)
						{
							_WaitStartKey = 1;
							_NeedleMotRunTo(2380, 240);	//_POS_SAMP_DOWN - 1420
							waitMotSampNeedle = 1;
							workStep = 41;
						}
						else if(2 == WithoutPuncture)
						{
							_NeedleMotRunTo(_POS_SAMP_DOWN, 240);	
							waitMotSampNeedle = 1;
							workStep = 40;
						}
					}
					break;
				case 30:				// ��ʼ����
#ifndef HalfCircle
					_NeedleMotRunTo(300 * 2, 180);	
#else
					_NeedleMotRunTo(300, 180);			
#endif
					waitMotSampNeedle = 1;
					//workStep = 31;
					detRetry = 0;
					workStep = 35;
					break;
				case 35:
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);      //������ѹ��baseֵ
					//if(liqDetBaseAdc < 400)
					if(liqDetBaseAdc < 430)
					{
						workStep = 60;
						//SetEValve(EV_ALL,EV_CLOSE);
						//SetEValve(EV1,EV_OPEN);    //��ϴҺ����3�Ź�
						SetEValve(EV3,EV_OPEN);    //��Һ������ϴ��
						_EffluentMotRun(12, 240);
						//��Һ�ſ���ϴͷ��ˮ
						waitEffluent = 1;
						SetDelayTime(MOT_SAMP_NEEDLE, 5); 
						if(detRetry >= 3)
						{
							detRetry = 0;
							workStep = 50;
							ErrorNum = 4;	// 3928
						}
						else
							detRetry++;
						break;
					}
					detRetry = 0;
					SetEValve(EV_ALL,EV_CLOSE);
					Uart0ReUnable;
					uart_Printf("// LiqDetBaseAdc:%d\r\n", liqDetBaseAdc);
					Uart0ReEnable;
					//workStep = 31;
					workStep = 36;
					break;
				case 36:
#ifndef HalfCircle
					_NeedleMotRunTo(560 * 2, 180);	
#else
					_NeedleMotRunTo(560, 180);
#endif
					waitMotSampNeedle = 1;
					workStep = 31;
					break;
				case 60:
					//_FluidMotRun(20, 40);           //��ϴ�øĳɵ���
					SetEValve(EV1,EV_OPEN);
					_FluidMotRun(-2, 40);
					//waitMotSampNeedle = 1;
					//waitEffluent = 1;
					waitMotFluid = 1;
					workStep = 35;
					break;
					
					
				case 31:				// ���̵��ﴩ�̹�źλ��
					SetMotRunPam(MOT_SAMP_NEEDLE, 120, 10, 10); 	// 60
#ifndef HalfCircle
					MotRun(MOT_SAMP_NEEDLE, 320 * 2);
#else
					MotRun(MOT_SAMP_NEEDLE, 320);
#endif
					waitMotSampNeedle = 1;
					workStep = 32;
					break;
				case 32:				// �жϴ��̽�� (PINA & 0x40):���̸�Ӧ������
					//ucTmp = PINA;
					//if((ucTmp & 0x40) == 0)	// ����ѹ��(�Թ�)��Ӧ
					{
						Uart0ReUnable;
						uart_Printf("%s\r\n",strM3225);
						Uart0ReEnable;
						workStep = 33;
						break;
					}
					/*
					else	// δ̽�⵽�Թ�,ȡ������
					{
						// ��������
						ErrorNum = 1;	// 3925
						workStep = 50;
						break;
					}
					*/
					break;
				case 33:	//  (PINA & 0x80):���̵�λ������
					//ucTmp = PINA;
					//if((ucTmp & 0x80) != 0)	// ���̵�λ��Ӧ
					{
						Uart0ReUnable;
						uart_Printf("%s\r\n",strM3226);
						Uart0ReEnable;
						workStep = 34;
					}
					/*
					else	// δ���̵�λ
					{
						ErrorNum = 1;	// 3926
						// ��������
						workStep = 50;
						break;
					}
					*/
					break;
				case 34:				// Һ��̽���ʼ��������ȡ�����½�
					SetMotRunPam(MOT_SAMP_NEEDLE, 180, 5, 10);	// 110
					MotRunTo(MOT_SAMP_NEEDLE, _POS_SAMP_DOWN);
					detRetry = 0;
					workStep = 37;
					break;
				case 37:		// Һ��̽��
					i = getLiqDetADC(NeedleChannel);// 0-200
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						// ������е�����г̴�,δ̽�⵽Һ��
						MotStop(MOT_SAMP_NEEDLE);
						SetBeepWarning();
						// �˳�����
						ErrorNum = 3;	// 3927
						Uart0ReUnable;
						uart_Printf("// LiqNotPassAdc:%d\r\n", i);
						Uart0ReEnable;
						workStep = 50;
						break;
					}
					if(i < liqDetBaseAdc)// 350
					{
						i = liqDetBaseAdc - i;//100+
						Num = liqDetBaseAdc / 5;
						//Num = liqDetBaseAdc / 8;//>25
						//if(Num > 25)
						//	Num = 25;
						if(i > Num)		// ����Һ���Ӧ������  16  25
						{
							if(detRetry < 3)  // 1
							{
								detRetry ++;	
								break;	
							}
							
							MotStop(MOT_SAMP_NEEDLE);   //�Ƚ�С��ֹͣ	
							Uart0ReUnable;
					        uart_Printf("// theNeedleMotIsStop\r\n");	
					        Uart0ReEnable;
							SetDelayTime(MOT_SAMP_NEEDLE, 4);  // ̽�⵽Һ��ֹͣ��ʱ
							SetBeepAck();
							workStep = 38;
						}
						else
							detRetry = 0;
					}
					break;
				case 38:	
				//	MotStop(MOT_SAMP_NEEDLE);
					theDifferenceOfStep = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);
					Uart0ReUnable;
					//uart_Printf("%s $%4d\r\n", strM3227,i);	
					uart_Printf("// theWaterLevelStep:%d\r\n", theDifferenceOfStep);	
					Uart0ReEnable;
					waitMotSampNeedle = 1;
					SetEValve(EV2,EV_CLOSE);  // ����ǰ�ر�EV2
				//	SetDelayTime(MOT_SAMP_NEEDLE, 5);
					workStep = 39;
					break;
				case 39:		// ̽�⵽Һ�沢ֹͣ�������
				
					theDownPointAdc=getLiqDetADC(NeedleChannel);
					
				    Uart0ReUnable;
					uart_Printf("// theDownPointAdc:%d\r\n", theDownPointAdc);	
					Uart0ReEnable;
					
					Uart0ReUnable;
					uart_Printf("// LiqChangeAdc:%d\r\n", i);	// ��ֵ
					Uart0ReEnable;
					//i = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);         //pan20170503
					theDifferenceOfStep=_POS_SAMP_DOWN - theDifferenceOfStep;
					Uart0ReUnable;
					uart_Printf("// theDifferenceOfStep :%d\r\n", theDifferenceOfStep);	// ����Һ��̽����
					Uart0ReEnable;
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					
					if (theDifferenceOfStep >= 60)                     //��1.5mm������
					{
						workStep = 41;
					}
					else
					{
						ErrorNum = 3;	// 3927
						Uart0ReUnable;
						uart_Printf("// theDifferenceOfStepisTooSmall:%d\r\n", theDifferenceOfStep);
						Uart0ReEnable;
						workStep = 50;
					}
					break;
				case 41:
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);	
					waitMotSampNeedle = 1;
					workStep = 40;
					break;
				case 40:
					_SetCurWorkParamter();
					_SetNewCardGet(NewTestInfo.cardStoreNum);
#endif				
					siTmp = CalSampVolume(NewTestInfo.sampDiluteMult, 0);
					siTmp += 45;
					Uart0ReUnable;
					uart_Printf("%s $%4d $%4d\r\n", strM3121, siTmp,NewTestInfo.sampDiluteMult);	// ��ʾ��ǰ������
					Uart0ReEnable;
#ifdef Puncture
					if(NewTestInfo.sampDiluteMult == 9)
					{
						if(WithoutPuncture == 0)	// �����Ҫ��������
							siTmp += 1955;
						else
							siTmp += 400 ;    
					}
#else
					if(NewTestInfo.sampDiluteMult == 9)
						siTmp += 400 ;    
#endif
					_SampPumpMotRun(siTmp, 180);	// 2017-05-22  60 -> 180	
					
					if(NewTestInfo.sampDiluteMult !=13)		//		�����1��4	�Ͳ�����ϴҺ 
					 _FluidMotRun(4, 64);
					 
					 
#if (DILUTE_TUBE == 14)
					_DiluentMotRun(20, 180);		// ��ϴϡ��Һ��·�ͻ��ȳصײ�   
#elif (DILUTE_TUBE == 16)
					_DiluentMotRun(6, 200);			// ��ϴϡ��Һ��·�ͻ��ȳصײ�	// 2017-05-22 160 -> 200
#endif
					//SetStateLedBusy();     //pan20170614
					waitMotSampPump = 1;
					waitEffluent = 1;	// ����ϴ���Һδ��ϣ��ȴ�
					workStep = 3;
					break;
				case 3:
					waitMotFluid = 1;
					waitMotDiluent = 1;
					workStep = 4;
					break;
				case 4:		
				
				    SetBeepPrompt();  //������� ��������ʼ����
					SetStateLedBusy();  //pan20170614  �������֮�󣬵Ʋű�ɺ�ɫ
				    SetEValve(EV_ALL,EV_CLOSE);   //20170614 pan
				    // _DiluentMotRun(100, 60);		
			        _EffluentMotRun(20, 160);
			        waitEffluent = 1;	
					workStep = 101;
		        	break;
				case 101:	
				    _DiluentMotRun(25, 160);		
			        _EffluentMotRun(30, 200);
			        waitEffluent = 1;	
					waitMotDiluent = 1;
					workStep = 102;
		        	break;
				
				
				
				case 102:                      //20170614 pan
					SetEValve(EV3, EV_OPEN);
					SetEValve(EV1, EV_OPEN);
					//SetBeepPrompt();
#ifndef HalfCircle
					_EffluentMotRun(112, 240);		// 2017-05-22 140 -> 112  
#else
					_EffluentMotRun(56, 240);		// 2017-05-22 70 -> 56
#endif
					SetDelayTime(MOT_EFFLUENT, 2);	// �ӳ�һ��ʱ�䣬�Ƚ�����ѹ
					workStep = 5;
					break;
				case 5:				// ����ϴҺ�ã���ϴ�����
					_FluidMotRun(20, 40);
					waitMotSampNeedle = 1;
					_NeedleMotRunTo(0, 200);	// ȡ��������
					workStep = 7;	
					break;
				case 7:				// ȡ�����Ѿ���������ߵ㣬�����ϴ����,ȡ����ص���ʼλ
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("%s $%8d\r\n",strM3104 ,NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3104);
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					MotStop(MOT_FLUID);
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					checkFluid = 0;
					SetDelayTime(MOT_SAMP_TRUN, 5);  // 2017-05-22  10->5
					workStep = 8;
					break;
				case 8:		// �ӳ�1���ֹͣ��ϴͷ����, ת�������ȳ��ſ�
					SetEValve(EV_ALL,EV_CLOSE);
					waitMotSampTurn = 1;
					workStep = 9;
					break;
				case 9:				// �걾��ȡ���
					if(NewTestInfo.sampDiluteMult != 1)
					{
						MotRunTo(MOT_SAMP_TRUN, NeedleOnMixSidePos);		// ȡ�������е����ȳر��Ϸ�
						waitMotSampTurn = 1;
						mainStep = 4;		
						workStep = 0;	// ����һ��ϡ�ͳ���
					}
					else
					{
						if(_DropMode == 0)
						{
							mainStep = 9;		
							workStep = 0;		// ֱ��������������
						}
						else 
						{	// 1:1��Ҫ������,ֱ������������
							if(0 == _MixtureMode)
							{
								mainStep = 8;
								workStep = 11;
							}
							// 1:1����Ҫ������,ֱ�ӵ�������С���ӵ��Լ�Ƭ
							else
							{
								mainStep = 8;
								workStep = 18;
							}
						}
					}
					waitEffluent = 1;
					break;
#ifdef Puncture			
				case 50:	// ����ʧ���˳�����
					SetMotRunPam(MOT_SAMP_NEEDLE, 100, 5, 10);		// 100
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					workStep = 51;
					break;
				case 51:
					SetEValve(EV3, EV_OPEN);
					SetEValve(EV1, EV_OPEN);
#ifndef HalfCircle
					_EffluentMotRun(140, 240);  
#else
					_EffluentMotRun(70, 240);
#endif
					SetDelayTime(MOT_EFFLUENT, 2);	// �ӳ�һ��ʱ�䣬�Ƚ�����ѹ
					workStep = 52;
					break;
				case 52:
					_FluidMotRun(20, 40);
					waitMotSampNeedle = 1;
					workStep = 53;
					break;
				case 53:
					MotStop(MOT_FLUID);
					_FluidMotRun(-1, 40);
					SetDelayTime(MOT_EFFLUENT, 10);
					waitMotFluid = 1;
					if(ErrorNum == 1)
					{
						ErrorNum = 0;
						Uart0ReUnable;
						uart_Printf("%s\r\n",strE3925);
					//	uart_Printf("%s $%8d\r\n",strE3925,NewTestInfo.testSerial);
						Uart0ReEnable;
					}
					else if(ErrorNum == 2)
					{
						ErrorNum = 0;
						Uart0ReUnable;
						uart_Printf("%s\r\n",strE3926);
					//	uart_Printf("%s $%8d\r\n",strE3926,NewTestInfo.testSerial);
						Uart0ReEnable;
					}
					else if(ErrorNum == 3)
					{
						ErrorNum = 0;
						Uart0ReUnable;
						uart_Printf("%s\r\n",strE3927);
					//	uart_Printf("%s $%8d\r\n",strE3927,NewTestInfo.testSerial);
						Uart0ReEnable;
					}
					else if(ErrorNum == 4)
					{
						ErrorNum = 0;
						Uart0ReUnable;
						uart_Printf("%s  $ %4d\r\n",strE3928,liqDetBaseAdc);
					//	uart_Printf("%s $%8d\r\n",strE3928,NewTestInfo.testSerial);
						Uart0ReEnable;
					}
					workStep = 54;
					break;
				case 54:
					SetEValve(EV_ALL,EV_CLOSE);
					MotStop(MOT_EFFLUENT);
					workStep = 0;
					break;
#endif
				}
			break;
		case 4:				// һ�λ���
			switch(workStep)
			{
				case 0:		// ȡ�����½������ȸ߶�
					_FluidMotRun(-1, 80);
					_EffluentMotRun(8, 240);
					_NeedleMotRunTo(_POS_MIX_NEEDLE, 180);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					waitMotDiluent = 1;
					if(NewTestInfo.sampDiluteMult == 9)		// 1:500
						workStep = 110;						// ��Ҫ����������
					else
						workStep = 1;						// ֱ��Ԥ����					
					SetEValve(EV_ALL,EV_CLOSE);
					break;
				case 110:		// 1:500ģʽ�²���Ч
#ifdef Puncture					
					if(WithoutPuncture == 0)			// ����1000
						_SampPumpMotRun(-1000, 180);	// ע��걾
					else
						_SampPumpMotRun(-200, 180);		// ���³������5uL
#else
					_SampPumpMotRun(-200, 180);			// ���³������5uL
#endif
					_FluidMotRun(6, 80);
					_DiluentMotRun(4, 140);	
					_EffluentMotRun(18, 240);			// 2017-05-23 200->240
					waitMotFluid = 1;
					waitMotDiluent = 1;
					waitMotSampPump = 1;
					waitEffluent = 1;
					workStep = 1;
					break;
				case 1:		// Ԥ����
					_SampPumpMotRun(-63 - _SAMP_PUMP_INTERVAL, 120);	// ע��걾  3ulԤ����	 2017-05-22 80 -> 120
#if 	(DILUTE_TUBE == 14)
					_DiluentMotRun(10, 200);
#elif	(DILUTE_TUBE == 16)
					_DiluentMotRun(4, 200);		// 2017-05-22 140->200
#endif
					waitMotDiluent = 1;
					waitMotSampPump = 1;
					workStep = 2;
					break;
				case 2:		// �ſջ��ȳ�
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(10, 220);
					SetMotRunPam(MOT_SAMP_TRUN,64,5,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitEffluent = 1;
					waitMotSampTurn = 1;
					workStep = 3;
					break;
				case 3:		// ȡ�������е����ȳر�λ��׼��ϡ��
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixSidePos);
					waitMotSampTurn = 1;
					workStep = 4;
					break;
				case 4:		// �������������ú�ϡ��Һ�ã���ʼ����
					siTmp = CalSampVolume(NewTestInfo.sampDiluteMult, 0);
					ucTmp = CalSampSyringSpeed(NewTestInfo.sampDiluteMult, 0);
					if(siTmp)
					{
						if(NewTestInfo.sampDiluteMult == 9)			// 1:500ֱ����ָ����
							_SampPumpMotRun(-siTmp, ucTmp);
						else										// ��1:500��Ҫ
							_SampPumpMotRun(-siTmp - 200, ucTmp);	// ����200������
					}
					siTmp1 = CalDiluteVolume(NewTestInfo.sampDiluteMult, 0);
					ucTmp = CalDiluentInjectSpeed(NewTestInfo.sampDiluteMult, 0);
					if(siTmp1)
						_DiluentMotRun(siTmp1, ucTmp);		// ����עҺ��������ϡ��Һע���ٶ�
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("%s $%8d\r\n",strM3107 ,NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3107);
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					checkDiluent = 1;
					waitMotDiluent = 1;
					waitMotSampPump = 1;
					workStep = 5;
					break;
				case 5:		// ���ݻ��ȴ���ִ�в�ͬ����
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					//2016-05-24 ���� > 11,ϡ�ͱ���1:3,1:4,...,1:40����Ҫ����2��ϡ��
					if(NewTestInfo.sampDiluteMult < 9 || NewTestInfo.sampDiluteMult > 11) 
						workStep = 7;
					else
						workStep = 6;
					break;
				case 6:		// ��������ϡ��
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP);
					waitMotSampNeedle = 1;
					workStep = 0;
					mainStep = 5;
					break;
				case 7:
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_NEEDLE + 30);	// ���½�
					waitMotSampNeedle = 1;
					switch(NewTestInfo.sampDiluteMult)
					{
						case 2:		workStep = 10;break;		// 1:2
						case 12:	workStep = 14;break;		// 1:3
						case 13:	workStep = 14;break;		// 1:4
						case 14:	workStep = 18;break;		// 1:40
						default:	workStep = 18;break;
							
					}
					break;
				case 10:		// 2016-10-31 ���������������
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM + 25);
					waitMotSampNeedle = 1;
					workStep = 11;
					break;
				case 11:
					workStep = 16;
					SetDelayTime(MOT_SAMP_NEEDLE, 2);	// �ӳ�һ��ʱ��
					break;
				case 14:
					_SampPumpMotRun(4244, 220);   // 100 / 0.023563 = 4244
					waitMotSampPump = 1;
					workStep = 15;
					Num++;
					break;
				case 15:
					_SampPumpMotRun(-4032, 240); // 95 / 0.023563 = 4032
					waitMotSampPump = 1;
					workStep = 16;
					break;
				case 16:
					Num++;
					if(Num > 5)
					{
						Num = 0;
					//	_SampPumpMotRun(3395, 240); // 80 / 0.02353 = 3395
						workStep = 18;
					}
					else
					{
						_SampPumpMotRun(3820, 240); // 90 / 0.02353 = 3820
						workStep = 17;
					}
					waitMotSampPump = 1;
					break;
				case 17:
					_SampPumpMotRun(-3820, 240);	// 90 / 0.02353 = 3820
					waitMotSampPump = 1;
					workStep = 16;
					break;
				case 18:
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					waitMotSampNeedle = 1;
					if(TestDebugMode == 1)
					{
						workStep = 19;
						break;
					}
					  workStep = 9;  //changed by pan 20161110	��ȡһ�ο�����		
					  break;
				case 19:
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn = 1;
					workStep = 20;
					_WaitStartKey = 1;
					break;
				case 20:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 21;
					break;		
				case 21:
					SetEValve(EV3, EV_CLOSE);
					MotRun(MOT_SAMP_PUMP, 82);		// ���������
					waitMotSampPump = 1;
					workStep = 0;
					mainStep = 8;
					break;			
				case 8:		// ����ϴͷ����ϴȡ�����ڱ�
					_EffluentMotRun(10, 180);
					_FluidMotRun(2, 32);		// ��ϴ���ڱ�
					waitMotFluid = 1;
					waitEffluent = 1;
					workStep = 9;
					break;				
				case 9:		// ������ȡһ�θ������
					SetEValve(EV_ALL,EV_CLOSE);
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_AIR_ISOLATE);
					waitMotSampPump = 1;
					workStep = 0;
					mainStep = 8;
					break;
				}
			break;
		case 5:				// ���λ���
			switch(workStep){
				case 0:
					SetDelayTime(MOT_SAMP_NEEDLE,5);	// 2017-05-22  20->5
					workStep = 200;
					break;
				case 200:// ��ϡ��Һ����ȡ���λ���ԭҺ
					siTmp = CalDilute2Volume(NewTestInfo.sampDiluteMult, 0);
					siTmp = siTmp + 3;				
#if 	(DILUTE_TUBE == 14)
					_DiluentMotRun(-siTmp, 160);
#elif	 (DILUTE_TUBE == 16)
					_DiluentMotRun(-siTmp, 200);			// 2017-05-22 120->200
#endif
					waitMotDiluent = 1;
					workStep = 1;
					break;
				case 1:
				  _EffluentMotRun(15, 240);					// 2017-05-22 200->240
#ifndef Puncture
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP + 200);
#else
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP + 400);
#endif
					waitEffluent = 1;
					waitMotSampNeedle = 1;
					SetEValve(EV2, EV_CLOSE);	// ȡ����ͨ���ر�
					SetEValve(EV1, EV_OPEN);	// ������ϴͷϴҺ��Ӧ
					workStep = 2;
					break;
				case 2:	// ��ϴȡ������ۣ���ʱ��Һ
					_FluidMotRun(8, 80);		// ������ϴҺϴ���ȳ�,ע��1.0mL��ϴҺ(12, 180) //180
					_EffluentMotRun(10, 100);	
					waitMotFluid = 1;
					waitEffluent = 1;
					workStep = 3;
					break;
				case 3:
					SetEValve(EV2, EV_OPEN);	// ����ȡ����ϴҺ��Ӧ
					workStep = 4;
					ucTmp = 0;
					break;
				case 4:		// ��ϴȡ�����ڱڣ���ʱ�ſ�
#ifndef Puncture
					_FluidMotRun(16, 80);		// ��ϴ���ڱ�(12, 80)
#else
					_FluidMotRun(16, 40);		// ��ϴ���ڱ�(12, 40)
#endif
					_EffluentMotRun(20, 240);	// 2017-05-23 100->240
					waitMotFluid = 1;
					waitEffluent = 1;
				//	ucTmp ++;
				//	if(ucTmp < 1)		// ������ϴ���� 2016-08-22 ������һ��
										// 2017-05-23 ucTmp < 2 -> ucTmp < 1
				//		workStep = 4;
				//	else
						workStep = 5;
					break;
				case 5:		// ��ϴȡ�������
					SetEValve(EV2, EV_CLOSE);	// ȡ����ͨ���ر�
					SetEValve(EV1, EV_OPEN);	// ������ϴͷϴҺ��Ӧ
					_FluidMotRun(8, 80);		// _FluidMotRun(12, 180);		// ������ϴҺϴ���ȳ�,ע��1.0mL��ϴҺ(12, 180)//(12,120)
					waitMotFluid = 1;
					waitEffluent = 1;
					SetEValve(EV3, EV_OPEN);	// ��ϴͷ��Һ��
					workStep = 6;
					break;
				case 6:		// �����ϴͷ
					_FluidMotRun(-1, 160);
					_EffluentMotRun(24, 240);
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					workStep = 7;
					break;
				case 7:		// �ſջ��ȳ�
					SetEValve(EV_ALL,EV_CLOSE);
					waitEffluent = 1;
					waitMotFluid = 1;
					workStep = 80;
					break;
				case 80:
					_DiluentMotRun(3, 120); 	
					waitMotDiluent = 1;
					workStep = 81;
					break;
				case 81:
					SetDelayTime(MOT_SAMP_NEEDLE, 2);	// 2017-05-22 10->2
					workStep = 82;
					break;
				case 82:
					_EffluentMotRun(10, 180);	// 2017-05-22 100->180
					waitEffluent = 1;
					workStep = 83;
					break;
				case 83:
					SetEValve(EV3, EV_OPEN);	// ��ϴͷ��Һ��
					_EffluentMotRun(15, 240);	// �����ϴͷ
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					waitMotSampNeedle = 1;
					workStep = 8;
					break;
				case 8:		//  ��ʼ���λ���
					siTmp = CalDilute2Volume(NewTestInfo.sampDiluteMult, 1);
#if 	(DILUTE_TUBE == 14)
					_DiluentMotRun(siTmp, 160);	// ��ϡ�͹ܵ���Ļ���Һ��ϡ��Һע����ȳ��л���
#elif 	(DILUTE_TUBE == 16)
					_DiluentMotRun(siTmp, 240); // 2017-05-22 200->240
#endif
					waitMotDiluent = 1;
					waitEffluent = 1;
					workStep = 9;
					break;
				case 9:
					if(TestDebugMode == 1)
					{
						MotRunTo(MOT_SAMP_NEEDLE, 0);
						waitMotSampNeedle = 1;
						workStep = 19;
						mainStep = 4;
						break;
					}
					else
						workStep = 10;
					break;
				case 10:
					SetEValve(EV3, EV_CLOSE);
					MotRun(MOT_SAMP_PUMP, 200);		// ���������
					waitMotSampPump = 1;
					workStep = 0;
					mainStep = 8;
					break;
				}
			break;
		case 8:	
		// ϡ�ͽ�������ȡ����û���Һ100uL��ȡ������������ߵ㣬ȡ�������е���ʼλ��׼������,ͬʱ��ϴ���ȳ�
			checkDiluent = 0;// 2016-10-22 ϡ�����,�ر�ϡ��Һ���
			switch(workStep){
				case 0:	// ȡ�������е����ȸ߶�
					temp1 = getLiqDetADC(NeedleChannel);
					if(NewTestInfo.sampDiluteMult == 2 || NewTestInfo.sampDiluteMult == 12 
					|| NewTestInfo.sampDiluteMult == 13)
						MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM + 25);
					else if(NewTestInfo.sampDiluteMult < 7 || NewTestInfo.sampDiluteMult == 14)
						MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM);
					else
						MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM - 150);
					waitMotSampNeedle = 1;
					workStep = 1;
					break;
				case 1:		// ��ȡ����û���Һ,
					temp2 = getLiqDetADC(NeedleChannel);
					Uart0ReUnable;
					uart_Printf("%s $%4d $%4d\r\n",strM3188, temp1,temp2);
					Uart0ReEnable;
					siTmp = _DropVolume + GetDropVolumeFactor();
					_SampPumpMotRun(siTmp + _SAMP_PUMP_INTERVAL + 400 , 220);	
					Uart0ReUnable;
					uart_Printf("%s $%4d\r\n",strM3174,siTmp);
					Uart0ReEnable;
					waitMotSampPump = 1;
					workStep = 2;
					break;
				case 2:
					SetDelayTime(MOT_SAMP_PUMP, 1);
					workStep = 3;
					break;
				case 3:		// �����̼�϶
					MotRun(MOT_SAMP_PUMP, -_SAMP_PUMP_INTERVAL);
					workStep = 4;	
					break;
				case 4:		// ȡ�����뿪Һ�� 
					_NeedleMotRun(-160,180);//240	
					waitMotSampNeedle = 1;
					waitMotSampPump = 1;
					workStep = 5;
					break;
				case 5:		// ȡ�������е���ߵ�, ͬʱ��ϴ�����
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetEValve(EV1, EV_OPEN);	
					checkFluid = 1;				
					_EffluentMotRun(90, 220);	
				//	_FluidMotRun(4, 80);		// ȡ��������Һ���ϴ�벽��
					waitMotSampNeedle = 1;
					workStep = 6;
					break;
				case 6:		// ȡ�����Ѿ����е���ߵ�, ��ϴ����
				//	MotStop(MOT_FLUID);
					SetDelayTime(MOT_FLUID, 1);
					checkFluid = 0;
					workStep = 7;
					break;
				case 7:		// ������ѹ�����������
					_FluidMotRun(-1, 80);
					SetEValve(EV3, EV_OPEN);
					SetDelayTime(MOT_SAMP_NEEDLE, 2);	// 2016-09-18   // 2017-05-22  5->2
					workStep = 8;
					break;
				case 8:		// ȡ�������е������Ϸ�
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn = 1;
					workStep = 9;
					break;
				case 9:		// ��ϴͷ����ֹͣ, ת�������ȳ��ſ�
					SetEValve(EV_ALL, EV_CLOSE);
					SetDelayTime(MOT_EFFLUENT, 2);
					workStep = 10;
					break;
				case 10:		// ȡ�����ڲ��Կ��Ϸ�(��ʼλ)���ȴ���Һ�������Կ�
					SetEValve(EV3, EV_CLOSE);		// ��Һ�л������ȳأ��������Һ
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("%s $%8d\r\n",strM3110 ,NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3110);
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					if(_DropMode == 0)
					{
						mainStep = 9;
						workStep = 0;
					}
					else
						workStep = 11;
					break;
				case 11:
					if(GetNewTestCard == 222)				//������׼��
					{
						Uart0ReUnable;
						uart_Printf("%s\r\n",strM3145);
						Uart0ReEnable;
						MixHeight = GetMixHeight();
						_NeedleMotRunTo(MixHeight,240);
						waitMotSampNeedle = 1;
						workStep = 12;
						break;
					}
					else if(GetNewTestCard == 250)			// ȡƬʧ�� 	
					{
						MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
						waitMotSampTurn = 1;
						mainStep = 9;
						workStep = 5;
					}
					break;
				case 12:
					_SampPumpMotRun(-4244, 240);	// ��һ����100ul  100 / 0.023563 = 4244
					//_SampPumpMotRun(-4371, 240);	// ��һ����103ul  103 / 0.023563 = 4371
					Num++;
					waitMotSampPump = 1;
					workStep = 13;
					break;
				case 13:	// �뿪�����ȸ߶�
					_NeedleMotRunTo(MixHeight - 200,240);
					waitMotSampNeedle = 1;
					workStep = 14;
					break;
				case 14:
					_SampPumpMotRun(424, 120);		// ��10ul����   10 / 0.023563 = 424
					waitMotSampPump = 1;
					workStep = 15;
					break;
				case 15:	// �����������ȸ߶�
					_NeedleMotRunTo(MixHeight,240);
					waitMotSampNeedle = 1;
					workStep = 20;
					break;
//#ifndef _FluidPumMix 
				// ΢��ע���ó�����
				case 20:
					//_SampPumpMotRun(4032, 240);		// ��95ul  95 / 0.023563 = 4032
					_SampPumpMotRun(3820, 240);			// ��90ul  90 / 0.023563 = 3820
					waitMotSampPump = 1;
					workStep = 21;
					break;
				case 21:		// ע��90Һ��
					_SampPumpMotRun(-3820, 240);	// ��90 90/0.023563 = 3820
					Num++;
					waitMotSampPump = 1;
					workStep = 22;
					break;
				case 22:
					_SampPumpMotRun(3820, 240);		// ��90 90/0.023563 = 3820	
					if(Num < _ReMixNum - 1)
						workStep = 21;
					else
						workStep = 23;
					waitMotSampPump = 1;
				   break;
				case 23:
					//_SampPumpMotRun(-4032,240);			// ��95 95/0.023563 = 4032
					_SampPumpMotRun(-3820,240);				// ��90 90/0.023563 = 3820
					waitMotSampPump = 1;
					workStep = 24;
					break;
				case 24:
					_SampPumpMotRun(3395, 240);			// ��80 80/0.023563 = 3395	
					workStep = 25;
					waitMotSampPump = 1;
				   break;
				case 25:
				   SetDelayTime(MOT_SAMP_NEEDLE,2);
				   workStep = 16;
				   break;
				case 16:
				   Num = 0;
				   workStep = 17;
				   Uart0ReUnable;
				   uart_Printf("%s\r\n",strM3199);
				   Uart0ReEnable;
				   break;
				case 17:
				   _NeedleMotRunTo(0,240);	//������ص���λ
				   waitMotSampNeedle = 1;
				   workStep = 18;
				   break;
				case 18:		//�����Ƚ���
				   SetDelayTime(MOT_SAMP_NEEDLE,3);
				   GetNewTestCard = 233;		// ����ȡ�����Կ���ʶ,�����Ƚ���,׼������
				   mainStep = 9;
				   workStep = 0;
				   break;
				}
			break;
		case 9:				// �ȴ����ȵ����Ƭ��,Ȼ����ϴ
			switch(workStep){
				case 0:		// ׼�������������Զ����Ƿ��п�λ���룬����ȴ�	// �����������ɴ˼��뿪ʼ
					if(GetNewTestCard == 254)	// ���Կ�Ƭ�Ѿ�׼����ϣ����濪ʼ�����Բ��������
					{		
						CurInsertRingNum = RingQueueInsertCalculate();	// ����ת��λ��						
						if(CurInsertRingNum != 0xff)	// ���ת��λ�ò��ҳɹ������Զ��в���
						{	
							FindNum = 0;
							ucTmp = InsertNewTest(&NewTestInfo,CurInsertRingNum);	// ���Զ��в���
							if(ucTmp == 0)	// �²��Բ������ɹ�
							{
								// ���²��Բ���������������ת��λ��
								RingQueueInsert(CurInsertRingNum,&NewTestInfo);		
								insertflag[CurInsertRingNum] = 0;
								// ����жƬ����
								if(NewTestInfo.testTime1)
									UnloadQueueAdd(CurInsertRingNum, SecondCount+NewTestInfo.testTime1+TEST_CYCLE_TIME+25); 
								else
									UnloadQueueAdd(CurInsertRingNum, SecondCount+NewTestInfo.testTime0+TEST_CYCLE_TIME+25);
								workStep = 1;
							}
						}
						else
						{
							// ���ת��λ�ò���ʧ�ܣ�5s֮�����²���
							SetDelayTime(MOT_SAMP_NEEDLE,50);	
							FindNum++;
							if(FindNum >= 3)	// ����3��ת��λ�ò���ʧ��,��ǰת��λ��Ϊǰһ��+1
							{
								FindNum = 0;
								//RingQueue.prevNum += 1;
								CurInsertRingNum = RingQueue.prevNum++;
								if(CurInsertRingNum >= RING_QUEUE_NUM)
									CurInsertRingNum -= RING_QUEUE_NUM;
								RingQueueInsert(CurInsertRingNum,&NewTestInfo);		
								insertflag[CurInsertRingNum] = 0;
								// ����жƬ����
								if(NewTestInfo.testTime1)
									UnloadQueueAdd(CurInsertRingNum, SecondCount+NewTestInfo.testTime1+TEST_CYCLE_TIME+25);
								else
									UnloadQueueAdd(CurInsertRingNum, SecondCount+NewTestInfo.testTime0+TEST_CYCLE_TIME+25);
								workStep = 1;
							}
						}
					}
					else if(GetNewTestCard == 250)
					{
					//	GetNewTestCard = 0;
						MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
						waitMotSampTurn = 1;
						workStep = 5;
					}
					break;
				case 1:		// ȡ�����½������Կ��Ϸ�
					_NeedleMotRunTo(DropHeight,180);	// �������Լ�Ƭ
					waitMotSampNeedle = 1;
					workStep = 2;
					break;
				case 2:		// ע��Һ��
					if(_DropVolume != 0)	// 2016-05-19 ���ӵ�����Ϊ0����
					{
						siTmp = _DropVolume +  GetDropVolumeFactor();
						_SampPumpMotRun(-siTmp, 180);
						Uart0ReUnable;
						uart_Printf("%s $%4d\r\n", strM3141, siTmp);
						Uart0ReEnable;
						waitMotSampPump = 1;
					}
					workStep = 3;
					break;
				case 3:		// ȡ������������ߵ�
					SetBeepAck();
					// �걾�Ѽ��룬���Կ�ʼ
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("%s $%8d",strM3111, NewTestInfo.testSerial);
					uart_Printf(" $%8d\r\n",(unsigned long)SecondCount);
					Uart0ReEnable;
#else		
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3111);
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf(" $ ");
					uart0SendInt(SecondCount);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					_NeedleMotRunTo(0,180);
					GetNewTestCard = 255;		// ��case 4�ƶ���������֤��������Ʒ���Բ���
					_FluidMotRun(5, 64);		// ������ϴҺϴ���ȳ�,ע��1.4mL��ϴҺ
#if 	(DILUTE_TUBE == 14)
					_DiluentMotRun(8, 160);		// ע��ϡ��Һ��ϴ
#elif 	(DILUTE_TUBE == 16)
					_DiluentMotRun(3, 200);		
#endif
					checkFluid = 1;
					waitMotSampNeedle = 1;
					workStep = 4;
					break;
				case 4:		// ȡ�������е����ȳ��в��Ϸ�
					_EffluentMotRun(5, 220);
					_FluidMotRun(-1, 100);		// �س�
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:		// ȡ�����½������ȳض���
					MotStop(MOT_EFFLUENT);		// �����Һ��δֹͣ������ֹͣ
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP);
					waitMotSampNeedle = 1;
					waitMotFluid = 1;	// �ȴ���ϴҺע����ȳ����
					SetEValve(EV2,EV_OPEN);		// ����ȡ����ͨ��׼����ϴȡ�����ڱ�
					SetEValve(EV3,EV_CLOSE);
					//workStep = 6;	// 2016-09-18
					//if(NewTestInfo.sampDiluteMult == 1 || NewTestInfo.sampDiluteMult == 2 || NewTestInfo.sampDiluteMult == 6)
					//add by pan 20161110ϡ�ͱ�����50��ʱ��
					if(NewTestInfo.sampDiluteMult < 7 || NewTestInfo.sampDiluteMult > 11)
					// ϡ�ͱ���С��100,��ԭ����ϴģʽ
					{
						sc = 0;		// ԭ����ϴ����
						workStep = 6;
					}
					else
					{
						workStep = 10;	
					}
					break;
				case 6:		// ԭ��ģʽȡ�����ڱ���ϴ
					sc ++;
					SetEValve(EV2,EV_OPEN);	
					_FluidMotRun(10, 100);		// ע����ϴҺ
					MotInitCheck(MOT_SAMP_PUMP);	// ��Ʒע��������
					waitMotFluid = 1;
					workStep = 7;
					break;
				case 7:
					_EffluentMotRun(15, 240);		// 2017-05-22 220->240
					waitEffluent = 1;
				//	if(sc < 3)						// 2017-05-23 sc < 3 -> sc < 1
				//		workStep = 6;
				//	else
						workStep = 10;
					break;
				case 10:		// ϡ��ģʽ��ϴȡ�����ڱ�
					//if(NewTestInfo.sampDiluteMult == 1)	// 2016-09-18
					//_SampPumpMotRunTo(0, 220);			// 2016-09-18
					_FluidMotRun(8, 100);		// ע����ϴҺ
					MotInitCheck(MOT_SAMP_PUMP);	// ��Ʒע��������
					waitMotSampPump = 1;
					waitMotFluid = 1;
					workStep = 11;
					break;
				case 11:
					SetEValve(EV2,EV_CLOSE);
					SetEValve(EV1,EV_OPEN);
					SetDelayTime(MOT_SAMP_NEEDLE, 2); 
					workStep = 12;
					break;
				case 12:		// ����ϡ��ģʽ ��ϴȡ�������
					SetEValve(EV2,EV_CLOSE);
					SetEValve(EV1,EV_OPEN);
					_FluidMotRun(8,120);					
					waitMotFluid = 1;
					workStep = 13;
					break;
				case 13:		// ������ѹ
					checkFluid = 0;
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(55, 240);		
					workStep = 14;
					break;
				case 14:		// ȡ����������������ϴ��
					_FluidMotRun(-1, 160);
					MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotSampNeedle = 1;
					waitMotFluid = 1;
					if(NewTestInfo.sampDiluteMult ==13)
					{
						sc=4;
						workStep = 100;
					}
					else
					workStep = 15;
					break;
					
				case 100:
				    sc--;
				    SetEValve(EV3, EV_CLOSE);	// ��Һ��ת���Ż��ȳ�
				    _DiluentMotRun(16, 140);	
					workStep = 101;
					waitMotDiluent = 1;
					SetDelayTime(MOT_FLUID, 2);
					break;
				case 101:
				    _EffluentMotRun(20, 240);	
					SetDelayTime(MOT_FLUID, 2);
					waitEffluent = 1;
					if(sc==0)
					workStep = 102;
					else
					workStep = 100;
					break;
				case 102:
				    _DiluentMotRun(16, 140);					
					waitMotDiluent = 1;
					SetDelayTime(MOT_FLUID, 2);
					workStep = 103;
					break;	
				case 103:
				    MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM - 150);
					waitMotSampNeedle = 1;
					SetDelayTime(MOT_FLUID, 2);
					workStep = 104;
					break;
				case 104:
				    SetEValve(EV1,EV_CLOSE);
					SetEValve(EV2, EV_OPEN);	// ������ϴҺȡ����ͨ����׼������ϴҺ�������Ũ����ϴҺ
					SetDelayTime(MOT_FLUID, 2);
					
					sc=6;				
					workStep = 105;
					break;
				case 105:
				    sc--;
					MotRun(MOT_FLUID, -150);//��
					waitMotFluid = 1;
					SetDelayTime(MOT_FLUID, 2);
					if(sc==0)
					workStep = 106;
					else
					workStep = 109;
					break;
				case 109:
				    MotRun(MOT_FLUID, 140);//��
					SetDelayTime(MOT_FLUID, 2);
					waitMotFluid = 1;
					workStep = 105;
					break;		
				case 106:
				    SetEValve(EV2, EV_CLOSE);
					SetEValve(EV1, EV_CLOSE);
					SetEValve(EV3, EV_CLOSE);
					SetDelayTime(MOT_FLUID, 2);
					workStep = 107;
					break;
				case 107:
				    _EffluentMotRun(20, 240);	
					waitEffluent = 1;
					SetDelayTime(MOT_FLUID, 2);
					workStep = 108;
					break;
					
				case 108:
				    MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotSampNeedle = 1;
					workStep = 15;
					break;
				
				
				case 15:	// ȡ������ת��ȡ��λ
					SetEValve(EV1,EV_CLOSE);
					SetMotRunPam(MOT_SAMP_TRUN,255,5,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
					waitMotSampTurn = 1;
					workStep = 19;
					break;
				case 19:
					SetEValve(EV3, EV_CLOSE);	// ��Һ��ת���Ż��ȳ�
					workStep = 16;
					break;
				case 16:	// ȡ�����½���ȡ��λ
				//	if(InsertRingFlag == 0)		break;		// �Լ�Ƭδ�ƽ�װ��
					InsertRingFlag = 0;
#ifndef Puncture				
					_NeedleMotRunTo(_POS_SAMP_DOWN, 255);
#endif					
					// Ԥ��������
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_INTERVAL + _SAMP_PUMP_AIR_ISOLATE);
					waitMotSampNeedle = 1;
					workStep = 17;
					break;
				case 17:	// 
					SetBeepPrompt();
					SetStateLedFree();
					workStep = 18;
					break;
				case 18:	// ����
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3102);
					Uart0ReEnable;
					mainStep = 1;
					workStep = 0;
					break;
				}
			break;
		case 10:	// �����˳�
			switch(workStep){
				case 0:
					_EffluentMotRun(30, 220);
					_NeedleMotRunTo(0, 180);//240
					waitMotSampNeedle = 1;
					workStep = 1;
					TestALampClose();	// �رչ�Դ
					SetStateLedBusy();	// ״ָ̬ʾ��Ϊ��
					break;
				case 1:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					waitEffluent = 1;
					workStep = 2;
					break;
				case 2:
					mainStep = 0;
					workStep = 0;
					return 2;
					break;
				default:
					break;
				}
			return 1;
			break;
		case 11:	// �쳣����
			switch(workStep){
				case 0:		// ֹͣ��ǰ����
					SetDelayTime(MOT_EFFLUENT, 30);
					workStep = 1;
					break;
				case 1:		// ��Һ���ӳ�ֹͣ
					MotStop(MOT_EFFLUENT);
					workStep = 2;
					break;
				case 2:		// ȡ����ص���ʼ��
					_NeedleMotRunTo(0, 180);//240
					waitMotSampNeedle = 1;
					workStep = 3;
					break;
				case 3:		// ��Һ����Һ
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(30, 200);
					waitEffluent = 1;
					workStep = 4;
					break;
				case 4:		// ȡ�������е���ϴ���Ϸ�
					MotInitCheck(MOT_SAMP_TRUN);
				//	MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:
					
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 6;
					break;
				case 6:		// ȡ�����½�����ϴ����׼����ϴ
					_NeedleMotRunTo(_POS_MIX_TOP, 180);//240
					waitMotSampNeedle = 1;
					workStep = 7;
					break;
				case 7:		// ע����ϴҺ�����ȳ��У�Ȼ����ת�������Ĳ��Խ�����ϴ���򣬼���������������
					_FluidMotRun(10, 80);		// ������ϴҺϴ���ȳ�,ע��1.4mL��ϴҺ
					waitMotFluid = 1;	// �ȴ���ϴҺע����ȳ����
					mainStep = 9;		// ֱ�ӽ�����ϴ
					workStep = 6;
					InsertRingFlag = 1;	// ��֤����������
					break;
				default:
					break;
				}
			break;
		case 12:	// ��ϴҺ��
			switch(workStep){
				case 0:
					if(0 == CleanMode)
					{
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("!3901 $%8d\r\n", NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("!3901 $ ");
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif	
					}
					else
					{
						if(1 == CleanMode)
							CleanMode = 250;
						else if(2 == CleanMode)
							CleanMode = 255;
						Uart0ReUnable;
						uart_Printf("%s \r\n", strE3905);
						Uart0ReEnable;
						mainStep = 0;
						workStep = 16;
						break;
					}
					MotStop(MOT_SAMP_NEEDLE);
					MotStop(MOT_FLUID);
					SetDelayTime(MOT_EFFLUENT, 30);
					workStep = 1;
					break;
				case 1:
					MotStop(MOT_EFFLUENT);
					workStep = 2;
					break;
				case 2:
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(30, 200);
					waitEffluent = 1;
					workStep = 3;
					break;
				case 3:
					MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotSampNeedle = 1;
					workStep = 4;
					break;
				case 4:	// ȡ����ص���ʼ��
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:	// ȡ�������е����ȳ���׼����ϴ���ڱ�
					MotRunTo(MOT_SAMP_TRUN,NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 6;
					break;
				case 6:
					_NeedleMotRunTo(_POS_MIX_TOP, 200);//240
					SetEValve(EV2, EV_OPEN);
					waitMotSampNeedle = 1;
					workStep = 7;
					break;
				case 7:
					_FluidMotRun(10, 60);		// ������ϴҺϴ���ȳ�,ע��1.4mL��ϴҺ
					_EffluentMotRun(20, 80);
					waitMotFluid = 1;	// �ȴ���ϴҺע����ȳ����
					waitEffluent = 1;
					workStep = 8;
					break;
				case 8:
					SetEValve(EV_ALL, EV_CLOSE);
					workStep = 9;
					break;
				case 9:
					SetDelayTime(MOT_EFFLUENT, 2);
					ucTmp = _PrimingFluid();
					if(ucTmp == 1){	// �Զ���ע���
						workStep = 3;
						mainStep = 11;
						}
					else if(ucTmp == 0xff){		// �Զ���עʧ�ܣ��������ʼ�׶εȴ��ֶ���ʼҺ���ע
					/*
#ifndef UartSendLong
						Uart0ReUnable;
						uart_Printf("%s $%8d\r\n", strE3902,NewTestInfo.testSerial);
						Uart0ReEnable;
#else
						Uart0ReUnable;
						uart_Printf("%s $ ",strE3902);
						uart0SendInt(NewTestInfo.testSerial);
						uart_Printf("\r\n");
						Uart0ReEnable;
#endif
					*/
						Uart0ReUnable;
						uart_Printf("%s\r\n", strE3902);
						Uart0ReEnable;
						mainStep = 0;
						workStep = 1;
						}
					break;
				default:
					break;
				}
			break;
		case 13:	// ϡ��Һ��
			switch(workStep){
				case 0:
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("!3903 $%8d\r\n", NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("!3903 $ ");
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					MotStop(MOT_SAMP_NEEDLE);
					MotStop(MOT_DILUENT);
					SetDelayTime(MOT_EFFLUENT, 30);
					workStep = 1;
					break;
				case 1:
					MotStop(MOT_EFFLUENT);
					workStep = 2;
					break;
				case 2:
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(30, 200);
					waitEffluent = 1;
					workStep = 3;
					break;
				case 3:
					MotInitCheck(MOT_SAMP_NEEDLE);
				//	_NeedleMotRunTo(0, 240);
					waitMotSampNeedle = 1;
					workStep = 4;
					break;
				case 4:	// ȡ����ص���ʼ��
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:	// ȡ�������е����ȳ���׼����ϴ���ڱ�
					MotRunTo(MOT_SAMP_TRUN,NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 6;
					break;
				case 6:
					_NeedleMotRunTo(_POS_MIX_TOP, 180);//240
					SetEValve(EV2, EV_OPEN);
					waitMotSampNeedle = 1;
					workStep = 7;
					break;
				case 7:
					_FluidMotRun(10, 60);		// ������ϴҺϴ���ȳ�,ע��1.4mL��ϴҺ
					_EffluentMotRun(20, 80);
					waitMotFluid = 1;	// �ȴ���ϴҺע����ȳ����
					waitEffluent = 1;
					workStep = 8;
					break;
				case 8:
					SetEValve(EV_ALL, EV_CLOSE);
					workStep = 9;
					break;
				case 9:
					SetDelayTime(MOT_EFFLUENT, 2);
					ucTmp = _PrimingDiluent();
					if(ucTmp == 1){	// ��ע���
						workStep = 3;
						mainStep = 11;
						}
					else if(ucTmp == 0xff){		// �Զ���עʧ�ܣ��������ʼ�׶εȴ��ֶ���ʼҺ���ע
					/*
#ifndef UartSendLong
						Uart0ReUnable;
						uart_Printf("%s $%8d\r\n", strE3904,NewTestInfo.testSerial);
						Uart0ReEnable;
#else
						Uart0ReUnable;
						uart_Printf("%s $ ",strE3904);
						uart0SendInt(NewTestInfo.testSerial);
						uart_Printf("\r\n");
						Uart0ReEnable;
#endif
					*/
						Uart0ReUnable;
						uart_Printf("%s\r\n", strE3904);
						Uart0ReEnable;
						mainStep = 0;
						workStep = 1;
						}
					break;
				default:
					break;
				}
			break;
		case 20:	// �������״̬, ȡ�������λ
			switch(workStep)
			{
				case 0:	// ȡ������������ߵ�
					SetStateLedBusy();
					_NeedleMotRunTo(0, 180);// 240
					waitMotSampNeedle = 1;
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3190);
					Uart0ReEnable;
					workStep = 1;
					break;
				case 1:	// ȡ������ת����ʼλ
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 2;
					break;
				case 2:	// �������
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3191);		// ȡ������״̬
				//	uart_Printf("%s\r\n",strM3101);		// �밴��������ʼ
					Uart0ReEnable;
					JumpMode = 2;						// ����ģʽ,����֮��Ӧ�ý���Һ·�Լ�
					mainStep = 0;
					workStep = 1;
					break;
				default:
					break;
			}
			break;
		case 100:	// �����Ũ����ϴҺ��ϴ
			switch(workStep)
			{
				/*      //ԭ�ȵ�ǿ����ϴģʽ   deleted by pan  20161227
				case 0:
					SetEValve(EV2, EV_OPEN);	// ������ϴҺȡ����ͨ����׼������ϴҺ�������Ũ����ϴҺ
					SetDelayTime(MOT_FLUID, 2);
					workStep = 1;
					break;
				case 1:
					_FluidMotRun(-2, 120);		// �����Ũ����ϴҺ
					waitMotFluid = 1;
					workStep = 2;
					break;
				case 2:
					SetEValve(EV2, EV_CLOSE);
					_NeedleMotRunTo(0, 180);	// ȡ��������	// 200
					waitMotSampNeedle = 1;
					workStep = 3;
					break;
				case 3:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);		// ȡ�������е����ȳر��Ϸ�
					waitMotSampTurn = 1;
					SetDelayTime(MOT_SAMP_TRUN, 10*30);		// ����30��
					workStep = 5;
					break;
				case 5:
					_NeedleMotRunTo(_POS_MIX_BUTTOM, 200); // 240
					waitMotSampNeedle = 1;
					sc = 5;
					mainStep = 0;
					workStep = 9;
					break;
				default:
					break;
				}
			break;
			*/   //ԭ�ȵ�ǿ����ϴģʽ   deleted by pan  20161227
				
				case 0:
#ifdef Puncture
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);
					waitMotSampNeedle = 1;
					_WaitStartKey = 1;
#endif
					workStep = 100;
					break;
				case 100:
					SetEValve(EV2, EV_OPEN);	// ������ϴҺȡ����ͨ����׼������ϴҺ�������Ũ����ϴҺ
					SetDelayTime(MOT_FLUID, 2);
					workStep = 1;
					break;
				case 1:
					//_FluidMotRun(-2, 120);		// �����Ũ����ϴҺ120uL
					MotRun(MOT_FLUID, -120);
					waitMotFluid = 1;
					workStep = 2;
					//workStep = 100;
					break;
				/*
				case 100:	
					SetEValve(EV3, EV_OPEN);
					SetEValve(EV1, EV_OPEN);
					SetBeepPrompt();
					_EffluentMotRun(70, 240);
					SetDelayTime(MOT_EFFLUENT, 2);	// �ӳ�һ��ʱ�䣬�Ƚ�����ѹ
					workStep = 101;
					break;
				case 101:				// ����ϴҺ�ã���ϴ�����
					_FluidMotRun(20, 40);
					waitMotSampNeedle = 1;
					_NeedleMotRunTo(0, 180);	// ȡ��������
					//workStep = 6; //2016-09-18
					workStep = 102;	//2016-09-18
					break;
				
				case 102:		// �ӳ�1���ֹͣ��ϴͷ����, ת�������ȳ��ſ�
					SetEValve(EV_ALL,EV_CLOSE);
					//waitMotSampTurn = 1;
					SetEValve(EV2, EV_OPEN);	
					workStep = 3;
					break;
					*/
				case 2:
					//SetEValve(EV2, EV_CLOSE);
					_NeedleMotRunTo(0, 180);	// ȡ��������	// 200
					waitMotSampNeedle = 1;
					workStep = 3;
					break;
				case 3:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);		// ȡ�������е����ȳر��Ϸ�
					waitMotSampTurn = 1;
					SetDelayTime(MOT_SAMP_TRUN, 10*3);		// ����3��    �ĳɸ���ʱ��
					workStep = 5;
					break;				
				case 5:
					_NeedleMotRunTo(_POS_MIX_BUTTOM, 200); // 240
					waitMotSampNeedle = 1;
					workStep = 7;
					break;	
				case 7:
					MotRun(MOT_FLUID, 1200);
					checkFluid = 1;
					waitMotFluid = 1;
					workStep = 8;
					sc = 125;
					break;
				case 8:		
				    sc--;		
					MotRun(MOT_FLUID, -120);
					waitMotFluid = 1;
					workStep = 9;
					break;
			    case 9:	
				    MotRun(MOT_FLUID, 120);
					waitMotFluid = 1;	
					if(sc % 5 == 0)	
						SetDelayTime(MOT_FLUID, 10*6);	// ÿ���5����ͣ6��
					
					if(sc==0)
					{
					   workStep = 6;
					// sc=15;
					}
					else
					workStep = 8;		
					break;
				case 6:	
				    _EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					workStep = 13;
					sc=5;		//��ϴ���5��
					break;
				case 13:		// ��ϴȡ�������
				    sc--;					
					SetEValve(EV2, EV_CLOSE);	// ȡ����ͨ���ر�
					SetEValve(EV1, EV_OPEN);	// ������ϴͷϴҺ��Ӧ
					_FluidMotRun(8, 80);		// _FluidMotRun(12, 180);		// ������ϴҺϴ���ȳ�,ע��1.0mL��ϴҺ(12, 180)//(12,120)
					waitMotFluid = 1;
					waitEffluent = 1;
					SetEValve(EV3, EV_OPEN);	// ��ϴͷ��Һ��
					workStep = 14;
					break;
				case 14:		// �����ϴͷ
					_FluidMotRun(-1, 160);
					_EffluentMotRun(24, 240);
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					workStep = 15;
					break;
				case 15:	
					// ��ɻ��ȳ�
					SetEValve(EV3,EV_CLOSE);	// ��ϴͷ��Һ��
					_EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					if (sc!=0)
					workStep = 13;
					else
					{
					workStep = 10;
					sc=15;     //��ϴ�ڱ�15��
					}
					break;
				case 10:	// ��ϴ�͹�עȡ����ͨ��
				    sc--;
					SetEValve(EV2, EV_OPEN);
					SetEValve(EV1, EV_CLOSE);
					SetEValve(EV3, EV_CLOSE);
					MotInitCheck(MOT_SAMP_PUMP);
					_FluidMotRun(16, 64);		// ע��1.4mL��ϴҺ
					waitMotFluid = 1;
					SetDelayTime(MOT_FLUID, 40);
					workStep = 11;
					break;
				case 11:
					_EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					if(sc == 0)
					workStep = 12;
					else
					workStep = 10;
					break;			   
				case 12:		
					sc = 5;
					mainStep = 0;
					workStep = 9;
					break;
				default:
					break;
				}
			break;
		case 105:	// �Զ���ϴ
			switch(workStep)
			{
				case 0:
					_NeedleMotRunTo(0, 180);	// ȡ�������� // 200
					waitMotSampNeedle = 1;
					workStep = 3;
					break;
				case 3:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);		// ȡ�������е����ȳر��Ϸ�
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:
					_NeedleMotRunTo(_POS_MIX_BUTTOM, 180);	 // 240
					waitMotSampNeedle = 1;
					sc = 5;
					checkFluid = 1;
					mainStep = 0;
					workStep = 9;
					break;
				default:
					break;
				}
			break;
		default:
			break;
		}
	return 0;
}

signed int CalSampVolume(unsigned char multipNum, unsigned char diluteTime)
{
	unsigned long n, l;
	signed int m;
	n = 0;
	// ���ò�ͬϡ�ͱ����±�׼������
#ifndef Puncture
	l = (unsigned long)DiluentCoff[multipNum];	// ��ȡϡ��У׼����
#else
	if(WithoutPuncture != 0)
		l = (unsigned long)DiluentCoff[multipNum];	// ��ȡϡ��У׼����
	else
	{
		l = (unsigned long)DiluentCoff[8];	// ��ȡϡ��У׼����
		uart_Printf("*9944 CurrentDiluentCoff $%4d\r\n", l);
	}
#endif
	if(diluteTime == 0)
	{
		switch(multipNum)		// ��һ��ϡ��
		{
			// һ��ϡ��
			case 1:		//m = _DropVolume;	// ������ == ��Һ��
						m = _DropVolume+_SAMP_PUMP_INTERVAL + 210 - 45;	return m; break;
		//	case 2:		n = l*4244;	break;	// 1:2	100/0.023562 = 4244	
			case 2:		n = l*3400; break;  // 1:2  80/0.023562  = 3395
			case 3:		n = l*4244;	break;	// 1:5	100/0.023562 = 4244
			case 4:		n = l*2355;	break;	// 1:10	55.5/0.023562 = 2355
			case 5:		n = l*1116;	break;	// 1:20	26.3/0.023562 = 1116
		//	case 6:		n = l*864;	break;	// 1:50	20.4/0.023562 = 864
		//	case 6:		n = l*1039;	break;	// 1:50	24.5/0.023562 = 1039  // 4015 ����Ϊ114׼ȷ
		//	case 6: 	n = l*1183; break;  // 1183 = 1039 * 672 / 590,����672Ϊ����114��ֵ��590Ϊ����100��ֵ
		//	case 6: 	n = l*1261; break;  // 29.7ul
		//	case 6: 	n = l*1230; break;  // 29ul
		//	case 6: 	n = l*1200; break;  // 28.3ul
		//	case 6: 	n = l*1220; break;  // 28.7ul
		//	case 6:		n = l*1060; break;  // 25ul
		//	case 6:		n = l*1082; break;  // 25.5ul
		//	case 6:		n = l*1188; break;  // 28ul
			case 6:		n = l*923; break;   // 2016-10-25����  21.75ul
			case 7:		n = l*429;	break;	// 1:100	10.1/0.023562 = 429
			case 8:		n = l*212;	break;	// 1:200	5/0.023562 = 212
			// �߱��ʵĵ�һ��ϡ��
		//	case 9:		n = l*429;	break;	// 1:100 X 5	= 1:500
			case 9:		n = l*300;	break;	// 1:100 X 5	= 1:500	7.07/0.023562 = 300
			case 10:	n = l*429;	break;	// 1:100 X 10	= 1:1000	10.1/0.023562 = 429
			case 11:	n = l*212;	break;	// 1:200 X 10	= 1:2000	5/0.023562 = 212
			//  2016-05-18
			case 12:	n = l*4244;	break;	// 1:3  100/0.023562 = 4244		
			//case 13:	n = l*4244;	break;	// 1:4	100/0.023562 = 4244
			case 13:	n = l*2831; break;	// 1:4  66.7/0.023562 = 2831 
			case 14:    n = l*1634;	break;	// 1:40	38.5/0.023562 = 1634
			default:	n = 0;		break;
			}
		}
	n = n / _DILUENT_PUMP_BASE_COEFF;
	m = (signed int)n;
	return m;
}

signed int CalDiluteVolume(unsigned char multipNum, unsigned char diluteTime){
	// �������ϡ�ͱ����õ�ϡ��Һ�������Ϊϡ�ͱõ�n����λ����
	signed int n;
	if(diluteTime == 0){
		switch(multipNum){		// ��һ��ϡ��
			// һ��ϡ��
			case 1:		n = 0;		break;		// 1:1
			case 2:		n = 1;		break;		// 1:2		100:(100+100)
			case 3:		n = 4;		break;		// 1:5		100:(100+400)
			case 4:		n = 5;		break;		// 1:10		55.5:(55.5+500)	
			case 5:		n = 5;		break;		// 1:20		26.3:(26.3+500)	
		//	case 6:		n = 10;		break;		// 1:50		20.4:(20.4+1000)
		    case 6:		n = 12;		break;		// 1:50		24.5:(24.5+1200)
			case 7:		n = 10;		break;		// 1:100	10.1:(10.1+1000)
			case 8:		n = 10;		break;		// 1:200	5:(5+1000)
		//	case 9:		n = 10;		break;		// 1:500	1:100	10.1:(10.1+1000)
			case 9:		n = 7;		break;		// 1:500	1:100	7.07:(7.07+700)
			case 10:	n = 10;		break;		// 1:1000	1:100	10.1:(10.1+1000)
			case 11:	n = 10;		break;		// 1:2000	1:200	5:(5+1000)
			// 2016-05-18
			case 12:	n = 2;		break;		// 1:3		100:(100+200)
			//case 13:	n = 3;		break;		// 1:4		100:(100+300)
			case 13:	n = 2;		break;		// 1:4      66.7:(66.7+200)
			case 14:	n = 15;		break;		// 1:40		38.5:(38.5+1500)
			default:	 n = 0;		break;
			}
		}
	return n;
}
signed int CalDilute2Volume(unsigned char multipNum, unsigned char dilutetype){
	signed int n;
	if(dilutetype == 0)
	{	// ԭҺ
		switch(multipNum)
		{
			case 2:		n = 2;	break;			// 1:2 = 100 + 100 
			case 12:	n = 3;	break;			// 1:3 = 100 + 200
			case 13:	n = 4;	break;			// 1:4 = 100 + 300
			case 3:		
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 14:	n = 5;	break;		// 500ul
			case 9:		n = 2;	break;		// 1:500  = 1:100 X 5
			case 10:	n = 1;	break;		// 1:1000	= 1:100 X 10	
			case 11:	n = 1;	break;		// 1:2000	= 1:200 X 10	
			default:	n = 0;	break;
		}
	}
	else
	{	// ���Һ��
		switch(multipNum)
		{
			case 2:		n = 2;	break;
			case 12:	n = 3;	break;
			case 13:	n = 4;	break;
			case 3:		
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 14:	n = 5;	break;
			case 9:		n = 10;		break;		// 1:500	 = 1:100 X 5	
			case 10:	n = 10;		break;		// 1:1000	 = 1:100 X 10
			case 11:	n = 10;		break;		// 1:2000	 = 1:200 X 10	
			default:	 n = 0;		break;
		}
	}
	return n;
}
unsigned char CalSampSyringSpeed(unsigned char multipNum, unsigned char diluteTime){
	// ����걾ע���ٶ�
	unsigned char n;
	if(diluteTime == 0){
		switch(multipNum){		// ��һ��ϡ��
			// һ��ϡ��
			case 1:		n = 200;	break;		// 100	1:1
			case 2:		n = 200;	break;		// 100	1:2
			case 3:		n = 200;	break;		// 100	1:5
			case 4:		n = 200;	break;		// 55.5	1:10
			case 5:		n = 200;	break;		// 26.3	1:20
			//case 6:		n = 140;	break;		// 20.4	1:50
			case 6:		n = 200;	break;		// 20.4	1:50
			case 7:		n = 80;		break;		// 10.1	1:100
			case 8:		n = 60;		break;		// 5		1:200
			case 9:								// 1:500
			case 10:	n = 80;		break;		// 1:1000
			case 11:	n = 40;		break;		// 1:2000		32
			// 2016-05-18
			case 12:	n = 200;	break;		// 100	1:3
			//case 13:	n = 200;	break;		// 100	1:4
			case 13:	n = 180;	break;		// 66.7 1:4
			//case 14:	n = 140;	break;		// 20.5	1:40
			case 14:	n = 200;	break;		// 38.5	1:40
			default:	 n = 180;	break;
			}
		}
	return n;
}

unsigned char CalDiluentInjectSpeed(unsigned char multipNum, unsigned char diluteTime){
	// ����ϡ��Һע���ٶ�
	// ����걾ע���ٶ�
	unsigned char n;
	if(diluteTime == 0){
		switch(multipNum){		// ��һ��ϡ��
			// һ��ϡ��
			case 1:		n = 0;		break;		// 0		1:1
			//case 2:		n = 4;		break;		// 100	1:2
			case 2:		n = 25;		break;		// 100	1:2
			case 3:		n = 16;		break;		// 400	1:5
			case 4:		n = 40;		break;		// 500	1:10
			case 5:		n = 75;		break;		// 500	1:20
			case 6:		n = 160;	break;		// 1000	1:50
			case 7:		n = 160;	break;		// 1000	1:100
			case 8:		n = 160;	break;		// 1000		1:200
			case 9:								// 1:500
			case 10:							// 1:1000
			case 11:	n = 200;	break;		// 1:2000		160-30(cv <1.5)
						// 2017-05-23  n=160-10 -> n = 200
			// 2016-05-18
			//case 12:	n = 8;		break;		// 200	1:3
			case 12:	n = 50;		break;		// 200	1:3
			//case 13:	n = 12;		break;		// 300  1:4
			case 13:	n = 60;		break;		// 300  1:4
			//case 14:	n = 150;	break;		// 800	1:40
			case 14:	n = 220;	break;		// 1500	1:40
			default:	 n = 0;		break;
			}
		}
	return n;
}


/******************************************************************************/

void SetWorkStoreNum(unsigned char num){
	// ���ò��Կ��ֺ�	1~5,��С�ֿ�ʼ��
	if(num > 5)
		num = 5;
	if(num == 0)
		num = 1;
	_NewCardStoreNum = num;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3137, _NewCardStoreNum);
	Uart0ReEnable;
}
unsigned char GetWorkStoreNum(void){
	return _NewCardStoreNum;
}
//_CONST unsigned int RatioNumber[] = {0,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000};
_CONST unsigned int RatioNumber[] = {0,1,2,5,10,20,50,100,200,500,1000,2000,3,4,40};
void SetDiluentRatio(unsigned char num){
	// ����ϡ�ͱ���
	//if(num>13)
	//	num = 13;
	if(num == 0)
		num = 1;
	if(num > 14)
		num = 14;
	_NewMultipNum = num;
	if(1 != _NewMultipNum)	// ���ϡ�ͱ�����Ϊ1:1
		_MixtureMode = 0;
	Uart0ReUnable;
	uart_Printf("%s $%4d $%4d\r\n",strM3136, _NewMultipNum, RatioNumber[_NewMultipNum]);
	Uart0ReEnable;
}
void SetReadTime0(unsigned int t){
	// ����A���ͷ����ʱ��
	if(t>900)
		t = 900;
	if(t<20)
		t = 20;
	_NewReadTime0 = t;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3138, _NewReadTime0);
	Uart0ReEnable;
}
void SetReadTime1(unsigned int t){
	// ����B���ͷ����ʱ��
	_NewReadTime1 = 0;
	return;

	if(t>900)
		t = 900;
	if(t<20)
		t = 20;
	_NewReadTime1 = t;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3139, _NewReadTime1);
	Uart0ReEnable;
}
void SetReadMolule(unsigned char n){
	// ���ö���ͷ
	if(n == 0)
		_NewTestType = 0;
	else
		_NewTestType = 1;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3140, _NewTestType);
	Uart0ReEnable;
}

unsigned long _NewTestSetial(void){
	// �������к�
	unsigned char c;
	unsigned int i;
	unsigned long l;

	EEPROM_READ(EEP_ADD_SERIAL, c);		// ��ȡ��ʼ����ʶ
	if(c != 0xc5){		// ��ʼ��
		c = 0xc5;
		EEPROM_WRITE(EEP_ADD_SERIAL, c);
		c = 0;
		EEPROM_WRITE(EEP_ADD_SERIAL + 1, c);	// ��ʾ��д���8λ��
		i = 1;
		EEPROM_WRITE(EEP_ADD_SERIAL + 2, i);	// ��ʾ��д���16λ��
		return 1;
		}
	EEPROM_READ(EEP_ADD_SERIAL + 1, c);		// ��ȡ��8λ
	if(c < 48){
		EEPROM_READ(EEP_ADD_SERIAL+2+c*2, i);
		i++;
		if(i==0){	// ���µĵ�16λд���ַ
			c ++;
			EEPROM_WRITE(EEP_ADD_SERIAL + 1, c);	// ��ʼ��д���8λ��
			}
		EEPROM_WRITE(EEP_ADD_SERIAL+2+c*2, i);
		}
	l = c * 0x10000 + i;
	return l;
}
unsigned long ReadCurTestSetial(void){
	unsigned char c;
	unsigned int i;
	unsigned long l;
	l = 1;
	EEPROM_READ(EEP_ADD_SERIAL, c);		// ��ȡ��ʼ����ʶ
	if(c != 0xc5){		// ��ʼ��
		c = 0xc5;
		EEPROM_WRITE(EEP_ADD_SERIAL, c);
		c = 0;
		EEPROM_WRITE(EEP_ADD_SERIAL + 1, c);	// ��ʾ��д���8λ��
		i = 1;
		EEPROM_WRITE(EEP_ADD_SERIAL + 2, i);	// ��ʾ��д���16λ��
		return 1;
		}
	EEPROM_READ(EEP_ADD_SERIAL + 1, c);		// ��ȡ��8λ
	if(c < 48){
		EEPROM_READ(EEP_ADD_SERIAL+2+c*2, i);
		l = c * 0x10000 + i;
		}
	return l;
}
void _SetCurWorkParamter(void){
	NewTestInfo.cardStoreNum = _NewCardStoreNum;
	NewTestInfo.sampDiluteMult = _NewMultipNum;
	NewTestInfo.testTime0 = _NewReadTime0;
	NewTestInfo.testTime1 = _NewReadTime1;
	NewTestInfo.readType = _NewTestType;
	NewTestInfo.testSerial = _NewTestSetial();
//	NewTestInfo.testSerial ++;	// �����Զ����
	// ����²�����Ϣ
#ifndef UartSendLong
	Uart0ReUnable;
	uart_Printf("%s $%8d ",strM3103,NewTestInfo.testSerial);
#else
	Uart0ReUnable;
	uart_Printf("%s $ ",strM3103);
	uart0SendInt(NewTestInfo.testSerial);
#endif
	uart_Printf("$%4d $%4d $%4d $%4d $%4d\r\n", 
			NewTestInfo.sampDiluteMult, 
			NewTestInfo.cardStoreNum, 
			NewTestInfo.readType, 
			NewTestInfo.testTime0, 
			NewTestInfo.testTime1);
	Uart0ReEnable; 
}
/*
void SetSleepTime(unsigned int t){
	unsigned int i;
	// ��������ʱ��
	if(t>4095)
		t = 4095;
	_SleepTime = t;
	i = _SleepTime  + 0xc000;	// 0xc000 ��ʼ�����
	EEPROM_WRITE(EEP_ADD_SLEEP_TIME,  i);
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3152, _SleepTime);
	Uart0ReEnable;
}
*/

void TestSleep(void)
{
	// ��������
	_SleepTime = 1;
}
void TestStartup(void){
	// ��������
	_WaitStartKey = 0;
}
void _SetNewCardGet(unsigned char num)
{
	if(GetNewTestCard != 254 && GetNewTestCard != 222)	// �����Ƭ�Ѿ�ȡ����������ȡ��Ƭ
	{
		GetNewTestCard = num;	// ���ñ���ȡ��Ƭ�ֺ�
	}
}

// ��������������
//signed char SetDropVolumeFactor(signed char n)
void SetDropVolumeFactor(signed int n)
{
	if(n <= 500 && n >= -500)
	{	// ������Χ�ж�
		EEPROM_WRITE(EEP_DROP_VOLUME_FACTOR, n);
	}
	else
	{
		EEPROM_READ(EEP_DROP_VOLUME_FACTOR, _DropVolumeFactor);		// ���¶�ȡд�������ֵ
		Uart0ReUnable;
		uart_Printf("%s $%4d\r\n", strM3218, _DropVolumeFactor);
		Uart0ReEnable;
		return;
	}
	EEPROM_READ(EEP_DROP_VOLUME_FACTOR, _DropVolumeFactor);		// ���¶�ȡд�������ֵ
	if(_DropVolumeFactor > 500 || _DropVolumeFactor < -500)
	{		// �����ֵ���쳣
		_DropVolumeFactor = 0;
		EEPROM_WRITE(EEP_DROP_VOLUME_FACTOR, _DropVolumeFactor);
	}
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3218, _DropVolumeFactor);
	Uart0ReEnable;
}

signed int GetDropVolumeFactor(void)
{
	signed int sc;
	EEPROM_READ(EEP_DROP_VOLUME_FACTOR, sc);		// ���¶�ȡд�������ֵ
	if(sc > 500 || sc < -500)
	{		// �����ֵ���쳣
		sc = 0;
		EEPROM_WRITE(EEP_DROP_VOLUME_FACTOR, sc);
	}
	return sc;
}

void SetDropVolume(unsigned int vol)
{
	if(vol > 110)
		vol = 110;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3141, vol);
	Uart0ReEnable;
	_DropVolume = (vol * 425) / 10;	// ÿuL42.5����0.023562uL/��
	// 2016-06-17 ���� if(_DropVolume != 0)
	if(_DropVolume != 0)
		_DropVolume += 149; 	// 3.5uL
}


//********************************************************
//add 2016.7.6
void SetSamplingVolume(unsigned int vol)
{
	if(vol > 110)
		vol = 110;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3121, vol);
	Uart0ReEnable;
	_SamplingVolume = (vol * 425) / 10;	// ÿuL42.5����0.023562uL/��
}
//********************************************************

//********************************************************
//add 2016.4.20
void SetDropMode(unsigned int mode)
{
	if(mode > 1)
		mode = 1;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3142, mode);
	Uart0ReEnable;
	_DropMode = mode;
	//2016-06-15  �Ƿ���Ҫͬʱ���õ�����
	if(_DropMode)
	{
		;
	}
}

// ����_DropMode ģʽ��,����1:1�Ƿ���Ҫ������
void SetMixtureMode(unsigned int mode)
{
	if(0 == _DropMode)		return;			// ��_DropModeģʽ��,��Ч
	if(mode > 1)
		mode = 1;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3166, mode);
	Uart0ReEnable;
	_MixtureMode = mode;
}

void SetReMixNum(unsigned int MixNum)
{
  	if(_DropMode == 0) 
  	{
		Uart0ReUnable;
		uart_Printf("%s $%4d\r\n", strM3142, _DropMode);
    	uart_Printf("%s\r\n",strM3144);
		Uart0ReEnable;
    	return;
  	}
  	if(MixNum < 5)
    	MixNum = 5;
  	else if(MixNum > 20)
    	MixNum = 20;
  	_ReMixNum = MixNum;
	Uart0ReUnable;
  	uart_Printf("%s $%4d\r\n", strM3143, _ReMixNum);
	Uart0ReEnable;
}
//********************************************************

// ��������

/*
unsigned int  CalculateCalStandCoeff(unsigned int n)
{
	unsigned long l1,l2;
	unsigned int i;
	if(n == 0)
		return _DiluentCalChart.calStand;	// ���ص�ǰ����ֵ

	if(n>_DILUENT_MIX_BASE_COEFF_UP)
		n = _DILUENT_MIX_BASE_COEFF_UP;
	if(n<_DILUENT_MIX_BASE_COEFF_DOWN)
		n = _DILUENT_MIX_BASE_COEFF_DOWN;
	_DiluentCalChart.calStand = n;
	//Save_DiluentCalChart();
	return _DiluentCalChart.calStand;		// �������ú��У׼����
}
*/

// ������������Ϊ��ͬϡ�ͱ�����
// m Ϊϡ�ͱ���   n Ϊ��������
unsigned int  CalculateCalStandCoeff(unsigned int m,unsigned int n)
{
//#ifndef Puncture
	if(m > 13)	m = 13;
	if(m == 0)	m = 9;
	if(n == 0)
		return _DiluentCalChart.calStand[m];	// ���ص�ǰ����ֵ
	if(n > _DILUENT_MIX_BASE_COEFF_UP)
		n = _DILUENT_MIX_BASE_COEFF_UP;
	if(n < _DILUENT_MIX_BASE_COEFF_DOWN)
		n = _DILUENT_MIX_BASE_COEFF_DOWN;
	_DiluentCalChart.calStand[m] = n;
	return _DiluentCalChart.calStand[m];	// ���ص�ǰ����ֵ
/*
#else
{
	if(WithoutPuncture != 0)		// ���贩��
	{
		if(m > 13)	m = 13;
		if(m == 0)	m = 9;
		if(n == 0)
			return _DiluentCalChart.calStand[m];	// ���ص�ǰ����ֵ
		if(n > _DILUENT_MIX_BASE_COEFF_UP)
		n = _DILUENT_MIX_BASE_COEFF_UP;
		if(n < _DILUENT_MIX_BASE_COEFF_DOWN)
		n = _DILUENT_MIX_BASE_COEFF_DOWN;
		_DiluentCalChart.calStand[m] = n;
		return _DiluentCalChart.calStand[m];	// ���ص�ǰ����ֵ
	}
	else
	{
		if(n == 0)
			return _DiluentCalChart.calStand[8];	// ���ص�ǰ����ֵ
		if(n > _DILUENT_MIX_BASE_COEFF_UP)
		n = _DILUENT_MIX_BASE_COEFF_UP;
		if(n < _DILUENT_MIX_BASE_COEFF_DOWN)
		n = _DILUENT_MIX_BASE_COEFF_DOWN;
		_DiluentCalChart.calStand[8] = n;
		return _DiluentCalChart.calStand[8];	// ���ص�ǰ����ֵ
	}
}
#endif
*/
}

// �˲��ֺ�����δʹ��
/*
unsigned int CalMixingHeight(unsigned char multipNum, unsigned char diluteTime)
{
	// �������Һ�߶�
	unsigned int n;
	if(diluteTime == 0)
	{
		switch(multipNum)		// ��һ��ϡ��
		{
			// һ��ϡ��
			case 1:		n = 1740;			break;		// 0 1:1
			case 2:		n = 1740-56;		break;		// 240uL-130uL=110uL 1:2
			case 3:		n = 1740-163;		break;		// 450uL-130uL=320uL 1:5
			case 4:		n = 1740-239;		break;		// 600uL-130uL=470uL 1:10
			case 5:		n = 1740-239;		break;		// 600uL-130uL=470uL 19 1:20
			case 6:		n = 1740-448;		break;		// 1010uL-130uL=880uL 33 1:50
			case 7:		n = 1740-443;		break;		// 1000uL-130uL=870uL 33 1:100
			case 8:		n = 1740-442;		break;		// 995uL-130uL=865uL 33 1:200
			// ����ϡ�͵ĵ�һ��ϡ��
			case 9:										// 1:500
			case 10:									// 1:1000
			case 11:									// 1:2000
			case 12:									// 1:5000	
			case 13:	n = 1740-443;		break;		// 33 1:10000
			default:	n = 1740;			break;
			}
		}
	else
	{			// ����ϡ��
		switch(multipNum)
		{
			// ����ϡ���еĵڶ���ϡ��
			case 9:		n = 1740-201;		break;		// 525uL-130uL=395uL 14 1:500
			case 10:	n = 1740-443;		break;		// 1000uL-130uL=870uL 30 1:1000
			case 11:	n = 1740-433;		break;		// 979uL-130uL=849uL 31 1:2000
			case 12:	n = 1740-417;		break;		// 949uL-130uL=819uL 31 1:5000
			case 13:	n = 1740-443;		break;		// 1000uL-130uL=870uL 33 1:10000
			default:	n = 1740;			break;
		}
	}
	return n;
}
*/

/********************************  Һ·�Լ촦������  ***************************************/

unsigned char DiluteStartCheck(INFO_EVENT * pInfoEvent)
{
//	static unsigned int liqDetBaseAdc;
//	static unsigned char waitMotSampTurn,waitMotSampNeedle, waitMotSampPump,waitMotFluid,waitMotDiluent,waitEffluent;
	static unsigned char mainStep,workStep, subStep;		
	static unsigned char pos;
	static unsigned char detRetry ;		// Һ��̽�����Դ���
	static unsigned char CalCnt;		// �ظ�У׼����
	static unsigned int CalValue1, CalValue2;
	static unsigned char cNum;

		
	static unsigned int mixLiqLevel;
	static unsigned int mixCalStartPos;
	unsigned char ucTmp;
	signed int siTmp, siTmp1;
	unsigned int i, n, m;

	if(pInfoEvent == 0)
	{
		if(WaitDelayTime(MOT_SAMP_PUMP))		return 0;
		if(WaitDelayTime(MOT_SAMP_TRUN))		return 0;
		if(WaitDelayTime(MOT_SAMP_NEEDLE))		return 0;
		if(WaitDelayTime(MOT_EFFLUENT))			return 0;
		if(WaitDelayTime(MOT_DILUENT))			return 0;
		
		if(waitMotSampTurn)		{	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
		if(waitMotSampNeedle)	{	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
		if(waitMotSampPump)		{	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotSampPump = 0;	}
		if(waitMotFluid)		{	if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)			return 0;	waitMotFluid = 0;	}
		if(waitMotDiluent)		{	if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE)		return 0;	waitMotDiluent = 0;	}
		if(waitEffluent)		{	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;	}
	}
	else
		return 0;

	switch(mainStep)
	{
		case 0:		// ȡ����λ�ó�ʼ��
			switch(workStep)
			{
				case 0:		// 	ȡ����ص���ʼλ
					ReadLiquidMonitorResult(0);
					ReadLiquidMonitorResult(1);
					ReadLiquidMonitorResult(2);
					ReadLiquidMonitorResult(3);
					SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
					SetMotRunPam(MOT_SAMP_TRUN,200,10,CURRENT_SAMP_TRUN);
					MotInitCheck(MOT_SAMP_NEEDLE);
					_EffluentMotRun(20, 120);
					waitMotSampNeedle = 1;
					workStep = 1;
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2100);
					Uart0ReEnable;
					break;
				case 1:		// ��ת�ۻص���ʼ��
					MotInitCheck(MOT_SAMP_TRUN);
					waitMotSampTurn = 1;
					// ��ȡȡ����λ��
					NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
					NeedleOnMixSidePos = GetNeedleOnMixSidePos();
					workStep = 2;
					break;
				case 2:		// ȡ�������е����ȳ����Ϸ�
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);	
					waitMotSampTurn = 1;
					waitEffluent = 1;
					detRetry = 0;
					workStep = 3;
					break;
				case 3:		// Ԥ����ϴͷ
					detRetry ++;
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(20, 120);
					_NeedleMotRunTo(_POS_MIX_TOP, 200);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					if( getLiqDetADC(NeedleChannel) < 500)	// ���Һ��̽��
						workStep = 110;
					else
						workStep = 4;
					break;
				case 110:	// ȡ�������,��Һ·�������
					SetEValve(EV2, EV_OPEN);
					_FluidMotRun(-1, 20);
					waitMotFluid = 1;
					workStep = 111;
					break;
				case 111:	// �ر�����
					SetDelayTime(MOT_FLUID, 20);
					SetEValve(EV2, EV_CLOSE);
					workStep = 112;
					break;
				case 112:	// �ٴμ��Һ��̽��
					if( getLiqDetADC(NeedleChannel) < 500)	// ���Һ��̽��
						workStep = 113;
					else
						workStep = 4;
					break;
				case 113:	// ��ϴͷҺ·����, ��Һ·�������
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(-1, 20);
					waitMotFluid = 1;
					workStep = 114;
					break;
				case 114:
					SetDelayTime(MOT_FLUID, 20);
					SetEValve(EV1, EV_CLOSE);
				//	if(detRetry < 4)
				//		workStep = 3;
				//	else
						workStep = 4;
					break;
				case 4:		// Һ��̽���ʼ��
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	
						workStep = 0;
						Uart0ReUnable;	
						uart_Printf("!2501\r\n");		// ��ʼҺ·�Լ죬�ڽ���Һ��̽�⹦�ܼ���ʱ��������
						Uart0ReEnable;
						break;// �˳�����
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc1 $%4d\r\n",liqDetBaseAdc);	
					workStep = 5;
					break;
				case 5:		// ȡ�������е����ȳ���,̽������Ƿ���Һ��, �����ν���
					SetEValve(EV_ALL, EV_CLOSE);
					SetMotRunPam(MOT_SAMP_NEEDLE, 100, 10, CURRENT_SAMP_NEEDLE);
					MotRunTo(MOT_SAMP_NEEDLE, (_POS_MIX_TOP+_POS_MIX_BUTTOM)/2);
					detRetry = 0;
					workStep = 6;
					break;
				case 6:		// Һ��̽�⣬�����̽�⵽Һ�壬�򱨾���Һ�쳣
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 200)
						{
							if(detRetry < 15)	// �ز����
							{
								detRetry ++;	
								break;	
							}
							// ���ȳ��в���Һ����Һ�쳣
							MotStop(MOT_SAMP_NEEDLE);
							SetBeepWarning();
							mainStep = 8;
							workStep = 0;
							SetDelayTime(MOT_SAMP_NEEDLE, 20);
							Uart0ReUnable;
							uart_Printf("!2502\r\n");	// ��ʼҺ·�Լ죬 �ڼ����ȳ�ʱ�� ���ֻ��ȳ����в���ˮ�� �����Һ���Լ���Һͨ��
							Uart0ReEnable;
							break;
						}
						else
							detRetry = 0;
					}
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						// ȡ�����½�ֹͣ��δ̽�⵽����Һ��,����
						workStep = 7;
					}
					break;
				case 7:		// Һ��̽���ʼ��
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	
						workStep = 0;	
						break;// �˳�����
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc2 $%4d\r\n",liqDetBaseAdc);	
					SetDelayTime(MOT_SAMP_NEEDLE, 20);
					workStep = 8;
					break;
				case 8:		// ȡ�������е����ȳ���,̽������Ƿ���Һ��, �ڶ�`��
					SetEValve(EV_ALL, EV_CLOSE);
					SetMotRunPam(MOT_SAMP_NEEDLE, 60, 20, CURRENT_SAMP_NEEDLE);
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM);
					detRetry = 0;
					workStep = 9;
					break;
				case 9:		// Һ��̽�⣬�����̽�⵽Һ�壬�򱨾���Һ�쳣
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 150)
						{
							if(detRetry < 15)	// �ز����
							{
								detRetry ++;	
								break;	
							}
							// ���ȳ��в���Һ����Һ�쳣
							MotStop(MOT_SAMP_NEEDLE);
							SetBeepWarning();
							mainStep = 8;
							workStep = 0;
							SetDelayTime(MOT_SAMP_NEEDLE, 5);
							Uart0ReUnable;
							uart_Printf("!2502\r\n");	// ��ʼҺ·�Լ죬 �ڼ����ȳ�ʱ�� ���ֻ��ȳ����в���ˮ�� �����Һ���Լ���Һͨ��
							Uart0ReEnable;
							break;
							}
						else
							detRetry = 0;
					}
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						// ȡ�����½�ֹͣ��δ̽�⵽����Һ��,����
						workStep = 0;
						mainStep = 1;
					}
					break;
				}
			break;
		case 1:		// ��ע��ϴҺ
			SetDelayTime(MOT_EFFLUENT, 2);
			ucTmp = _PrimingFluid();
			if(ucTmp == 1)
			{
				workStep = 0;
				mainStep = 100;
			}
			else if(ucTmp == 0xff)
			{
				Uart0ReUnable;
				uart_Printf("%s\r\n",strE3902);	
				Uart0ReEnable;
				mainStep = 8;
				workStep = 0;
			}
			break;
		case 100:	// ��ϴ��ϴͷ��ȡ����ͨ��
			switch(workStep)
			{
				case 0:	// ������ѹ
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(100, 200);
					SetDelayTime(MOT_EFFLUENT, 5);
					workStep = 1;
				case 1:	// ��ϴ
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(30, 30);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetDelayTime(MOT_EFFLUENT, 20);
					waitMotSampNeedle = 1;
					workStep = 2;
					break;
				case 2:	// �ȹر���ϴҺ
					MotStop(MOT_FLUID);
					_FluidMotRun(-1, 80);
					SetDelayTime(MOT_EFFLUENT, 5);
					workStep = 3;
					break;
				case 3:	// �رո�ѹ
					SetEValve(EV_ALL, EV_CLOSE);
					MotStop(MOT_EFFLUENT);
					_NeedleMotRunTo(_POS_MIX_TOP, 240);
					waitMotSampNeedle = 1;
					workStep = 4;
					break;
				case 4:	// ��ϴ�͹�עȡ����ͨ��
					SetEValve(EV2, EV_OPEN);
					SetMotRunPam(MOT_SAMP_PUMP,64,60,CURRENT_SAMP_PUMP);
					MotInitCheck(MOT_SAMP_PUMP);
					_FluidMotRun(10, 64);
					waitMotFluid = 1;
					workStep = 5;
					break;
				case 5:	// ��ϴ��ϣ�����ϴͷ����ˮ
					SetEValve(EV_ALL, EV_CLOSE);
					SetEValve(EV3, EV_OPEN);
					_NeedleMotRunTo(0, 240);
					_EffluentMotRun(20, 200);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					workStep = 6;
					break;
				case 6:	// �Ÿɻ��ȳ�ˮ
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(20, 220);
					waitEffluent = 1;
					workStep = 7;
					break;
				case 7:	// ������س�Һ�壬�γɸ����
					SetEValve(EV2, EV_OPEN);
					SetDelayTime(MOT_DILUENT, 2);
					workStep = 8;
					break;
				case 8:
					_FluidMotRun(-2, 64);
					waitMotFluid = 1;
					workStep = 9;
					break;
				case 9:
					SetEValve(EV2, EV_CLOSE);
					workStep = 0;
					mainStep = 2;
					break;
				}
			break;
		case 2:		// ��עϡ��Һ
			SetDelayTime(MOT_EFFLUENT, 2);
			ucTmp = _PrimingDiluent();
			if(ucTmp == 1)
			{
				workStep = 0;
				mainStep = 3;
			}
			else if(ucTmp == 0xff)
			{
				Uart0ReUnable;
				uart_Printf("%s\r\n", strE3904);
				Uart0ReEnable;
				mainStep = 8;
				workStep = 0;
			}
			break;
		// ��ϴҺ�úͷ�Һ����������
		case 3:	
			switch(workStep)
			{
				case 0:
					SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
					MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotSampNeedle = 1;
					workStep = 100;
					break;
				case 100:
					SetMotRunPam(MOT_SAMP_TRUN,200,5,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn = 1;
					workStep = 101;
					break;
				case 101:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 102;
					break;
				case 102:		// ȡ�������е�����Һ��ʼ��λ
					_NeedleMotRunTo(_POS_MIX_CAL_START, 240);
					waitMotSampNeedle = 1;
					//workStep = 103;
					workStep = 1;
					break;
				/*
				case 103:
					if(CheckLiqDetBase())
					{
						SetEValve(EV1, EV_OPEN);
						_FluidMotRun(-1, 30);
						waitMotFluid = 1;
						workStep = 104;
					}
					else
						workStep = 1;
					break;
				case 104:
					SetEValve(EV1, EV_CLOSE);
					if(CheckLiqDetBase())
					{
						SetEValve(EV2, EV_OPEN);
						_FluidMotRun(-1, 30);
						waitMotFluid = 1;
					}
					workStep = 1;
					break;
				*/
				case 1:		// ��ʼ��Һ��̽��
					SetEValve(EV_ALL, EV_CLOSE);
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	
						workStep = 0;	
						Uart0ReUnable;
						uart_Printf("!2510\r\n");	// ��ʼ׼����ϴҺ���������ʱ��Һ��̽�⹦���Լ���ִ���
						Uart0ReEnable;
						break;// �˳�����
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc3 $%4d\r\n",liqDetBaseAdc);	
					_SampPumpMotRun(100, 64);		// ע������룬��ֹ��ͷ��ˮ��
					workStep = 2;
					break;
				case 2:		// ע�����800mL��ϴҺ������ҺԤ���߶�	
					_FluidMotRun(9, 32);
					detRetry = 0;
					workStep = 3;
					break;
				case 3:		// �ȴ�̽�⵽Һ��
					if(GetLiquidMonitorState(1) == INFO_LIQ_EMPTY)
					{
						// ��ϴҺ��Ӧ�쳣
						SetBeepWarning();
						// �˳�����
						mainStep = 8;
						workStep = 0;
						MotStop(MOT_FLUID);
						MotInitCheck(MOT_SAMP_PUMP);
						SetDelayTime(MOT_SAMP_NEEDLE, 5);
						Uart0ReUnable;
						uart_Printf("!2511\r\n");	// ��ϴҺ���������ʱ��������ϴҺ��Ӧ�жϣ��������ϴҺ������
						Uart0ReEnable;
						break;
					}
					if(GetMotState(MOT_FLUID) == STA_SLAVE_FREE)
					{
						// �����ϴҺ���Ƿ������У����ֹͣ��ʾδע��Һ���ע��Һ��ƫ��
						SetBeepWarning();
						// �˳�����
						mainStep = 8;
						workStep = 0;
						MotInitCheck(MOT_SAMP_PUMP);
						SetDelayTime(MOT_SAMP_NEEDLE, 5);
						Uart0ReUnable;
						uart_Printf("!2512\r\n");	// ��ϴҺ���������ʱ��δ��⵽��ϴҺע�룬������ϴҺ�ú���������Һ·�Լ����
						Uart0ReEnable;
						break;
						}
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 30)
						{
							if(detRetry < 5)	// �ز����
							{
								detRetry ++;	
								break;	
							}
							MotStop(MOT_FLUID);
							MotInitCheck(MOT_SAMP_PUMP);
							workStep = 4;
							SetBeepAck();
						}
						else
							detRetry = 0;
					}
					break;
				case 4:		// ȡ�������е���λ
					_NeedleMotRunTo(_POS_MIX_TOP, 240);
					waitMotSampNeedle = 1;
					workStep = 5;
					break;
				case 5:		// ע��1.2mL����Һ
					_FluidMotRun(12, 64);
					workStep = 6;
					break;
				case 6:		// �����ϴҺ��Ӧ
					if(GetLiquidMonitorState(1) == INFO_LIQ_EMPTY)
					{
						// ��ϴҺ��Ӧ�쳣
						SetBeepWarning();
						// �˳�����
						mainStep = 8;
						workStep = 0;
						MotStop(MOT_FLUID);
						MotInitCheck(MOT_SAMP_PUMP);
						SetDelayTime(MOT_SAMP_NEEDLE, 5);
						Uart0ReUnable;
						uart_Printf("!2511\r\n");	// ��ϴҺ���������ʱ��������ϴҺ��Ӧ�жϣ��������ϴҺ������
						Uart0ReEnable;
						break;
						}
					if(GetMotState(MOT_FLUID) == STA_SLAVE_FREE)
					{
						workStep = 7;	// עҺ���
					}
					break;
				case 7:			// ��ʼ��Һ��̽��
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	
						workStep = 0;	
						Uart0ReUnable;
						uart_Printf("!2513\r\n");	// ��ϴҺ���������ʱ��Һ��̽�⹦���Լ���ִ���
						Uart0ReEnable;
						break;// �˳�����
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc4 $%4d\r\n",liqDetBaseAdc);	
					_SampPumpMotRun(100, 64);
					_NeedleMotRunTo(_POS_MIX_CAL_START-200, 16);
					detRetry = 0;
					workStep = 8;
					break;
				case 8:		// ȡ�����½�̽��Һ��
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						// ��� ȡ�����Ƿ������У����ֹͣ��ʾҺ��߶�̫��
						SetBeepWarning();
						// �˳�����
						mainStep = 8;
						workStep = 0;
						MotInitCheck(MOT_SAMP_PUMP);
					//	uart_Printf("!2514\r\n");	// ��ϴҺ���������ʱ�ڼ������ʱ��δ��⵽Һ��ע�룬����ϡ��Һ�ú���������Һ·�Լ����
						Uart0ReUnable;
						uart_Printf("!2514 $%4d $%4d\r\n",liqDetBaseAdc,i);  // 2016-08-24 ����
						Uart0ReEnable;
						break;
					}
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 90)
						{
							if(detRetry < 10)	// �ز����
							{
								detRetry ++;	
								break;	
							}
							MotStop(MOT_SAMP_NEEDLE);
							MotInitCheck(MOT_SAMP_PUMP);
							SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
							workStep = 9;
							SetDelayTime(MOT_SAMP_NEEDLE, 5);
							SetBeepAck();
						}
						else
							detRetry = 0;
					}
					break;
				case 9:		// ���㶨����
					MotInitCheck(MOT_SAMP_PUMP);
					waitMotSampPump = 1;
					i = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);
					mixLiqLevel = i;
					i = _POS_MIX_CAL_START - mixLiqLevel;
					Uart0ReUnable;
					uart_Printf("%s $%d\r\n",strM2111, i);
					Uart0ReEnable;
					SetDelayTime(MOT_SAMP_NEEDLE, 10);
					workStep = 10;
					break;
				// ��ʼ��Һ����
				case 10:
					_EffluentMotRun(10, 140);		// 1000/125 = 8
					waitEffluent = 1;
					workStep = 11;
					break;
				case 11:			// ��ʼ��Һ��̽��
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	workStep = 0;	
						Uart0ReUnable;
						uart_Printf("!2520\r\n");	// ��ʼ��Һ���������ʱ��Һ��̽�⹦���Լ���ִ���
						Uart0ReEnable;
						break;// �˳�����
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc5 $%4d\r\n",liqDetBaseAdc);	
					_SampPumpMotRun(100, 64);		// ע������룬��ֹ��ͷ��ˮ��
					_NeedleMotRunTo(_POS_MIX_CAL_START, 16);
					detRetry = 0;
					workStep = 12;
					break;
				case 12:	// Һ��̽��
					// ��� ȡ�����Ƿ������У����ֹͣ��ʾҺ��߶�̫��
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						SetBeepWarning();
						// �˳�����
					//	mainStep = 8;	
					//	workStep = 0;
					//	MotInitCheck(MOT_SAMP_PUMP);
					//	uart_Printf("!2521\r\n");	// �ڷ�Һ���������ʱ��ȡ����δ̽�⵽Һ�壬����Һ��̽�⹦�ܺ���������Һ·�Լ����
						workStep = 13;
						break;
					}
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 50)
						{
							if(detRetry < 10)	// �ز����
							{
								detRetry ++;	
								break;	
							}
							MotStop(MOT_SAMP_NEEDLE);
							SetDelayTime(MOT_SAMP_NEEDLE, 5);
							SetBeepAck();
							workStep = 13;
						}
						else
							detRetry = 0;
					}
					break;
				case 13:	// �����Һ������
					MotInitCheck(MOT_SAMP_PUMP);	// ע��ø�λ
					i = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);
					i = i-mixLiqLevel;
					Uart0ReUnable;
					uart_Printf("%s $%d\r\n",strM2112, i);
					Uart0ReEnable;
					SetDelayTime(MOT_SAMP_NEEDLE, 10);
					workStep = 14;
					break;
				case 14:
					SetEValve(EV_ALL,EV_CLOSE);
					_NeedleMotRunTo(0, 240);
					workStep = 15;
					break;
				case 15:
					_EffluentMotRun(20, 200);
					workStep = 0;
					mainStep = 4;
					subStep = 0;
					waitEffluent = 1;
					waitMotSampNeedle = 1;
					CalCnt = 5;
					CalValue1 = 0;
					CalValue2 = 0;
					InitFlowMeter(); // ��ʼ�����������ж�
					break;
				}
			break;
		case 4:		//  ϡ��Һ�䶯����������
			switch(workStep)
			{
				case 0:		// ׼��
					switch(subStep)
					{
						case 0:
							SetMotRunPam(MOT_SAMP_NEEDLE, 220, 20, CURRENT_SAMP_NEEDLE);
							MotInitCheck(MOT_SAMP_NEEDLE);
							waitMotSampNeedle = 1;
							subStep = 1;
							break;
						case 1:
							SetMotRunPam(MOT_SAMP_TRUN,200,10,CURRENT_SAMP_TRUN);
							MotRunTo(MOT_SAMP_TRUN, 0);
							waitMotSampTurn = 1;
							subStep = 2;
							break;
						case 2:
							MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
							waitMotSampTurn = 1;
							subStep = 3;
							break;
						case 3:		// ȡ�������е���λ,ͬʱȡ����������ϴͷ
							_FluidMotRun(-1, 70);
							SetEValve(EV3,EV_OPEN);
							_EffluentMotRun(100, 200);
							_NeedleMotRunTo(_POS_MIX_CAL_START - 100 - 100, 220); // 2017-05-26 -100 -> -100 - 100
							waitMotSampNeedle = 1;
							subStep = 4;
							break;
						case 4:
							MotStop(MOT_EFFLUENT);
							SetEValve(0, 0);
							subStep = 5;
							SetDelayTime(MOT_EFFLUENT, 5);
							break;
						case 5:
						/*	if(CheckLiqDetBase())
							{
								mainStep = 8;	
								workStep = 0;	// �˳�����
								Uart0ReUnable;
								uart_Printf("!2530\r\n");	// ��ʼ׼��ϡ��Һ��������ʱ��Һ��̽�⹦���Լ���ִ���
								Uart0ReEnable;
							}
							else
						*/
								subStep = 6;
							break;
						case 6:		// ע���һ��Һ��
							_SampPumpMotRun(100, 64);
							liqDetBaseAdc = getLiqDetADC(NeedleChannel);
							//uart_Printf("//LiqDetBaseAdc6 $%4d\r\n",liqDetBaseAdc);	
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(12+5, 16);
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(4, 64);	
#endif
							waitMotDiluent = 1;
						//	workStep = 2;
							subStep = 7;
							break;
						case 7:		// ϡ��Һ�س�
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(-5, 64);	
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(-1, 64);	
#endif
							waitMotDiluent = 1;
							workStep = 2;
							subStep = 0;
							break;
						}
					break;
				case 2:		// ̽��Һ��߶�
					switch(subStep)
					{
						case 0:		// ���Һ��̽��
							i = getLiqDetADC(NeedleChannel);
							if(i < liqDetBaseAdc)
							{
								i = liqDetBaseAdc - i;
								if(i > 30)		// Һ��߶��쳣
								{
									mainStep = 8;	
									workStep = 0;	// �˳�����
									Uart0ReUnable;
									uart_Printf("!2531\r\n");	// ��ϡ��Һ����������ʼҺ��߶Ȳ���ʱ������Һ��߶ȳ��ߣ�������µ���ȡ����߶Ⱥ���������Һ·�Լ����
									Uart0ReEnable;
									break;
								}
							}
							subStep = 1;
							break;
						case 1:
							liqDetBaseAdc = getLiqDetADC(NeedleChannel);
							//uart_Printf("//LiqDetBaseAdc7 $%4d\r\n",liqDetBaseAdc);	
							_NeedleMotRunTo(_POS_MIX_CAL_START + 50, 8);   // 2017-05-26 + 100 -> +50
							mixCalStartPos = 0;
							detRetry = 0;
							subStep = 2;
							break;
						case 2:	// �ȴ�ȡ����Ӵ���Һ��
							// ���ȡ�����Ƿ������У����ֹͣ��ʾ��ָ���г���δ̽�⵽Һ��
							if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
							{
								SetBeepWarning();
								// �˳�����
								mainStep = 8;
								workStep = 0;
								MotInitCheck(MOT_SAMP_PUMP);
								Uart0ReUnable;
								uart_Printf("!2532\r\n");	// ��ϡ��Һ����������ʼҺ��߶Ȳ���ʱ��δ̽�⵽Һ�棬����ϡ��Һ�ú���������Һ·�Լ����
								Uart0ReEnable;
								break;
							}
							i = getLiqDetADC(NeedleChannel);
							if(i < liqDetBaseAdc)
							{
								i = liqDetBaseAdc - i;
								if(i > 30)
								{
									if(detRetry < 5)	// �ز����
									{
										detRetry ++;	
										break;	
									}
									MotStop(MOT_SAMP_NEEDLE);
									subStep = 3;
									SetDelayTime(MOT_SAMP_NEEDLE, 10);
									SetBeepAck();
								}
								else
									detRetry = 0;
							}
							break;
						case 3:		// ��¼��ʼҺ��߶�
							mixCalStartPos = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);
							_NeedleMotRunTo(_POS_MIX_TOP, 200);
							MotInitCheck(MOT_SAMP_PUMP);
							SetDelayTime(MOT_DILUENT, 10);
							subStep = 0;
							workStep = 3;
							waitMotSampNeedle = 1;
							waitMotSampPump = 1;
							break;
						}
					break;
				case 3:		// ע�붨��Һ
					switch(subStep)
					{
						case 0:
					//		_DiluentMotRun(50, 80);		// ע��50����λ��Һ
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(40+5, 64);		// ע��40����λ��Һ
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(12+2, 64);		// ע��12+2����λ��Һ
#endif
							subStep = 1;
							break;
						case 1:
							if(GetLiquidMonitorState(0) == INFO_LIQ_EMPTY)
							{
								// ϡ��Һ��Ӧ�쳣
								SetBeepWarning();
								// �˳�����
								mainStep = 8;
								workStep = 0;
								MotStop(MOT_DILUENT);
								SetDelayTime(MOT_SAMP_NEEDLE, 5);
								Uart0ReUnable;
								uart_Printf("!2533\r\n");	// ��ϡ��Һ��������ʱ��ϡ��Һ��Ӧ�жϣ�����͸���ϡ��Һ����������Һ·�Լ����
								Uart0ReEnable;
								break;
							}
							if(GetMotState(MOT_DILUENT) == STA_SLAVE_FREE)
							{
								subStep = 2;
							}
							break;
						case 2:	
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(-5, 64);
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(-2, 64);
#endif
							waitMotDiluent = 1;
							subStep = 3;
							break;
						case 3:		// ȡ�����½�̽��Һ��߶�
						/*
							if(CheckLiqDetBase())
							{
								mainStep = 8;	workStep = 0;
								Uart0ReUnable;
								uart_Printf("!2534\r\n");	// ��ϡ��Һ��������Һ��߶Ȳ���ʱ��Һ��̽�⹦���Լ���ִ���
								Uart0ReEnable;
								break;// �˳�����
							}
							*/
							liqDetBaseAdc = getLiqDetADC(NeedleChannel);
							//uart_Printf("//LiqDetBaseAdc8 $%4d\r\n",liqDetBaseAdc);	
							_SampPumpMotRun(100, 64);
							_NeedleMotRunTo(_POS_MIX_CAL_START-225, 8);
							detRetry = 0;
							subStep = 4;
							break;
						case 4:	// �ȴ�ȡ����Ӵ���Һ��
							// ���ϡ��Һ���Ƿ������У����ֹͣ��ʾδע��Һ���Һ��̽��ʧ��
							if(GetMotState(MOT_SAMP_NEEDLE)==STA_SLAVE_FREE)
							{
								MotStop(MOT_SAMP_NEEDLE);
								SetBeepWarning();
								// �˳�����
								mainStep = 8;
								workStep = 0;
								MotInitCheck(MOT_SAMP_PUMP);
								Uart0ReUnable;
								uart_Printf("!2535\r\n");	// ��ϡ��Һ��������Һ��߶Ȳ���ʱ��δ̽�⵽Һ�棬����ϡ��Һ�ú���������Һ·�Լ����
								Uart0ReEnable;
								break;
							}
							i = getLiqDetADC(NeedleChannel);
							if(i < liqDetBaseAdc)
							{
								i = liqDetBaseAdc - i;
								if(i>90)
								{
									if(detRetry < 5)	// �ز����
									{
										detRetry ++;	
										break;	
									}
									MotStop(MOT_SAMP_NEEDLE);
									subStep = 5;
									SetDelayTime(MOT_SAMP_NEEDLE, 10);
									SetBeepAck();
								}
								else
									detRetry = 0;
							}
							break;
						case 5:
						//	MotInitCheck(MOT_SAMP_PUMP);
							MotRunTo(MOT_SAMP_PUMP, 0);
							i = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);		// ��¼����ʼֵ
							i = mixCalStartPos - i;	// 1680
#ifndef Puncture
							i = i*2;
#endif
							Uart0ReUnable;
							uart_Printf("// DiluteFlow:%d\r\n", i);
							Uart0ReEnable;
							m = JudgeFlowMeter(i);
							if(m)
							{
								Uart0ReUnable;
								uart_Printf("// FlowJudge:%d\r\n", m);
								Uart0ReEnable;
								i = InsetrDiluentFlowCalValue(m);
								if(i)
								{
									Uart0ReUnable;
									uart_Printf("%s $%4d\r\n",strM2110, i);
									Uart0ReEnable;
									CalCnt = 0;		
								}
								else
								{	
									Uart0ReUnable;
									uart_Printf("// CalibRetry:\r\n");
									Uart0ReEnable;
								//	CalCnt --;
								}
							}
							else
							{
							//	CalCnt = 2;
							}
							subStep = 0;
							workStep = 4;
							waitMotSampPump = 1;
							break;
						case 6:
							subStep = 0;
							workStep = 4;
							break;
						default:
							break;
						}
					break;
				case 4:
					_NeedleMotRunTo(0, 240);
					_EffluentMotRun(30, 120);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					if(CalCnt != 0)
					{
						CalCnt --;
						workStep = 0;
						mainStep = 4;
						subStep = 0;
					}
					else
						workStep = 5;
					break;
				case 5:
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn= 1;
					workStep = 6;
					break;
				case 6:
					if(WorkProcessStep == 3)
					{

						MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
						waitMotSampTurn= 1;
						cNum = 3;
						workStep = 9;
/*
						MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
						waitMotSampTurn= 1;
#ifndef Puncture
						workStep = 7;
#else
						workStep = 8;
#endif
*/
					}
					else
					{
						workStep = 0;
						mainStep = 7;
						subStep = 0;
					}
					break;
				case 7:
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);
					waitMotSampNeedle = 1;
					workStep = 8;
					break;
				case 8:
					workStep = 0;
					mainStep = 7;
					subStep = 0;
					break;
				case 9:	
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP);
					waitMotSampNeedle = 1;
					workStep = 10;
					break;
				case 10:// ��ϴ�͹�עȡ����ͨ��
					cNum--;
					SetEValve(EV2, EV_OPEN);
					MotInitCheck(MOT_SAMP_PUMP);
					_FluidMotRun(12, 100);		// ע��1mL��ϴҺ
					waitMotFluid = 1;
					SetDelayTime(MOT_FLUID, 40);
					workStep = 11;
					break;
				case 11:
					_EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					workStep = 12;
					break;
				case 12:
					if(cNum != 0)
						workStep = 10;
					else
						workStep = 13;
					break;
				case 13:	// ��ϴ��ϣ�����ϴͷ����ˮ
					SetEValve(EV_ALL, EV_CLOSE);
					SetEValve(EV3, EV_OPEN);
					_NeedleMotRunTo(0, 180);//240
					_EffluentMotRun(30, 200);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					workStep = 14;
					break;
				case 14:	// �Ÿɻ��ȳ�ˮ
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(20, 220);
					waitEffluent = 1;
					workStep = 15;
					break;
				case 15:
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn= 1;
					workStep = 16;
					break;
				case 16:
					MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
					waitMotSampTurn= 1;
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_INTERVAL + _SAMP_PUMP_AIR_ISOLATE);
					waitMotSampPump = 1;
#ifndef Puncture
					workStep = 7;
#else
					workStep = 8;
#endif
					break;
			}
			break;
		case 7:		// Һ·�Լ����
			SetBeepAck();
			SetEValve(EV_ALL, EV_CLOSE);
			workStep = 0;
			mainStep = 0;
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2120);
			Uart0ReEnable;
			return 1;
			break;
		case 8:		// Һ·�Լ�ʧ�ܣ�ʧ����ת������
			switch(workStep)
			{
				case 0:
					SetMotRunPam(MOT_SAMP_NEEDLE, 220, 20, CURRENT_SAMP_NEEDLE);	// �ָ�ȡ�����������в���
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(30, 200);
					waitMotSampNeedle = 1;
					workStep = 1;
					break;
				case 1:
					MotInitCheck(MOT_SAMP_TRUN);
					waitMotSampTurn= 1;
					waitEffluent = 1;
					if(WorkProcessStep == 3)		// ����ģʽ������������
						workStep = 3;
					else
						workStep = 2;
					break;
				case 2:
					workStep = 0;
					mainStep = 0;
					subStep = 0;
					Uart0ReUnable;
					uart_Printf("!2550\r\n");	// Һ·�Լ�ʧ���˳�
					Uart0ReEnable;
					return 1;
					break;
				case 3:
					MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
					waitMotSampTurn= 1;
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_INTERVAL + _SAMP_PUMP_AIR_ISOLATE);
					waitMotSampPump = 1;
#ifndef Puncture
					workStep = 4;
#else
					workStep = 2;
#endif
					break;
				case 4:
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);
					waitMotSampNeedle = 1;
					workStep = 2;
					break;
			}
			break;
		default:
			mainStep = 0;
			workStep = 0;
			break;
		}
	return 0;
}


/*******************************************************************************************/


void DiluteProcDatInit(void)
{
	unsigned int i;
	unsigned char *pChar;

	pChar = (unsigned char *)&(NewTestInfo);
	for(i=0; i<sizeof(SAMP_INFO); i++)
	{
		*pChar++ = 0;
	}
	CurInsertRingNum = 0xff;
	GetNewTestCard = 0;
	SecondCount = 0;
	// ��ʼ�����Բ���
	_NewCardStoreNum = 1;
	_NewMultipNum = 9;
	_NewReadTime0 = 60;
	_NewReadTime1 = 0;
	_NewTestType = 0;
//	_DropVolume = 0; 
	_DropVolume = 0;//60*42; 
	Read_DiluentCalChart();
	EEPROM_READ(EEP_ADD_SLEEP_TIME,  i);
	if((i & 0xc000) != 0xc000)		// 0xc000 ��ʼ�����
	{
		i = 1800;
	}
	_SleepTime = i;
	EEPROM_WRITE(EEP_ADD_SLEEP_TIME,  i + 0xc000);
	NewTestInfo.testSerial = ReadCurTestSetial();
}

void _FluidMotRun(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_FLUID, vel, 10, CURRENT_FLUID);
	else
		SetMotRunPam(MOT_FLUID, vel, 10, CURRENT_FLUID);
	MotRun(MOT_FLUID, n*100);			// ������ϴҺ��
}

void _EffluentMotRun(signed int n,unsigned char vel)
{
	SetMotRunPam(MOT_EFFLUENT, vel, 10, CURRENT_EFFLUENT);
	MotRun(MOT_EFFLUENT, n*100);			// ������Һ��
}

void _DiluentMotRun(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_DILUENT, vel, 10, CURRENT_DILUENT);
	else
		SetMotRunPam(MOT_DILUENT, vel, 10, CURRENT_DILUENT);
	MotRun(MOT_DILUENT, n*100);			// ����ϡ��Һ��
}

void _NewDiluentMotRun(signed int n,unsigned char vel)
{
	SetMotRunPam(MOT_DILUENT, vel, 10, CURRENT_DILUENT);
	MotRun(MOT_DILUENT, n);			
}

void _NeedleMotRun(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_SAMP_NEEDLE, vel, 5, CURRENT_SAMP_NEEDLE);
	else
		SetMotRunPam(MOT_SAMP_NEEDLE, vel, 5, CURRENT_SAMP_NEEDLE);
	MotRun(MOT_SAMP_NEEDLE, n);	
}

void _NeedleMotRunTo(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_SAMP_NEEDLE, vel, 5, CURRENT_SAMP_NEEDLE);
	else
		SetMotRunPam(MOT_SAMP_NEEDLE, vel, 5, CURRENT_SAMP_NEEDLE);
	MotRunTo(MOT_SAMP_NEEDLE, n);	
}

void _SampPumpMotRun(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_SAMP_PUMP, vel, 5, CURRENT_SAMP_PUMP);
	else
		SetMotRunPam(MOT_SAMP_PUMP, vel, 5, CURRENT_SAMP_PUMP);
	MotRun(MOT_SAMP_PUMP, n);	
}
void _SampPumpMotRunTo(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_SAMP_PUMP, vel, 5, CURRENT_SAMP_PUMP);
	else
		SetMotRunPam(MOT_SAMP_PUMP, vel, 5, CURRENT_SAMP_PUMP);
	MotRunTo(MOT_SAMP_PUMP, n);	
}

void SetAutoTestCycle(unsigned int num)
{
	_AutoTestCycleNum = num;
}

unsigned char SetDiluentQuit(void)
{
	_DiluentQuitFlag = 1;
	return 0;
}

unsigned char _CheckFluidSupply(void)
{
	if(GetLiquidMonitorState(1) == INFO_LIQ_EMPTY)
	{
		// ��ϴҺ��
		SetBeepWarning();
		if(0 == CleanMode)
		{
			Uart0ReUnable;
			uart_Printf("%s\r\n",strE2953);
			Uart0ReEnable;
		}
		return 1;
	}
	/*	
	else if(GetLiquidMonitorState(1) == INFO_LIQ_BUBBLE)
	{
		// ��ϴҺ����
		SetBeepWarning();
		uart0SendString(strE2956);	uart_Printf("\r\n");
		return 2;
	}
	*/
	return 0;
}
unsigned char _CheckDiluentSupply(void)
{
	if(GetLiquidMonitorState(0) == INFO_LIQ_EMPTY)
	{
		// ϡ��Һ��
		SetBeepWarning();
		Uart0ReUnable;
		uart_Printf("%s\r\n",strE2952);
		Uart0ReEnable;
		return 1;
	}
	/*
	else if(GetLiquidMonitorState(0) == INFO_LIQ_BUBBLE)
	{
		// ϡ��Һ����
		SetBeepWarning();
		uart0SendString(strE2955);	uart_Printf("\r\n");
		return 2;
	}
	*/
	return 0;
}

unsigned char _PrimingDiluent(void)
{
	static unsigned char mainStep = 0;
	static unsigned int i;
	static unsigned char ucTmp;
	static unsigned char detRetry;

	switch(mainStep)
	{
		case 0:		// ȡ����߶Ȼ���
			ReadLiquidMonitorResult(0);
			ReadLiquidMonitorResult(1);
			ReadLiquidMonitorResult(2);
			ReadLiquidMonitorResult(3);
			SetMotRunPam(MOT_SAMP_NEEDLE, 220, 20, 4);
			MotInitCheck(MOT_SAMP_NEEDLE);
			waitMotSampNeedle = 1;
			// ��ȡȡ����λ��
			NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
			SetEValve(EV_ALL, EV_CLOSE);
			mainStep = 1;
		//	mainStep = 9;	// �ر�Һ��̽��
			runNum = 5;
			break;
		case 1:
			SetMotRunPam(MOT_SAMP_TRUN,240,10,CURRENT_SAMP_TRUN);
			MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
			waitMotSampTurn = 1;
			mainStep = 4;
			break;
		case 4:		// 
			SetMotRunPam(MOT_SAMP_NEEDLE, 100, 20, CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			SetEValve(EV3, EV_OPEN);
			_EffluentMotRun(40, 160);	// ��ϴͷ����
			waitMotSampNeedle = 1;
			waitEffluent = 1;
			mainStep = 5;
			break;
		case 5:	// 	ȡ�������е����ȳ��Ϸ�
			SetEValve(EV3, EV_CLOSE);
			MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP + 100);	// 200
			waitMotSampNeedle = 1;
			detRetry = 0;
			mainStep = 6;
			break;
		case 6:	// ȡ����ܵ��������,��Һ·��ȡ�������
			SetEValve(EV1, EV_CLOSE);
			SetEValve(EV2, EV_OPEN);
			_FluidMotRun(-1, 20);
			waitMotFluid = 1;
			detRetry ++;
			mainStep = 7;
			break;
		case 7:
			SetDelayTime(MOT_FLUID, 30);
			SetEValve(EV2, EV_CLOSE);
			mainStep = 8;
			break;
		case 8:		// ��ʼ��Һ��̽��
			liqDetBaseAdc = getLiqDetADC(NeedleChannel);
			if(liqDetBaseAdc > 1015)
			{
				SetBeepWarning();
				Uart0ReUnable;
				uart_Printf("!2901 $%d\r\n", liqDetBaseAdc);		// Һ��̽��缫δ����, ����Һ��̽��缫�����Ƿ����
				Uart0ReEnable;
				mainStep = 0;
				return 0xff;
			}
			//else if(liqDetBaseAdc < 400)
			else if(liqDetBaseAdc < 200)
			{
				if(detRetry < 3)
				{
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(-1, 20);
					waitMotFluid = 1;
					mainStep = 6;
					break;
				}
				else
				{
					SetBeepWarning();
					MotInitCheck(MOT_SAMP_NEEDLE);
					Uart0ReUnable;
					uart_Printf("!2911\r\n");		// �ڽ���ϡ��Һ��עʱ��Һ��̽���Լ�����쳣	
					Uart0ReEnable;
					mainStep = 0;
					return 0xff;	
				}
			}
			else
			{
				liqDetBaseAdc = getLiqDetADC(NeedleChannel);
				mainStep = 9;
			}
			break;
		case 9:		// ����ϡ��Һ�úͷ�Һ��
			SetEValve(EV_ALL, EV_CLOSE);
#if		(DILUTE_TUBE == 14)
			_DiluentMotRun(300, 160);		// ����ע9mLҺ�壬������ʾ��עʧ�ܣ���ʾ�û���鹩Һ
			_EffluentMotRun(160, 80);
#elif	(DILUTE_TUBE == 16)
			_DiluentMotRun(100, 120);		// ����ע9mLҺ�壬������ʾ��עʧ�ܣ���ʾ�û���鹩Һ
			_EffluentMotRun(160, 140);
#endif
			SetDelayTime(15, 50);		// ���ù�ע��ʱ���������ڲ����ڵ�������ֹ�ⲿ�ܵ�Һ��δ����ʱ���жϹ�ע���
			detRetry = 0;
			mainStep = 10;
			break;
		case 10:		// �ȴ�Һ·�¼�
			SetDelayTime(MOT_EFFLUENT, 3);
			ReadLiquidMonitorResult(0);
			i = getLiqDetADC(NeedleChannel);
			if(i<liqDetBaseAdc)
			{
				i = liqDetBaseAdc - i;
				//if(i>300)	
				if(i > 150)
				{
					if(detRetry < 10)	// �ز����
					{
						detRetry ++;	
						break;	
						}
					// ̽�⵽��Һ�쳣
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepWarning();
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					uart_Printf("!2912\r\n");	// ϡ��Һ��ע�����м�⵽���ȳ���Һ��ˮλ���ߣ������Һ����Һ״̬
					// �˳�����
					mainStep = 21;
					}
				else
					detRetry = 0;
				}
				
			if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE && GetMotState(MOT_EFFLUENT) != STA_SLAVE_FREE)
			{
				if(WaitDelayTime(15)!=0)
				{
					ucTmp = GetLiquidMonitorState(0);	// ��ȡ0��Һ·
					if(ucTmp==INFO_LIQ_EMPTY || ucTmp==INFO_LIQ_BUBBLE)
					{
						SetDelayTime(15, 50);	// Һ·�п��������¿�ʼ��ʱ
					}
				}
				else
				{
					// ��ע���
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepAck();
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2102);
					Uart0ReEnable;
					mainStep = 20;
				}
			}
			else
			{
				if(runNum!=0)		// ��Ϊϡ��Һ������С����Ҫ����ע3��
				{
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
				//	_EffluentMotRun(5, 220);
					SetDelayTime(MOT_DILUENT, 20);
					mainStep = 9;
					runNum --;
					break;
				}
				// ��Һ�Ѿ�ֹͣ����עʧ��
				MotStop(MOT_DILUENT);
				MotStop(MOT_EFFLUENT);
				MotInitCheck(MOT_SAMP_NEEDLE);
				SetBeepWarning();
				Uart0ReUnable;
				uart_Printf("%s\r\n",strE3904);// ϡ��Һ��עʧ�ܣ� ����ϡ��Һ������������������������µ���ϡ��Һ�����������Թ�ע
				Uart0ReEnable;
				mainStep = 21;
			}
			break;
		case 20:		// ����˳�
			//SetEValve(EV_ALL, EV_CLOSE);
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			waitMotSampNeedle = 1;
			mainStep = 0;
			return 1;
			break;
		case 21:		// ʧ���˳�
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			waitMotSampNeedle = 1;
			mainStep = 0;
			return 0xff;
			break;
		default:
			mainStep = 0;
			break;
		}
	return 0;
}

unsigned char _PrimingFluid(void)
{
	static unsigned char mainStep = 0;
	static unsigned int i;
	static unsigned char ucTmp;
	static unsigned char detRetry;
//	static unsigned char waitMotSampTurn,waitMotSampNeedle,waitMotFluid,waitEffluent;

//	if(WaitDelayTime(MOT_FLUID))			return 0;
		
	if(waitMotSampTurn)		{	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
	if(waitMotSampNeedle)	{	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
	if(waitMotFluid)		{	if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)			return 0;	waitMotFluid = 0;	}
	if(waitEffluent)		{	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;	}
		
	switch(mainStep)
	{
		case 0:		// ȡ����߶Ȼ���
			SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			// ��ȡȡ����λ��
			NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
			waitMotSampNeedle = 1;
			SetEValve(EV_ALL, EV_CLOSE);
			mainStep = 1;
		//	mainStep = 8;	// �ر�Һ��̽��
			runNum = 5;
			break;
		case 1:		// ȡ������ת
			SetMotRunPam(MOT_SAMP_TRUN,240,10,CURRENT_SAMP_TRUN);
			MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
			waitMotSampTurn = 1;
			mainStep = 4;
			break;
		case 4:		// ȡ�������е�ϡ��Һ���Ϸ�λ��
			ReadLiquidMonitorResult(1);
			ReadLiquidMonitorResult(2);
			_NeedleMotRunTo(_POS_MIX_TOP + 100, 240);
			waitMotSampNeedle = 1;
			mainStep = 5;
			detRetry = 0;
			break;
		case 5:	// ȡ�������,��Һ·�������
			SetEValve(EV_ALL, EV_CLOSE);
			SetEValve(EV2, EV_OPEN);
			_FluidMotRun(-1, 20);
			waitMotFluid = 1;
			detRetry ++;
			mainStep = 6;
			break;
		case 6:
			SetDelayTime(MOT_FLUID, 30);
			SetEValve(EV2, EV_CLOSE);
			mainStep = 7;
			break;
		case 7:		// ��ʼ��Һ��̽��
			liqDetBaseAdc = getLiqDetADC(NeedleChannel);
			if(liqDetBaseAdc > 1015)
			{
				SetBeepWarning();
				MotInitCheck(MOT_SAMP_NEEDLE);
				Uart0ReUnable;
				uart_Printf("!2901 $%d\r\n", liqDetBaseAdc);		// Һ��̽��缫δ����, ����Һ��̽��缫�����Ƿ����
				Uart0ReEnable;
				mainStep = 0;
				return 0xff;
			}
			//else if(liqDetBaseAdc < 400)
			else if(liqDetBaseAdc < 200)
			{
				if(detRetry < 4)
				{
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(-2, 20);
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(40, 160);	// ��ϴͷ����
					waitMotFluid = 1;
					waitEffluent = 1;
					mainStep = 5;
					break;
				}
				else
				{
					SetBeepWarning();
					MotInitCheck(MOT_SAMP_NEEDLE);
					Uart0ReUnable;
					uart_Printf("!2906\r\n");		// �ڽ���ϡ��Һ��עʱ��Һ��̽���Լ�����쳣	
					Uart0ReEnable;
					mainStep = 0;
					return 0xff;	
				}
			}
			else
			{
				liqDetBaseAdc = getLiqDetADC(NeedleChannel);
				SetEValve(EV_ALL, EV_CLOSE);
				detRetry = 0;
				mainStep = 8;
			}
			/*
			if(CheckLiqDetBase())
			{
				MotInitCheck(MOT_SAMP_NEEDLE);
				mainStep = 0;
				Uart0ReUnable;
				uart_Printf("!2906\r\n");		// �ڽ���ϡ��Һ��עʱ��Һ��̽���Լ�����쳣
				Uart0ReEnable;
				return 0xff;	
			}
			*/
			break;
		case 8:		// ������ϴҺ�úͷ�Һ��
			_FluidMotRun(100, 60);		// ����ע30mLҺ�壬������ʾ��עʧ�ܣ���ʾ�û���鹩Һ
			_EffluentMotRun(105, 60);
			SetDelayTime(15, 60);		// ���ù�ע��ʱ���������ڲ����ڵ�������ֹ�ⲿ�ܵ�Һ��δ����ʱ���жϹ�ע���
			ReadLiquidMonitorResult(1);
			mainStep = 9;
			break;
		case 9:		// �ȴ�Һ·�¼�
			SetDelayTime(MOT_EFFLUENT, 3);
			ReadLiquidMonitorResult(1);
			if(GetMotState(MOT_EFFLUENT) != STA_SLAVE_FREE && GetMotState(MOT_FLUID) != STA_SLAVE_FREE) 
			{
				if(WaitDelayTime(15)!=0)
				{
					ucTmp = GetLiquidMonitorState(1);
					if(ucTmp==INFO_LIQ_EMPTY || ucTmp==INFO_LIQ_BUBBLE)
					{
						SetDelayTime(15, 60);	// Һ·�п��������¿�ʼ��ʱ
					}
				}
				else
				{
					// ��ע���
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepAck();
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2101);
					Uart0ReEnable;
					mainStep = 10;
				}
			}
			else
			{
				if(runNum!=0)		// ��Ϊϡ��Һ������С����Ҫ����ע3��
				{
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					SetDelayTime(MOT_DILUENT, 20);
					mainStep = 8;
					runNum --;
					break;
				}
				// ��Һ�Ѿ�ֹͣ����עʧ��
				MotStop(MOT_FLUID);
				MotStop(MOT_EFFLUENT);
				MotInitCheck(MOT_SAMP_NEEDLE);
				SetBeepWarning();
				Uart0ReUnable;
				uart_Printf("%s\r\n",strE3902);	// ��ϴҺ��עʧ�ܣ� ������ϴҺ������������������������µ�����ϴҺ�����������Թ�ע
				Uart0ReEnable;
				mainStep = 11;
			}
			i = getLiqDetADC(NeedleChannel);
			if(i < liqDetBaseAdc)
			{
				i = liqDetBaseAdc - i;
				//if(i > 300)
				if(i > 150)
				{
					if(detRetry < 10)	// �ز����
					{
						detRetry ++;	
						break;	
					}
					// ̽�⵽��Һ�쳣
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepWarning();
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					uart_Printf("!2907\r\n");	// ��ϴҺ��ע�����м�⵽���ȳ���Һ��ˮλ���ߣ������Һ����Һ״̬
					// �˳�����
					mainStep = 11;
				}
				else
					detRetry = 0;
			}
			break;
		case 10:		// ����˳�
			//SetEValve(EV_ALL, EV_CLOSE);
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			waitMotSampNeedle = 1;
			mainStep = 0;
			return 1;
			break;
		case 11:		// ʧ���˳�
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			waitMotSampNeedle = 1;
			mainStep = 0;
			return 0xff;
			break;
		default:
			mainStep = 0;
			break;
		}
	return 0;
}

unsigned char _ManualPrimingDiluent(void)
{
	static unsigned char mainStep = 0;
	unsigned char ucTmp;
	
	if(WaitDelayTime(MOT_EFFLUENT))		return 0;
	if(WaitDelayTime(MOT_DILUENT))		return 0;
	if(waitEffluent){if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;}
	
	switch(mainStep)
	{
		case 0:
			runNum = 5;
			mainStep = 1;
			break;
		case 1:		// ����ϡ��Һ�úͷ�Һ��
			SetEValve(EV_ALL, EV_CLOSE);
		//	_DiluentMotRun(600, 120);		// ����ע50mLҺ�壬������ʾ��עʧ�ܣ���ʾ�û���鹩Һ
		//	_EffluentMotRun(1000, 140);
			_DiluentMotRun(120, 120);		// ����ע50mLҺ�壬������ʾ��עʧ�ܣ���ʾ�û���鹩Һ
			_EffluentMotRun(200, 140);
			SetDelayTime(15, 30);		// ����������ˮ3���жϹ�ע�ɹ�
			mainStep = 2;
			break;
		case 2:		// �ȴ�Һ·�¼�
			SetDelayTime(MOT_EFFLUENT, 3);
			ReadLiquidMonitorResult(0);

			if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE && GetMotState(MOT_EFFLUENT) != STA_SLAVE_FREE)
			{
				if(WaitDelayTime(15) != 0)
				{
					ucTmp = GetLiquidMonitorState(0);	// ��ȡ0��Һ·
					if(ucTmp==INFO_LIQ_EMPTY || ucTmp==INFO_LIQ_BUBBLE)
					{
						SetDelayTime(15, 30);	// Һ·�п��������¿�ʼ��ʱ
					}
				}
				else
				{
					// ��ע���
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
					SetBeepAck();
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2102);
					Uart0ReEnable;
					mainStep = 3;
				}
			}
			else
			{
				if(runNum!=0)		// ��Ϊϡ��Һ������С����Ҫ����ע3��
				{
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
				//	_EffluentMotRun(5, 220);
					SetDelayTime(MOT_DILUENT, 20);
					mainStep = 1;
					runNum --;
					break;
				}
				// ��Һ�Ѿ�ֹͣ����עʧ��
				MotStop(MOT_DILUENT);
				MotStop(MOT_EFFLUENT);
				//MotInitCheck(MOT_SAMP_NEEDLE);
				SetBeepWarning();
				Uart0ReUnable;
//				uart_Printf("!2910\r\n");	// ϡ��Һ��עʧ�ܣ� ����ϡ��Һ������������������������µ���ϡ��Һ�����������Թ�ע
				uart_Printf("%s\r\n",strE3912);
				Uart0ReEnable;
				mainStep = 3;
			}
			break;
		case 3:		// ����˳�
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			mainStep = 4;
		case 4:
			mainStep = 0;
			return 1;
	}
	return 0;
}

unsigned char _ManualPrimingFluid(void)
{
	static unsigned char mainStep = 0;
	static unsigned int i;
	static unsigned char ucTmp;
	static unsigned char detRetry;
	
	if(WaitDelayTime(MOT_EFFLUENT))		return 0;
	if(WaitDelayTime(MOT_DILUENT))		return 0;
	if(waitEffluent){if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;}
	
	switch(mainStep)
	{
		case 0:
			runNum = 5;
			mainStep = 1;
			break;
		case 1:		// ������ϴҺ�úͷ�Һ��
			_FluidMotRun(100, 60);		// ����ע30mLҺ�壬������ʾ��עʧ�ܣ���ʾ�û���鹩Һ
			_EffluentMotRun(105, 60);
			SetDelayTime(15, 40);		// ����������ˮ3���жϹ�ע�ɹ�
			ReadLiquidMonitorResult(1);
			mainStep = 2;
			break;
		case 2:		// �ȴ�Һ·�¼�
			SetDelayTime(MOT_EFFLUENT, 3);
			ReadLiquidMonitorResult(1);
			if(GetMotState(MOT_EFFLUENT) != STA_SLAVE_FREE && GetMotState(MOT_FLUID) != STA_SLAVE_FREE) 
			{
				if(WaitDelayTime(15)!=0)
				{
					ucTmp = GetLiquidMonitorState(1);
					if(ucTmp==INFO_LIQ_EMPTY || ucTmp==INFO_LIQ_BUBBLE)
					{
						SetDelayTime(15, 40);	// Һ·�п��������¿�ʼ��ʱ
					}
				}
				else
				{
					// ��ע���
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					//MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepAck();
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2101);
					Uart0ReEnable;
					mainStep = 3;
				}
			}
			else
			{
				if(runNum!=0)		// ��Ϊϡ��Һ������С����Ҫ����ע3��
				{
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					SetDelayTime(MOT_DILUENT, 20);
					mainStep = 1;
					runNum --;
					break;
				}
				// ��Һ�Ѿ�ֹͣ����עʧ��
				MotStop(MOT_FLUID);
				MotStop(MOT_EFFLUENT);
				SetBeepWarning();
				Uart0ReUnable;
				uart_Printf("%s\r\n",strE3911);	// ��ϴҺ��עʧ�ܣ� ������ϴҺ������������������������µ�����ϴҺ�����������Թ�ע
				Uart0ReEnable;
				mainStep = 3;
			}
			break;
		case 3:		// ���/ʧ���˳�
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			mainStep = 4;
			break;
		case 4:
			mainStep = 0;
			return 1;
			break;
		}
	return 0;
}


void SetCleanMode(unsigned char m)
{
	if(m > 2)
		m = 0;
	if(CleanMode == 0)
	{
		CleanMode = m;
		Uart0ReUnable;
		uart_Printf("%s %4d\r\n",strM3156, CleanMode);
		Uart0ReEnable;
	}
}


/**************************************** File end *******************************************/
