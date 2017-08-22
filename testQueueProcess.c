

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"
#include "eeprom.h"

unsigned char insertflag[30];	// 插入转盘标志  255 插入  200 下片 0 空闲

TEST_QUEUE TestQueueA;
TEST_QUEUE TestQueueB;
extern RING_QUEUE	RingQueue;
extern unsigned long SecondCount;	// 秒时钟计数
extern unsigned char TurnPlateUsedLock;		// 转盘使用锁
static unsigned char LampAState;
static unsigned char LampBState;
static unsigned char ReadColseAnswer;
extern unsigned char CardNoneUseful;

void TestALampOpen(void){
	DDRH |= 0x70;	
	PORTH &= 0xcf;
	PORTH |= 0x40;
	LampAState = 1;
}
void TestALampClose(void){
	DDRH &= 0x8f;	
	PORTH &= 0x8f;
	LampAState = 0;
}
void TestBLampOpen(void){
	DDRL |= 0x70;	
	PORTL &= 0xcf;
	PORTL |= 0x40;
	LampBState = 1;
}
void TestBLampClose(void){
	DDRL &= 0x8f;
	PORTL &= 0x8f;
	LampBState = 0;
}

void ReSetTestLampPWM(unsigned char n){
	// 读取光源设置值并设置
	unsigned int lampPWM;
	if(n<4){
		EEPROM_READ(EEP_ADD_LAMPSET+n*2,   lampPWM);
	//	lampPWM = lampPWM + 512;
		switch(n){
			case 0:		OCR4BH = lampPWM>>8;	OCR4BL = (unsigned char)lampPWM;	break;
			case 1:		OCR4CH = lampPWM>>8;	OCR4CL = (unsigned char)lampPWM;	break;
			case 2:		OCR5BH = lampPWM>>8;	OCR5BL = (unsigned char)lampPWM;	break;
			case 3:		OCR5CH = lampPWM>>8;	OCR5CL = (unsigned char)lampPWM;	break;
			default:	break;
			}
		}
}
unsigned int ReadTestLampPWM(unsigned char n){
	unsigned int lampPWM;
	if(n<4){
		EEPROM_READ(EEP_ADD_LAMPSET+n*2,   lampPWM);
		return lampPWM;
		}
	return 0;
}

// 调整光源值,上位机输入正或者负的调整量,完毕向上位机返回调整完后的结果
unsigned int AdjustTestLamp(unsigned char n, unsigned int adj){
	// 光源序号和调整量
	// 返回调整后的结果
	unsigned int lampPWM;
	signed int i;
	
	if(n<4){
		if(adj == 0)
			adj = 1;
		if(adj > 1020)
			adj = 1020;
		lampPWM = adj;
		EEPROM_WRITE(EEP_ADD_LAMPSET+n*2,   lampPWM);
		ReSetTestLampPWM(n);
		}
	return lampPWM;
}

unsigned char SetReadCloseAnswer(void)
{
	ReadColseAnswer = 1;
	Uart0ReUnable;
	uart_Printf("%s\r\n",strM3159);
	Uart0ReEnable;
}

void TestQueueDatInit(void){
	unsigned int i;
	unsigned char *pChar;

	pChar = (unsigned char *)&(TestQueueA);
	for(i=0; i<sizeof(TEST_QUEUE); i++){
		*pChar++ = 0;
		}
	pChar = (unsigned char *)&(TestQueueB);
	for(i=0; i<sizeof(TEST_QUEUE); i++){
		*pChar++ = 0;
		}
	for(i=0; i<TEST_QUEUE_NUM; i++){
		TestQueueA.ringNum[i] = 0xff;
		TestQueueB.ringNum[i] = 0xff;
		}
	ReSetTestLampPWM(0);
	ReSetTestLampPWM(1);
	ReSetTestLampPWM(2);
	ReSetTestLampPWM(3);
}

unsigned char TestQueueInsertCalculate(TEST_QUEUE * pTestQueue, signed long testTime){
	unsigned char i;
	signed long time;
	for(i=0; i<TEST_QUEUE_NUM; i++){		// 按顺序遍历队列，以测试时间为条件找出合适的位置插入队列
		time = pTestQueue->testTime[i];
		if(time == 0){
			// 在队尾，插入队尾
			return i;	// 返回插入位置
			}
		else{	// 在队列中间，和当前队列元素的时间做比较
			time += TEST_CYCLE_TIME;
			if(testTime>time){	// 插入的时间在当前元素时间之后，继续比较下个队列元素
				continue;		// 继续判断下一个
				}
			else if(testTime<=(time-TEST_CYCLE_TIME)){	// 插入的时间在当前元素时间前
				// 插入队中间
				return i;	// 返回插入位置
				}
			else{
				// 和当前元素时间有冲突，不能插入
				return 0xff;
				}
			}
		}
	return 0xff;
}

void TestQueueInsert(TEST_QUEUE * pTestQueue, unsigned char idx, unsigned char ringNum, signed long testTime){
	// 测试队列插入新任务
	// pTestQueue:测试队列，idx:插入位置，ringNum:转盘上的编号，testTime:测试时间
	unsigned char i;
	i = TEST_QUEUE_NUM-1;
	while(i--){
		if(i != idx){
			pTestQueue->testTime[i] = pTestQueue->testTime[i-1];
			pTestQueue->ringNum[i] = pTestQueue->ringNum[i-1];
			}
		else{
			pTestQueue->testTime[i] = testTime;
			pTestQueue->ringNum[i] = ringNum;
			return;
			}
		}
}
void TestQueueForward(TEST_QUEUE * pTestQueue){
	// 测试队列前进一步，当队头测试完毕后，队头元素删除队列前进一步
	unsigned char i;
	for(i=0; i<TEST_QUEUE_NUM; i++){
		if(i != (TEST_QUEUE_NUM-1)){
			// 每个元素往前移动一个，队头元素被后面覆盖
			pTestQueue->testTime[i] = pTestQueue->testTime[i+1];
			pTestQueue->ringNum[i] = pTestQueue->ringNum[i+1];
			}
		else{
			// 队尾填充零
			pTestQueue->testTime[i] = 0;
			pTestQueue->ringNum[i] = 0xff;
			}
		}
}

unsigned char InsertNewTest(SAMP_INFO * sampInfo, unsigned char ringNum){
	// 新的测试插入测试队列，稀释完准备滴样时调用本函数，直到插入成功
	// 插入成功返回0，失败返回0xff
	unsigned char inst0, inst1;
	signed long testTime0, testTime1;
	TEST_QUEUE * pTestQueue;

	// 计算测试时间
	testTime0 = sampInfo->testTime0;	// 第一测试时间，如果值为零表示没有此测试
	if(testTime0 != 0)
		testTime0 += SecondCount;
	testTime1 = sampInfo->testTime1;	// 第二测试时间，如果值为零表示没有此测试
	if(testTime1 != 0)
		testTime1 += SecondCount;
	
	// 根据测试方法选则对应的测试队列
	if(sampInfo->readType == 0)
		pTestQueue = &TestQueueA;
	else
		pTestQueue = &TestQueueB;
	
	inst0 = 0;
	inst1 = 0;
	// 测试队列插入位置计算
	if(testTime0)
		inst0 = TestQueueInsertCalculate(pTestQueue,testTime0);
	if(testTime1)
		inst1 = TestQueueInsertCalculate(pTestQueue,testTime1);
	if(inst0 != 0xff && inst1 != 0xff){		// 如果有两个测试时间，则必需要两个测试都插入成功才可以插入本次测试到测试队列
		// 测试插入计算成功，开始插入
		if(testTime0)
			TestQueueInsert(pTestQueue,inst0,ringNum,testTime0);
		if(testTime1)
			TestQueueInsert(pTestQueue,inst1,ringNum,testTime1);
		// 同时计算卸片时间并放入卸片队列
		
		return 0;
		}
	else{
		return 0xff;
		}
}

static unsigned char TestAProcess_workStep,TestAProcess_mainStep;
static unsigned char TestBProcess_workStep,TestBProcess_mainStep;

void printf_TestProcess_StepState(void)
{
	uart_Printf("*3204 TestAStepState $%2d $%2d\r\n",TestAProcess_mainStep,TestAProcess_workStep);
	uart_Printf("*3205 TestBStepState $%2d $%2d\r\n",TestBProcess_mainStep,TestBProcess_workStep);
}

extern unsigned char _TestAMainStep, _TestAWorkStep;
unsigned char ReReadFlag; 		// 重新读取标记，在测试周期将此变量赋值将重新运行转盘和读数
unsigned char TestAQueueProcess(void){
	// 监测测试队列上的测试时间，将到时间的测试任务启动
	static unsigned char mainStep;		
	static unsigned char workStep;
	static unsigned char inWork;
	static unsigned char ringNum;
	static unsigned char watiMotTurnplate, waitMotLifter;
	static unsigned char ReTestCnt;				// 重复读取计数
	unsigned char ucTmp;
	unsigned int i,j;
	unsigned long l;
	
	TestAProcess_mainStep = mainStep;
	TestAProcess_workStep = workStep;
	
	if(inWork){
		if(WaitDelayTime(MOT_TURN_PLATE))
		{
			if(mainStep == 5)	// 测试期间
			{
				if(ReReadFlag == 1)	// 重新测试标记
				{
					ReTestCnt++;
					if(ReTestCnt == 1)
					{
						MotRunToSite(MOT_TURN_PLATE,TestQueueA.ringNum[0]);		// 转盘转动
						ReReadFlag = 0;		// 执行完后清除重测标识
						watiMotTurnplate = 1;
						mainStep = 1;
					}
					else
					{
						ReReadFlag = 0;		// 执行完后清除重测标识
						mainStep = 21;
					}
				}
				if(ReadColseAnswer)		// 接收到检测完成应答信号，关闭检测
				{
					SetDelayTime(MOT_TURN_PLATE, 5);
					ReadColseAnswer = 0;
				}
			}
			return 0;
			}
		if(watiMotTurnplate){	if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	return 0;	watiMotTurnplate = 0;	}
		}
	switch(mainStep){
		case 0:		// 监测队头的测试时间
			if(TestQueueA.ringNum[0] == 0xff)
			{
		//		if(LampAState != 0)		// 无测试任务,关闭测试光源
		//			TestALampClose();
				return 1;	// 无测试任务
			}
			else
			{
		//		if(LampAState == 0)		// 有测试任务,开启光源
		//			TestALampOpen();
			}
			if(TestQueueA.testTime[0] != 0)
			{
				ringNum = TestQueueA.ringNum[0];
				if(SecondCount > (TestQueueA.testTime[0]-10))
				{
					// 测试时间到
					if(TurnPlateUsedLock == 0)
					{
						if(255 == insertflag[ringNum])
						{
							insertflag[ringNum] = 200;
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n",strM3122,ringNum,insertflag[ringNum]);
							Uart0ReEnable;
							TurnPlateUsedLock = 1;		// 占用转盘标识
							//mainStep = 1;
							mainStep = 10;
							workStep = 0;
							inWork = 1; 
						}
					}
				}
			}
			break;
		case 10:
			SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);// 140,2
			MotRunToSite(MOT_TURN_PLATE,0);		// 转盘转到零位	
			watiMotTurnplate = 1;
			mainStep = 11;
			break;
		case 11:
			SetDelayTime(MOT_TURN_PLATE,2);
			mainStep = 1;
			break;
		case 1:		// 转盘转到当前位置
		//	ringNum = TestQueueA.ringNum[0];
			ucTmp = ringNum + 11;
#ifndef UartSendLong
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3148,RingQueue.sampInfo[ringNum].testSerial);//2016-05-17
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3148);
			uart0SendInt(RingQueue.sampInfo[ringNum].testSerial);
#endif
			uart_Printf(" $%4d\r\n",ringNum);//2016-05-17
			Uart0ReUnable;
			if(ucTmp>=RING_QUEUE_NUM)
				ucTmp -= RING_QUEUE_NUM;
			SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);// 140,2
			MotRunToSite(MOT_TURN_PLATE,ucTmp);		// 转盘转到当前位置			
			watiMotTurnplate = 1;
			mainStep = 2;
			break;
		case 2:
			MotRun(MOT_TURN_PLATE, 100);	// 20
			watiMotTurnplate = 1;
			SetDelayTime(MOT_TURN_PLATE, 5);
			mainStep = 3;
			break;
		case 3:
			//MotRun(MOT_TURN_PLATE, -120);	// 20
			//2016-06-16 回来的少走一点，速度变慢
			SetMotRunPam(MOT_TURN_PLATE,140,20,CURRENT_TURN_PLATE);// 140,2
			MotRun(MOT_TURN_PLATE, -110);	// 20
			////////////////////////////////////////
			watiMotTurnplate = 1;
			SetDelayTime(MOT_TURN_PLATE, 10);
			ReReadFlag = 0;	// 重测标识初始化
			if(LampAState == 0)
				TestALampOpen();
			mainStep = 4;
			break;
		case 4:		// 开始检测
			if(SecondCount < (TestQueueA.testTime[0]))
				break;
			ringNum = TestQueueA.ringNum[0];
			// 开启检测光源
		//	TestALampOpen();
			l = RingQueue.sampInfo[ringNum].testSerial;
		//	j = RingQueue.sampInfo[ringNum].testTime0;
			j = RingQueue.sampInfo[ringNum].testTime0+(unsigned int)(SecondCount-(TestQueueA.testTime[0]));
#ifndef UartSendLong
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3112, l);
			uart_Printf(" $%4d $%4d",j, ringNum);
			uart_Printf(" $%8d\r\n",(unsigned long)SecondCount);
			Uart0ReUnable;
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3112);
			uart0SendInt(l);
			uart_Printf(" $%4d $%4d $ ",j, ringNum);
			uart0SendInt(SecondCount);
			uart_Printf("\r\n");
			Uart0ReUnable;
#endif
			ReadColseAnswer = 0;	// 测试开始初始化关闭信号
			SetDelayTime(MOT_TURN_PLATE, 100);	// 读数延迟
			mainStep = 5;
			break;
		case 5:		// 检测完毕,释放转盘使用权,关闭检测光源
			l = RingQueue.sampInfo[ringNum].testSerial;
#ifndef UartSendLong
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3114, l);
			uart_Printf(" $%8d\r\n",(unsigned long)SecondCount);
			Uart0ReEnable;
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3114);
			uart0SendInt(l);
			uart_Printf(" $ ");
			uart0SendInt(SecondCount);
			uart_Printf("\r\n");
			Uart0ReEnable;
#endif
			// 关闭检测光源
			if(LampAState != 0 && CardNoneUseful == 1)
				TestALampClose();
			TestQueueForward(&TestQueueA);		// 删除已完成的队头任务，队列前进一步
			
		//	MotRunToSite(MOT_TURN_PLATE,0);		// 转盘运行到零位
		//	SetDelayTime(MOT_TURN_PLATE, 10);
		//	watiMotTurnplate = 1;
			mainStep = 6;
			break;
		
			case 21:		// 转盘转到当前位置			 //  20170518pan
#ifndef UartSendLong	
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3148,RingQueue.sampInfo[ringNum].testSerial);//2016-05-17
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3148);
			uart0SendInt(RingQueue.sampInfo[ringNum].testSerial);
#endif
			uart_Printf(" $%4d\r\n",ringNum);//2016-05-17
			Uart0ReEnable;
			mainStep = 3;
			workStep = 0;
			break;

		case 6:		// 释放转盘使用权
			mainStep = 0;
			workStep = 0;
			inWork = 0;
			TurnPlateUsedLock = 0;
			TurnPlateUsedLock = 0;
			break;
		}
	return 0;
}

unsigned char TestBQueueProcess(void){
	// 监测测试队列上的测试时间，将到时间的测试任务启动
	static unsigned char mainStep;		
	static unsigned char workStep;
	static unsigned char inWork;
	static unsigned char ringNum;
	static unsigned char watiMotTurnplate, waitMotLifter;
	static unsigned char ReTestCnt;				// 重复读取计数
	unsigned char ucTmp;
	unsigned int i, j;
	unsigned long l;
	TestBProcess_mainStep = mainStep;
	TestBProcess_workStep = workStep;
	if(inWork){
		if(WaitDelayTime(MOT_TURN_PLATE))
		{
			if(mainStep == 5)	// 测试期间
			{
				if(ReReadFlag == 1)
				{
					ReTestCnt++;
					if(ReTestCnt == 1)
					{
						MotRunToSite(MOT_TURN_PLATE,TestQueueB.ringNum[0]);		// 转盘转动
						ReReadFlag = 0;		// 执行完后清除重测标识
						watiMotTurnplate = 1;
						mainStep = 1;
					}
					else
					{
						ReReadFlag = 0;		// 执行完后清除重测标识
						mainStep = 21;
					}
				}
				if(ReadColseAnswer)	// 接收到检测完成应答信号，关闭检测
				{
					SetDelayTime(MOT_TURN_PLATE, 5);
					ReadColseAnswer = 0;
				}
			}
			return 0;
			}
		if(watiMotTurnplate){	if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	return 0;	watiMotTurnplate = 0;	}
		}
	
	switch(mainStep)
	{
		case 0:		// 监测队头的测试时间
			if(TestQueueB.ringNum[0] == 0xff)
			{
			//	if(LampBState != 0)		// 无测试任务,关闭测试光源
			//		TestBLampClose();
				return 1;
			}
			else
			{
			//	if(LampBState == 0)		// 有测试任务,开启光源
			//		TestBLampOpen();
			}
			if(TestQueueB.testTime[0] != 0)
			{
				if(SecondCount > (TestQueueB.testTime[0]-10))
				{
					ringNum = TestQueueB.ringNum[0];	
					// 测试时间到
					if(TurnPlateUsedLock == 0)
					{
						if(255 == insertflag[ringNum])
						{
							insertflag[ringNum] = 200;
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n",strM3122,ringNum,insertflag[ringNum]);
							Uart0ReEnable;
							TurnPlateUsedLock = 1;		// 占用转盘标识
							//mainStep = 1;
							mainStep = 10;
							workStep = 0;
							inWork = 1; 
						} 
					}
				}
			}
			break;
		case 10:
			SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);// 140,2
			MotRunToSite(MOT_TURN_PLATE,0);		// 转盘转到零位	
			watiMotTurnplate = 1;
			mainStep = 11;
			break;
		case 11:
			SetDelayTime(MOT_TURN_PLATE,2);
			mainStep = 1;
			break;
		case 1:		// 转盘转到当前位置			
		//	ringNum = TestQueueB.ringNum[0];
			ucTmp = ringNum + 19;	
#ifndef UartSendLong	
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3148,RingQueue.sampInfo[ringNum].testSerial);//2016-05-17
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3148);
			uart0SendInt(RingQueue.sampInfo[ringNum].testSerial);
#endif
			uart_Printf(" $%4d\r\n",ringNum);//2016-05-17
			Uart0ReEnable;
			if(ucTmp>=RING_QUEUE_NUM)
				ucTmp -= RING_QUEUE_NUM;
			SetMotRunPam(MOT_TURN_PLATE,200,20,CURRENT_TURN_PLATE);
			MotRunToSite(MOT_TURN_PLATE,ucTmp);		// 转盘转到当前位置
			watiMotTurnplate = 1;
			mainStep = 2;
			workStep = 0;
			break;
		case 2:
			MotRun(MOT_TURN_PLATE, 100);	// 20
			watiMotTurnplate = 1;
			SetDelayTime(MOT_TURN_PLATE, 5);
			mainStep = 3;
			break;
		case 3:
			MotRun(MOT_TURN_PLATE, -120);	// 20
			watiMotTurnplate = 1;
			SetDelayTime(MOT_TURN_PLATE, 10);
			ReReadFlag = 0;	// 重测标识初始化
			//将B改成A
			if(LampAState == 0)
				TestALampOpen();
			mainStep = 4;
			break;
		case 4:		// 开始检测
			if(SecondCount < (TestQueueB.testTime[0]))
				break;
			ringNum = TestQueueB.ringNum[0];
			// 开启检测光源
		//	TestBLampOpen();
			l = RingQueue.sampInfo[ringNum].testSerial;
		//	j = RingQueue.sampInfo[ringNum].testTime0;
			j = RingQueue.sampInfo[ringNum].testTime0+(unsigned int)(SecondCount-(TestQueueB.testTime[0]));
#ifndef UartSendLong
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3113, l);
			uart_Printf(" $%4d $%4d",j, ringNum);
			uart_Printf(" $%8d\r\n",(unsigned long)SecondCount);
			Uart0ReEnable;
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3113);
			uart0SendInt(l);
			uart_Printf(" $%4d $%4d $ ",j, ringNum);
			uart0SendInt(SecondCount);
			uart_Printf("\r\n");
			Uart0ReEnable;
#endif
			ReadColseAnswer = 0;	// 测试开始初始化关闭信号
			SetDelayTime(MOT_TURN_PLATE, 100);	// 读数延迟
			mainStep = 5;
			break;
		case 5:		// 检测完毕,释放转盘使用权,关闭检测光源
			l = RingQueue.sampInfo[ringNum].testSerial;
#ifndef UartSendLong
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3115, l);
			uart_Printf(" $%8d\r\n",(unsigned long)SecondCount);
			Uart0ReEnable;
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3115, l);
			uart0SendInt(l);
			uart_Printf(" $ ");
			uart0SendInt(SecondCount);
			uart_Printf("\r\n");
			Uart0ReEnable;
#endif
			// 关闭检测光源,B改成A
			if(LampAState != 0 && CardNoneUseful == 1)
				TestALampClose();
			TestQueueForward(&TestQueueB);		// 删除已完成的队头任务，队列前进一步
			
		//	MotRunToSite(MOT_TURN_PLATE,0);		// 转盘运行到零位
			SetDelayTime(MOT_TURN_PLATE, 10);
		//	watiMotTurnplate = 1;
			mainStep = 6;
			break;
			
			case 21:		// 转盘转到当前位置			 //  20170518pan
#ifndef UartSendLong	
			Uart0ReUnable;
			uart_Printf("%s $%8d",strM3148,RingQueue.sampInfo[ringNum].testSerial);//2016-05-17
#else
			Uart0ReUnable;
			uart_Printf("%s $ ",strM3148);
			uart0SendInt(RingQueue.sampInfo[ringNum].testSerial);
#endif
			uart_Printf(" $%4d\r\n",ringNum);//2016-05-17
			Uart0ReEnable;
			mainStep = 3;
			workStep = 0;
			break;
			
		case 6:		// 释放转盘使用权
			mainStep = 0;
			workStep = 0;
			inWork = 0;
			ReTestCnt = 0;
			TurnPlateUsedLock = 0;
			TurnPlateUsedLock = 0;
			break;
		}
	return 0;
}

void SetReReadFlag(void)
{
	ReReadFlag = 1;
	Uart0ReUnable;
	uart_Printf("%s\r\n",strM3155);
	Uart0ReEnable;
}


