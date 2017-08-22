

#ifndef _B1404_LIB_H
#define _B1404_LIB_H


#ifndef _CONST
#define _CONST __flash
#endif

#define Uart0ReEnable		 	(UCSR0B |= (1 << 4))
#define Uart0ReUnable			(UCSR0B &= ~(1 << 4))

#define STX		0x02
#define ETX		0x03
#define ACK		0x06
#define NAK		0x15

#define ERR_SLAVE_NO_REPLY		1		// 从机无应答
#define ERR_COMMAND_ERROR		2		// 命令错误
#define ERR_SLAVE_ZERO_LOSE		3		// 从机找不到起始位
#define ERR_SLAVE_POSITION_LOSE	4		// 从机位置错误
#define ERR_SLAVE_ABNORMAL_STOP	5		// 从机运行异常停止
#define ERR_SLAVR_INFO_LOSE		6

// 数组相关常量
#define SLAVE_NUM			15
#define RETURN_OVER_TIME	10
#define INFO_EVENT_NUM		50
#define BACK_INFO_LEN		10
//#define STATE_EVENT_NUM		30
//#define ERROR_EVENT_NUM		30
#define COMMAND_BUF_LEN		15

// 模块名称编号定义
#define COMMON_ADDRESS		0
#define MOT_TURN_PLATE		1		// 
#define MOT_SAMP_TRUN		2
#define MOT_SAMP_NEEDLE		3
#define MOT_CARD_LOAD		4
#define MOT_CARD_UNLOAD		5

#define MOT_STORE_CARD_MOVE	8
#define MOT_DILUENT			9
#define MOT_FLUID			10
#define MOT_EFFLUENT		11
#define MOT_SAMP_PUMP		12
#define LIQUID_CONTROL		13		// 液路控制
#define STORE_MONITOR		14		// 片仓监测
#define LIQUID_DETECTED		15

#define waitCardBarcode		6		// 等待试剂片条码采集

#define	StopMonitor			1
#define ZeroMonitor			0

// 电机模块命令
#define CMD_MOT_ENABLE		0
#define CMD_MOT_RUN			1
#define CMD_MOT_RUN_TO		2
#define CMD_MOT_RUN_TO_SITE	3
#define CMD_MOT_SET_PAM		4
#define CMD_MOT_GET_POS		5
#define CMD_MOT_INIT		6
#define CMD_MOT_ADJUST		7
#define CMD_MOT_SET_BASE_PAM	8
#define CMD_MOT_STOP		9
#define CMD_MOT_SET_ADD		7

// 液路控制模块命令
#define CMD_LIQ_SET_VALVE		1	// 设置电磁阀开关状态
#define CMD_LIQ_GET_STATE		2	// 获取液路光耦状态
#define CMD_LIQ_SET_MONITOR		3	// 设置液路监测模式
#define CMD_LIQ_START_MONITOR	4	// 启动液路监测
#define CMD_LIQ_READ_MONITOR	5	// 读取液路监测结果
#define CMD_LIQ_CAL_PHOTO		6	// 自动校准液路光耦信号值，自动调整在有水的情况下液路光耦信号到最佳值
#define CMD_LIQ_GET_PHOTO		7	// 获取液路光耦信号值
#define CMD_LIQ_FLOW_CHECK		8	// 流量检查

// 片仓监测模块电路
#define CMD_STORE_GET_STATE	1		// 获取全部片仓状态

#define CMD_STORE_CAL		3		// 同时校准全部片仓光路
#define CMD_STORE_GET_VOL	4		// 校准指定片仓
#define CMD_STORE_OPEN		5		// 开启仓门

// 状态信息
#define STA_SLAVE_IDLE		0x00
#define STA_SLAVE_FREE		0x01
#define STA_SLAVE_BUSY		0x02
#define STA_SLAVE_CMD_DONE	0x03
#define STA_SLAVE_CMD_ERR	0x04
#define STA_SLAVE_CHANGLE	0x05
#define STA_SLAVE_RECEIVE_OK	0x06
#define STA_SLAVE_RECEIVE_ERR	0x07


// 状态改变类型
#define STA_INFO_NULL		0x00
// 电机模块状态信息
#define STA_CHANGE_POS		1
#define STA_INFO_POS		2
#define STA_MOT_PHO			3		// 电机光藕信息

#define STA_INFO_PLATEPOS	5	// 当前转盘槽位置
#define STA_INFO_CENTRE_PLATEPOS	6	// 当前转盘槽中心位置


// 液路控制状态信息
#define INFO_LIQ_PHO_VAL	10		// 液路光耦输出信号值
#define INFO_LIQ_PHO_ADJ	11		// 液路光耦调整结果
#define INFO_LIQ_PHO_ON		12		// 液路检测光耦检测到液体
#define INFO_LIQ_PHO_OFF	13		// 液路检测光耦检测到空
#define INFO_LIQ_EMPTY		14		// 液路空
#define INFO_LIQ_FULL		15		// 液路满
#define INFO_LIQ_BUBBLE		16		// 液路有气泡
#define INFO_LIQ_FLOW		17		// 液路流动
// 片仓监测状态信息
#define INFO_STORE_OPEN		20		// 片仓打开
#define INFO_STORE_CLOSE	21		// 片仓关闭
#define INFO_STORE_FULL		22		// 片仓满
#define INFO_STORE_LITTLE	23		// 片仓少量
#define INFO_STORE_EMPTY	24		// 片仓空
#define INFO_STORE_ERROR	25		// 片仓状态错误
#define	INFO_STORE_CAL 		26		// 片仓光路校准正确
#define INFO_STORE_PHO_VOL	28		// 片仓光电接收管电压
#define INFO_STORE_STATE_ALL	29	// 所有片仓状态信息
#define INFO_STORE_STATE_SPC	30	// 指定片仓状态信息
#define INFO_STORE_HUMITURE		31	// 片仓温湿度
#define INFO_STORE_OPEN_ERR		32	// 片仓开启错误

#define INFO_LIQDETECT_LARGEN	40	// 探测到一体容量增加
#define INFO_LIQDETECT_LESSEN	41	// 探测到一体容量减少	

#define EV_OPEN			1
#define EV_CLOSE		0

#define EV_ALL	0
#define EV1		1		// 混匀池和清洗头之间清洗液切换，混匀池为常开状态
#define EV2		2		// 取样针管路清洗液切换，常闭状态
#define EV3		3		// 混匀池和清洗头之间废液切换，混匀池为常开状态
#define EV4		4
#define EV5		5

// EEPROM分配定义
#define EEP_ADD_DOWNLOAD_FLAG			50
#define EEP_ADD_SLEEP_TIME				80
#define EEP_ADD_LAMPSET					84
#define EEP_ADD_CAL_DAT					100		// 2*5+2+2+2*14 = 42Byte
#define EEP_ADD_SERIAL					150		// 测试序列号 48+2 Byte
#define EEP_ADD_TRANSH_CNT				200		// 废片盒计数 4Byte
#define EEP_ADD_CARDSCANF_POS			205		// 条码扫描相对零位位置调整量
#define EEP_ADD_NEEDLE_MIX_ADJ			220		// 取样臂到混匀池右边沿位置调整量
#define EEP_ADD_DROP_HEIGHT_ADJ			221		// 滴样高度调整量
#define EEP_ADD_MIX_HEIGHT_ADJ			222		// 混匀高度调整量
#define EEP_DROP_VOLUME_FACTOR			224		// 滴样量调节因子
/*
#define EEP_ADD_SERIAL					200		// 测试序列号 48+2 Byte
#define EEP_ADD_TRANSH_CNT				300		// 废片盒计数 4Byte
#define EEP_ADD_CARDSCANF_POS			305		// 条码扫描相对零位位置调整量
#define EEP_ADD_NEEDLE_MIX_ADJ			320		// 取样臂到混匀池右边沿位置调整量
#define EEP_ADD_DROP_HEIGHT_ADJ			321		// 滴样高度调整量
#define EEP_ADD_MIX_HEIGHT_ADJ			322		// 混匀高度调整量
*/

typedef struct _COMMAND_STRING{		// 命令结构
	unsigned char cmd;					// 若当前无命令自动设成状态查询命令
	unsigned char pam[6];				// 参数串
	unsigned char cmdLen;				// 参数长度
}COMMAND_STRING;

typedef struct _INFO_EVENT{		// 信息事件结构
	unsigned char slaveNum;			// 从机编号
	unsigned char event;			// 事件类型
	unsigned char info[BACK_INFO_LEN];	// 事件参数
	unsigned char infoLen;
}INFO_EVENT;


void printf_DiluteProcess_StepState(void);
void printf_CardstoreProcess_StepState(void);
void printf_TestProcess_StepState(void);
void printf_UnloadProcess_StepState(void);


// Control
void InitControlLayerData(void);
unsigned char GetRingPieceState(unsigned char n);
unsigned char GetStoreHumi(void);
unsigned char GetStoreTemp(void);
unsigned char GetMotPositionOfSite(unsigned char slaveNum);
int GetMotPositionOfStep(unsigned char slaveNum);
unsigned char GetLiquidMonitorState(unsigned char num);
unsigned char GetLiquidMonitorStatePam(unsigned char num);
unsigned char RegisterPosChangeEvenProcess(void * proc);
void CleanPosChangeEvenProcess(void);
unsigned char RegisterLiquidEvenProcess(void * proc);
void CleanLiquidEvenProcess(void);
unsigned char RegisterCardStoreEvenProcess(void * proc);
void CleanPosCardStoreProcess(void);
void UpLoadingModuleSensorState(unsigned char slaveNum, unsigned char num);
void UpLoadingAllSensorState(void);
void SlaveEventAssignProcess(INFO_EVENT * pInfoEvent);
unsigned char MachinePositionInit(void);
unsigned char WaitStartKey(void);
void SetBeepWarning(void);
void SetBeepAck(void);
void SetBeepPrompt(void);
void SetBeepError(void);
void SetBeep(unsigned char n);
void BeepStop(void);
void Beep(void);
void SetStateLedBusy(void);
void SetStateLedFree(void);

//unsigned char GetTurnPlateMonitorState(void) ;  
signed char GetMotorMonitorState(unsigned char slave,unsigned char num);
// Control End


// Functional Interface
unsigned char  GetMotState(unsigned char slaveNum);
unsigned char MotStop(unsigned char slaveNum);
unsigned char MotRun(unsigned char slaveNum, int n);
unsigned char MotRunTo(unsigned char slaveNum, int x);
unsigned char MotRunToSite(unsigned char slaveNum, unsigned char definePos);
unsigned char SetMotRunPam(unsigned char slaveNum, unsigned char maxVel,unsigned char accel,  unsigned char current);
unsigned char MotSetLock(unsigned char slaveNum, unsigned char lock);
unsigned char MotSetEnable(unsigned char slaveNum, unsigned char enable);
unsigned char GetMotPositionFromSlave(unsigned char slaveNum);
unsigned char MotInitCheck(unsigned char slaveNum);
unsigned char MotAdjustPosition(unsigned char slaveNum, unsigned posNum);
unsigned char MotSetPam(unsigned char slaveNum, unsigned char pam0, unsigned char pam1);
unsigned char SlaveSetAddress(unsigned char address);
unsigned char SetEValve(unsigned char evNum, unsigned char sw);
unsigned char GetLiquidState(unsigned char phNum);
unsigned char SetLiquidMonitor(unsigned char phNum, unsigned char mode);
unsigned char StartLiquidMonitor(unsigned char phNum);
unsigned char ReadLiquidMonitorResult(unsigned char phNum);
unsigned char SetLiquidPhotoAdjust(unsigned char phNum);
unsigned char GetLiquidPhotoInfo(void);
unsigned char LiquidFlowCheck(unsigned char liqNum);
unsigned char GetStoreState(unsigned  char chNum);
unsigned char SetStoreCAL(unsigned char num);
unsigned char GetStorePhoVol(unsigned char num);
unsigned char SetStoreDoorOpen(unsigned char num);
// Functional Interface End


// DiluteProcess
void SetDropVolumeFactor(signed int n);
void SetTestDebugMode(unsigned char m);
unsigned char SamplingSwitch(unsigned char  sw);
unsigned char DiluteStartCheck(INFO_EVENT * pInfoEvent);
void SetAutoTestCycle(unsigned int num);
unsigned char SetDiluentQuit(void);
unsigned char DiluteProcess(INFO_EVENT * pInfoEvent);
void SetWorkStoreNum(unsigned char num);
unsigned char GetWorkStoreNum(void);
void SetDiluentRatio(unsigned char num);
void SetReadTime0(unsigned int t);
void SetReadTime1(unsigned int t);
void SetReadMolule(unsigned char n);
void SetSleepTime(unsigned int t);
void TestSleep(void);
void TestStartup(void);
void SetDropMode(unsigned int mode);
void SetReMixNum(unsigned int MixNum);
unsigned long ReadCurTestSetial(void);
unsigned char _ManualPrimingDiluent(void);
unsigned char _ManualPrimingFluid(void);
// DiluteProcess  [End]

// CardStoreProcess
void clearstopFlag(void);
void SetGetCardTestMode(unsigned char m);
void SetWorkStoreNum(unsigned char num);
void SetCardScanf(unsigned char num);
unsigned char GetNewPieceFromStoreProcess(INFO_EVENT * pInfoEvent);
unsigned char GetStoreProcess(void);
void ReportCardStoreState(unsigned char num);
void SetCardTrolleyState(unsigned char state);
unsigned char GetCardTrolleyState(void);
unsigned char CardStoreSteteProcess(INFO_EVENT * pInfoEvent);
unsigned int  CalculateCalStandCoeff(unsigned int m,unsigned int n);

void TrashCanMonitor(void);
unsigned char TrashCanCheck(void);
void SetReCardScanf(const unsigned char num);
// CardStoreProcess  [End]

// TestQueueProcess
unsigned char TestAQueueProcess(void);
unsigned char TestBQueueProcess(void);
void TestALampOpen(void);
void TestALampClose(void);
void TestBLampOpen(void);
void TestBLampClose(void);
void ReSetTestLampPWM(unsigned char n);
unsigned int ReadTestLampPWM(unsigned char n);
unsigned int AdjustTestLamp(unsigned char n, unsigned int adj);

void SetReReadFlag(void);
unsigned char SetReadCloseAnswer(void);
// TestQueueProcess  [End]

unsigned int getLiqDetADC(unsigned char num);
unsigned char CheckLiqDetBase(void);
signed int GetLiqDetDiff(void);
unsigned char GetLiqDetResult(signed int ref);

void SetWasteCardState(unsigned char m);
unsigned char GetwasteCardState(void);
// Maintain
unsigned char SetMaintianSubfunctionQuitFlag(void);

void SetNeedleOnMixPosFactor(signed int n);
signed int GetNeedleOnMixCenterPos(void);
signed int GetNeedleOnMixSidePos(void);
void SetDropHeightFactor(signed int n);
signed int GetDropHeight(void);


unsigned char CardLoadStartAdjust(void);
unsigned char CardLoadEndAdjust(void);
unsigned char CardUnloadStartAdjust(void);
unsigned char CardUnloadEndAdjust(void);
void SetLiquidPhotoAdjustNum(unsigned char n);
unsigned char LiquidPhotoAdjust(void);
void SetCardStorePhotoAdjustNum(unsigned char n);
unsigned char CardStorePhotoAdjust(void);
unsigned char LiquidPhotoCheck(void);
unsigned char CardStorePhotoCheck(void);

unsigned char TurnPlateCheck(void);
unsigned char NeedleTurnCheck(void);
unsigned char NeedleUpdownCheck(void);
unsigned char CardStoreMoveCheck(void);
unsigned char CardTakeHookCheck(void);
unsigned char CardLoadCheck(void);
unsigned char CardUnloadCheck(void);
unsigned char DiluentPumpCheck(void);
unsigned char LeanerPumpCheck(void);
unsigned char EffluentPumpCheck(void);
unsigned char SamplingSyringCheck(void);
unsigned char NeedleOnMixSideCheck(void);
unsigned char DropHeightCheck(void);
unsigned char MixHeightCheck(void);

void SetDiluentQuantifyVolume(unsigned char n);
unsigned char DiluentQuantifyTest(void);
void SetLeanerQuantifyVolume(unsigned char n);
unsigned char LeanerQuantifyTest(void);
void SetSampQuantifyVolume(unsigned char n);
unsigned char SampQuantifyTest(void);
// Maintain [End]

/************************************ B1404_LIB *****************************************/
// Uart0	连接PC
unsigned char uart0SendData(unsigned char * data, unsigned short len);
unsigned char uart0SendChar(unsigned char c);
unsigned char uart0SendString(_CONST char * p);
unsigned char uart0GetChar(void);
void  uart_Printf(char *f, ...);
void  uart0SendInt(unsigned long num);


// 延时函数
void SetDelayTime(unsigned char num, unsigned int t);
unsigned char WaitDelayTime(unsigned char num);

// 分机控制通讯
void InitCommandTransferData(void);
unsigned char RegisterSlave(unsigned char slaveNum);
void StartCommandTransfer(void);


// 一般通用函数
void MemCopy(void *ps, void *pt, unsigned char n);
unsigned char StringMatching(_CONST char * str1, char * str2);
unsigned int  StringToInt(const char * pStr);
unsigned int  StringToInt2(const char * pStr);
unsigned int AbsDifference(unsigned int a, unsigned int b);


unsigned char AT24C256_Read(unsigned int addr,unsigned char *ptr,unsigned int len);
unsigned char AT24C256_Write(unsigned int addr,unsigned char *ptr,unsigned int len);

/******************************* 稀释液流量定标 ***********************************/

// 校准常数
#define _DILUENT_MIX_BASE_COEFF			100		// 仪器校准基准值，主要用于校准混匀池和取样注塞
#define _DILUENT_MIX_BASE_COEFF_UP		250
//#define _DILUENT_MIX_BASE_COEFF_DOWN	50
#define _DILUENT_MIX_BASE_COEFF_DOWN	10
#define _DILUENT_PUMP_BASE_COEFF		625		// 稀释泵流量定标基准值
#define _DILUENT_PUMP_BASE_COEFF_UP		800
#define _DILUENT_PUMP_BASE_COEFF_DOWN	455
#define _FLOW_CAL_LIST_NUM 5

typedef struct _FLOW_CAL_CHART{	// 流量定标图标
	unsigned int list[_FLOW_CAL_LIST_NUM];		// 10个定标值取其平均值为结果
	unsigned char pnt;
	unsigned int calValue;				// 泵管定标值 (定标液在混匀池中的高度)
	unsigned int calStand[14];			// 定标标准值 (计算出1200uL液体在混匀池中的高度)
}FLOW_CAL_CHART;

extern FLOW_CAL_CHART _DiluentCalChart;
extern unsigned int DiluentCoff[14];		// 稀释校准因数，校准吸样量，反向因数，值越大调整结果越小

void InitFlowMeter(void);
unsigned int JudgeFlowMeter(unsigned int i);

unsigned int InsetrDiluentFlowCalValue(unsigned int n);
void Read_DiluentCalChart(void);
//void Save_DiluentCalChart(void);
//unsigned int Save_DiluentCalChart(void);
unsigned int Save_DiluentCalChart(unsigned int m);


/*****************************************************************************************/
#endif


/********************************************* File end **********************************************/
