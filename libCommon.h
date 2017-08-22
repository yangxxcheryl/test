

#ifndef _LIB_COMMON_H
#define _LIB_COMMON_H

void TWI_Init(void);

// 串口通讯
#define UARTBUF_TXD_LEN	500
#define UARTBUF_RXD_LEN	1

typedef struct _UARTBUF{
	unsigned char buffer[UARTBUF_TXD_LEN];// 缓冲区
	unsigned int pEnd;					// 缓冲区数据尾指针
	unsigned int pHead;					// 缓冲区数据头指针
	unsigned int remanentLen;			// 剩余的可用长度
}UARTBUF_TXD;

// 串口缓冲数据区结构
typedef struct {
	unsigned char buf[UARTBUF_RXD_LEN];// 串口数据缓冲区
	unsigned char pEnd;					// 缓冲区数据尾指针
	unsigned char pHead;				// 缓冲区数据头指针
}UARTBUF_RXD;

void uartDataInit(void);
void uart0DataReceive(unsigned char dat);

void uart0Transfer(void);
// 串口通讯 [End]

// Uart2	RS485总线与从机通讯
unsigned char GetUart2DataSendResult(void);
void Uart2SendAdd(unsigned char add);
void Uart2SendDat(unsigned char dat);

// Uart3	RS485总线与从机通讯
void Uart3SendAdd(unsigned char add);
void Uart3SendDat(unsigned char dat);

// 分机命令
unsigned char InsertCommand(unsigned char slaveNum, COMMAND_STRING * pCmdStr);
//unsigned char InsertUrgentCommand(unsigned char slaveNum, COMMAND_STRING * pCmdStr);
unsigned char GetSlaveState(unsigned char slaveNum);
void ReceiveSlaveInfo(unsigned char info);


unsigned char InsertUrgentCommand(unsigned char slaveNum, COMMAND_STRING * pCmdStr);
void SendCommandData(void);

#endif