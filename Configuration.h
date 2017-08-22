#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#define DEBUG

#define DILUTE_TUBE	16	// ϡ��Һ�ùܶ��� #14 or #16

#define UartSendLong	// ���������65535������

#define LoadCheck		// ����Ƭ�����⹦��

//#define _FluidPumMix	// �˶��������ϴҺ�ó�����

//#define Puncture		// ���̹���	

#ifndef Puncture

	#define HalfCircle						// ����ͬ���ִ�С ͬ���ִ�Сһ��,����ģʽ����Ч
	#define _POS_SAMPTURN_SAMP		980		// 103mm/0.105=980
	#define _POS_MIX_TOP			360		
#else

	#define _POS_SAMPTURN_SAMP		1024	// 103mm/0.105=980
	#define _POS_MIX_TOP			260		// 360
	
#endif

#define  NeedleChannel		0			// ������ AD2
#define  LoadChannel		1			// ��Ƭ���� AD10
#define  UnloadChannel		2			// ��Ƭ���� AD11

#define  CardLocationAD		900			// ����Ƭ����ο�ֵ

// ���Զ�������
#define RING_QUEUE_NUM		30
#define TEST_QUEUE_NUM		40
#define TEST_CYCLE_TIME		15
#define TEST_READY_TIME		10		// ������ʱ��

#define _POS_SAMP_HOME			0		// ȡ������ʼλ��Ҳ�ǵ��������Կ�λ��

#define _POS_NEEDLE_ON_MIXCENTRE	343-20	// 36mm/0.105=343
#define _POS_NEEDLE_ON_MIXSIDE		371-20	// 39mm/0.105=371


// ȡ����߶�λ��
#define _POS_NEEDLE_HOME		0		// ȡ��������ʼλλ��
#define _POS_NEEDLE_SAMP		5		// ȡ����������λ��	



#ifndef HalfCircle

	#define _POS_MIX_BUTTOM			(_POS_MIX_TOP+540) * 2	
	#define _POS_MIX_NEEDLE			 _POS_MIX_BUTTOM - 40  
	#define _POS_MIX_CAL_START		(_POS_MIX_TOP+500) * 2	
	#define _POS_CARD_DROP			(_POS_MIX_TOP+20) * 2
	#define _POS_CARD_MIX			(_POS_MIX_TOP+40) * 2	
	#define _POS_SAMP_DOWN			3800//3920	// ȡ����ȡ��λ���½��߶�
	
#else

	#define _POS_MIX_BUTTOM			_POS_MIX_TOP+540	
	#define _POS_MIX_NEEDLE			_POS_MIX_BUTTOM-20  // ����ʱ��߶�  // һ��0.05mm
	#define _POS_MIX_CAL_START		_POS_MIX_TOP+500	// ����Һ��ʼ�߶�
	#define _POS_CARD_DROP			_POS_MIX_TOP+20		// �����߶�  380
	#define _POS_CARD_MIX			_POS_MIX_TOP+40		// ���ȸ߶�  400
	#define _POS_SAMP_DOWN			1960	// ȡ����ȡ��λ���½��߶�
	
#endif

// ��Ƭ����С��λ�ñ��
#define _POS_CARDLOAD_HOME		0		// ��Ƭ����С������ʼλ
#define _POS_CARDLOAD_RING		1600	// ��Ƭ����С����ת��λ  1168
#define _POS_CARDLOAD_DROP		74		// ��Ƭ�ڵ���λ��    6 / 0.08012 = 74.88
#define _POS_CARDLOAD_MIX		141		// ��Ƭ�ڻ��ȳ��λ�� 10.5 / 0.08012 = 131
// жƬС��λ�ñ��
#define _POS_UNLOAD_HOME		0		// жƬС����ʼλ��
#define _POS_UNLOAD_OUT			1600	// жƬС���Ƴ�жƬ�г�  812
// ע�����϶�г�
//#define _SAMP_PUMP_AIR_ISOLATE	150		// ȡ�����������	
#define _SAMP_PUMP_AIR_ISOLATE	300		// ȡ�����������	
#define _SAMP_PUMP_INTERVAL		(40+20)		// ȡ���ü�϶	
// ����ɨ��λ��
#define _POS_CARD_SCANF			305


// �����������
#define CURRENT_TURN_PLATE		1		// ת�̵������
#define CURRENT_SAMP_TRUN		4		// ȡ������ת�������
#define CURRENT_SAMP_NEEDLE		3		// ȡ���������������
#define CURRENT_CARD_LOAD		4		// �Լ�Ƭװ�ص������
#define CURRENT_CARD_UNLOAD		4		// �Լ�Ƭж�ص������
#define CURRENT_STORE_MOVE		10		// Ƭ�ֵ������
#define CURRENT_DILUENT			4		// ϡ��Һ�õ������
#define CURRENT_FLUID			4		// ��ϴҺ�õ������
#define CURRENT_EFFLUENT		4		// ��Һ�õ������
#define CURRENT_SAMP_PUMP		6		// ȡ���õ������

#endif