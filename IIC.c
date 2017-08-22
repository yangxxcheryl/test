

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "LibCommon.h"



/* Master */
#define TW_START			0x08	//START已发送
#define TW_REP_START		0x10//重复START已发送
/* Master Transmitter */
#define TW_MT_SLA_ACK		0x18//SLA+W 已发送收到ACK  
#define TW_MT_SLA_NACK		0x20//SLA+W 已发送接收到NOT ACK 
#define TW_MT_DATA_ACK		0x28//数据已发送接收到ACK  
#define TW_MT_DATA_NACK		0x30//数据已发送接收到NOT ACK 
#define TW_MT_ARB_LOST		0x38//SLA+W 或数据的仲裁失败从发送状态码
/* Master Receiver */
#define TW_MR_ARB_LOST		0x38	//
#define TW_MR_SLA_ACK		0x40
#define TW_MR_SLA_NACK		0x48	//
#define TW_MR_DATA_ACK		0x50	//
#define TW_MR_DATA_NACK		0x58	//
/* Slave Receiver */
#define TW_SR_SLA_ACK				0x60	//
#define TW_SR_ARB_LOST_SLA_ACK		0x68	//
#define TW_SR_GCALL_ACK				0x70	//
#define TW_SR_ARB_LOST_GCALL_ACK	0x78	//
#define TW_SR_DATA_ACK				0x80	//
#define TW_SR_DATA_NACK				0x88	//
#define TW_SR_GCALL_DATA_ACK		0x90	//
#define TW_SR_GCALL_DATA_NACK		0x98	//
#define TW_SR_STOP					0xA0	//
/* Slave Transmitter */
#define TW_ST_SLA_ACK			0xA8	//自己的SLA+R 已经被接收ACK 已返回
#define TW_ST_ARB_LOST_SLA_ACK	0xB0	//SLA+R/W 作为主机的仲裁失败；自己的SLA+R 已经被接收ACK 已返回
#define TW_ST_DATA_ACK			0xB8	//TWDR 里数据已经发送接收到ACK  
#define TW_ST_DATA_NACK			0xC0	//TWDR 里数据已经发送接收到NOT ACK  
#define TW_ST_LAST_DATA			0xC8	//TWDR 的一字节数据已经发送(TWAE = “0”);接收到ACK 

//管脚定义 
#define  pinSCL    0     //PC0 SCL 
#define  pinSDA    1     //PC1 SDA  
#define F_CPU	14745600
#define fSCL    100000    			 
#if F_CPU < fSCL*36   
	#define TWBR_SET    10;     	
#else   
	#define TWBR_SET    (F_CPU/fSCL-16)/2;			//计算TWBR值 
#endif 
#define TW_ACT    (1<<TWINT)|(1<<TWEN)|(1<<TWIE) 	

#define SLA_24CXX   	0xA0    	
#define ADDR_24C256		0x00 
//TWI_操作状态 
#define TW_BUSY		0 
#define TW_OK		1 
#define TW_FAIL		2 
//TWI_读写命令状态 
#define OP_BUSY		0 
#define OP_RUN		1  

//TWI读写操作公共步骤 
#define ST_FAIL		0 		// 出错状态 
#define ST_START	1 		// START状态检查 
#define ST_SLAW		2 		// SLAW状态检查 
#define ST_WADDR_H	3		// 
#define ST_WADDR_L	4 		// ADDR状态检查 
//TWI读操作步骤 
#define ST_RESTART  5 		// RESTART状态检查 
#define ST_SLAR		6		// SLAR状态检查 
#define ST_RDATA	7		// 读取数据状态检查，循环n字节 
//TWI写操作步骤 
#define ST_WDATA	8		// 写数据状态检查，循环n字节 
#define FAIL_MAX	20		// 重试次数最大值  

#define TW_READ		1
#define TW_WRITE	0

//定义全局变量 
unsigned char ORGDATA[8]={0xAA,0xA5,0x55,0x5A,0x01,0x02,0x03,0x04}; //原始数据 
unsigned char CMPDATA[8];      	//比较数据 
unsigned char BUFFER[256];      //缓冲区，可以装载整个AC24C02的数据 
struct str_TWI{         //TWI数据结构 
	volatile unsigned char STATUS;	//TWI_操作状态     
	unsigned char SLA;      		//从设备的器件地址     
	unsigned int ADDR;				//从设备的数据地址     
	unsigned char *pBUF;			//数据缓冲区指针     
	unsigned int DATALEN;			//数据长度     
	unsigned char STATE;			//TWI读写操作步骤     
	unsigned char FAILCNT;			//失败重试次数 
}; 
struct str_TWI strTWI;       //TWI的数据结构变量 

unsigned char TWI_RW(unsigned char sla,unsigned int addr,unsigned char *ptr,unsigned int len) {
	unsigned char i;     
	if (strTWI.STATUS==TW_BUSY){//TWI忙，不能进行操作         
		return OP_BUSY;     
		}     
	strTWI.STATUS=TW_BUSY;     
//	i=(addr>>8)<<1;     
//	i&=0x06;         //考虑了24C04/08的EEPROM地址高位放在SLA里面     
//	strTWI.SLA=sla+i;    
	strTWI.SLA=sla; 
	strTWI.ADDR=addr;     
	strTWI.pBUF=ptr;     
	strTWI.DATALEN=len;     
	strTWI.STATE=ST_START; 
	strTWI.FAILCNT=0;     
	TWCR=(1<<TWSTA)|TW_ACT; 
	return OP_RUN; 
}  

#pragma interrupt_handler twi_isr:iv_TWI
void twi_isr(void) {
	//IIC中断     
	unsigned char action,state,status;     
	action = strTWI.SLA & TW_READ;    
	state = strTWI.STATE;     
	status = TWSR & 0xF8;  
	if ((status>=0x60)||(status==0x00)){  
		return;     
		}     
	switch(state){     
		case ST_START:						//START状态检查         
			if(status==TW_START) {			
				TWDR = strTWI.SLA & 0xFE;	         
				TWCR = TW_ACT;				      
				}         
			else{	//发送start信号出错            
				state=ST_FAIL;         
				}
			break;
		case ST_SLAW: //SLAW状态检查
			if(status==TW_MT_SLA_ACK){		//发送器件地址成功 
				TWDR = (strTWI.ADDR&0xff00)>>8;    
				TWCR = TW_ACT;                  
				}         
			else{		//发送器件地址出错             
				state=ST_FAIL;         
				}         
			break;  
		case ST_WADDR_H:	// 高字节地址已写入，开始写低字节地址
			if(status==TW_MT_DATA_ACK){		
				TWDR = strTWI.ADDR&0x00ff; 
				TWCR = TW_ACT;                   
				}         
			else{		//发送器件地址出错             
				state=ST_FAIL;         
				}         
			break;
		case ST_WADDR_L:	// 低字节地址已写入
			if(status==TW_MT_DATA_ACK){			//发送eeprom地址成功 
				if (action==TW_READ){			
					TWCR=(1<<TWSTA)|TW_ACT;		
					}             
				else{							
					TWDR=*strTWI.pBUF++;		
					strTWI.DATALEN--;
					state=ST_WDATA-1;			
					TWCR=TW_ACT;				
					}
				}
			else{								//发送eeprom地址出错
				state=ST_FAIL;
				}
			break;
		case ST_RESTART:					//RESTART状态检查，只有读操作模式才能跳到这里
			if(status==TW_REP_START){		
				TWDR=strTWI.SLA;			
				TWCR=TW_ACT;				
				}
			else{							
				state=ST_FAIL;
				}
			break;
		case ST_SLAR:						//SLAR状态检查，只有读操作模式才能跳到这里 
			if(status==TW_MR_SLA_ACK){		
				if (strTWI.DATALEN--){		
					TWCR=(1<<TWEA)|TW_ACT;	
					}
				else{						
					TWCR=TW_ACT;			
					}
				}
			else{							//发送器件地址出错
				state=ST_FAIL;
				}
			break;
		case ST_RDATA:						//读取数据状态检查，只有读操作模式才能跳到这里
			state--;						
			if(status==TW_MR_DATA_ACK){		
				*strTWI.pBUF++=TWDR;
				if (strTWI.DATALEN--){		
					TWCR=(1<<TWEA)|TW_ACT;
					}
				else{						
					TWCR=TW_ACT;			
					}
				} 
			else if(status==TW_MR_DATA_NACK){	
				*strTWI.pBUF++=TWDR;
				TWCR=(1<<TWSTO)|TW_ACT;   
				strTWI.STATUS=TW_OK;
				}
			else{							//读取数据出错
				state=ST_FAIL;
				}
			break; 
		case ST_WDATA:						//写数据状态检查，只有写操作模式才能跳到这里
			state--;						
			if(status==TW_MT_DATA_ACK) {	
				if (strTWI.DATALEN) {		
					TWDR=*strTWI.pBUF++; 
					strTWI.DATALEN--;  
					TWCR=TW_ACT;
					//触发下一步动作
					} 
				else  {						//写够了
					TWCR=(1<<TWSTO)|TW_ACT;
					strTWI.STATUS=TW_OK;  
					} 
				} 
			else  {							//写数据失败
				state=ST_FAIL; 
				}  
			break; 
		default:						//错误状态
			state=ST_FAIL; 
			break; 
		}  
	if (state==ST_FAIL) {				//错误处理
		strTWI.FAILCNT++;  
		if (strTWI.FAILCNT<FAIL_MAX) {	
			TWCR=(1<<TWSTA)|TW_ACT;		
			} 
		else  {							
			TWCR=(1<<TWSTO)|TW_ACT;		
			strTWI.STATUS=TW_FAIL; 
			}
		}  
	state++;  
	strTWI.STATE=state;					//保存状态
}  

void TWI_Init(void){  
	PORTD |= 0x03;		//SCL,SDA使能了内部的10K上拉电阻
	DDRD &= 0xFC; 		// 配置为输入引脚
	//TWI初始化
	TWSR=0x00;			//预分频=0^4=1 
	TWBR=TWBR_SET;   
	TWAR=0x00;			//主机模式，该地址无效
	TWCR=0x00;			//关闭TWI模块
	strTWI.STATUS=TW_OK; 
}

unsigned char AT24C256_Read(unsigned int addr,unsigned char *ptr,unsigned int len){
	return TWI_RW(SLA_24CXX+(ADDR_24C256<<1)+TW_READ,addr,ptr,len);	
}
unsigned char AT24C256_Write(unsigned int addr,unsigned char *ptr,unsigned int len){
	return TWI_RW(SLA_24CXX+(ADDR_24C256<<1)+TW_WRITE,addr,ptr,len);	
}

   
