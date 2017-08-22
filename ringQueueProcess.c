#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"


// 转盘队列处理

unsigned char TurnPlateUsedLock;		// 转盘使用锁

RING_QUEUE	RingQueue;
extern unsigned long SecondCount;	// 秒时钟计数

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
	// 查找出转盘下一个插入位置，返回查找到的位置编号
	unsigned char i, ringNum;
	ringNum = RingQueue.prevNum;		
	i = RING_QUEUE_NUM;
	while(i--){
		// 从转盘上次插入干片位置开始遍历查找空位置
		ringNum ++;
		if(ringNum == RING_QUEUE_NUM)
			ringNum = 0;
		if(RingQueue.flag[ringNum] == 0)	// 找到转盘空位置
			break;
		}
	if(i == 255)   // 0-1 = 255
	{
		Uart0ReUnable;
		uart_Printf("%s $%d\r\n",strE3935,RingQueue.prevNum); 
		Uart0ReEnable;
		return 0xff;			// 未找到空位置，查找失败
	}
	RingQueue.prevNum = ringNum;
	return ringNum;
}

/*
unsigned char RingQueueInsertCalculate(void)
{
	// 查找出转盘下一个插入位置，返回查找到的位置编号
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
	// 转盘队列插入新测试，只将标本信息放入转盘，等待干片装入转盘后再设置新插入位置有效标识
	//RingQueue.flag[ringNum] = 1;		// 设置使用标记
	MemCopy(newSamp, &(RingQueue.sampInfo[ringNum]), sizeof(SAMP_INFO));		// 存入测试信息
}

void RingQueueDelete(unsigned char ringNum){
	RingQueue.flag[ringNum] = 0;
}

void SetRingQueueUnitUsed(unsigned char ringNum){
	RingQueue.flag[ringNum] = 1;
}
