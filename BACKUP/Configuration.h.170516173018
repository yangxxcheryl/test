#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#define DEBUG

#define DILUTE_TUBE	16	// 稀释液泵管定义 #14 or #16

#define UartSendLong	// 可输出大于65535的整数

#define LoadCheck		// 上下片光耦检测功能

//#define _FluidPumMix	// 此定义采用清洗液泵抽打混匀

//#define Puncture		// 穿刺功能	

#ifndef Puncture

	#define HalfCircle						// 更换同步轮大小 同步轮大小一半,穿刺模式下无效
	#define _POS_SAMPTURN_SAMP		980		// 103mm/0.105=980
	#define _POS_MIX_TOP			360		
#else

	#define _POS_SAMPTURN_SAMP		1024	// 103mm/0.105=980
	#define _POS_MIX_TOP			260		// 360
	
#endif

#define  NeedleChannel		0			// 吸样针 AD2
#define  LoadChannel		1			// 上片光耦 AD10
#define  UnloadChannel		2			// 下片光耦 AD11

#define  CardLocationAD		900			// 上下片光耦参考值

// 测试队列配置
#define RING_QUEUE_NUM		30
#define TEST_QUEUE_NUM		40
#define TEST_CYCLE_TIME		15
#define TEST_READY_TIME		10		// 检测读数时间

#define _POS_SAMP_HOME			0		// 取样臂起始位，也是滴样到测试卡位置

#define _POS_NEEDLE_ON_MIXCENTRE	343-20	// 36mm/0.105=343
#define _POS_NEEDLE_ON_MIXSIDE		371-20	// 39mm/0.105=371


// 取样针高度位置
#define _POS_NEEDLE_HOME		0		// 取样针在起始位位置
#define _POS_NEEDLE_SAMP		5		// 取样针在吸样位置	



#ifndef HalfCircle

	#define _POS_MIX_BUTTOM			(_POS_MIX_TOP+540) * 2	
	#define _POS_MIX_NEEDLE			 _POS_MIX_BUTTOM - 40  
	#define _POS_MIX_CAL_START		(_POS_MIX_TOP+500) * 2	
	#define _POS_CARD_DROP			(_POS_MIX_TOP+20) * 2
	#define _POS_CARD_MIX			(_POS_MIX_TOP+40) * 2	
	#define _POS_SAMP_DOWN			3800//3920	// 取样针取样位置下降高度
	
#else

	#define _POS_MIX_BUTTOM			_POS_MIX_TOP+540	
	#define _POS_MIX_NEEDLE			_POS_MIX_BUTTOM-20  // 混匀时针高度  // 一步0.05mm
	#define _POS_MIX_CAL_START		_POS_MIX_TOP+500	// 定标液起始高度
	#define _POS_CARD_DROP			_POS_MIX_TOP+20		// 滴样高度  380
	#define _POS_CARD_MIX			_POS_MIX_TOP+40		// 混匀高度  400
	#define _POS_SAMP_DOWN			1960	// 取样针取样位置下降高度
	
#endif

// 干片插入小车位置编号
#define _POS_CARDLOAD_HOME		0		// 干片插入小车在起始位
#define _POS_CARDLOAD_RING		1600	// 干片插入小车在转盘位  1168
#define _POS_CARDLOAD_DROP		74		// 干片在滴样位置    6 / 0.08012 = 74.88
#define _POS_CARDLOAD_MIX		141		// 干片在混匀抽打位置 10.5 / 0.08012 = 131
// 卸片小车位置编号
#define _POS_UNLOAD_HOME		0		// 卸片小车起始位置
#define _POS_UNLOAD_OUT			1600	// 卸片小车推出卸片行程  812
// 注射泵消隙行程
//#define _SAMP_PUMP_AIR_ISOLATE	150		// 取样空气隔离段	
#define _SAMP_PUMP_AIR_ISOLATE	300		// 取样空气隔离段	
#define _SAMP_PUMP_INTERVAL		(40+20)		// 取样泵间隙	
// 条码扫描位置
#define _POS_CARD_SCANF			305


// 电机电流设置
#define CURRENT_TURN_PLATE		1		// 转盘电机电流
#define CURRENT_SAMP_TRUN		4		// 取样臂旋转电机电流
#define CURRENT_SAMP_NEEDLE		3		// 取样针升降电机电流
#define CURRENT_CARD_LOAD		4		// 试剂片装载电机电流
#define CURRENT_CARD_UNLOAD		4		// 试剂片卸载电机电流
#define CURRENT_STORE_MOVE		10		// 片仓电机电流
#define CURRENT_DILUENT			4		// 稀释液泵电机电流
#define CURRENT_FLUID			4		// 清洗液泵电机电流
#define CURRENT_EFFLUENT		4		// 废液泵电机电流
#define CURRENT_SAMP_PUMP		6		// 取样泵电机电流

#endif