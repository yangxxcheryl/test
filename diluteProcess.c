

#include <iom1280v.h>
#include "B1404_LIB.h"
#include "Common.h"
#include "eeprom.h"

/* 本模块包含以下功能:
1.液路自检
2.泵定标
3.稀释标本
4.从干片仓提取新干片
5.将滴完样本的干片放入转盘
6.调用其它模块将测试任务加入任务队列
*/


/*		稀释比例
比例	标本(uL)		稀释液(uL)		混匀液(uL)
				[一次混匀]
1		100
2		90				90	(4)			180
5		90				360	(12)		450
10		60				540	(18)		600

20		30				570	(19)		600
50		9.8				480	(16)		489.8		(做为二次混匀的一次稀释)
100		10				990	(33)		1000		
200		4.97			990	(33)		994.7
				[二次混匀]
10		60				540	(18)		600			X 50 = 500
20		30				570	(19)		600			X 50 = 1000
40		20				780	(26)		800			X 50 = 2000
50		18.98			930	(31)		948.99		X 100 = 5000
100		10				990	(33)		1000		X 100 = 10000
二次稀释以100倍的一次稀释液为标本再次稀释


稀释泵流量单位是：30uL，即每转过一个泵小轮的管道流量1
柱塞泵步进行程：16 * 2.032mm = 32.512mm / 400 = 0.08128mm
柱塞泵每步注射量：0.08128mm * （1*1*3.1416）mm2 = 0.255349248mm3 = 0.25535mm3
*/


void _FluidMotRun(signed int n,unsigned char vel);
void _EffluentMotRun(signed int n,unsigned char vel);
void _DiluentMotRun(signed int n,unsigned char vel);
void _NewDiluentMotRun(signed int n,unsigned char vel);
void _NeedleMotRun(signed int n,unsigned char vel);
void _NeedleMotRunTo(signed int n,unsigned char vel);
void _SampPumpMotRun(signed int n,unsigned char vel);
void _SampPumpMotRunTo(signed int n,unsigned char vel);

//********************************************
extern unsigned char CardNoneUseful;
unsigned char InsertRingFlag = 0;
extern unsigned char insertflag[30];
unsigned char JudgeFlag;					// 如果设置吸样量,可通过此变量调节稀释倍数
unsigned char WithoutPuncture = 0;			// 默认为0 穿刺   1  不穿刺
extern unsigned char preCardStoreNum;
extern unsigned char CardStoreTestFlag;	
extern unsigned char LastCardGetState;
extern unsigned char primeProcessSW;		
extern unsigned char WorkProcessStep;		// 工作进程号
//********************************************

SAMP_INFO NewTestInfo;		// 新测试信息

// 工作参数
unsigned char CurInsertRingNum;			// 新测试卡要插入转盘的位置，0xff表示无效
unsigned char GetNewTestCard;			// 取新的测试卡，0:无, 1:取插入卡, 2:取仓1卡, 3:取仓2卡, 4:取仓3卡, 5:取仓4卡, 6:取仓5卡，
										// 254:取卡完成等待滴样, 255:卡片滴样完成等待放入转盘, 250:提取卡片出错
										// 222:取片准备抽打混匀, 233:抽打混匀结束,准备进入254取片完成准备滴样
unsigned long SecondCount = 0;			// 秒时钟计数，在定时中断计数
extern unsigned char TurnPlateUsedLock;	// 转盘使用锁


static unsigned char _DiluentQuitFlag = 0;
static unsigned int liqDetBaseAdc;
static unsigned char runNum;
static unsigned char waitMotSampTurn,waitMotSampNeedle, waitMotSampPump,waitMotFluid,waitMotDiluent,waitEffluent;
signed int _DropVolume;					// 滴样量
static signed int _DropVolumeFactor;	// 滴样量调节因子
signed int _SamplingVolume;				// 吸样量
unsigned int _DropMode;					// 滴样模式 0 胶体金试剂片	1 荧光试剂片
unsigned char _MixtureMode;				// 1:1是否需要混匀 0 需要  1  不需要
unsigned int _ReMixNum = 5;				// 抽打次数
unsigned int _SleepTime;				// 休眠时间
unsigned char _SleepSwitch;				// 休眠开关
unsigned int _AutoTestCycleNum = 0;		// 自动测试次数


static signed int NeedleOnMixSidePos;	// 取样针到混匀池右边沿位置
static signed int NeedleOnMixCenterPos;	// 取样针到混匀池中间位置
static signed int DropHeight;			// 滴样高度
static signed int MixHeight;			// 抽打混匀高度
static unsigned char CleanMode = 0;		// 清洗模式  0:无清洗，1:普通清洗， 2:高浓度清洗
static unsigned char TestDebugMode = 0;	// 混匀测试模式 0:正常测试  1：混匀测试
unsigned char _SampSW = 1;		// 按键判断
static unsigned char _WaitStartKey;		// 等待按键

// 新设置参数
unsigned char _NewCardStoreNum;			// 片仓号
unsigned char _NewMultipNum;			// 稀释比例编号
static unsigned int _NewReadTime0;		// 第一读数时间
static unsigned int _NewReadTime1;		// 第二读数时间
static unsigned char _NewTestType;		// 测试类型


unsigned char JumpMode = 0;		// 直接测试模式  0 需要机械自检和液路自检   1 直接进入测试  2 休眠接入液路自检 3 休眠模式完成3322液路自检
unsigned char stopTestFlag = 0; // 停止测试,废片仓连续开启时间超过20分钟

static unsigned char DiluteProcess_workStep;	
static unsigned char DiluteProcess_mainStep;	

/**********************************  稀释处理程序  *****************************************/

unsigned int CalMixingHeight(unsigned char multipNum, unsigned char diluteTime);
void _SetCurWorkParamter(void);
void _SetNewCardGet(unsigned char num);
unsigned char _CheckFluidSupply(void);
unsigned char _CheckDiluentSupply(void);


void SetDropVolume(unsigned int vol);
signed int CalSampVolume(unsigned char multipNum, unsigned char diluteTime);
signed int CalDiluteVolume(unsigned char multipNum, unsigned char diluteTime);
signed int CalDilute2Volume(unsigned char multipNum, unsigned char dilutetype);
unsigned char CalSampSyringSpeed(unsigned char multipNum, unsigned char diluteTime);
unsigned char CalDiluentInjectSpeed(unsigned char multipNum, unsigned char diluteTime);	



void SetTestDebugMode(unsigned char m)
{
	// 设置测试状态下的调试模式	0:正常，1:混匀液测量
	if(m>1)
		m = 0;
	TestDebugMode = m;
}

unsigned char SamplingSwitch(unsigned char  sw)
{
	_SampSW = sw;
}

extern unsigned char _DiluteMainStep, _DiluteWorkStep;
extern unsigned char CardSurplusState[];		// 卡片剩余状态
extern unsigned char CardStoretate[];
extern RING_QUEUE	RingQueue;





void printf_DiluteProcess_StepState(void)
{
	Uart0ReUnable;
	uart_Printf("%s $%2d $%2d\r\n",strM3201,DiluteProcess_mainStep,DiluteProcess_workStep);
	Uart0ReEnable;
}

void SetPunctureState(unsigned char m)
{
	if(m > 2)
		m = 2;
	WithoutPuncture = m;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3119,WithoutPuncture);
	Uart0ReEnable;
}

unsigned char DiluteProcess(INFO_EVENT * pInfoEvent)
{
	// 稀释运行处理程序
	static unsigned char mainStep;		
	static unsigned char workStep;
	static unsigned char startKey;
	static unsigned char  checkDiluent, checkFluid;
	static unsigned char pos;
	static unsigned char ucTmp;
	static unsigned char Num;
	signed int siTmp,siTmp1;
	static signed char sc;
	static unsigned int i;
	static unsigned int theDifferenceOfStep,theDownPointAdc;
	static long lastSampTime = 0;// 最后一次取样时间,用于待机判断
	static unsigned char FindNum = 0;// 查找转盘次数
	static unsigned char temp1,temp2;	


#ifdef Puncture
	static unsigned char detRetry ;			// 穿刺使用
	static unsigned char ErrorNum = 0;		// 传递穿刺模式下的错误,根据不同的错误发送不同的信息
#endif
	
	DiluteProcess_mainStep = mainStep;
	DiluteProcess_workStep = workStep;
	
	if(1 == checkDiluent)					// 检查稀释液
	{	
		if(_CheckDiluentSupply() == 1)
		{	
			mainStep = 13;	
			workStep = 0;	
			checkDiluent = 0;
		}	
	}
	if(1 == checkFluid)						// 检查清洗液
	{		
		if(_CheckFluidSupply()==1)
		{	
			mainStep = 12;	
			workStep = 0;	
			checkFluid = 0;
		}
	}
	
	if(JumpMode == 3)		// 休眠模式下手动3322液路自检完成之后,跳转到测试
	{
		JumpMode = 0;
		mainStep = 1;
		workStep = 1;
		if(_SampSW == 0)	// 如果此时按键被屏蔽,解锁屏蔽
			_SampSW = 1;
	}
	
	if(WaitDelayTime(MOT_SAMP_PUMP))		return 0;
	if(WaitDelayTime(MOT_SAMP_TRUN))		return 0;
	if(WaitDelayTime(MOT_SAMP_NEEDLE))		return 0;
	if(WaitDelayTime(MOT_EFFLUENT))			return 0;
	if(WaitDelayTime(MOT_FLUID))			return 0;
	if(WaitDelayTime(MOT_DILUENT))			return 0;
	
	if(waitMotSampTurn){	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
	if(waitMotSampNeedle){	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
	if(waitMotSampPump){	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotSampPump = 0;	}
	if(waitMotFluid){		if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)			return 0;	waitMotFluid = 0;	}
	if(waitMotDiluent){		if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE)		return 0;	waitMotDiluent = 0;	}
	if(waitEffluent){		if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;	}

	if(_WaitStartKey)
	{
		if(_DiluentQuitFlag != 0)	// 稀释程序退出处理
		{
			_DiluentQuitFlag = 0;
			_WaitStartKey = 0;
			mainStep = 10;		// 进入退出程序
			workStep = 0;
			return 1;
		}
		if(WaitStartKey() == 0)
		{		// 按键事件处理
			if(mainStep == 1 && workStep == 1)
			{		// 测试空闲状态下，处理休眠事件
				if(_SleepTime == 1)
				{		// 休眠时间有效时
					// 待机任务
					TestALampClose();	// 关闭光源
					_WaitStartKey = 0;
					_SleepTime = 0;
					mainStep = 20;		// 进入待机程序
					workStep = 0;
					return 0;
				}
			}
			return 1;
		}

		if(_SampSW)
		{
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM3200);	// 2016-10-10 发送按键信息
			Uart0ReEnable;
			_WaitStartKey = 0;
		}
		else
			return 1;
	}
	
	switch(mainStep)
	{
		case 0:
		// 初始化准备
			switch(workStep)
			{
				case 0:		// 等待用户按下吸样键后取样针运行到吸样位置
					if(JumpMode == 1)	
					{
						_NeedleMotRunTo(0, 180);
						waitMotSampNeedle = 1;
						workStep = 24;
						JumpMode = 0;
						break;
					}
					if(MachinePositionInit())
					{
						_DiluentQuitFlag = 0;
						Uart0ReUnable;
//						uart_Printf("%s\r\n",strM3191);		// 取样休眠状态
						uart_Printf("%s\r\n",strM3101);		// 请按吸样键开始
						Uart0ReEnable;
						workStep = 1;
						TestDebugMode = 0;
					}
					break;
				case 1:	// 等待用户按吸样键
					_WaitStartKey = 1;	
					workStep = 2;
					if(CardNoneUseful == 0)
						TestALampOpen();		
					break;
				case 2:
					if(JumpMode == 2)			// 正在休眠,此时按下吸样键,进行液路自检
					{
						primeProcessSW = 3;
						mainStep = 1;
						workStep = 0;
						JumpMode = 0;
						break;
					}
					SetEValve(EV_ALL, EV_CLOSE);
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3100);		// 启动
					Uart0ReEnable;
					lastSampTime = SecondCount;		// 计算待机时间用
					// 读取取样针位置信息
					NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
					NeedleOnMixSidePos = GetNeedleOnMixSidePos();
					DropHeight = GetDropHeight();
					SetMotRunPam(MOT_SAMP_TRUN,240,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 3;
					break;
				case 3:		// 灌注清洗液
					ucTmp = _PrimingFluid();
					if(ucTmp == 1)	workStep = 4;		// 进入稀释液灌注
					else if(ucTmp == 0xff)	
					{	// 重新灌注
						workStep = 1;	
						Uart0ReUnable;
						uart_Printf("%s\r\n",strE3902);	
						Uart0ReEnable;
					}
					break;
				case 4:		// 灌注稀释液
					ucTmp = _PrimingDiluent();
					if(ucTmp == 1)	workStep = 5;		// 全部灌注完毕
					else if(ucTmp == 0xff)	
					{	// 重新灌注
						Uart0ReUnable;
						uart_Printf("%s\r\n", strE3904);
						Uart0ReEnable;
						workStep = 1;	
					}
					break;
				case 5:	// 清洗清洗头和取样针通道
					// 建立负压
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(100, 200);
					SetDelayTime(MOT_EFFLUENT, 5);
					workStep = 6;
				case 6:	// 清洗
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(30, 30);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetDelayTime(MOT_EFFLUENT, 10);
					waitMotSampNeedle = 1;
					workStep = 100;
					break;
				case 100:
					MotStop(MOT_FLUID);
					waitMotFluid = 1;
					SetDelayTime(MOT_EFFLUENT, 10);
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampNeedle = 1;
					workStep = 7;
					break;
				case 7:	// 先关闭清洗液
					_FluidMotRun(-2, 80);
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotFluid = 1;
					waitMotSampNeedle = 1;
					workStep = 8;
					break;
				case 8:	// 关闭负压
					SetEValve(EV_ALL, EV_CLOSE);
					MotStop(MOT_EFFLUENT);
					_NeedleMotRunTo(_POS_MIX_BUTTOM, 180);//240
					waitMotSampNeedle = 1;
					sc = 3;		// 设置清洗次数
					workStep = 9;
					break;
				case 9:	// 清洗和灌注取样针通道
					sc --;
					SetEValve(EV2, EV_OPEN);
					MotInitCheck(MOT_SAMP_PUMP);
					_FluidMotRun(12, 64);		// 注入1mL清洗液
					waitMotFluid = 1;
					SetDelayTime(MOT_FLUID, 40);
					workStep = 10;
					break;
				case 10:
					_EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					workStep = 11;
					break;
				case 11:
					if(sc != 0){
						workStep = 9;
						}
					else{
						workStep = 15;
						}
					break;
				case 15:	// 清洗完毕，抽清洗头残留水
					SetEValve(EV_ALL, EV_CLOSE);
					SetEValve(EV3, EV_OPEN);
					_NeedleMotRunTo(0, 180);//240
					_EffluentMotRun(30, 200);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					workStep = 16;
					break;
				case 16:	// 排干混匀池水
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(20, 220);
					waitEffluent = 1;
					workStep = 17;
					break;
				case 17:			// 取样臂不在吸样位置，需先运行到吸样位
					SetMotRunPam(MOT_SAMP_NEEDLE, 200, 5, CURRENT_SAMP_NEEDLE);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetMotRunPam(MOT_SAMP_PUMP,120,5,CURRENT_SAMP_PUMP);
					MotInitCheck(MOT_SAMP_PUMP);
					waitMotSampNeedle = 1;
					workStep = 18;
					SetBeepAck();
					break;
				case 18:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotInitCheck(MOT_SAMP_TRUN);
					waitMotSampTurn = 1;
					workStep = 19;
					break;
				case 19:
					MotRunTo(MOT_SAMP_TRUN,_POS_SAMPTURN_SAMP);
					waitMotSampTurn = 1;
					workStep = 20;
					break;
				case 20:
#ifndef Puncture				
					_NeedleMotRunTo(_POS_SAMP_DOWN, 240);	//穿刺屏蔽
					waitMotSampNeedle = 1;
#endif					
					_EffluentMotRun(100, 200);
					SetEValve(EV3, EV_OPEN);
					SetDelayTime(MOT_SAMP_TRUN, 30);
					workStep = 21;
					break;
				case 21:		// 取样针在吸样位置，跳到吸样程序
					SetBeepPrompt();
					MotStop(MOT_EFFLUENT);
					SetEValve(EV_ALL,EV_CLOSE);
					workStep = 22;	
					break;
				case 22:
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_INTERVAL+_SAMP_PUMP_AIR_ISOLATE);
					waitMotSampPump = 1;
					workStep = 23;
					break;
				case 23:
					SetStateLedFree();
					mainStep = 1;
					workStep = 0;
					if(CleanMode != 0)
					{
						if(1 == CleanMode)
						{
							Uart0ReUnable;
							uart_Printf("%s\r\n", strM3167);
							Uart0ReEnable;
						}
						else if(2 == CleanMode)
						{
							Uart0ReUnable;
							uart_Printf("%s\r\n", strM3168);
							Uart0ReEnable;
						}
						else if(250 == CleanMode)
						{
							Uart0ReUnable;
							uart_Printf("%s\r\n", strM3169);
							Uart0ReEnable;
						}
						else if(255 == CleanMode)
						{
							Uart0ReUnable;
							uart_Printf("%s\r\n", strM3170);
							Uart0ReEnable;
						}
						CleanMode = 0;
						checkFluid = 0;
					}
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3102);
					Uart0ReEnable;
					SetStateLedFree();
					break;
				case 24:
					SetDelayTime(MOT_SAMP_NEEDLE,10);
					workStep = 25;
					break;
				case 25:
					if(GetMotorMonitorState(MOT_SAMP_NEEDLE,ZeroMonitor) == 1)		// 判断吸样针是否回到零位
					{
						MotRunTo(MOT_SAMP_TRUN, 0);
						waitMotSampTurn = 1;
						workStep = 26;
					}
					else
					{
						workStep = 0;
						JumpMode = 1;
					}
					break;
				case 26:
					SetDelayTime(MOT_SAMP_TRUN,10);
					workStep = 27;
					break;
				case 27:
					if(GetMotorMonitorState(MOT_SAMP_TRUN,ZeroMonitor) == 1)		// 判断旋转臂是否回到零位
					{
						MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
						waitMotSampTurn = 1;
						// 读取取样针位置信息 到混匀池中心  边缘  滴样高度
						NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
						NeedleOnMixSidePos = GetNeedleOnMixSidePos();
						DropHeight = GetDropHeight();
#ifndef Puncture
						workStep = 28;
#else
						workStep = 21;
#endif
					}
					else
					{
						workStep = 25;
					}
					break;
				case 28:
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);
					waitMotSampNeedle = 1;
					workStep = 21;
					break;
			}
			break;
		case 1:	
// 吸取样本，清洗吸样针
			switch(workStep)
			{
				case 0:	// 等待用户按吸样键
					if(_AutoTestCycleNum == 0)
						_WaitStartKey = 1;
					else
					{
						_AutoTestCycleNum --;
						Uart0ReUnable;
						uart_Printf("// AutoTestCyc: %d\r\n", _AutoTestCycleNum);
						Uart0ReEnable;
					}
					lastSampTime = SecondCount;		// 更新最后的取样时间
					GetStoreState(0);				// 查看片仓信息
					waitMotSampPump = 1;
					workStep = 1;	
					break;
				case 1:		
					// 如果停止测试
					if(1 == stopTestFlag)
					{
						// 上报错误信息,提示必须关闭废片仓
						Uart0ReUnable;
						uart_Printf("%s\r\n", strE3906);
						Uart0ReEnable;
						SetBeepWarning();
						workStep = 0;
						return 1;
					}
					
					if(CleanMode == 1)
					{	// 普通清洗模式
						Uart0ReUnable;
						uart_Printf("%s\r\n", strM3157);
						Uart0ReEnable;
						mainStep = 105;
						workStep = 0;
						SetStateLedBusy();
						break;
					}
					else if(2 == CleanMode)
					{	// 强力清洗模式
						Uart0ReUnable;
						uart_Printf("%s\r\n", strM3158);
						Uart0ReEnable;
						mainStep = 100;
						workStep = 0;
						SetStateLedBusy();
						break;
					}
					// 如果是取片测试
					if(0 != CardStoreTestFlag)
					{	
						if(0 == LastCardGetState)					// 取片测试模式下,前一个没有完成,无法执行下一次取片动作
							return;
						//LastCardGetState = 0;
						if(_NewCardStoreNum != preCardStoreNum)
						{
							SetWorkStoreNum(preCardStoreNum);
						}
					}
					// 先检查片仓是否被开启
					if(CardStoretate[_NewCardStoreNum - 1] == INFO_STORE_OPEN)
					{
						Uart0ReUnable;
						uart_Printf("!3521 $%4d\r\n", _NewCardStoreNum);
						Uart0ReEnable;
						_WaitStartKey = 1;
						SetDelayTime(MOT_SAMP_NEEDLE, 10);
						SetBeepWarning();
						break;
					}
					// 检查片仓是否有干片
					if(CardNoneUseful == 0)
					{
						
						if(CardSurplusState[_NewCardStoreNum - 1] == INFO_STORE_EMPTY)
						{
							Uart0ReUnable;
							uart_Printf("!3520 $%4d\r\n", _NewCardStoreNum);
							Uart0ReEnable;
							_WaitStartKey = 1;
							SetDelayTime(MOT_SAMP_NEEDLE, 10);
							SetBeepWarning();
							break;
						}
						if(CardSurplusState[_NewCardStoreNum - 1] == INFO_STORE_ERROR)
						{
							Uart0ReUnable;
							uart_Printf("!3522 $%4d\r\n", _NewCardStoreNum);
							Uart0ReEnable;
							_WaitStartKey = 1;
							SetDelayTime(MOT_SAMP_NEEDLE, 10);
							SetBeepWarning();
							break;
						}
					}
#ifndef Puncture
					// 开始测试，保存本次测试的测试卡类型和稀释比例	
					_SetCurWorkParamter();
					_SetNewCardGet(NewTestInfo.cardStoreNum);
#endif
					SetBeepAck();
					SetEValve(EV_ALL,EV_CLOSE);
					SetDelayTime(MOT_SAMP_NEEDLE, 2);  // 2017-05-22 5 -> 2 
					workStep = 2;
					break;
				case 2:
#ifdef Puncture		
					if(0 == WithoutPuncture)
					{		
						MotRunTo(MOT_SAMP_TRUN,_POS_SAMPTURN_SAMP);
						waitMotSampTurn = 1;
						workStep = 30;
					}
					else
					{
						if(1 == WithoutPuncture)
						{
							_WaitStartKey = 1;
							_NeedleMotRunTo(2380, 240);	//_POS_SAMP_DOWN - 1420
							waitMotSampNeedle = 1;
							workStep = 41;
						}
						else if(2 == WithoutPuncture)
						{
							_NeedleMotRunTo(_POS_SAMP_DOWN, 240);	
							waitMotSampNeedle = 1;
							workStep = 40;
						}
					}
					break;
				case 30:				// 开始穿刺
#ifndef HalfCircle
					_NeedleMotRunTo(300 * 2, 180);	
#else
					_NeedleMotRunTo(300, 180);			
#endif
					waitMotSampNeedle = 1;
					//workStep = 31;
					detRetry = 0;
					workStep = 35;
					break;
				case 35:
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);      //采样电压的base值
					//if(liqDetBaseAdc < 400)
					if(liqDetBaseAdc < 430)
					{
						workStep = 60;
						//SetEValve(EV_ALL,EV_CLOSE);
						//SetEValve(EV1,EV_OPEN);    //清洗液供给3号管
						SetEValve(EV3,EV_OPEN);    //废液连接清洗池
						_EffluentMotRun(12, 240);
						//废液排空清洗头的水
						waitEffluent = 1;
						SetDelayTime(MOT_SAMP_NEEDLE, 5); 
						if(detRetry >= 3)
						{
							detRetry = 0;
							workStep = 50;
							ErrorNum = 4;	// 3928
						}
						else
							detRetry++;
						break;
					}
					detRetry = 0;
					SetEValve(EV_ALL,EV_CLOSE);
					Uart0ReUnable;
					uart_Printf("// LiqDetBaseAdc:%d\r\n", liqDetBaseAdc);
					Uart0ReEnable;
					//workStep = 31;
					workStep = 36;
					break;
				case 36:
#ifndef HalfCircle
					_NeedleMotRunTo(560 * 2, 180);	
#else
					_NeedleMotRunTo(560, 180);
#endif
					waitMotSampNeedle = 1;
					workStep = 31;
					break;
				case 60:
					//_FluidMotRun(20, 40);           //清洗泵改成倒吸
					SetEValve(EV1,EV_OPEN);
					_FluidMotRun(-2, 40);
					//waitMotSampNeedle = 1;
					//waitEffluent = 1;
					waitMotFluid = 1;
					workStep = 35;
					break;
					
					
				case 31:				// 穿刺到达穿刺光藕位置
					SetMotRunPam(MOT_SAMP_NEEDLE, 120, 10, 10); 	// 60
#ifndef HalfCircle
					MotRun(MOT_SAMP_NEEDLE, 320 * 2);
#else
					MotRun(MOT_SAMP_NEEDLE, 320);
#endif
					waitMotSampNeedle = 1;
					workStep = 32;
					break;
				case 32:				// 判断穿刺结果 (PINA & 0x40):穿刺感应传感器
					//ucTmp = PINA;
					//if((ucTmp & 0x40) == 0)	// 穿刺压力(试管)感应
					{
						Uart0ReUnable;
						uart_Printf("%s\r\n",strM3225);
						Uart0ReEnable;
						workStep = 33;
						break;
					}
					/*
					else	// 未探测到试管,取消测试
					{
						// 撤销吸样
						ErrorNum = 1;	// 3925
						workStep = 50;
						break;
					}
					*/
					break;
				case 33:	//  (PINA & 0x80):穿刺到位传感器
					//ucTmp = PINA;
					//if((ucTmp & 0x80) != 0)	// 穿刺到位感应
					{
						Uart0ReUnable;
						uart_Printf("%s\r\n",strM3226);
						Uart0ReEnable;
						workStep = 34;
					}
					/*
					else	// 未穿刺到位
					{
						ErrorNum = 1;	// 3926
						// 撤销吸样
						workStep = 50;
						break;
					}
					*/
					break;
				case 34:				// 液面探测初始化并启动取样针下降
					SetMotRunPam(MOT_SAMP_NEEDLE, 180, 5, 10);	// 110
					MotRunTo(MOT_SAMP_NEEDLE, _POS_SAMP_DOWN);
					detRetry = 0;
					workStep = 37;
					break;
				case 37:		// 液面探测
					i = getLiqDetADC(NeedleChannel);// 0-200
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						// 电机运行到最大行程处,未探测到液面
						MotStop(MOT_SAMP_NEEDLE);
						SetBeepWarning();
						// 退出处理
						ErrorNum = 3;	// 3927
						Uart0ReUnable;
						uart_Printf("// LiqNotPassAdc:%d\r\n", i);
						Uart0ReEnable;
						workStep = 50;
						break;
					}
					if(i < liqDetBaseAdc)// 350
					{
						i = liqDetBaseAdc - i;//100+
						Num = liqDetBaseAdc / 5;
						//Num = liqDetBaseAdc / 8;//>25
						//if(Num > 25)
						//	Num = 25;
						if(i > Num)		// 设置液面感应灵敏度  16  25
						{
							if(detRetry < 3)  // 1
							{
								detRetry ++;	
								break;	
							}
							
							MotStop(MOT_SAMP_NEEDLE);   //先将小针停止	
							Uart0ReUnable;
					        uart_Printf("// theNeedleMotIsStop\r\n");	
					        Uart0ReEnable;
							SetDelayTime(MOT_SAMP_NEEDLE, 4);  // 探测到液面停止延时
							SetBeepAck();
							workStep = 38;
						}
						else
							detRetry = 0;
					}
					break;
				case 38:	
				//	MotStop(MOT_SAMP_NEEDLE);
					theDifferenceOfStep = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);
					Uart0ReUnable;
					//uart_Printf("%s $%4d\r\n", strM3227,i);	
					uart_Printf("// theWaterLevelStep:%d\r\n", theDifferenceOfStep);	
					Uart0ReEnable;
					waitMotSampNeedle = 1;
					SetEValve(EV2,EV_CLOSE);  // 吸样前关闭EV2
				//	SetDelayTime(MOT_SAMP_NEEDLE, 5);
					workStep = 39;
					break;
				case 39:		// 探测到液面并停止电机运行
				
					theDownPointAdc=getLiqDetADC(NeedleChannel);
					
				    Uart0ReUnable;
					uart_Printf("// theDownPointAdc:%d\r\n", theDownPointAdc);	
					Uart0ReEnable;
					
					Uart0ReUnable;
					uart_Printf("// LiqChangeAdc:%d\r\n", i);	// 差值
					Uart0ReEnable;
					//i = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);         //pan20170503
					theDifferenceOfStep=_POS_SAMP_DOWN - theDifferenceOfStep;
					Uart0ReUnable;
					uart_Printf("// theDifferenceOfStep :%d\r\n", theDifferenceOfStep);	// 报告液面探测结果
					Uart0ReEnable;
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					
					if (theDifferenceOfStep >= 60)                     //留1.5mm的余量
					{
						workStep = 41;
					}
					else
					{
						ErrorNum = 3;	// 3927
						Uart0ReUnable;
						uart_Printf("// theDifferenceOfStepisTooSmall:%d\r\n", theDifferenceOfStep);
						Uart0ReEnable;
						workStep = 50;
					}
					break;
				case 41:
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);	
					waitMotSampNeedle = 1;
					workStep = 40;
					break;
				case 40:
					_SetCurWorkParamter();
					_SetNewCardGet(NewTestInfo.cardStoreNum);
#endif				
					siTmp = CalSampVolume(NewTestInfo.sampDiluteMult, 0);
					siTmp += 45;
					Uart0ReUnable;
					uart_Printf("%s $%4d $%4d\r\n", strM3121, siTmp,NewTestInfo.sampDiluteMult);	// 显示当前吸样量
					Uart0ReEnable;
#ifdef Puncture
					if(NewTestInfo.sampDiluteMult == 9)
					{
						if(WithoutPuncture == 0)	// 如果需要经过穿刺
							siTmp += 1955;
						else
							siTmp += 400 ;    
					}
#else
					if(NewTestInfo.sampDiluteMult == 9)
						siTmp += 400 ;    
#endif
					_SampPumpMotRun(siTmp, 180);	// 2017-05-22  60 -> 180	
					
					if(NewTestInfo.sampDiluteMult !=13)		//		如果是1：4	就不吐清洗液 
					 _FluidMotRun(4, 64);
					 
					 
#if (DILUTE_TUBE == 14)
					_DiluentMotRun(20, 180);		// 清洗稀释液管路和混匀池底部   
#elif (DILUTE_TUBE == 16)
					_DiluentMotRun(6, 200);			// 清洗稀释液管路和混匀池底部	// 2017-05-22 160 -> 200
#endif
					//SetStateLedBusy();     //pan20170614
					waitMotSampPump = 1;
					waitEffluent = 1;	// 如果上次排液未完毕，等待
					workStep = 3;
					break;
				case 3:
					waitMotFluid = 1;
					waitMotDiluent = 1;
					workStep = 4;
					break;
				case 4:		
				
				    SetBeepPrompt();  //吸样完成 蜂鸣器开始响声
					SetStateLedBusy();  //pan20170614  吸样完成之后，灯才变成红色
				    SetEValve(EV_ALL,EV_CLOSE);   //20170614 pan
				    // _DiluentMotRun(100, 60);		
			        _EffluentMotRun(20, 160);
			        waitEffluent = 1;	
					workStep = 101;
		        	break;
				case 101:	
				    _DiluentMotRun(25, 160);		
			        _EffluentMotRun(30, 200);
			        waitEffluent = 1;	
					waitMotDiluent = 1;
					workStep = 102;
		        	break;
				
				
				
				case 102:                      //20170614 pan
					SetEValve(EV3, EV_OPEN);
					SetEValve(EV1, EV_OPEN);
					//SetBeepPrompt();
#ifndef HalfCircle
					_EffluentMotRun(112, 240);		// 2017-05-22 140 -> 112  
#else
					_EffluentMotRun(56, 240);		// 2017-05-22 70 -> 56
#endif
					SetDelayTime(MOT_EFFLUENT, 2);	// 延迟一段时间，先建立负压
					workStep = 5;
					break;
				case 5:				// 开启洗液泵，清洗针外壁
					_FluidMotRun(20, 40);
					waitMotSampNeedle = 1;
					_NeedleMotRunTo(0, 200);	// 取样针上升
					workStep = 7;	
					break;
				case 7:				// 取样针已经上升到最高点，外壁清洗结束,取样针回到起始位
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("%s $%8d\r\n",strM3104 ,NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3104);
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					MotStop(MOT_FLUID);
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					checkFluid = 0;
					SetDelayTime(MOT_SAMP_TRUN, 5);  // 2017-05-22  10->5
					workStep = 8;
					break;
				case 8:		// 延迟1秒后停止清洗头吸空, 转换到混匀池排空
					SetEValve(EV_ALL,EV_CLOSE);
					waitMotSampTurn = 1;
					workStep = 9;
					break;
				case 9:				// 标本吸取完毕
					if(NewTestInfo.sampDiluteMult != 1)
					{
						MotRunTo(MOT_SAMP_TRUN, NeedleOnMixSidePos);		// 取样臂运行到混匀池壁上方
						waitMotSampTurn = 1;
						mainStep = 4;		
						workStep = 0;	// 进入一次稀释程序
					}
					else
					{
						if(_DropMode == 0)
						{
							mainStep = 9;		
							workStep = 0;		// 直接跳到滴样程序
						}
						else 
						{	// 1:1需要抽打混匀,直接跳到抽打混匀
							if(0 == _MixtureMode)
							{
								mainStep = 8;
								workStep = 11;
							}
							// 1:1不需要抽打混匀,直接滴样至有小杯子的试剂片
							else
							{
								mainStep = 8;
								workStep = 18;
							}
						}
					}
					waitEffluent = 1;
					break;
#ifdef Puncture			
				case 50:	// 穿刺失败退出处理
					SetMotRunPam(MOT_SAMP_NEEDLE, 100, 5, 10);		// 100
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					workStep = 51;
					break;
				case 51:
					SetEValve(EV3, EV_OPEN);
					SetEValve(EV1, EV_OPEN);
#ifndef HalfCircle
					_EffluentMotRun(140, 240);  
#else
					_EffluentMotRun(70, 240);
#endif
					SetDelayTime(MOT_EFFLUENT, 2);	// 延迟一段时间，先建立负压
					workStep = 52;
					break;
				case 52:
					_FluidMotRun(20, 40);
					waitMotSampNeedle = 1;
					workStep = 53;
					break;
				case 53:
					MotStop(MOT_FLUID);
					_FluidMotRun(-1, 40);
					SetDelayTime(MOT_EFFLUENT, 10);
					waitMotFluid = 1;
					if(ErrorNum == 1)
					{
						ErrorNum = 0;
						Uart0ReUnable;
						uart_Printf("%s\r\n",strE3925);
					//	uart_Printf("%s $%8d\r\n",strE3925,NewTestInfo.testSerial);
						Uart0ReEnable;
					}
					else if(ErrorNum == 2)
					{
						ErrorNum = 0;
						Uart0ReUnable;
						uart_Printf("%s\r\n",strE3926);
					//	uart_Printf("%s $%8d\r\n",strE3926,NewTestInfo.testSerial);
						Uart0ReEnable;
					}
					else if(ErrorNum == 3)
					{
						ErrorNum = 0;
						Uart0ReUnable;
						uart_Printf("%s\r\n",strE3927);
					//	uart_Printf("%s $%8d\r\n",strE3927,NewTestInfo.testSerial);
						Uart0ReEnable;
					}
					else if(ErrorNum == 4)
					{
						ErrorNum = 0;
						Uart0ReUnable;
						uart_Printf("%s  $ %4d\r\n",strE3928,liqDetBaseAdc);
					//	uart_Printf("%s $%8d\r\n",strE3928,NewTestInfo.testSerial);
						Uart0ReEnable;
					}
					workStep = 54;
					break;
				case 54:
					SetEValve(EV_ALL,EV_CLOSE);
					MotStop(MOT_EFFLUENT);
					workStep = 0;
					break;
#endif
				}
			break;
		case 4:				// 一次混匀
			switch(workStep)
			{
				case 0:		// 取样针下降到混匀高度
					_FluidMotRun(-1, 80);
					_EffluentMotRun(8, 240);
					_NeedleMotRunTo(_POS_MIX_NEEDLE, 180);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					waitMotDiluent = 1;
					if(NewTestInfo.sampDiluteMult == 9)		// 1:500
						workStep = 110;						// 需要打出多余的量
					else
						workStep = 1;						// 直接预混匀					
					SetEValve(EV_ALL,EV_CLOSE);
					break;
				case 110:		// 1:500模式下才有效
#ifdef Puncture					
					if(WithoutPuncture == 0)			// 先排1000
						_SampPumpMotRun(-1000, 180);	// 注入标本
					else
						_SampPumpMotRun(-200, 180);		// 先吐出来大概5uL
#else
					_SampPumpMotRun(-200, 180);			// 先吐出来大概5uL
#endif
					_FluidMotRun(6, 80);
					_DiluentMotRun(4, 140);	
					_EffluentMotRun(18, 240);			// 2017-05-23 200->240
					waitMotFluid = 1;
					waitMotDiluent = 1;
					waitMotSampPump = 1;
					waitEffluent = 1;
					workStep = 1;
					break;
				case 1:		// 预混匀
					_SampPumpMotRun(-63 - _SAMP_PUMP_INTERVAL, 120);	// 注入标本  3ul预混匀	 2017-05-22 80 -> 120
#if 	(DILUTE_TUBE == 14)
					_DiluentMotRun(10, 200);
#elif	(DILUTE_TUBE == 16)
					_DiluentMotRun(4, 200);		// 2017-05-22 140->200
#endif
					waitMotDiluent = 1;
					waitMotSampPump = 1;
					workStep = 2;
					break;
				case 2:		// 排空混匀池
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(10, 220);
					SetMotRunPam(MOT_SAMP_TRUN,64,5,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitEffluent = 1;
					waitMotSampTurn = 1;
					workStep = 3;
					break;
				case 3:		// 取样针运行到混匀池壁位置准备稀释
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixSidePos);
					waitMotSampTurn = 1;
					workStep = 4;
					break;
				case 4:		// 开启样本柱塞泵和稀释液泵，开始混匀
					siTmp = CalSampVolume(NewTestInfo.sampDiluteMult, 0);
					ucTmp = CalSampSyringSpeed(NewTestInfo.sampDiluteMult, 0);
					if(siTmp)
					{
						if(NewTestInfo.sampDiluteMult == 9)			// 1:500直接排指定量
							_SampPumpMotRun(-siTmp, ucTmp);
						else										// 非1:500需要
							_SampPumpMotRun(-siTmp - 200, ucTmp);	// 多排200步空气
					}
					siTmp1 = CalDiluteVolume(NewTestInfo.sampDiluteMult, 0);
					ucTmp = CalDiluentInjectSpeed(NewTestInfo.sampDiluteMult, 0);
					if(siTmp1)
						_DiluentMotRun(siTmp1, ucTmp);		// 根据注液量来设置稀释液注入速度
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("%s $%8d\r\n",strM3107 ,NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3107);
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					checkDiluent = 1;
					waitMotDiluent = 1;
					waitMotSampPump = 1;
					workStep = 5;
					break;
				case 5:		// 根据混匀次数执行不同程序
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					//2016-05-24 添加 > 11,稀释比列1:3,1:4,...,1:40不需要进行2次稀释
					if(NewTestInfo.sampDiluteMult < 9 || NewTestInfo.sampDiluteMult > 11) 
						workStep = 7;
					else
						workStep = 6;
					break;
				case 6:		// 跳往二次稀释
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP);
					waitMotSampNeedle = 1;
					workStep = 0;
					mainStep = 5;
					break;
				case 7:
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_NEEDLE + 30);	// 多下降
					waitMotSampNeedle = 1;
					switch(NewTestInfo.sampDiluteMult)
					{
						case 2:		workStep = 10;break;		// 1:2
						case 12:	workStep = 14;break;		// 1:3
						case 13:	workStep = 14;break;		// 1:4
						case 14:	workStep = 18;break;		// 1:40
						default:	workStep = 18;break;
							
					}
					break;
				case 10:		// 2016-10-31 增加吸样针抽打混匀
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM + 25);
					waitMotSampNeedle = 1;
					workStep = 11;
					break;
				case 11:
					workStep = 16;
					SetDelayTime(MOT_SAMP_NEEDLE, 2);	// 延迟一段时间
					break;
				case 14:
					_SampPumpMotRun(4244, 220);   // 100 / 0.023563 = 4244
					waitMotSampPump = 1;
					workStep = 15;
					Num++;
					break;
				case 15:
					_SampPumpMotRun(-4032, 240); // 95 / 0.023563 = 4032
					waitMotSampPump = 1;
					workStep = 16;
					break;
				case 16:
					Num++;
					if(Num > 5)
					{
						Num = 0;
					//	_SampPumpMotRun(3395, 240); // 80 / 0.02353 = 3395
						workStep = 18;
					}
					else
					{
						_SampPumpMotRun(3820, 240); // 90 / 0.02353 = 3820
						workStep = 17;
					}
					waitMotSampPump = 1;
					break;
				case 17:
					_SampPumpMotRun(-3820, 240);	// 90 / 0.02353 = 3820
					waitMotSampPump = 1;
					workStep = 16;
					break;
				case 18:
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					waitMotSampNeedle = 1;
					if(TestDebugMode == 1)
					{
						workStep = 19;
						break;
					}
					  workStep = 9;  //changed by pan 20161110	吸取一段空气柱		
					  break;
				case 19:
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn = 1;
					workStep = 20;
					_WaitStartKey = 1;
					break;
				case 20:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 21;
					break;		
				case 21:
					SetEValve(EV3, EV_CLOSE);
					MotRun(MOT_SAMP_PUMP, 82);		// 吸入空气段
					waitMotSampPump = 1;
					workStep = 0;
					mainStep = 8;
					break;			
				case 8:		// 在清洗头中清洗取样针内壁
					_EffluentMotRun(10, 180);
					_FluidMotRun(2, 32);		// 清洗针内壁
					waitMotFluid = 1;
					waitEffluent = 1;
					workStep = 9;
					break;				
				case 9:		// 样针吸取一段隔离空气
					SetEValve(EV_ALL,EV_CLOSE);
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_AIR_ISOLATE);
					waitMotSampPump = 1;
					workStep = 0;
					mainStep = 8;
					break;
				}
			break;
		case 5:				// 二次混匀
			switch(workStep){
				case 0:
					SetDelayTime(MOT_SAMP_NEEDLE,5);	// 2017-05-22  20->5
					workStep = 200;
					break;
				case 200:// 由稀释液泵吸取二次混匀原液
					siTmp = CalDilute2Volume(NewTestInfo.sampDiluteMult, 0);
					siTmp = siTmp + 3;				
#if 	(DILUTE_TUBE == 14)
					_DiluentMotRun(-siTmp, 160);
#elif	 (DILUTE_TUBE == 16)
					_DiluentMotRun(-siTmp, 200);			// 2017-05-22 120->200
#endif
					waitMotDiluent = 1;
					workStep = 1;
					break;
				case 1:
				  _EffluentMotRun(15, 240);					// 2017-05-22 200->240
#ifndef Puncture
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP + 200);
#else
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP + 400);
#endif
					waitEffluent = 1;
					waitMotSampNeedle = 1;
					SetEValve(EV2, EV_CLOSE);	// 取样针通道关闭
					SetEValve(EV1, EV_OPEN);	// 开启清洗头洗液供应
					workStep = 2;
					break;
				case 2:	// 清洗取样针外臂，即时排液
					_FluidMotRun(8, 80);		// 开启清洗液洗混匀池,注入1.0mL清洗液(12, 180) //180
					_EffluentMotRun(10, 100);	
					waitMotFluid = 1;
					waitEffluent = 1;
					workStep = 3;
					break;
				case 3:
					SetEValve(EV2, EV_OPEN);	// 开启取样针洗液供应
					workStep = 4;
					ucTmp = 0;
					break;
				case 4:		// 清洗取样针内壁，即时排空
#ifndef Puncture
					_FluidMotRun(16, 80);		// 清洗针内壁(12, 80)
#else
					_FluidMotRun(16, 40);		// 清洗针内壁(12, 40)
#endif
					_EffluentMotRun(20, 240);	// 2017-05-23 100->240
					waitMotFluid = 1;
					waitEffluent = 1;
				//	ucTmp ++;
				//	if(ucTmp < 1)		// 设置清洗次数 2016-08-22 再增加一次
										// 2017-05-23 ucTmp < 2 -> ucTmp < 1
				//		workStep = 4;
				//	else
						workStep = 5;
					break;
				case 5:		// 清洗取样针外壁
					SetEValve(EV2, EV_CLOSE);	// 取样针通道关闭
					SetEValve(EV1, EV_OPEN);	// 开启清洗头洗液供应
					_FluidMotRun(8, 80);		// _FluidMotRun(12, 180);		// 开启清洗液洗混匀池,注入1.0mL清洗液(12, 180)//(12,120)
					waitMotFluid = 1;
					waitEffluent = 1;
					SetEValve(EV3, EV_OPEN);	// 清洗头排液打开
					workStep = 6;
					break;
				case 6:		// 抽干清洗头
					_FluidMotRun(-1, 160);
					_EffluentMotRun(24, 240);
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					workStep = 7;
					break;
				case 7:		// 排空混匀池
					SetEValve(EV_ALL,EV_CLOSE);
					waitEffluent = 1;
					waitMotFluid = 1;
					workStep = 80;
					break;
				case 80:
					_DiluentMotRun(3, 120); 	
					waitMotDiluent = 1;
					workStep = 81;
					break;
				case 81:
					SetDelayTime(MOT_SAMP_NEEDLE, 2);	// 2017-05-22 10->2
					workStep = 82;
					break;
				case 82:
					_EffluentMotRun(10, 180);	// 2017-05-22 100->180
					waitEffluent = 1;
					workStep = 83;
					break;
				case 83:
					SetEValve(EV3, EV_OPEN);	// 清洗头排液打开
					_EffluentMotRun(15, 240);	// 抽干清洗头
					MotRunTo(MOT_SAMP_NEEDLE, 0);
					waitMotSampNeedle = 1;
					workStep = 8;
					break;
				case 8:		//  开始二次混匀
					siTmp = CalDilute2Volume(NewTestInfo.sampDiluteMult, 1);
#if 	(DILUTE_TUBE == 14)
					_DiluentMotRun(siTmp, 160);	// 将稀释管道里的混匀液和稀释液注入混匀池中混匀
#elif 	(DILUTE_TUBE == 16)
					_DiluentMotRun(siTmp, 240); // 2017-05-22 200->240
#endif
					waitMotDiluent = 1;
					waitEffluent = 1;
					workStep = 9;
					break;
				case 9:
					if(TestDebugMode == 1)
					{
						MotRunTo(MOT_SAMP_NEEDLE, 0);
						waitMotSampNeedle = 1;
						workStep = 19;
						mainStep = 4;
						break;
					}
					else
						workStep = 10;
					break;
				case 10:
					SetEValve(EV3, EV_CLOSE);
					MotRun(MOT_SAMP_PUMP, 200);		// 吸入空气段
					waitMotSampPump = 1;
					workStep = 0;
					mainStep = 8;
					break;
				}
			break;
		case 8:	
		// 稀释结束，吸取检测用混匀液100uL，取样针上升到最高点，取样臂运行到起始位，准备滴样,同时清洗混匀池
			checkDiluent = 0;// 2016-10-22 稀释完成,关闭稀释液检测
			switch(workStep){
				case 0:	// 取样针运行到混匀高度
					temp1 = getLiqDetADC(NeedleChannel);
					if(NewTestInfo.sampDiluteMult == 2 || NewTestInfo.sampDiluteMult == 12 
					|| NewTestInfo.sampDiluteMult == 13)
						MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM + 25);
					else if(NewTestInfo.sampDiluteMult < 7 || NewTestInfo.sampDiluteMult == 14)
						MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM);
					else
						MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM - 150);
					waitMotSampNeedle = 1;
					workStep = 1;
					break;
				case 1:		// 吸取检测用混匀液,
					temp2 = getLiqDetADC(NeedleChannel);
					Uart0ReUnable;
					uart_Printf("%s $%4d $%4d\r\n",strM3188, temp1,temp2);
					Uart0ReEnable;
					siTmp = _DropVolume + GetDropVolumeFactor();
					_SampPumpMotRun(siTmp + _SAMP_PUMP_INTERVAL + 400 , 220);	
					Uart0ReUnable;
					uart_Printf("%s $%4d\r\n",strM3174,siTmp);
					Uart0ReEnable;
					waitMotSampPump = 1;
					workStep = 2;
					break;
				case 2:
					SetDelayTime(MOT_SAMP_PUMP, 1);
					workStep = 3;
					break;
				case 3:		// 消返程间隙
					MotRun(MOT_SAMP_PUMP, -_SAMP_PUMP_INTERVAL);
					workStep = 4;	
					break;
				case 4:		// 取样针离开液面 
					_NeedleMotRun(-160,180);//240	
					waitMotSampNeedle = 1;
					waitMotSampPump = 1;
					workStep = 5;
					break;
				case 5:		// 取样针运行到最高点, 同时清洗针外壁
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetEValve(EV1, EV_OPEN);	
					checkFluid = 1;				
					_EffluentMotRun(90, 220);	
				//	_FluidMotRun(4, 80);		// 取消吸混匀液后的洗针步骤
					waitMotSampNeedle = 1;
					workStep = 6;
					break;
				case 6:		// 取样针已经运行到最高点, 清洗结束
				//	MotStop(MOT_FLUID);
					SetDelayTime(MOT_FLUID, 1);
					checkFluid = 0;
					workStep = 7;
					break;
				case 7:		// 启动负压，吸干针外壁
					_FluidMotRun(-1, 80);
					SetEValve(EV3, EV_OPEN);
					SetDelayTime(MOT_SAMP_NEEDLE, 2);	// 2016-09-18   // 2017-05-22  5->2
					workStep = 8;
					break;
				case 8:		// 取样针运行到滴样上方
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn = 1;
					workStep = 9;
					break;
				case 9:		// 清洗头吸干停止, 转换到混匀池排空
					SetEValve(EV_ALL, EV_CLOSE);
					SetDelayTime(MOT_EFFLUENT, 2);
					workStep = 10;
					break;
				case 10:		// 取样针在测试卡上方(起始位)，等待将液体滴入测试卡
					SetEValve(EV3, EV_CLOSE);		// 废液切换到混匀池，继续抽废液
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("%s $%8d\r\n",strM3110 ,NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3110);
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					if(_DropMode == 0)
					{
						mainStep = 9;
						workStep = 0;
					}
					else
						workStep = 11;
					break;
				case 11:
					if(GetNewTestCard == 222)				//抽打混匀准备
					{
						Uart0ReUnable;
						uart_Printf("%s\r\n",strM3145);
						Uart0ReEnable;
						MixHeight = GetMixHeight();
						_NeedleMotRunTo(MixHeight,240);
						waitMotSampNeedle = 1;
						workStep = 12;
						break;
					}
					else if(GetNewTestCard == 250)			// 取片失败 	
					{
						MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
						waitMotSampTurn = 1;
						mainStep = 9;
						workStep = 5;
					}
					break;
				case 12:
					_SampPumpMotRun(-4244, 240);	// 第一次吐100ul  100 / 0.023563 = 4244
					//_SampPumpMotRun(-4371, 240);	// 第一次吐103ul  103 / 0.023563 = 4371
					Num++;
					waitMotSampPump = 1;
					workStep = 13;
					break;
				case 13:	// 离开抽打混匀高度
					_NeedleMotRunTo(MixHeight - 200,240);
					waitMotSampNeedle = 1;
					workStep = 14;
					break;
				case 14:
					_SampPumpMotRun(424, 120);		// 吸10ul空气   10 / 0.023563 = 424
					waitMotSampPump = 1;
					workStep = 15;
					break;
				case 15:	// 降低至抽打混匀高度
					_NeedleMotRunTo(MixHeight,240);
					waitMotSampNeedle = 1;
					workStep = 20;
					break;
//#ifndef _FluidPumMix 
				// 微量注塞泵抽打混匀
				case 20:
					//_SampPumpMotRun(4032, 240);		// 吸95ul  95 / 0.023563 = 4032
					_SampPumpMotRun(3820, 240);			// 吸90ul  90 / 0.023563 = 3820
					waitMotSampPump = 1;
					workStep = 21;
					break;
				case 21:		// 注入90液体
					_SampPumpMotRun(-3820, 240);	// 吐90 90/0.023563 = 3820
					Num++;
					waitMotSampPump = 1;
					workStep = 22;
					break;
				case 22:
					_SampPumpMotRun(3820, 240);		// 吸90 90/0.023563 = 3820	
					if(Num < _ReMixNum - 1)
						workStep = 21;
					else
						workStep = 23;
					waitMotSampPump = 1;
				   break;
				case 23:
					//_SampPumpMotRun(-4032,240);			// 吐95 95/0.023563 = 4032
					_SampPumpMotRun(-3820,240);				// 吐90 90/0.023563 = 3820
					waitMotSampPump = 1;
					workStep = 24;
					break;
				case 24:
					_SampPumpMotRun(3395, 240);			// 吸80 80/0.023563 = 3395	
					workStep = 25;
					waitMotSampPump = 1;
				   break;
				case 25:
				   SetDelayTime(MOT_SAMP_NEEDLE,2);
				   workStep = 16;
				   break;
				case 16:
				   Num = 0;
				   workStep = 17;
				   Uart0ReUnable;
				   uart_Printf("%s\r\n",strM3199);
				   Uart0ReEnable;
				   break;
				case 17:
				   _NeedleMotRunTo(0,240);	//吸样针回到零位
				   waitMotSampNeedle = 1;
				   workStep = 18;
				   break;
				case 18:		//抽打混匀结束
				   SetDelayTime(MOT_SAMP_NEEDLE,3);
				   GetNewTestCard = 233;		// 设置取出测试卡标识,抽打混匀结束,准备滴样
				   mainStep = 9;
				   workStep = 0;
				   break;
				}
			break;
		case 9:				// 等待混匀滴入干片中,然后清洗
			switch(workStep){
				case 0:		// 准备滴样，检查测试队列是否有空位加入，否则等待	// 检测队列任务由此加入开始
					if(GetNewTestCard == 254)	// 测试卡片已经准备完毕，下面开始将测试插入检测队列
					{		
						CurInsertRingNum = RingQueueInsertCalculate();	// 查找转盘位置						
						if(CurInsertRingNum != 0xff)	// 如果转盘位置查找成功，测试队列插入
						{	
							FindNum = 0;
							ucTmp = InsertNewTest(&NewTestInfo,CurInsertRingNum);	// 测试队列插入
							if(ucTmp == 0)	// 新测试插入计算成功
							{
								// 将新测试插入上面计算出来的转盘位置
								RingQueueInsert(CurInsertRingNum,&NewTestInfo);		
								insertflag[CurInsertRingNum] = 0;
								// 加入卸片队列
								if(NewTestInfo.testTime1)
									UnloadQueueAdd(CurInsertRingNum, SecondCount+NewTestInfo.testTime1+TEST_CYCLE_TIME+25); 
								else
									UnloadQueueAdd(CurInsertRingNum, SecondCount+NewTestInfo.testTime0+TEST_CYCLE_TIME+25);
								workStep = 1;
							}
						}
						else
						{
							// 如果转盘位置查找失败，5s之后重新查找
							SetDelayTime(MOT_SAMP_NEEDLE,50);	
							FindNum++;
							if(FindNum >= 3)	// 连续3次转盘位置查找失败,当前转盘位置为前一次+1
							{
								FindNum = 0;
								//RingQueue.prevNum += 1;
								CurInsertRingNum = RingQueue.prevNum++;
								if(CurInsertRingNum >= RING_QUEUE_NUM)
									CurInsertRingNum -= RING_QUEUE_NUM;
								RingQueueInsert(CurInsertRingNum,&NewTestInfo);		
								insertflag[CurInsertRingNum] = 0;
								// 加入卸片队列
								if(NewTestInfo.testTime1)
									UnloadQueueAdd(CurInsertRingNum, SecondCount+NewTestInfo.testTime1+TEST_CYCLE_TIME+25);
								else
									UnloadQueueAdd(CurInsertRingNum, SecondCount+NewTestInfo.testTime0+TEST_CYCLE_TIME+25);
								workStep = 1;
							}
						}
					}
					else if(GetNewTestCard == 250)
					{
					//	GetNewTestCard = 0;
						MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
						waitMotSampTurn = 1;
						workStep = 5;
					}
					break;
				case 1:		// 取样针下降到测试卡上方
					_NeedleMotRunTo(DropHeight,180);	// 触碰到试剂片
					waitMotSampNeedle = 1;
					workStep = 2;
					break;
				case 2:		// 注入液体
					if(_DropVolume != 0)	// 2016-05-19 添加滴样量为0处理
					{
						siTmp = _DropVolume +  GetDropVolumeFactor();
						_SampPumpMotRun(-siTmp, 180);
						Uart0ReUnable;
						uart_Printf("%s $%4d\r\n", strM3141, siTmp);
						Uart0ReEnable;
						waitMotSampPump = 1;
					}
					workStep = 3;
					break;
				case 3:		// 取样针上升到最高点
					SetBeepAck();
					// 标本已加入，测试开始
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("%s $%8d",strM3111, NewTestInfo.testSerial);
					uart_Printf(" $%8d\r\n",(unsigned long)SecondCount);
					Uart0ReEnable;
#else		
					Uart0ReUnable;
					uart_Printf("%s $ ",strM3111);
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf(" $ ");
					uart0SendInt(SecondCount);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					_NeedleMotRunTo(0,180);
					GetNewTestCard = 255;		// 从case 4移动上来，保证滴样的样品可以测试
					_FluidMotRun(5, 64);		// 开启清洗液洗混匀池,注入1.4mL清洗液
#if 	(DILUTE_TUBE == 14)
					_DiluentMotRun(8, 160);		// 注入稀释液清洗
#elif 	(DILUTE_TUBE == 16)
					_DiluentMotRun(3, 200);		
#endif
					checkFluid = 1;
					waitMotSampNeedle = 1;
					workStep = 4;
					break;
				case 4:		// 取样臂运行到混匀池中部上方
					_EffluentMotRun(5, 220);
					_FluidMotRun(-1, 100);		// 回抽
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:		// 取样针下降到混匀池顶部
					MotStop(MOT_EFFLUENT);		// 如果废液泵未停止，将其停止
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP);
					waitMotSampNeedle = 1;
					waitMotFluid = 1;	// 等待清洗液注入混匀池完毕
					SetEValve(EV2,EV_OPEN);		// 开启取样针通道准备清洗取样针内壁
					SetEValve(EV3,EV_CLOSE);
					//workStep = 6;	// 2016-09-18
					//if(NewTestInfo.sampDiluteMult == 1 || NewTestInfo.sampDiluteMult == 2 || NewTestInfo.sampDiluteMult == 6)
					//add by pan 20161110稀释倍数是50的时候
					if(NewTestInfo.sampDiluteMult < 7 || NewTestInfo.sampDiluteMult > 11)
					// 稀释比例小于100,按原倍清洗模式
					{
						sc = 0;		// 原倍清洗次数
						workStep = 6;
					}
					else
					{
						workStep = 10;	
					}
					break;
				case 6:		// 原倍模式取样针内壁清洗
					sc ++;
					SetEValve(EV2,EV_OPEN);	
					_FluidMotRun(10, 100);		// 注入清洗液
					MotInitCheck(MOT_SAMP_PUMP);	// 样品注射器回零
					waitMotFluid = 1;
					workStep = 7;
					break;
				case 7:
					_EffluentMotRun(15, 240);		// 2017-05-22 220->240
					waitEffluent = 1;
				//	if(sc < 3)						// 2017-05-23 sc < 3 -> sc < 1
				//		workStep = 6;
				//	else
						workStep = 10;
					break;
				case 10:		// 稀释模式清洗取样针内壁
					//if(NewTestInfo.sampDiluteMult == 1)	// 2016-09-18
					//_SampPumpMotRunTo(0, 220);			// 2016-09-18
					_FluidMotRun(8, 100);		// 注入清洗液
					MotInitCheck(MOT_SAMP_PUMP);	// 样品注射器回零
					waitMotSampPump = 1;
					waitMotFluid = 1;
					workStep = 11;
					break;
				case 11:
					SetEValve(EV2,EV_CLOSE);
					SetEValve(EV1,EV_OPEN);
					SetDelayTime(MOT_SAMP_NEEDLE, 2); 
					workStep = 12;
					break;
				case 12:		// 正常稀释模式 清洗取样针外壁
					SetEValve(EV2,EV_CLOSE);
					SetEValve(EV1,EV_OPEN);
					_FluidMotRun(8,120);					
					waitMotFluid = 1;
					workStep = 13;
					break;
				case 13:		// 启动负压
					checkFluid = 0;
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(55, 240);		
					workStep = 14;
					break;
				case 14:		// 取样针上升，吸干清洗池
					_FluidMotRun(-1, 160);
					MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotSampNeedle = 1;
					waitMotFluid = 1;
					if(NewTestInfo.sampDiluteMult ==13)
					{
						sc=4;
						workStep = 100;
					}
					else
					workStep = 15;
					break;
					
				case 100:
				    sc--;
				    SetEValve(EV3, EV_CLOSE);	// 废液泵转到排混匀池
				    _DiluentMotRun(16, 140);	
					workStep = 101;
					waitMotDiluent = 1;
					SetDelayTime(MOT_FLUID, 2);
					break;
				case 101:
				    _EffluentMotRun(20, 240);	
					SetDelayTime(MOT_FLUID, 2);
					waitEffluent = 1;
					if(sc==0)
					workStep = 102;
					else
					workStep = 100;
					break;
				case 102:
				    _DiluentMotRun(16, 140);					
					waitMotDiluent = 1;
					SetDelayTime(MOT_FLUID, 2);
					workStep = 103;
					break;	
				case 103:
				    MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM - 150);
					waitMotSampNeedle = 1;
					SetDelayTime(MOT_FLUID, 2);
					workStep = 104;
					break;
				case 104:
				    SetEValve(EV1,EV_CLOSE);
					SetEValve(EV2, EV_OPEN);	// 开启清洗液取样针通道，准备由清洗液泵吸入高浓度清洗液
					SetDelayTime(MOT_FLUID, 2);
					
					sc=6;				
					workStep = 105;
					break;
				case 105:
				    sc--;
					MotRun(MOT_FLUID, -150);//吸
					waitMotFluid = 1;
					SetDelayTime(MOT_FLUID, 2);
					if(sc==0)
					workStep = 106;
					else
					workStep = 109;
					break;
				case 109:
				    MotRun(MOT_FLUID, 140);//吐
					SetDelayTime(MOT_FLUID, 2);
					waitMotFluid = 1;
					workStep = 105;
					break;		
				case 106:
				    SetEValve(EV2, EV_CLOSE);
					SetEValve(EV1, EV_CLOSE);
					SetEValve(EV3, EV_CLOSE);
					SetDelayTime(MOT_FLUID, 2);
					workStep = 107;
					break;
				case 107:
				    _EffluentMotRun(20, 240);	
					waitEffluent = 1;
					SetDelayTime(MOT_FLUID, 2);
					workStep = 108;
					break;
					
				case 108:
				    MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotSampNeedle = 1;
					workStep = 15;
					break;
				
				
				case 15:	// 取样臂旋转到取样位
					SetEValve(EV1,EV_CLOSE);
					SetMotRunPam(MOT_SAMP_TRUN,255,5,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
					waitMotSampTurn = 1;
					workStep = 19;
					break;
				case 19:
					SetEValve(EV3, EV_CLOSE);	// 废液泵转到排混匀池
					workStep = 16;
					break;
				case 16:	// 取样针下降到取样位
				//	if(InsertRingFlag == 0)		break;		// 试剂片未推进装盘
					InsertRingFlag = 0;
#ifndef Puncture				
					_NeedleMotRunTo(_POS_SAMP_DOWN, 255);
#endif					
					// 预吸空气段
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_INTERVAL + _SAMP_PUMP_AIR_ISOLATE);
					waitMotSampNeedle = 1;
					workStep = 17;
					break;
				case 17:	// 
					SetBeepPrompt();
					SetStateLedFree();
					workStep = 18;
					break;
				case 18:	// 结束
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3102);
					Uart0ReEnable;
					mainStep = 1;
					workStep = 0;
					break;
				}
			break;
		case 10:	// 结束退出
			switch(workStep){
				case 0:
					_EffluentMotRun(30, 220);
					_NeedleMotRunTo(0, 180);//240
					waitMotSampNeedle = 1;
					workStep = 1;
					TestALampClose();	// 关闭光源
					SetStateLedBusy();	// 状态指示灯为红
					break;
				case 1:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					waitEffluent = 1;
					workStep = 2;
					break;
				case 2:
					mainStep = 0;
					workStep = 0;
					return 2;
					break;
				default:
					break;
				}
			return 1;
			break;
		case 11:	// 异常处理
			switch(workStep){
				case 0:		// 停止当前运行
					SetDelayTime(MOT_EFFLUENT, 30);
					workStep = 1;
					break;
				case 1:		// 废液泵延迟停止
					MotStop(MOT_EFFLUENT);
					workStep = 2;
					break;
				case 2:		// 取样针回到起始点
					_NeedleMotRunTo(0, 180);//240
					waitMotSampNeedle = 1;
					workStep = 3;
					break;
				case 3:		// 废液泵排液
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(30, 200);
					waitEffluent = 1;
					workStep = 4;
					break;
				case 4:		// 取样针运行到清洗池上方
					MotInitCheck(MOT_SAMP_TRUN);
				//	MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:
					
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 6;
					break;
				case 6:		// 取样针下降到清洗池中准备清洗
					_NeedleMotRunTo(_POS_MIX_TOP, 180);//240
					waitMotSampNeedle = 1;
					workStep = 7;
					break;
				case 7:		// 注入清洗液到混匀池中，然后跳转到正常的测试结束清洗程序，继续运行正常程序
					_FluidMotRun(10, 80);		// 开启清洗液洗混匀池,注入1.4mL清洗液
					waitMotFluid = 1;	// 等待清洗液注入混匀池完毕
					mainStep = 9;		// 直接进入清洗
					workStep = 6;
					InsertRingFlag = 1;	// 保证吸样针下来
					break;
				default:
					break;
				}
			break;
		case 12:	// 清洗液空
			switch(workStep){
				case 0:
					if(0 == CleanMode)
					{
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("!3901 $%8d\r\n", NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("!3901 $ ");
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif	
					}
					else
					{
						if(1 == CleanMode)
							CleanMode = 250;
						else if(2 == CleanMode)
							CleanMode = 255;
						Uart0ReUnable;
						uart_Printf("%s \r\n", strE3905);
						Uart0ReEnable;
						mainStep = 0;
						workStep = 16;
						break;
					}
					MotStop(MOT_SAMP_NEEDLE);
					MotStop(MOT_FLUID);
					SetDelayTime(MOT_EFFLUENT, 30);
					workStep = 1;
					break;
				case 1:
					MotStop(MOT_EFFLUENT);
					workStep = 2;
					break;
				case 2:
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(30, 200);
					waitEffluent = 1;
					workStep = 3;
					break;
				case 3:
					MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotSampNeedle = 1;
					workStep = 4;
					break;
				case 4:	// 取样针回到起始点
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:	// 取样针运行到混匀池中准备清洗针内壁
					MotRunTo(MOT_SAMP_TRUN,NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 6;
					break;
				case 6:
					_NeedleMotRunTo(_POS_MIX_TOP, 200);//240
					SetEValve(EV2, EV_OPEN);
					waitMotSampNeedle = 1;
					workStep = 7;
					break;
				case 7:
					_FluidMotRun(10, 60);		// 开启清洗液洗混匀池,注入1.4mL清洗液
					_EffluentMotRun(20, 80);
					waitMotFluid = 1;	// 等待清洗液注入混匀池完毕
					waitEffluent = 1;
					workStep = 8;
					break;
				case 8:
					SetEValve(EV_ALL, EV_CLOSE);
					workStep = 9;
					break;
				case 9:
					SetDelayTime(MOT_EFFLUENT, 2);
					ucTmp = _PrimingFluid();
					if(ucTmp == 1){	// 自动灌注完成
						workStep = 3;
						mainStep = 11;
						}
					else if(ucTmp == 0xff){		// 自动灌注失败，进入程序开始阶段等待手动开始液体灌注
					/*
#ifndef UartSendLong
						Uart0ReUnable;
						uart_Printf("%s $%8d\r\n", strE3902,NewTestInfo.testSerial);
						Uart0ReEnable;
#else
						Uart0ReUnable;
						uart_Printf("%s $ ",strE3902);
						uart0SendInt(NewTestInfo.testSerial);
						uart_Printf("\r\n");
						Uart0ReEnable;
#endif
					*/
						Uart0ReUnable;
						uart_Printf("%s\r\n", strE3902);
						Uart0ReEnable;
						mainStep = 0;
						workStep = 1;
						}
					break;
				default:
					break;
				}
			break;
		case 13:	// 稀释液空
			switch(workStep){
				case 0:
#ifndef UartSendLong
					Uart0ReUnable;
					uart_Printf("!3903 $%8d\r\n", NewTestInfo.testSerial);
					Uart0ReEnable;
#else
					Uart0ReUnable;
					uart_Printf("!3903 $ ");
					uart0SendInt(NewTestInfo.testSerial);
					uart_Printf("\r\n");
					Uart0ReEnable;
#endif
					MotStop(MOT_SAMP_NEEDLE);
					MotStop(MOT_DILUENT);
					SetDelayTime(MOT_EFFLUENT, 30);
					workStep = 1;
					break;
				case 1:
					MotStop(MOT_EFFLUENT);
					workStep = 2;
					break;
				case 2:
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(30, 200);
					waitEffluent = 1;
					workStep = 3;
					break;
				case 3:
					MotInitCheck(MOT_SAMP_NEEDLE);
				//	_NeedleMotRunTo(0, 240);
					waitMotSampNeedle = 1;
					workStep = 4;
					break;
				case 4:	// 取样针回到起始点
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:	// 取样针运行到混匀池中准备清洗针内壁
					MotRunTo(MOT_SAMP_TRUN,NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 6;
					break;
				case 6:
					_NeedleMotRunTo(_POS_MIX_TOP, 180);//240
					SetEValve(EV2, EV_OPEN);
					waitMotSampNeedle = 1;
					workStep = 7;
					break;
				case 7:
					_FluidMotRun(10, 60);		// 开启清洗液洗混匀池,注入1.4mL清洗液
					_EffluentMotRun(20, 80);
					waitMotFluid = 1;	// 等待清洗液注入混匀池完毕
					waitEffluent = 1;
					workStep = 8;
					break;
				case 8:
					SetEValve(EV_ALL, EV_CLOSE);
					workStep = 9;
					break;
				case 9:
					SetDelayTime(MOT_EFFLUENT, 2);
					ucTmp = _PrimingDiluent();
					if(ucTmp == 1){	// 灌注完成
						workStep = 3;
						mainStep = 11;
						}
					else if(ucTmp == 0xff){		// 自动灌注失败，进入程序开始阶段等待手动开始液体灌注
					/*
#ifndef UartSendLong
						Uart0ReUnable;
						uart_Printf("%s $%8d\r\n", strE3904,NewTestInfo.testSerial);
						Uart0ReEnable;
#else
						Uart0ReUnable;
						uart_Printf("%s $ ",strE3904);
						uart0SendInt(NewTestInfo.testSerial);
						uart_Printf("\r\n");
						Uart0ReEnable;
#endif
					*/
						Uart0ReUnable;
						uart_Printf("%s\r\n", strE3904);
						Uart0ReEnable;
						mainStep = 0;
						workStep = 1;
						}
					break;
				default:
					break;
				}
			break;
		case 20:	// 进入待机状态, 取样针回零位
			switch(workStep)
			{
				case 0:	// 取样针上升到最高点
					SetStateLedBusy();
					_NeedleMotRunTo(0, 180);// 240
					waitMotSampNeedle = 1;
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3190);
					Uart0ReEnable;
					workStep = 1;
					break;
				case 1:	// 取样针旋转到起始位
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 2;
					break;
				case 2:	// 进入待机
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM3191);		// 取样休眠状态
				//	uart_Printf("%s\r\n",strM3101);		// 请按吸样键开始
					Uart0ReEnable;
					JumpMode = 2;						// 休眠模式,按键之后应该进入液路自检
					mainStep = 0;
					workStep = 1;
					break;
				default:
					break;
			}
			break;
		case 100:	// 吸入高浓度清洗液清洗
			switch(workStep)
			{
				/*      //原先的强力清洗模式   deleted by pan  20161227
				case 0:
					SetEValve(EV2, EV_OPEN);	// 开启清洗液取样针通道，准备由清洗液泵吸入高浓度清洗液
					SetDelayTime(MOT_FLUID, 2);
					workStep = 1;
					break;
				case 1:
					_FluidMotRun(-2, 120);		// 吸入高浓度清洗液
					waitMotFluid = 1;
					workStep = 2;
					break;
				case 2:
					SetEValve(EV2, EV_CLOSE);
					_NeedleMotRunTo(0, 180);	// 取样针上升	// 200
					waitMotSampNeedle = 1;
					workStep = 3;
					break;
				case 3:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);		// 取样臂运行到混匀池壁上方
					waitMotSampTurn = 1;
					SetDelayTime(MOT_SAMP_TRUN, 10*30);		// 浸泡30秒
					workStep = 5;
					break;
				case 5:
					_NeedleMotRunTo(_POS_MIX_BUTTOM, 200); // 240
					waitMotSampNeedle = 1;
					sc = 5;
					mainStep = 0;
					workStep = 9;
					break;
				default:
					break;
				}
			break;
			*/   //原先的强力清洗模式   deleted by pan  20161227
				
				case 0:
#ifdef Puncture
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);
					waitMotSampNeedle = 1;
					_WaitStartKey = 1;
#endif
					workStep = 100;
					break;
				case 100:
					SetEValve(EV2, EV_OPEN);	// 开启清洗液取样针通道，准备由清洗液泵吸入高浓度清洗液
					SetDelayTime(MOT_FLUID, 2);
					workStep = 1;
					break;
				case 1:
					//_FluidMotRun(-2, 120);		// 吸入高浓度清洗液120uL
					MotRun(MOT_FLUID, -120);
					waitMotFluid = 1;
					workStep = 2;
					//workStep = 100;
					break;
				/*
				case 100:	
					SetEValve(EV3, EV_OPEN);
					SetEValve(EV1, EV_OPEN);
					SetBeepPrompt();
					_EffluentMotRun(70, 240);
					SetDelayTime(MOT_EFFLUENT, 2);	// 延迟一段时间，先建立负压
					workStep = 101;
					break;
				case 101:				// 开启洗液泵，清洗针外壁
					_FluidMotRun(20, 40);
					waitMotSampNeedle = 1;
					_NeedleMotRunTo(0, 180);	// 取样针上升
					//workStep = 6; //2016-09-18
					workStep = 102;	//2016-09-18
					break;
				
				case 102:		// 延迟1秒后停止清洗头吸空, 转换到混匀池排空
					SetEValve(EV_ALL,EV_CLOSE);
					//waitMotSampTurn = 1;
					SetEValve(EV2, EV_OPEN);	
					workStep = 3;
					break;
					*/
				case 2:
					//SetEValve(EV2, EV_CLOSE);
					_NeedleMotRunTo(0, 180);	// 取样针上升	// 200
					waitMotSampNeedle = 1;
					workStep = 3;
					break;
				case 3:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);		// 取样臂运行到混匀池壁上方
					waitMotSampTurn = 1;
					SetDelayTime(MOT_SAMP_TRUN, 10*3);		// 浸泡3秒    改成更长时间
					workStep = 5;
					break;				
				case 5:
					_NeedleMotRunTo(_POS_MIX_BUTTOM, 200); // 240
					waitMotSampNeedle = 1;
					workStep = 7;
					break;	
				case 7:
					MotRun(MOT_FLUID, 1200);
					checkFluid = 1;
					waitMotFluid = 1;
					workStep = 8;
					sc = 125;
					break;
				case 8:		
				    sc--;		
					MotRun(MOT_FLUID, -120);
					waitMotFluid = 1;
					workStep = 9;
					break;
			    case 9:	
				    MotRun(MOT_FLUID, 120);
					waitMotFluid = 1;	
					if(sc % 5 == 0)	
						SetDelayTime(MOT_FLUID, 10*6);	// 每抽打5次暂停6秒
					
					if(sc==0)
					{
					   workStep = 6;
					// sc=15;
					}
					else
					workStep = 8;		
					break;
				case 6:	
				    _EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					workStep = 13;
					sc=5;		//清洗外壁5次
					break;
				case 13:		// 清洗取样针外壁
				    sc--;					
					SetEValve(EV2, EV_CLOSE);	// 取样针通道关闭
					SetEValve(EV1, EV_OPEN);	// 开启清洗头洗液供应
					_FluidMotRun(8, 80);		// _FluidMotRun(12, 180);		// 开启清洗液洗混匀池,注入1.0mL清洗液(12, 180)//(12,120)
					waitMotFluid = 1;
					waitEffluent = 1;
					SetEValve(EV3, EV_OPEN);	// 清洗头排液打开
					workStep = 14;
					break;
				case 14:		// 抽干清洗头
					_FluidMotRun(-1, 160);
					_EffluentMotRun(24, 240);
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					workStep = 15;
					break;
				case 15:	
					// 抽干混匀池
					SetEValve(EV3,EV_CLOSE);	// 清洗头排液打开
					_EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					if (sc!=0)
					workStep = 13;
					else
					{
					workStep = 10;
					sc=15;     //清洗内壁15次
					}
					break;
				case 10:	// 清洗和灌注取样针通道
				    sc--;
					SetEValve(EV2, EV_OPEN);
					SetEValve(EV1, EV_CLOSE);
					SetEValve(EV3, EV_CLOSE);
					MotInitCheck(MOT_SAMP_PUMP);
					_FluidMotRun(16, 64);		// 注入1.4mL清洗液
					waitMotFluid = 1;
					SetDelayTime(MOT_FLUID, 40);
					workStep = 11;
					break;
				case 11:
					_EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					if(sc == 0)
					workStep = 12;
					else
					workStep = 10;
					break;			   
				case 12:		
					sc = 5;
					mainStep = 0;
					workStep = 9;
					break;
				default:
					break;
				}
			break;
		case 105:	// 自动清洗
			switch(workStep)
			{
				case 0:
					_NeedleMotRunTo(0, 180);	// 取样针上升 // 200
					waitMotSampNeedle = 1;
					workStep = 3;
					break;
				case 3:
					SetMotRunPam(MOT_SAMP_TRUN,255,10,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN,0);
					waitMotSampTurn = 1;
					workStep = 4;
					break;
				case 4:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);		// 取样臂运行到混匀池壁上方
					waitMotSampTurn = 1;
					workStep = 5;
					break;
				case 5:
					_NeedleMotRunTo(_POS_MIX_BUTTOM, 180);	 // 240
					waitMotSampNeedle = 1;
					sc = 5;
					checkFluid = 1;
					mainStep = 0;
					workStep = 9;
					break;
				default:
					break;
				}
			break;
		default:
			break;
		}
	return 0;
}

signed int CalSampVolume(unsigned char multipNum, unsigned char diluteTime)
{
	unsigned long n, l;
	signed int m;
	n = 0;
	// 设置不同稀释比例下标准吸样量
#ifndef Puncture
	l = (unsigned long)DiluentCoff[multipNum];	// 读取稀释校准因数
#else
	if(WithoutPuncture != 0)
		l = (unsigned long)DiluentCoff[multipNum];	// 读取稀释校准因数
	else
	{
		l = (unsigned long)DiluentCoff[8];	// 读取稀释校准因数
		uart_Printf("*9944 CurrentDiluentCoff $%4d\r\n", l);
	}
#endif
	if(diluteTime == 0)
	{
		switch(multipNum)		// 第一次稀释
		{
			// 一次稀释
			case 1:		//m = _DropVolume;	// 吸样量 == 滴液量
						m = _DropVolume+_SAMP_PUMP_INTERVAL + 210 - 45;	return m; break;
		//	case 2:		n = l*4244;	break;	// 1:2	100/0.023562 = 4244	
			case 2:		n = l*3400; break;  // 1:2  80/0.023562  = 3395
			case 3:		n = l*4244;	break;	// 1:5	100/0.023562 = 4244
			case 4:		n = l*2355;	break;	// 1:10	55.5/0.023562 = 2355
			case 5:		n = l*1116;	break;	// 1:20	26.3/0.023562 = 1116
		//	case 6:		n = l*864;	break;	// 1:50	20.4/0.023562 = 864
		//	case 6:		n = l*1039;	break;	// 1:50	24.5/0.023562 = 1039  // 4015 设置为114准确
		//	case 6: 	n = l*1183; break;  // 1183 = 1039 * 672 / 590,其中672为设置114的值，590为设置100的值
		//	case 6: 	n = l*1261; break;  // 29.7ul
		//	case 6: 	n = l*1230; break;  // 29ul
		//	case 6: 	n = l*1200; break;  // 28.3ul
		//	case 6: 	n = l*1220; break;  // 28.7ul
		//	case 6:		n = l*1060; break;  // 25ul
		//	case 6:		n = l*1082; break;  // 25.5ul
		//	case 6:		n = l*1188; break;  // 28ul
			case 6:		n = l*923; break;   // 2016-10-25调整  21.75ul
			case 7:		n = l*429;	break;	// 1:100	10.1/0.023562 = 429
			case 8:		n = l*212;	break;	// 1:200	5/0.023562 = 212
			// 高倍率的第一次稀释
		//	case 9:		n = l*429;	break;	// 1:100 X 5	= 1:500
			case 9:		n = l*300;	break;	// 1:100 X 5	= 1:500	7.07/0.023562 = 300
			case 10:	n = l*429;	break;	// 1:100 X 10	= 1:1000	10.1/0.023562 = 429
			case 11:	n = l*212;	break;	// 1:200 X 10	= 1:2000	5/0.023562 = 212
			//  2016-05-18
			case 12:	n = l*4244;	break;	// 1:3  100/0.023562 = 4244		
			//case 13:	n = l*4244;	break;	// 1:4	100/0.023562 = 4244
			case 13:	n = l*2831; break;	// 1:4  66.7/0.023562 = 2831 
			case 14:    n = l*1634;	break;	// 1:40	38.5/0.023562 = 1634
			default:	n = 0;		break;
			}
		}
	n = n / _DILUENT_PUMP_BASE_COEFF;
	m = (signed int)n;
	return m;
}

signed int CalDiluteVolume(unsigned char multipNum, unsigned char diluteTime){
	// 计算各种稀释比例用的稀释液量，结果为稀释泵的n个单位流量
	signed int n;
	if(diluteTime == 0){
		switch(multipNum){		// 第一次稀释
			// 一次稀释
			case 1:		n = 0;		break;		// 1:1
			case 2:		n = 1;		break;		// 1:2		100:(100+100)
			case 3:		n = 4;		break;		// 1:5		100:(100+400)
			case 4:		n = 5;		break;		// 1:10		55.5:(55.5+500)	
			case 5:		n = 5;		break;		// 1:20		26.3:(26.3+500)	
		//	case 6:		n = 10;		break;		// 1:50		20.4:(20.4+1000)
		    case 6:		n = 12;		break;		// 1:50		24.5:(24.5+1200)
			case 7:		n = 10;		break;		// 1:100	10.1:(10.1+1000)
			case 8:		n = 10;		break;		// 1:200	5:(5+1000)
		//	case 9:		n = 10;		break;		// 1:500	1:100	10.1:(10.1+1000)
			case 9:		n = 7;		break;		// 1:500	1:100	7.07:(7.07+700)
			case 10:	n = 10;		break;		// 1:1000	1:100	10.1:(10.1+1000)
			case 11:	n = 10;		break;		// 1:2000	1:200	5:(5+1000)
			// 2016-05-18
			case 12:	n = 2;		break;		// 1:3		100:(100+200)
			//case 13:	n = 3;		break;		// 1:4		100:(100+300)
			case 13:	n = 2;		break;		// 1:4      66.7:(66.7+200)
			case 14:	n = 15;		break;		// 1:40		38.5:(38.5+1500)
			default:	 n = 0;		break;
			}
		}
	return n;
}
signed int CalDilute2Volume(unsigned char multipNum, unsigned char dilutetype){
	signed int n;
	if(dilutetype == 0)
	{	// 原液
		switch(multipNum)
		{
			case 2:		n = 2;	break;			// 1:2 = 100 + 100 
			case 12:	n = 3;	break;			// 1:3 = 100 + 200
			case 13:	n = 4;	break;			// 1:4 = 100 + 300
			case 3:		
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 14:	n = 5;	break;		// 500ul
			case 9:		n = 2;	break;		// 1:500  = 1:100 X 5
			case 10:	n = 1;	break;		// 1:1000	= 1:100 X 10	
			case 11:	n = 1;	break;		// 1:2000	= 1:200 X 10	
			default:	n = 0;	break;
		}
	}
	else
	{	// 混合液量
		switch(multipNum)
		{
			case 2:		n = 2;	break;
			case 12:	n = 3;	break;
			case 13:	n = 4;	break;
			case 3:		
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 14:	n = 5;	break;
			case 9:		n = 10;		break;		// 1:500	 = 1:100 X 5	
			case 10:	n = 10;		break;		// 1:1000	 = 1:100 X 10
			case 11:	n = 10;		break;		// 1:2000	 = 1:200 X 10	
			default:	 n = 0;		break;
		}
	}
	return n;
}
unsigned char CalSampSyringSpeed(unsigned char multipNum, unsigned char diluteTime){
	// 计算标本注射速度
	unsigned char n;
	if(diluteTime == 0){
		switch(multipNum){		// 第一次稀释
			// 一次稀释
			case 1:		n = 200;	break;		// 100	1:1
			case 2:		n = 200;	break;		// 100	1:2
			case 3:		n = 200;	break;		// 100	1:5
			case 4:		n = 200;	break;		// 55.5	1:10
			case 5:		n = 200;	break;		// 26.3	1:20
			//case 6:		n = 140;	break;		// 20.4	1:50
			case 6:		n = 200;	break;		// 20.4	1:50
			case 7:		n = 80;		break;		// 10.1	1:100
			case 8:		n = 60;		break;		// 5		1:200
			case 9:								// 1:500
			case 10:	n = 80;		break;		// 1:1000
			case 11:	n = 40;		break;		// 1:2000		32
			// 2016-05-18
			case 12:	n = 200;	break;		// 100	1:3
			//case 13:	n = 200;	break;		// 100	1:4
			case 13:	n = 180;	break;		// 66.7 1:4
			//case 14:	n = 140;	break;		// 20.5	1:40
			case 14:	n = 200;	break;		// 38.5	1:40
			default:	 n = 180;	break;
			}
		}
	return n;
}

unsigned char CalDiluentInjectSpeed(unsigned char multipNum, unsigned char diluteTime){
	// 计算稀释液注入速度
	// 计算标本注射速度
	unsigned char n;
	if(diluteTime == 0){
		switch(multipNum){		// 第一次稀释
			// 一次稀释
			case 1:		n = 0;		break;		// 0		1:1
			//case 2:		n = 4;		break;		// 100	1:2
			case 2:		n = 25;		break;		// 100	1:2
			case 3:		n = 16;		break;		// 400	1:5
			case 4:		n = 40;		break;		// 500	1:10
			case 5:		n = 75;		break;		// 500	1:20
			case 6:		n = 160;	break;		// 1000	1:50
			case 7:		n = 160;	break;		// 1000	1:100
			case 8:		n = 160;	break;		// 1000		1:200
			case 9:								// 1:500
			case 10:							// 1:1000
			case 11:	n = 200;	break;		// 1:2000		160-30(cv <1.5)
						// 2017-05-23  n=160-10 -> n = 200
			// 2016-05-18
			//case 12:	n = 8;		break;		// 200	1:3
			case 12:	n = 50;		break;		// 200	1:3
			//case 13:	n = 12;		break;		// 300  1:4
			case 13:	n = 60;		break;		// 300  1:4
			//case 14:	n = 150;	break;		// 800	1:40
			case 14:	n = 220;	break;		// 1500	1:40
			default:	 n = 0;		break;
			}
		}
	return n;
}


/******************************************************************************/

void SetWorkStoreNum(unsigned char num){
	// 设置测试卡仓号	1~5,从小仓开始数
	if(num > 5)
		num = 5;
	if(num == 0)
		num = 1;
	_NewCardStoreNum = num;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3137, _NewCardStoreNum);
	Uart0ReEnable;
}
unsigned char GetWorkStoreNum(void){
	return _NewCardStoreNum;
}
//_CONST unsigned int RatioNumber[] = {0,1,2,5,10,20,50,100,200,500,1000,2000,5000,10000};
_CONST unsigned int RatioNumber[] = {0,1,2,5,10,20,50,100,200,500,1000,2000,3,4,40};
void SetDiluentRatio(unsigned char num){
	// 设置稀释比例
	//if(num>13)
	//	num = 13;
	if(num == 0)
		num = 1;
	if(num > 14)
		num = 14;
	_NewMultipNum = num;
	if(1 != _NewMultipNum)	// 如果稀释比例不为1:1
		_MixtureMode = 0;
	Uart0ReUnable;
	uart_Printf("%s $%4d $%4d\r\n",strM3136, _NewMultipNum, RatioNumber[_NewMultipNum]);
	Uart0ReEnable;
}
void SetReadTime0(unsigned int t){
	// 设置A检测头测试时间
	if(t>900)
		t = 900;
	if(t<20)
		t = 20;
	_NewReadTime0 = t;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n",strM3138, _NewReadTime0);
	Uart0ReEnable;
}
void SetReadTime1(unsigned int t){
	// 设置B检测头测试时间
	_NewReadTime1 = 0;
	return;

	if(t>900)
		t = 900;
	if(t<20)
		t = 20;
	_NewReadTime1 = t;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3139, _NewReadTime1);
	Uart0ReEnable;
}
void SetReadMolule(unsigned char n){
	// 设置读数头
	if(n == 0)
		_NewTestType = 0;
	else
		_NewTestType = 1;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3140, _NewTestType);
	Uart0ReEnable;
}

unsigned long _NewTestSetial(void){
	// 保存序列号
	unsigned char c;
	unsigned int i;
	unsigned long l;

	EEPROM_READ(EEP_ADD_SERIAL, c);		// 读取初始化标识
	if(c != 0xc5){		// 初始化
		c = 0xc5;
		EEPROM_WRITE(EEP_ADD_SERIAL, c);
		c = 0;
		EEPROM_WRITE(EEP_ADD_SERIAL + 1, c);	// 出示化写入高8位数
		i = 1;
		EEPROM_WRITE(EEP_ADD_SERIAL + 2, i);	// 出示化写入低16位数
		return 1;
		}
	EEPROM_READ(EEP_ADD_SERIAL + 1, c);		// 读取高8位
	if(c < 48){
		EEPROM_READ(EEP_ADD_SERIAL+2+c*2, i);
		i++;
		if(i==0){	// 换新的地16位写入地址
			c ++;
			EEPROM_WRITE(EEP_ADD_SERIAL + 1, c);	// 初始化写入高8位数
			}
		EEPROM_WRITE(EEP_ADD_SERIAL+2+c*2, i);
		}
	l = c * 0x10000 + i;
	return l;
}
unsigned long ReadCurTestSetial(void){
	unsigned char c;
	unsigned int i;
	unsigned long l;
	l = 1;
	EEPROM_READ(EEP_ADD_SERIAL, c);		// 读取初始化标识
	if(c != 0xc5){		// 初始化
		c = 0xc5;
		EEPROM_WRITE(EEP_ADD_SERIAL, c);
		c = 0;
		EEPROM_WRITE(EEP_ADD_SERIAL + 1, c);	// 出示化写入高8位数
		i = 1;
		EEPROM_WRITE(EEP_ADD_SERIAL + 2, i);	// 出示化写入低16位数
		return 1;
		}
	EEPROM_READ(EEP_ADD_SERIAL + 1, c);		// 读取高8位
	if(c < 48){
		EEPROM_READ(EEP_ADD_SERIAL+2+c*2, i);
		l = c * 0x10000 + i;
		}
	return l;
}
void _SetCurWorkParamter(void){
	NewTestInfo.cardStoreNum = _NewCardStoreNum;
	NewTestInfo.sampDiluteMult = _NewMultipNum;
	NewTestInfo.testTime0 = _NewReadTime0;
	NewTestInfo.testTime1 = _NewReadTime1;
	NewTestInfo.readType = _NewTestType;
	NewTestInfo.testSerial = _NewTestSetial();
//	NewTestInfo.testSerial ++;	// 测试自动编号
	// 输出新测试信息
#ifndef UartSendLong
	Uart0ReUnable;
	uart_Printf("%s $%8d ",strM3103,NewTestInfo.testSerial);
#else
	Uart0ReUnable;
	uart_Printf("%s $ ",strM3103);
	uart0SendInt(NewTestInfo.testSerial);
#endif
	uart_Printf("$%4d $%4d $%4d $%4d $%4d\r\n", 
			NewTestInfo.sampDiluteMult, 
			NewTestInfo.cardStoreNum, 
			NewTestInfo.readType, 
			NewTestInfo.testTime0, 
			NewTestInfo.testTime1);
	Uart0ReEnable; 
}
/*
void SetSleepTime(unsigned int t){
	unsigned int i;
	// 设置休眠时间
	if(t>4095)
		t = 4095;
	_SleepTime = t;
	i = _SleepTime  + 0xc000;	// 0xc000 初始化标记
	EEPROM_WRITE(EEP_ADD_SLEEP_TIME,  i);
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3152, _SleepTime);
	Uart0ReEnable;
}
*/

void TestSleep(void)
{
	// 进入休眠
	_SleepTime = 1;
}
void TestStartup(void){
	// 测试启动
	_WaitStartKey = 0;
}
void _SetNewCardGet(unsigned char num)
{
	if(GetNewTestCard != 254 && GetNewTestCard != 222)	// 如果卡片已经取出，将不再取卡片
	{
		GetNewTestCard = num;	// 设置本次取卡片仓号
	}
}

// 滴样量调节因子
//signed char SetDropVolumeFactor(signed char n)
void SetDropVolumeFactor(signed int n)
{
	if(n <= 500 && n >= -500)
	{	// 参数范围判断
		EEPROM_WRITE(EEP_DROP_VOLUME_FACTOR, n);
	}
	else
	{
		EEPROM_READ(EEP_DROP_VOLUME_FACTOR, _DropVolumeFactor);		// 重新读取写入的设置值
		Uart0ReUnable;
		uart_Printf("%s $%4d\r\n", strM3218, _DropVolumeFactor);
		Uart0ReEnable;
		return;
	}
	EEPROM_READ(EEP_DROP_VOLUME_FACTOR, _DropVolumeFactor);		// 重新读取写入的设置值
	if(_DropVolumeFactor > 500 || _DropVolumeFactor < -500)
	{		// 保存的值有异常
		_DropVolumeFactor = 0;
		EEPROM_WRITE(EEP_DROP_VOLUME_FACTOR, _DropVolumeFactor);
	}
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3218, _DropVolumeFactor);
	Uart0ReEnable;
}

signed int GetDropVolumeFactor(void)
{
	signed int sc;
	EEPROM_READ(EEP_DROP_VOLUME_FACTOR, sc);		// 重新读取写入的设置值
	if(sc > 500 || sc < -500)
	{		// 保存的值有异常
		sc = 0;
		EEPROM_WRITE(EEP_DROP_VOLUME_FACTOR, sc);
	}
	return sc;
}

void SetDropVolume(unsigned int vol)
{
	if(vol > 110)
		vol = 110;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3141, vol);
	Uart0ReEnable;
	_DropVolume = (vol * 425) / 10;	// 每uL42.5步，0.023562uL/步
	// 2016-06-17 增加 if(_DropVolume != 0)
	if(_DropVolume != 0)
		_DropVolume += 149; 	// 3.5uL
}


//********************************************************
//add 2016.7.6
void SetSamplingVolume(unsigned int vol)
{
	if(vol > 110)
		vol = 110;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3121, vol);
	Uart0ReEnable;
	_SamplingVolume = (vol * 425) / 10;	// 每uL42.5步，0.023562uL/步
}
//********************************************************

//********************************************************
//add 2016.4.20
void SetDropMode(unsigned int mode)
{
	if(mode > 1)
		mode = 1;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3142, mode);
	Uart0ReEnable;
	_DropMode = mode;
	//2016-06-15  是否需要同时设置滴样量
	if(_DropMode)
	{
		;
	}
}

// 设置_DropMode 模式下,设置1:1是否需要抽打混匀
void SetMixtureMode(unsigned int mode)
{
	if(0 == _DropMode)		return;			// 非_DropMode模式下,无效
	if(mode > 1)
		mode = 1;
	Uart0ReUnable;
	uart_Printf("%s $%4d\r\n", strM3166, mode);
	Uart0ReEnable;
	_MixtureMode = mode;
}

void SetReMixNum(unsigned int MixNum)
{
  	if(_DropMode == 0) 
  	{
		Uart0ReUnable;
		uart_Printf("%s $%4d\r\n", strM3142, _DropMode);
    	uart_Printf("%s\r\n",strM3144);
		Uart0ReEnable;
    	return;
  	}
  	if(MixNum < 5)
    	MixNum = 5;
  	else if(MixNum > 20)
    	MixNum = 20;
  	_ReMixNum = MixNum;
	Uart0ReUnable;
  	uart_Printf("%s $%4d\r\n", strM3143, _ReMixNum);
	Uart0ReEnable;
}
//********************************************************

// 定标数据

/*
unsigned int  CalculateCalStandCoeff(unsigned int n)
{
	unsigned long l1,l2;
	unsigned int i;
	if(n == 0)
		return _DiluentCalChart.calStand;	// 返回当前设置值

	if(n>_DILUENT_MIX_BASE_COEFF_UP)
		n = _DILUENT_MIX_BASE_COEFF_UP;
	if(n<_DILUENT_MIX_BASE_COEFF_DOWN)
		n = _DILUENT_MIX_BASE_COEFF_DOWN;
	_DiluentCalChart.calStand = n;
	//Save_DiluentCalChart();
	return _DiluentCalChart.calStand;		// 返回设置后的校准因子
}
*/

// 将吸样量调整为不同稀释倍数下
// m 为稀释倍数   n 为调节因子
unsigned int  CalculateCalStandCoeff(unsigned int m,unsigned int n)
{
//#ifndef Puncture
	if(m > 13)	m = 13;
	if(m == 0)	m = 9;
	if(n == 0)
		return _DiluentCalChart.calStand[m];	// 返回当前设置值
	if(n > _DILUENT_MIX_BASE_COEFF_UP)
		n = _DILUENT_MIX_BASE_COEFF_UP;
	if(n < _DILUENT_MIX_BASE_COEFF_DOWN)
		n = _DILUENT_MIX_BASE_COEFF_DOWN;
	_DiluentCalChart.calStand[m] = n;
	return _DiluentCalChart.calStand[m];	// 返回当前设置值
/*
#else
{
	if(WithoutPuncture != 0)		// 无需穿刺
	{
		if(m > 13)	m = 13;
		if(m == 0)	m = 9;
		if(n == 0)
			return _DiluentCalChart.calStand[m];	// 返回当前设置值
		if(n > _DILUENT_MIX_BASE_COEFF_UP)
		n = _DILUENT_MIX_BASE_COEFF_UP;
		if(n < _DILUENT_MIX_BASE_COEFF_DOWN)
		n = _DILUENT_MIX_BASE_COEFF_DOWN;
		_DiluentCalChart.calStand[m] = n;
		return _DiluentCalChart.calStand[m];	// 返回当前设置值
	}
	else
	{
		if(n == 0)
			return _DiluentCalChart.calStand[8];	// 返回当前设置值
		if(n > _DILUENT_MIX_BASE_COEFF_UP)
		n = _DILUENT_MIX_BASE_COEFF_UP;
		if(n < _DILUENT_MIX_BASE_COEFF_DOWN)
		n = _DILUENT_MIX_BASE_COEFF_DOWN;
		_DiluentCalChart.calStand[8] = n;
		return _DiluentCalChart.calStand[8];	// 返回当前设置值
	}
}
#endif
*/
}

// 此部分函数暂未使用
/*
unsigned int CalMixingHeight(unsigned char multipNum, unsigned char diluteTime)
{
	// 计算混匀液高度
	unsigned int n;
	if(diluteTime == 0)
	{
		switch(multipNum)		// 第一次稀释
		{
			// 一次稀释
			case 1:		n = 1740;			break;		// 0 1:1
			case 2:		n = 1740-56;		break;		// 240uL-130uL=110uL 1:2
			case 3:		n = 1740-163;		break;		// 450uL-130uL=320uL 1:5
			case 4:		n = 1740-239;		break;		// 600uL-130uL=470uL 1:10
			case 5:		n = 1740-239;		break;		// 600uL-130uL=470uL 19 1:20
			case 6:		n = 1740-448;		break;		// 1010uL-130uL=880uL 33 1:50
			case 7:		n = 1740-443;		break;		// 1000uL-130uL=870uL 33 1:100
			case 8:		n = 1740-442;		break;		// 995uL-130uL=865uL 33 1:200
			// 二次稀释的第一次稀释
			case 9:										// 1:500
			case 10:									// 1:1000
			case 11:									// 1:2000
			case 12:									// 1:5000	
			case 13:	n = 1740-443;		break;		// 33 1:10000
			default:	n = 1740;			break;
			}
		}
	else
	{			// 二次稀释
		switch(multipNum)
		{
			// 二次稀释中的第二次稀释
			case 9:		n = 1740-201;		break;		// 525uL-130uL=395uL 14 1:500
			case 10:	n = 1740-443;		break;		// 1000uL-130uL=870uL 30 1:1000
			case 11:	n = 1740-433;		break;		// 979uL-130uL=849uL 31 1:2000
			case 12:	n = 1740-417;		break;		// 949uL-130uL=819uL 31 1:5000
			case 13:	n = 1740-443;		break;		// 1000uL-130uL=870uL 33 1:10000
			default:	n = 1740;			break;
		}
	}
	return n;
}
*/

/********************************  液路自检处理程序  ***************************************/

unsigned char DiluteStartCheck(INFO_EVENT * pInfoEvent)
{
//	static unsigned int liqDetBaseAdc;
//	static unsigned char waitMotSampTurn,waitMotSampNeedle, waitMotSampPump,waitMotFluid,waitMotDiluent,waitEffluent;
	static unsigned char mainStep,workStep, subStep;		
	static unsigned char pos;
	static unsigned char detRetry ;		// 液体探测重试次数
	static unsigned char CalCnt;		// 重复校准计数
	static unsigned int CalValue1, CalValue2;
	static unsigned char cNum;

		
	static unsigned int mixLiqLevel;
	static unsigned int mixCalStartPos;
	unsigned char ucTmp;
	signed int siTmp, siTmp1;
	unsigned int i, n, m;

	if(pInfoEvent == 0)
	{
		if(WaitDelayTime(MOT_SAMP_PUMP))		return 0;
		if(WaitDelayTime(MOT_SAMP_TRUN))		return 0;
		if(WaitDelayTime(MOT_SAMP_NEEDLE))		return 0;
		if(WaitDelayTime(MOT_EFFLUENT))			return 0;
		if(WaitDelayTime(MOT_DILUENT))			return 0;
		
		if(waitMotSampTurn)		{	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
		if(waitMotSampNeedle)	{	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
		if(waitMotSampPump)		{	if(GetMotState(MOT_SAMP_PUMP)!=STA_SLAVE_FREE)		return 0;	waitMotSampPump = 0;	}
		if(waitMotFluid)		{	if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)			return 0;	waitMotFluid = 0;	}
		if(waitMotDiluent)		{	if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE)		return 0;	waitMotDiluent = 0;	}
		if(waitEffluent)		{	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;	}
	}
	else
		return 0;

	switch(mainStep)
	{
		case 0:		// 取样臂位置初始化
			switch(workStep)
			{
				case 0:		// 	取样针回到起始位
					ReadLiquidMonitorResult(0);
					ReadLiquidMonitorResult(1);
					ReadLiquidMonitorResult(2);
					ReadLiquidMonitorResult(3);
					SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
					SetMotRunPam(MOT_SAMP_TRUN,200,10,CURRENT_SAMP_TRUN);
					MotInitCheck(MOT_SAMP_NEEDLE);
					_EffluentMotRun(20, 120);
					waitMotSampNeedle = 1;
					workStep = 1;
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2100);
					Uart0ReEnable;
					break;
				case 1:		// 旋转臂回到起始点
					MotInitCheck(MOT_SAMP_TRUN);
					waitMotSampTurn = 1;
					// 读取取样臂位置
					NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
					NeedleOnMixSidePos = GetNeedleOnMixSidePos();
					workStep = 2;
					break;
				case 2:		// 取样臂运行到混匀池正上方
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);	
					waitMotSampTurn = 1;
					waitEffluent = 1;
					detRetry = 0;
					workStep = 3;
					break;
				case 3:		// 预排清洗头
					detRetry ++;
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(20, 120);
					_NeedleMotRunTo(_POS_MIX_TOP, 200);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					if( getLiqDetADC(NeedleChannel) < 500)	// 检查液体探测
						workStep = 110;
					else
						workStep = 4;
					break;
				case 110:	// 取样针空吸,将液路与针隔离
					SetEValve(EV2, EV_OPEN);
					_FluidMotRun(-1, 20);
					waitMotFluid = 1;
					workStep = 111;
					break;
				case 111:	// 关闭吸空
					SetDelayTime(MOT_FLUID, 20);
					SetEValve(EV2, EV_CLOSE);
					workStep = 112;
					break;
				case 112:	// 再次检查液体探测
					if( getLiqDetADC(NeedleChannel) < 500)	// 检查液体探测
						workStep = 113;
					else
						workStep = 4;
					break;
				case 113:	// 清洗头液路吸空, 将液路与针隔离
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(-1, 20);
					waitMotFluid = 1;
					workStep = 114;
					break;
				case 114:
					SetDelayTime(MOT_FLUID, 20);
					SetEValve(EV1, EV_CLOSE);
				//	if(detRetry < 4)
				//		workStep = 3;
				//	else
						workStep = 4;
					break;
				case 4:		// 液体探测初始化
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	
						workStep = 0;
						Uart0ReUnable;	
						uart_Printf("!2501\r\n");		// 开始液路自检，在进行液面探测功能检验时发生错误
						Uart0ReEnable;
						break;// 退出处理
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc1 $%4d\r\n",liqDetBaseAdc);	
					workStep = 5;
					break;
				case 5:		// 取样针运行到混匀池中,探测池中是否有液体, 分两段进行
					SetEValve(EV_ALL, EV_CLOSE);
					SetMotRunPam(MOT_SAMP_NEEDLE, 100, 10, CURRENT_SAMP_NEEDLE);
					MotRunTo(MOT_SAMP_NEEDLE, (_POS_MIX_TOP+_POS_MIX_BUTTOM)/2);
					detRetry = 0;
					workStep = 6;
					break;
				case 6:		// 液面探测，如果有探测到液体，则报警排液异常
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 200)
						{
							if(detRetry < 15)	// 重测计数
							{
								detRetry ++;	
								break;	
							}
							// 混匀池有残留液，排液异常
							MotStop(MOT_SAMP_NEEDLE);
							SetBeepWarning();
							mainStep = 8;
							workStep = 0;
							SetDelayTime(MOT_SAMP_NEEDLE, 20);
							Uart0ReUnable;
							uart_Printf("!2502\r\n");	// 开始液路自检， 在检查混匀池时， 发现混匀池中有残留水， 请检查废液泵以及排液通道
							Uart0ReEnable;
							break;
						}
						else
							detRetry = 0;
					}
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						// 取样针下降停止，未探测到残留液体,正常
						workStep = 7;
					}
					break;
				case 7:		// 液体探测初始化
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	
						workStep = 0;	
						break;// 退出处理
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc2 $%4d\r\n",liqDetBaseAdc);	
					SetDelayTime(MOT_SAMP_NEEDLE, 20);
					workStep = 8;
					break;
				case 8:		// 取样针运行到混匀池中,探测池中是否有液体, 第二`段
					SetEValve(EV_ALL, EV_CLOSE);
					SetMotRunPam(MOT_SAMP_NEEDLE, 60, 20, CURRENT_SAMP_NEEDLE);
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_BUTTOM);
					detRetry = 0;
					workStep = 9;
					break;
				case 9:		// 液面探测，如果有探测到液体，则报警排液异常
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 150)
						{
							if(detRetry < 15)	// 重测计数
							{
								detRetry ++;	
								break;	
							}
							// 混匀池有残留液，排液异常
							MotStop(MOT_SAMP_NEEDLE);
							SetBeepWarning();
							mainStep = 8;
							workStep = 0;
							SetDelayTime(MOT_SAMP_NEEDLE, 5);
							Uart0ReUnable;
							uart_Printf("!2502\r\n");	// 开始液路自检， 在检查混匀池时， 发现混匀池中有残留水， 请检查废液泵以及排液通道
							Uart0ReEnable;
							break;
							}
						else
							detRetry = 0;
					}
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						// 取样针下降停止，未探测到残留液体,正常
						workStep = 0;
						mainStep = 1;
					}
					break;
				}
			break;
		case 1:		// 灌注清洗液
			SetDelayTime(MOT_EFFLUENT, 2);
			ucTmp = _PrimingFluid();
			if(ucTmp == 1)
			{
				workStep = 0;
				mainStep = 100;
			}
			else if(ucTmp == 0xff)
			{
				Uart0ReUnable;
				uart_Printf("%s\r\n",strE3902);	
				Uart0ReEnable;
				mainStep = 8;
				workStep = 0;
			}
			break;
		case 100:	// 清洗清洗头和取样针通道
			switch(workStep)
			{
				case 0:	// 建立负压
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(100, 200);
					SetDelayTime(MOT_EFFLUENT, 5);
					workStep = 1;
				case 1:	// 清洗
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(30, 30);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetDelayTime(MOT_EFFLUENT, 20);
					waitMotSampNeedle = 1;
					workStep = 2;
					break;
				case 2:	// 先关闭清洗液
					MotStop(MOT_FLUID);
					_FluidMotRun(-1, 80);
					SetDelayTime(MOT_EFFLUENT, 5);
					workStep = 3;
					break;
				case 3:	// 关闭负压
					SetEValve(EV_ALL, EV_CLOSE);
					MotStop(MOT_EFFLUENT);
					_NeedleMotRunTo(_POS_MIX_TOP, 240);
					waitMotSampNeedle = 1;
					workStep = 4;
					break;
				case 4:	// 清洗和灌注取样针通道
					SetEValve(EV2, EV_OPEN);
					SetMotRunPam(MOT_SAMP_PUMP,64,60,CURRENT_SAMP_PUMP);
					MotInitCheck(MOT_SAMP_PUMP);
					_FluidMotRun(10, 64);
					waitMotFluid = 1;
					workStep = 5;
					break;
				case 5:	// 清洗完毕，抽清洗头残留水
					SetEValve(EV_ALL, EV_CLOSE);
					SetEValve(EV3, EV_OPEN);
					_NeedleMotRunTo(0, 240);
					_EffluentMotRun(20, 200);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					workStep = 6;
					break;
				case 6:	// 排干混匀池水
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(20, 220);
					waitEffluent = 1;
					workStep = 7;
					break;
				case 7:	// 吸样针回抽液体，形成隔离段
					SetEValve(EV2, EV_OPEN);
					SetDelayTime(MOT_DILUENT, 2);
					workStep = 8;
					break;
				case 8:
					_FluidMotRun(-2, 64);
					waitMotFluid = 1;
					workStep = 9;
					break;
				case 9:
					SetEValve(EV2, EV_CLOSE);
					workStep = 0;
					mainStep = 2;
					break;
				}
			break;
		case 2:		// 灌注稀释液
			SetDelayTime(MOT_EFFLUENT, 2);
			ucTmp = _PrimingDiluent();
			if(ucTmp == 1)
			{
				workStep = 0;
				mainStep = 3;
			}
			else if(ucTmp == 0xff)
			{
				Uart0ReUnable;
				uart_Printf("%s\r\n", strE3904);
				Uart0ReEnable;
				mainStep = 8;
				workStep = 0;
			}
			break;
		// 清洗液泵和废液泵流量定标
		case 3:	
			switch(workStep)
			{
				case 0:
					SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
					MotInitCheck(MOT_SAMP_NEEDLE);
					waitMotSampNeedle = 1;
					workStep = 100;
					break;
				case 100:
					SetMotRunPam(MOT_SAMP_TRUN,200,5,CURRENT_SAMP_TRUN);
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn = 1;
					workStep = 101;
					break;
				case 101:
					MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
					waitMotSampTurn = 1;
					workStep = 102;
					break;
				case 102:		// 取样针运行到定标液起始高位
					_NeedleMotRunTo(_POS_MIX_CAL_START, 240);
					waitMotSampNeedle = 1;
					//workStep = 103;
					workStep = 1;
					break;
				/*
				case 103:
					if(CheckLiqDetBase())
					{
						SetEValve(EV1, EV_OPEN);
						_FluidMotRun(-1, 30);
						waitMotFluid = 1;
						workStep = 104;
					}
					else
						workStep = 1;
					break;
				case 104:
					SetEValve(EV1, EV_CLOSE);
					if(CheckLiqDetBase())
					{
						SetEValve(EV2, EV_OPEN);
						_FluidMotRun(-1, 30);
						waitMotFluid = 1;
					}
					workStep = 1;
					break;
				*/
				case 1:		// 初始化液面探测
					SetEValve(EV_ALL, EV_CLOSE);
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	
						workStep = 0;	
						Uart0ReUnable;
						uart_Printf("!2510\r\n");	// 开始准备清洗液泵流量检查时，液面探测功能自检出现错误
						Uart0ReEnable;
						break;// 退出处理
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc3 $%4d\r\n",liqDetBaseAdc);	
					_SampPumpMotRun(100, 64);		// 注射泵吸入，防止针头挂水滴
					workStep = 2;
					break;
				case 2:		// 注入最大800mL清洗液到定标液预定高度	
					_FluidMotRun(9, 32);
					detRetry = 0;
					workStep = 3;
					break;
				case 3:		// 等待探测到液面
					if(GetLiquidMonitorState(1) == INFO_LIQ_EMPTY)
					{
						// 清洗液供应异常
						SetBeepWarning();
						// 退出处理
						mainStep = 8;
						workStep = 0;
						MotStop(MOT_FLUID);
						MotInitCheck(MOT_SAMP_PUMP);
						SetDelayTime(MOT_SAMP_NEEDLE, 5);
						Uart0ReUnable;
						uart_Printf("!2511\r\n");	// 清洗液泵流量检查时，出现清洗液供应中断，请更换清洗液后重试
						Uart0ReEnable;
						break;
					}
					if(GetMotState(MOT_FLUID) == STA_SLAVE_FREE)
					{
						// 检查清洗液泵是否在运行，如果停止表示未注入液体或注入液体偏少
						SetBeepWarning();
						// 退出处理
						mainStep = 8;
						workStep = 0;
						MotInitCheck(MOT_SAMP_PUMP);
						SetDelayTime(MOT_SAMP_NEEDLE, 5);
						Uart0ReUnable;
						uart_Printf("!2512\r\n");	// 清洗液泵流量检查时，未检测到清洗液注入，请检查清洗液泵后重新运行液路自检程序
						Uart0ReEnable;
						break;
						}
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 30)
						{
							if(detRetry < 5)	// 重测计数
							{
								detRetry ++;	
								break;	
							}
							MotStop(MOT_FLUID);
							MotInitCheck(MOT_SAMP_PUMP);
							workStep = 4;
							SetBeepAck();
						}
						else
							detRetry = 0;
					}
					break;
				case 4:		// 取样针运行到高位
					_NeedleMotRunTo(_POS_MIX_TOP, 240);
					waitMotSampNeedle = 1;
					workStep = 5;
					break;
				case 5:		// 注入1.2mL定标液
					_FluidMotRun(12, 64);
					workStep = 6;
					break;
				case 6:		// 监测清洗液供应
					if(GetLiquidMonitorState(1) == INFO_LIQ_EMPTY)
					{
						// 清洗液供应异常
						SetBeepWarning();
						// 退出处理
						mainStep = 8;
						workStep = 0;
						MotStop(MOT_FLUID);
						MotInitCheck(MOT_SAMP_PUMP);
						SetDelayTime(MOT_SAMP_NEEDLE, 5);
						Uart0ReUnable;
						uart_Printf("!2511\r\n");	// 清洗液泵流量检查时，出现清洗液供应中断，请更换清洗液后重试
						Uart0ReEnable;
						break;
						}
					if(GetMotState(MOT_FLUID) == STA_SLAVE_FREE)
					{
						workStep = 7;	// 注液完毕
					}
					break;
				case 7:			// 初始化液面探测
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	
						workStep = 0;	
						Uart0ReUnable;
						uart_Printf("!2513\r\n");	// 清洗液泵流量检查时，液面探测功能自检出现错误
						Uart0ReEnable;
						break;// 退出处理
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc4 $%4d\r\n",liqDetBaseAdc);	
					_SampPumpMotRun(100, 64);
					_NeedleMotRunTo(_POS_MIX_CAL_START-200, 16);
					detRetry = 0;
					workStep = 8;
					break;
				case 8:		// 取样针下降探测液面
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						// 检查 取样针是否在运行，如果停止表示液面高度太低
						SetBeepWarning();
						// 退出处理
						mainStep = 8;
						workStep = 0;
						MotInitCheck(MOT_SAMP_PUMP);
					//	uart_Printf("!2514\r\n");	// 清洗液泵流量检查时在检测流量时，未检测到液体注入，请检查稀释液泵后重新运行液路自检程序
						Uart0ReUnable;
						uart_Printf("!2514 $%4d $%4d\r\n",liqDetBaseAdc,i);  // 2016-08-24 更改
						Uart0ReEnable;
						break;
					}
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 90)
						{
							if(detRetry < 10)	// 重测计数
							{
								detRetry ++;	
								break;	
							}
							MotStop(MOT_SAMP_NEEDLE);
							MotInitCheck(MOT_SAMP_PUMP);
							SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
							workStep = 9;
							SetDelayTime(MOT_SAMP_NEEDLE, 5);
							SetBeepAck();
						}
						else
							detRetry = 0;
					}
					break;
				case 9:		// 计算定标结果
					MotInitCheck(MOT_SAMP_PUMP);
					waitMotSampPump = 1;
					i = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);
					mixLiqLevel = i;
					i = _POS_MIX_CAL_START - mixLiqLevel;
					Uart0ReUnable;
					uart_Printf("%s $%d\r\n",strM2111, i);
					Uart0ReEnable;
					SetDelayTime(MOT_SAMP_NEEDLE, 10);
					workStep = 10;
					break;
				// 开始废液定标
				case 10:
					_EffluentMotRun(10, 140);		// 1000/125 = 8
					waitEffluent = 1;
					workStep = 11;
					break;
				case 11:			// 初始化液面探测
				/*	if(CheckLiqDetBase())
					{
						mainStep = 8;	workStep = 0;	
						Uart0ReUnable;
						uart_Printf("!2520\r\n");	// 开始废液泵流量检查时，液面探测功能自检出现错误
						Uart0ReEnable;
						break;// 退出处理
					}
				*/
					liqDetBaseAdc = getLiqDetADC(NeedleChannel);
					//uart_Printf("//LiqDetBaseAdc5 $%4d\r\n",liqDetBaseAdc);	
					_SampPumpMotRun(100, 64);		// 注射泵吸入，防止针头挂水滴
					_NeedleMotRunTo(_POS_MIX_CAL_START, 16);
					detRetry = 0;
					workStep = 12;
					break;
				case 12:	// 液面探测
					// 检查 取样针是否在运行，如果停止表示液面高度太低
					if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
					{
						SetBeepWarning();
						// 退出处理
					//	mainStep = 8;	
					//	workStep = 0;
					//	MotInitCheck(MOT_SAMP_PUMP);
					//	uart_Printf("!2521\r\n");	// 在废液泵流量检测时，取样针未探测到液体，请检查液面探测功能后重新运行液路自检程序
						workStep = 13;
						break;
					}
					i = getLiqDetADC(NeedleChannel);
					if(i < liqDetBaseAdc)
					{
						i = liqDetBaseAdc - i;
						if(i > 50)
						{
							if(detRetry < 10)	// 重测计数
							{
								detRetry ++;	
								break;	
							}
							MotStop(MOT_SAMP_NEEDLE);
							SetDelayTime(MOT_SAMP_NEEDLE, 5);
							SetBeepAck();
							workStep = 13;
						}
						else
							detRetry = 0;
					}
					break;
				case 13:	// 计算废液泵流量
					MotInitCheck(MOT_SAMP_PUMP);	// 注射泵复位
					i = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);
					i = i-mixLiqLevel;
					Uart0ReUnable;
					uart_Printf("%s $%d\r\n",strM2112, i);
					Uart0ReEnable;
					SetDelayTime(MOT_SAMP_NEEDLE, 10);
					workStep = 14;
					break;
				case 14:
					SetEValve(EV_ALL,EV_CLOSE);
					_NeedleMotRunTo(0, 240);
					workStep = 15;
					break;
				case 15:
					_EffluentMotRun(20, 200);
					workStep = 0;
					mainStep = 4;
					subStep = 0;
					waitEffluent = 1;
					waitMotSampNeedle = 1;
					CalCnt = 5;
					CalValue1 = 0;
					CalValue2 = 0;
					InitFlowMeter(); // 初始化流量定标判断
					break;
				}
			break;
		case 4:		//  稀释液蠕动泵流量定标
			switch(workStep)
			{
				case 0:		// 准备
					switch(subStep)
					{
						case 0:
							SetMotRunPam(MOT_SAMP_NEEDLE, 220, 20, CURRENT_SAMP_NEEDLE);
							MotInitCheck(MOT_SAMP_NEEDLE);
							waitMotSampNeedle = 1;
							subStep = 1;
							break;
						case 1:
							SetMotRunPam(MOT_SAMP_TRUN,200,10,CURRENT_SAMP_TRUN);
							MotRunTo(MOT_SAMP_TRUN, 0);
							waitMotSampTurn = 1;
							subStep = 2;
							break;
						case 2:
							MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
							waitMotSampTurn = 1;
							subStep = 3;
							break;
						case 3:		// 取样针运行到低位,同时取样针吸干清洗头
							_FluidMotRun(-1, 70);
							SetEValve(EV3,EV_OPEN);
							_EffluentMotRun(100, 200);
							_NeedleMotRunTo(_POS_MIX_CAL_START - 100 - 100, 220); // 2017-05-26 -100 -> -100 - 100
							waitMotSampNeedle = 1;
							subStep = 4;
							break;
						case 4:
							MotStop(MOT_EFFLUENT);
							SetEValve(0, 0);
							subStep = 5;
							SetDelayTime(MOT_EFFLUENT, 5);
							break;
						case 5:
						/*	if(CheckLiqDetBase())
							{
								mainStep = 8;	
								workStep = 0;	// 退出处理
								Uart0ReUnable;
								uart_Printf("!2530\r\n");	// 开始准备稀释液流量定标时，液面探测功能自检出现错误
								Uart0ReEnable;
							}
							else
						*/
								subStep = 6;
							break;
						case 6:		// 注入第一段液体
							_SampPumpMotRun(100, 64);
							liqDetBaseAdc = getLiqDetADC(NeedleChannel);
							//uart_Printf("//LiqDetBaseAdc6 $%4d\r\n",liqDetBaseAdc);	
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(12+5, 16);
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(4, 64);	
#endif
							waitMotDiluent = 1;
						//	workStep = 2;
							subStep = 7;
							break;
						case 7:		// 稀释液回抽
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(-5, 64);	
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(-1, 64);	
#endif
							waitMotDiluent = 1;
							workStep = 2;
							subStep = 0;
							break;
						}
					break;
				case 2:		// 探测液面高度
					switch(subStep)
					{
						case 0:		// 检查液面探测
							i = getLiqDetADC(NeedleChannel);
							if(i < liqDetBaseAdc)
							{
								i = liqDetBaseAdc - i;
								if(i > 30)		// 液面高度异常
								{
									mainStep = 8;	
									workStep = 0;	// 退出处理
									Uart0ReUnable;
									uart_Printf("!2531\r\n");	// 在稀释液流量定标起始液面高度测量时，检测出液面高度超高，请检重新调整取样针高度后重新运行液路自检程序
									Uart0ReEnable;
									break;
								}
							}
							subStep = 1;
							break;
						case 1:
							liqDetBaseAdc = getLiqDetADC(NeedleChannel);
							//uart_Printf("//LiqDetBaseAdc7 $%4d\r\n",liqDetBaseAdc);	
							_NeedleMotRunTo(_POS_MIX_CAL_START + 50, 8);   // 2017-05-26 + 100 -> +50
							mixCalStartPos = 0;
							detRetry = 0;
							subStep = 2;
							break;
						case 2:	// 等待取样针接触到液面
							// 检查取样针是否在运行，如果停止表示在指定行程内未探测到液面
							if(GetMotState(MOT_SAMP_NEEDLE) == STA_SLAVE_FREE)
							{
								SetBeepWarning();
								// 退出处理
								mainStep = 8;
								workStep = 0;
								MotInitCheck(MOT_SAMP_PUMP);
								Uart0ReUnable;
								uart_Printf("!2532\r\n");	// 在稀释液流量定标起始液面高度测量时，未探测到液面，请检查稀释液泵后重新运行液路自检程序
								Uart0ReEnable;
								break;
							}
							i = getLiqDetADC(NeedleChannel);
							if(i < liqDetBaseAdc)
							{
								i = liqDetBaseAdc - i;
								if(i > 30)
								{
									if(detRetry < 5)	// 重测计数
									{
										detRetry ++;	
										break;	
									}
									MotStop(MOT_SAMP_NEEDLE);
									subStep = 3;
									SetDelayTime(MOT_SAMP_NEEDLE, 10);
									SetBeepAck();
								}
								else
									detRetry = 0;
							}
							break;
						case 3:		// 记录起始液面高度
							mixCalStartPos = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);
							_NeedleMotRunTo(_POS_MIX_TOP, 200);
							MotInitCheck(MOT_SAMP_PUMP);
							SetDelayTime(MOT_DILUENT, 10);
							subStep = 0;
							workStep = 3;
							waitMotSampNeedle = 1;
							waitMotSampPump = 1;
							break;
						}
					break;
				case 3:		// 注入定标液
					switch(subStep)
					{
						case 0:
					//		_DiluentMotRun(50, 80);		// 注入50个单位溶液
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(40+5, 64);		// 注入40个单位溶液
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(12+2, 64);		// 注入12+2个单位溶液
#endif
							subStep = 1;
							break;
						case 1:
							if(GetLiquidMonitorState(0) == INFO_LIQ_EMPTY)
							{
								// 稀释液供应异常
								SetBeepWarning();
								// 退出处理
								mainStep = 8;
								workStep = 0;
								MotStop(MOT_DILUENT);
								SetDelayTime(MOT_SAMP_NEEDLE, 5);
								Uart0ReUnable;
								uart_Printf("!2533\r\n");	// 在稀释液流量定标时，稀释液供应中断，请检查和更换稀释液后重新运行液路自检程序
								Uart0ReEnable;
								break;
							}
							if(GetMotState(MOT_DILUENT) == STA_SLAVE_FREE)
							{
								subStep = 2;
							}
							break;
						case 2:	
#if 	(DILUTE_TUBE == 14)
							_DiluentMotRun(-5, 64);
#elif	(DILUTE_TUBE == 16)
							_DiluentMotRun(-2, 64);
#endif
							waitMotDiluent = 1;
							subStep = 3;
							break;
						case 3:		// 取样针下降探测液面高度
						/*
							if(CheckLiqDetBase())
							{
								mainStep = 8;	workStep = 0;
								Uart0ReUnable;
								uart_Printf("!2534\r\n");	// 在稀释液流量定标液面高度测量时，液面探测功能自检出现错误
								Uart0ReEnable;
								break;// 退出处理
							}
							*/
							liqDetBaseAdc = getLiqDetADC(NeedleChannel);
							//uart_Printf("//LiqDetBaseAdc8 $%4d\r\n",liqDetBaseAdc);	
							_SampPumpMotRun(100, 64);
							_NeedleMotRunTo(_POS_MIX_CAL_START-225, 8);
							detRetry = 0;
							subStep = 4;
							break;
						case 4:	// 等待取样针接触到液面
							// 检查稀释液泵是否在运行，如果停止表示未注入液体或液面探测失败
							if(GetMotState(MOT_SAMP_NEEDLE)==STA_SLAVE_FREE)
							{
								MotStop(MOT_SAMP_NEEDLE);
								SetBeepWarning();
								// 退出处理
								mainStep = 8;
								workStep = 0;
								MotInitCheck(MOT_SAMP_PUMP);
								Uart0ReUnable;
								uart_Printf("!2535\r\n");	// 在稀释液流量定标液面高度测量时，未探测到液面，请检查稀释液泵后重新运行液路自检程序
								Uart0ReEnable;
								break;
							}
							i = getLiqDetADC(NeedleChannel);
							if(i < liqDetBaseAdc)
							{
								i = liqDetBaseAdc - i;
								if(i>90)
								{
									if(detRetry < 5)	// 重测计数
									{
										detRetry ++;	
										break;	
									}
									MotStop(MOT_SAMP_NEEDLE);
									subStep = 5;
									SetDelayTime(MOT_SAMP_NEEDLE, 10);
									SetBeepAck();
								}
								else
									detRetry = 0;
							}
							break;
						case 5:
						//	MotInitCheck(MOT_SAMP_PUMP);
							MotRunTo(MOT_SAMP_PUMP, 0);
							i = (unsigned int)GetMotPositionOfStep(MOT_SAMP_NEEDLE);		// 记录泵起始值
							i = mixCalStartPos - i;	// 1680
#ifndef Puncture
							i = i*2;
#endif
							Uart0ReUnable;
							uart_Printf("// DiluteFlow:%d\r\n", i);
							Uart0ReEnable;
							m = JudgeFlowMeter(i);
							if(m)
							{
								Uart0ReUnable;
								uart_Printf("// FlowJudge:%d\r\n", m);
								Uart0ReEnable;
								i = InsetrDiluentFlowCalValue(m);
								if(i)
								{
									Uart0ReUnable;
									uart_Printf("%s $%4d\r\n",strM2110, i);
									Uart0ReEnable;
									CalCnt = 0;		
								}
								else
								{	
									Uart0ReUnable;
									uart_Printf("// CalibRetry:\r\n");
									Uart0ReEnable;
								//	CalCnt --;
								}
							}
							else
							{
							//	CalCnt = 2;
							}
							subStep = 0;
							workStep = 4;
							waitMotSampPump = 1;
							break;
						case 6:
							subStep = 0;
							workStep = 4;
							break;
						default:
							break;
						}
					break;
				case 4:
					_NeedleMotRunTo(0, 240);
					_EffluentMotRun(30, 120);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					if(CalCnt != 0)
					{
						CalCnt --;
						workStep = 0;
						mainStep = 4;
						subStep = 0;
					}
					else
						workStep = 5;
					break;
				case 5:
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn= 1;
					workStep = 6;
					break;
				case 6:
					if(WorkProcessStep == 3)
					{

						MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
						waitMotSampTurn= 1;
						cNum = 3;
						workStep = 9;
/*
						MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
						waitMotSampTurn= 1;
#ifndef Puncture
						workStep = 7;
#else
						workStep = 8;
#endif
*/
					}
					else
					{
						workStep = 0;
						mainStep = 7;
						subStep = 0;
					}
					break;
				case 7:
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);
					waitMotSampNeedle = 1;
					workStep = 8;
					break;
				case 8:
					workStep = 0;
					mainStep = 7;
					subStep = 0;
					break;
				case 9:	
					MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP);
					waitMotSampNeedle = 1;
					workStep = 10;
					break;
				case 10:// 清洗和灌注取样针通道
					cNum--;
					SetEValve(EV2, EV_OPEN);
					MotInitCheck(MOT_SAMP_PUMP);
					_FluidMotRun(12, 100);		// 注入1mL清洗液
					waitMotFluid = 1;
					SetDelayTime(MOT_FLUID, 40);
					workStep = 11;
					break;
				case 11:
					_EffluentMotRun(20, 200);
					SetDelayTime(MOT_FLUID, 20);
					waitEffluent = 1;
					workStep = 12;
					break;
				case 12:
					if(cNum != 0)
						workStep = 10;
					else
						workStep = 13;
					break;
				case 13:	// 清洗完毕，抽清洗头残留水
					SetEValve(EV_ALL, EV_CLOSE);
					SetEValve(EV3, EV_OPEN);
					_NeedleMotRunTo(0, 180);//240
					_EffluentMotRun(30, 200);
					waitMotSampNeedle = 1;
					waitEffluent = 1;
					workStep = 14;
					break;
				case 14:	// 排干混匀池水
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(20, 220);
					waitEffluent = 1;
					workStep = 15;
					break;
				case 15:
					MotRunTo(MOT_SAMP_TRUN, 0);
					waitMotSampTurn= 1;
					workStep = 16;
					break;
				case 16:
					MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
					waitMotSampTurn= 1;
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_INTERVAL + _SAMP_PUMP_AIR_ISOLATE);
					waitMotSampPump = 1;
#ifndef Puncture
					workStep = 7;
#else
					workStep = 8;
#endif
					break;
			}
			break;
		case 7:		// 液路自检完成
			SetBeepAck();
			SetEValve(EV_ALL, EV_CLOSE);
			workStep = 0;
			mainStep = 0;
			Uart0ReUnable;
			uart_Printf("%s\r\n",strM2120);
			Uart0ReEnable;
			return 1;
			break;
		case 8:		// 液路自检失败，失败跳转到这里
			switch(workStep)
			{
				case 0:
					SetMotRunPam(MOT_SAMP_NEEDLE, 220, 20, CURRENT_SAMP_NEEDLE);	// 恢复取样针正常运行参数
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetEValve(EV_ALL, EV_CLOSE);
					_EffluentMotRun(30, 200);
					waitMotSampNeedle = 1;
					workStep = 1;
					break;
				case 1:
					MotInitCheck(MOT_SAMP_TRUN);
					waitMotSampTurn= 1;
					waitEffluent = 1;
					if(WorkProcessStep == 3)		// 测试模式下吸样针下来
						workStep = 3;
					else
						workStep = 2;
					break;
				case 2:
					workStep = 0;
					mainStep = 0;
					subStep = 0;
					Uart0ReUnable;
					uart_Printf("!2550\r\n");	// 液路自检失败退出
					Uart0ReEnable;
					return 1;
					break;
				case 3:
					MotRunTo(MOT_SAMP_TRUN, _POS_SAMPTURN_SAMP);
					waitMotSampTurn= 1;
					MotRun(MOT_SAMP_PUMP, _SAMP_PUMP_INTERVAL + _SAMP_PUMP_AIR_ISOLATE);
					waitMotSampPump = 1;
#ifndef Puncture
					workStep = 4;
#else
					workStep = 2;
#endif
					break;
				case 4:
					_NeedleMotRunTo(_POS_SAMP_DOWN, 180);
					waitMotSampNeedle = 1;
					workStep = 2;
					break;
			}
			break;
		default:
			mainStep = 0;
			workStep = 0;
			break;
		}
	return 0;
}


/*******************************************************************************************/


void DiluteProcDatInit(void)
{
	unsigned int i;
	unsigned char *pChar;

	pChar = (unsigned char *)&(NewTestInfo);
	for(i=0; i<sizeof(SAMP_INFO); i++)
	{
		*pChar++ = 0;
	}
	CurInsertRingNum = 0xff;
	GetNewTestCard = 0;
	SecondCount = 0;
	// 初始化测试参数
	_NewCardStoreNum = 1;
	_NewMultipNum = 9;
	_NewReadTime0 = 60;
	_NewReadTime1 = 0;
	_NewTestType = 0;
//	_DropVolume = 0; 
	_DropVolume = 0;//60*42; 
	Read_DiluentCalChart();
	EEPROM_READ(EEP_ADD_SLEEP_TIME,  i);
	if((i & 0xc000) != 0xc000)		// 0xc000 初始化标记
	{
		i = 1800;
	}
	_SleepTime = i;
	EEPROM_WRITE(EEP_ADD_SLEEP_TIME,  i + 0xc000);
	NewTestInfo.testSerial = ReadCurTestSetial();
}

void _FluidMotRun(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_FLUID, vel, 10, CURRENT_FLUID);
	else
		SetMotRunPam(MOT_FLUID, vel, 10, CURRENT_FLUID);
	MotRun(MOT_FLUID, n*100);			// 开启清洗液泵
}

void _EffluentMotRun(signed int n,unsigned char vel)
{
	SetMotRunPam(MOT_EFFLUENT, vel, 10, CURRENT_EFFLUENT);
	MotRun(MOT_EFFLUENT, n*100);			// 开启废液泵
}

void _DiluentMotRun(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_DILUENT, vel, 10, CURRENT_DILUENT);
	else
		SetMotRunPam(MOT_DILUENT, vel, 10, CURRENT_DILUENT);
	MotRun(MOT_DILUENT, n*100);			// 开启稀释液泵
}

void _NewDiluentMotRun(signed int n,unsigned char vel)
{
	SetMotRunPam(MOT_DILUENT, vel, 10, CURRENT_DILUENT);
	MotRun(MOT_DILUENT, n);			
}

void _NeedleMotRun(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_SAMP_NEEDLE, vel, 5, CURRENT_SAMP_NEEDLE);
	else
		SetMotRunPam(MOT_SAMP_NEEDLE, vel, 5, CURRENT_SAMP_NEEDLE);
	MotRun(MOT_SAMP_NEEDLE, n);	
}

void _NeedleMotRunTo(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_SAMP_NEEDLE, vel, 5, CURRENT_SAMP_NEEDLE);
	else
		SetMotRunPam(MOT_SAMP_NEEDLE, vel, 5, CURRENT_SAMP_NEEDLE);
	MotRunTo(MOT_SAMP_NEEDLE, n);	
}

void _SampPumpMotRun(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_SAMP_PUMP, vel, 5, CURRENT_SAMP_PUMP);
	else
		SetMotRunPam(MOT_SAMP_PUMP, vel, 5, CURRENT_SAMP_PUMP);
	MotRun(MOT_SAMP_PUMP, n);	
}
void _SampPumpMotRunTo(signed int n,unsigned char vel)
{
	if(vel>128)
		SetMotRunPam(MOT_SAMP_PUMP, vel, 5, CURRENT_SAMP_PUMP);
	else
		SetMotRunPam(MOT_SAMP_PUMP, vel, 5, CURRENT_SAMP_PUMP);
	MotRunTo(MOT_SAMP_PUMP, n);	
}

void SetAutoTestCycle(unsigned int num)
{
	_AutoTestCycleNum = num;
}

unsigned char SetDiluentQuit(void)
{
	_DiluentQuitFlag = 1;
	return 0;
}

unsigned char _CheckFluidSupply(void)
{
	if(GetLiquidMonitorState(1) == INFO_LIQ_EMPTY)
	{
		// 清洗液空
		SetBeepWarning();
		if(0 == CleanMode)
		{
			Uart0ReUnable;
			uart_Printf("%s\r\n",strE2953);
			Uart0ReEnable;
		}
		return 1;
	}
	/*	
	else if(GetLiquidMonitorState(1) == INFO_LIQ_BUBBLE)
	{
		// 清洗液气泡
		SetBeepWarning();
		uart0SendString(strE2956);	uart_Printf("\r\n");
		return 2;
	}
	*/
	return 0;
}
unsigned char _CheckDiluentSupply(void)
{
	if(GetLiquidMonitorState(0) == INFO_LIQ_EMPTY)
	{
		// 稀释液空
		SetBeepWarning();
		Uart0ReUnable;
		uart_Printf("%s\r\n",strE2952);
		Uart0ReEnable;
		return 1;
	}
	/*
	else if(GetLiquidMonitorState(0) == INFO_LIQ_BUBBLE)
	{
		// 稀释液气泡
		SetBeepWarning();
		uart0SendString(strE2955);	uart_Printf("\r\n");
		return 2;
	}
	*/
	return 0;
}

unsigned char _PrimingDiluent(void)
{
	static unsigned char mainStep = 0;
	static unsigned int i;
	static unsigned char ucTmp;
	static unsigned char detRetry;

	switch(mainStep)
	{
		case 0:		// 取样针高度回零
			ReadLiquidMonitorResult(0);
			ReadLiquidMonitorResult(1);
			ReadLiquidMonitorResult(2);
			ReadLiquidMonitorResult(3);
			SetMotRunPam(MOT_SAMP_NEEDLE, 220, 20, 4);
			MotInitCheck(MOT_SAMP_NEEDLE);
			waitMotSampNeedle = 1;
			// 读取取样臂位置
			NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
			SetEValve(EV_ALL, EV_CLOSE);
			mainStep = 1;
		//	mainStep = 9;	// 关闭液面探测
			runNum = 5;
			break;
		case 1:
			SetMotRunPam(MOT_SAMP_TRUN,240,10,CURRENT_SAMP_TRUN);
			MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
			waitMotSampTurn = 1;
			mainStep = 4;
			break;
		case 4:		// 
			SetMotRunPam(MOT_SAMP_NEEDLE, 100, 20, CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			SetEValve(EV3, EV_OPEN);
			_EffluentMotRun(40, 160);	// 清洗头吸空
			waitMotSampNeedle = 1;
			waitEffluent = 1;
			mainStep = 5;
			break;
		case 5:	// 	取样针运行到混匀池上方
			SetEValve(EV3, EV_CLOSE);
			MotRunTo(MOT_SAMP_NEEDLE, _POS_MIX_TOP + 100);	// 200
			waitMotSampNeedle = 1;
			detRetry = 0;
			mainStep = 6;
			break;
		case 6:	// 取样针管道吸入空气,将液路与取样针隔离
			SetEValve(EV1, EV_CLOSE);
			SetEValve(EV2, EV_OPEN);
			_FluidMotRun(-1, 20);
			waitMotFluid = 1;
			detRetry ++;
			mainStep = 7;
			break;
		case 7:
			SetDelayTime(MOT_FLUID, 30);
			SetEValve(EV2, EV_CLOSE);
			mainStep = 8;
			break;
		case 8:		// 初始化液面探测
			liqDetBaseAdc = getLiqDetADC(NeedleChannel);
			if(liqDetBaseAdc > 1015)
			{
				SetBeepWarning();
				Uart0ReUnable;
				uart_Printf("!2901 $%d\r\n", liqDetBaseAdc);		// 液体探测电极未连接, 请检查液体探测电极连接是否完好
				Uart0ReEnable;
				mainStep = 0;
				return 0xff;
			}
			//else if(liqDetBaseAdc < 400)
			else if(liqDetBaseAdc < 200)
			{
				if(detRetry < 3)
				{
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(-1, 20);
					waitMotFluid = 1;
					mainStep = 6;
					break;
				}
				else
				{
					SetBeepWarning();
					MotInitCheck(MOT_SAMP_NEEDLE);
					Uart0ReUnable;
					uart_Printf("!2911\r\n");		// 在进行稀释液灌注时，液面探测自检出现异常	
					Uart0ReEnable;
					mainStep = 0;
					return 0xff;	
				}
			}
			else
			{
				liqDetBaseAdc = getLiqDetADC(NeedleChannel);
				mainStep = 9;
			}
			break;
		case 9:		// 启动稀释液泵和废液泵
			SetEValve(EV_ALL, EV_CLOSE);
#if		(DILUTE_TUBE == 14)
			_DiluentMotRun(300, 160);		// 最大灌注9mL液体，超出表示灌注失败，提示用户检查供液
			_EffluentMotRun(160, 80);
#elif	(DILUTE_TUBE == 16)
			_DiluentMotRun(100, 120);		// 最大灌注9mL液体，超出表示灌注失败，提示用户检查供液
			_EffluentMotRun(160, 140);
#endif
			SetDelayTime(15, 50);		// 设置灌注延时量，跳过内部存在的量，防止外部管道液体未进入时就判断灌注结果
			detRetry = 0;
			mainStep = 10;
			break;
		case 10:		// 等待液路事件
			SetDelayTime(MOT_EFFLUENT, 3);
			ReadLiquidMonitorResult(0);
			i = getLiqDetADC(NeedleChannel);
			if(i<liqDetBaseAdc)
			{
				i = liqDetBaseAdc - i;
				//if(i>300)	
				if(i > 150)
				{
					if(detRetry < 10)	// 重测计数
					{
						detRetry ++;	
						break;	
						}
					// 探测到排液异常
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepWarning();
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					uart_Printf("!2912\r\n");	// 稀释液灌注过程中检测到混匀池中液体水位超高，请检查废液泵排液状态
					// 退出处理
					mainStep = 21;
					}
				else
					detRetry = 0;
				}
				
			if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE && GetMotState(MOT_EFFLUENT) != STA_SLAVE_FREE)
			{
				if(WaitDelayTime(15)!=0)
				{
					ucTmp = GetLiquidMonitorState(0);	// 读取0号液路
					if(ucTmp==INFO_LIQ_EMPTY || ucTmp==INFO_LIQ_BUBBLE)
					{
						SetDelayTime(15, 50);	// 液路有空气，重新开始计时
					}
				}
				else
				{
					// 灌注完成
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepAck();
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2102);
					Uart0ReEnable;
					mainStep = 20;
				}
			}
			else
			{
				if(runNum!=0)		// 因为稀释液泵流量小，需要最多灌注3遍
				{
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
				//	_EffluentMotRun(5, 220);
					SetDelayTime(MOT_DILUENT, 20);
					mainStep = 9;
					runNum --;
					break;
				}
				// 供液已经停止，灌注失败
				MotStop(MOT_DILUENT);
				MotStop(MOT_EFFLUENT);
				MotInitCheck(MOT_SAMP_NEEDLE);
				SetBeepWarning();
				Uart0ReUnable;
				uart_Printf("%s\r\n",strE3904);// 稀释液灌注失败， 请检查稀释液供给，如果供给正常则请重新调试稀释液传感器后重试灌注
				Uart0ReEnable;
				mainStep = 21;
			}
			break;
		case 20:		// 完成退出
			//SetEValve(EV_ALL, EV_CLOSE);
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			waitMotSampNeedle = 1;
			mainStep = 0;
			return 1;
			break;
		case 21:		// 失败退出
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			waitMotSampNeedle = 1;
			mainStep = 0;
			return 0xff;
			break;
		default:
			mainStep = 0;
			break;
		}
	return 0;
}

unsigned char _PrimingFluid(void)
{
	static unsigned char mainStep = 0;
	static unsigned int i;
	static unsigned char ucTmp;
	static unsigned char detRetry;
//	static unsigned char waitMotSampTurn,waitMotSampNeedle,waitMotFluid,waitEffluent;

//	if(WaitDelayTime(MOT_FLUID))			return 0;
		
	if(waitMotSampTurn)		{	if(GetMotState(MOT_SAMP_TRUN)!=STA_SLAVE_FREE)		return 0;	waitMotSampTurn = 0;	}
	if(waitMotSampNeedle)	{	if(GetMotState(MOT_SAMP_NEEDLE)!=STA_SLAVE_FREE)	return 0;	waitMotSampNeedle = 0;	}
	if(waitMotFluid)		{	if(GetMotState(MOT_FLUID)!=STA_SLAVE_FREE)			return 0;	waitMotFluid = 0;	}
	if(waitEffluent)		{	if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;	}
		
	switch(mainStep)
	{
		case 0:		// 取样针高度回零
			SetMotRunPam(MOT_SAMP_NEEDLE, 240, 20, CURRENT_SAMP_NEEDLE);
			MotInitCheck(MOT_SAMP_NEEDLE);
			// 读取取样臂位置
			NeedleOnMixCenterPos = GetNeedleOnMixCenterPos();
			waitMotSampNeedle = 1;
			SetEValve(EV_ALL, EV_CLOSE);
			mainStep = 1;
		//	mainStep = 8;	// 关闭液面探测
			runNum = 5;
			break;
		case 1:		// 取样针旋转
			SetMotRunPam(MOT_SAMP_TRUN,240,10,CURRENT_SAMP_TRUN);
			MotRunTo(MOT_SAMP_TRUN, NeedleOnMixCenterPos);
			waitMotSampTurn = 1;
			mainStep = 4;
			break;
		case 4:		// 取样针运行到稀释液口上方位置
			ReadLiquidMonitorResult(1);
			ReadLiquidMonitorResult(2);
			_NeedleMotRunTo(_POS_MIX_TOP + 100, 240);
			waitMotSampNeedle = 1;
			mainStep = 5;
			detRetry = 0;
			break;
		case 5:	// 取样针空吸,将液路与针隔离
			SetEValve(EV_ALL, EV_CLOSE);
			SetEValve(EV2, EV_OPEN);
			_FluidMotRun(-1, 20);
			waitMotFluid = 1;
			detRetry ++;
			mainStep = 6;
			break;
		case 6:
			SetDelayTime(MOT_FLUID, 30);
			SetEValve(EV2, EV_CLOSE);
			mainStep = 7;
			break;
		case 7:		// 初始化液面探测
			liqDetBaseAdc = getLiqDetADC(NeedleChannel);
			if(liqDetBaseAdc > 1015)
			{
				SetBeepWarning();
				MotInitCheck(MOT_SAMP_NEEDLE);
				Uart0ReUnable;
				uart_Printf("!2901 $%d\r\n", liqDetBaseAdc);		// 液体探测电极未连接, 请检查液体探测电极连接是否完好
				Uart0ReEnable;
				mainStep = 0;
				return 0xff;
			}
			//else if(liqDetBaseAdc < 400)
			else if(liqDetBaseAdc < 200)
			{
				if(detRetry < 4)
				{
					SetEValve(EV1, EV_OPEN);
					_FluidMotRun(-2, 20);
					SetEValve(EV3, EV_OPEN);
					_EffluentMotRun(40, 160);	// 清洗头吸空
					waitMotFluid = 1;
					waitEffluent = 1;
					mainStep = 5;
					break;
				}
				else
				{
					SetBeepWarning();
					MotInitCheck(MOT_SAMP_NEEDLE);
					Uart0ReUnable;
					uart_Printf("!2906\r\n");		// 在进行稀释液灌注时，液面探测自检出现异常	
					Uart0ReEnable;
					mainStep = 0;
					return 0xff;	
				}
			}
			else
			{
				liqDetBaseAdc = getLiqDetADC(NeedleChannel);
				SetEValve(EV_ALL, EV_CLOSE);
				detRetry = 0;
				mainStep = 8;
			}
			/*
			if(CheckLiqDetBase())
			{
				MotInitCheck(MOT_SAMP_NEEDLE);
				mainStep = 0;
				Uart0ReUnable;
				uart_Printf("!2906\r\n");		// 在进行稀释液灌注时，液面探测自检出现异常
				Uart0ReEnable;
				return 0xff;	
			}
			*/
			break;
		case 8:		// 启动清洗液泵和废液泵
			_FluidMotRun(100, 60);		// 最大灌注30mL液体，超出表示灌注失败，提示用户检查供液
			_EffluentMotRun(105, 60);
			SetDelayTime(15, 60);		// 设置灌注延时量，跳过内部存在的量，防止外部管道液体未进入时就判断灌注结果
			ReadLiquidMonitorResult(1);
			mainStep = 9;
			break;
		case 9:		// 等待液路事件
			SetDelayTime(MOT_EFFLUENT, 3);
			ReadLiquidMonitorResult(1);
			if(GetMotState(MOT_EFFLUENT) != STA_SLAVE_FREE && GetMotState(MOT_FLUID) != STA_SLAVE_FREE) 
			{
				if(WaitDelayTime(15)!=0)
				{
					ucTmp = GetLiquidMonitorState(1);
					if(ucTmp==INFO_LIQ_EMPTY || ucTmp==INFO_LIQ_BUBBLE)
					{
						SetDelayTime(15, 60);	// 液路有空气，重新开始计时
					}
				}
				else
				{
					// 灌注完成
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepAck();
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2101);
					Uart0ReEnable;
					mainStep = 10;
				}
			}
			else
			{
				if(runNum!=0)		// 因为稀释液泵流量小，需要最多灌注3遍
				{
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					SetDelayTime(MOT_DILUENT, 20);
					mainStep = 8;
					runNum --;
					break;
				}
				// 供液已经停止，灌注失败
				MotStop(MOT_FLUID);
				MotStop(MOT_EFFLUENT);
				MotInitCheck(MOT_SAMP_NEEDLE);
				SetBeepWarning();
				Uart0ReUnable;
				uart_Printf("%s\r\n",strE3902);	// 清洗液灌注失败， 请检查清洗液供给，如果供给正常则请重新调试清洗液传感器后重试灌注
				Uart0ReEnable;
				mainStep = 11;
			}
			i = getLiqDetADC(NeedleChannel);
			if(i < liqDetBaseAdc)
			{
				i = liqDetBaseAdc - i;
				//if(i > 300)
				if(i > 150)
				{
					if(detRetry < 10)	// 重测计数
					{
						detRetry ++;	
						break;	
					}
					// 探测到排液异常
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepWarning();
					SetDelayTime(MOT_SAMP_NEEDLE, 5);
					uart_Printf("!2907\r\n");	// 清洗液灌注过程中检测到混匀池中液体水位超高，请检查废液泵排液状态
					// 退出处理
					mainStep = 11;
				}
				else
					detRetry = 0;
			}
			break;
		case 10:		// 完成退出
			//SetEValve(EV_ALL, EV_CLOSE);
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			waitMotSampNeedle = 1;
			mainStep = 0;
			return 1;
			break;
		case 11:		// 失败退出
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			waitMotSampNeedle = 1;
			mainStep = 0;
			return 0xff;
			break;
		default:
			mainStep = 0;
			break;
		}
	return 0;
}

unsigned char _ManualPrimingDiluent(void)
{
	static unsigned char mainStep = 0;
	unsigned char ucTmp;
	
	if(WaitDelayTime(MOT_EFFLUENT))		return 0;
	if(WaitDelayTime(MOT_DILUENT))		return 0;
	if(waitEffluent){if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;}
	
	switch(mainStep)
	{
		case 0:
			runNum = 5;
			mainStep = 1;
			break;
		case 1:		// 启动稀释液泵和废液泵
			SetEValve(EV_ALL, EV_CLOSE);
		//	_DiluentMotRun(600, 120);		// 最大灌注50mL液体，超出表示灌注失败，提示用户检查供液
		//	_EffluentMotRun(1000, 140);
			_DiluentMotRun(120, 120);		// 最大灌注50mL液体，超出表示灌注失败，提示用户检查供液
			_EffluentMotRun(200, 140);
			SetDelayTime(15, 30);		// 设置连续供水3秒判断灌注成功
			mainStep = 2;
			break;
		case 2:		// 等待液路事件
			SetDelayTime(MOT_EFFLUENT, 3);
			ReadLiquidMonitorResult(0);

			if(GetMotState(MOT_DILUENT)!=STA_SLAVE_FREE && GetMotState(MOT_EFFLUENT) != STA_SLAVE_FREE)
			{
				if(WaitDelayTime(15) != 0)
				{
					ucTmp = GetLiquidMonitorState(0);	// 读取0号液路
					if(ucTmp==INFO_LIQ_EMPTY || ucTmp==INFO_LIQ_BUBBLE)
					{
						SetDelayTime(15, 30);	// 液路有空气，重新开始计时
					}
				}
				else
				{
					// 灌注完成
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
					SetBeepAck();
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2102);
					Uart0ReEnable;
					mainStep = 3;
				}
			}
			else
			{
				if(runNum!=0)		// 因为稀释液泵流量小，需要最多灌注3遍
				{
					MotStop(MOT_DILUENT);
					MotStop(MOT_EFFLUENT);
				//	_EffluentMotRun(5, 220);
					SetDelayTime(MOT_DILUENT, 20);
					mainStep = 1;
					runNum --;
					break;
				}
				// 供液已经停止，灌注失败
				MotStop(MOT_DILUENT);
				MotStop(MOT_EFFLUENT);
				//MotInitCheck(MOT_SAMP_NEEDLE);
				SetBeepWarning();
				Uart0ReUnable;
//				uart_Printf("!2910\r\n");	// 稀释液灌注失败， 请检查稀释液供给，如果供给正常则请重新调试稀释液传感器后重试灌注
				uart_Printf("%s\r\n",strE3912);
				Uart0ReEnable;
				mainStep = 3;
			}
			break;
		case 3:		// 完成退出
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			mainStep = 4;
		case 4:
			mainStep = 0;
			return 1;
	}
	return 0;
}

unsigned char _ManualPrimingFluid(void)
{
	static unsigned char mainStep = 0;
	static unsigned int i;
	static unsigned char ucTmp;
	static unsigned char detRetry;
	
	if(WaitDelayTime(MOT_EFFLUENT))		return 0;
	if(WaitDelayTime(MOT_DILUENT))		return 0;
	if(waitEffluent){if(GetMotState(MOT_EFFLUENT)!=STA_SLAVE_FREE)		return 0;	waitEffluent = 0;}
	
	switch(mainStep)
	{
		case 0:
			runNum = 5;
			mainStep = 1;
			break;
		case 1:		// 启动清洗液泵和废液泵
			_FluidMotRun(100, 60);		// 最大灌注30mL液体，超出表示灌注失败，提示用户检查供液
			_EffluentMotRun(105, 60);
			SetDelayTime(15, 40);		// 设置连续供水3秒判断灌注成功
			ReadLiquidMonitorResult(1);
			mainStep = 2;
			break;
		case 2:		// 等待液路事件
			SetDelayTime(MOT_EFFLUENT, 3);
			ReadLiquidMonitorResult(1);
			if(GetMotState(MOT_EFFLUENT) != STA_SLAVE_FREE && GetMotState(MOT_FLUID) != STA_SLAVE_FREE) 
			{
				if(WaitDelayTime(15)!=0)
				{
					ucTmp = GetLiquidMonitorState(1);
					if(ucTmp==INFO_LIQ_EMPTY || ucTmp==INFO_LIQ_BUBBLE)
					{
						SetDelayTime(15, 40);	// 液路有空气，重新开始计时
					}
				}
				else
				{
					// 灌注完成
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					//MotInitCheck(MOT_SAMP_NEEDLE);
					SetBeepAck();
					Uart0ReUnable;
					uart_Printf("%s\r\n",strM2101);
					Uart0ReEnable;
					mainStep = 3;
				}
			}
			else
			{
				if(runNum!=0)		// 因为稀释液泵流量小，需要最多灌注3遍
				{
					MotStop(MOT_FLUID);
					MotStop(MOT_EFFLUENT);
					SetDelayTime(MOT_DILUENT, 20);
					mainStep = 1;
					runNum --;
					break;
				}
				// 供液已经停止，灌注失败
				MotStop(MOT_FLUID);
				MotStop(MOT_EFFLUENT);
				SetBeepWarning();
				Uart0ReUnable;
				uart_Printf("%s\r\n",strE3911);	// 清洗液灌注失败， 请检查清洗液供给，如果供给正常则请重新调试清洗液传感器后重试灌注
				Uart0ReEnable;
				mainStep = 3;
			}
			break;
		case 3:		// 完成/失败退出
			_EffluentMotRun(20, 220);
			waitEffluent = 1;
			mainStep = 4;
			break;
		case 4:
			mainStep = 0;
			return 1;
			break;
		}
	return 0;
}


void SetCleanMode(unsigned char m)
{
	if(m > 2)
		m = 0;
	if(CleanMode == 0)
	{
		CleanMode = m;
		Uart0ReUnable;
		uart_Printf("%s %4d\r\n",strM3156, CleanMode);
		Uart0ReEnable;
	}
}


/**************************************** File end *******************************************/

