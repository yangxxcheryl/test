#include "B1404_LIB.h"
#include <iom1280v.h>
#include "Common.h"
#include <macros.h>
#include "eeprom.h"


unsigned char ControlModel;					// 0:������ 1:�����ն˵���
unsigned char WorkProcessStep;				// �������̺�
extern unsigned char CardStoreOpenLook;		// Ƭ�ֹ���״̬
extern unsigned char JumpMode;				// ��������ģʽ


//**************************************
unsigned char CardNoneUseful = 0;			// �ϻ�ģʽ
unsigned char workSwitch = 1;				// ֹͣ��������
extern unsigned char CardStoreTestFlag;		// ȡƬ����ģʽ
//**************************************

unsigned char primeProcessSW;		// ��ע���أ�1:��עϡ��Һ�� 2:��ע��ϴҺ��3:����ģʽ�µ�Һ·�Լ�

void init_devices(void);
void StartWork(void);
void MotorModelConfig(void);
void SetWorkStep(unsigned char n);
void CommandClear(void);

extern unsigned char GetNewTestCard;		// ȡ�µĲ��Կ���0:��, 1:ȡ���뿨, 2:ȡ��1��, 3:ȡ��2��, 4:ȡ��3��, 5:ȡ��4��, 6:ȡ��5���� 255:ȡ�����
extern unsigned char TurnPlateUsedLock;		// ת��ʹ����
extern unsigned char _LEDSTATE;				// ָʾ��״̬���� 0 ��  1 ��
unsigned char EchoSW = 1;

unsigned char (*miantianSubFunction)(void);
unsigned int MaintainSubFunParam;

void ReStart(unsigned char type)
{
	unsigned char i;
	CLI();
	Uart0ReUnable;
	uart_Printf("%s\r\n", strM0199);
	Uart0ReEnable;
	if(type != 0)
	{
		// �������س���ģʽ��ʶ
		i = 1;
		EEPROM_WRITE(EEP_ADD_DOWNLOAD_FLAG, i);
	}
	WDR (); //this prevents a timeout on enabling
	WDTCSR |= (1<<WDCE) | (1<<WDE); //30-Oct-2006 Umesh 
	WDTCSR = 0x0F; // 0x08 WATCHDOG ENABLED - dont forget to issue WDRs
	SEI();
}


/*************************************************************************************************/

/******************************************* ������ **********************************************/
extern unsigned long SecondCount;
unsigned int cmd;
unsigned char uart[] = {'#','3','0','0','2','$','3',0x20,'G','B'};
void main(void)
{
	static unsigned char cmdState;
	static unsigned char taskSate;
	static unsigned char diluteProcessState;	// ϡ��������ִ��״̬
	static unsigned char GetNewPieceProcessState;//ȡƬ����ִ��״̬
	static unsigned char CardScanfSW;			// �Լ�Ƭ������λ�õ���
//	static unsigned char workSwitch=1;			// ֹͣ��������
	static unsigned char dustbinOldState;

	int iPam;
	unsigned char dat, i;
	signed char si;
	unsigned int iTmp0, iTmp1;

	unsigned char workStep = 0;
	_CONST char * pFlash;
	
	miantianSubFunction = 0;	

	DiluteProcDatInit();
	RingQueueDatInit();
	TestQueueDatInit();
	UnloadQueueDatInit();
	InitCommandTransferData();
	InitControlLayerData();
	init_devices();
	
	Uart0ReUnable;
	uart_Printf("%s $%s\r\n", strM0100, strVersion);
	Uart0ReEnable;
	
	ControlModel = 1;
	SetCardTrolleyState(0);
	SetBeepAck();
	SetStateLedBusy();
	WorkProcessStep = 255;
	// ���빤��ѭ��
	while(1)
	{
		// ��������ִ��
		switch(WorkProcessStep)
		{
		/**************** ����  *****************/
			case 255:
				if(ControlCmd.cmdState == 2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 0:		// ���Կ���
							if(ControlCmd.pam[0]==0)
								EchoSW = 0;
							else
								EchoSW = 1;
							break;
						case 1:		// #0001 ��������
							StartWork();
							SetWorkStep(0);		// �����㹤��״̬
							GetStoreState(0);	// ��ʼ��ʱ��ȡƬ����Ϣ
							cmdState = 0;
							break;
						case 3:		// #0003 ��ѯ��������
							Uart0ReUnable;
							uart_Printf("%s $ 255 [Invalid]\r\n",strM0110);
							Uart0ReEnable;
							break;
						case 99:	// ����, ���ÿ��Ź�
							ReStart(ControlCmd.pam[0]);
							break;
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
					}
					if(cmdState)
					{
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					else
					{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					CommandClear();
				}
				break;
			case 0:	
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 0:		// ���Կ���
							if(ControlCmd.pam[0]==0)
								EchoSW = 0;
							else
								EchoSW = 1;
							break;
						case 1:		// #0001 ��������
							StartWork();
							cmdState = 0;
							break;
						case 2:		// #0002 ���ù�������
							if(ControlCmd.pam[0] < 5 || ControlCmd.pam[0] == 9)
							{
								SetWorkStep(ControlCmd.pam[0]);
							}
							else
								cmdState = 1;	// ���ò�������
							break;
						case 3:		// #0003 ��ѯ��������
							Uart0ReUnable;
							uart_Printf("%s $   0 [Free]\r\n",strM0110);
							Uart0ReEnable;
							break;
						case 10:	// #0010	����Ƭ��
							if(CardStoreOpenLook == 0)
								SetStoreDoorOpen(ControlCmd.pam[0]);
							else
							{
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0112);
								Uart0ReEnable;
							}
							break;
						case 11:	// #0011	��ȡƬ����ʪ��
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n", strM0111, GetStoreHumi(), GetStoreTemp());
							Uart0ReEnable;
							break;
						case 99:	// ����, ���ÿ��Ź�
							ReStart(ControlCmd.pam[0]);
							break;
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
						}
					if(cmdState)
					{
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					else
					{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
					}
				break;
		/************** ��е�Լ�  ***************/
			case 1:		
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 1:		// #0001 ��ѯ��ǰ�����汾��
							Uart0ReUnable;
							uart_Printf("%s $%s [MechineCheck]\r\n", strM0100, strVersion);
							Uart0ReEnable;
							break;
						case 3:		// #0003 ��ѯ��������
							Uart0ReUnable;
							uart_Printf("%s $   1 [MechineCheck]\r\n",strM0110);
							Uart0ReEnable;
							break;
						case 5:
							if(ControlCmd.pam[0] == 0)
							{
								workSwitch = 1;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0105);
								Uart0ReEnable;
							}
							else
							{
								workSwitch = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0106);
								Uart0ReEnable;
							}
							break;
						case 99:	// ����, ���ÿ��Ź�
							ReStart(ControlCmd.pam[0]);
							break;
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
						}
					if(cmdState){
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
						}
					else{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
						}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
					}
				if(workSwitch)
				{
					if(MachinePositionInit() == 1)
					{
						SetWorkStep(0);
					}
				}
				break;
		/*************** Һ·�Լ� ***************/
			case 2:		
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 1:		// #0001 ��ѯ��ǰ�����汾��
							Uart0ReUnable;
							uart_Printf("%s $%s [LiquidCheck]\r\n", strM0100, strVersion);
							Uart0ReEnable;
							break;
						case 3:		// #0003 ��ѯ��������
							Uart0ReUnable;
							uart_Printf("%s $   2 [LiquidCheck]\r\n",strM0110);
							Uart0ReEnable;
							break;
						case 5:
							if(ControlCmd.pam[0] == 0)
							{
								workSwitch = 1;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0105);
								Uart0ReEnable;
							}
							else
							{
								workSwitch = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0106);
								Uart0ReEnable;
							}
							break;
						case 99:	// ����, ���ÿ��Ź�
							ReStart(ControlCmd.pam[0]);
							break;
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
						}
					if(cmdState){
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
						}
					else{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
						}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
					}
				if(workSwitch)
				{
					if(DiluteStartCheck(0) == 1)	// ��������1��ʾ�������
					{
						SetWorkStep(0);
					}
				}
				break;
		/************ �������� *************/
			case 3:		
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 1:		// #0001 ��ѯ��ǰ�����汾��
							Uart0ReUnable;
							uart_Printf("%s $%s [Test/Sleep]\r\n", strM0100, strVersion);
							Uart0ReEnable;
							break;
						case 3:		// #0003 ��ѯ��������
							Uart0ReUnable;
							if(JumpMode == 2)
								uart_Printf("%s $   5 [Sleep]\r\n",strM0110);
							else
								uart_Printf("%s $   3 [Test]\r\n",strM0110);
							Uart0ReEnable;
							if(taskSate >= 2)
							{
								Uart0ReUnable;
								uart_Printf("%s $   0 [Free]\r\n",strM3120);
								Uart0ReEnable;
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s $   1 [Busy]\r\n",strM3120);
								//DiluteProcess ����
								printf_DiluteProcess_StepState();
								//CardStoreProcess ����
								printf_CardstoreProcess_StepState();
								//TestQueueProcess ����
								printf_TestProcess_StepState();
								//UnLoadProcess ����
								printf_UnloadProcess_StepState();
								Uart0ReEnable;
							}
							break;
						case 4:	// �����˳�
							if(taskSate >= 2)
							{
								SetDiluentQuit();
								SetStateLedBusy();
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s $   1 [Busy]\r\n",strM3120);
								Uart0ReEnable;
							}
							break;
						case 5:	// ������ͣ
							if(ControlCmd.pam[0] == 0)
							{
								workSwitch = 1;
								clearstopFlag();
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0105);
								Uart0ReEnable;
								SetStateLedFree();
							}
							else
							{
								workSwitch = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0106);
								Uart0ReEnable;
								SetStateLedBusy();
							}
							break;
						case 10:	// ����Ƭ��
							if(CardStoreOpenLook == 0)
								SetStoreDoorOpen(ControlCmd.pam[0]);
							else
							{
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0112);
								Uart0ReEnable;
							}
							break;
						case 99:	// ����, ���ÿ��Ź�
							ReStart(ControlCmd.pam[0]);
							break;
						case 3001:	// #3001 ���ò��Բ���
							if(ControlCmd.pamLen>=4)
							{
								SetDiluentRatio((unsigned char)ControlCmd.pam[0]);
								SetWorkStoreNum((unsigned char)ControlCmd.pam[1]);
								SetReadMolule(ControlCmd.pam[2]);
								SetReadTime0(ControlCmd.pam[3]);
								SetReadTime1(ControlCmd.pam[4]);
							}
							else
								cmdState =  1;
							break;
						case 3002:	// #3002 ����Ƭ�ֺ�
							SetWorkStoreNum(ControlCmd.pam[0]);
							break;
						case 3003:	// #3003 ����ϡ�ͱ���
							SetDiluentRatio(ControlCmd.pam[0]);
							break;
						case 3004:	// #3004 ���õ�һ����ʱ��
							SetReadTime0(ControlCmd.pam[0]);
							break;
						case 3005:	// #3005 ���õڶ�����ʱ��
							SetReadTime1(ControlCmd.pam[0]);
							break;
						case 3006:	// ���ö���ģ��
							SetReadMolule(ControlCmd.pam[0]);
							break;
						case 3007:	// ���õ�Һ��
							SetDropVolume(ControlCmd.pam[0]);
							break;
						case 3008:	// ���õ���ģʽ
							SetDropMode(ControlCmd.pam[0]);
							break;
						case 3009:	// ���ó����ȴ���
							SetReMixNum(ControlCmd.pam[0]);
							break;
						case 3010:	// �����Զ���������
							SetAutoTestCycle(ControlCmd.pam[0]);
							break;
						case 3011:	// ��ȡƬ��״̬��Ϣ
							ReportCardStoreState(ControlCmd.pam[0]);
							break;
						case 3012:	// SetLampLum
							if(ControlCmd.pam[0] == 0)
							{
								iTmp0 = AdjustTestLamp(0, ControlCmd.pam[1]);
								iTmp1 = AdjustTestLamp(1, ControlCmd.pam[2]);
							}
							else
							{
								iTmp0 = AdjustTestLamp(2, ControlCmd.pam[1]);
								iTmp1 = AdjustTestLamp(3, ControlCmd.pam[2]);
							}
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n", strM4114, iTmp0, iTmp1);
							Uart0ReEnable;
							break;
						case 3013:		// TurnOnLamp
							if(ControlCmd.pam[0] == 0)
								TestALampOpen();
							else	
								TestBLampOpen();
							break;
						case 3014:		// TurnOffLamp
							if(ControlCmd.pam[0] == 0)
								TestALampClose();
							else	
								TestBLampClose();
							break;
						case 3015:
							Uart0ReUnable;
							uart_Printf("%s $%4d\r\n",strM3137, GetWorkStoreNum());
							Uart0ReEnable;
							break;
						//case 3016:	// ��������ɨ��
						//	SetCardScanf(ControlCmd.pam[0]);
						//	break;
						case 3017:	// ����ϡ�Ͳ�����4015ָ��һ��
							iTmp0 = CalculateCalStandCoeff(ControlCmd.pam[0],ControlCmd.pam[1]);
							iTmp1 = Save_DiluentCalChart(ControlCmd.pam[0]);
							if(ControlCmd.pam[0] == 0)
								ControlCmd.pam[0] = 9;
							else if(ControlCmd.pam[0] > 13)
								ControlCmd.pam[0] = 13;
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d $%4d\r\n", strM4117, ControlCmd.pam[0],iTmp0,iTmp1);
							Uart0ReEnable;
							break;
						//case 3018:  // �����ظ�����ɨ��
						//	SetReCardScanf(1);
						//	break;
						//case 3019:  // ����ɨ��λ�õ���
						//	if(GetNewPieceProcessState == 1)
						//	{
						//		iPam = SetCardScanfPos((signed char)ControlCmd.pam[0]);
						//		Uart0ReUnable;
						//		uart_Printf("%s $%4d\r\n",strM3124, iPam);
						//		Uart0ReEnable;
						//		CardScanfSW = 1;
						//	}
						//	else
						//	{
						//		Uart0ReUnable;
						//		uart_Printf("%s $1\r\n",strM3120);
						//		Uart0ReEnable;
						//	}
						//	break;
						case 3020:
							SetTestDebugMode(ControlCmd.pam[0]);
							break;
						case 3021:	// ����ȡƬ����ģʽ
							SetGetCardTestMode(ControlCmd.pam[0]);
							break;
						case 3022:	// ����1:1ģʽ��,�Ƿ���Ҫ���г�����
							SetMixtureMode(ControlCmd.pam[0]);
							break;
						case 3023:	// ���ö���ģʽ��,��Ƭ����Ч
							SetWasteCardState(ControlCmd.pam[0]);
							break;
#ifdef Puncture
						case 3024:	// �����Ƿ���Ҫ����
							SetPunctureState(ControlCmd.pam[0]);
							break;
#endif

						case 3030:
							// ����������
							SetSamplingVolume(ControlCmd.pam[0]);
							break;
						case 3050:		// ����
							TestSleep();
							break;
						case 3051:		// �ָ�
							TestStartup();
							break;
					//	case 3052:		// ��������ʱ��
					//		SetSleepTime(ControlCmd.pam[0]);
					//		break;
						case 3053:		// ����ȡ�����ܿ���
							if(ControlCmd.pam[0])
							{
								SamplingSwitch(0);
								SetStateLedBusy();	// ��������,����֮ǰ�Ƿ�Ϊ�죬����Ϊ��
								
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3153);
								Uart0ReEnable;
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3154);
								Uart0ReEnable;
								SamplingSwitch(1);
								if(_LEDSTATE == 1)		// ����ǰ�����̵�
								{
									SetStateLedFree();
								}
							}
							break;
						case 3054:	// ���õ�ǰ�����ز��ʶ
							SetReReadFlag(); 

							break;
						case 3055:	// ������ϴģʽ
							SetCleanMode(ControlCmd.pam[0]);
							break;
						case 3056:	// ���ö�ȡ����Ӧ���ź�
							SetReadCloseAnswer();	
							break;
						case 3060:		// �ֶ���עϡ��Һ
							if(diluteProcessState == 1) 
							{
								if(primeProcessSW == 0)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3160);
									Uart0ReEnable;
									SetStateLedBusy();
									primeProcessSW = 1;
								}
								else if(primeProcessSW == 1)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3171);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 2)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3172);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 3)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3173);
									Uart0ReEnable;
								}
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s $1\r\n",strM3120);
								Uart0ReEnable;
							}
							break;
						case 3061:		// �ֶ���ע��ϴҺ
							if(diluteProcessState == 1) 
							{
								if(primeProcessSW == 0)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3161);
									Uart0ReEnable;
									SetStateLedBusy();
									primeProcessSW = 2;
								}
								else if(primeProcessSW == 1)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3171);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 2)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3172);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 3)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3173);
									Uart0ReEnable;
								}
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s $1\r\n",strM3120);
								Uart0ReEnable;
							}
							break;
						case 3107:		// ���������������趨
							//si = SetDropVolumeFactor((signed char)ControlCmd.pam[0]);
							SetDropVolumeFactor((signed int)ControlCmd.pam[0]);
							//Uart0ReUnable;
							//uart_Printf("%s $%4d\r\n", strM3218, si);
							//Uart0ReEnable;
							break;
						case 3108:
							GetMotorMonitorState(ControlCmd.pam[0],ControlCmd.pam[1]);
							break;
						case 3322:
							if(diluteProcessState == 1) 
							{
								if(primeProcessSW == 0)
								{
									Uart0ReUnable;
									uart_Printf("%s $ 3 [LiquidCheck] $ 3322\r\n",strM0110);
									Uart0ReEnable;
									SetStateLedBusy();
									primeProcessSW = 3;
								}
								else if(primeProcessSW == 1)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3171);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 2)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3172);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 3)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3173);
									Uart0ReEnable;
								}
							}
							else 
							{
								Uart0ReUnable;
								uart_Printf("%s $1\r\n",strM3120);
								Uart0ReEnable;
							}
							break;
						case 3333:
							if(CardStoreTestFlag == 1)		
								break;
							if(ControlCmd.pam[0] == 0)
								CardNoneUseful = 0;
							else
							{	
								CardNoneUseful = 1;
							//	TestALampClose();
							//	TestBLampClose();
							}
							Uart0ReUnable;
							uart_Printf("%s $%4d\r\n", strM3333, CardNoneUseful);
							Uart0ReEnable;
							break;
						//*******************************************************
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
						}
					if(cmdState)
					{
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					else
					{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
				}
				if(workSwitch)
				{
					taskSate = 0;
					if(primeProcessSW == 0)
					{
						diluteProcessState = DiluteProcess(0);
						taskSate += diluteProcessState;
					}
					else
					{
						if(primeProcessSW == 1)
						{
							i = _ManualPrimingDiluent();
							if(i == 1)
							{
								primeProcessSW = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3162);
								Uart0ReEnable;
								if(JumpMode != 2)
									SetStateLedFree();
							}
						}
						else if(primeProcessSW == 2)
						{
							i = _ManualPrimingFluid();
							if(i == 1)
							{
								primeProcessSW = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3163);
								Uart0ReEnable;
								if(JumpMode != 2)
									SetStateLedFree();
							}
						}
						else if(primeProcessSW == 3)
						{
							i = DiluteStartCheck(0);// ��������1��ʾ�������
							if(i == 1)
							{
								primeProcessSW = 0;
								if(JumpMode == 2)	// ���������ģʽ�µ�3322
									JumpMode = 3;
								else
									JumpMode = 0;
								Uart0ReUnable;
								uart_Printf("%s [Test] $ 3322\r\n",strM2120);
								Uart0ReEnable;
								SetStateLedFree();
							}
						}
					}
					if(CardScanfSW == 0)
					{
						GetNewPieceProcessState = GetNewPieceFromStoreProcess(0);
					}
					else
					{
						if(1 == CardScanfPosCheck())
						{
							CardScanfSW = 0;
							Uart0ReUnable;
							uart_Printf("%sDone\r\n",strM3124);
							Uart0ReEnable;
						}
					}
					TestAQueueProcess();
					TestBQueueProcess();
					taskSate += UnloadQueueProcess();
					if(taskSate > 2)
						SetWorkStep(0);
				}
				break;
		/************ ����ά�� **************/
			case 4:		
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					if(miantianSubFunction == 0)
					{
						// ����״̬�½�������
						switch(ControlCmd.cmdIdx)
						{
							case 1:		// #0001 ��ѯ��ǰ�����汾��
								Uart0ReUnable;
								uart_Printf("%s $%s [Maintain]\r\n", strM0100, strVersion);
								Uart0ReEnable;
								break;
							case 3:		// #0003 ��ѯ��������
								Uart0ReUnable;
								uart_Printf("%s $   4 [Maintain]\r\n",strM0110);
								Uart0ReEnable;
								break;
							case 4:	// quit
								SetWorkStep(0);
								break;
							case 5:
								if(ControlCmd.pam[0] == 0)
								{
									workSwitch = 1;
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM0105);
									Uart0ReEnable;
								}
								else
								{
									workSwitch = 0;
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM0106);
									Uart0ReEnable;
								}
								break;
							case 10:	// #0010	����Ƭ��
								if(CardStoreOpenLook == 0)
									SetStoreDoorOpen(ControlCmd.pam[0]);
							else
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0112);
								Uart0ReEnable;
								break;
							case 11:	// #0011	��ȡƬ����ʪ��
								Uart0ReUnable;
								uart_Printf("%s $%4d $%4d\r\n",strM0111, GetStoreHumi(), GetStoreTemp());
								Uart0ReEnable;
								break;
							case 99:	// ����, ���ÿ��Ź�
								ReStart(ControlCmd.pam[0]);	break;
							case 4002:	// ȡ�����ڻ��ȳ��ұ���λ�õ���
								SetNeedleOnMixPosFactor((signed int)ControlCmd.pam[0]);break;
							case 4004:	// �����߶ȵ���
								SetDropHeightFactor((signed int)ControlCmd.pam[0]);		break;
							case 4005:	miantianSubFunction = CardLoadStartAdjust;		break;
							case 4006:	miantianSubFunction = CardLoadEndAdjust;		break;
							case 4007:	miantianSubFunction = CardUnloadStartAdjust;	break;
							case 4008:	miantianSubFunction = CardUnloadEndAdjust;		break;
							case 4009:
								SetLiquidPhotoAdjustNum(ControlCmd.pam[0]);
								miantianSubFunction = LiquidPhotoAdjust;
								break;
							case 4010:
								SetCardStorePhotoAdjustNum(ControlCmd.pam[0]);
								miantianSubFunction = CardStorePhotoAdjust;
								break;
							case 4011:		// SetLampLum
								if(ControlCmd.pam[0] == 0)
								{
									iTmp0 = AdjustTestLamp(0, ControlCmd.pam[1]);
									iTmp1 = AdjustTestLamp(1, ControlCmd.pam[2]);
								}
								else
								{
									iTmp0 = AdjustTestLamp(2, ControlCmd.pam[1]);
									iTmp1 = AdjustTestLamp(3, ControlCmd.pam[2]);
								}
								Uart0ReUnable;
								uart_Printf("%s $%4d $%4d\r\n", strM4114, iTmp0, iTmp1);
								Uart0ReEnable;
								break;
							case 4012:		// GetLampLum
								if(ControlCmd.pam[0] == 0)
								{
									iTmp0 = ReadTestLampPWM(0);
									iTmp1 = ReadTestLampPWM(1);
								}
								else
								{
									iTmp0 = ReadTestLampPWM(2);
									iTmp1 = ReadTestLampPWM(3);
								}
								Uart0ReUnable;
								uart_Printf("%s $%4d $%4d\r\n", strM4114, iTmp0, iTmp1);
								Uart0ReEnable;
								break;
							case 4013:		// TurnOnLamp
								if(ControlCmd.pam[0] == 0)
									TestALampOpen();
								else	
									TestBLampOpen();
								break;
							case 4014:		// TurnOffLamp
								if(ControlCmd.pam[0] == 0)
									TestALampClose();
								else	
									TestBLampClose();
								break;
							case 4015:
								iTmp0 = CalculateCalStandCoeff(ControlCmd.pam[0],ControlCmd.pam[1]);
								iTmp1 = Save_DiluentCalChart(ControlCmd.pam[0]);
								if(ControlCmd.pam[0] == 0)
									ControlCmd.pam[0] = 9;
								else if(ControlCmd.pam[0] > 13)
									ControlCmd.pam[0] = 13;
								Uart0ReUnable;
								uart_Printf("%s $%4d $%4d $%4d\r\n", strM4117, ControlCmd.pam[0],iTmp0,iTmp1);
								Uart0ReEnable;
								break;
							case 4016:
								miantianSubFunction = DiluentQuantifyTest;
								SetDiluentQuantifyVolume(40);
								break;
							case 4017:	// �����ȸ߶ȵ���
								si = SetMixHeight((signed char)ControlCmd.pam[0]);
								Uart0ReUnable;
								uart_Printf("%s $%4d\r\n",strM4118, si);
								Uart0ReEnable;
								break;
							// 
							case 4020:	miantianSubFunction = TurnPlateCheck;		break;
							case 4021:	miantianSubFunction = NeedleTurnCheck;		break;
							case 4022:	miantianSubFunction = NeedleUpdownCheck;	break;
							case 4023:	miantianSubFunction = CardStoreMoveCheck;	break;
							case 4024:	miantianSubFunction = CardTakeHookCheck;	break;
							case 4025:	miantianSubFunction = CardLoadCheck;		break;
							case 4026:	miantianSubFunction = CardUnloadCheck;		break;
							case 4029:	miantianSubFunction = DiluentPumpCheck;		break;
							case 4030:	miantianSubFunction = LeanerPumpCheck;		break;
							case 4031:	miantianSubFunction = EffluentPumpCheck;	break;
							case 4032:	miantianSubFunction = SamplingSyringCheck;	break;
							case 4033:
							//	SetLiquidPhotoAdjustNum(HTCmdBuf.pam[0]);
								miantianSubFunction = LiquidPhotoCheck;
								break;
							case 4034:
								SetCardStorePhotoAdjustNum(ControlCmd.pam[0]);
								miantianSubFunction = CardStorePhotoCheck;
								break;
							case 4035:	miantianSubFunction = NeedleOnMixSideCheck;	break;
							case 4036:	miantianSubFunction = DropHeightCheck;	break;
							case 4037:	miantianSubFunction = MixHeightCheck;	break;
							case 4099:	miantianSubFunction = GetStoreProcess;break;
							
							case 4050:
								miantianSubFunction = DiluentQuantifyTest;
								SetDiluentQuantifyVolume(ControlCmd.pam[0]);
								break;
							case 4051:
								miantianSubFunction = LeanerQuantifyTest;
								SetLeanerQuantifyVolume(ControlCmd.pam[0]);
								break;
							case 4052:
								miantianSubFunction = SampQuantifyTest;
								SetSampQuantifyVolume(ControlCmd.pam[0]);
								break;
							case 4055:	// �ϴ�������״̬��Ϣ
								UpLoadingModuleSensorState((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1]);
								break;
							case 4056:	// �ϴ����д�����״̬��Ϣ
								UpLoadingAllSensorState();
								break;
							// ��������
							case 4060:		// Ԥ����λ������
								cmdState =  MotAdjustPosition((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1]);
								break;
							case 4061:		// ��ʼλ���
								cmdState =  MotInitCheck((unsigned char)ControlCmd.pam[0]);
								break;
							case 4062:		// �������
								cmdState =  MotRun((unsigned char)ControlCmd.pam[0], ControlCmd.pam[1]);
								break;
							case 4063:		// ������е�
								cmdState =  MotRunTo((unsigned char)ControlCmd.pam[0], ControlCmd.pam[1]);
								break;
							case 4064:		// �������ָ��λ��
								cmdState =  MotRunToSite((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1]);
								break;
							case 4065:		// ��������ʱ����
								cmdState =  SetMotRunPam((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1], (unsigned char)ControlCmd.pam[2],(unsigned char)ControlCmd.pam[3]);
								break;
							case 4066:		// ���û�������
								cmdState =  MotSetPam((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1], (unsigned char)ControlCmd.pam[2]);
								break;
							case 4067:	// ���ôӻ���ַ
								cmdState =  SlaveSetAddress((unsigned char)ControlCmd.pam[0]);
								break;
							/****************************** Һ·���� ******************************/	
							case 4068:	// ���õ�ŷ�
								cmdState =  SetEValve((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1]);
								break;
							case 4069:	// Get photo info
								cmdState =  GetLiquidPhotoInfo();
								break;
							case 4070:	// Liquid photo adjust
								cmdState =  SetLiquidPhotoAdjust((unsigned char)ControlCmd.pam[0]);
								break;
								/**************************** Ƭ�ֿ��� *********************************/	
							case 4071:	// ��ȡƬ��״̬��Ϣ
								cmdState =  GetStoreState((unsigned char)ControlCmd.pam[0]);
								break;
							case 4072:	// ����Ƭ��У׼
								cmdState =  SetStoreCAL((unsigned char)ControlCmd.pam[0]);
								break;
							case 4073:	// ��ȡƬ�ֹ����չ��ź�ֵ
								cmdState =  GetStorePhoVol((unsigned char)ControlCmd.pam[0]);
								break;
							case 4074:	// ��ȡҺ��̽���ź�ֵ
								iPam = getLiqDetADC(NeedleChannel);
								Uart0ReUnable;
								uart_Printf("%s $%4d\r\n",strM4174, iPam);
								Uart0ReEnable;
								break;
							case 4075:
								iPam = getLiqDetADC(LoadChannel);
								Uart0ReUnable;
								uart_Printf("%s $%4d\r\n",strM4175, iPam);
								Uart0ReEnable;
								break;
							case 4076:
								iPam = getLiqDetADC(UnloadChannel);
								Uart0ReUnable;
								uart_Printf("%s $%4d\r\n",strM4176, iPam);
								Uart0ReEnable;
								break;
							case 4077:
								if(ControlCmd.pam[0] > 12)
									ControlCmd.pam[0] = 12;
								else if(ControlCmd.pam[0] < 1)
									ControlCmd.pam[0] = 1;
								MotStop(ControlCmd.pam[0]);
								break;
							default:
								SetBeepWarning();
								Uart0ReUnable;
								uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
								Uart0ReEnable;
								break;	
							}
						}
					else	// ����״̬�½�������
					{
						switch(ControlCmd.cmdIdx)
						{
							case 3:		// #0003 ��ѯ��������
								Uart0ReUnable;
								uart_Printf("%s $   4 [Maintain]\r\n",strM0110);
								Uart0ReEnable;
								break;
							case 4:	// quit
								SetMaintianSubfunctionQuitFlag();
								break;
							case 5:
								if(ControlCmd.pam[0] == 0)
								{
									workSwitch = 1;
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM0105);
									Uart0ReEnable;
								}
								else
								{
									workSwitch = 0;
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM0106);
									Uart0ReEnable;
								}
								break;
							case 99:	// ����, ���ÿ��Ź�
								ReStart(ControlCmd.pam[0]);
								break;
							default:
								SetBeepWarning();
								Uart0ReUnable;
								uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
								Uart0ReEnable;
								break;	
						}
					}
					if(cmdState)
					{
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					else
					{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
				}
				if(workSwitch)
				{
					if(miantianSubFunction)	// �����ӹ��ܺ�����ֱ����������1��ֹ
					{
						if(ControlCmd.pamLen != 0)
						{
							MaintainSubFunParam = ControlCmd.pam[0];
							ControlCmd.pam[0] = 0;
						}
						else
							MaintainSubFunParam = 0;
						i = miantianSubFunction();
						if(i==1)
						{
							miantianSubFunction = 0;
						}
					}
				}
				break;
			default:
				break;
		}
		
		// ��Ƭ�ֹ��ܿ���
		if(GetwasteCardState() == 0)
		{
			// ��Ƭ�ֿ��ؼ��
			if((PINK & 0x02) == 0)
			{
				if(dustbinOldState > 0)
					dustbinOldState --;
				if(dustbinOldState == 2)
				{
					Uart0ReUnable;
					uart_Printf("%s $%4d\r\n",strM0200 , dustbinOldState);
					Uart0ReEnable;
				}
			}
			else		// ��Ƭ�ֹر�
			{
				if(dustbinOldState < 255)
					dustbinOldState ++;
				if(dustbinOldState == 253)
				{
					Uart0ReUnable;
					uart_Printf("%s $%4d\r\n",strM0200 , dustbinOldState);
					Uart0ReEnable;
				}
			}
		}
	}
}

void SetWorkStep(unsigned char n)
{
	if(n > 4 && n != 9)
		n = 0;
	if(n != WorkProcessStep)
	{
		WorkProcessStep = n;
		switch(WorkProcessStep)
		{
			case 0:	Uart0ReUnable;uart_Printf("%s $   0 [Free]\r\n",strM0110);Uart0ReEnable;
				break;
			case 1:	Uart0ReUnable;uart_Printf("%s $   1 [MechineCheck]\r\n",strM0110);Uart0ReEnable;
				break;
			case 2:	Uart0ReUnable;uart_Printf("%s $   2 [LiquidCheck]\r\n",strM0110);Uart0ReEnable;
				break;
			case 3:	GetStoreState(0);	Uart0ReUnable;uart_Printf("%s $   3 [Test]\r\n",strM0110);Uart0ReEnable;
				break;
			case 4:	Uart0ReUnable;uart_Printf("%s $   4 [Maintain]\r\n",strM0110);Uart0ReEnable;
				break;
			case 9:	JumpMode = 1;GetStoreState(0);	WorkProcessStep = 3; Uart0ReUnable;uart_Printf("%s $   3 [Test]\r\n",strM0110);Uart0ReEnable;
				break;
		}
	}
}


/*************************************************************************************************/

/*************************************************************************************************/
void StartWork(void)
{
	CLI();

	// ��ʼ��Ӧ�ò�����
	DiluteProcDatInit();
	RingQueueDatInit();
	TestQueueDatInit();
	UnloadQueueDatInit();
	
	InitControlLayerData();
	InitCommandTransferData();

//	RegisterSlave(COMMON_ADDRESS);
	RegisterSlave(MOT_TURN_PLATE);
	RegisterSlave(MOT_SAMP_TRUN);
	RegisterSlave(MOT_SAMP_NEEDLE);
	RegisterSlave(MOT_CARD_LOAD);
	RegisterSlave(MOT_CARD_UNLOAD);
	RegisterSlave(MOT_STORE_CARD_MOVE);
	RegisterSlave(MOT_DILUENT);
	RegisterSlave(MOT_FLUID);
	RegisterSlave(MOT_EFFLUENT);
	RegisterSlave(MOT_SAMP_PUMP);
	RegisterSlave(LIQUID_CONTROL);
	RegisterSlave(STORE_MONITOR);

	StartCommandTransfer();
	SEI();

	//MotorModelConfig();
	Uart0ReUnable;
	uart_Printf("%s $%s\r\n", strM0101, strVersion);
	uart_Printf("%s\r\n", strM0103);
	uart_Printf("%s\r\n", strM0104);
	Uart0ReEnable;
	
}

void MotorModelConfig(void)
{
	// ���û�������
	/*	����0��bit[7]ֹͣ������
		bit[6:5]���з�ʽ,[00]ֱ�߸�ʽ,[01]Բ��ѭ��ʽ
		Bit[4:3]��λ���������ͣ�[00]�޴�������[01]���͹��[10]΢�����أ�
		Bit[2:1]��λ���������ͣ�[00]�޴�������[01]���͹��[10]΢�����أ�
		Bit[0]����ת���ã�[0]��ת��[1]��ת��
		����1��Բ�ܵȷ�����
	*/
	SetMotRunPam(MOT_TURN_PLATE,120,10,CURRENT_TURN_PLATE);
	SetMotRunPam(MOT_SAMP_TRUN,50,40,CURRENT_SAMP_TRUN);
	SetMotRunPam(MOT_SAMP_NEEDLE,200,10,CURRENT_SAMP_NEEDLE);
	SetMotRunPam(MOT_CARD_LOAD,160,20,CURRENT_CARD_LOAD);
	SetMotRunPam(MOT_CARD_UNLOAD,160,20,CURRENT_CARD_UNLOAD);
	SetMotRunPam(MOT_STORE_CARD_MOVE,32,10,CURRENT_STORE_MOVE);
	SetMotRunPam(MOT_DILUENT,200,20,CURRENT_DILUENT);
	SetMotRunPam(MOT_FLUID,128,60,CURRENT_FLUID);
	SetMotRunPam(MOT_EFFLUENT,128,60,CURRENT_EFFLUENT);
	SetMotRunPam(MOT_SAMP_PUMP,64,60,CURRENT_SAMP_PUMP);

}


/********************************************* File end **********************************************/
