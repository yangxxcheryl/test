
#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"
#include "eeprom.h"


extern RING_QUEUE	RingQueue;
extern unsigned char insertflag[30];
UNLOAD_QUEUE UnloadQueue;
extern unsigned long SecondCount;	// ��ʱ�Ӽ���
extern unsigned char TurnPlateUsedLock;		// ת��ʹ����

// ��Ƭ�д���
unsigned long TranCanCountBase;
unsigned char TrashCanState;
extern unsigned char WorkProcessStep;
extern SAMP_INFO NewTestInfo;
extern unsigned char stopTestFlag;			// ֹͣ����,�����Ƭ����������ʱ�䳬��20����
unsigned char wasteCardNoneUseful = 0;		// ��Ƭ�ֹ��ܿ��� 0   �ر� 1

// �����Ƿ�ʹ�÷�Ƭ��  0 ʹ��  1 ��ʹ��
void SetWasteCardState(unsigned char m)
{
	if(m > 1)
		m = 1;
	wasteCardNoneUseful = m;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3118, m);
	Uart0ReEnable;
}

unsigned char GetwasteCardState(void)
{
	return wasteCardNoneUseful;
}

void UnloadQueueDatInit(void)
{
	unsigned int i;
	unsigned char *pChar;

	pChar = (unsigned char *)&(UnloadQueue);
	for(i = 0; i < sizeof(UNLOAD_QUEUE); i++)
	{
		*pChar++ = 0;
	}
	for(i = 0; i < TEST_QUEUE_NUM; i++)
	{
		UnloadQueue.ringNum[i] = 0xff;
	}
	TranCanCountBase = ReadCurTestSetial();
}

// жƬ�������
void UnloadQueueAdd(unsigned char ringNum, signed long time)
{
	// ����ʱ���Ⱥ�˳�򽫴�ж�ظ�Ƭλ�úŷ���жƬ�����еȴ�жƬ
	unsigned char n, i;

	// ���ҳ�����λ��
	for(n=0; n<RING_QUEUE_NUM; n++)
	{
		if(UnloadQueue.ringNum[n] == 0xff)		// �Ѿ�����β�ˣ�ֱ�Ӳ��뵽��β
			break;
		else
		{
			if(UnloadQueue.unloadTime[n] > time)
			{
				break;							// ���ҵ���ǰ��Ԫ�ص�ж��ʱ��Ȳ���ĸ������뵱ǰλ��
			}
		}
	}
	// nΪ���ҵ��Ĳ���λ��
	i = RING_QUEUE_NUM - 1;
	while(i--)
	{
		if(i != n)
		{
			// ����λ�ú����Ԫ�������ƶ�һ��
			UnloadQueue.unloadTime[i] = UnloadQueue.unloadTime[i-1];
			UnloadQueue.ringNum[i] = UnloadQueue.ringNum[i-1];
		}
		else
		{	
			// �����µ�����
			UnloadQueue.unloadTime[i] = time;
			UnloadQueue.ringNum[i] = ringNum;
			break;
		}
	}
}

void UnloadQueueForward(){
	// ���Զ���ǰ��һ��������ͷ������Ϻ󣬶�ͷԪ��ɾ������ǰ��һ��
	unsigned char i;
	for(i=0; i<RING_QUEUE_NUM; i++)
	{
		if(i != (RING_QUEUE_NUM-1))
		{
			// ÿ��Ԫ����ǰ�ƶ�һ������ͷԪ�ر����渲��
			UnloadQueue.unloadTime[i] = UnloadQueue.unloadTime[i+1];
			UnloadQueue.ringNum[i] = UnloadQueue.ringNum[i+1];
		}
		else
		{
			// ��β�����
			UnloadQueue.unloadTime[i] = 0;
			UnloadQueue.ringNum[i] = 0xff;
		}
	}
}

static unsigned char UnloadProcess_workStep,UnloadProcess_mainStep;

void printf_UnloadProcess_StepState(void)
{
	uart_Printf("*3202 UnloadStepState $%2d $%2d\r\n",UnloadProcess_mainStep,UnloadProcess_workStep);
}

unsigned char UnloadQueueProcess(void){
	// ���жƬ�����ϵ�жƬʱ�䣬����ʱ��ĸ�Ƭж��
	static unsigned char mainStep;		
	static unsigned char workStep;
	static unsigned char inWork;
	static unsigned char ringNum;
	static unsigned char n,m,j,k;
	static unsigned char watiMotTurnplate, waitMotUnload;
	unsigned char ucTmp;
	unsigned int i;
	unsigned long l;
	UnloadProcess_mainStep = mainStep;
	UnloadProcess_workStep = workStep;
	if(inWork)
	{
		if(WaitDelayTime(MOT_CARD_UNLOAD))		return 0;
		if(watiMotTurnplate)
		{	// �ȴ�ת�̵��
			if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	return 0;
			watiMotTurnplate = 0;
		}
		if(waitMotUnload)	// �ȴ���Ƭж�ص��
		{
			if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;
			waitMotUnload = 0;
		}
	}
	switch(mainStep)
	{
		case 0:		// ����ͷ��ʱ��
			if(UnloadQueue.ringNum[0] != 0xff)
			{
				if(UnloadQueue.unloadTime[0] < SecondCount)
				{
					ringNum = UnloadQueue.ringNum[0];
					
					// ��Ƭ�ֹ��ܿ���
					if(GetwasteCardState() == 0)
					{
						// жƬʱ�䵽
						if(TrashCanCheck())
						{
							// ����Ƭ��
							return 0;
						}
						if((PINK & 0x02) == 0)
						{
							return 0;
						}
					}
					if(TurnPlateUsedLock == 0)
					{
						if(200 == insertflag[ringNum])
						{
							TurnPlateUsedLock = 1;		// ռ��ת�̱�ʶ
							mainStep = 1;
							workStep = 0;
							inWork = 1; 
						}
					}
				}
			}
			else
			{
				return 1;
			}
			break;
		case 1:		// ת��ת����ǰλ�ã�����Ƭж��
			switch(workStep)
			{
				case 0:		// ת��ת��0λ
					SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);
					MotRunToSite(MOT_TURN_PLATE,0);		// ת�����е���λ
					workStep = 10;
					watiMotTurnplate = 1;
					break;	
				case 10:		// ת��ת����ǰλ��
					ringNum = UnloadQueue.ringNum[0];
					ucTmp = ringNum + 25;
					if(ucTmp>=RING_QUEUE_NUM)
						ucTmp -= RING_QUEUE_NUM;
					//SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);
					MotRunToSite(MOT_TURN_PLATE,ucTmp);		// ת��ת����ǰλ��
					watiMotTurnplate = 1;
					workStep = 1;
					break;
				//////////////////////////////////////////////////////
				case 1:
					SetDelayTime(MOT_CARD_UNLOAD,5);
#ifndef LoadCheck
					workStep = 3;
#else					
					workStep = 2;
#endif
					break;
				case 2:
					i = getLiqDetADC(UnloadChannel);
					/*
					if(i < 500)	// ��ǰλ��û���Լ�Ƭ
					{
						//mainStep = 2;
						//workStep = 0;
						//uart_Printf("%s $ %d\r\n",strE3933,i);
						j++;
						if(j > 1)
						{
							j = 0;
							mainStep = 2;
							workStep = 0;
							uart_Printf("%s $ %d\r\n",strE3933,i);
						}
						else
						{
							workStep = 0;	// ����ת����λ�ü��
						}
					}
					else if(i > 900)	// ���Լ�Ƭ
					{
						workStep = 3;
						uart_Printf("%s $ %d\r\n",strM3194,i);
					}
					else
					{
						uart_Printf("!3945 UnloadAdcOutofScope $ %d\r\n",(unsigned int)i);
						workStep = 3;
					}
					*/
					if(i < CardLocationAD)	// ��ǰλ��û���Լ�Ƭ
					{
						j++;
						if(j > 1)
						{
							j = 0;
							mainStep = 2;
							workStep = 0;
							Uart0ReUnable;
							uart_Printf("%s $ %d\r\n",strE3933,i);
							Uart0ReEnable;
						}
						else
						{
							workStep = 0;	// ����ת����λ�ü��
						}
					}
					else 					// ���Լ�Ƭ
					{
						workStep = 3;
						Uart0ReUnable;
						uart_Printf("%s $ %d\r\n",strM3194,i);
						Uart0ReEnable;
					}
					break;
				case 3:
					if(GetwasteCardState() == 0)
					{
						if((PINK & 0x02) == 0)
						{		// ��Ƭ�ִ�
							TurnPlateUsedLock = 0;
							TurnPlateUsedLock = 0;
							insertflag[ringNum] = 200;
							mainStep = 0;
							workStep = 0;
							inWork = 0;
							break;
						}
					}
					SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
					MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);		// жƬ�г�67mm/0.08128 = 824
					waitMotUnload = 1;
					workStep = 4;
					break;
				case 4:
					SetMotRunPam(MOT_CARD_UNLOAD,100,20,2);
					MotRunTo(MOT_CARD_UNLOAD,0);
					waitMotUnload = 1;
					workStep = 5;
					break;
				case 5:
					//SetDelayTime(MOT_CARD_UNLOAD,5);
					SetDelayTime(MOT_CARD_UNLOAD,10);
#ifndef LoadCheck
					mainStep = 2; // �������Ƭ
					workStep = 0; // �������Ƭ
#else
					workStep = 6; // �����Ƭ
#endif
					break;
				case 6:	// ��Ƭ�������֮��,���¼���Ƿ����Լ�Ƭ
					i = getLiqDetADC(UnloadChannel);
					/*
					if(i < 500)	// ��ǰλ��û���Լ�Ƭ
					{
						mainStep = 2;
						workStep = 0;
					}
					else if(i > 900)	// ���Լ�Ƭ
					{
						workStep = 0;
						uart_Printf("%s $%4d $%4d\r\n",strM3117,ringNum,j + 2);		// ��Ӧλ���ٴ���Ƭ
						j++;
						if(j > 2)
						{
							j = 0;
							mainStep = 2;
							workStep = 0;
							uart_Printf("%s $%4d\r\n",strE4909,ringNum);		// ��Ӧλ������3����Ƭʧ��
						}
					}
					else
					{
						uart_Printf("!3945 UnloadAdcOutofScope $ %d\r\n",(unsigned int)i);
						mainStep = 2;
						workStep = 0;
					}
					*/
					if(i < CardLocationAD)	// ��ǰλ��û���Լ�Ƭ
					{
						mainStep = 2;
						workStep = 0;
					}
					else 					// ���Լ�Ƭ
					{
						workStep = 0;
						Uart0ReUnable;
						uart_Printf("%s $%4d $%4d\r\n",strM3117,ringNum,j + 2);		// ��Ӧλ���ٴ���Ƭ
						Uart0ReEnable;
						j++;
						if(j > 2)
						{
							j = 0;
							mainStep = 2;
							workStep = 0;
							Uart0ReUnable;
							uart_Printf("%s $%4d\r\n",strE4909,ringNum);		// ��Ӧλ������3����Ƭʧ��
							Uart0ReEnable;
						}
					}
					break;
			}
			break;
		case 2:		// ��Ƭж�أ��ͷ�ת��ʹ��Ȩ
			TurnPlateUsedLock = 0;
			TurnPlateUsedLock = 0;					
			ringNum = UnloadQueue.ringNum[0];
			insertflag[ringNum] = 0;
			l = RingQueue.sampInfo[ringNum].testSerial;
			RingQueueDelete(ringNum);			// ɾ��ת���϶�Ӧλ�õĸ�Ƭ��Ϣ���ͷų�λ�ü���ʹ��
#ifndef UartSendLong
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3116 , l);
			uart_Printf(" $%4d",(unsigned char)ringNum);
			uart_Printf(" $%8d\r\n",(unsigned long)SecondCount);
			Uart0ReEnable;
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3116);
			uart0SendInt(l);
			uart_Printf(" $%4d $ ",(unsigned char)ringNum);
			uart0SendInt(SecondCount);
			uart_Printf("\r\n");
			Uart0ReEnable;
#endif
			mainStep = 3;
			break;
		case 3:
			// ��Ƭ��������⴦��
			mainStep = 0;
			workStep = 0;
			inWork = 0;
			UnloadQueueForward();		// ɾ������ɵĶ�ͷ���񣬶���ǰ��һ��
			break;
		}
	return 0;
}



void TrashCanMonitor(void){
	static unsigned char delayCnt, delayCnt2;
	static unsigned char dustbinOldState;
	static unsigned int openTime;
	
	if(WorkProcessStep == 3)
	{
		if((PINK & 0x02) == 0)		// ��Ƭ�ִ�
		{
			delayCnt ++;
			if(0 == stopTestFlag)
				openTime++;
			if(delayCnt == 1)
			{
				if(dustbinOldState != 0)
				{
					dustbinOldState = 0;
				}
			}
			if(delayCnt == 5)
			{
				// Ƭ���Ѵ򿪲��ſ�
				TranCanCountBase = ReadCurTestSetial();		// ��ȡ��ǰ�Ĳ����Զ����к�
				EEPROM_WRITE(EEP_ADD_TRANSH_CNT, TranCanCountBase);		// д���µķ�Ƭ��������
			}
			if(delayCnt == 60)	// ��Ƭ�ִ�ʱ�䳬ʱ����
			{
				SetBeepWarning();
				delayCnt = 40;
			}
			if(openTime >= 1200)	// Ƭ�ֿ���ʱ�䳬��20����
			{
				openTime = 0;
				stopTestFlag = 1;
			}
		}
		else		// ��Ƭ�ֹر�
		{
			if(dustbinOldState == 0)
			{
				dustbinOldState = 1;
			}
			delayCnt = 0;
			openTime = 0;
			stopTestFlag = 0;
			if(TrashCanState)
			{
				delayCnt2 ++;
				if(delayCnt2 > 20)	// ��Ƭ��������
					SetBeepWarning();
				delayCnt2 = 0;
			}
		}
	}
}
unsigned char TrashCanCheck(void)
{
	// �رշ�Ƭ�������
/*	if((NewTestInfo.testSerial - TranCanCountBase) > 100){
		TrashCanState = 1;
		return 1;
		}
		*/
	TrashCanState = 0;
	return 0;
}

