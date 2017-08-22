
/*
1.命令字符串定义
2.终端输入命令解析提取
*/
/*
输入:void CommandExplain(unsigned char c)	在串口输入中断中调用
输出:CONTROL_CMD ControlCmd					任务调度中读取并处理
*/

#include "Common.h"
/*
typedef struct _HT_COMMAND_STRING{
	_CONST char * pChar;
	_CONST unsigned int indx;
}HTCMD_STR;

typedef struct _HTCOMMAND_BUF{
	unsigned char itemPnt;			// 当前条目指针
	unsigned char charPnt;			// 当前条目字符指针
	unsigned char cmdStr[35];		// 命令字符串
	unsigned char pamStr[6][10];	// 参数
}HTCOMMAND_BUF;
*/
typedef struct _UICOMMAND_BUF{
	unsigned char itemPnt;			// 当前接收的项目号
	unsigned char charPnt;			// 接收指针
	unsigned char cmdStr[10];		// 命令名称字符串
}UICOMMAND_BUF;
/*
// 启动
_CONST char CStr0000[] 		= "echo";		// 回显开关
_CONST HTCMD_STR HTCmd0000 	= {CStr0000, 0};
_CONST char CStr0001[] 		= "startrap01";		// 启动仪器
_CONST HTCMD_STR HTCmd0001 	= {CStr0001, 1};
_CONST char CStr0002[] 		= "setwork";		// 设置工作进程
_CONST HTCMD_STR HTCmd0002 	= {CStr0002, 2};
_CONST char CStr0003[] 		= "askwork";		// 询问当前工作进程
_CONST HTCMD_STR HTCmd0003 	= {CStr0003, 3};
_CONST char CStr0004[] 		= "quit";		// 退出当前工作进程
_CONST HTCMD_STR HTCmd0004 	= {CStr0004, 4};
_CONST char CStr0005[] 		= "suspend";				// 	暂停工作开关
_CONST HTCMD_STR HTCmd0005 	= {CStr0005, 5};
_CONST char CStr0010[] 		= "storedooropen";		// 开启片仓
_CONST HTCMD_STR HTCmd0010 	= {CStr0010, 10};
_CONST char CStr0011[] 		= "getstorehumiture";	// 读取片仓温湿度
_CONST HTCMD_STR HTCmd0011 	= {CStr0011, 11};
_CONST char CStr0099[] 		= "restart";	// 重起
_CONST HTCMD_STR HTCmd0099 	= {CStr0099, 99};

_CONST HTCMD_STR * CmdString0[] = {
	&HTCmd0000, &HTCmd0001, &HTCmd0002, &HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0010, &HTCmd0011, &HTCmd0099, 0
};
// 机械自检
_CONST HTCMD_STR * CmdString1[] = {
	&HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0099, 0
};
// 液路自检
_CONST HTCMD_STR * CmdString2[] = {
	&HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0099, 0
};
// 测试
_CONST char CStr3001[] 		= "settestparamter";	// 设置测试参数
_CONST HTCMD_STR HTCmd3001 	= {CStr3001, 3001};
_CONST char CStr3002[] 		= "setworkstore";		// 设置当前取片卡仓号
_CONST HTCMD_STR HTCmd3002 	= {CStr3002, 3002};
_CONST char CStr3003[] 		= "setdiluteratio";		// 设置稀释比例
_CONST HTCMD_STR HTCmd3003 	= {CStr3003, 3003};
_CONST char CStr3004[] 		= "setreadtime1";		// 设置读数时间1
_CONST HTCMD_STR HTCmd3004 	= {CStr3004, 3004};
_CONST char CStr3005[] 		= "setreadtime2";		// 设置读数时间2
_CONST HTCMD_STR HTCmd3005 	= {CStr3005, 3005};
_CONST char CStr3006[] 		= "setreadmodule";		// 设置读数模块
_CONST HTCMD_STR HTCmd3006 	= {CStr3006, 3006};
_CONST char CStr3007[] 		= "setdropvolume";		// 设置滴液容量
_CONST HTCMD_STR HTCmd3007 	= {CStr3007, 3007};
_CONST char CStr3010[] 		= "setautotest";		// 设置连续测试周期
_CONST HTCMD_STR HTCmd3010 	= {CStr3010, 3010};
_CONST char CStr3011[] 		= "askstorestate";		// 查询片仓状态
_CONST HTCMD_STR HTCmd3011 	= {CStr3011, 3011};
_CONST char CStr3012[] 		= "setlamplum";	// 
_CONST HTCMD_STR HTCmd3012 	= {CStr3012, 3012};
_CONST char CStr3013[] 		= "turnonlamp";			// 开启测试光源
_CONST HTCMD_STR HTCmd3013 	= {CStr3013, 3013};
_CONST char CStr3014[] 		= "turnofflamp";		// 关闭测试光源
_CONST HTCMD_STR HTCmd3014 	= {CStr3014, 3014};
_CONST char CStr3015[] 		= "getworkstore";		// 获取当前工作仓号
_CONST HTCMD_STR HTCmd3015 	= {CStr3015, 3015};
_CONST char CStr3020[] 		= "setdebugmode";		// 	设置测试状态下的调试模式，0:正常，1:混匀液测量
_CONST HTCMD_STR HTCmd3020 	= {CStr3020, 3020};

_CONST char CStr3050[] 		= "sleep";				// 	进入休眠
_CONST HTCMD_STR HTCmd3050 	= {CStr3050, 3050};
_CONST char CStr3051[] 		= "startup";			// 	恢复测试
_CONST HTCMD_STR HTCmd3051 	= {CStr3051, 3051};
_CONST char CStr3052[] 		= "setsleeptime";		// 	设置休眠时间
_CONST HTCMD_STR HTCmd3052 	= {CStr3052, 3052};
_CONST char CStr3053[] 		= "samplingsw";			// 	取样功能开关
_CONST HTCMD_STR HTCmd3053 	= {CStr3053, 3053};
_CONST char CStr3054[] 		= "rereadtest";			// 	重新读取测试卡
_CONST HTCMD_STR HTCmd3054 	= {CStr3054, 3054};
_CONST char CStr3055[] 		= "setcleanmode";		// 	设置清洗模式
_CONST HTCMD_STR HTCmd3055 	= {CStr3055, 3055};
_CONST char CStr3056[] 		= "setreadclose";		// 	设置读数结束
_CONST HTCMD_STR HTCmd3056 	= {CStr3056, 3056};

_CONST char CStr3060[] 		= "manualprimedil";		// 	手动灌注稀释液
_CONST HTCMD_STR HTCmd3060 	= {CStr3060, 3060};
_CONST char CStr3061[] 		= "manualprimeflu";		// 	手动灌注清洗液
_CONST HTCMD_STR HTCmd3061 	= {CStr3061, 3061};

//_CONST char CStr3090[] 		= "quit";			// 测试退出
//_CONST HTCMD_STR HTCmd3090 	= {CStr3090, 3090};

_CONST HTCMD_STR * CmdString3[] = {
	 &HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0010, &HTCmd0011, &HTCmd0099, 
	 &HTCmd3001, &HTCmd3002, &HTCmd3003, &HTCmd3004, &HTCmd3005, &HTCmd3006, &HTCmd3007, 
	 &HTCmd3010, &HTCmd3011, &HTCmd3012, 
	 &HTCmd3013, &HTCmd3014, &HTCmd3015, &HTCmd3020, 
	 &HTCmd3050, &HTCmd3051, &HTCmd3052, &HTCmd3053, &HTCmd3054, &HTCmd3055, &HTCmd3056,
	 &HTCmd3060, &HTCmd3061, 0
};
// 维护
	// 位置调试命令
_CONST char CStr4002[] 		= "setneedleonmixpos";		// 取样针位置在混匀池边沿位置调整
_CONST HTCMD_STR HTCmd4002 	= {CStr4002, 4002};
_CONST char CStr4004[] 		= "setdropheight";		// 取样针高度调整
_CONST HTCMD_STR HTCmd4004 	= {CStr4004, 4004};
_CONST char CStr4005[] 		= "cardloadstartadjust";	// 装片小车起始位调整
_CONST HTCMD_STR HTCmd4005 	= {CStr4005, 4005};
_CONST char CStr4006[] 		= "cardloadendadjust";		// 装片小车终止位调整
_CONST HTCMD_STR HTCmd4006 	= {CStr4006, 4006};
_CONST char CStr4007[] 		= "cardunloadstartadjust";	// 卸片小车起始位调整
_CONST HTCMD_STR HTCmd4007 	= {CStr4007, 4007};
_CONST char CStr4008[] 		= "cardunloadendadjust";	// 卸片小车终止位调整
_CONST HTCMD_STR HTCmd4008 	= {CStr4008, 4008};
_CONST char CStr4009[] 		= "liquidphotoadjust";	// 
_CONST HTCMD_STR HTCmd4009 	= {CStr4009, 4009};
_CONST char CStr4010[] 		= "cardstorephotoadjust";	// 
_CONST HTCMD_STR HTCmd4010 	= {CStr4010, 4010};
// 测试灯源控制
_CONST char CStr4011[] 		= "setlamplum";	// 
_CONST HTCMD_STR HTCmd4011 	= {CStr4011, 4011};
_CONST char CStr4012[] 		= "getlamplum";	// 
_CONST HTCMD_STR HTCmd4012 	= {CStr4012, 4012};
_CONST char CStr4013[] 		= "turnonlamp";	// 
_CONST HTCMD_STR HTCmd4013 	= {CStr4013, 4013};
_CONST char CStr4014[] 		= "turnofflamp";	// 
_CONST HTCMD_STR HTCmd4014 	= {CStr4014, 4014};

_CONST char CStr4015[] 		= "calibvalue";		// 		仪器校准
_CONST HTCMD_STR HTCmd4015 	= {CStr4015, 4015};
_CONST char CStr4016[] 		= "calibtest";		// 		仪器校准运行, 称量流量
_CONST HTCMD_STR HTCmd4016 	= {CStr4016, 4016};
	// 机械测试命令
_CONST char CStr4020[] 		= "turnplatecheck";		// 转盘测试
_CONST HTCMD_STR HTCmd4020 	= {CStr4020, 4020};
_CONST char CStr4021[] 		= "needleturncheck";	// 取样臂旋转测试
_CONST HTCMD_STR HTCmd4021 	= {CStr4021, 4021};
_CONST char CStr4022[] 		= "needleupdowncheck";	// 取样针上下运行测试
_CONST HTCMD_STR HTCmd4022 	= {CStr4022, 4022};
_CONST char CStr4023[] 		= "cardstoremovecheck";	// 片仓小车移动测试
_CONST HTCMD_STR HTCmd4023 	= {CStr4023, 4023};
_CONST char CStr4024[] 		= "cardtakehookcheck";		// 卡片钩测试
_CONST HTCMD_STR HTCmd4024 	= {CStr4024, 4024};
_CONST char CStr4025[] 		= "cardloadcheck";		// 卡片装载小车测试
_CONST HTCMD_STR HTCmd4025 	= {CStr4025, 4025};
_CONST char CStr4026[] 		= "cardunloadcheck";	// 卡片卸载小车测试
_CONST HTCMD_STR HTCmd4026 	= {CStr4026, 4026};
_CONST char CStr4029[] 		= "diluentpumpcheck";	// 稀释液泵测试
_CONST HTCMD_STR HTCmd4029 	= {CStr4029, 4029};
_CONST char CStr4030[] 		= "leanerpumpcheck";	// 清洗液泵测试
_CONST HTCMD_STR HTCmd4030 	= {CStr4030, 4030};
_CONST char CStr4031[] 		= "effluentpumpcheck";	// 废液泵测试
_CONST HTCMD_STR HTCmd4031 	= {CStr4031, 4031};
_CONST char CStr4032[] 		= "sampsyringecheck";	// 取样注射器测试
_CONST HTCMD_STR HTCmd4032 	= {CStr4032, 4032};

_CONST char CStr4033[] 		= "liquidphotocheck";	// 
_CONST HTCMD_STR HTCmd4033 	= {CStr4033, 4033};
_CONST char CStr4034[] 		= "cardstorephotocheck";	// 
_CONST HTCMD_STR HTCmd4034 	= {CStr4034, 4034};

_CONST char CStr4035[] 		= "needleonmixsidecheck";	// 
_CONST HTCMD_STR HTCmd4035 	= {CStr4035, 4035};
_CONST char CStr4036[] 		= "dropheightcheck";	// 
_CONST HTCMD_STR HTCmd4036 	= {CStr4036, 4036};

_CONST char CStr4050[] 		= "diluentquantifytest";	// 稀释液定量测试
_CONST HTCMD_STR HTCmd4050 	= {CStr4050, 4050};
_CONST char CStr4051[] 		= "leanerquantifytest";		// 清洗液泵定量测试
_CONST HTCMD_STR HTCmd4051 	= {CStr4051, 4051};
_CONST char CStr4052[] 		= "sampquantifytest";		// 取样注射器定量测试
_CONST HTCMD_STR HTCmd4052 	= {CStr4052, 4052};
_CONST char CStr4055[] 		= "getsensor";		// 获取电机光藕状态
_CONST HTCMD_STR HTCmd4055 	= {CStr4055, 4055};
_CONST char CStr4056[] 		= "getallsensor";		// 获取电机光藕状态
_CONST HTCMD_STR HTCmd4056 	= {CStr4056, 4056};

	// 基本命令
_CONST char CStr4060[] 		= "motsetposition";		// 设置电机位置
_CONST HTCMD_STR HTCmd4060 	= {CStr4060, 4060};
_CONST char CStr4061[] 		= "motinitcheck";		// 电机位置初始化
_CONST HTCMD_STR HTCmd4061 	= {CStr4061, 4061};
_CONST char CStr4062[] 		= "motrun";				// 电机运行
_CONST HTCMD_STR HTCmd4062 	= {CStr4062, 4062};
_CONST char CStr4063[] 		= "motrunto";			// 电机运行到
_CONST HTCMD_STR HTCmd4063 	= {CStr4063, 4063};
_CONST char CStr4064[] 		= "motruntosite";		// 电机运行到指定位置
_CONST HTCMD_STR HTCmd4064 	= {CStr4064, 4064};
_CONST char CStr4065[] 		= "setmotparamter";		// 设置电机运行参数
_CONST HTCMD_STR HTCmd4065 	= {CStr4065, 4065};
_CONST char CStr4066[] 		= "setmotbaseparamter";		// 设置电机基本参数
_CONST HTCMD_STR HTCmd4066 	= {CStr4066, 4066};
_CONST char CStr4067[] 		= "setaddress";			// 设置模块地址
_CONST HTCMD_STR HTCmd4067 	= {CStr4067, 4067};
_CONST char CStr4068[] 		= "setevalve";			// 设置电磁阀
_CONST HTCMD_STR HTCmd4068 	= {CStr4068, 4068};
_CONST char CStr4069[] 		= "getliquidphosignal";	// 获取液路光耦信号
_CONST HTCMD_STR HTCmd4069 	= {CStr4069, 4069};
_CONST char CStr4070[] 		= "liquidphoadjust";	// 液路光耦校准
_CONST HTCMD_STR HTCmd4070 	= {CStr4070, 4070};
_CONST char CStr4071[] 		= "getstorestate";		// 获取片仓状态
_CONST HTCMD_STR HTCmd4071 	= {CStr4071, 4071};
_CONST char CStr4072[] 		= "cardstorecal";		// 片仓光耦调整
_CONST HTCMD_STR HTCmd4072 	= {CStr4072, 4072};
_CONST char CStr4073[] 		= "getstorephosignal";	// 获取片仓光耦信号
_CONST HTCMD_STR HTCmd4073 	= {CStr4073, 4073};
_CONST char CStr4074[] 		= "getliquiddetsignal";	// 获取液路探测信号
_CONST HTCMD_STR HTCmd4074 	= {CStr4074, 4074};

//_CONST char CStr4090[] 		= "quit";	// 子功能退出
//_CONST HTCMD_STR HTCmd4090 	= {CStr4090, 4090};


_CONST HTCMD_STR * CmdString4[] = {
 	&HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0010, &HTCmd0011, &HTCmd0099, 
	&HTCmd4002, &HTCmd4004, &HTCmd4005, &HTCmd4006, &HTCmd4007, &HTCmd4008, &HTCmd4009, &HTCmd4010,
	&HTCmd4011, &HTCmd4012, &HTCmd4013, &HTCmd4014, &HTCmd4015, &HTCmd4016,
	&HTCmd4020, &HTCmd4021, &HTCmd4022, &HTCmd4023, &HTCmd4024, &HTCmd4025, &HTCmd4026, &HTCmd4029, 
	&HTCmd4030, &HTCmd4031, &HTCmd4032, &HTCmd4033, &HTCmd4034, &HTCmd4035, &HTCmd4036, 
	&HTCmd4050, &HTCmd4051, &HTCmd4052, &HTCmd4055, &HTCmd4056,
	&HTCmd4060, &HTCmd4061, &HTCmd4062, &HTCmd4063, &HTCmd4064, &HTCmd4065, &HTCmd4066, &HTCmd4067, &HTCmd4068, &HTCmd4069, 
	&HTCmd4070, &HTCmd4071, &HTCmd4072, &HTCmd4073, &HTCmd4074, 0
};
*/
UICOMMAND_BUF UICmdBuf;
//HTCOMMAND_BUF HTCmdBuf;

extern unsigned char WorkProcessStep;		// 工作进程号
extern unsigned char EchoSW;

CONTROL_CMD ControlCmd;		// 存放解析好的命令, 调用模块直接读取此结构信息, 读取结束后将信息清除
/*
unsigned char HTCommandStrMatch(void){
	// 命令字符串匹配
	unsigned char i;
	_CONST HTCMD_STR ** CmdString;

	switch(WorkProcessStep){
		case 0:
		case 255:
			i = 0;
			while(CmdString0[i]){
				if(StringMatching(CmdString0[i]->pChar, HTCmdBuf.cmdStr)){
					ControlCmd.cmdIdx = CmdString0[i]->indx;		// 匹配成功，保存命令索引
					return 1;
					}
				else
					i++;
				}
			break;
		case 1:
			i = 0;
			while(CmdString1[i]){
				if(StringMatching(CmdString0[i]->pChar, HTCmdBuf.cmdStr)){
					ControlCmd.cmdIdx = CmdString0[i]->indx;		// 匹配成功，保存命令索引
					return 1;
					}
				else
					i++;
				}
			break;
		case 2:
			i = 0;
			while(CmdString2[i]){
				if(StringMatching(CmdString0[i]->pChar, HTCmdBuf.cmdStr)){
					ControlCmd.cmdIdx = CmdString0[i]->indx;		// 匹配成功，保存命令索引
					return 1;
					}
				else
					i++;
				}
			break;
		case 3:
			i = 0;
			while(CmdString3[i]){
				if(StringMatching(CmdString3[i]->pChar, HTCmdBuf.cmdStr)){
					ControlCmd.cmdIdx = CmdString3[i]->indx;		// 匹配成功，保存命令索引
					return 1;
					}
				else
					i++;
				}
			break;
		case 4:
			i = 0;
			while(CmdString4[i]){
				if(StringMatching(CmdString4[i]->pChar, HTCmdBuf.cmdStr)){
					ControlCmd.cmdIdx = CmdString4[i]->indx;		// 匹配成功，保存命令索引
					return 1;
					}
				else
					i++;
				}
			break;
		default:
			i = 0;
			while(CmdString0[i]){
				if(StringMatching(CmdString0[i]->pChar, HTCmdBuf.cmdStr)){
					ControlCmd.cmdIdx = CmdString0[i]->indx;		// 匹配成功，保存命令索引
					return 1;
					}
				else
					i++;
				}
			break;
		}
	return 0;
}

unsigned char HTCommandPammterConvert(void){
	// 命令参数转换
	unsigned char i;
	i = HTCmdBuf.itemPnt;
	if(i==0)
		return 0;
	ControlCmd.pam[i-1] = StringToInt(HTCmdBuf.pamStr[i-1]);
	return 1;
}
void CommandClear(void){
	unsigned char i;
	unsigned char * pChar;
	pChar = (unsigned char *)(&HTCmdBuf);
	for(i=0; i<sizeof(HTCOMMAND_BUF); i++){
		*pChar = 0;
		pChar ++;
		}
	pChar = (unsigned char *)(&ControlCmd);
	for(i=0; i<sizeof(CONTROL_CMD); i++){
		*pChar = 0;
		pChar ++;
		}
}

unsigned char HTCommandExplain(unsigned char c)
{
	// 超级终端命令解释,接收并回显正确的字符
	unsigned char i;
	unsigned char * pChar;
	if(ControlCmd.cmdState == 2)
		return 0;
	if(ControlCmd.cmdState == 0){	
		CommandClear();
		}
	// 字符接收和处理
	switch(HTCmdBuf.itemPnt)		// 根据参数接收进度做不同处理
	{
		case 0:		// 当前接收命令字符串
			if(c>='A' && c<='Z')
				c += 0x20;
			if((c>='a' && c<='z') || (c>='0' && c<='9'))	// 命令字符接收
			{
				ControlCmd.cmdState = 1;
				if(HTCmdBuf.charPnt < 33)
				{
					HTCmdBuf.cmdStr[HTCmdBuf.charPnt] = c;
					HTCmdBuf.charPnt ++;
					uart0SendChar(c);
				}
			}
			else if(c == ':')		// 命令字符串结束
			{
				HTCmdBuf.cmdStr[HTCmdBuf.charPnt] = 0;		// 字符串结束标识
				HTCmdBuf.charPnt = 0;
				HTCmdBuf.itemPnt = 1;			// 进入参数接收
				uart0SendChar(':');
				// 命令字符串匹配，匹配成功返显参数格式，错误则清除
				if(HTCommandStrMatch()==0)
				{
					// 命令匹配错误
					uart0SendChar(0x0d);
					uart0SendChar(0x0a);
					HTCmdBuf.itemPnt = 0;
					HTCmdBuf.charPnt = 0;
					//ControlCmd.cmdState == 0;
					ControlCmd.cmdState = 0;
					return 0;
				}
			}
			else if(c == 0x0d )		// 命令接收结束
			{
				if(HTCmdBuf.charPnt != 0)
				{
					HTCmdBuf.cmdStr[HTCmdBuf.charPnt] = 0;		// 字符串结束标识
					if(HTCommandStrMatch())	// 命令匹配成功
					{
						HTCmdBuf.charPnt = 0;
						ControlCmd.pamLen = 0;		// 命令不带参数
						ControlCmd.cmdState = 2;		// 设置接收到命令标识
					}	
					else	// 命令匹配错误
					{
						HTCmdBuf.itemPnt = 0;
						HTCmdBuf.charPnt = 0;
						//ControlCmd.cmdState == 0;
						ControlCmd.cmdState = 0;
					}
				}
				else
				{
					HTCmdBuf.itemPnt = 0;
					HTCmdBuf.charPnt = 0;
					//ControlCmd.cmdState == 0;
					ControlCmd.cmdState = 0;
				}
				uart0SendChar(0x0d);
				uart0SendChar(0x0a);
				return 0;
			}
			else if(c == 0x08){		// 退格
				if(HTCmdBuf.charPnt != 0)
				{
					HTCmdBuf.charPnt --;
					HTCmdBuf.cmdStr[HTCmdBuf.charPnt] = 0;
					uart0SendChar(0x08);
					uart0SendChar(' ');
					uart0SendChar(0x08);
				}
				else
				{
					HTCmdBuf.itemPnt = 0;
					HTCmdBuf.charPnt = 0;
					//ControlCmd.cmdState == 0;
					ControlCmd.cmdState = 0;
					uart0SendChar(0x0d);
					uart0SendChar(0x0a);
					return 0;
				}
			}
			break;
		case 1:		// 参数0
		case 2:
		case 3:
		case 4:
		case 5:
			if(c == '-')
			{
				if(HTCmdBuf.charPnt == 0)
				{
					HTCmdBuf.pamStr[HTCmdBuf.itemPnt-1][0] = c;
					HTCmdBuf.charPnt ++;
					uart0SendChar(c);
				}
			}
			else if((c>='0' && c<='9'))
			{
				if(HTCmdBuf.charPnt < 8)
				{
					HTCmdBuf.pamStr[HTCmdBuf.itemPnt-1][HTCmdBuf.charPnt] = c;
					HTCmdBuf.charPnt ++;
					uart0SendChar(c);
				}
			}
			else if(c == ',')		// 参数分隔符
			{
				HTCmdBuf.pamStr[HTCmdBuf.itemPnt-1][HTCmdBuf.charPnt] = 0;
				if(HTCmdBuf.itemPnt<6)
				{
					uart0SendChar(',');
					if(HTCommandPammterConvert())
					{
						HTCmdBuf.itemPnt ++;
						HTCmdBuf.charPnt = 0;
					}
				}
			}
			else if(c == 0x0d)
			{
				HTCmdBuf.pamStr[HTCmdBuf.itemPnt-1][HTCmdBuf.charPnt] = 0;
				HTCmdBuf.charPnt = 0;
				if(HTCommandPammterConvert())
				{
					HTCmdBuf.charPnt = 0;
					ControlCmd.pamLen = HTCmdBuf.itemPnt;
					ControlCmd.cmdState = 2;			// 设置命令接收完成标识
				}
				else		// 接收错误删除命令
				{
					HTCmdBuf.itemPnt = 0;
					HTCmdBuf.charPnt = 0;
					//ControlCmd.cmdState == 0;
					ControlCmd.cmdState = 0;
				}
				uart0SendChar(0x0d);
				uart0SendChar(0x0a);
				return 0;
			}
			else if(c == 0x08)			// 退格
			{
				if(HTCmdBuf.charPnt != 0)
				{
					HTCmdBuf.charPnt --;
					HTCmdBuf.pamStr[HTCmdBuf.itemPnt-1][HTCmdBuf.charPnt] = 0;
					uart0SendChar(0x08);
					uart0SendChar(' ');
					uart0SendChar(0x08);
				}
				else			// 退回到上一个参数
				{
					if(HTCmdBuf.itemPnt > 1)
					{
						HTCmdBuf.itemPnt --;
						for(i=0; i<10; i++)
						{
							if(HTCmdBuf.pamStr[HTCmdBuf.itemPnt-1][i] == 0)
							{
								HTCmdBuf.charPnt = i;
								break;
							}
						}
						uart0SendChar(0x08);
						uart0SendChar(' ');
						uart0SendChar(0x08);
					}
					else		// 删除命令
					{
						uart0SendChar(0x0d);
						uart0SendChar(0x0a);
						HTCmdBuf.itemPnt = 0;
						HTCmdBuf.charPnt = 0;
						//ControlCmd.cmdState == 0;
						ControlCmd.cmdState = 0;
						return 0;
					}
				}
			}
			break;
		default:
			break;
		}
	return 1;
}
*/

void CommandClear(void){
	unsigned char i;
	unsigned char * pChar;
	pChar = (unsigned char *)(&ControlCmd);
	for(i=0; i<sizeof(CONTROL_CMD); i++){
		*pChar = 0;
		pChar ++;
		}
}


/*************************************************************************************************
typedef struct _UICOMMAND_BUF{
	unsigned char itemPnt;			// 当前接收的项目号
	unsigned char charPnt;			// 接收指针
	unsigned char cmdStr[10];		// 命令名称字符串
}UICOMMAND_BUF;
/************************************ 用户界面命令解释 *******************************************/
extern unsigned char checkFlag;
extern unsigned char UartReceiveBuf[60];
unsigned char UICommandExplain(unsigned char c){
	unsigned char i;
	unsigned char * pChar;
	unsigned int cmd;
	static unsigned char checkSum = 0;
	if(ControlCmd.cmdState == 2)
		return 0;
	if(ControlCmd.cmdState == 0)		// 清零
	{
		CommandClear();
	}
	switch(UICmdBuf.itemPnt)
	{
		case 0:
			if(c == '#')		// 命令开始标志
			{
				UICmdBuf.charPnt = 0;
				ControlCmd.cmdState = 1;
				if(EchoSW)
				{
					if(checkFlag)
					{
						checkFlag = 0;
						uart0SendChar(0x02);
						uart0SendChar(c);
						checkSum += c;			
					}
					else
						uart0SendChar(c);
				}
			}
			else if(c >= '0' && c <= '9')
			{
				if(UICmdBuf.charPnt < 4)
				{
					UICmdBuf.cmdStr[UICmdBuf.charPnt] = c;
					UICmdBuf.charPnt ++;
					if(EchoSW)
					{
						if(checkFlag)
						{
							checkFlag = 0;
							uart0SendChar(c);
							checkSum += c;
						}
						else
							uart0SendChar(c);
					}
				}
			}
			else if(c == '$')	// 出现参数符，结束命令进入参数接收
			{
				if(UICmdBuf.charPnt != 0)
				{
					UICmdBuf.cmdStr[UICmdBuf.charPnt] = 0;
					ControlCmd.cmdIdx = StringToInt(UICmdBuf.cmdStr);
					if(checkFlag)
					{
						cmd = StringToInt2(&UartReceiveBuf[1]);
						if(ControlCmd.cmdIdx == cmd)
							ControlCmd.cmdState = 1;
					}
				}
				else
				{
					ControlCmd.cmdIdx = 0;
				}
				if(EchoSW)
				{
					if(checkFlag)
					{
						checkFlag = 0;
						uart0SendChar('$');
						checkSum += '$';
					}
					else
					{
						uart0SendChar(' ');
						uart0SendChar('$');
					}
				}
				UICmdBuf.itemPnt ++;
				UICmdBuf.charPnt = 0;
				ControlCmd.pamLen = 0;
			}
			else if(c == 0x0d)// 回车号，命令结束
			{
				if(UICmdBuf.charPnt != 0)
				{
					UICmdBuf.cmdStr[UICmdBuf.charPnt] = 0;
					ControlCmd.cmdIdx = StringToInt(UICmdBuf.cmdStr);
					if(checkFlag)
					{
						cmd = StringToInt(&UartReceiveBuf[1]);
						if(ControlCmd.cmdIdx == cmd)
							ControlCmd.cmdState = 2;
						else
						{
							ControlCmd.cmdState = 0;
							uart0SendChar(0X04);		// EOT
							return 0;
						}
					}
					else
						ControlCmd.cmdState = 2;
				}
				else
				{
					ControlCmd.cmdState = 0;
				}
				ControlCmd.pamLen = 0;
				UICmdBuf.itemPnt = 0;
				UICmdBuf.charPnt = 0;
				if(EchoSW)
				{
					if(checkFlag)
					{
						checkFlag = 0;
						uart0SendChar(0x0d);
						uart0SendChar(0X03);
						checkSum += 0x0d;
						uart0SendChar((checkSum & 0x0F) + 'A');
						uart0SendChar(((checkSum & 0xF0) >> 4) + 'A');
						uart0SendChar(0x0d);
						uart0SendChar(0x0a);
						checkSum = 0;
					}
					else
					{
						uart0SendChar(0x0d);
						uart0SendChar(0x0a);
					}
				}
				return 0;
			}
			else if(c == 0x08)		// 退格
			{
				if(UICmdBuf.charPnt != 0)
				{
					UICmdBuf.charPnt--;
					UICmdBuf.cmdStr[UICmdBuf.charPnt] = 0;
					if(EchoSW)
					{
						uart0SendChar(0x08);
						uart0SendChar(' ');
						uart0SendChar(0x08);
					}
				}
				else		// 清除命令
				{
					ControlCmd.cmdState = 0;
					ControlCmd.pamLen = 0;
					UICmdBuf.itemPnt = 0;
					UICmdBuf.charPnt = 0;
					if(EchoSW)
					{
						uart0SendChar(0x0d);
						uart0SendChar(0x0a);
					}
					return 0;
				}
			}
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			if(c>='0' && c<= '9')
			{
				if(UICmdBuf.charPnt<5)
				{
					UICmdBuf.cmdStr[UICmdBuf.charPnt] = c;
					UICmdBuf.charPnt ++;
					if(EchoSW)
					{
						if(checkFlag)
						{
							checkFlag = 0;
							uart0SendChar(c);
							checkSum += c;
						}
						else
						{
							uart0SendChar(c);
						}
					}
				}
			}
			else if(c == '$')
			{
				if(UICmdBuf.charPnt != 0)
				{
					UICmdBuf.cmdStr[UICmdBuf.charPnt] = 0;
					ControlCmd.pam[ControlCmd.pamLen] = StringToInt(UICmdBuf.cmdStr);
					ControlCmd.pamLen ++;
					UICmdBuf.itemPnt ++;
					UICmdBuf.charPnt = 0;
					if(EchoSW)
					{
						if(checkFlag)
						{
							checkFlag = 0;
							uart0SendChar('$');
							checkSum += '$';
						}
						else
						{
							uart0SendChar(' ');
							uart0SendChar('$');
						}
					}
				}
				else
				{
					ControlCmd.pam[ControlCmd.pamLen] = 0;
					ControlCmd.pamLen ++;
					UICmdBuf.itemPnt ++;
					UICmdBuf.charPnt = 0;
					if(EchoSW)
					{
						if(checkFlag)
						{
							checkFlag = 0;
							uart0SendChar('$');
							checkSum += '$';
						}
						else
						{
							uart0SendChar(' ');
							uart0SendChar('$');
						}
					}
				}
			}
			else if(c == '-')	// 负号
			{
				if(UICmdBuf.charPnt==0)
				{
					UICmdBuf.cmdStr[0] = '-';
					UICmdBuf.charPnt ++;
					if(EchoSW)
						uart0SendChar(c);
				}
			}
			else if(c == 0x0d)	// 回车号，命令结束
			{
				if(UICmdBuf.charPnt != 0)
				{
					UICmdBuf.cmdStr[UICmdBuf.charPnt] = 0;
					ControlCmd.pam[ControlCmd.pamLen] = StringToInt(UICmdBuf.cmdStr);
					ControlCmd.pamLen ++;
				}
				if(checkFlag)
				{
					if(ControlCmd.cmdState == 1)
						ControlCmd.cmdState = 2;
					else
						ControlCmd.cmdState = 0;
				}
				else
					ControlCmd.cmdState = 2;
					
				UICmdBuf.itemPnt = 0;
				UICmdBuf.charPnt = 0;
				if(EchoSW)
				{
					if(checkFlag)
					{
						checkFlag = 0;
						uart0SendChar(0x0d);
						uart0SendChar(0X03);
						checkSum += 0x0d;
						uart0SendChar((checkSum & 0x0F) + 'A');
						uart0SendChar(((checkSum & 0xF0) >> 4) + 'A');
						uart0SendChar(0x0D);
						uart0SendChar(0x0a);
						checkSum = 0;
					}
					else
					{
						uart0SendChar(0x0D);
						uart0SendChar(0x0a);
					}
				}
				return 0;
			}
			else if(c == 0x08){		// 退格
				if(UICmdBuf.charPnt != 0)
				{
					UICmdBuf.charPnt--;
					UICmdBuf.cmdStr[UICmdBuf.charPnt] = 0;
					if(EchoSW)
					{
						uart0SendChar(0x08);
						uart0SendChar(' ');
						uart0SendChar(0x08);
					}
				}
				else		// 清除命令
				{
					ControlCmd.cmdState = 0;
					ControlCmd.pamLen = 0;
					UICmdBuf.itemPnt = 0;
					UICmdBuf.charPnt = 0;
					if(EchoSW)
					{
						uart0SendChar(0x0d);
						uart0SendChar(0x0a);
					}
					return 0;
				}
			}
			break;
		}
	if(UICmdBuf.itemPnt > 8)
	{
		UICmdBuf.itemPnt = 8;
	}
	if(ControlCmd.pamLen >8)
	{
		ControlCmd.pamLen = 8;
	}
	return 1;
}

/*
void CommandExplain(unsigned char c)
{
	static unsigned char receiveSW = 0;
	unsigned char i;
		
	if(receiveSW == 0)
	{
		if(c == '#')
		{
			receiveSW = 1;
		}
		else 
		{
			receiveSW = 2;
		}
	}
	switch(receiveSW)
	{
		case 0:
			break;
		case 1:
			i = UICommandExplain(c);
			if(i == 0)
				receiveSW = 0;
			break;
		case 2:
			i = HTCommandExplain(c);
			if(i == 0)
				receiveSW = 0;
			break;
	}
}
*/

void CommandExplain(unsigned char c)
{
	static unsigned char receiveSW = 0;
	unsigned char i;
		
	if(receiveSW == 0)
	{
		if(c == '#')
		{
			receiveSW = 1;
		}
	}
	switch(receiveSW)
	{
		case 0:
			break;
		case 1:
			i = UICommandExplain(c);
			if(i == 0)
				receiveSW = 0;
			break;
	}
}