

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "LibCommon.h"



/* Master */
#define TW_START			0x08	//START�ѷ���
#define TW_REP_START		0x10//�ظ�START�ѷ���
/* Master Transmitter */
#define TW_MT_SLA_ACK		0x18//SLA+W �ѷ����յ�ACK  
#define TW_MT_SLA_NACK		0x20//SLA+W �ѷ��ͽ��յ�NOT ACK 
#define TW_MT_DATA_ACK		0x28//�����ѷ��ͽ��յ�ACK  
#define TW_MT_DATA_NACK		0x30//�����ѷ��ͽ��յ�NOT ACK 
#define TW_MT_ARB_LOST		0x38//SLA+W �����ݵ��ٲ�ʧ�ܴӷ���״̬��
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
#define TW_ST_SLA_ACK			0xA8	//�Լ���SLA+R �Ѿ�������ACK �ѷ���
#define TW_ST_ARB_LOST_SLA_ACK	0xB0	//SLA+R/W ��Ϊ�������ٲ�ʧ�ܣ��Լ���SLA+R �Ѿ�������ACK �ѷ���
#define TW_ST_DATA_ACK			0xB8	//TWDR �������Ѿ����ͽ��յ�ACK  
#define TW_ST_DATA_NACK			0xC0	//TWDR �������Ѿ����ͽ��յ�NOT ACK  
#define TW_ST_LAST_DATA			0xC8	//TWDR ��һ�ֽ������Ѿ�����(TWAE = ��0��);���յ�ACK 

//�ܽŶ��� 
#define  pinSCL    0     //PC0 SCL 
#define  pinSDA    1     //PC1 SDA  
#define F_CPU	14745600
#define fSCL    100000    			 
#if F_CPU < fSCL*36   
	#define TWBR_SET    10;     	
#else   
	#define TWBR_SET    (F_CPU/fSCL-16)/2;			//����TWBRֵ 
#endif 
#define TW_ACT    (1<<TWINT)|(1<<TWEN)|(1<<TWIE) 	

#define SLA_24CXX   	0xA0    	
#define ADDR_24C256		0x00 
//TWI_����״̬ 
#define TW_BUSY		0 
#define TW_OK		1 
#define TW_FAIL		2 
//TWI_��д����״̬ 
#define OP_BUSY		0 
#define OP_RUN		1  

//TWI��д������������ 
#define ST_FAIL		0 		// ����״̬ 
#define ST_START	1 		// START״̬��� 
#define ST_SLAW		2 		// SLAW״̬��� 
#define ST_WADDR_H	3		// 
#define ST_WADDR_L	4 		// ADDR״̬��� 
//TWI���������� 
#define ST_RESTART  5 		// RESTART״̬��� 
#define ST_SLAR		6		// SLAR״̬��� 
#define ST_RDATA	7		// ��ȡ����״̬��飬ѭ��n�ֽ� 
//TWIд�������� 
#define ST_WDATA	8		// д����״̬��飬ѭ��n�ֽ� 
#define FAIL_MAX	20		// ���Դ������ֵ  

#define TW_READ		1
#define TW_WRITE	0

//����ȫ�ֱ��� 
unsigned char ORGDATA[8]={0xAA,0xA5,0x55,0x5A,0x01,0x02,0x03,0x04}; //ԭʼ���� 
unsigned char CMPDATA[8];      	//�Ƚ����� 
unsigned char BUFFER[256];      //������������װ������AC24C02������ 
struct str_TWI{         //TWI���ݽṹ 
	volatile unsigned char STATUS;	//TWI_����״̬     
	unsigned char SLA;      		//���豸��������ַ     
	unsigned int ADDR;				//���豸�����ݵ�ַ     
	unsigned char *pBUF;			//���ݻ�����ָ��     
	unsigned int DATALEN;			//���ݳ���     
	unsigned char STATE;			//TWI��д��������     
	unsigned char FAILCNT;			//ʧ�����Դ��� 
}; 
struct str_TWI strTWI;       //TWI�����ݽṹ���� 

unsigned char TWI_RW(unsigned char sla,unsigned int addr,unsigned char *ptr,unsigned int len) {
	unsigned char i;     
	if (strTWI.STATUS==TW_BUSY){//TWIæ�����ܽ��в���         
		return OP_BUSY;     
		}     
	strTWI.STATUS=TW_BUSY;     
//	i=(addr>>8)<<1;     
//	i&=0x06;         //������24C04/08��EEPROM��ַ��λ����SLA����     
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
	//IIC�ж�     
	unsigned char action,state,status;     
	action = strTWI.SLA & TW_READ;    
	state = strTWI.STATE;     
	status = TWSR & 0xF8;  
	if ((status>=0x60)||(status==0x00)){  
		return;     
		}     
	switch(state){     
		case ST_START:						//START״̬���         
			if(status==TW_START) {			
				TWDR = strTWI.SLA & 0xFE;	         
				TWCR = TW_ACT;				      
				}         
			else{	//����start�źų���            
				state=ST_FAIL;         
				}
			break;
		case ST_SLAW: //SLAW״̬���
			if(status==TW_MT_SLA_ACK){		//����������ַ�ɹ� 
				TWDR = (strTWI.ADDR&0xff00)>>8;    
				TWCR = TW_ACT;                  
				}         
			else{		//����������ַ����             
				state=ST_FAIL;         
				}         
			break;  
		case ST_WADDR_H:	// ���ֽڵ�ַ��д�룬��ʼд���ֽڵ�ַ
			if(status==TW_MT_DATA_ACK){		
				TWDR = strTWI.ADDR&0x00ff; 
				TWCR = TW_ACT;                   
				}         
			else{		//����������ַ����             
				state=ST_FAIL;         
				}         
			break;
		case ST_WADDR_L:	// ���ֽڵ�ַ��д��
			if(status==TW_MT_DATA_ACK){			//����eeprom��ַ�ɹ� 
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
			else{								//����eeprom��ַ����
				state=ST_FAIL;
				}
			break;
		case ST_RESTART:					//RESTART״̬��飬ֻ�ж�����ģʽ������������
			if(status==TW_REP_START){		
				TWDR=strTWI.SLA;			
				TWCR=TW_ACT;				
				}
			else{							
				state=ST_FAIL;
				}
			break;
		case ST_SLAR:						//SLAR״̬��飬ֻ�ж�����ģʽ������������ 
			if(status==TW_MR_SLA_ACK){		
				if (strTWI.DATALEN--){		
					TWCR=(1<<TWEA)|TW_ACT;	
					}
				else{						
					TWCR=TW_ACT;			
					}
				}
			else{							//����������ַ����
				state=ST_FAIL;
				}
			break;
		case ST_RDATA:						//��ȡ����״̬��飬ֻ�ж�����ģʽ������������
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
			else{							//��ȡ���ݳ���
				state=ST_FAIL;
				}
			break; 
		case ST_WDATA:						//д����״̬��飬ֻ��д����ģʽ������������
			state--;						
			if(status==TW_MT_DATA_ACK) {	
				if (strTWI.DATALEN) {		
					TWDR=*strTWI.pBUF++; 
					strTWI.DATALEN--;  
					TWCR=TW_ACT;
					//������һ������
					} 
				else  {						//д����
					TWCR=(1<<TWSTO)|TW_ACT;
					strTWI.STATUS=TW_OK;  
					} 
				} 
			else  {							//д����ʧ��
				state=ST_FAIL; 
				}  
			break; 
		default:						//����״̬
			state=ST_FAIL; 
			break; 
		}  
	if (state==ST_FAIL) {				//������
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
	strTWI.STATE=state;					//����״̬
}  

void TWI_Init(void){  
	PORTD |= 0x03;		//SCL,SDAʹ�����ڲ���10K��������
	DDRD &= 0xFC; 		// ����Ϊ��������
	//TWI��ʼ��
	TWSR=0x00;			//Ԥ��Ƶ=0^4=1 
	TWBR=TWBR_SET;   
	TWAR=0x00;			//����ģʽ���õ�ַ��Ч
	TWCR=0x00;			//�ر�TWIģ��
	strTWI.STATUS=TW_OK; 
}

unsigned char AT24C256_Read(unsigned int addr,unsigned char *ptr,unsigned int len){
	return TWI_RW(SLA_24CXX+(ADDR_24C256<<1)+TW_READ,addr,ptr,len);	
}
unsigned char AT24C256_Write(unsigned int addr,unsigned char *ptr,unsigned int len){
	return TWI_RW(SLA_24CXX+(ADDR_24C256<<1)+TW_WRITE,addr,ptr,len);	
}

   
