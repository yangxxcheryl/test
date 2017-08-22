

#ifndef _LIB_COMMON_H
#define _LIB_COMMON_H

void TWI_Init(void);

// ����ͨѶ
#define UARTBUF_TXD_LEN	500
#define UARTBUF_RXD_LEN	1

typedef struct _UARTBUF{
	unsigned char buffer[UARTBUF_TXD_LEN];// ������
	unsigned int pEnd;					// ����������βָ��
	unsigned int pHead;					// ����������ͷָ��
	unsigned int remanentLen;			// ʣ��Ŀ��ó���
}UARTBUF_TXD;

// ���ڻ����������ṹ
typedef struct {
	unsigned char buf[UARTBUF_RXD_LEN];// �������ݻ�����
	unsigned char pEnd;					// ����������βָ��
	unsigned char pHead;				// ����������ͷָ��
}UARTBUF_RXD;

void uartDataInit(void);
void uart0DataReceive(unsigned char dat);

void uart0Transfer(void);
// ����ͨѶ [End]

// Uart2	RS485������ӻ�ͨѶ
unsigned char GetUart2DataSendResult(void);
void Uart2SendAdd(unsigned char add);
void Uart2SendDat(unsigned char dat);

// Uart3	RS485������ӻ�ͨѶ
void Uart3SendAdd(unsigned char add);
void Uart3SendDat(unsigned char dat);

// �ֻ�����
unsigned char InsertCommand(unsigned char slaveNum, COMMAND_STRING * pCmdStr);
//unsigned char InsertUrgentCommand(unsigned char slaveNum, COMMAND_STRING * pCmdStr);
unsigned char GetSlaveState(unsigned char slaveNum);
void ReceiveSlaveInfo(unsigned char info);


unsigned char InsertUrgentCommand(unsigned char slaveNum, COMMAND_STRING * pCmdStr);
void SendCommandData(void);

#endif