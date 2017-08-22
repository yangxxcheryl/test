
#include <iom1280v.h>
#include <macros.h>
#include "B1404_LIB.h"
#include "LibCommon.h"



/************************* �ӻ�������� **************************/
typedef struct _COMMAND_BUF
{
	COMMAND_STRING buf[COMMAND_BUF_LEN];
	unsigned char pTop;
	unsigned char pEnd;
	unsigned char transmitState;	// �����״̬ 0�Ѿ�ִ����ϻ�����1���������ȴ��ͣ� 2�������ѷ��ʹ���ִ��״̬
}COMMAND_BUF;

// ����Ϳ���
typedef struct _COMMAND_TRANSMIT_CONTROL
{
	unsigned char registerSlave[SLAVE_NUM];	// ʹ�õĴӻ�������Ǽ�
	unsigned char curSlave;					// ��ǰ��ͨ�ŵĴӻ�
	unsigned char waitCount;				// ��Ϣ���س�ʱ��ʱ
	COMMAND_BUF cmdBuf[SLAVE_NUM];			// ������飬ÿ��Ԫ���±��ʾ��Ӧ�Ĵӻ����
	unsigned char slaveState[SLAVE_NUM];	// ��¼ÿ���ӻ���״̬
}COMMAND_TRANSMIT_CONTROL;

// ��ǰ����ͻ�����
typedef struct _TRANSFER_BUF
{
	COMMAND_STRING cmd;
	unsigned char pSend;					// �������ݷ���ָ��
}TRANSFER_BUF;

/*typedef struct _COMMAND_URGENT_BUF{
	COMMAND_STRING buf[COMMAND_BUF_LEN];
	unsigned char slaveNum[COMMAND_BUF_LEN];	// �����Ӧ�Ĵӻ���
	unsigned char pTop;
	unsigned char pEnd;
}COMMAND_URGENT_BUF;*/
typedef struct _COMMAND_URGENT_BUF{
	COMMAND_STRING buf[SLAVE_NUM];	// �����±��Ӧ�ӻ���
	unsigned char state[SLAVE_NUM];	// ������Ч״̬, 0:��Ч��1:��Ч
//	unsigned char pTop;
//	unsigned char pEnd;
}COMMAND_URGENT_BUF;


/*************************** ��Ϣ�¼��� *****************************/

// ��Ϣ�¼�����
typedef struct _INFO_EVENT_STACK{	// ��Ϣ�¼�����
	unsigned char pTop;
	unsigned char pEnd;
	INFO_EVENT info[INFO_EVENT_NUM];
}INFO_EVENT_STACK;
/*************************** �����¼��� *****************************
typedef struct _ERROR_EVENT{
	unsigned char slaveNum;				// �ӻ����
	unsigned char error;				// ������Ϣ
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
unsigned char TransferStart;		// �����������ʶ
DEVICE_INFO DeviceInfo;
COMMAND_URGENT_BUF CmdUrgentBuf;	// ��������
extern unsigned char ControlModel;			// 0:������ 1:�����ն˵���

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


// ���յ��Ĵӻ���Ϣ������Ϣ���еȴ�����
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
		InfoEventStack.info[pEnd].event = *(info ++);	// ��Ϣ����
		len --;
		InfoEventStack.info[pEnd].infoLen = len;	// ��Ϣ��������
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
			MemCopy(&(CmdUrgentBuf.buf[pTop]),&TransferBuf.cmd, sizeof(COMMAND_STRING));	// ����������뷢�ͻ�����
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
		// �н�������
		MemCopy(&(CmdUrgentBuf.buf[slaveNum]),&TransferBuf.cmd, sizeof(COMMAND_STRING));	// ����������뷢�ͻ�����
		CmdUrgentBuf.state[slaveNum] = 0;	// �����Ѵ������
	}
	else	// �޽���������Ϳ��в�ѯ
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
		// �н�������
		MemCopy(&(CmdUrgentBuf.buf[slaveNum]),&TransferBuf.cmd, sizeof(COMMAND_STRING));	// ����������뷢�ͻ�����
		CmdUrgentBuf.state[slaveNum] = 0;	// �����Ѵ������
	}
	else	// �޽�������
	{
		pCmdBuf = &TransmitControl.cmdBuf[slaveNum];
		pTop = pCmdBuf->pTop;
		pEnd = pCmdBuf->pEnd;
		if(pTop != pEnd)			// �����µ�ָ��
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
static unsigned char CommandBackFlag;	// ����ͷ��ر�ʶ�����û�յ�������Ϣ��ʾ�ӻ���ʧ
// ���������ֽ�
void SendCommandData(void)
{
	// ������������
	unsigned char curSlave;
	unsigned char p,l,c;
	unsigned char pTop, pEnd;
	static unsigned char checkSum;
	static unsigned char cmdReSendCnt = 0;
	COMMAND_BUF * pCmdBuf;

	if(TransferStart == 0)
		return;				// �����δ����
	
	curSlave = TransmitControl.curSlave;
	pTop = TransferBuf.pSend;
	pEnd = TransferBuf.cmd.cmdLen;
	
	if(pTop < pEnd)
	{
		// ��������
		if(pTop == 0)
		{
			if(cmdReSendCnt == 0)
			{
				Uart2SendAdd(0);
				cmdReSendCnt = 1;
				return;	// ��ַ�ط�
			}
			Uart2SendAdd(TransferBuf.cmd.cmd);		// ���͵�ַ������
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
			Uart2SendDat(c);	// ���Ͳ���
			checkSum += c;
		}
		TransferBuf.pSend ++;	// ���ݷ���ָ���һ
	}
	else if(pTop == pEnd)
	{
		// ����У��
		if(GetUart2DataSendResult()!=0)
		{
			pTop = 0;
			Uart0ReUnable;
			uart_Printf("//DataSendErr2,No:%d\r\n", curSlave);
			Uart0ReEnable;
		}
		if(pEnd > 1)
			Uart2SendDat(checkSum);	// ����У���
		TransferBuf.pSend ++;	// ���ݷ���ָ���һ
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
		/*	if(CommandBackFlag != 255){	// ǰһ�η��͵�����δ�յ�������Ϣ����Ҫ���·���
			//	TransmitControl.cmdBuf[curSlave].transmitState = 2;	// ���·�������
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
				
			TransmitControl.curSlave = curSlave;		// ָ����һ����Ч�Ĵӻ�
			pCmdBuf = &TransmitControl.cmdBuf[curSlave];
			switch(pCmdBuf->transmitState)
			{
				case 0:				// ��ʼ�´���
					InsertCmdToTransferBuf(curSlave);
				/*	pTop = CmdUrgentBuf.pTop;
					pEnd = CmdUrgentBuf.pEnd;
					if(pTop == pEnd){	// ��֤���������ȷ���
						pTop = pCmdBuf->pTop;
						pEnd = pCmdBuf->pEnd;
						if(pTop != pEnd){			// �����µ�ָ��
							pCmdBuf->transmitState = 1;	// ��������ڵȴ�����״̬
							MemCopy(&(pCmdBuf->buf[pTop]),&TransferBuf.cmd, sizeof(COMMAND_STRING));	// ����������뷢�ͻ�����
							}
						else
							SetTransferBufStateQueryCmd(curSlave);
						}
					else
						SetTransferBufStateQueryCmd(curSlave);*/
					break;
				case 2:				// �������ϣ���ѯ�ӻ�״̬
					SetTransferBufStateQueryCmd(curSlave);
					break;
				default:
					break;
			}
			TransferBuf.pSend = 0;	// ��ʼ������ָ��
			DeviceInfo.pRec = 0;
		}
	}
}
// ����һ���ֽڵĴӻ���Ϣ������Ϣ������
void ReceiveSlaveInfo(unsigned char info)
{
	// ���մӻ�������Ϣ
	unsigned char curSlave;
	static unsigned char state;
	static unsigned char checkSum;
	unsigned char pTop, pEnd;
	unsigned char *pChar;

	curSlave = TransmitControl.curSlave;    // ��ǰ�ӻ���
	TransmitControl.waitCount = 15;			// ��ʱ�ȴ����� 5
	// �����豸��Ϣ
	if(DeviceInfo.pRec == 0)
	{
		// ����״̬�ֽ�
		CommandBackFlag = info;      //���ص��ĵ�һ���������״̬���ϳ���
		state = info >> 3;
		DeviceInfo.infoLen = info & 0x07;      //�����Ǻ���λ����ʾ
		if(DeviceInfo.infoLen == 0)	
			DeviceInfo.isValid = 1;
		else                                   //����Ȳ���Ϊ0��������
		{
			DeviceInfo.buf[DeviceInfo.pRec] = info;
			DeviceInfo.pRec ++;
			checkSum = 0;
		}
	}
	else
	{
		// �����豸������Ϣ
		if(DeviceInfo.pRec < DeviceInfo.infoLen + 1)
		{
			DeviceInfo.buf[DeviceInfo.pRec] = info;
			DeviceInfo.pRec ++;
			checkSum += info;
		}
		else         //���һ�����ݾ���У��͵���ֵ
		{
			if(checkSum == info)				// �豸������Ϣ���ճɹ�
			{
				DeviceInfo.isValid = 1;         //������Ϣ���������
				Uart2SendDat(STA_SLAVE_RECEIVE_OK);	
			}
			else
			{							
				CommandBackFlag = 254;
				Uart2SendDat(STA_SLAVE_RECEIVE_ERR);
				if(ControlModel != 0)
				{                    //У��ʹ��󱨴���
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
	// �豸��Ϣ����
	if(DeviceInfo.isValid)
	{
		CommandBackFlag = 255;
		switch(state)
		{
			case 0:			// ����
				TransmitControl.slaveState[curSlave] = state;
				break;
			case 1:			// ����
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
			case 2:			// æ
				TransmitControl.slaveState[curSlave] = state;
				break;
			case 3:			// �������
			case 4:			// �������
				// �����������ָ�����
				pTop = TransmitControl.cmdBuf[curSlave].pTop;
				pEnd = TransmitControl.cmdBuf[curSlave].pEnd;
				if(pTop != pEnd)
				{
					pTop ++;
					if(pTop == COMMAND_BUF_LEN)
						pTop = 0;
					TransmitControl.cmdBuf[curSlave].pTop = pTop;		// �����µ�ָ��
				}
				TransmitControl.cmdBuf[curSlave].transmitState = 0;		// �µ�ͨѶ��ʼ
			case 5:			// ״̬�����ı�
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
					if(DeviceInfo.infoLen != 0)		// ����Ϣ������
						AddInfoEventToStack(curSlave, &DeviceInfo.buf[1], DeviceInfo.infoLen);	
				break;
			case 6:			// ����ͳɹ�
				TransmitControl.cmdBuf[curSlave].transmitState = 2;	
				break;
			case 7:			// �����ʧ��
				TransmitControl.cmdBuf[curSlave].transmitState = 0;	// ���·�������
				break;
			default:
				break;
			}
		TransmitControl.waitCount = 2;		// ͨ�Ž������˳���ʱ�ȴ�
		DeviceInfo.isValid = 0;
		DeviceInfo.infoLen = 0;
		DeviceInfo.pRec = 0;
		}
}

unsigned char RegisterSlave(unsigned char slaveNum)
{
//	if(TransferStart)
//		return;				// �������������������ע��ӻ�
	TransmitControl.registerSlave[slaveNum] = 1;
}
void StartCommandTransfer(void)
{
	unsigned char i;
	for(i = 0; i < SLAVE_NUM; i++)
	{
		if(TransmitControl.registerSlave[i] != 0)		// �ҵ���һ���ӻ�������Ĭ�������������
		{
			SetTransferBufStateQueryCmd(i);
			TransmitControl.curSlave = i;
			TransferStart = 1;
		}
	}
}
// ������
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
	// ��ȡ�ӻ�״̬
	unsigned char pEnd, pTop;
	pTop = TransmitControl.cmdBuf[slaveNum].pTop;
	pEnd = TransmitControl.cmdBuf[slaveNum].pEnd;
	if(pTop == pEnd)		// ���������пգ����ؼ�¼�ĵ��״̬
		return TransmitControl.slaveState[slaveNum];
	else					// ���������в��գ�����æ״̬
		return STA_SLAVE_BUSY;
}

static unsigned char SendEventCritical = 0;		// �ٽ�ֵ��������������
void SendInfoEvent(void)
{
	// ��ģ����ϲ�Ӧ�÷�����Ϣ�¼�����ʱ����
	unsigned char pTop;
	
	if(SendEventCritical)
		return;
	SendEventCritical = 1;
	pTop = InfoEventStack.pTop;
	if(pTop != InfoEventStack.pEnd)
	{
		// ������Ϣ�¼�
		SlaveEventAssignProcess(&InfoEventStack.info[pTop]);
		pTop ++;
		if(pTop == INFO_EVENT_NUM)
			pTop = 0;
		InfoEventStack.pTop = pTop;
	}
	SendEventCritical = 0;
}


/********************************************* File end **********************************************/

