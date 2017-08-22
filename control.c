

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"


extern  unsigned char _SampSW;         
unsigned char _LEDSTATE = 0;   //0表示测试过程是红灯，1表示测试过程是绿灯


typedef struct _MOTOR_POSITION{
	unsigned char defPosNum;	// 预定义位置
	int stepPos;		// 步进数位置
}MOTOR_POSITION;


extern unsigned char ControlModel;
extern unsigned char WorkProcessStep;		// 工作进程号


unsigned char (*EvenPosChangeProcess)(INFO_EVENT * pInfoEvent);
unsigned char (*EvenLiquidProcess)(INFO_EVENT * pInfoEvent);
unsigned char (*EvenCardStoreProcess)(INFO_EVENT * pInfoEvent);

MOTOR_POSITION MotorPosition[SLAVE_NUM];
unsigned char MotModulePhoSta[SLAVE_NUM][2];	// 电机模块光藕状态信息
unsigned char LiquidState[4][2];	// 液路状态和参数
unsigned char StoreHumi;			// 片仓湿度
unsigned char StoreTemp;			// 片仓温度

unsigned char _RingPieceState[RING_QUEUE_NUM];	// 转盘干片状态, 0:空, 1:存在, 255:无效


// 数据初始化
void InitControlLayerData(void){
	unsigned char i;
	for(i=0; i<SLAVE_NUM; i++){
		MotorPosition[i].defPosNum = 0xf0;
		MotorPosition[i].stepPos = 0x7fff;
		}
	EvenPosChangeProcess = 0;
	EvenLiquidProcess = 0;
	EvenCardStoreProcess = 0;
}

unsigned int time;
unsigned int uart1ReceiveOutTime;
//test for github


unsigned char GetRingPieceState(unsigned char n){
	if(n>=RING_QUEUE_NUM)
		return 0xff;
	return _RingPieceState[n];
}

// 温湿度
unsigned char GetStoreHumi(void){
	return StoreHumi;
}
unsigned char GetStoreTemp(void){
	return StoreTemp;
}

// 电机位置
void SetMotPosIdle(unsigned char slaveNum)
{
	MotorPosition[slaveNum].defPosNum = 0xf0;
//	MotorPosition[slaveNum].stepPos = 0x7fff;
}
unsigned char GetMotPositionOfSite(unsigned char slaveNum){
	return MotorPosition[slaveNum].defPosNum;
}
int GetMotPositionOfStep(unsigned char slaveNum){
	return MotorPosition[slaveNum].stepPos;
}

// 获取液路状态
unsigned char GetLiquidMonitorState(unsigned char num){
	if(num>3)
		num = 3;
	return LiquidState[num][0];
}
unsigned char GetLiquidMonitorStatePam(unsigned char num){
	if(num>3)
		num = 3;
	return LiquidState[num][1];
}


// 注册位置改变自动处理函数
unsigned char RegisterPosChangeEvenProcess(void * proc){
	EvenPosChangeProcess = proc;
}
void CleanPosChangeEvenProcess(void){
	EvenPosChangeProcess = 0;
}
// 注册液路事件处理函数
unsigned char RegisterLiquidEvenProcess(void * proc){
	EvenLiquidProcess = proc;
}
void CleanLiquidEvenProcess(void){
	EvenLiquidProcess = 0;
}
// 注册片仓事件处理函数
unsigned char RegisterCardStoreEvenProcess(void * proc)
{
	EvenCardStoreProcess = proc;
}
void CleanPosCardStoreProcess(void)
{
	EvenCardStoreProcess = 0;
}

// 上传指定传感器状态信息
extern unsigned char CardSurplusState[6];	// 卡片剩余状态
extern unsigned char CardStoretate[6];		// 片仓状态
void UpLoadingModuleSensorState(unsigned char slaveNum, unsigned char num){
	char s[2][2] = {"0","1"};
	unsigned char i;
	switch(slaveNum){
		case 0:		// 主控制板上的信号
			switch(num){
				case 0:		// J4 取片检测开关信号
					i = (PINL & 0x04);
					if(i!=0)
						i = 1;
					Uart0ReUnable;
					uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
					Uart0ReEnable;
					break;
				case 1:		// J7 转盘干片检测光藕信号
					i = PINK & 0x01;
					if(i!=0)
						i = 1;
					Uart0ReUnable;
					uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
					Uart0ReEnable;
					break;
				case 2:		// J8 废片盒开关信号
					if(GetwasteCardState() == 0)// 废片仓功能开启
					{
						i = PINK & 0x02;
						if(i!=0)
							i = 1;
						Uart0ReUnable;
						uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
						Uart0ReEnable;
					}
					break;
				case 3:		// J10 液面传感器信号
					Uart0ReUnable;
					uart_Printf("%s $%2d $%2d $%4d\r\n",strM4201, slaveNum, num, getLiqDetADC(NeedleChannel));
					Uart0ReEnable;
					break;
				case 4:		// J12 吸样开关信号
					i = PINJ & 0x40;
					if(i!=0)
						i = 1;
					Uart0ReUnable;
					uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
					Uart0ReEnable;
					break;
				}
			break;
		case MOT_TURN_PLATE:
		case MOT_SAMP_TRUN:
		case MOT_SAMP_NEEDLE:
		case MOT_CARD_LOAD:
		case MOT_CARD_UNLOAD:
		case MOT_STORE_CARD_MOVE:
		case MOT_SAMP_PUMP:
			if(num>1)
				num = 1;
			i = MotModulePhoSta[slaveNum][num];
			if(i!=0)
				i = 1;
			// 发送电机模块光藕信号
			Uart0ReUnable;
			uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
			Uart0ReEnable;
			break;
		case LIQUID_CONTROL:
			if(num>2)
				num = 2;
			// 发送液路状态信号
			if(LiquidState[num][0] == INFO_LIQ_FULL)
				i = 1;
			else
				i = 0;
			Uart0ReUnable;
			uart_Printf("%s $%2d $%2d $%c\r\n",strM4201, slaveNum, num, s[i][0]);
			Uart0ReEnable;
			break;
		case STORE_MONITOR:
			if(num>4)
				num = 4;
			if(CardStoretate[num] == INFO_STORE_OPEN)
			{
				Uart0ReUnable;
				uart_Printf("%s $%2d $%2d $%4d\r\n",strM4201, slaveNum, num, CardStoretate[num]);
				Uart0ReEnable;
			}
			else
			{
				Uart0ReUnable;
				uart_Printf("%s $%2d $%2d $%4d\r\n",strM4201, slaveNum, num, CardSurplusState[num]);
				Uart0ReEnable;
			}
			break;
		default:
			break;
		}
}
/*
unsigned char GetTurnPlateMonitorState(void)
{	
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3175,MotModulePhoSta[MOT_TURN_PLATE][1]);
	Uart0ReEnable;
	return MotModulePhoSta[MOT_TURN_PLATE][1];
}
*/

signed char GetMotorMonitorState(unsigned char slave,unsigned char num)
{
	if(slave == 0 || slave == 6 || slave == 7)	
		return -1;
	if(num > 1)
		num = 1;
	Uart0ReUnable;
	uart_Printf("%s",strM3175);
	switch(slave)
	{
		case 1:uart_Printf(":MOT_TURN_PLATE");break;
		case 2:uart_Printf(":MOT_SAMP_TRUN");break;
		case 3:uart_Printf(":MOT_SAMP_NEEDLE");break;
		case 4:uart_Printf(":MOT_CARD_LOAD");break;
		case 5:uart_Printf(":MOT_CARD_UNLOAD");break;
		case 8:uart_Printf(":MOT_STORE_CARD_MOVE");break;
		case 9:uart_Printf(":MOT_DILUENT");break;
		case 10:uart_Printf(":MOT_FLUID");break;
		case 11:uart_Printf(":MOT_EFFLUENT");break;
		case 12:uart_Printf(":MOT_SAMP_PUMP");break;
		case 13:uart_Printf(":LIQUID_CONTROL");break;
		case 14:uart_Printf(":STORE_MONITOR");break;
		default:break;
	}
	uart_Printf("$%2d $%2d\r\n",num,MotModulePhoSta[slave][num]);
	//uart_Printf("%s:%s.%s $%2d\r\n",strM3175,SlaveID[slave],Monitor[num],MotModulePhoSta[MOT_TURN_PLATE][num]);
	Uart0ReEnable;
	return MotModulePhoSta[slave][num];
}

// 上传所有传感器信息
void UpLoadingAllSensorState(void){
	char s[2][2] = {"0","1"};
	unsigned char i,n, f;
	Uart0ReUnable;
	uart_Printf("%s",strM4202);	// 发送开始
	Uart0ReEnable;
	// 发送电机模块光藕信息
	for(i=1; i<=12; i++)
	{
		if(i==6 || i==7)
			continue;
		for(n=0; n<2; n++)
		{
			f = MotModulePhoSta[i][n];
			if(f!=0)
				f = 1;
			// 发送电机模块光藕信号
			Uart0ReUnable;
			uart_Printf(" $%c",s[f][0]);
			Uart0ReEnable;
		}
	}
	// 发送液路模块信息
	i = 13;
	for(n=0; n<3; n++)
	{
		if(LiquidState[n][0] == INFO_LIQ_FULL)
			f = 1;
		else
			f = 0;
		Uart0ReUnable;
		uart_Printf(" $%c",s[f][0]);
		Uart0ReEnable;
	}
	// 发送片仓模块信息
	i = 14;
	for(n=0; n<5; n++){
		if(CardStoretate[n] == INFO_STORE_OPEN)
		{
			Uart0ReUnable;
			uart_Printf(" $%2d",CardStoretate[n]);
			Uart0ReEnable;
		}
		else
		{
			Uart0ReUnable;
			uart_Printf(" $%2d",CardSurplusState[n]);
			Uart0ReEnable;
		}
	}
	//发送其它模块信息
	i = 0;
	for(n=0; n<5; n++){
		switch(n){
			case 0:		// J4 取片检测开关信号
				f = (PINL & 0x04);
				if(f!=0)
					f = 1;
				Uart0ReUnable;
				uart_Printf(" $%c",s[f][0]);
				Uart0ReEnable;
				break;
			case 1:		// J7 转盘干片检测光藕信号
				f = PINK & 0x01;
				if(f!=0)
					f = 1;
				Uart0ReUnable;
				uart_Printf(" $%c",s[f][0]);
				Uart0ReEnable;
				break;
			case 2:		// J8 废片盒开关信号
				if(GetwasteCardState() == 0)
				{
					f = PINK & 0x02;
					if(f!=0)
						f = 1;
					Uart0ReUnable;
					uart_Printf(" $%c",s[f][0]);
					Uart0ReEnable;
				}
				break;
			case 3:		// J10 液面传感器信号
				Uart0ReUnable;
				uart_Printf(" $%4d",getLiqDetADC(NeedleChannel));
				Uart0ReEnable;
				break;
			case 4:		// J12 吸样开关信号
				f = PINJ & 0x40;
				if(f!=0)
					f = 1;
				Uart0ReUnable;
				uart_Printf(" $%c",s[f][0]);
				Uart0ReEnable;
				break;
			}
		}
	uart_Printf("\r\n");	// 发送结束
}


//git test comment for branch "old"

// 从机事件处理和分配
void SlaveEventAssignProcess(INFO_EVENT * pInfoEvent){
	// 事件分配处理
	unsigned char curSlave, infoLen;
	unsigned char * pInfo, *p;
	int pos;
	unsigned char even;
	unsigned char i, j;
	static unsigned char pieceState;

	curSlave = pInfoEvent->slaveNum;
	even = pInfoEvent->event;
	pInfo = &(pInfoEvent->info[0]);
	
	switch(even)
	{
		case STA_CHANGE_POS:
		case STA_INFO_POS:
			pos = *(pInfo+1);
			pos = pos << 8;
			pos += *(pInfo+2);
			MotorPosition[curSlave].defPosNum = *(pInfo);
			MotorPosition[curSlave].stepPos = pos;
			if(EvenPosChangeProcess){
				i = EvenPosChangeProcess(pInfoEvent);
				if(i)
					EvenPosChangeProcess = 0;
				}
			break;
		case STA_MOT_PHO:	// 电机模块光藕状态改变信息
			MotModulePhoSta[curSlave][*(pInfo)] = *(pInfo+1);
			break;

		case INFO_LIQ_EMPTY:		// 液路空
		case INFO_LIQ_FULL:			// 液路满
			i = *pInfo;
			if(i<4){
				LiquidState[i][0] = even;
				LiquidState[i][1] = 0;
				}
			break;
		case INFO_LIQ_BUBBLE:		// 有气泡
		case INFO_LIQ_FLOW:			// 有液段
			i = *pInfo;
			if(i<4){
				LiquidState[i][0] = even;
				LiquidState[i][1] = *(pInfo+1);
				}
			break;
		case INFO_LIQ_PHO_ON:		// 液路检测光耦检测到液体
		case INFO_LIQ_PHO_OFF:		// 液路检测光耦检测到空
		case INFO_LIQ_PHO_VAL:		// 液路光耦输出信号值
		case INFO_LIQ_PHO_ADJ:		// 液路光耦调整结果
			switch(WorkProcessStep){
				case 0:		// 空闲
					break;
				case 1:		// 机械自检
					break;
				case 2:		// 液路自检
					DiluteStartCheck(pInfoEvent);
					break;
				case 3:		// 正常测试
					DiluteProcess(pInfoEvent);
					break;
				case 4:		// 调试维护
					break;
				}
			if(EvenLiquidProcess){
				i = EvenLiquidProcess(pInfoEvent);
				if(i)
					EvenLiquidProcess = 0;
				}
			break;
		case INFO_STORE_OPEN:		// 片仓打开
		case INFO_STORE_CLOSE:		// 片仓关闭
		case INFO_STORE_FULL:		// 片仓满
		case INFO_STORE_LITTLE:		// 片仓少量
		case INFO_STORE_EMPTY:		// 片仓空
		case INFO_STORE_ERROR:		// 片仓状态错误
		case INFO_STORE_CAL:		// 片仓光路校准信息
		case INFO_STORE_PHO_VOL:
		case INFO_STORE_STATE_ALL:	// 全部片仓状态信息
		case INFO_STORE_STATE_SPC:	// 指定片仓状态信息
			CardStoreSteteProcess(pInfoEvent);
			if(EvenCardStoreProcess)
			{
				i = EvenCardStoreProcess(pInfoEvent);
				if(i)
					EvenCardStoreProcess = 0;
			}
			break;
		case INFO_STORE_HUMITURE:	// 片仓温湿度
			StoreHumi = *pInfo;
			StoreTemp = *(pInfo+1);
			Uart0ReUnable;
			uart_Printf("%s $%4d $%4d\r\n",strM0111, StoreHumi, StoreTemp);
			Uart0ReEnable;
			break;
		case INFO_STORE_OPEN_ERR:	// 片仓开启超时错误
			Uart0ReUnable;
			uart_Printf("%s $%4d\r\n",strE0910, *pInfo);
			Uart0ReEnable;
			break;
		default:
			break;
		}

}

/*********************************************************************************************/

//git test commet 2017/08/22

/****************************************************************************************************/
// 机械运行初始化
/*
unsigned char MachinePositionInit(void){
	// 机械位置初始化
	static unsigned char mainStep;		
	static unsigned char waitMotSampTurn,waitMotSampNeedle, waitMotSampPump;
	static unsigned char waitMotCardTrolley, waitMotCardLoad, waitMotCardUnLoad, waitMotTurnPlate;
	static unsigned char waitMotLifterA, waitMotLifterB;
	static unsigned char i, n, m;


	if(WaitDelayTime(MOT_SAMP_PUMP))		return 0;
	if(WaitDelayTime(MOT_SAMP_TRUN))		return 0;
	if(WaitDelayTime(MOT_SAMP_NEEDLE))		return 0;
	if(WaitDelayTime(MOT_EFFLUENT))			return 0;
		
	if(waitMotSampTurn){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
	if(waitMotSampNeedle){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
	if(waitMotSampPump){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotSampPump = 0;	}
	if(waitMotCardLoad){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)		return 0;	waitMotCardLoad = 0;	}
	if(waitMotCardUnLoad){	if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;	waitMotCardUnLoad = 0;	}
	if(waitMotTurnPlate){	if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)		return 0;	waitMotTurnPlate = 0;	}
	if(waitMotCardTrolley){	if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)	return 0;	waitMotCardTrolley = 0;	}
	
	switch(mainStep){
		case 0:		// 设置运行参数
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			SetMotRunPam(MOT_CARD_LOAD,160,20,CURRENT_CARD_LOAD);
			SetMotRunPam(MOT_CARD_UNLOAD,160,20,CURRENT_CARD_UNLOAD);
			SetMotRunPam(MOT_SAMP_PUMP,180,10,CURRENT_SAMP_PUMP);
			
			MotInitCheck(MOT_SAMP_NEEDLE);
			MotInitCheck(MOT_CARD_UNLOAD);
			MotInitCheck(MOT_CARD_LOAD);
			MotInitCheck(MOT_SAMP_PUMP);
			SetCardTrolleyState(0);
			SetEValve(EV1, EV_OPEN);
			SetMotRunPam(MOT_EFFLUENT, 180, 2, CURRENT_EFFLUENT);
			MotRun(MOT_EFFLUENT, 2500);			// 开启废液泵
			waitMotCardUnLoad = 1;
			mainStep = 1;
			break;
		case 1:		// 转盘位置初始化
			SetMotRunPam(MOT_TURN_PLATE,240,20,CURRENT_TURN_PLATE);
			MotInitCheck(MOT_TURN_PLATE);
			waitMotTurnPlate = 1;
			mainStep = 2;
			break;
		case 2:		// 转盘运行到0位
			MotRunToSite(MOT_TURN_PLATE,0);
			waitMotTurnPlate = 1;
			waitMotSampNeedle = 1;
			mainStep = 3;
			break;
		case 3:		// 片仓小车运行到起始位
			SetMotRunPam(MOT_STORE_CARD_MOVE,200,10,CURRENT_STORE_MOVE);
			MotInitCheck(MOT_STORE_CARD_MOVE);
			waitMotSampNeedle = 1;
			mainStep = 100;
			break;
		case 100:		// 取样针回零经常出现在运转时给出空闲信号, 此处从新运行以避免错误
			MotRunTo(MOT_SAMP_NEEDLE, 0);
			waitMotSampNeedle = 1;
			mainStep = 4;
			break;
		case 4:		// 取样臂回到起始位
			SetEValve(EV_ALL, EV_CLOSE);
			SetMotRunPam(MOT_SAMP_TRUN,200,5,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			SetEValve(EV1, EV_CLOSE);
			waitMotSampTurn = 1;
			waitMotSampPump = 1;
			waitMotTurnPlate = 1;
			//mainStep = 5;
			mainStep = 7;
			i = 0;
			break;
		case 5:	// 取样臂旋转试运行
			MotRunTo(MOT_SAMP_TRUN,_POS_SAMPTURN_SAMP+200);
			waitMotSampTurn = 1;
			mainStep = 6;
			break;
		case 6:
			MotRunTo(MOT_SAMP_TRUN,0);
			waitMotSampTurn = 1;
			i++;
			if(i<1){
				mainStep = 5;
				SetDelayTime(MOT_SAMP_TRUN, 10);
				}
			else{	
				mainStep = 7;	
				i = 0;	
				}
			break;
		case 7:		// 扫描转盘上遗留干片	_RingPieceState
			i++;
			if(i>=RING_QUEUE_NUM)
				i = 0;
			m = PINK & 0x01;
			_RingPieceState[i] = 0;
			MotRunToSite(MOT_TURN_PLATE,i);
			mainStep = 8;
			break;
		case 8:
			n = PINK & 0x01;
			if(m != n)
				_RingPieceState[i] = 1;
			if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	
				break;
			if(i==0)
				mainStep = 9;
			else
				mainStep = 7;
			break;
		case 9:		// 扫描转盘
			MotRunToSite(MOT_TURN_PLATE,10);	// 25 让转盘的0号转到卸片位置
			waitMotTurnPlate = 1;
			i = 0;
			mainStep = 10;
			break;
		case 10:		// 按顺序寻找剩余干片
			n = i + 10;
			if(n>=RING_QUEUE_NUM)
				n -= RING_QUEUE_NUM;
			if(GetRingPieceState(i)==1){	// 转盘上有剩余干片
				MotRunToSite(MOT_TURN_PLATE,n);	// 转盘运行到当前位置
				waitMotTurnPlate = 1;
				mainStep = 11;
				m = 0;		// 废片仓打开计时
				}
			else
				mainStep = 14;		// 继续查找下一个
			break;
		case 11:		// 开始卸载转盘上的干片
			if((PINK & 0x02) == 0){
				SetDelayTime(MOT_TURN_PLATE, 10);
				m ++;
				if(m == 2){
					uart_Printf("%s $   1\r\n",strM0200);
					SetBeepWarning();
					}
				if(m == 25){
					SetBeepWarning();
					m = 5;
					}
				
				break;
				}
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);
			waitMotCardUnLoad = 1;
			mainStep = 12;
			break;
		case 12:
			SetDelayTime(MOT_SAMP_TRUN, 10);
			mainStep = 13;
			break;
		case 13:	// 卸片小车回到起始点
			MotRunTo(MOT_CARD_UNLOAD,0);
			waitMotCardUnLoad = 1;
			mainStep = 14;
			break;
		case 14:
			i++;
			if(i < RING_QUEUE_NUM)	// 未完继续
				mainStep = 10;
			else
				mainStep = 15;	// 检查结束
			break;
		case 15:	// 检查片仓小车上是否有干片
			SetMotRunPam(MOT_STORE_CARD_MOVE,64,10,CURRENT_STORE_MOVE);
			MotRunTo(MOT_STORE_CARD_MOVE,100);
			waitMotCardTrolley = 1;
			mainStep = 16;
			break;
		case 16:
			i = PINL & 0x04;		// 读取取片检测传感器空闲状态
			MotRunTo(MOT_STORE_CARD_MOVE,0);	// 取片小车运行到零位
			waitMotCardTrolley = 1;
			mainStep = 17;
			break;
		case 17:
			n = PINL & 0x04;		// 读取取片检测传感器状态
			if(i != n){		// 取片小车上有剩余干片
				MotRunToSite(MOT_TURN_PLATE,0);	// 转盘运行到0号位置
				waitMotTurnPlate = 1;
				mainStep = 18;
				}
			else
				mainStep = 26;		// 取片小车上没有干片, 跳过
			break;
		case 18:		// 干片推入转盘
			SetMotRunPam(MOT_CARD_LOAD,160,10,CURRENT_CARD_LOAD);
			MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_RING);		// 装片行程94mm/0.08128 = 1156
			waitMotCardLoad = 1;
			mainStep = 20;
			break;
		case 19:		// 干片推入转盘步骤2
		//	SetMotRunPam(MOT_CARD_LOAD,64,2,2);
			MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_RING);		// 装片行程94mm/0.08128 = 1156
			waitMotCardLoad = 1;
			mainStep = 20;
			break;
		case 20:
			SetDelayTime(MOT_SAMP_TRUN, 10);
			mainStep = 21;
			break;
		case 21:		// 干片推入复位
			SetMotRunPam(MOT_CARD_LOAD,200,10,CURRENT_CARD_LOAD);
			MotRunTo(MOT_CARD_LOAD,0);
			waitMotCardLoad = 1;
			mainStep = 22;
			break;
		case 22:		// 转盘0号转到卸片位置
		//	SetMotRunPam(MOT_TURN_PLATE,240,10,2);
			MotRunToSite(MOT_TURN_PLATE,25);
			waitMotTurnPlate = 1;
			mainStep = 23;
			break;
		case 23:		// 开始卸片
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);
			waitMotCardUnLoad = 1;
			mainStep = 24;
			break;
		case 24:
			SetDelayTime(MOT_SAMP_TRUN,10);
			mainStep = 25;
			break;
		case 25:		// 卸片小车回到起始位
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,2);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_HOME);
			waitMotCardUnLoad = 1;
			mainStep = 26;
			break;
		case 26:		// 转盘运转到初始状态
			MotRunToSite(MOT_TURN_PLATE,29);
			waitMotTurnPlate = 1;
			MotRunTo(MOT_STORE_CARD_MOVE,432);
			waitMotCardTrolley = 1;
			mainStep = 27;
			break;
		case 27:
			uart_Printf("%s\r\n", strM1101);
			mainStep = 0;
			return 1;
			break;
		}
	return 0;
}
*/

unsigned char MachinePositionInit(void){
	// 机械位置初始化
	static unsigned char mainStep;		
	static unsigned char waitMotSampTurn,waitMotSampNeedle, waitMotSampPump;
	static unsigned char waitMotCardTrolley, waitMotCardLoad, waitMotCardUnLoad, waitMotTurnPlate;
	static unsigned char waitMotLifterA, waitMotLifterB;
	static unsigned char i, n, m;


	if(WaitDelayTime(MOT_SAMP_PUMP))		return 0;
	if(WaitDelayTime(MOT_SAMP_TRUN))		return 0;
	if(WaitDelayTime(MOT_SAMP_NEEDLE))		return 0;
	if(WaitDelayTime(MOT_EFFLUENT))			return 0;
		
	if(waitMotSampTurn){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
	if(waitMotSampNeedle){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
	if(waitMotSampPump){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotSampPump = 0;	}
	if(waitMotCardLoad){	if(GetMotState(MOT_CARD_LOAD)!=STA_SLAVE_FREE)		return 0;	waitMotCardLoad = 0;	}
	if(waitMotCardUnLoad){	if(GetMotState(MOT_CARD_UNLOAD)!=STA_SLAVE_FREE)	return 0;	waitMotCardUnLoad = 0;	}
	if(waitMotTurnPlate){	if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)		return 0;	waitMotTurnPlate = 0;	}
	if(waitMotCardTrolley){	if(GetMotState(MOT_STORE_CARD_MOVE)!=STA_SLAVE_FREE)	return 0;	waitMotCardTrolley = 0;	}
	
	switch(mainStep)
	{
		case 0:		// 设置运行参数
			SetMotRunPam(MOT_SAMP_NEEDLE,240,10,CURRENT_SAMP_NEEDLE);
			SetMotRunPam(MOT_CARD_LOAD,160,20,CURRENT_CARD_LOAD);
			SetMotRunPam(MOT_CARD_UNLOAD,160,20,CURRENT_CARD_UNLOAD);
			SetMotRunPam(MOT_SAMP_PUMP,180,10,CURRENT_SAMP_PUMP);
			
			MotInitCheck(MOT_SAMP_NEEDLE);
			MotInitCheck(MOT_CARD_UNLOAD);
			MotInitCheck(MOT_CARD_LOAD);
			MotInitCheck(MOT_SAMP_PUMP);
			SetCardTrolleyState(0);
			SetEValve(EV1, EV_OPEN);
			SetMotRunPam(MOT_EFFLUENT, 180, 2, CURRENT_EFFLUENT);
			MotRun(MOT_EFFLUENT, 2500);			// 开启废液泵
			waitMotCardUnLoad = 1;
			mainStep = 1;
			break;
		case 1:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2103);
			Uart0ReEnable;
			mainStep = 2;
			break;
		case 2:		// 转盘位置初始化
			SetMotRunPam(MOT_TURN_PLATE,240,20,CURRENT_TURN_PLATE);
			MotInitCheck(MOT_TURN_PLATE);
			waitMotTurnPlate = 1;
			mainStep = 3;
			break;
		case 3:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2104);
			Uart0ReEnable;
			mainStep = 4;
			break;
		case 4:		// 转盘运行到0位
			MotRunToSite(MOT_TURN_PLATE,0);
			waitMotTurnPlate = 1;
			waitMotSampNeedle = 1;
			mainStep = 5;
			break;
		case 5:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2107);
			Uart0ReEnable;
			mainStep = 6;
			break;
		case 6:		// 片仓小车运行到起始位
			SetMotRunPam(MOT_STORE_CARD_MOVE,200,10,CURRENT_STORE_MOVE);
			MotInitCheck(MOT_STORE_CARD_MOVE);
			waitMotSampNeedle = 1;
			mainStep = 7;
			break;
		case 7:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2108);
			Uart0ReEnable;
			mainStep = 8;
			break;
		case 8:		// 取样针回零经常出现在运转时给出空闲信号, 此处从新运行以避免错误
			MotRunTo(MOT_SAMP_NEEDLE, 0);
			waitMotSampNeedle = 1;
			mainStep = 9;
			break;
		case 9:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2109);
			Uart0ReEnable;
			mainStep = 10;
			break;
		case 10:		// 取样臂回到起始位
			SetEValve(EV_ALL, EV_CLOSE);
			SetMotRunPam(MOT_SAMP_TRUN,200,5,CURRENT_SAMP_TRUN);
			MotInitCheck(MOT_SAMP_TRUN);
			SetEValve(EV1, EV_CLOSE);
			waitMotSampTurn = 1;
			waitMotSampPump = 1;
			waitMotTurnPlate = 1;
			mainStep = 11;
			i = 0;
			break;
		case 11:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2113);
			uart_Printf("%s\r\n",strM2118);
			Uart0ReEnable;
			mainStep = 12;
			break;
		case 12:		// 扫描转盘上遗留干片	_RingPieceState
			i++;
			if(i >= RING_QUEUE_NUM)
				i = 0;
			m = PINK & 0x01;
			_RingPieceState[i] = 0;
			MotRunToSite(MOT_TURN_PLATE,i);
			mainStep = 13;
			break;
		case 13:
			n = PINK & 0x01;
			if(m != n)
				_RingPieceState[i] = 1;
			if(GetMotState(MOT_TURN_PLATE)!=STA_SLAVE_FREE)	
				break;
			if(i == 0)
				mainStep = 14;
			else
				mainStep = 12;
			break;
		case 14:		// 扫描转盘
			MotRunToSite(MOT_TURN_PLATE,10);	// 25 让转盘的0号转到卸片位置
			waitMotTurnPlate = 1;
			i = 0;
			mainStep = 15;
			break;
		case 15:		// 按顺序寻找剩余干片
			n = i + 10;
			if(n>=RING_QUEUE_NUM)
				n -= RING_QUEUE_NUM;
			if(GetRingPieceState(i) == 1)	// 转盘上有剩余干片
			{
				MotRunToSite(MOT_TURN_PLATE,n);	// 转盘运行到当前位置
				waitMotTurnPlate = 1;
				mainStep = 16;
				m = 0;		// 废片仓打开计时
			}
			else
				mainStep = 19;		// 继续查找下一个
			break;
		case 16:		// 开始卸载转盘上的干片
			if(GetwasteCardState() == 0)
			{
				if((PINK & 0x02) == 0)
				{
					SetDelayTime(MOT_TURN_PLATE, 10);
					m ++;
					if(m == 2)
					{
						Uart0ReUnable;
						uart_Printf("%s $   1\r\n",strM0200);
						Uart0ReEnable;
						SetBeepWarning();
					}
					if(m == 25)
					{
						SetBeepWarning();
						m = 5;
					}
					break;
				}
			}
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);
			waitMotCardUnLoad = 1;
			mainStep = 17;
			break;
		case 17:
			SetDelayTime(MOT_SAMP_TRUN, 10);
			mainStep = 18;
			break;
		case 18:	// 卸片小车回到起始点
			MotRunTo(MOT_CARD_UNLOAD,0);
			waitMotCardUnLoad = 1;
			mainStep = 19;
			break;
		case 19:
			i++;
			if(i < RING_QUEUE_NUM)	// 未完继续
				mainStep = 15;
			else
			{
				mainStep = 20;	// 检查结束
				Uart0ReUnable;
				uart_Printf("%s\r\n",strM2114);
				Uart0ReEnable;
			}
			break;
		case 20:	// 检查片仓小车上是否有干片
			SetMotRunPam(MOT_STORE_CARD_MOVE,64,10,CURRENT_STORE_MOVE);
			MotRunTo(MOT_STORE_CARD_MOVE,100);
			waitMotCardTrolley = 1;
			mainStep = 21;
			break;
		case 21:
			i = PINL & 0x04;					// 读取取片检测传感器空闲状态
			MotRunTo(MOT_STORE_CARD_MOVE,0);	// 取片小车运行到零位
			waitMotCardTrolley = 1;
			mainStep = 22;
			break;
		case 22:
			n = PINL & 0x04;					// 读取取片检测传感器状态
			if(i != n)		// 取片小车上有剩余干片
			{
				MotRunToSite(MOT_TURN_PLATE,0);	// 转盘运行到0号位置
				waitMotTurnPlate = 1;
				mainStep = 23;
				Uart0ReUnable;
				uart_Printf("%s\r\n",strM2115);
				Uart0ReEnable;
			}
			else
				mainStep = 31;		// 取片小车上没有干片, 跳过
			break;
		case 23:		// 干片推入转盘
			SetMotRunPam(MOT_CARD_LOAD,160,10,CURRENT_CARD_LOAD);
			MotRunTo(MOT_CARD_LOAD,_POS_CARDLOAD_RING);		// 装片行程94mm/0.08128 = 1156
			waitMotCardLoad = 1;
			mainStep = 24;
			break;
		case 24:
			SetDelayTime(MOT_SAMP_TRUN, 10);
			mainStep = 25;
			break;
		case 25:		// 干片推入复位
			SetMotRunPam(MOT_CARD_LOAD,200,10,CURRENT_CARD_LOAD);
			MotRunTo(MOT_CARD_LOAD,0);
			waitMotCardLoad = 1;
			mainStep = 26;
			break;
		case 26:		// 转盘0号转到卸片位置
		//	SetMotRunPam(MOT_TURN_PLATE,240,10,2);
			MotRunToSite(MOT_TURN_PLATE,25);
			waitMotTurnPlate = 1;
			mainStep = 27;
			break;
		case 27:		// 开始卸片
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,CURRENT_CARD_UNLOAD);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_OUT);
			waitMotCardUnLoad = 1;
			mainStep = 28;
			break;
		case 28:
			SetDelayTime(MOT_SAMP_TRUN,10);
			mainStep = 29;
			break;
		case 29:		// 卸片小车回到起始位
			SetMotRunPam(MOT_CARD_UNLOAD,200,20,2);
			MotRunTo(MOT_CARD_UNLOAD,_POS_UNLOAD_HOME);
			waitMotCardUnLoad = 1;
			mainStep = 30;
			break;
		case 30:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2116);
			Uart0ReEnable;
			mainStep = 31;
			break;
		case 31:		// 转盘运转到初始状态
			MotRunToSite(MOT_TURN_PLATE,29);
			waitMotTurnPlate = 1;
			MotRunTo(MOT_STORE_CARD_MOVE,432);
			waitMotCardTrolley = 1;
			mainStep = 32;
			break;
		case 32:
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2117);
			Uart0ReEnable;
			mainStep = 33;
			break;
		case 33:
			Uart0ReUnable;
			uart_Printf("%s\r\n", strM1101);
			Uart0ReEnable;
			mainStep = 0;
			return 1;
			break;
		}
	return 0;
}

// 等待吸样按键
unsigned char WaitStartKey(void){
	// 等待启动按键按下
	static unsigned char callCnt;
	unsigned char key;
	
	key = (PINJ & 0x40);
	if(callCnt == 0){
		if(key == 0)	// 按键在松开是启动，如果按键一直按着视为无动作
			callCnt = 1;
		}
	else {
		if(key == 0x40){	// 按键按下
			callCnt = 0;
			return 1;
			}
		}
	return 0;
}

/********************************************* 蜂鸣提示音 **********************************************/
static unsigned char BeepNum=0;
static unsigned char BeepState=0;
static unsigned int BeepCnt;

void SetBeepBusy(void){
	// 1短 1长
	BeepState=1;
	BeepNum = 0;
	BeepCnt=0;
}
void SetBeepWarning(void){
	// 1长音
	BeepState=2;
	BeepNum = 0;
	BeepCnt=0;
}
void SetBeepAck(void){
	// 1短音
	BeepState=3;
	BeepNum = 0;
	BeepCnt=0;
	
}
void SetBeepPrompt(void){
	// 2短音
	BeepState=4;
	BeepNum = 0;
	BeepCnt=0;
}
void SetBeepError(void){
	// 连续长音
	BeepState=5;
	BeepNum = 0;
	BeepCnt=0;
}
void SetBeep(unsigned char n){
	BeepState=0;
	BeepNum = n;
	BeepCnt=0;
}
void BeepStop(void){
	BeepState=0;
	BeepNum = 0;
	BeepCnt=0;
}

void Beep(void){
	BeepCnt ++;
	switch(BeepState){
		case 0:
			if(BeepNum){
				switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 200:	DDRB  &= 0x7f;				break;
					case 500:	BeepCnt = 0;	BeepNum --;	break;
					default:	break;
				}
			}
			break;
		case 1:// 提示忙 1短 1长
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 200:	DDRB  &= 0x7f;				break;
					case 500:	DDRB  |= 0x80;				break;
					case 2000:	DDRB  &= 0x7f;				break;
					case 2500:	BeepCnt = 0; BeepState = 0;	break;
					default:	break;
				}
			break;
		case 2:// 提示警告  1长音
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 2500:	DDRB  &= 0x7f;				break;
					case 3000:	BeepCnt = 0; BeepState = 0;	break;
					default:	break;
				}
			break;
		case 3:	// 操作应答  1短音
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 200:	DDRB  &= 0x7f;				break;
					case 500:	BeepCnt = 0; BeepState = 0;	break;
					default:	break;
				}
			break;
		case 4:	// 输出提示  2短音
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 200:	DDRB  &= 0x7f;				break;
					case 500:	DDRB  |= 0x80;				break;
					case 700:	DDRB  &= 0x7f;				break;
					case 1000:	BeepCnt = 0; BeepState = 0;	break;
					default:	break;
				}
			break;
		case 5:	// 错误 连续长音
			switch(BeepCnt){
					case 10:	DDRB  |= 0x80;				break;
					case 2500:	DDRB  &= 0x7f;				break;
					case 3000:	BeepCnt = 0; 				break;
					default:	break;
				}
			break;
		default:
			break;
		}
}

// 设置状态指示灯
void SetStateLedBusy(void)
{
	DDRE |= 0x30;
	PORTE |= 0x10;
	PORTE &= 0xdf;
	if(_SampSW == 1)		// 未锁定按键,LED变红
		_LEDSTATE = 0;
}
void SetStateLedFree(void)
{
	DDRE |= 0x30;
	PORTE |= 0x20;
	PORTE &= 0xef;
	if(_SampSW == 1)		// 未锁定按键,LED变绿
		_LEDSTATE = 1;
}
/****************************************************************************************************/


/********************************************* File end **********************************************/
