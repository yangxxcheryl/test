

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"
#include "eeprom.h"

unsigned char insertflag[30];	// ����ת�̱�־  255 ����  200 ��Ƭ 0 ����

TEST_QUEUE TestQueueA;
TEST_QUEUE TestQueueB;
extern RING_QUEUE	RingQueue;
extern unsigned long SecondCount;	// ��ʱ�Ӽ���
extern unsigned char TurnPlateUsedLock;		// ת��ʹ����
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
	// ��ȡ��Դ����ֵ������
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

// ������Դֵ,��λ�����������߸��ĵ�����,�������λ�����ص������Ľ��
unsigned int AdjustTestLamp(unsigned char n, unsigned int adj){
	// ��Դ��ź͵�����
	// ���ص�����Ľ��
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
	for(i=0; i<TEST_QUEUE_NUM; i++){		// ��˳��������У��Բ���ʱ��Ϊ�����ҳ����ʵ�λ�ò������
		time = pTestQueue->testTime[i];
		if(time == 0){
			// �ڶ�β�������β
			return i;	// ���ز���λ��
			}
		else{	// �ڶ����м䣬�͵�ǰ����Ԫ�ص�ʱ�����Ƚ�
			time += TEST_CYCLE_TIME;
			if(testTime>time){	// �����ʱ���ڵ�ǰԪ��ʱ��֮�󣬼����Ƚ��¸�����Ԫ��
				continue;		// �����ж���һ��
				}
			else if(testTime<=(time-TEST_CYCLE_TIME)){	// �����ʱ���ڵ�ǰԪ��ʱ��ǰ
				// ������м�
				return i;	// ���ز���λ��
				}
			else{
				// �͵�ǰԪ��ʱ���г�ͻ�����ܲ���
				return 0xff;
				}
			}
		}
	return 0xff;
}

void TestQueueInsert(TEST_QUEUE * pTestQueue, unsigned char idx, unsigned char ringNum, signed long testTime){
	// ���Զ��в���������
	// pTestQueue:���Զ��У�idx:����λ�ã�ringNum:ת���ϵı�ţ�testTime:����ʱ��
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
	// ���Զ���ǰ��һ��������ͷ������Ϻ󣬶�ͷԪ��ɾ������ǰ��һ��
	unsigned char i;
	for(i=0; i<TEST_QUEUE_NUM; i++){
		if(i != (TEST_QUEUE_NUM-1)){
			// ÿ��Ԫ����ǰ�ƶ�һ������ͷԪ�ر����渲��
			pTestQueue->testTime[i] = pTestQueue->testTime[i+1];
			pTestQueue->ringNum[i] = pTestQueue->ringNum[i+1];
			}
		else{
			// ��β�����
			pTestQueue->testTime[i] = 0;
			pTestQueue->ringNum[i] = 0xff;
			}
		}
}

unsigned char InsertNewTest(SAMP_INFO * sampInfo, unsigned char ringNum){
	// �µĲ��Բ�����Զ��У�ϡ����׼������ʱ���ñ�������ֱ������ɹ�
	// ����ɹ�����0��ʧ�ܷ���0xff
	unsigned char inst0, inst1;
	signed long testTime0, testTime1;
	TEST_QUEUE * pTestQueue;

	// �������ʱ��
	testTime0 = sampInfo->testTime0;	// ��һ����ʱ�䣬���ֵΪ���ʾû�д˲���
	if(testTime0 != 0)
		testTime0 += SecondCount;
	testTime1 = sampInfo->testTime1;	// �ڶ�����ʱ�䣬���ֵΪ���ʾû�д˲���
	if(testTime1 != 0)
		testTime1 += SecondCount;
	
	// ���ݲ��Է���ѡ���Ӧ�Ĳ��Զ���
	if(sampInfo->readType == 0)
		pTestQueue = &TestQueueA;
	else
		pTestQueue = &TestQueueB;
	
	inst0 = 0;
	inst1 = 0;
	// ���Զ��в���λ�ü���
	if(testTime0)
		inst0 = TestQueueInsertCalculate(pTestQueue,testTime0);
	if(testTime1)
		inst1 = TestQueueInsertCalculate(pTestQueue,testTime1);
	if(inst0 != 0xff && inst1 != 0xff){		// �������������ʱ�䣬�����Ҫ�������Զ�����ɹ��ſ��Բ��뱾�β��Ե����Զ���
		// ���Բ������ɹ�����ʼ����
		if(testTime0)
			TestQueueInsert(pTestQueue,inst0,ringNum,testTime0);
		if(testTime1)
			TestQueueInsert(pTestQueue,inst1,ringNum,testTime1);
		// ͬʱ����жƬʱ�䲢����жƬ����
		
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
unsigned char ReReadFlag; 		// ���¶�ȡ��ǣ��ڲ������ڽ��˱�����ֵ����������ת�̺Ͷ���
unsigned char TestAQueueProcess(void){
	// �����Զ����ϵĲ���ʱ�䣬����ʱ��Ĳ�����������
	static unsigned char mainStep;		
	static unsigned char workStep;
	static unsigned char inWork;
	static unsigned char ringNum;
	static unsigned char watiMotTurnplate, waitMotLifter;
	static unsigned char ReTestCnt;				// �ظ���ȡ����
	unsigned char ucTmp;
	unsigned int i,j;
	unsigned long l;
	
	TestAProcess_mainStep = mainStep;
	TestAProcess_workStep = workStep;
	
	if(inWork){
		if(WaitDelayTime(MOT_TURN_PLATE))
		{
			if(mainStep == 5)	// �����ڼ�
			{
				if(ReReadFlag == 1)	// ���²��Ա��
				{
					ReTestCnt++;
					if(ReTestCnt == 1)
					{
						MotRunToSite(MOT_TURN_PLATE,TestQueueA.ringNum[0]);		// ת��ת��
						ReReadFlag = 0;		// ִ���������ز��ʶ
						watiMotTurnplate = 1;
						mainStep = 1;
					}
					else
					{
						ReReadFlag = 0;		// ִ���������ز��ʶ
						mainStep = 21;
					}
				}
				if(ReadColseAnswer)		// ���յ�������Ӧ���źţ��رռ��
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
		case 0:		// ����ͷ�Ĳ���ʱ��
			if(TestQueueA.ringNum[0] == 0xff)
			{
		//		if(LampAState != 0)		// �޲�������,�رղ��Թ�Դ
		//			TestALampClose();
				return 1;	// �޲�������
			}
			else
			{
		//		if(LampAState == 0)		// �в�������,������Դ
		//			TestALampOpen();
			}
			if(TestQueueA.testTime[0] != 0)
			{
				ringNum = TestQueueA.ringNum[0];
				if(SecondCount > (TestQueueA.testTime[0]-10))
				{
					// ����ʱ�䵽
					if(TurnPlateUsedLock == 0)
					{
						if(255 == insertflag[ringNum])
						{
							insertflag[ringNum] = 200;
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n",strM3122,ringNum,insertflag[ringNum]);
							Uart0ReEnable;
							TurnPlateUsedLock = 1;		// ռ��ת�̱�ʶ
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
			MotRunToSite(MOT_TURN_PLATE,0);		// ת��ת����λ	
			watiMotTurnplate = 1;
			mainStep = 11;
			break;
		case 11:
			SetDelayTime(MOT_TURN_PLATE,2);
			mainStep = 1;
			break;
		case 1:		// ת��ת����ǰλ��
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
			MotRunToSite(MOT_TURN_PLATE,ucTmp);		// ת��ת����ǰλ��			
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
			//2016-06-16 ����������һ�㣬�ٶȱ���
			SetMotRunPam(MOT_TURN_PLATE,140,20,CURRENT_TURN_PLATE);// 140,2
			MotRun(MOT_TURN_PLATE, -110);	// 20
			////////////////////////////////////////
			watiMotTurnplate = 1;
			SetDelayTime(MOT_TURN_PLATE, 10);
			ReReadFlag = 0;	// �ز��ʶ��ʼ��
			if(LampAState == 0)
				TestALampOpen();
			mainStep = 4;
			break;
		case 4:		// ��ʼ���
			if(SecondCount < (TestQueueA.testTime[0]))
				break;
			ringNum = TestQueueA.ringNum[0];
			// ��������Դ
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
			ReadColseAnswer = 0;	// ���Կ�ʼ��ʼ���ر��ź�
			SetDelayTime(MOT_TURN_PLATE, 100);	// �����ӳ�
			mainStep = 5;
			break;
		case 5:		// ������,�ͷ�ת��ʹ��Ȩ,�رռ���Դ
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
			// �رռ���Դ
			if(LampAState != 0 && CardNoneUseful == 1)
				TestALampClose();
			TestQueueForward(&TestQueueA);		// ɾ������ɵĶ�ͷ���񣬶���ǰ��һ��
			
		//	MotRunToSite(MOT_TURN_PLATE,0);		// ת�����е���λ
		//	SetDelayTime(MOT_TURN_PLATE, 10);
		//	watiMotTurnplate = 1;
			mainStep = 6;
			break;
		
			case 21:		// ת��ת����ǰλ��			 //  20170518pan
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

		case 6:		// �ͷ�ת��ʹ��Ȩ
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
	// �����Զ����ϵĲ���ʱ�䣬����ʱ��Ĳ�����������
	static unsigned char mainStep;		
	static unsigned char workStep;
	static unsigned char inWork;
	static unsigned char ringNum;
	static unsigned char watiMotTurnplate, waitMotLifter;
	static unsigned char ReTestCnt;				// �ظ���ȡ����
	unsigned char ucTmp;
	unsigned int i, j;
	unsigned long l;
	TestBProcess_mainStep = mainStep;
	TestBProcess_workStep = workStep;
	if(inWork){
		if(WaitDelayTime(MOT_TURN_PLATE))
		{
			if(mainStep == 5)	// �����ڼ�
			{
				if(ReReadFlag == 1)
				{
					ReTestCnt++;
					if(ReTestCnt == 1)
					{
						MotRunToSite(MOT_TURN_PLATE,TestQueueB.ringNum[0]);		// ת��ת��
						ReReadFlag = 0;		// ִ���������ز��ʶ
						watiMotTurnplate = 1;
						mainStep = 1;
					}
					else
					{
						ReReadFlag = 0;		// ִ���������ز��ʶ
						mainStep = 21;
					}
				}
				if(ReadColseAnswer)	// ���յ�������Ӧ���źţ��رռ��
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
		case 0:		// ����ͷ�Ĳ���ʱ��
			if(TestQueueB.ringNum[0] == 0xff)
			{
			//	if(LampBState != 0)		// �޲�������,�رղ��Թ�Դ
			//		TestBLampClose();
				return 1;
			}
			else
			{
			//	if(LampBState == 0)		// �в�������,������Դ
			//		TestBLampOpen();
			}
			if(TestQueueB.testTime[0] != 0)
			{
				if(SecondCount > (TestQueueB.testTime[0]-10))
				{
					ringNum = TestQueueB.ringNum[0];	
					// ����ʱ�䵽
					if(TurnPlateUsedLock == 0)
					{
						if(255 == insertflag[ringNum])
						{
							insertflag[ringNum] = 200;
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n",strM3122,ringNum,insertflag[ringNum]);
							Uart0ReEnable;
							TurnPlateUsedLock = 1;		// ռ��ת�̱�ʶ
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
			MotRunToSite(MOT_TURN_PLATE,0);		// ת��ת����λ	
			watiMotTurnplate = 1;
			mainStep = 11;
			break;
		case 11:
			SetDelayTime(MOT_TURN_PLATE,2);
			mainStep = 1;
			break;
		case 1:		// ת��ת����ǰλ��			
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
			MotRunToSite(MOT_TURN_PLATE,ucTmp);		// ת��ת����ǰλ��
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
			ReReadFlag = 0;	// �ز��ʶ��ʼ��
			//��B�ĳ�A
			if(LampAState == 0)
				TestALampOpen();
			mainStep = 4;
			break;
		case 4:		// ��ʼ���
			if(SecondCount < (TestQueueB.testTime[0]))
				break;
			ringNum = TestQueueB.ringNum[0];
			// ��������Դ
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
			ReadColseAnswer = 0;	// ���Կ�ʼ��ʼ���ر��ź�
			SetDelayTime(MOT_TURN_PLATE, 100);	// �����ӳ�
			mainStep = 5;
			break;
		case 5:		// ������,�ͷ�ת��ʹ��Ȩ,�رռ���Դ
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
			// �رռ���Դ,B�ĳ�A
			if(LampAState != 0 && CardNoneUseful == 1)
				TestALampClose();
			TestQueueForward(&TestQueueB);		// ɾ������ɵĶ�ͷ���񣬶���ǰ��һ��
			
		//	MotRunToSite(MOT_TURN_PLATE,0);		// ת�����е���λ
			SetDelayTime(MOT_TURN_PLATE, 10);
		//	watiMotTurnplate = 1;
			mainStep = 6;
			break;
			
			case 21:		// ת��ת����ǰλ��			 //  20170518pan
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
			
		case 6:		// �ͷ�ת��ʹ��Ȩ
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


