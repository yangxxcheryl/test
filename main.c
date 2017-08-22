#include "B1404_LIB.h"
#include <iom1280v.h>
#include "Common.h"
#include <macros.h>
#include "eeprom.h"


unsigned char ControlModel;					// 0:正常， 1:超级终端调试
unsigned char WorkProcessStep;				// 工作进程号
extern unsigned char CardStoreOpenLook;		// 片仓工作状态
extern unsigned char JumpMode;				// 调整测试模式


//**************************************
unsigned char CardNoneUseful = 0;			// 老化模式
unsigned char workSwitch = 1;				// 停止工作开关
extern unsigned char CardStoreTestFlag;		// 取片测试模式
//**************************************

unsigned char primeProcessSW;		// 灌注开关，1:灌注稀释液； 2:灌注清洗液；3:测试模式下的液路自检

void init_devices(void);
void StartWork(void);
void MotorModelConfig(void);
void SetWorkStep(unsigned char n);
void CommandClear(void);

extern unsigned char GetNewTestCard;		// 取新的测试卡，0:无, 1:取插入卡, 2:取仓1卡, 3:取仓2卡, 4:取仓3卡, 5:取仓4卡, 6:取仓5卡， 255:取卡完成
extern unsigned char TurnPlateUsedLock;		// 转盘使用锁
extern unsigned char _LEDSTATE;				// 指示灯状态锁定 0 红  1 绿
unsigned char EchoSW = 1;

unsigned char (*miantianSubFunction)(void);
unsigned int MaintainSubFunParam;

void ReStart(unsigned char type)
{
	unsigned char i;
	CLI();
	Uart0ReUnable;
	uart_Printf("%s\r\n", strM0199);
	Uart0ReEnable;
	if(type != 0)
	{
		// 设置下载程序模式标识
		i = 1;
		EEPROM_WRITE(EEP_ADD_DOWNLOAD_FLAG, i);
	}
	WDR (); //this prevents a timeout on enabling
	WDTCSR |= (1<<WDCE) | (1<<WDE); //30-Oct-2006 Umesh 
	WDTCSR = 0x0F; // 0x08 WATCHDOG ENABLED - dont forget to issue WDRs
	SEI();
}


/*************************************************************************************************/

/******************************************* 主函数 **********************************************/
extern unsigned long SecondCount;
unsigned int cmd;
unsigned char uart[] = {'#','3','0','0','2','$','3',0x20,'G','B'};
void main(void)
{
	static unsigned char cmdState;
	static unsigned char taskSate;
	static unsigned char diluteProcessState;	// 稀释主程序执行状态
	static unsigned char GetNewPieceProcessState;//取片程序执行状态
	static unsigned char CardScanfSW;			// 试剂片条码检测位置调节
//	static unsigned char workSwitch=1;			// 停止工作开关
	static unsigned char dustbinOldState;

	int iPam;
	unsigned char dat, i;
	signed char si;
	unsigned int iTmp0, iTmp1;

	unsigned char workStep = 0;
	_CONST char * pFlash;
	
	miantianSubFunction = 0;	

	DiluteProcDatInit();
	RingQueueDatInit();
	TestQueueDatInit();
	UnloadQueueDatInit();
	InitCommandTransferData();
	InitControlLayerData();
	init_devices();
	
	Uart0ReUnable;
	uart_Printf("%s $%s\r\n", strM0100, strVersion);
	Uart0ReEnable;
	
	ControlModel = 1;
	SetCardTrolleyState(0);
	SetBeepAck();
	SetStateLedBusy();
	WorkProcessStep = 255;
	// 进入工作循环
	while(1)
	{
		// 进程命令执行
		switch(WorkProcessStep)
		{
		/**************** 空闲  *****************/
			case 255:
				if(ControlCmd.cmdState == 2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 0:		// 回显开关
							if(ControlCmd.pam[0]==0)
								EchoSW = 0;
							else
								EchoSW = 1;
							break;
						case 1:		// #0001 启动工作
							StartWork();
							SetWorkStep(0);		// 设置零工作状态
							GetStoreState(0);	// 初始化时读取片仓信息
							cmdState = 0;
							break;
						case 3:		// #0003 查询工作进程
							Uart0ReUnable;
							uart_Printf("%s $ 255 [Invalid]\r\n",strM0110);
							Uart0ReEnable;
							break;
						case 99:	// 重启, 设置看门狗
							ReStart(ControlCmd.pam[0]);
							break;
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
					}
					if(cmdState)
					{
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					else
					{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					CommandClear();
				}
				break;
			case 0:	
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 0:		// 回显开关
							if(ControlCmd.pam[0]==0)
								EchoSW = 0;
							else
								EchoSW = 1;
							break;
						case 1:		// #0001 启动工作
							StartWork();
							cmdState = 0;
							break;
						case 2:		// #0002 设置工作进程
							if(ControlCmd.pam[0] < 5 || ControlCmd.pam[0] == 9)
							{
								SetWorkStep(ControlCmd.pam[0]);
							}
							else
								cmdState = 1;	// 设置参数错误
							break;
						case 3:		// #0003 查询工作进程
							Uart0ReUnable;
							uart_Printf("%s $   0 [Free]\r\n",strM0110);
							Uart0ReEnable;
							break;
						case 10:	// #0010	开启片仓
							if(CardStoreOpenLook == 0)
								SetStoreDoorOpen(ControlCmd.pam[0]);
							else
							{
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0112);
								Uart0ReEnable;
							}
							break;
						case 11:	// #0011	读取片仓温湿度
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n", strM0111, GetStoreHumi(), GetStoreTemp());
							Uart0ReEnable;
							break;
						case 99:	// 重起, 设置看门狗
							ReStart(ControlCmd.pam[0]);
							break;
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
						}
					if(cmdState)
					{
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					else
					{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
					}
				break;
		/************** 机械自检  ***************/
			case 1:		
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 1:		// #0001 查询当前软件版本号
							Uart0ReUnable;
							uart_Printf("%s $%s [MechineCheck]\r\n", strM0100, strVersion);
							Uart0ReEnable;
							break;
						case 3:		// #0003 查询工作进程
							Uart0ReUnable;
							uart_Printf("%s $   1 [MechineCheck]\r\n",strM0110);
							Uart0ReEnable;
							break;
						case 5:
							if(ControlCmd.pam[0] == 0)
							{
								workSwitch = 1;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0105);
								Uart0ReEnable;
							}
							else
							{
								workSwitch = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0106);
								Uart0ReEnable;
							}
							break;
						case 99:	// 重启, 设置看门狗
							ReStart(ControlCmd.pam[0]);
							break;
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
						}
					if(cmdState){
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
						}
					else{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
						}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
					}
				if(workSwitch)
				{
					if(MachinePositionInit() == 1)
					{
						SetWorkStep(0);
					}
				}
				break;
		/*************** 液路自检 ***************/
			case 2:		
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 1:		// #0001 查询当前软件版本号
							Uart0ReUnable;
							uart_Printf("%s $%s [LiquidCheck]\r\n", strM0100, strVersion);
							Uart0ReEnable;
							break;
						case 3:		// #0003 查询工作进程
							Uart0ReUnable;
							uart_Printf("%s $   2 [LiquidCheck]\r\n",strM0110);
							Uart0ReEnable;
							break;
						case 5:
							if(ControlCmd.pam[0] == 0)
							{
								workSwitch = 1;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0105);
								Uart0ReEnable;
							}
							else
							{
								workSwitch = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0106);
								Uart0ReEnable;
							}
							break;
						case 99:	// 重起, 设置看门狗
							ReStart(ControlCmd.pam[0]);
							break;
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
						}
					if(cmdState){
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
						}
					else{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
						}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
					}
				if(workSwitch)
				{
					if(DiluteStartCheck(0) == 1)	// 函数返回1表示处理完毕
					{
						SetWorkStep(0);
					}
				}
				break;
		/************ 正常测试 *************/
			case 3:		
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					switch(ControlCmd.cmdIdx)
					{
						case 1:		// #0001 查询当前软件版本号
							Uart0ReUnable;
							uart_Printf("%s $%s [Test/Sleep]\r\n", strM0100, strVersion);
							Uart0ReEnable;
							break;
						case 3:		// #0003 查询工作进程
							Uart0ReUnable;
							if(JumpMode == 2)
								uart_Printf("%s $   5 [Sleep]\r\n",strM0110);
							else
								uart_Printf("%s $   3 [Test]\r\n",strM0110);
							Uart0ReEnable;
							if(taskSate >= 2)
							{
								Uart0ReUnable;
								uart_Printf("%s $   0 [Free]\r\n",strM3120);
								Uart0ReEnable;
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s $   1 [Busy]\r\n",strM3120);
								//DiluteProcess 函数
								printf_DiluteProcess_StepState();
								//CardStoreProcess 函数
								printf_CardstoreProcess_StepState();
								//TestQueueProcess 函数
								printf_TestProcess_StepState();
								//UnLoadProcess 函数
								printf_UnloadProcess_StepState();
								Uart0ReEnable;
							}
							break;
						case 4:	// 测试退出
							if(taskSate >= 2)
							{
								SetDiluentQuit();
								SetStateLedBusy();
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s $   1 [Busy]\r\n",strM3120);
								Uart0ReEnable;
							}
							break;
						case 5:	// 整机暂停
							if(ControlCmd.pam[0] == 0)
							{
								workSwitch = 1;
								clearstopFlag();
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0105);
								Uart0ReEnable;
								SetStateLedFree();
							}
							else
							{
								workSwitch = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0106);
								Uart0ReEnable;
								SetStateLedBusy();
							}
							break;
						case 10:	// 开启片仓
							if(CardStoreOpenLook == 0)
								SetStoreDoorOpen(ControlCmd.pam[0]);
							else
							{
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0112);
								Uart0ReEnable;
							}
							break;
						case 99:	// 重启, 设置看门狗
							ReStart(ControlCmd.pam[0]);
							break;
						case 3001:	// #3001 设置测试参数
							if(ControlCmd.pamLen>=4)
							{
								SetDiluentRatio((unsigned char)ControlCmd.pam[0]);
								SetWorkStoreNum((unsigned char)ControlCmd.pam[1]);
								SetReadMolule(ControlCmd.pam[2]);
								SetReadTime0(ControlCmd.pam[3]);
								SetReadTime1(ControlCmd.pam[4]);
							}
							else
								cmdState =  1;
							break;
						case 3002:	// #3002 设置片仓号
							SetWorkStoreNum(ControlCmd.pam[0]);
							break;
						case 3003:	// #3003 设置稀释比例
							SetDiluentRatio(ControlCmd.pam[0]);
							break;
						case 3004:	// #3004 设置第一读数时间
							SetReadTime0(ControlCmd.pam[0]);
							break;
						case 3005:	// #3005 设置第二读数时间
							SetReadTime1(ControlCmd.pam[0]);
							break;
						case 3006:	// 设置读数模块
							SetReadMolule(ControlCmd.pam[0]);
							break;
						case 3007:	// 设置滴液量
							SetDropVolume(ControlCmd.pam[0]);
							break;
						case 3008:	// 设置滴样模式
							SetDropMode(ControlCmd.pam[0]);
							break;
						case 3009:	// 设置抽打混匀次数
							SetReMixNum(ControlCmd.pam[0]);
							break;
						case 3010:	// 设置自动连续测试
							SetAutoTestCycle(ControlCmd.pam[0]);
							break;
						case 3011:	// 获取片仓状态信息
							ReportCardStoreState(ControlCmd.pam[0]);
							break;
						case 3012:	// SetLampLum
							if(ControlCmd.pam[0] == 0)
							{
								iTmp0 = AdjustTestLamp(0, ControlCmd.pam[1]);
								iTmp1 = AdjustTestLamp(1, ControlCmd.pam[2]);
							}
							else
							{
								iTmp0 = AdjustTestLamp(2, ControlCmd.pam[1]);
								iTmp1 = AdjustTestLamp(3, ControlCmd.pam[2]);
							}
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d\r\n", strM4114, iTmp0, iTmp1);
							Uart0ReEnable;
							break;
						case 3013:		// TurnOnLamp
							if(ControlCmd.pam[0] == 0)
								TestALampOpen();
							else	
								TestBLampOpen();
							break;
						case 3014:		// TurnOffLamp
							if(ControlCmd.pam[0] == 0)
								TestALampClose();
							else	
								TestBLampClose();
							break;
						case 3015:
							Uart0ReUnable;
							uart_Printf("%s $%4d\r\n",strM3137, GetWorkStoreNum());
							Uart0ReEnable;
							break;
						//case 3016:	// 设置条码扫描
						//	SetCardScanf(ControlCmd.pam[0]);
						//	break;
						case 3017:	// 设置稀释参数和4015指令一样
							iTmp0 = CalculateCalStandCoeff(ControlCmd.pam[0],ControlCmd.pam[1]);
							iTmp1 = Save_DiluentCalChart(ControlCmd.pam[0]);
							if(ControlCmd.pam[0] == 0)
								ControlCmd.pam[0] = 9;
							else if(ControlCmd.pam[0] > 13)
								ControlCmd.pam[0] = 13;
							Uart0ReUnable;
							uart_Printf("%s $%4d $%4d $%4d\r\n", strM4117, ControlCmd.pam[0],iTmp0,iTmp1);
							Uart0ReEnable;
							break;
						//case 3018:  // 设置重复条码扫描
						//	SetReCardScanf(1);
						//	break;
						//case 3019:  // 条码扫描位置调整
						//	if(GetNewPieceProcessState == 1)
						//	{
						//		iPam = SetCardScanfPos((signed char)ControlCmd.pam[0]);
						//		Uart0ReUnable;
						//		uart_Printf("%s $%4d\r\n",strM3124, iPam);
						//		Uart0ReEnable;
						//		CardScanfSW = 1;
						//	}
						//	else
						//	{
						//		Uart0ReUnable;
						//		uart_Printf("%s $1\r\n",strM3120);
						//		Uart0ReEnable;
						//	}
						//	break;
						case 3020:
							SetTestDebugMode(ControlCmd.pam[0]);
							break;
						case 3021:	// 设置取片测试模式
							SetGetCardTestMode(ControlCmd.pam[0]);
							break;
						case 3022:	// 设置1:1模式下,是否需要进行抽打混匀
							SetMixtureMode(ControlCmd.pam[0]);
							break;
						case 3023:	// 设置多排模式下,废片盒无效
							SetWasteCardState(ControlCmd.pam[0]);
							break;
#ifdef Puncture
						case 3024:	// 设置是否需要穿刺
							SetPunctureState(ControlCmd.pam[0]);
							break;
#endif

						case 3030:
							// 设置吸样量
							SetSamplingVolume(ControlCmd.pam[0]);
							break;
						case 3050:		// 休眠
							TestSleep();
							break;
						case 3051:		// 恢复
							TestStartup();
							break;
					//	case 3052:		// 设置休眠时间
					//		SetSleepTime(ControlCmd.pam[0]);
					//		break;
						case 3053:		// 设置取样功能开关
							if(ControlCmd.pam[0])
							{
								SamplingSwitch(0);
								SetStateLedBusy();	// 锁定仪器,无论之前是否为红，都设为红
								
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3153);
								Uart0ReEnable;
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3154);
								Uart0ReEnable;
								SamplingSwitch(1);
								if(_LEDSTATE == 1)		// 锁定前灯是绿的
								{
									SetStateLedFree();
								}
							}
							break;
						case 3054:	// 设置当前测试重测标识
							SetReReadFlag(); 

							break;
						case 3055:	// 设置清洗模式
							SetCleanMode(ControlCmd.pam[0]);
							break;
						case 3056:	// 设置读取结束应答信号
							SetReadCloseAnswer();	
							break;
						case 3060:		// 手动灌注稀释液
							if(diluteProcessState == 1) 
							{
								if(primeProcessSW == 0)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3160);
									Uart0ReEnable;
									SetStateLedBusy();
									primeProcessSW = 1;
								}
								else if(primeProcessSW == 1)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3171);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 2)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3172);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 3)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3173);
									Uart0ReEnable;
								}
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s $1\r\n",strM3120);
								Uart0ReEnable;
							}
							break;
						case 3061:		// 手动灌注清洗液
							if(diluteProcessState == 1) 
							{
								if(primeProcessSW == 0)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3161);
									Uart0ReEnable;
									SetStateLedBusy();
									primeProcessSW = 2;
								}
								else if(primeProcessSW == 1)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3171);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 2)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3172);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 3)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3173);
									Uart0ReEnable;
								}
							}
							else
							{
								Uart0ReUnable;
								uart_Printf("%s $1\r\n",strM3120);
								Uart0ReEnable;
							}
							break;
						case 3107:		// 滴样量调节因子设定
							//si = SetDropVolumeFactor((signed char)ControlCmd.pam[0]);
							SetDropVolumeFactor((signed int)ControlCmd.pam[0]);
							//Uart0ReUnable;
							//uart_Printf("%s $%4d\r\n", strM3218, si);
							//Uart0ReEnable;
							break;
						case 3108:
							GetMotorMonitorState(ControlCmd.pam[0],ControlCmd.pam[1]);
							break;
						case 3322:
							if(diluteProcessState == 1) 
							{
								if(primeProcessSW == 0)
								{
									Uart0ReUnable;
									uart_Printf("%s $ 3 [LiquidCheck] $ 3322\r\n",strM0110);
									Uart0ReEnable;
									SetStateLedBusy();
									primeProcessSW = 3;
								}
								else if(primeProcessSW == 1)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3171);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 2)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3172);
									Uart0ReEnable;
								}
								else if(primeProcessSW == 3)
								{
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM3173);
									Uart0ReEnable;
								}
							}
							else 
							{
								Uart0ReUnable;
								uart_Printf("%s $1\r\n",strM3120);
								Uart0ReEnable;
							}
							break;
						case 3333:
							if(CardStoreTestFlag == 1)		
								break;
							if(ControlCmd.pam[0] == 0)
								CardNoneUseful = 0;
							else
							{	
								CardNoneUseful = 1;
							//	TestALampClose();
							//	TestBLampClose();
							}
							Uart0ReUnable;
							uart_Printf("%s $%4d\r\n", strM3333, CardNoneUseful);
							Uart0ReEnable;
							break;
						//*******************************************************
						default:
							SetBeepWarning();
							Uart0ReUnable;
							uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
							Uart0ReEnable;
							break;
						}
					if(cmdState)
					{
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					else
					{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
				}
				if(workSwitch)
				{
					taskSate = 0;
					if(primeProcessSW == 0)
					{
						diluteProcessState = DiluteProcess(0);
						taskSate += diluteProcessState;
					}
					else
					{
						if(primeProcessSW == 1)
						{
							i = _ManualPrimingDiluent();
							if(i == 1)
							{
								primeProcessSW = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3162);
								Uart0ReEnable;
								if(JumpMode != 2)
									SetStateLedFree();
							}
						}
						else if(primeProcessSW == 2)
						{
							i = _ManualPrimingFluid();
							if(i == 1)
							{
								primeProcessSW = 0;
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM3163);
								Uart0ReEnable;
								if(JumpMode != 2)
									SetStateLedFree();
							}
						}
						else if(primeProcessSW == 3)
						{
							i = DiluteStartCheck(0);// 函数返回1表示处理完毕
							if(i == 1)
							{
								primeProcessSW = 0;
								if(JumpMode == 2)	// 如果是休眠模式下的3322
									JumpMode = 3;
								else
									JumpMode = 0;
								Uart0ReUnable;
								uart_Printf("%s [Test] $ 3322\r\n",strM2120);
								Uart0ReEnable;
								SetStateLedFree();
							}
						}
					}
					if(CardScanfSW == 0)
					{
						GetNewPieceProcessState = GetNewPieceFromStoreProcess(0);
					}
					else
					{
						if(1 == CardScanfPosCheck())
						{
							CardScanfSW = 0;
							Uart0ReUnable;
							uart_Printf("%sDone\r\n",strM3124);
							Uart0ReEnable;
						}
					}
					TestAQueueProcess();
					TestBQueueProcess();
					taskSate += UnloadQueueProcess();
					if(taskSate > 2)
						SetWorkStep(0);
				}
				break;
		/************ 调试维护 **************/
			case 4:		
				if(ControlCmd.cmdState==2)
				{
					cmdState = 0;
					Uart0ReUnable;
					uart_Printf("#%4d CommandReceive $   0\r\n",ControlCmd.cmdIdx);
					Uart0ReEnable;
					if(miantianSubFunction == 0)
					{
						// 空闲状态下接收命令
						switch(ControlCmd.cmdIdx)
						{
							case 1:		// #0001 查询当前软件版本号
								Uart0ReUnable;
								uart_Printf("%s $%s [Maintain]\r\n", strM0100, strVersion);
								Uart0ReEnable;
								break;
							case 3:		// #0003 查询工作进程
								Uart0ReUnable;
								uart_Printf("%s $   4 [Maintain]\r\n",strM0110);
								Uart0ReEnable;
								break;
							case 4:	// quit
								SetWorkStep(0);
								break;
							case 5:
								if(ControlCmd.pam[0] == 0)
								{
									workSwitch = 1;
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM0105);
									Uart0ReEnable;
								}
								else
								{
									workSwitch = 0;
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM0106);
									Uart0ReEnable;
								}
								break;
							case 10:	// #0010	开启片仓
								if(CardStoreOpenLook == 0)
									SetStoreDoorOpen(ControlCmd.pam[0]);
							else
								Uart0ReUnable;
								uart_Printf("%s\r\n",strM0112);
								Uart0ReEnable;
								break;
							case 11:	// #0011	读取片仓温湿度
								Uart0ReUnable;
								uart_Printf("%s $%4d $%4d\r\n",strM0111, GetStoreHumi(), GetStoreTemp());
								Uart0ReEnable;
								break;
							case 99:	// 重起, 设置看门狗
								ReStart(ControlCmd.pam[0]);	break;
							case 4002:	// 取样针在混匀池右边沿位置调整
								SetNeedleOnMixPosFactor((signed int)ControlCmd.pam[0]);break;
							case 4004:	// 滴样高度调整
								SetDropHeightFactor((signed int)ControlCmd.pam[0]);		break;
							case 4005:	miantianSubFunction = CardLoadStartAdjust;		break;
							case 4006:	miantianSubFunction = CardLoadEndAdjust;		break;
							case 4007:	miantianSubFunction = CardUnloadStartAdjust;	break;
							case 4008:	miantianSubFunction = CardUnloadEndAdjust;		break;
							case 4009:
								SetLiquidPhotoAdjustNum(ControlCmd.pam[0]);
								miantianSubFunction = LiquidPhotoAdjust;
								break;
							case 4010:
								SetCardStorePhotoAdjustNum(ControlCmd.pam[0]);
								miantianSubFunction = CardStorePhotoAdjust;
								break;
							case 4011:		// SetLampLum
								if(ControlCmd.pam[0] == 0)
								{
									iTmp0 = AdjustTestLamp(0, ControlCmd.pam[1]);
									iTmp1 = AdjustTestLamp(1, ControlCmd.pam[2]);
								}
								else
								{
									iTmp0 = AdjustTestLamp(2, ControlCmd.pam[1]);
									iTmp1 = AdjustTestLamp(3, ControlCmd.pam[2]);
								}
								Uart0ReUnable;
								uart_Printf("%s $%4d $%4d\r\n", strM4114, iTmp0, iTmp1);
								Uart0ReEnable;
								break;
							case 4012:		// GetLampLum
								if(ControlCmd.pam[0] == 0)
								{
									iTmp0 = ReadTestLampPWM(0);
									iTmp1 = ReadTestLampPWM(1);
								}
								else
								{
									iTmp0 = ReadTestLampPWM(2);
									iTmp1 = ReadTestLampPWM(3);
								}
								Uart0ReUnable;
								uart_Printf("%s $%4d $%4d\r\n", strM4114, iTmp0, iTmp1);
								Uart0ReEnable;
								break;
							case 4013:		// TurnOnLamp
								if(ControlCmd.pam[0] == 0)
									TestALampOpen();
								else	
									TestBLampOpen();
								break;
							case 4014:		// TurnOffLamp
								if(ControlCmd.pam[0] == 0)
									TestALampClose();
								else	
									TestBLampClose();
								break;
							case 4015:
								iTmp0 = CalculateCalStandCoeff(ControlCmd.pam[0],ControlCmd.pam[1]);
								iTmp1 = Save_DiluentCalChart(ControlCmd.pam[0]);
								if(ControlCmd.pam[0] == 0)
									ControlCmd.pam[0] = 9;
								else if(ControlCmd.pam[0] > 13)
									ControlCmd.pam[0] = 13;
								Uart0ReUnable;
								uart_Printf("%s $%4d $%4d $%4d\r\n", strM4117, ControlCmd.pam[0],iTmp0,iTmp1);
								Uart0ReEnable;
								break;
							case 4016:
								miantianSubFunction = DiluentQuantifyTest;
								SetDiluentQuantifyVolume(40);
								break;
							case 4017:	// 抽打混匀高度调整
								si = SetMixHeight((signed char)ControlCmd.pam[0]);
								Uart0ReUnable;
								uart_Printf("%s $%4d\r\n",strM4118, si);
								Uart0ReEnable;
								break;
							// 
							case 4020:	miantianSubFunction = TurnPlateCheck;		break;
							case 4021:	miantianSubFunction = NeedleTurnCheck;		break;
							case 4022:	miantianSubFunction = NeedleUpdownCheck;	break;
							case 4023:	miantianSubFunction = CardStoreMoveCheck;	break;
							case 4024:	miantianSubFunction = CardTakeHookCheck;	break;
							case 4025:	miantianSubFunction = CardLoadCheck;		break;
							case 4026:	miantianSubFunction = CardUnloadCheck;		break;
							case 4029:	miantianSubFunction = DiluentPumpCheck;		break;
							case 4030:	miantianSubFunction = LeanerPumpCheck;		break;
							case 4031:	miantianSubFunction = EffluentPumpCheck;	break;
							case 4032:	miantianSubFunction = SamplingSyringCheck;	break;
							case 4033:
							//	SetLiquidPhotoAdjustNum(HTCmdBuf.pam[0]);
								miantianSubFunction = LiquidPhotoCheck;
								break;
							case 4034:
								SetCardStorePhotoAdjustNum(ControlCmd.pam[0]);
								miantianSubFunction = CardStorePhotoCheck;
								break;
							case 4035:	miantianSubFunction = NeedleOnMixSideCheck;	break;
							case 4036:	miantianSubFunction = DropHeightCheck;	break;
							case 4037:	miantianSubFunction = MixHeightCheck;	break;
							case 4099:	miantianSubFunction = GetStoreProcess;break;
							
							case 4050:
								miantianSubFunction = DiluentQuantifyTest;
								SetDiluentQuantifyVolume(ControlCmd.pam[0]);
								break;
							case 4051:
								miantianSubFunction = LeanerQuantifyTest;
								SetLeanerQuantifyVolume(ControlCmd.pam[0]);
								break;
							case 4052:
								miantianSubFunction = SampQuantifyTest;
								SetSampQuantifyVolume(ControlCmd.pam[0]);
								break;
							case 4055:	// 上传传感器状态信息
								UpLoadingModuleSensorState((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1]);
								break;
							case 4056:	// 上传所有传感器状态信息
								UpLoadingAllSensorState();
								break;
							// 基本命令
							case 4060:		// 预定义位置设置
								cmdState =  MotAdjustPosition((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1]);
								break;
							case 4061:		// 起始位检测
								cmdState =  MotInitCheck((unsigned char)ControlCmd.pam[0]);
								break;
							case 4062:		// 电机运行
								cmdState =  MotRun((unsigned char)ControlCmd.pam[0], ControlCmd.pam[1]);
								break;
							case 4063:		// 电机运行到
								cmdState =  MotRunTo((unsigned char)ControlCmd.pam[0], ControlCmd.pam[1]);
								break;
							case 4064:		// 电机运行指定位置
								cmdState =  MotRunToSite((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1]);
								break;
							case 4065:		// 设置运行时参数
								cmdState =  SetMotRunPam((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1], (unsigned char)ControlCmd.pam[2],(unsigned char)ControlCmd.pam[3]);
								break;
							case 4066:		// 设置基本参数
								cmdState =  MotSetPam((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1], (unsigned char)ControlCmd.pam[2]);
								break;
							case 4067:	// 设置从机地址
								cmdState =  SlaveSetAddress((unsigned char)ControlCmd.pam[0]);
								break;
							/****************************** 液路控制 ******************************/	
							case 4068:	// 设置电磁阀
								cmdState =  SetEValve((unsigned char)ControlCmd.pam[0], (unsigned char)ControlCmd.pam[1]);
								break;
							case 4069:	// Get photo info
								cmdState =  GetLiquidPhotoInfo();
								break;
							case 4070:	// Liquid photo adjust
								cmdState =  SetLiquidPhotoAdjust((unsigned char)ControlCmd.pam[0]);
								break;
								/**************************** 片仓控制 *********************************/	
							case 4071:	// 获取片仓状态信息
								cmdState =  GetStoreState((unsigned char)ControlCmd.pam[0]);
								break;
							case 4072:	// 设置片仓校准
								cmdState =  SetStoreCAL((unsigned char)ControlCmd.pam[0]);
								break;
							case 4073:	// 获取片仓光电接收管信号值
								cmdState =  GetStorePhoVol((unsigned char)ControlCmd.pam[0]);
								break;
							case 4074:	// 获取液体探测信号值
								iPam = getLiqDetADC(NeedleChannel);
								Uart0ReUnable;
								uart_Printf("%s $%4d\r\n",strM4174, iPam);
								Uart0ReEnable;
								break;
							case 4075:
								iPam = getLiqDetADC(LoadChannel);
								Uart0ReUnable;
								uart_Printf("%s $%4d\r\n",strM4175, iPam);
								Uart0ReEnable;
								break;
							case 4076:
								iPam = getLiqDetADC(UnloadChannel);
								Uart0ReUnable;
								uart_Printf("%s $%4d\r\n",strM4176, iPam);
								Uart0ReEnable;
								break;
							case 4077:
								if(ControlCmd.pam[0] > 12)
									ControlCmd.pam[0] = 12;
								else if(ControlCmd.pam[0] < 1)
									ControlCmd.pam[0] = 1;
								MotStop(ControlCmd.pam[0]);
								break;
							default:
								SetBeepWarning();
								Uart0ReUnable;
								uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
								Uart0ReEnable;
								break;	
							}
						}
					else	// 工作状态下接收命令
					{
						switch(ControlCmd.cmdIdx)
						{
							case 3:		// #0003 查询工作进程
								Uart0ReUnable;
								uart_Printf("%s $   4 [Maintain]\r\n",strM0110);
								Uart0ReEnable;
								break;
							case 4:	// quit
								SetMaintianSubfunctionQuitFlag();
								break;
							case 5:
								if(ControlCmd.pam[0] == 0)
								{
									workSwitch = 1;
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM0105);
									Uart0ReEnable;
								}
								else
								{
									workSwitch = 0;
									Uart0ReUnable;
									uart_Printf("%s\r\n",strM0106);
									Uart0ReEnable;
								}
								break;
							case 99:	// 重起, 设置看门狗
								ReStart(ControlCmd.pam[0]);
								break;
							default:
								SetBeepWarning();
								Uart0ReUnable;
								uart_Printf("#%d CommandUnknown $   2\r\n",ControlCmd.cmdIdx);
								Uart0ReEnable;
								break;	
						}
					}
					if(cmdState)
					{
						SetBeepWarning();
						Uart0ReUnable;
						uart_Printf("#%4d CommandError $   1\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
					else
					{
						SetBeepAck();
						Uart0ReUnable;
						uart_Printf("#%4d CommandDone $   3\r\n",ControlCmd.cmdIdx);
						Uart0ReEnable;
					}
				//	if(ControlCmd.cmdIdx)
						CommandClear();
				}
				if(workSwitch)
				{
					if(miantianSubFunction)	// 调用子功能函数，直到函数返回1终止
					{
						if(ControlCmd.pamLen != 0)
						{
							MaintainSubFunParam = ControlCmd.pam[0];
							ControlCmd.pam[0] = 0;
						}
						else
							MaintainSubFunParam = 0;
						i = miantianSubFunction();
						if(i==1)
						{
							miantianSubFunction = 0;
						}
					}
				}
				break;
			default:
				break;
		}
		
		// 废片仓功能开启
		if(GetwasteCardState() == 0)
		{
			// 废片仓开关监测
			if((PINK & 0x02) == 0)
			{
				if(dustbinOldState > 0)
					dustbinOldState --;
				if(dustbinOldState == 2)
				{
					Uart0ReUnable;
					uart_Printf("%s $%4d\r\n",strM0200 , dustbinOldState);
					Uart0ReEnable;
				}
			}
			else		// 废片仓关闭
			{
				if(dustbinOldState < 255)
					dustbinOldState ++;
				if(dustbinOldState == 253)
				{
					Uart0ReUnable;
					uart_Printf("%s $%4d\r\n",strM0200 , dustbinOldState);
					Uart0ReEnable;
				}
			}
		}
	}
}

void SetWorkStep(unsigned char n)
{
	if(n > 4 && n != 9)
		n = 0;
	if(n != WorkProcessStep)
	{
		WorkProcessStep = n;
		switch(WorkProcessStep)
		{
			case 0:	Uart0ReUnable;uart_Printf("%s $   0 [Free]\r\n",strM0110);Uart0ReEnable;
				break;
			case 1:	Uart0ReUnable;uart_Printf("%s $   1 [MechineCheck]\r\n",strM0110);Uart0ReEnable;
				break;
			case 2:	Uart0ReUnable;uart_Printf("%s $   2 [LiquidCheck]\r\n",strM0110);Uart0ReEnable;
				break;
			case 3:	GetStoreState(0);	Uart0ReUnable;uart_Printf("%s $   3 [Test]\r\n",strM0110);Uart0ReEnable;
				break;
			case 4:	Uart0ReUnable;uart_Printf("%s $   4 [Maintain]\r\n",strM0110);Uart0ReEnable;
				break;
			case 9:	JumpMode = 1;GetStoreState(0);	WorkProcessStep = 3; Uart0ReUnable;uart_Printf("%s $   3 [Test]\r\n",strM0110);Uart0ReEnable;
				break;
		}
	}
}


/*************************************************************************************************/

/*************************************************************************************************/
void StartWork(void)
{
	CLI();

	// 初始化应用层数据
	DiluteProcDatInit();
	RingQueueDatInit();
	TestQueueDatInit();
	UnloadQueueDatInit();
	
	InitControlLayerData();
	InitCommandTransferData();

//	RegisterSlave(COMMON_ADDRESS);
	RegisterSlave(MOT_TURN_PLATE);
	RegisterSlave(MOT_SAMP_TRUN);
	RegisterSlave(MOT_SAMP_NEEDLE);
	RegisterSlave(MOT_CARD_LOAD);
	RegisterSlave(MOT_CARD_UNLOAD);
	RegisterSlave(MOT_STORE_CARD_MOVE);
	RegisterSlave(MOT_DILUENT);
	RegisterSlave(MOT_FLUID);
	RegisterSlave(MOT_EFFLUENT);
	RegisterSlave(MOT_SAMP_PUMP);
	RegisterSlave(LIQUID_CONTROL);
	RegisterSlave(STORE_MONITOR);

	StartCommandTransfer();
	SEI();

	//MotorModelConfig();
	Uart0ReUnable;
	uart_Printf("%s $%s\r\n", strM0101, strVersion);
	uart_Printf("%s\r\n", strM0103);
	uart_Printf("%s\r\n", strM0104);
	Uart0ReEnable;
	
}

void MotorModelConfig(void)
{
	// 设置基本参数
	/*	参数0：bit[7]停止锁开关
		bit[6:5]运行方式,[00]直线复式,[01]圆周循环式
		Bit[4:3]限位传感器类型，[00]无传感器，[01]槽型光耦，[10]微动开关；
		Bit[2:1]零位传感器类型，[00]无传感器，[01]槽型光耦，[10]微动开关；
		Bit[0]正反转设置，[0]正转，[1]反转；
		参数1：圆周等分数。
	*/
	SetMotRunPam(MOT_TURN_PLATE,120,10,CURRENT_TURN_PLATE);
	SetMotRunPam(MOT_SAMP_TRUN,50,40,CURRENT_SAMP_TRUN);
	SetMotRunPam(MOT_SAMP_NEEDLE,200,10,CURRENT_SAMP_NEEDLE);
	SetMotRunPam(MOT_CARD_LOAD,160,20,CURRENT_CARD_LOAD);
	SetMotRunPam(MOT_CARD_UNLOAD,160,20,CURRENT_CARD_UNLOAD);
	SetMotRunPam(MOT_STORE_CARD_MOVE,32,10,CURRENT_STORE_MOVE);
	SetMotRunPam(MOT_DILUENT,200,20,CURRENT_DILUENT);
	SetMotRunPam(MOT_FLUID,128,60,CURRENT_FLUID);
	SetMotRunPam(MOT_EFFLUENT,128,60,CURRENT_EFFLUENT);
	SetMotRunPam(MOT_SAMP_PUMP,64,60,CURRENT_SAMP_PUMP);

}


/********************************************* File end **********************************************/

