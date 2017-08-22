#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"


// ת�̶��д���

unsigned char TurnPlateUsedLock;		// ת��ʹ����

RING_QUEUE	RingQueue;
extern unsigned long SecondCount;	// ��ʱ�Ӽ���

void RingQueueDatInit(void){
	unsigned int i;
	unsigned char *pChar;

	pChar = (unsigned char *)&(RingQueue);
	for(i=0; i<sizeof(RING_QUEUE); i++){
		*pChar++ = 0;
		}
	TurnPlateUsedLock = 0;
}

unsigned char RingQueueInsertCalculate(void){
	// ���ҳ�ת����һ������λ�ã����ز��ҵ���λ�ñ��
	unsigned char i, ringNum;
	ringNum = RingQueue.prevNum;		
	i = RING_QUEUE_NUM;
	while(i--){
		// ��ת���ϴβ����Ƭλ�ÿ�ʼ�������ҿ�λ��
		ringNum ++;
		if(ringNum == RING_QUEUE_NUM)
			ringNum = 0;
		if(RingQueue.flag[ringNum] == 0)	// �ҵ�ת�̿�λ��
			break;
		}
	if(i == 255)   // 0-1 = 255
	{
		Uart0ReUnable;
		uart_Printf("%s $%d\r\n",strE3935,RingQueue.prevNum); 
		Uart0ReEnable;
		return 0xff;			// δ�ҵ���λ�ã�����ʧ��
	}
	RingQueue.prevNum = ringNum;
	return ringNum;
}

/*
unsigned char RingQueueInsertCalculate(void)
{
	// ���ҳ�ת����һ������λ�ã����ز��ҵ���λ�ñ��
	unsigned char ringNum;
	ringNum = RingQueue.prevNum;
	ringNum++;
	if(ringNum >= RING_QUEUE_NUM)
		ringNum = 0;
	RingQueue.prevNum = ringNum;
	return ringNum;
}
*/

void RingQueueInsert(unsigned char ringNum,SAMP_INFO * newSamp){
	// ת�̶��в����²��ԣ�ֻ���걾��Ϣ����ת�̣��ȴ���Ƭװ��ת�̺��������²���λ����Ч��ʶ
	//RingQueue.flag[ringNum] = 1;		// ����ʹ�ñ��
	MemCopy(newSamp, &(RingQueue.sampInfo[ringNum]), sizeof(SAMP_INFO));		// ���������Ϣ
}

void RingQueueDelete(unsigned char ringNum){
	RingQueue.flag[ringNum] = 0;
}

void SetRingQueueUnitUsed(unsigned char ringNum){
	RingQueue.flag[ringNum] = 1;
}
