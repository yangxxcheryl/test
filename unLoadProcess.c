
#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"
#include "eeprom.h"


extern RING_QUEUE	RingQueue;
extern unsigned char insertflag[30];
UNLOAD_QUEUE UnloadQueue;
extern unsigned long SecondCount;	// 秒时钟计数
extern unsigned char TurnPlateUsedLock;		// 转盘使用锁

// 废片盒处理
unsigned long TranCanCountBase;
unsigned char TrashCanState;
extern unsigned char WorkProcessStep;
extern SAMP_INFO NewTestInfo;
extern unsigned char stopTestFlag;			// 停止测试,如果废片仓连续开启时间超过20分钟
unsigned char wasteCardNoneUseful = 0;		// 废片仓功能开启 0   关闭 1

// 设置是否使用废片仓  0 使用  1 不使用
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

// 卸片任务入队
void UnloadQueueAdd(unsigned char ringNum, signed long time)
{
	// 根据时间先后顺序将待卸载干片位置号放入卸片队列中等待卸片
	unsigned char n, i;

	// 查找出插入位置
	for(n=0; n<RING_QUEUE_NUM; n++)
	{
		if(UnloadQueue.ringNum[n] == 0xff)		// 已经到队尾了，直接插入到队尾
			break;
		else
		{
			if(UnloadQueue.unloadTime[n] > time)
			{
				break;							// 查找到当前的元素的卸载时间比插入的更晚，插入当前位置
			}
		}
	}
	// n为查找到的插入位置
	i = RING_QUEUE_NUM - 1;
	while(i--)
	{
		if(i != n)
		{
			// 插入位置后面的元素往后移动一步
			UnloadQueue.unloadTime[i] = UnloadQueue.unloadTime[i-1];
			UnloadQueue.ringNum[i] = UnloadQueue.ringNum[i-1];
		}
		else
		{	
			// 插入新的任务
			UnloadQueue.unloadTime[i] = time;
			UnloadQueue.ringNum[i] = ringNum;
			break;
		}
	}
}

void UnloadQueueForward(){
	// 测试队列前进一步，当队头测试完毕后，队头元素删除队列前进一步
	unsigned char i;
	for(i=0; i<RING_QUEUE_NUM; i++)
	{
		if(i != (RING_QUEUE_NUM-1))
		{
			// 每个元素往前移动一个，队头元素被后面覆盖
			UnloadQueue.unloadTime[i] = UnloadQueue.unloadTime[i+1];
			UnloadQueue.ringNum[i] = UnloadQueue.ringNum[i+1];
		}
		else
		{
			// 队尾填充零
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
	// 监测卸片队列上的卸片时间，将到时间的干片卸载
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
		{	// 等待转盘电机
			if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	return 0;
			watiMotTurnplate = 0;
		}
		if(waitMotUnload)	// 等待干片卸载电机
		{
			if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;
			waitMotUnload = 0;
		}
	}
	switch(mainStep)
	{
		case 0:		// 监测队头的时间
			if(UnloadQueue.ringNum[0] != 0xff)
			{
				if(UnloadQueue.unloadTime[0] < SecondCount)
				{
					ringNum = UnloadQueue.ringNum[0];
					
					// 废片仓功能开启
					if(GetwasteCardState() == 0)
					{
						// 卸片时间到
						if(TrashCanCheck())
						{
							// 监测废片盒
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
							TurnPlateUsedLock = 1;		// 占用转盘标识
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
		case 1:		// 转盘转到当前位置，将干片卸载
			switch(workStep)
			{
				case 0:		// 转盘转至0位
					SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);
					MotRunToSite(MOT_TURN_PLATE,0);		// 转盘运行到零位
					workStep = 10;
					watiMotTurnplate = 1;
					break;	
				case 10:		// 转盘转到当前位置
					ringNum = UnloadQueue.ringNum[0];
					ucTmp = ringNum + 25;
					if(ucTmp>=RING_QUEUE_NUM)
						ucTmp -= RING_QUEUE_NUM;
					//SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);
					MotRunToSite(MOT_TURN_PLATE,ucTmp);		// 转盘转到当前位置
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
					if(i < 500)	// 当前位置没有试剂片
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
							workStep = 0;	// 重新转至该位置检测
						}
					}
					else if(i > 900)	// 有试剂片
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
					if(i < CardLocationAD)	// 当前位置没有试剂片
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
							workStep = 0;	// 重新转至该位置检测
						}
					}
					else 					// 有试剂片
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
						{		// 废片仓打开
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
					MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);		// 卸片行程67mm/0.08128 = 824
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
					mainStep = 2; // 不检测下片
					workStep = 0; // 不检测下片
#else
					workStep = 6; // 检测下片
#endif
					break;
				case 6:	// 下片动作完成之后,重新检测是否有试剂片
					i = getLiqDetADC(UnloadChannel);
					/*
					if(i < 500)	// 当前位置没有试剂片
					{
						mainStep = 2;
						workStep = 0;
					}
					else if(i > 900)	// 有试剂片
					{
						workStep = 0;
						uart_Printf("%s $%4d $%4d\r\n",strM3117,ringNum,j + 2);		// 对应位置再次下片
						j++;
						if(j > 2)
						{
							j = 0;
							mainStep = 2;
							workStep = 0;
							uart_Printf("%s $%4d\r\n",strE4909,ringNum);		// 对应位置连续3次下片失败
						}
					}
					else
					{
						uart_Printf("!3945 UnloadAdcOutofScope $ %d\r\n",(unsigned int)i);
						mainStep = 2;
						workStep = 0;
					}
					*/
					if(i < CardLocationAD)	// 当前位置没有试剂片
					{
						mainStep = 2;
						workStep = 0;
					}
					else 					// 有试剂片
					{
						workStep = 0;
						Uart0ReUnable;
						uart_Printf("%s $%4d $%4d\r\n",strM3117,ringNum,j + 2);		// 对应位置再次下片
						Uart0ReEnable;
						j++;
						if(j > 2)
						{
							j = 0;
							mainStep = 2;
							workStep = 0;
							Uart0ReUnable;
							uart_Printf("%s $%4d\r\n",strE4909,ringNum);		// 对应位置连续3次下片失败
							Uart0ReEnable;
						}
					}
					break;
			}
			break;
		case 2:		// 干片卸载，释放转盘使用权
			TurnPlateUsedLock = 0;
			TurnPlateUsedLock = 0;					
			ringNum = UnloadQueue.ringNum[0];
			insertflag[ringNum] = 0;
			l = RingQueue.sampInfo[ringNum].testSerial;
			RingQueueDelete(ringNum);			// 删除转盘上对应位置的干片信息，释放出位置继续使用
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
			// 废片盒容量监测处理
			mainStep = 0;
			workStep = 0;
			inWork = 0;
			UnloadQueueForward();		// 删除已完成的队头任务，队列前进一步
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
		if((PINK & 0x02) == 0)		// 废片仓打开
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
				// 片仓已打开并排空
				TranCanCountBase = ReadCurTestSetial();		// 读取当前的测试自动序列号
				EEPROM_WRITE(EEP_ADD_TRANSH_CNT, TranCanCountBase);		// 写入新的废片基数基数
			}
			if(delayCnt == 60)	// 废片仓打开时间超时报警
			{
				SetBeepWarning();
				delayCnt = 40;
			}
			if(openTime >= 1200)	// 片仓开启时间超过20分钟
			{
				openTime = 0;
				stopTestFlag = 1;
			}
		}
		else		// 废片仓关闭
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
				if(delayCnt2 > 20)	// 废片仓满报警
					SetBeepWarning();
				delayCnt2 = 0;
			}
		}
	}
}
unsigned char TrashCanCheck(void)
{
	// 关闭废片仓满监控
/*	if((NewTestInfo.testSerial - TranCanCountBase) > 100){
		TrashCanState = 1;
		return 1;
		}
		*/
	TrashCanState = 0;
	return 0;
}

