#ifndef _COMMON_H
#define _COMMON_H

#ifndef _CONST
#define _CONST __flash
#endif

#include "Configuration.h"

// Command String
typedef struct _CONTROL_CMD{
	unsigned char cmdState;			// 命令状态	0:空， 1:接收中，2:接收完毕待处理
	unsigned int cmdIdx;			// 命令号
	unsigned int pam[10];			// 参数列表
	unsigned char pamLen;			// 参数个数
}CONTROL_CMD;

 extern CONTROL_CMD ControlCmd;
// Command String [End]

#define TEST_STA_READY		0
#define TEST_STA_LOAD		1
#define TEST_STA_TEST		2
#define TEST_STA_GETOUT		3
#define TEST_STA_DONE		4
#define TEST_STA_TIMEOVER	5

#define UNLOAD_STA_READY	0
#define UNLOAD_STA_TAKEOUT	1
#define UNLOAD_STA_OUTSIDE	2
#define UNLOAD_STA_GOHOME	3

#define STA_NEW_SAMP_NO			0	// 无新标本
#define STA_NEW_SAMP_SUCTION	1	// 吸取标本
#define STA_NEW_SAMP_DILUTE		2	// 稀释标本
#define STA_NEW_SAMP_INJECT		3	// 注入标本到测试卡
#define STA_NEW_SAMP_DONE		4	// 新测试准备好

#define POS_OFFSET_LOAD			0
#define POS_OFFSET_TESTA		11
#define POS_OFFSET_TESTB		19
#define POS_OFFSET_UNLOAD		25

// 位置编号定义




typedef struct _SAMP_INFO{		// 
	unsigned char	isUsed;
	unsigned long	testSerial;			// 测试序列号
	unsigned char	projectType;		// 项目类型
	unsigned char	cardStoreNum;		// 片仓号
	unsigned char	sampDiluteMult;		// 标本稀释倍率
	unsigned char	readType;			// 结果读取类型
	unsigned int	testTime0;
	unsigned int	testTime1;
//	char			projectBarcode[20];	// 项目类型条码
}SAMP_INFO;

typedef struct _NEW_TEST{
	unsigned char state;		// 0:无新的测试，1吸样阶段，2稀释阶段，3滴样阶段，4等待传送
	SAMP_INFO sampInfo;
}NEW_TEST;


// 转盘装片队列
typedef struct _RING_QUEUE{
	SAMP_INFO	sampInfo[RING_QUEUE_NUM];	// 转盘队列，数组下标对应转盘位置序号
	unsigned char flag[RING_QUEUE_NUM];		// 标记:	0空，1干片在转盘上，2干片在检测A，3干片在检测B，4干片在卸片推车上
	unsigned char prevNum;					// 前一个插入的位置序号，用于寻找下次加入位置
}RING_QUEUE;

// 测试队列
typedef struct _TEST_QUEUE{
	// 插入式队列，根据插入条件计算插入位置，当第一个队列元素取出后，后面的元素都要往前移动一格
	// 以测试时间为条件计算新元素插入位置
	unsigned char ringNum[TEST_QUEUE_NUM];		// 测试编号队列，对应转盘的编号0xff NULL
	signed long testTime[TEST_QUEUE_NUM];		// 对应测试编号的测试时间队列
//	unsigned char pCur;						// 随时间移动的指针	需要根据时间实时更新
//	unsigned int curBasetime;				// 当前基时间	
	unsigned char state;					// 测试状态	0空闲，1干片进入检测，2干片检测完等待取出	需要根据时间实时更新
}TEST_QUEUE;

// 卸片队列
typedef struct _UNLOAD_QUEUE{
	unsigned char	ringNum[RING_QUEUE_NUM];		// 待测测试条所在转盘位置
	signed long		unloadTime[RING_QUEUE_NUM];
	unsigned char	pTop;
	unsigned char 	state;			// 
}UNLOAD_QUEUE;

// output display String
extern _CONST char strVersion[];
extern _CONST char strM0100[];
extern _CONST char strM0101[];
extern _CONST char strM0102[];
extern _CONST char strM0103[];
extern _CONST char strM0104[];
extern _CONST char strM0105[];
extern _CONST char strM0106[];
extern _CONST char strM0110[];
extern _CONST char strM0111[];
extern _CONST char strM0112[];
extern _CONST char strM0199[];
extern _CONST char strM0200[];
extern _CONST char strE0901[];
extern _CONST char strE0902[];
extern _CONST char strE0903[];
extern _CONST char strE0910[];

extern _CONST char strM1101[];

extern _CONST char strE1901[];
extern _CONST char strE1910[];
extern _CONST char strE1911[];
extern _CONST char strE1912[];
extern _CONST char strE1913[];
extern _CONST char strE1914[];
extern _CONST char strE1915[];
extern _CONST char strE1916[];
extern _CONST char strE1917[];
extern _CONST char strE1918[];
extern _CONST char strE1919[];
extern _CONST char strE1920[];
extern _CONST char strE1921[];

extern _CONST char strE1930[];
extern _CONST char strE1931[];
extern _CONST char strE1932[];
extern _CONST char strE1933[];
extern _CONST char strE1934[];
extern _CONST char strE1935[];
extern _CONST char strE1936[];
extern _CONST char strE1937[];
extern _CONST char strE1938[];
extern _CONST char strE1939[];
extern _CONST char strE1940[];
extern _CONST char strE1941[];

extern _CONST char strM2100[];
extern _CONST char strM2101[];
extern _CONST char strM2102[];
extern _CONST char strM2103[];
extern _CONST char strM2104[];
extern _CONST char strM2105[];
extern _CONST char strM2106[];
extern _CONST char strM2107[];
extern _CONST char strM2108[];
extern _CONST char strM2109[];
extern _CONST char strM2110[];
extern _CONST char strM2111[];
extern _CONST char strM2112[];
extern _CONST char strM2113[];
extern _CONST char strM2114[];
extern _CONST char strM2115[];
extern _CONST char strM2116[];
extern _CONST char strM2117[];
extern _CONST char strM2118[];

extern _CONST char strM2120[];

extern _CONST char strM2150[];

extern _CONST char strE2505[];
extern _CONST char strE2506[];

extern _CONST char strE2901[];
extern _CONST char strE2902[];
extern _CONST char strE2904[];
extern _CONST char strE2905[];
extern _CONST char strE2906[];
//extern _CONST char strE2907[];
extern _CONST char strE2908[];

//extern _CONST char strE2910[];
extern _CONST char strE2911[];
extern _CONST char strE2912[];
extern _CONST char strE2920[];

extern _CONST char strE2950[];
extern _CONST char strE2951[];
extern _CONST char strE2952[];
extern _CONST char strE2953[];
extern _CONST char strE2954[];
extern _CONST char strE2955[];
extern _CONST char strE2956[];

extern _CONST char strM3100[];
extern _CONST char strM3101[];
extern _CONST char strM3102[];
extern _CONST char strM3103[];
extern _CONST char strM3104[];
extern _CONST char strM3105[];
extern _CONST char strM3106[];
extern _CONST char strM3107[];
extern _CONST char strM3108[];
extern _CONST char strM3109[];

extern _CONST char strM3110[];
extern _CONST char strM3111[];
extern _CONST char strM3112[];
extern _CONST char strM3113[];
extern _CONST char strM3114[];
extern _CONST char strM3115[];
extern _CONST char strM3116[];
extern _CONST char strM3117[];
extern _CONST char strM3118[];
extern _CONST char strM3119[];

extern _CONST char strM3120[];
extern _CONST char strM3121[];
extern _CONST char strM3122[];
extern _CONST char strM3123[];
extern _CONST char strM3124[];
extern _CONST char strM3125[];
extern _CONST char strM3126[];
extern _CONST char strM3127[];
extern _CONST char strM3128[];
extern _CONST char strM3129[];

extern _CONST char strM3130[];
extern _CONST char strM3131[];
extern _CONST char strM3132[];
extern _CONST char strM3133[];
extern _CONST char strM3134[];
extern _CONST char strM3135[];
extern _CONST char strM3136[];
extern _CONST char strM3137[];
extern _CONST char strM3138[];
extern _CONST char strM3139[];

extern _CONST char strM3140[];
extern _CONST char strM3141[];
extern _CONST char strM3142[];
extern _CONST char strM3143[];
extern _CONST char strM3144[];
extern _CONST char strM3145[];
extern _CONST char strM3146[];
extern _CONST char strM3147[];
extern _CONST char strM3148[];
extern _CONST char strM3149[];

extern _CONST char strM3150[];
extern _CONST char strM3151[];
extern _CONST char strM3152[];
extern _CONST char strM3153[];
extern _CONST char strM3154[];
extern _CONST char strM3155[];
extern _CONST char strM3156[];
extern _CONST char strM3157[];
extern _CONST char strM3158[];
extern _CONST char strM3159[];

extern _CONST char strM3160[];
extern _CONST char strM3161[];
extern _CONST char strM3162[];
extern _CONST char strM3163[];


extern _CONST char strM3166[];
extern _CONST char strM3167[];
extern _CONST char strM3168[];
extern _CONST char strM3169[];
extern _CONST char strM3170[];
extern _CONST char strM3171[];
extern _CONST char strM3172[];
extern _CONST char strM3173[];
extern _CONST char strM3174[];
extern _CONST char strM3175[];

extern _CONST char strM3188[];

extern _CONST char strM3190[];
extern _CONST char strM3191[];

extern _CONST char strM3194[];
extern _CONST char strM3195[];

extern _CONST char strM3199[];

extern _CONST char strM3200[];
extern _CONST char strM3201[];

extern _CONST char strM3217[];
extern _CONST char strM3218[];

extern _CONST char strM3225[];
extern _CONST char strM3226[];
extern _CONST char strM3227[];


extern _CONST char strM3333[];

extern _CONST char strE3902[];
extern _CONST char strE3904[];
extern _CONST char strE3905[];
extern _CONST char strE3906[];

extern _CONST char strE3911[];
extern _CONST char strE3912[];


extern _CONST char strE3933[];
extern _CONST char strE3934[];
extern _CONST char strE3935[];
extern _CONST char strE3936[];


extern _CONST char strE3925[];
extern _CONST char strE3926[];
extern _CONST char strE3927[];
extern _CONST char strE3928[];

extern _CONST char strM4101[];
extern _CONST char strM4102[];
extern _CONST char strM4104[];
extern _CONST char strM4105[];
extern _CONST char strM4106[];
extern _CONST char strM4107[];
extern _CONST char strM4108[];
extern _CONST char strM4109[];
extern _CONST char strM4110[];
extern _CONST char strM4111[];
extern _CONST char strM4112[];
extern _CONST char strM4113[];
extern _CONST char strM4114[];
extern _CONST char strM4115[];
extern _CONST char strM4116[];
extern _CONST char strM4117[];
extern _CONST char strM4118[];
extern _CONST char strM4119[];
extern _CONST char strM4120[];
extern _CONST char strM4120[];
extern _CONST char strM4121[];
extern _CONST char strM4122[];
extern _CONST char strM4123[];
extern _CONST char strM4124[];
extern _CONST char strM4125[];
extern _CONST char strM4126[];
extern _CONST char strM4127[];
extern _CONST char strM4128[];
extern _CONST char strM4129[];
extern _CONST char strM4130[];
extern _CONST char strM4131[];
extern _CONST char strM4132[];
extern _CONST char strM4133[];
extern _CONST char strM4134[];
extern _CONST char strM4135[];
extern _CONST char strM4136[];
extern _CONST char strM4137[];
extern _CONST char strM4138[];

extern _CONST char strM4150[];
extern _CONST char strM4151[];
extern _CONST char strM4152[];
extern _CONST char strM4153[];
extern _CONST char strM4154[];

extern _CONST char strM4174[];
extern _CONST char strM4175[];
extern _CONST char strM4176[];

extern _CONST char strM4199[];

extern _CONST char strM4201[];
extern _CONST char strM4202[];

extern _CONST char strE4901[];
extern _CONST char strE4902[];
extern _CONST char strE4903[];
extern _CONST char strE4904[];
extern _CONST char strE4905[];
extern _CONST char strE4906[];
extern _CONST char strE4907[];
extern _CONST char strE4908[];
extern _CONST char strE4909[];
extern _CONST char strE4910[];

extern _CONST char strE4920[];
extern _CONST char strE4921[];
extern _CONST char strE4922[];
extern _CONST char strE4923[];
extern _CONST char strE4924[];
extern _CONST char strE4925[];
extern _CONST char strE4926[];
extern _CONST char strE4927[];
extern _CONST char strE4928[];
extern _CONST char strE4929[];
extern _CONST char strE4930[];
extern _CONST char strE4931[];
extern _CONST char strE4932[];
// output display String [End]

#endif
