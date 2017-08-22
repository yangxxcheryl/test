
#include <iom1280v.h>
#include <macros.h>
#include "B1404_LIB.h"
#include "LibCommon.h"



/************************* 从机命令缓冲区 **************************/
typedef struct _COMMAND_BUF
{
	COMMAND_STRING buf[COMMAND_BUF_LEN];
	unsigned char pTop;
	unsigned char pEnd;
	unsigned char transmitState;	// 命令发送状态 0已经执行完毕或空命令，1新命令存入等待送， 2新命令已发送处于执行状态
}COMMAND_BUF;

// 命令发送控制
typedef struct _COMMAND_TRANSMIT_CONTROL
{
	unsigned char registerSlave[SLAVE_NUM];	// 使用的从机在这里登记
	unsigned char curSlave;					// 当前的通信的从机
	unsigned char waitCount;				// 信息返回超时计时
	COMMAND_BUF cmdBuf[SLAVE_NUM];			// 命令缓冲组，每个元素下标表示相应的从机编号
	unsigned char slaveState[SLAVE_NUM];	// 记录每个从机的状态
}COMMAND_TRANSMIT_CONTROL;

// 当前命令发送缓冲区
typedef struct _TRANSFER_BUF
{
	COMMAND_STRING cmd;
	unsigned char pSend;					// 命令数据发送指针
}TRANSFER_BUF;

/*typedef struct _COMMAND_URGENT_BUF{
	COMMAND_STRING buf[COMMAND_BUF_LEN];
	unsigned char slaveNum[COMMAND_BUF_LEN];	// 命令对应的从机号
	unsigned char pTop;
	unsigned char pEnd;
}COMMAND_URGENT_BUF;*/
typedef struct _COMMAND_URGENT_BUF{
	COMMAND_STRING buf[SLAVE_NUM];	// 数组下标对应从机号
	unsigned char state[SLAVE_NUM];	// 命令有效状态, 0:无效，1:有效
//	unsigned char pTop;
//	unsigned char pEnd;
}COMMAND_URGENT_BUF;


/*************************** 信息事件堆 *****************************/

// 信息事件队列
typedef struct _INFO_EVENT_STACK{	// 信息事件队列
	unsigned char pTop;
	unsigned char pEnd;
	INFO_EVENT info[INFO_EVENT_NUM];
}INFO_EVENT_STACK;
/*************************** 错误事件堆 *****************************
typedef struct _ERROR_EVENT{
	unsigned char slaveNum;				// 从机编号
	unsigned char error;				// 错误信息
}ERROR_EVENT;
typedef struct _ERROR_EVENT_STACK{
	unsigned char pTop;
	unsigned char pEnd;
	unsigned char error[ERROR_EVENT_NUM];
}ERROR_EVENT_STACK;
*/
typedef struct _DEVIVE_INFO{
	unsigned char buf[20];
	unsigned char pRec;
	unsigned char infoLen;
	unsigned char isValid;
}DEVICE_INFO;


INFO_EVENT_STACK InfoEventStack;
//ERROR_EVENT_STACK ErrorEventStack;
COMMAND_TRANSMIT_CONTROL TransmitControl;
TRANSFER_BUF TransferBuf;
unsigned char TransferStart;		// 命令传输启动标识
DEVICE_INFO DeviceInfo;
COMMAND_URGENT_BUF CmdUrgentBuf;	// 紧急命令
extern unsigned char ControlModel;			// 0:正常， 1:超级终端调试

void InitCommandTransferData(void){
	unsigned int i;
	unsigned char *p;

	TransferStart = 0;
	p = (unsigned char *)&InfoEventStack;
	for(i=0; i<sizeof(INFO_EVENT_STACK); i++){
		*(p++) = 0;
		}
/*	p = (unsigned char *)&ErrorEventStack;
	for(i=0; i<sizeof(ERROR_EVENT_STACK); i++){
		*(p++) = 0;
		}*/
	p = (unsigned char *)&TransmitControl;
	for(i=0; i<sizeof(COMMAND_TRANSMIT_CONTROL); i++){
		*(p++) = 0;
		}
	p = (unsigned char *)&TransferBuf;
	for(i=0; i<sizeof(TRANSFER_BUF); i++){
		*(p++) = 0;
		}
	p = (unsigned char *)&CmdUrgentBuf;
	for(i=0; i<sizeof(COMMAND_URGENT_BUF); i++){
		*(p++) = 0;
		}
}


// 将收到的从机信息放入信息队列等待处理
void AddInfoEventToStack(unsigned char slave, unsigned char * info, unsigned char len){
	unsigned char pEnd, i, p;
	unsigned char *pChar;
	pEnd = InfoEventStack.pEnd;
	p = pEnd;
	p ++;
	if(p == INFO_EVENT_NUM)
		p = 0;
	if(p != InfoEventStack.pTop){
		InfoEventStack.info[pEnd].slaveNum = slave;
		InfoEventStack.info[pEnd].event = *(info ++);	// 消息类型
		len --;
		InfoEventStack.info[pEnd].infoLen = len;	// 消息参数长度
		pChar = &InfoEventStack.info[pEnd].info[0];
		for(i=0; i<len; i++){
			*(pChar++) = *(info++);
			}
		InfoEventStack.pEnd = p;
		}
	else
	{
		Uart0ReUnable;
		uart_Printf("// Even save fail [%d]\r\n", slave);
		Uart0ReEnable;
	}
}
/*void SetTransferBufStateQueryCmd(unsigned char slaveNum){
	unsigned char pEnd, pTop, p;
	pTop = CmdUrgentBuf.pTop;
	pEnd = CmdUrgentBuf.pEnd;
	
	if(pTop != pEnd){
		if(CmdUrgentBuf.slaveNum[pTop] == slaveNum){
			MemCopy(&(CmdUrgentBuf.buf[pTop]),&TransferBuf.cmd, sizeof(COMMAND_STRING));	// 将新命令存入发送缓冲区
			pTop ++;
			if(pTop == COMMAND_BUF_LEN)
				pTop = 0;
			CmdUrgentBuf.pTop = pTop;
			return;
			}
		}
	TransferBuf.cmd.cmd = (slaveNum<<3) & 0xf8;
	TransferBuf.cmd.cmdLen = 1;
	
}*/
void SetTransferBufStateQueryCmd(unsigned char slaveNum)
{
	if(slaveNum>=SLAVE_NUM)
	{
		Uart0ReUnable;
		uart_Printf("// SlaveNum error! \r\n");
		Uart0ReEnable;
	}
	if(CmdUrgentBuf.state[slaveNum])
	{
		// 有紧急命令
		MemCopy(&(CmdUrgentBuf.buf[slaveNum]),&TransferBuf.cmd, sizeof(COMMAND_STRING));	// 将新命令存入发送缓冲区
		CmdUrgentBuf.state[slaveNum] = 0;	// 命令已处理，清除
	}
	else	// 无紧急命令，发送空闲查询
	{
		TransferBuf.cmd.cmd = (slaveNum<<3) & 0xf8;
		TransferBuf.cmd.cmdLen = 1;
	}
}

void InsertCmdToTransferBuf(unsigned char slaveNum)
{
	COMMAND_BUF * pCmdBuf;
	unsigned char pTop, pEnd;
	
	if(slaveNum>=SLAVE_NUM)
	{
		Uart0ReUnable;
		uart_Printf("// SlaveNum error! \r\n");
		Uart0ReEnable;
	}
	if(CmdUrgentBuf.state[slaveNum])
	{
		// 有紧急命令
		MemCopy(&(CmdUrgentBuf.buf[slaveNum]),&TransferBuf.cmd, sizeof(COMMAND_STRING));	// 将新命令存入发送缓冲区
		CmdUrgentBuf.state[slaveNum] = 0;	// 命令已处理，清除
	}
	else	// 无紧急命令
	{
		pCmdBuf = &TransmitControl.cmdBuf[slaveNum];
		pTop = pCmdBuf->pTop;
		pEnd = pCmdBuf->pEnd;
		if(pTop != pEnd)			// 设置新的指针
		{
			MemCopy(&(pCmdBuf->buf[pTop]),&TransferBuf.cmd, sizeof(COMMAND_STRING));
		}
		else
		{
			TransferBuf.cmd.cmd = (slaveNum<<3) & 0xf8;
			TransferBuf.cmd.cmdLen = 1;
		}
			//	SetTransferBufStateQueryCmd(slaveNum);
	}
}
static unsigned char CommandBackFlag;	// 命令发送返回标识，如果没收到返回信息表示从机丢失
// 发送命令字节
void SendCommandData(void)
{
	// 发送命令数据
	unsigned char curSlave;
	unsigned char p,l,c;
	unsigned char pTop, pEnd;
	static unsigned char checkSum;
	static unsigned char cmdReSendCnt = 0;
	COMMAND_BUF * pCmdBuf;

	if(TransferStart == 0)
		return;				// 命令传输未启动
	
	curSlave = TransmitControl.curSlave;
	pTop = TransferBuf.pSend;
	pEnd = TransferBuf.cmd.cmdLen;
	
	if(pTop < pEnd)
	{
		// 发送命令
		if(pTop == 0)
		{
			if(cmdReSendCnt == 0)
			{
				Uart2SendAdd(0);
				cmdReSendCnt = 1;
				return;	// 地址重发
			}
			Uart2SendAdd(TransferBuf.cmd.cmd);		// 发送地址和命令
			cmdReSendCnt = 0;
			checkSum = 0;
		}
		else
		{
			if(GetUart2DataSendResult()!=0)
			{
				pTop = 0;
				Uart0ReUnable;
				uart_Printf("//DataSendErr1,No:%d\r\n", curSlave);
				Uart0ReEnable;
			}
			c = TransferBuf.cmd.pam[pTop-1];
			Uart2SendDat(c);	// 发送参数
			checkSum += c;
		}
		TransferBuf.pSend ++;	// 数据发送指针加一
	}
	else if(pTop == pEnd)
	{
		// 发送校验
		if(GetUart2DataSendResult()!=0)
		{
			pTop = 0;
			Uart0ReUnable;
			uart_Printf("//DataSendErr2,No:%d\r\n", curSlave);
			Uart0ReEnable;
		}
		if(pEnd > 1)
			Uart2SendDat(checkSum);	// 发送校验和
		TransferBuf.pSend ++;	// 数据发送指针加一
		DeviceInfo.pRec == 0;
		TransmitControl.waitCount = 15;//5
	}
	else
	{
		if(TransmitControl.waitCount)
		{
			TransmitControl.waitCount --;
		}
		else
		{
		/*	if(CommandBackFlag != 255){	// 前一次发送的命令未收到反馈信息，需要重新发送
			//	TransmitControl.cmdBuf[curSlave].transmitState = 2;	// 重新发送命令
				if(CommandBackFlag == 28)
					uart_Printf("// Command back lose: %d %d %d %d %d\r\n", DeviceInfo.buf[0], DeviceInfo.buf[1], DeviceInfo.buf[2], DeviceInfo.buf[3], DeviceInfo.buf[4]);
				else
					uart_Printf("// Command back lose :%d %d, cmd%d\r\n", CommandBackFlag,DeviceInfo.pRec,TransferBuf.cmd.cmd);
				}*/
			CommandBackFlag = 0;
			do
			{
				curSlave ++;
				if(curSlave == SLAVE_NUM)
					curSlave = 0;
			}while(TransmitControl.registerSlave[curSlave]==0);
				
			TransmitControl.curSlave = curSlave;		// 指向下一个有效的从机
			pCmdBuf = &TransmitControl.cmdBuf[curSlave];
			switch(pCmdBuf->transmitState)
			{
				case 0:				// 开始新传输
					InsertCmdToTransferBuf(curSlave);
				/*	pTop = CmdUrgentBuf.pTop;
					pEnd = CmdUrgentBuf.pEnd;
					if(pTop == pEnd){	// 保证紧急命令先发送
						pTop = pCmdBuf->pTop;
						pEnd = pCmdBuf->pEnd;
						if(pTop != pEnd){			// 设置新的指针
							pCmdBuf->transmitState = 1;	// 设置命令处于等待发送状态
							MemCopy(&(pCmdBuf->buf[pTop]),&TransferBuf.cmd, sizeof(COMMAND_STRING));	// 将新命令存入发送缓冲区
							}
						else
							SetTransferBufStateQueryCmd(curSlave);
						}
					else
						SetTransferBufStateQueryCmd(curSlave);*/
					break;
				case 2:				// 命令发送完毕，查询从机状态
					SetTransferBufStateQueryCmd(curSlave);
					break;
				default:
					break;
			}
			TransferBuf.pSend = 0;	// 初始化发送指针
			DeviceInfo.pRec = 0;
		}
	}
}
// 接收一个字节的从机消息存入消息缓冲区
void ReceiveSlaveInfo(unsigned char info)
{
	// 接收从机返回信息
	unsigned char curSlave;
	static unsigned char state;
	static unsigned char checkSum;
	unsigned char pTop, pEnd;
	unsigned char *pChar;

	curSlave = TransmitControl.curSlave;    // 当前从机号
	TransmitControl.waitCount = 15;			// 超时等待计数 5
	// 接收设备消息
	if(DeviceInfo.pRec == 0)
	{
		// 接收状态字节
		CommandBackFlag = info;      //返回到的第一个命令就是状态加上长度
		state = info >> 3;
		DeviceInfo.infoLen = info & 0x07;      //长度是后三位来表示
		if(DeviceInfo.infoLen == 0)	
			DeviceInfo.isValid = 1;
		else                                   //命令长度不是为0，有命令
		{
			DeviceInfo.buf[DeviceInfo.pRec] = info;
			DeviceInfo.pRec ++;
			checkSum = 0;
		}
	}
	else
	{
		// 接收设备返回消息
		if(DeviceInfo.pRec < DeviceInfo.infoLen + 1)
		{
			DeviceInfo.buf[DeviceInfo.pRec] = info;
			DeviceInfo.pRec ++;
			checkSum += info;
		}
		else         //最后一个数据就是校验和的数值
		{
			if(checkSum == info)				// 设备返回消息接收成功
			{
				DeviceInfo.isValid = 1;         //整个消息接受完成了
				Uart2SendDat(STA_SLAVE_RECEIVE_OK);	
			}
			else
			{							
				CommandBackFlag = 254;
				Uart2SendDat(STA_SLAVE_RECEIVE_ERR);
				if(ControlModel != 0)
				{                    //校验和错误报错了
					Uart0ReUnable;
					uart_Printf("// Info err,Num:%d,Sum:%d,%d(%d,%d)\r\n",curSlave,checkSum,info,DeviceInfo.buf[1],DeviceInfo.buf[2]);
					Uart0ReEnable;
				}
				DeviceInfo.infoLen = 0;
				DeviceInfo.pRec = 0;
				TransmitControl.waitCount = 2;
			}
		}
	}
	// 设备消息处理
	if(DeviceInfo.isValid)
	{
		CommandBackFlag = 255;
		switch(state)
		{
			case 0:			// 待机
				TransmitControl.slaveState[curSlave] = state;
				break;
			case 1:			// 空闲
				TransmitControl.slaveState[curSlave] = state;
				if(TransmitControl.cmdBuf[curSlave].transmitState == 2)
				{
					pTop = TransmitControl.cmdBuf[curSlave].pTop;
					pEnd = TransmitControl.cmdBuf[curSlave].pEnd;
					if(pTop != pEnd)
					{
						pTop ++;
						if(pTop == COMMAND_BUF_LEN)
							pTop = 0;
						TransmitControl.cmdBuf[curSlave].pTop = pTop;
					}
					TransmitControl.cmdBuf[curSlave].transmitState = 0;
					Uart0ReUnable;	
					uart_Printf("//CmdBackErr,No:%d\r\n", curSlave);
					Uart0ReEnable;
				}
				break;
			case 2:			// 忙
				TransmitControl.slaveState[curSlave] = state;
				break;
			case 3:			// 命令完成
			case 4:			// 命令出错
				// 发送命令缓冲区指针更新
				pTop = TransmitControl.cmdBuf[curSlave].pTop;
				pEnd = TransmitControl.cmdBuf[curSlave].pEnd;
				if(pTop != pEnd)
				{
					pTop ++;
					if(pTop == COMMAND_BUF_LEN)
						pTop = 0;
					TransmitControl.cmdBuf[curSlave].pTop = pTop;		// 设置新的指针
				}
				TransmitControl.cmdBuf[curSlave].transmitState = 0;		// 新的通讯开始
			case 5:			// 状态发生改变
#ifdef DEBUG
				if(state==4)
				{
					if(ControlModel != 0)
					{
						Uart0ReUnable;
						uart_Printf("//CmdErr,No:%d\r\n", curSlave);
						Uart0ReEnable;
					}
				}
#endif
				if(state != 4)
					if(DeviceInfo.infoLen != 0)		// 空消息不处理
						AddInfoEventToStack(curSlave, &DeviceInfo.buf[1], DeviceInfo.infoLen);	
				break;
			case 6:			// 命令发送成功
				TransmitControl.cmdBuf[curSlave].transmitState = 2;	
				break;
			case 7:			// 命令发送失败
				TransmitControl.cmdBuf[curSlave].transmitState = 0;	// 重新发送命令
				break;
			default:
				break;
			}
		TransmitControl.waitCount = 2;		// 通信结束，退出超时等待
		DeviceInfo.isValid = 0;
		DeviceInfo.infoLen = 0;
		DeviceInfo.pRec = 0;
		}
}

unsigned char RegisterSlave(unsigned char slaveNum)
{
//	if(TransferStart)
//		return;				// 命令传输已启动，不能再注册从机
	TransmitControl.registerSlave[slaveNum] = 1;
}
void StartCommandTransfer(void)
{
	unsigned char i;
	for(i = 0; i < SLAVE_NUM; i++)
	{
		if(TransmitControl.registerSlave[i] != 0)		// 找到第一个从机并设置默认命令，启动传输
		{
			SetTransferBufStateQueryCmd(i);
			TransmitControl.curSlave = i;
			TransferStart = 1;
		}
	}
}
// 新命令
unsigned char InsertCommand(unsigned char slaveNum, COMMAND_STRING * pCmdStr){
	unsigned char pEnd, pTop, p;
	CLI();
	pTop = TransmitControl.cmdBuf[slaveNum].pTop;
	pEnd = TransmitControl.cmdBuf[slaveNum].pEnd;
	p = pEnd; // 1 1 

	pEnd ++; // 1 2
	if(pEnd >= COMMAND_BUF_LEN)
		pEnd = 0;
	
	if(pEnd != pTop) // 1 2 
	{
		MemCopy(pCmdStr, &TransmitControl.cmdBuf[slaveNum].buf[p], sizeof(COMMAND_STRING)); // 1 2
		p++; // 22 
		if(p == COMMAND_BUF_LEN)
			p = 0;
		TransmitControl.cmdBuf[slaveNum].pEnd = p; // 2 2 
	}
	else
	{
		Uart0ReUnable;
		uart_Printf("// Command insert fail [%d]\r\n", slaveNum);
		Uart0ReEnable;
		SEI();
		return 1;
	}
	SEI();
	return 0;
}

unsigned char InsertUrgentCommand(unsigned char slaveNum, COMMAND_STRING * pCmdStr){
/*	unsigned char pEnd, pTop, p;
	pTop = CmdUrgentBuf.pTop;
	pEnd = CmdUrgentBuf.pEnd;
	p = pEnd;
	pEnd ++;
	if(pEnd == COMMAND_BUF_LEN)
		pEnd = 0;
	if(pEnd != pTop){
		MemCopy(pCmdStr, &CmdUrgentBuf.buf[p], sizeof(COMMAND_STRING));
		CmdUrgentBuf.slaveNum[p] = slaveNum;
		CmdUrgentBuf.pEnd = pEnd;
		}
	else{
		uart_Printf("// UrgentCommand insert fail [%d]\r\n", slaveNum);
		return 1;	
		}
	return 0;*/
	CLI();
	if(CmdUrgentBuf.state[slaveNum] == 0){
		MemCopy(pCmdStr, &CmdUrgentBuf.buf[slaveNum], sizeof(COMMAND_STRING));
		CmdUrgentBuf.state[slaveNum] = 1;
		}
	else{
		Uart0ReUnable;
		uart_Printf("// UrgentCommand insert fail [%d]\r\n", slaveNum);
		Uart0ReEnable;
		SEI();
		return 1;
		}
	SEI();
	return 0;
}

unsigned char GetSlaveState(unsigned char slaveNum)
{
	// 获取从机状态
	unsigned char pEnd, pTop;
	pTop = TransmitControl.cmdBuf[slaveNum].pTop;
	pEnd = TransmitControl.cmdBuf[slaveNum].pEnd;
	if(pTop == pEnd)		// 如果命令队列空，返回记录的电机状态
		return TransmitControl.slaveState[slaveNum];
	else					// 如果命令队列不空，返回忙状态
		return STA_SLAVE_BUSY;
}

static unsigned char SendEventCritical = 0;		// 临界值保护防函数重入
void SendInfoEvent(void)
{
	// 向模块的上层应用发送信息事件，定时调用
	unsigned char pTop;
	
	if(SendEventCritical)
		return;
	SendEventCritical = 1;
	pTop = InfoEventStack.pTop;
	if(pTop != InfoEventStack.pEnd)
	{
		// 发送消息事件
		SlaveEventAssignProcess(&InfoEventStack.info[pTop]);
		pTop ++;
		if(pTop == INFO_EVENT_NUM)
			pTop = 0;
		InfoEventStack.pTop = pTop;
	}
	SendEventCritical = 0;
}


/********************************************* File end **********************************************/

