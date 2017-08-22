#include <iom1280v.h>
#include <macros.h>
#include "B1404_LIB.h"
#include "LibCommon.h"
#include "Common.h"


extern unsigned char ControlModel;			// 0:������ 1:�����ն˵���


//void uartDataInit(void);
//void uart0DataReceive(unsigned char dat);

UARTBUF_TXD		uart0TxdBuf;	// �������ݻ�����
UARTBUF_RXD 	uart0RxdBuf;
unsigned char 	uartBufProtect = 0;

unsigned char checkFlag = 0;

void port_init(void)
{
	PORTA = 0x00;
	DDRA  = 0x00;
	PORTB = 0x80;
	DDRB  = 0x00;
	PORTC = 0x00; //m103 output only
	DDRC  = 0x00;
	PORTD = 0xFF;
	//DDRD  = 0x00;
	DDRD  = 0x80;
	PORTE = 0x33;
	DDRE  = 0x32;
	PORTF = 0x00;
	DDRF  = 0x03;
	PORTG = 0x00;
	DDRG  = 0x00;
	PORTH = 0x03;
	DDRH  = 0x0c;
	PORTJ = 0x03;
	DDRJ  = 0x0c;
	PORTK = 0x0f;
	DDRK  = 0x00;
	PORTL = 0x00;
	DDRL  = 0x00;
}

//TIMER0 initialize - prescale:64
// WGM: Normal
// desired value: 8KHz
// actual value: 8.229KHz (2.8%)
void timer0_init(void){
	TCCR0B = 0x00; //stop
	TCNT0 = 0x8d;//0xc7;//0xE4; //set count
	OCR0A  = 0xc6;//0x39;//0x1C;
	TCCR0A = 0x40; //start timer
	TCCR0B = 0x03; //start timer
}


#pragma interrupt_handler timer0_ovf_isr:iv_TIM0_OVF
void timer0_ovf_isr(void)
{
	// OCR0A  = 0x04;
	SendCommandData();
	uart0Transfer();		// ����0�����Զ�����
	SendInfoEvent();
	
//	CardTrolleyTurnProcess();
	
	Beep();
	// 0xb4 3k, 0x8d 2k, 0x67 1k5, 0x1a 1k
	TCNT0 = 0x8d;//0xc7;//0x01; //reload counter value
}

//TIMER1 initialize - prescale:256
// WGM: 0) Normal, TOP=0xFFFF
// desired value: 100mSec
// actual value: 99.983mSec (0.0%)
void timer1_init(void){
	TCCR1B = 0x00; //stop
	TCNT1H = 0xE9; //setup
	TCNT1L = 0x81;
	OCR1AH = 0x16;
	OCR1AL = 0x7F;
	OCR1BH = 0x16;
	OCR1BL = 0x7F;
	OCR1CH = 0x00;
	OCR1CL = 0x00;
	ICR1H  = 0x16;
	ICR1L  = 0x7F;
	TCCR1A = 0x00;
	TCCR1C = 0x00;
	TCCR1B = 0x04; //start Timer
}

unsigned int DelayCount[16];
void SetDelayTime(unsigned char num, unsigned int t){
	// 100mS
	if(num>15)
		return;
	DelayCount[num] = t;
}
unsigned char WaitDelayTime(unsigned char num){
	if(num>15)
		return 0;
	if(DelayCount[num] == 0)
		return 0;
	else
		return 1;
}
extern unsigned long SecondCount;
#pragma interrupt_handler timer1_ovf_isr:iv_TIM1_OVF
void timer1_ovf_isr(void){
	unsigned char n;
	static unsigned char i,j;
	//TIMER1 has overflowed
	TCNT1H = 0xE9; //reload counter high value
	TCNT1L = 0x81; //reload counter low value

	for(n=0; n<16; n++){		// Ϊÿ����ʱ������ʱ
		if(DelayCount[n] != 0)
			DelayCount[n] --;	// ����ֵ������ʱֹͣ
		}
//	CardTrolleyTurnProcess();
	i++;
	j++;
	if(i>9)
	{
		SecondCount ++;
		i = 0;
		// ��Ƭ�ֹ��ܿ���
		if(GetwasteCardState() == 0)	
			TrashCanMonitor();
	}
	if(j > 4)
	{
		j = 0;
		PORTD ^= (1 << 7);
	}
}
//TIMER4 initialize - prescale:1
// WGM: 7) PWM 10bit fast, TOP=0x03FF
// desired value: 10KHz
// actual value: 14.400KHz (30.6%)
void timer4_init(void)
{
 TCCR4B = 0x00; //stop
 TCNT4H = 0xFC; //setup
 TCNT4L = 0x01;
 OCR4AH = 0x03;
 OCR4AL = 0xFF;
 OCR4BH = 0x03;
 OCR4BL = 0xFF;
 OCR4CH = 0x03;
 OCR4CL = 0xFF;
 ICR4H  = 0x03;
 ICR4L  = 0xFF;
 TCCR4A = 0x2B; //0x2B;	//0x3F;
 TCCR4C = 0x00;
 TCCR4B = 0x09; //start Timer
}

//TIMER5 initialize - prescale:1
// WGM: 7) PWM 10bit fast, TOP=0x03FF
// desired value: 10KHz
// actual value: 14.400KHz (30.6%)
void timer5_init(void)
{
 TCCR5B = 0x00; //stop
 TCNT5H = 0xFC; //setup
 TCNT5L = 0x01;
 OCR5AH = 0x03;
 OCR5AL = 0xFF;
 OCR5BH = 0x03;
 OCR5BL = 0xFF;
 OCR5CH = 0x03;
 OCR5CL = 0xFF;
 ICR5H  = 0x03;
 ICR5L  = 0xFF;
 TCCR5A = 0x2B; //0x2B;	//0x3F;
 TCCR5C = 0x00;
 TCCR5B = 0x09; //start Timer
}

//UART0 initialize
// desired baud rate: 115200
// actual: baud rate:115200 (0.0%) 
// char size: 8 bit
// parity: Disabled
void uart0_init(void){
	UCSR0B = 0x00; //disable while setting baud rate
	UCSR0A = 0x00;
	UCSR0C = 0x06;
	//UBRR0L = 0x07; //set baud rate lo
	UBRR0L = 0X0F;
	UBRR0H = 0x00; //set baud rate hi
	UCSR0B = 0x98;//0xD8;
}

unsigned char HexToBin(unsigned char c){
	unsigned char n=0;
	if(c>='0' && c<='9')
		n = c-0x30;
	if(c>='a' && c<='f')
		n = c-'a' + 10;
	if(c>='A' && c<='F')
		n = c-'A' + 10;
	return n;
	
}
//unsigned char CmdBuf[30];
//unsigned char CmdLen=0;
//unsigned char rxBuf[60];
//unsigned char rxPnt=0;
#pragma interrupt_handler uart0_rx_isr:iv_USART0_RXC
void uart0_rx_isr(void){
 //uart has received a character in UDR
	unsigned char dat;
	dat = UDR0;
	uart0DataReceive(dat);
}

#pragma interrupt_handler uart0_tx_isr:iv_USART0_TXC
void uart0_tx_isr(void){
	//character has been transmitted

}

//UART1 initialize
// desired baud rate:9600
// actual baud rate:9600 (0.0%)
// char size: 8 bit
// parity: Disabled
void uart1_init(void){
	UCSR1B = 0x00; //disable while setting baud rate
	UCSR1A = 0x00;
	UCSR1C = 0x06;
	UBRR1L = 0x5F; //set baud rate lo
	UBRR1H = 0x00; //set baud rate hi
	UCSR1B = 0x98;
}

#pragma interrupt_handler uart1_rx_isr:iv_USART1_RXC
void uart1_rx_isr(void){
 //uart has received a character in UDR
	
}

//UART2 initialize
// desired baud rate:115200
// actual baud rate:115200 (0.0%)
// char size: 9 bit
// parity: Disabled
void uart2_init(void){
	UCSR2B = 0x00; //disable while setting baud rate
	UCSR2A = 0x00;
	UCSR2C = 0x06;
	UBRR2L = 0x07; //set baud rate lo	115200
	UBRR2H = 0x00; //set baud rate hi
	UCSR2B = 0xDC;//0xDC;
}

unsigned char _Uart2DetectorSelfFlag = 0;	// ���ݷ��ͱ��, ����������ⷢ������
unsigned char _Uart2DetectorSelfData;		// ����������⵽������
unsigned char _Uart2CurrentSendData;		// ��ǰ���͵�����
unsigned char _Uart2DataSendResult = 0;
/*void SetUart2DetectorSelf(void){
	_Uart2DetectorSelfFlag = 1;
}
unsigned char GetUart2DetectorSelfData(void){
	return _Uart2DetectorSelfData;
}*/
unsigned char GetUart2DataSendResult(void){
	return _Uart2DataSendResult;
}

#pragma interrupt_handler uart2_rx_isr:iv_USART2_RXC
void uart2_rx_isr(void)
{
	//uart has received a character in UDR
	unsigned char dat;
	dat = UDR2;
	if(_Uart2DetectorSelfFlag)
	{
		_Uart2DetectorSelfData = dat;
		_Uart2DetectorSelfFlag = 0;
		if(_Uart2DetectorSelfData == _Uart2CurrentSendData)
			_Uart2DataSendResult = 0;
		else
			_Uart2DataSendResult = 1;
	}
	else
		ReceiveSlaveInfo(dat);
}

#pragma interrupt_handler uart2_tx_isr:iv_USART2_TXC
void uart2_tx_isr(void){
	//character has been transmitted
	PORTH = 0x03;
	PORTH = 0x03;
//	BeepNum = 2;
}
void Uart2SendAdd(unsigned char add)
{
	PORTH = 0x0b;	//0x0f;
	_Uart2DetectorSelfFlag = 1;
	while(!(UCSR2A & 0x20));
	UCSR2B |= 0x01;
	UDR2 = add;
	_Uart2CurrentSendData = add;
}
void Uart2SendDat(unsigned char dat)
{
	PORTH = 0x0b;	//0x0f;
	_Uart2DetectorSelfFlag = 1;
	while(!(UCSR2A & 0x20));
	UCSR2B &= 0xFE;
	UDR2 = dat;
	_Uart2CurrentSendData = dat;
}

//UART3 initialize
// desired baud rate:115200
// actual baud rate:115200 (0.0%)
// char size: 9 bit
// parity: Disabled
void uart3_init(void){
	UCSR3B = 0x00; //disable while setting baud rate
	UCSR3A = 0x00;
	UCSR3C = 0x06;
	UBRR3L = 0x07; //set baud rate lo
//	UBRR3L = 0x0f; //set baud rate lo
	UBRR3H = 0x00; //set baud rate hi
	UCSR3B = 0xDC;//0xDC;
}

#pragma interrupt_handler uart3_rx_isr:iv_USART3_RXC
void uart3_rx_isr(void){
	//uart has received a character in UDR
	unsigned char dat;
	while(!(UCSR3A & 0x80));
	dat = UDR3;
	while(!(UCSR0A & 0x20));
	UDR0 = dat;
//	dat = dat;
	//BeepNum = 2;
}

#pragma interrupt_handler uart3_tx_isr:iv_USART3_TXC
void uart3_tx_isr(void){
	//character has been transmitted
	PORTJ = 0x03;
	PORTJ = 0x03;
}
void Uart3SendAdd(unsigned char add){
	PORTJ = 0x0f;
	while(!(UCSR2A & 0x20));
	UCSR3B |= 0x01;
	UDR3 = add;
}
void Uart3SendDat(unsigned char dat){
	PORTJ = 0x0f;
	while(!(UCSR3A & 0x20));
	UCSR3B &= 0xFE;
	UDR3 = dat;
}

#pragma interrupt_handler pcint2_isr:iv_PCINT2
void pcint2_isr(void){
	//pin change interrupt 2
}

//ADC initialize
// Conversion time: 112uS
void adc_init(void){
	ADCSRA = 0x00; //disable adc
	ADMUX = 0x42;  //select adc input 2
	ACSR  = 0x80;
	ADCSRB = 0x00;
	ADCSRA = 0xEF;
}

void SetNextADChannel(unsigned char num)
{
	switch(num)
	{
		case NeedleChannel:	ADCSRB = 0X08;ADMUX = 0X42;break;	 // Next ADC 10
		case LoadChannel:	ADCSRB = 0X08;ADMUX = 0X43;break;	 // Next ADC 11
		case UnloadChannel: ADCSRB = 0X00;ADMUX = 0X42;break;	 // Next ADC 02
	//	case NeedleChannel:	ADCSRB = 0X08;ADMUX = 0X43;break;	 // ADC 2
	//	case LoadChannel:	ADCSRB = 0X00;ADMUX = 0X42;break;	 // ADC 11
	//	case UnloadChannel: ADCSRB = 0X08;ADMUX = 0X42;break;	 // ADC 10 
		default:break;
	}
}

unsigned int AdcResult[3];		// 0 NeedleChannel,1 LoadChannel, 2 UnloadChannel 
#define _ADC_DIFF_BUF_LEN 20
signed int _Adc_DiffBuf[_ADC_DIFF_BUF_LEN];
signed int _Adc_Wave = 0;
unsigned char _Adc_DiffBufPnt=0;

unsigned int getLiqDetADC(unsigned char num)
{
	return AdcResult[num];
}
unsigned char CheckLiqDetBase(void)
{
	if(AdcResult[NeedleChannel] > 1015)	// Һ��̽��缫δ����
	{		
		SetBeepWarning();
		Uart0ReUnable;
		uart_Printf("!2901 $%d\r\n", AdcResult[NeedleChannel]);		// Һ��̽��缫δ����, ����Һ��̽��缫�����Ƿ����
		Uart0ReEnable;
		return 2;
	}
	else if(AdcResult[NeedleChannel] < 400) // Һ��̽��缫����Ӱ���
	{
		SetBeepWarning();
		Uart0ReUnable;
		uart_Printf("!2902 $%d\r\n", AdcResult[NeedleChannel]);	// Һ��̽��缫��Ӱ�죬����ȡ�����ͷ���Լ���ϴͷ���Ƿ�����Ⱦ��
		Uart0ReEnable;
		return 1;
	}
	
	return 0;
}
signed int GetLiqDetDiff(void){
	return _Adc_Wave;
}
unsigned char GetLiqDetResult(signed int ref){
	if(ref>0){
		if(_Adc_Wave > ref){
			Uart0ReUnable;
			uart_Printf("// Liquid detect: %d\r\n", _Adc_Wave);
			Uart0ReEnable;
			return 1;
			}
		}
	else{
		if(_Adc_Wave < ref){
			Uart0ReUnable;
			uart_Printf("// Liquid detect: %d\r\n", _Adc_Wave);
			Uart0ReEnable;
			return 1;
			}
		}
	return 0;
}

#pragma interrupt_handler adc_isr:iv_ADC
void adc_isr(void)
{
	unsigned int tmp;
	static signed int adc_change = 0;	
	tmp = ADCL;            //Read 8 low bits first (important)
	tmp  += ((unsigned int)ADCH << 8); //read 2 high bits and shift into top byte
	if(tmp > AdcResult[adc_change])
	{
		AdcResult[adc_change] ++;
	}
	else if(tmp < AdcResult[adc_change])
	{
		AdcResult[adc_change] --;
	}
	adc_change++;
	if(adc_change > 2)
		adc_change = 0;
	SetNextADChannel(adc_change);
}

//call this routine to initialize all peripherals
void init_devices(void){
	//stop errant interrupts until set up
	CLI(); //disable all interrupts
//	XMCRA = 0x98; //external memory
//	XMCRB = 0x00; //external memory
	uartDataInit();
	port_init();
	timer0_init();
	timer1_init();
	timer4_init();
	timer5_init();
	uart0_init();
//	uart1_init();
	uart2_init();
//	uart3_init();
	adc_init();
	TWI_Init();

	MCUCR  = 0x00;
	EICRA  = 0x00; //pin change int edge 0:3
	EICRB  = 0x00; //pin change int edge 4:7
	// PCICR  = 0x04; //pin change int enable
	PCMSK0 = 0x00; //pin change mask
	PCMSK1 = 0x00; //pin change mask
	PCMSK2 = 0x00; //pin change mask
	EIMSK  = 0x00;
	TIMSK0 = 0x01; //timer0 interrupt sources
	TIMSK1 = 0x01; //timer1 interrupt sources
	TIMSK2 = 0x00; //timer2 interrupt sources
	TIMSK3 = 0x00; //timer3 interrupt sources
	TIMSK4 = 0x00; //timer4 interrupt sources
	TIMSK5 = 0x00; //timer5 interrupt sources
	PRR0   = 0x00;
	PRR1   = 0x00;
 
	SEI(); //re-enable interrupts
	//all peripherals are now initialized
}

///////////////// Uart1 buffer /////////////////////
void uartDataInit(void){
	unsigned int i;
	unsigned char *p0, *p1;

	p0 = (unsigned char *)(&uart0RxdBuf);
	for(i=0; i<sizeof(UARTBUF_RXD); i++){
		*(p0++) = 0;
		}
	p0 = (unsigned char *)(&uart0TxdBuf);
	for(i=0; i<sizeof(UARTBUF_TXD); i++){
		*(p0++) = 0;
		}
	uart0TxdBuf.remanentLen = UARTBUF_TXD_LEN-1;
}

// 0x15(NAK),	0x06(ACK),	0x02(STX),	0x03(ETX)
static unsigned char PacketState=0;
//static unsigned char UartReceiveBuf[60];	// ���������մ��ڽ��ջ���
unsigned char UartReceiveBuf[60];
static unsigned char PRecBuf;
static unsigned char WaitAnswer;
void CommandExplain(unsigned char c);

void uart0DataReceive(unsigned char dat)
{
	unsigned char i;
	static unsigned char checkSum,	checkLB, checkHB;	// У���
	switch(PacketState)
	{
		case 0:
			if(WaitAnswer)
			{
				if(dat == ACK)
				{
					WaitAnswer = 0;
					PacketState = 13;
				}
				else
				{	
					uart0SendChar(0x0F);	// �ȴ�ACK������,���յ���ACK����
					WaitAnswer = 0;
				}
				break;
			}
			else
			{
				if(dat == STX)
				{
					PacketState = 2;
					checkSum = 0;
					PRecBuf = 0;
					UartReceiveBuf[PRecBuf] = STX;
					CommandExplain(0x0d);	// ������ǰ����	
					break;
				}
				if(dat < 128)
					CommandExplain(dat);	// �������
			}
			break;
		case 1:
			if(dat == STX)
			{
				PacketState = 2;
				checkSum = 0;
				PRecBuf = 0;
				UartReceiveBuf[PRecBuf] = STX;
				CommandExplain(0x0d);	// ������ǰ����	
			}
			break;
		case 2:			// �����������ݰ�״̬
			if(dat == ETX)
			{
				PRecBuf ++;
				UartReceiveBuf[PRecBuf] = ETX;
				PacketState = 10;
			}
			else if(dat == STX)		// ���¿�ʼ����
			{
				checkSum = 0;
				PRecBuf = 0;
				UartReceiveBuf[PRecBuf] = STX;
				CommandExplain(0x0d);	// ������ǰ����	
				break;
			}
			else 				// ��������
			{
				PRecBuf ++;
				UartReceiveBuf[PRecBuf] = dat;
				checkSum += dat;
			}
			break;
		case 10:	// ���յ�һ�ֽ�У��
			PRecBuf ++;
			UartReceiveBuf[PRecBuf] = dat;
			checkLB = dat - 'A';
			PacketState = 11;
			break;
		case 11:	// ���յڶ��ֽ�У��
			checkHB = dat - 'A';
			checkHB <<= 4;
			PRecBuf ++;
			UartReceiveBuf[PRecBuf] = dat;
			UartReceiveBuf[PRecBuf + 1] = 0;
			if(checkSum == checkLB + checkHB)
			{
				uart0SendChar(ACK);
				PacketState = 12;
				PRecBuf = 0;
			}
			else
			{
				PacketState = 14; //PacketState = 1;
				PRecBuf = 0;
			}
			break;
		case 12:	// ������յ�������,�ŵ����ݷ��Ͷ�ʱ�������д���
		case 13:
		case 14:
			uart0SendChar(0X0E);
			break;
		default:
			break;
	}
}

void uart0Transfer(void)
{
// ����0����1�ֽ�����
	//static unsigned int  pTextStart;	// ���ݰ���ʼָ��
	//static unsigned int  pTransfer;	// ���ݷ���ָ��
	//static unsigned char checkSum,	checkLB, checkHB;	// У���
	//static unsigned char reTransferCnt;	// �ط�����
	unsigned int pTop, pEnd;
	unsigned char c;
	static unsigned int timeOut;
	if(uartBufProtect )		// ���ͻ��汣��
		return;
	switch(PacketState){
		case 0:		// ��ͨģʽ
			pTop = uart0TxdBuf.pHead;
			pEnd = uart0TxdBuf.pEnd;
			if(pTop != pEnd)
			{
				if(UCSR0A & 0x20 )
				{
					c = uart0TxdBuf.buffer[pTop];
					//if((c != STX) && (c!= ETX))		
						UDR0 = c;
					pTop ++;
					if(pTop == UARTBUF_TXD_LEN)
						pTop = 0;
					uart0TxdBuf.pHead = pTop;
				}
			}
			
			// ACK ��ʱ�ȴ�
			if(WaitAnswer == 1)
			{
				timeOut++;
				//if(timeOut > 800)	// 0.5 * 800  -> 400ms
				//if(timeOut > 2000)	
				if(timeOut > 6000)	
				{
					timeOut = 0;
					WaitAnswer = 0;
					Uart0ReUnable;
					uart_Printf("*3344 WaitAckOutOfTime\r\n");	
					Uart0ReEnable;
				}
			}
			else
			{
				timeOut = 0;
			}
			break;	
		case 1:		// ����,���ݰ�δ��ʼ
			pTop = uart0TxdBuf.pHead;
			pEnd = uart0TxdBuf.pEnd;
			if(pTop != pEnd)
			{
				if(UCSR0A & 0x20)
				{
					c = uart0TxdBuf.buffer[pTop];
					UDR0 = c;
					pTop ++;
					if(pTop == UARTBUF_TXD_LEN)
						pTop = 0;
					uart0TxdBuf.pHead = pTop;
					if(c == 0x0d)	// ���ݰ�����
					{
						PacketState = 0;
						break;
					}
				}
			}
			break;
		case 12:	// �����յ�������
			if(UartReceiveBuf[PRecBuf] != 0)
			{
				uart0SendChar(UartReceiveBuf[PRecBuf]);
				PRecBuf++;
			}
			else
			{
				uart0SendChar('\r');
				uart0SendChar('\n');
				PRecBuf = 0;
				WaitAnswer = 1;
				PacketState = 0;
				
			}
			break;
		case 13:	// �����յ�������
			if(UartReceiveBuf[PRecBuf] != 0)
			{
				checkFlag = 1;
				CommandExplain(UartReceiveBuf[PRecBuf]);	// �������
				PRecBuf ++;
			}
			else
			{
				checkFlag = 0;
				PRecBuf = 0;
				PacketState = 0;
			}
			break;
		case 14:
			if(UartReceiveBuf[PRecBuf] != 0)
			{
				uart0SendChar(UartReceiveBuf[PRecBuf]);
				PRecBuf++;
			}
			else
			{
				uart0SendChar('\r');
				uart0SendChar('\n');
				uart0SendChar(NAK);
				PRecBuf = 0;
				PacketState = 0;
			}
			break;
		default:
			break;
		}
}

unsigned char uart0SendData(unsigned char * data, unsigned short len)
{
	// ָ����������д�뻺����
	unsigned short i;
	unsigned int pTop, pEnd, p;
	uartBufProtect = 1;
	pTop = uart0TxdBuf.pHead;
	pEnd = uart0TxdBuf.pEnd;
	for(i=0; i<len; i++)
	{
		p = pEnd;
		pEnd ++;
		if(pEnd == UARTBUF_TXD_LEN)
			pEnd = 0;
		if(pEnd != pTop)
			uart0TxdBuf.buffer[p] = (* (data+i));
		else
		{
			pEnd = p;
			break;
		}
	}
	uart0TxdBuf.pEnd = pEnd;
	uartBufProtect = 0;
	return 1;
}

unsigned char uart0SendChar(unsigned char c){
	// 1�ַ�����д�뻺����
	unsigned int pTop, pEnd, p;
	if(c){
		pTop = uart0TxdBuf.pHead;
		pEnd = uart0TxdBuf.pEnd;
		p = pEnd;
		pEnd ++;
		if(pEnd == UARTBUF_TXD_LEN)
			pEnd = 0;
		if(pEnd != pTop)
			uart0TxdBuf.buffer[p] = c;
		else
			return 1;
		uart0TxdBuf.pEnd = pEnd;
	
		uartBufProtect = 0;
		return 0;
		}
	return 1;
}

unsigned char uart0SendString(_CONST  char * pS)
{
	unsigned short i;
	unsigned int pTop, pEnd, p;
	// �ַ�������д�뻺����
	uartBufProtect = 1;
	pTop = uart0TxdBuf.pHead;
	pEnd = uart0TxdBuf.pEnd;
	while(*pS){
		p = pEnd;
		pEnd ++;
		if(pEnd == UARTBUF_TXD_LEN){
			pEnd = 0;
			}
		if(pEnd != pTop){
			uart0TxdBuf.buffer[p] = (* (pS));
			pS ++;
			}
		else{
			pEnd = p;
			break;
			}
		}
	uart0TxdBuf.pEnd = pEnd;
	uartBufProtect = 0;
	return 1;
}

unsigned char uart0GetChar(void){
	unsigned char c;
	unsigned char pHead, pEnd;
	pHead = uart0RxdBuf.pHead;
	pEnd  = uart0RxdBuf.pEnd;
	if(pEnd == pHead){
		return 0;
		}
	else{
//		UCSR0B &= 0x7f;		// ��ֹ�����ж�
		c = uart0RxdBuf.buf[pHead];
		uart0RxdBuf.buf[pHead] = 0;
		uart0RxdBuf.pHead ++;
		if(uart0RxdBuf.pHead == UARTBUF_RXD_LEN)
			uart0RxdBuf.pHead = 0;
		}
//	UCSR0B |= 0x80;		// ���������ж�
	return c;
}

/********************************************* File end **********************************************/
