

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"
#include "eeprom.h"


// ά�����ܳ���

static unsigned char mainStep = 0;
static unsigned char workStep = 0;
static unsigned char subStep = 0;

static unsigned char waitMotor0 = 0;
static unsigned char waitMotor1 = 0;
static unsigned char waitMotor2 = 0;
static unsigned char waitMotor3 = 0;
static unsigned char waitMotor4 = 0;
static unsigned char waitStartKey = 0;

static unsigned char quitFlag;
static unsigned char timeOut;

static signed int DropHeightFactor;

extern unsigned int MaintainSubFunParam;

unsigned char SetMaintianSubfunctionQuitFlag(void){
	quitFlag = 1;
	return 0;
}

// ���õ����뵽���ȳ��ұ�Ե�ĵ�������
// ��Χ-30 ~ 30
// ���ڴ˷�Χ,���Բ�ѯԭ����ֵ
void SetNeedleOnMixPosFactor(signed int n)
{
	signed char i;
	if(n < 30 && n > -30) // ������Χ�ж�
	{	
		EEPROM_WRITE(EEP_ADD_NEEDLE_MIX_ADJ, n);
	}
	else
	{
		EEPROM_READ(EEP_ADD_NEEDLE_MIX_ADJ, i);		// ���¶�ȡд�������ֵ
		Uart0ReUnable;
		uart_Printf("%s $%4d\r\n",strM4102, i);
		Uart0ReEnable;
		return;
	}
	EEPROM_READ(EEP_ADD_NEEDLE_MIX_ADJ, i);		// ���¶�ȡд�������ֵ
	if(i > 30 || i < -30) // �����ֵ���쳣
	{		
		i = 0;
		EEPROM_WRITE(EEP_ADD_NEEDLE_MIX_ADJ, i);
	}
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM4102, i);
	Uart0ReEnable;
}

signed int GetNeedleOnMixCenterPos(void)
{
	signed int si;
	signed char sc;
	EEPROM_READ(EEP_ADD_NEEDLE_MIX_ADJ, sc);		// ���¶�ȡд�������ֵ
	if(sc>30 || sc<-30){		// �����ֵ���쳣
		sc = 0;
		EEPROM_WRITE(EEP_ADD_NEEDLE_MIX_ADJ, sc);
		}
	si = _POS_NEEDLE_ON_MIXCENTRE + (signed int)sc;
	return si;
}


signed int GetNeedleOnMixSidePos(void)
{
	signed int si;
	signed char sc;
	EEPROM_READ(EEP_ADD_NEEDLE_MIX_ADJ, sc);		// ���¶�ȡд�������ֵ
	if(sc>30 || sc<-30){		// �����ֵ���쳣
		sc = 0;
		EEPROM_WRITE(EEP_ADD_NEEDLE_MIX_ADJ, sc);
		}
	si = _POS_NEEDLE_ON_MIXSIDE + (signed int)sc;
	return si;
}


// ���õ���������ĸ߶ȵ�������
// ��Χ-100 ~ 200
// ���ڴ˷�Χ,���Բ�ѯԭ����ֵ
void SetDropHeightFactor(signed int n)
{
	if(n <= 200 && n >= -100)
	{	// ������Χ�ж�
		EEPROM_WRITE(EEP_ADD_DROP_HEIGHT_ADJ, n);
	}
	else
	{
		EEPROM_READ(EEP_ADD_DROP_HEIGHT_ADJ, DropHeightFactor);		// ���¶�ȡд�������ֵ
		Uart0ReUnable;
		uart_Printf("%s $%4d\r\n",strM4104, DropHeightFactor);
		Uart0ReEnable;
		return;
	}
	EEPROM_READ(EEP_ADD_DROP_HEIGHT_ADJ, DropHeightFactor);		// ���¶�ȡд�������ֵ
	if(DropHeightFactor > 200 || DropHeightFactor < -100)
	{		// �����ֵ���쳣
		DropHeightFactor = 0;
		EEPROM_WRITE(EEP_ADD_DROP_HEIGHT_ADJ, DropHeightFactor);
	}
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM4104, DropHeightFactor);
	Uart0ReEnable;
}

signed int GetDropHeight(void)
{
	signed int si;
	signed int sc;
	EEPROM_READ(EEP_ADD_DROP_HEIGHT_ADJ, sc);		// ���¶�ȡд�������ֵ
	if(sc > 200 || sc < -100)
	{	// �����ֵ���쳣
		sc = 0;
		EEPROM_WRITE(EEP_ADD_DROP_HEIGHT_ADJ, sc);
		Uart0ReUnable;
		uart_Printf("%s $%4d\r\n",strE3936, DropHeightFactor);
		Uart0ReEnable;
	}
	si = _POS_CARD_DROP + (signed int)sc;
	return si;
}

signed char SetMixHeight(signed char n)
{
	signed char i;
	if(n <= 100 && n >= -100)
	{	// ������Χ�ж�
		EEPROM_WRITE(EEP_ADD_MIX_HEIGHT_ADJ, n);
	}
	EEPROM_READ(EEP_ADD_MIX_HEIGHT_ADJ, i);		// ���¶�ȡд�������ֵ
	if(i > 100 || i < -100)
	{		// �����ֵ���쳣
		i = 0;
		EEPROM_WRITE(EEP_ADD_MIX_HEIGHT_ADJ, i);
	}
	return i;
}

signed int GetMixHeight(void)
{
	signed int si;
	signed char sc;
	EEPROM_READ(EEP_ADD_MIX_HEIGHT_ADJ, sc);		// ���¶�ȡд�������ֵ
	if(sc > 100 || sc < -100)
	{		// �����ֵ���쳣
		sc = 0;
		EEPROM_WRITE(EEP_ADD_MIX_HEIGHT_ADJ, sc);
	}
	si = _POS_CARD_MIX + (signed int)sc;
	return si;
}

unsigned char CardLoadStartAdjust(void){
	// ��Ƭװ����ʼλ����
	if(waitMotor0){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)	return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_CARD_LOAD, 64, 20, CURRENT_CARD_LOAD);
			MotInitCheck(MOT_CARD_LOAD);	// ��Ƭװ��С���ص���ʼλ�ȴ�����
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4105);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char CardLoadEndAdjust(void){
	// ��Ƭװ���յ�λ����
	if(waitMotor0){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)	return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_CARD_LOAD, 64, 20, CURRENT_CARD_LOAD);
			MotAdjustPosition(MOT_CARD_LOAD,_POS_CARDLOAD_RING);	// ��Ƭװ��С��λ�õ���
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4106);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char CardUnloadStartAdjust(void){
	// ��Ƭж����ʼλ����
	if(waitMotor0){	if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_CARD_UNLOAD, 64, 20, CURRENT_CARD_UNLOAD);
			MotInitCheck(MOT_CARD_UNLOAD);	// ��Ƭװ��С���ص���ʼλ�ȴ�����
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4107);
			Uart0ReEnable;;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char CardUnloadEndAdjust(void){
	// ��Ƭж���յ�λ����
	if(waitMotor0){	if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_CARD_UNLOAD, 64, 20, CURRENT_CARD_UNLOAD);
			MotAdjustPosition(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);	// ��Ƭװ��С��λ�õ���
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4108);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

static unsigned char LiquidPhoNum;
void SetLiquidPhotoAdjustNum(unsigned char n){
	if(n>2)
		n = 2;
	LiquidPhoNum = n;
}
unsigned char LiquidPhotoAdj_Even(INFO_EVENT * pInfoEvent){
	unsigned char * pInfo;
	switch(mainStep){
		case 1:
			if(pInfoEvent->event == INFO_LIQ_PHO_ADJ){
				pInfo = &(pInfoEvent->info[0]);
				if(*(pInfo) == LiquidPhoNum){
					// ���Һ·����������
					Uart0ReUnable;
					uart_Printf("%s $%4d Num $%4d PWM $%4d Vol\r\n",strM4109, LiquidPhoNum, *(pInfo+1), *(pInfo+2));
					Uart0ReEnable;
					mainStep = 2;
					return 1;
					}
				}
			break;
		default:
			break;
		}
	return 0;
}
unsigned char LiquidPhotoAdjust(void){
	switch(mainStep){
		case 0:
			SetLiquidPhotoAdjust(LiquidPhoNum);
			mainStep = 1;
			RegisterLiquidEvenProcess(LiquidPhotoAdj_Even);
			break;
		case 2:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strM4110, LiquidPhoNum);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

static unsigned char CardStorePhoNum;
void SetCardStorePhotoAdjustNum(unsigned char n){
//	if(n>6)
//		n = 6;
	if(n > 5)
		n = 5;
	if(n == 0)
		n = 1;
	CardStorePhoNum = n;
}
unsigned char CardStorePhotoAdj_Even(INFO_EVENT * pInfoEvent){
	unsigned char * pInfo;
	switch(mainStep){
		case 1:
			if(pInfoEvent->event == INFO_STORE_CAL){
				pInfo = &(pInfoEvent->info[0]);
		//		if(*(pInfo) == CardStorePhoNum){
					// ���Һ·����������
					Uart0ReUnable;
					uart_Printf("%s $%4d $%4d $%4d $%4d\r\n",strM4111, (*(pInfo)), *(pInfo+1), *(pInfo+2), *(pInfo+3));
					Uart0ReEnable;
					mainStep = 2;
					return 1;
		//			}
				}
			break;
		default:
			break;
		}
	return 0;
}
unsigned char CardStorePhotoAdjust(void){
	switch(mainStep){
		case 0:
			SetStoreCAL(CardStorePhoNum);
			mainStep = 1;
			RegisterCardStoreEvenProcess(CardStorePhotoAdj_Even);
			break;
		case 2:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strM4112, CardStorePhoNum);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}
unsigned char TurnPlateCheck(void){
	// ת�̲���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_TURN_PLATE,120,10,CURRENT_TURN_PLATE);
		//	MotRun(MOT_TURN_PLATE,8000);
			MotInitCheck(MOT_TURN_PLATE);
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			SetDelayTime(1, 10);
			mainStep = 2;
			subStep = 0;
			break;
		case 2:
			subStep ++;
			if(subStep<30){
				MotRunToSite(MOT_TURN_PLATE, subStep);
				waitMotor0 = 1;
				mainStep = 3;
				}
			else{
				waitMotor0 = 1;
				mainStep = 4;
				}
			break;
		case 3:
			SetDelayTime(1, 5);
			mainStep = 2;
			break;
		case 4:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4120);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char NeedleTurnCheck(void){
	// ��ת�۲���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotor1 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			waitMotor1 = 1;
			mainStep = 1;
			break;
		case 1:
			SetMotRunPam(MOT_SAMP_TRUN,120,40,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			waitMotor0 = 1;
			mainStep = 2;
			break;
		case 2:
			SetDelayTime(1, 10);
			mainStep = 3;
			break;
		case 3:
			MotRunTo(MOT_SAMP_TRUN, _POS_NEEDLE_ON_MIXSIDE);
			waitMotor0 = 1;
			mainStep = 4;
		case 4:
			SetDelayTime(1, 10);
			mainStep = 5;
			break;
		case 5:
			MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
			waitMotor0 = 1;
			mainStep = 6;
			break;
		case 6:
			SetDelayTime(1, 10);
			mainStep = 7;
			break;
		case 7:
			MotRunTo(MOT_SAMP_TRUN, 0);
			waitMotor0 = 1;
			mainStep = 8;
			break;
		case 8:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4121);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char NeedleUpdownCheck(void){
	// ȡ�����������в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotor1 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			waitMotor1 = 1;
			mainStep = 1;
			break;
		case 1:
			SetMotRunPam(MOT_SAMP_TRUN,120,10,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			waitMotor0 = 1;
			mainStep = 2;
			break;
		case 2:
			SetDelayTime(1, 10);
			mainStep = 3;
			break;
		case 3:
			MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
			waitMotor0 = 1;
			mainStep = 4;
		case 4:
			SetDelayTime(1, 10);
			mainStep = 5;
			break;
		case 5:
			MotRunTo(MOT_SAMP_NEEDLE, _POS_SAMP_DOWN);
			waitMotor1 = 1;
			mainStep = 6;
			break;
		case 6:
			SetDelayTime(1, 10);
			mainStep = 7;
			break;
		case 7:
			MotRunTo(MOT_SAMP_NEEDLE, 0);
			waitMotor1 = 1;
			mainStep = 8;
			break;
		case 8:
			MotRunTo(MOT_SAMP_TRUN, 0);
			waitMotor0 = 1;
			mainStep = 9;
			break;
		case 9:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4122);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char CardStoreMoveCheck(void){
	// Ƭ��С�����в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

	switch(mainStep)
	{
		case 0:
			SetMotRunPam(MOT_STORE_CARD_MOVE,160,10,CURRENT_STORE_MOVE);
			MotInitCheck(MOT_STORE_CARD_MOVE);
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			SetDelayTime(1, 5);
			mainStep = 2;
			break;
		case 2:
			if(GetMotorMonitorState(MOT_STORE_CARD_MOVE,ZeroMonitor) == 1)		// �ж�ȡƬС���Ƿ�ص���λ
			{
				mainStep = 20;
			}
			else
			{
				mainStep = 0;
			}
			break;
		case 20:
			MotRunTo(MOT_STORE_CARD_MOVE, 2550);
			waitMotor0 = 1;
			mainStep = 3;
		case 3:
			SetDelayTime(1, 10);
			mainStep = 4;
			break;
		case 4:
			MotRunTo(MOT_STORE_CARD_MOVE, 0);
			waitMotor0 = 1;
			mainStep = 50;
			break;
		case 50:
			SetDelayTime(1, 5);
			mainStep = 51;
			break;
		case 51:
			if(GetMotorMonitorState(MOT_STORE_CARD_MOVE,ZeroMonitor) == 1)		// �ж�ȡƬС���Ƿ�ص���λ
			{
				mainStep = 5;
			}
			else
			{
				mainStep = 4;
			}
			break;
		case 5:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4123);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char CardTakeHookCheck(void){
	// Ƭ��ȡƬ�������в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_STORE_CARD_MOVE,160,10,CURRENT_STORE_MOVE);
			MotInitCheck(MOT_STORE_CARD_MOVE);
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			SetDelayTime(1, 5);
			mainStep = 2;
			break;
		case 2:
			MotRunTo(MOT_STORE_CARD_MOVE, CalCardStorePos(1));
			waitMotor0 = 1;
			mainStep = 3;
		case 3:
			SetDelayTime(1, 5);
			mainStep = 4;
			break;
		case 4:
			SetCardTrolleyState(1);	// ��Ƭ̧��
			SetDelayTime(1, 20);
			mainStep = 5;
			break;
		case 5:
			SetCardTrolleyState(0);
			SetDelayTime(1, 5);
			mainStep = 6;
			break;
		case 6:
			MotRunTo(MOT_STORE_CARD_MOVE, 0);
			waitMotor0 = 1;
			mainStep = 7;
			break;
		case 7:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4124);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char CardLoadCheck(void){
	// ��Ƭװ��С�����в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_CARD_LOAD,100,20,CURRENT_CARD_LOAD);
			MotInitCheck(MOT_CARD_LOAD);
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			SetDelayTime(1, 5);
			mainStep = 2;
			break;
		case 2:
			if(GetMotorMonitorState(MOT_CARD_LOAD,ZeroMonitor) == 1)		// �ж��������Ƿ�ص���λ
			{
				mainStep = 20;
			}
			else
			{
				mainStep = 0;
			}
			break;
		case 20:
			MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_RING);
			waitMotor0 = 1;
			mainStep = 3;
		case 3:
			SetDelayTime(1, 10);
			mainStep = 4;
			break;
		case 4:
			MotRunTo(MOT_CARD_LOAD, 0);
			waitMotor0 = 1;
			mainStep = 5;
			break;
		case 50:
			SetDelayTime(1, 5);
			mainStep = 51;
		case 51:
			if(GetMotorMonitorState(MOT_CARD_LOAD,ZeroMonitor) == 1)		// �ж��������Ƿ�ص���λ
			{
				mainStep = 5;
			}
			else
			{
				mainStep = 4;
			}
		case 5:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4125);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char CardUnloadCheck(void){
	// ��Ƭж��С�����в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_CARD_UNLOAD,100,20,CURRENT_CARD_UNLOAD);
			MotInitCheck(MOT_CARD_UNLOAD);
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			SetDelayTime(1, 10);
			mainStep = 2;
			break;
		case 2:
			MotRunTo(MOT_CARD_UNLOAD, _POS_UNLOAD_OUT);
			waitMotor0 = 1;
			mainStep = 3;
		case 3:
			SetDelayTime(1, 10);
			mainStep = 4;
			break;
		case 4:
			MotRunTo(MOT_CARD_UNLOAD, 0);
			waitMotor0 = 1;
			mainStep = 5;
			break;
		case 5:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4126);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char DiluentPumpCheck(void){
	// ϡ��Һ�����в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_DILUENT, 100, 2, CURRENT_DILUENT);
			MotRun(MOT_DILUENT, 800);
			waitMotor0 = 1;
			mainStep = 1;
		case 1:
			SetDelayTime(1, 10);
			mainStep = 2;
			break;
		case 2:
			MotRun(MOT_DILUENT, 800);
			waitMotor0 = 1;
			mainStep = 3;
			break;
		case 3:
			SetMotRunPam(MOT_EFFLUENT, 200, 2, CURRENT_EFFLUENT);
			//MotRun(MOT_EFFLUENT, 8000);
			MotRun(MOT_EFFLUENT, 3000);
			waitMotor1 = 1;
			mainStep = 4;
			break;
		case 4:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4129);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char LeanerPumpCheck(void){
	// ��ϴҺ�����в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_FLUID, 80, 2, CURRENT_FLUID);
			MotRun(MOT_FLUID, 800);
			waitMotor0 = 1;
			mainStep = 1;
		case 1:
			SetDelayTime(1, 10);
			mainStep = 2;
			break;
		case 2:
			MotRun(MOT_FLUID, 800);
			waitMotor0 = 1;
			mainStep = 3;
			break;
		case 3:
			SetMotRunPam(MOT_EFFLUENT, 200, 2, CURRENT_EFFLUENT);
			//MotRun(MOT_EFFLUENT, 2000);
			MotRun(MOT_EFFLUENT, 3000);
			waitMotor1 = 1;
			mainStep = 4;
			break;
		case 4:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4130);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char EffluentPumpCheck(void){
	// ��Һ�����в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_EFFLUENT, 200, 2, CURRENT_EFFLUENT);
			MotRun(MOT_EFFLUENT, 800);
			waitMotor0 = 1;
			mainStep = 1;
		case 1:
			SetDelayTime(1, 10);
			mainStep = 2;
			break;
		case 2:
			MotRun(MOT_EFFLUENT, 800);
			waitMotor0 = 1;
			mainStep = 3;
			break;
		case 3:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4131);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}
/*
unsigned char SamplingSyringCheck(void){
	// ȡ��ע������в���
	unsigned char i;
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

//	i = MaintainSubFunParam ;
//	if(i==0)
//		i = 1;
	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_SAMP_PUMP, 200, 10, CURRENT_SAMP_PUMP);
			MotInitCheck(MOT_SAMP_PUMP);
			waitMotor0 = 1;
			mainStep = 1;
			i = 1;
			break;
		case 1:
			i--;
			SetDelayTime(1, 10);
			mainStep = 2;
			break;
		case 2:
			MotRunTo(MOT_SAMP_PUMP, 4800);
			waitMotor0 = 1;
			mainStep = 3;
		case 3:
			SetDelayTime(1, 10);
			mainStep = 4;
			break;
		case 4:
			MotRunTo(MOT_SAMP_PUMP, 0);
			waitMotor0 = 1;
			if(i != 0)
				mainStep = 1;
			else
				mainStep = 5;
			break;
		case 5:
			uart_Printf("%s\r\n",strM4132);
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}
*/
//2016-5-17�� �޸� ���£�
unsigned char SamplingSyringCheck(void){
	// ȡ��ע������в���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_SAMP_PUMP, 200, 10, CURRENT_SAMP_PUMP);
			MotInitCheck(MOT_SAMP_PUMP);
			waitMotor0 = 1;
			mainStep = 1;
			break;
		case 1:
			SetDelayTime(1, 10);
			mainStep = 2;
			break;
		case 2:
			MotRunTo(MOT_SAMP_PUMP, 4800);
			waitMotor0 = 1;
			mainStep = 3;
			break;
		case 3:
			SetDelayTime(1, 10);
			mainStep = 4;
			break;
		case 4:
			MotRunTo(MOT_SAMP_PUMP, 0);
			waitMotor0 = 1;
			mainStep = 5;
			break;
		case 5:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4132);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char LiquidPhotoCheck_Even(INFO_EVENT * pInfoEvent){
	unsigned char * pInfo;
	switch(mainStep){
		case 1:
			if(pInfoEvent->event == INFO_LIQ_PHO_VAL){
				pInfo = &(pInfoEvent->info[0]);
				// ���Һ·������Ϣ
				Uart0ReUnable;
				uart_Printf("%s $%4d $%4d $%4d $%4d\r\n",strM4133, *(pInfo), *(pInfo+1), *(pInfo+2), *(pInfo+3));
				Uart0ReEnable;
				mainStep = 2;
				return 1;
				}
			break;
		default:
			break;
		}
	return 0;
}
unsigned char LiquidPhotoCheck(void)
{
	switch(mainStep){
		case 0:
			GetLiquidPhotoInfo();
			mainStep = 1;
			RegisterLiquidEvenProcess(LiquidPhotoCheck_Even);
			break;
		case 2:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4134);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char CardStorePhotoCheck_Even(INFO_EVENT * pInfoEvent){
	unsigned char * pInfo;
	switch(mainStep){
		case 1:
			if(pInfoEvent->event == INFO_STORE_PHO_VOL){
				pInfo = &(pInfoEvent->info[0]);
			//	if(*(pInfo) == CardStorePhoNum){
					// ���Һ·����������
					Uart0ReUnable;
					uart_Printf("%s $%4d $%4d L $%4d H $%4d\r\n",strM4135, *(pInfo), *(pInfo+1), *(pInfo+2), *(pInfo+3));
					Uart0ReEnable;
					mainStep = 2;
					return 1;
			//		}
				}
			break;
		default:
			break;
		}
	return 0;
}
unsigned char CardStorePhotoCheck(void){
	switch(mainStep){
		case 0:
			GetStorePhoVol(CardStorePhoNum);
			mainStep = 1;
			RegisterCardStoreEvenProcess(CardStorePhotoCheck_Even);
			break;
		case 2:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strM4136, CardStorePhoNum);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}

unsigned char NeedleOnMixSideCheck(void){
	// ��ת�۲���
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotor1 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			waitMotor1 = 1;
			mainStep = 1;
			break;
		case 1:
			SetMotRunPam(MOT_SAMP_TRUN,120,40,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			waitMotor0 = 1;
			mainStep = 2;
			break;
		case 2:
			SetDelayTime(1, 5);
			mainStep = 3;
			break;
		case 3:
			MotRunTo(MOT_SAMP_TRUN, GetNeedleOnMixSidePos());
			waitMotor0 = 1;
			mainStep = 4;
		case 4:
			SetDelayTime(1, 5);
			mainStep = 5;
			break;
		case 5:
			MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP);
			waitMotor1 = 1;
			mainStep = 6;
			break;
		case 6:
			SetDelayTime(1, 30);
			mainStep = 7;
			break;
		case 7:
			MotRunTo(MOT_SAMP_NEEDLE, 0);
			waitMotor1 = 1;
			mainStep = 8;
			break;
		case 8:
			MotRunTo(MOT_SAMP_TRUN, 0);
			waitMotor0 = 1;
			mainStep = 9;
			break;
		case 9:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4137);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;
}
unsigned char DropHeightCheck(void){
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotor1 = 0;	}
	if(waitMotor2){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)	return 0;	waitMotor2 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			waitMotor1 = 1;
			mainStep = 1;
			break;
		case 1:
			SetMotRunPam(MOT_SAMP_TRUN,120,40,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			SetMotRunPam(MOT_CARD_LOAD,140,10,CURRENT_CARD_LOAD);
			MotInitCheck(MOT_CARD_LOAD);
			waitMotor0 = 1;
			mainStep = 2;
			break;
		case 2:
			SetDelayTime(1, 5);
			mainStep = 3;
			break;
		case 3:
			MotRunTo(MOT_CARD_LOAD, _POS_CARDLOAD_DROP);
			waitMotor2 = 1;
			mainStep = 4;
			break;
		case 4:
			SetDelayTime(1, 5);
			mainStep = 5;
			break;
		case 5:
			MotRunTo(MOT_SAMP_NEEDLE, GetDropHeight());
			waitMotor1 = 1;
			mainStep = 6;
			break;
		case 6:
			SetDelayTime(1, 30);
			mainStep = 7;
			break;
		case 7:
			MotRunTo(MOT_SAMP_NEEDLE, 0);
			waitMotor1 = 1;
			mainStep = 8;
			break;
		case 8:
			MotRunTo(MOT_CARD_LOAD, 0);
			waitMotor2 = 1;
			mainStep = 9;
			break;
		case 9:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4138);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;

}


unsigned char MixHeightCheck(void)
{
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotor1 = 0;	}
	if(waitMotor2){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)	return 0;	waitMotor2 = 0;	}

	switch(mainStep){
		case 0:
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			waitMotor1 = 1;
			mainStep = 1;
			break;
		case 1:
			SetMotRunPam(MOT_SAMP_TRUN,120,40,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			SetMotRunPam(MOT_CARD_LOAD,140,10,CURRENT_CARD_LOAD);
			MotInitCheck(MOT_CARD_LOAD);
			waitMotor0 = 1;
			mainStep = 2;
			break;
		case 2:
			SetDelayTime(1, 5);
			mainStep = 3;
			break;
		case 3:
			MotRunTo(MOT_CARD_LOAD, _POS_CARDLOAD_MIX);
			waitMotor2 = 1;
			mainStep = 4;
			break;
		case 4:
			SetDelayTime(1, 5);
			mainStep = 5;
			break;
		case 5:
			MotRunTo(MOT_SAMP_NEEDLE, GetMixHeight());
			waitMotor1 = 1;
			mainStep = 6;
			break;
		case 6:
			SetDelayTime(1, 30);
			mainStep = 7;
			break;
		case 7:
			MotRunTo(MOT_SAMP_NEEDLE, 0);
			waitMotor1 = 1;
			mainStep = 8;
			break;
		case 8:
			MotRunTo(MOT_CARD_LOAD, 0);
			waitMotor2 = 1;
			mainStep = 9;
			break;
		case 9:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4119);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		default:
			break;
		}
	return 0;

}


static unsigned int DiluentQuantifyVolume = 10*100;
void SetDiluentQuantifyVolume(unsigned char n){
	if(n > 50)
		n = 50;
	if(n == 0)
		n = 1;
	DiluentQuantifyVolume = n * 100;
}

unsigned int QuantifyTestCnt = 0;

unsigned char DiluentQuantifyTest(void){
	// ϡ��Һ�ö�������
	
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitMotor1 = 0;	}

	if(waitStartKey){
		if(quitFlag == 1){
			mainStep = 0;
			waitStartKey = 0;
			quitFlag = 0;
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4151);
			Uart0ReEnable;
			return 1;		// �����˳�
			}
		if(WaitStartKey()==0)
			return 0;
		waitStartKey = 0;	
		}
	
	switch(mainStep){
		case 0:
			quitFlag = 0;
			SetEValve(EV_ALL, EV_CLOSE);
			_EffluentMotRun(150,200);
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(40+5, 64);		// ע��40����λ��Һ
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(12+2, 64);		// ע��12+2����λ��Һ
#endif
			SetDelayTime(1, 30);
			mainStep = 1;
			break;
		case 1:
			MotStop(MOT_DILUENT);
			MotStop(MOT_EFFLUENT);
			QuantifyTestCnt = 0;
			mainStep = 2;
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4150);
			Uart0ReEnable;
			break;
		case 2:
			waitStartKey = 1;
			mainStep = 3;
			break;
		case 3:
			QuantifyTestCnt ++;
			MotRun(MOT_DILUENT, DiluentQuantifyVolume);
			waitMotor0 = 1;
			mainStep = 4;
			break;
		case 4:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strM4154, QuantifyTestCnt);
			Uart0ReEnable;
			MotRun(MOT_EFFLUENT, DiluentQuantifyVolume);
			waitMotor1 = 1;
			mainStep = 2;
			break;
		default:
			break;
		}
	return 0;
}

static unsigned int LeanerQuantifyVolume = 10*100;

void SetLeanerQuantifyVolume(unsigned char n){
	if(n > 15)
		n = 15;
	if(n == 0)
		n = 1;
	LeanerQuantifyVolume = n * 100;
}

unsigned char LeanerQuantifyTest(void){
	// ϡ��Һ�ö�������
	
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitMotor1 = 0;	}

	if(waitStartKey){
		if(quitFlag == 1){
			mainStep = 0;
			waitStartKey = 0;
			quitFlag = 0;
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4153);
			Uart0ReEnable;
			return 1;		// �����˳�
			}
		if(WaitStartKey()==0)
			return 0;
		waitStartKey = 0;	
		}
	
	switch(mainStep){
		case 0:
			quitFlag = 0;
			SetEValve(EV_ALL, EV_CLOSE);
			SetMotRunPam(MOT_EFFLUENT, 240, 2, CURRENT_EFFLUENT);
			MotRun(MOT_EFFLUENT, 150*100);
			SetMotRunPam(MOT_FLUID, 60, 2, CURRENT_FLUID);
			MotRun(MOT_FLUID, 70*100);			// ����ϡ��Һ��
			SetDelayTime(1, 30);
			mainStep = 1;
			break;
		case 1:
			MotStop(MOT_FLUID);
			MotStop(MOT_EFFLUENT);
			QuantifyTestCnt = 0; 
			mainStep = 2;
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM4152);
			Uart0ReEnable;
			break;
		case 2:
			waitStartKey = 1;
			mainStep = 3;
			break;
		case 3:
			QuantifyTestCnt ++;
			MotRun(MOT_FLUID, LeanerQuantifyVolume);
			waitMotor0 = 1;
			mainStep = 4;
			break;
		case 4:
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strM4154, QuantifyTestCnt);
			Uart0ReEnable;
			MotRun(MOT_EFFLUENT, LeanerQuantifyVolume);
			waitMotor1 = 1;
			mainStep = 2;
			break;
		default:
			break;
		}
	return 0;
}

static unsigned int SampQuantifyVolume = 40;
void SetSampQuantifyVolume(unsigned char n){
	if(n > 120)
		n = 120;
	if(n <5)
		n = 5;
	SampQuantifyVolume = n * 4;
}

unsigned char SampQuantifyTest(void){
	static unsigned int liqDetBaseAdc;
	unsigned int i;
	// ȡ���ö�������
	if(WaitDelayTime(1))		return 0;
	if(waitMotor0){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotor0 = 0;	}
	if(waitMotor1){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotor1 = 0;	}
	if(waitMotor2){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotor2 = 0;	}
	if(waitMotor3){	if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)			return 0;	waitMotor3 = 0;	}
	if(waitMotor4){	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitMotor4 = 0;	}

	if(waitStartKey){
		if(quitFlag == 1){
			mainStep = 3;
			workStep = 0;
			waitStartKey = 0;
	//		quitFlag = 0;
	//		uart_Printf("*4151 DiluentQuantifyTestQuit\r\n");
	//		return 1;		// �����˳�
			}
		if(WaitStartKey()==0)
			return 0;
		waitStartKey = 0;	
		}
	
	switch(mainStep){
		case 0:		// ׼������
			switch(workStep){
				case 0:
					quitFlag = 0;
					SetEValve(EV_ALL, EV_CLOSE);
					SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
					MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotor2 = 1;
					workStep = 1;
					break;
				case 1:
					SetMotRunPam(MOT_SAMP_TRUN, 32, 20, CURRENT_SAMP_TRUN);
					MotInitCheck(MOT_SAMP_TRUN);
					waitMotor1 = 1;
					workStep = 2;
					break;
				case 2:
					MotRunTo(MOT_SAMP_TRUN,_POS_NEEDLE_ON_MIXCENTRE);
					waitMotor1 = 1;
					workStep = 3;
					break;
				case 3:	// 
					MotRunTo(MOT_SAMP_NEEDLE, 1000);
					waitMotor2 = 1;
					workStep = 4;
					timeOut = 0;
					break;
				case 4:	// ��עȡ�����·
					SetEValve(EV4, EV_OPEN);
					SetMotRunPam(MOT_FLUID, 40, 2, CURRENT_FLUID);
					MotRun(MOT_FLUID, 5000);
					SetMotRunPam(MOT_EFFLUENT, 60, 2, CURRENT_EFFLUENT);
					MotRun(MOT_EFFLUENT, 9000);
					waitMotor3 = 1;
					waitMotor4 = 1;
					workStep = 5;
					break;
				case 5:
				//	MotStop(MOT_EFFLUENT);
					SetEValve(EV_ALL, EV_CLOSE);
					workStep = 6;
					break;
				case 6:	// ����ȳ�ע��ϡ��Һ��ֱ��ȡ����߶�λ��
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					if(liqDetBaseAdc > 500)
					{
						Uart0ReUnable;
						uart_Printf("%s $%d\r\n",strM2150, liqDetBaseAdc);
						Uart0ReEnable;
					}
					else
					{
						Uart0ReUnable;
						uart_Printf("%s $%d\r\n",strE2950, liqDetBaseAdc);
						Uart0ReEnable;
						// �˳�����
						mainStep = 3;
						workStep = 0;
						break;
					}
					SetMotRunPam(MOT_DILUENT, 64, 2, CURRENT_DILUENT);
					MotRun(MOT_DILUENT, 80*100);			// ����ϡ��Һ��
					workStep = 7;
					break;
				case 7:
					if(GetMotState(MOT_DILUENT)==STA_SLAVE_FREE){
						timeOut ++;
						if(timeOut<3){
							workStep = 6;
							}
						else{
							// �˳�����
							mainStep = 3;
							workStep = 0;
							Uart0ReUnable;
//							uart_Printf("%s\r\n",strE2910);
							uart_Printf("%s\r\n",strE3904);
							Uart0ReEnable;
							break;
							}
						}
					i = getLiqDetADC(NeedleChannel);
					if(i<liqDetBaseAdc){
						i = liqDetBaseAdc - i;
						if(i>200){
							MotStop(MOT_DILUENT);
							MotRun(MOT_SAMP_NEEDLE, 40);
							SetMotRunPam(MOT_SAMP_PUMP, 32, 2, CURRENT_SAMP_PUMP);
							MotInitCheck(MOT_SAMP_PUMP);
							waitMotor0 = 1;
							waitMotor2 = 1;
							workStep = 0;
							mainStep = 1;
							}
						}
					break;
				default:
					break;
				}
			break;
		case 1:		// ��ȡҺ��
			switch(workStep){
				case 0:
					waitStartKey = 1;
					workStep = 1;
					break;
				case 1:
					MotRun(MOT_SAMP_PUMP,SampQuantifyVolume + 15);
					waitMotor0 = 1;
					workStep = 2;
					break;
				case 2:
					MotRun(MOT_SAMP_PUMP,-5);
					waitMotor0 = 1;
					workStep = 3;
					break;
				case 3:
					SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					waitMotor2 = 1;
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
					waitMotor1 = 1;
					workStep = 5;
					break;
				case 5:
					MotRunTo(MOT_SAMP_NEEDLE, 3600);
					waitMotor2 = 1;
					workStep = 0;
					mainStep = 2;
					break;
				}
			break;
		case 2:
			switch(workStep){
				case 0:
					waitStartKey = 1;
					workStep = 1;
					break;
				case 1:
					MotRun(MOT_SAMP_PUMP,-SampQuantifyVolume);
					waitMotor0 = 1;
					workStep = 2;
					break;
				case 2:
					SetDelayTime(1, 5);
					workStep = 3;
					break;
				case 3:
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					waitMotor2 = 1;
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotor1 = 1;
					workStep = 5;
					break;
				case 5:
					MotRunTo(MOT_SAMP_TRUN, _POS_NEEDLE_ON_MIXCENTRE);
					waitMotor1 = 1;
					workStep = 6;
					break;
				case 6:
					MotRunTo(MOT_SAMP_NEEDLE, 800);
					waitMotor2 = 1;
					workStep = 7;
					break;
				case 7:
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					if(liqDetBaseAdc > 500){
						Uart0ReUnable;
						uart_Printf("%s $%d\r\n",strM2150, liqDetBaseAdc);
						Uart0ReEnable;
						}
					else{
						Uart0ReUnable;
						uart_Printf("%s $%d\r\n",strE2950, liqDetBaseAdc);
						Uart0ReEnable;
						// �˳�����
						mainStep = 3;
						workStep = 0;
						break;
						}
					SetMotRunPam(MOT_SAMP_NEEDLE, 64, 2, CURRENT_SAMP_NEEDLE);
					MotRun(MOT_SAMP_NEEDLE, 800);			// ����ϡ��Һ��
					workStep = 8;
					break;
				case 8:
					if(GetMotState(MOT_SAMP_NEEDLE)==STA_SLAVE_FREE){
						Uart0ReUnable;
					//	uart_Printf("%s\r\n",strE2910);
						uart_Printf("%s\r\n",strE3904);
						Uart0ReEnable;
						// �˳�����
						mainStep = 3;
						workStep = 0;
						break;
						}
					i = getLiqDetADC(NeedleChannel);
					if(i<liqDetBaseAdc){
						i = liqDetBaseAdc - i;
						if(i>200){
							MotStop(MOT_SAMP_NEEDLE);
							MotRun(MOT_SAMP_NEEDLE, 40);
							SetMotRunPam(MOT_SAMP_PUMP, 32, 2, CURRENT_SAMP_PUMP);
							MotInitCheck(MOT_SAMP_PUMP);
							waitMotor0 = 1;
							waitMotor2 = 1;
							workStep = 0;
							mainStep = 1;
							}
						}
					break;
				}
			break;
		case 3:		// �˳�
			switch(workStep){
				case 0:
					SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					SetMotRunPam(MOT_EFFLUENT, 240, 2, CURRENT_EFFLUENT);
					MotRun(MOT_EFFLUENT, 1500);
					waitMotor2 = 1;
					workStep = 1;
					break;
				case 1:
					SetMotRunPam(MOT_SAMP_TRUN, 32, 20, CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotor1 = 1;
					workStep = 2;
					break;
				case 2:
					mainStep = 0;
					workStep = 0;
					waitStartKey = 0;
					quitFlag = 0;
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM4151);
					Uart0ReEnable;
					return 1;
					break;
				}
			break;
		default:
			break;
		}
	return 0;
}

