
#include <iom1280v.h>
#include "B1404_LIB.h"
#include "LibCommon.h"
#include "eeprom.h"


/*********************** 泵定标 ****************************/

FLOW_CAL_CHART _DiluentCalChart;
unsigned int DiluentCoff[14];		// 稀释校准因数，校准吸样量，反向因数，值越大调整结果越小

unsigned int _FlowNum0, _FlowNum1;

// 初始化流量定标判断
void InitFlowMeter(void){
	_FlowNum0 = 0;
	_FlowNum1 = 0;
}

// 流量定标数据判断处理
unsigned int JudgeFlowMeter(unsigned int i)
{
	unsigned int m, n;
	if(_FlowNum0 == 0)
	{
		_FlowNum0 = i;
		return 0;
	}
	if(_FlowNum1 == 0)
	{
		_FlowNum1 = i;
	}
	else
	{
		m = AbsDifference(i, _FlowNum0);
		n = AbsDifference(i, _FlowNum1);
		if(m > n)
		{
			if(n <= 8)
				_FlowNum0 = i;
			else
			{
				_FlowNum0 = 0;
				_FlowNum1 = 0;
				return 0;
			}
		}
		else
		{
			if(m <= 8)
				_FlowNum1 = i;
			else
			{
				_FlowNum0 = 0;
				_FlowNum1 = 0;
				return 0;
			}
		}
	}
	n = AbsDifference(_FlowNum0, _FlowNum1);
	if(n <= 8)
	{
		n = (_FlowNum0 + _FlowNum1) / 2;
		_FlowNum0 = 0;
		_FlowNum1 = 0;
		return n;
	}
	else
		return 0;
}

unsigned int InsetrDiluentFlowCalValue(unsigned int n)
{
	unsigned int sum, cal, calHD, calLD;
	unsigned char cnt, i;

	Read_DiluentCalChart();

	if(_DiluentCalChart.pnt >= _FLOW_CAL_LIST_NUM)
		_DiluentCalChart.pnt = 0;
	_DiluentCalChart.list[_DiluentCalChart.pnt] = n;	// 存入新的定标值
	_DiluentCalChart.pnt ++;
	
	// 计算新的定标结果
	sum = 0;	
	cnt = 0;
	for(i=0; i<_FLOW_CAL_LIST_NUM; i++)
	{
		if(_DiluentCalChart.list[i]>_DILUENT_PUMP_BASE_COEFF_DOWN && _DiluentCalChart.list[i]<_DILUENT_PUMP_BASE_COEFF_UP)
		{
			sum += _DiluentCalChart.list[i];
			cnt ++;
		}
	}
	cal = sum/cnt;
	sum = 0;	
	cnt = 0;
	calHD = cal + 6;
	calLD = cal - 6;
	for(i=0; i<_FLOW_CAL_LIST_NUM; i++)
	{
		if(_DiluentCalChart.list[i]>calLD && _DiluentCalChart.list[i] < calHD)
		{
			sum += _DiluentCalChart.list[i];
			cnt ++;
			}
	}
	if(cnt != 0)
		cal = sum/cnt;
	else
		cal = 0;
	Uart0ReUnable;
	uart_Printf("// CurCalib:[cal]%d [n]%d\r\n", cal, n);
	Uart0ReEnable; ;
	if(cal)
	{
		if((n > (cal-6)) && (n < (cal+6)))
		{
			_DiluentCalChart.calValue = cal;
		}
		else
		{
			cal = 0;
		}
	}
	Save_DiluentCalChart(9);
	return cal;
}

void Read_DiluentCalChart(void)
{
	unsigned long l1,l2,l3;
	unsigned int i;
	EEPROM_READ(EEP_ADD_CAL_DAT, _DiluentCalChart);
	// 检查仪器校准因数和法性
	for(i = 1;i < 14;++i)
	{
		if(_DiluentCalChart.calStand[i] < _DILUENT_MIX_BASE_COEFF_DOWN || _DiluentCalChart.calStand[i] > _DILUENT_MIX_BASE_COEFF_UP)
		{
			_DiluentCalChart.calStand[i] = _DILUENT_MIX_BASE_COEFF;
		}
	}
	EEPROM_WRITE(EEP_ADD_CAL_DAT, _DiluentCalChart);	
	
	// 检查稀释液泵校准因数和法性
	if(_DiluentCalChart.calValue < _DILUENT_PUMP_BASE_COEFF_DOWN || _DiluentCalChart.calValue > _DILUENT_PUMP_BASE_COEFF_UP)
	{
		// 初始化校准数据
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF;
		_DiluentCalChart.pnt = 0;
		_DiluentCalChart.list[0] = _DILUENT_PUMP_BASE_COEFF;
		_DiluentCalChart.pnt ++;
		EEPROM_WRITE(EEP_ADD_CAL_DAT, _DiluentCalChart);
	}
	// 计算当前稀释校准因数
	l3 = _DiluentCalChart.calValue;
	
	Uart0ReUnable;
	uart_Printf("//DiluentCoff $ %4d\r\n", l3);
	Uart0ReEnable;
		
	for(i = 1;i < 14;i++)
	{
		l2 = _DiluentCalChart.calStand[i];
		l1 = (l3 * l2)/_DILUENT_MIX_BASE_COEFF;
		DiluentCoff[i] = (unsigned int)l1;
		
		Uart0ReUnable;
		uart_Printf("//[%d] $ %4d\r\n", i, DiluentCoff[i]);
		Uart0ReEnable;
	}
}
/*
void Save_DiluentCalChart(void){
	unsigned long l1, l2;
	// 检查仪器校准因数和法性
	if(_DiluentCalChart.calStand < _DILUENT_MIX_BASE_COEFF_DOWN)
		_DiluentCalChart.calStand = _DILUENT_MIX_BASE_COEFF_DOWN;
	if(_DiluentCalChart.calStand > _DILUENT_MIX_BASE_COEFF_UP)
		_DiluentCalChart.calStand = _DILUENT_MIX_BASE_COEFF_UP;
	// 检查稀释液泵校准因数和法性
	if(_DiluentCalChart.calValue < _DILUENT_PUMP_BASE_COEFF_DOWN)
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF_DOWN;
	if(_DiluentCalChart.calValue > _DILUENT_PUMP_BASE_COEFF_UP)
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF_UP;
	EEPROM_WRITE(EEP_ADD_CAL_DAT, _DiluentCalChart);
	// 重新计算当前稀释校准因数
	l1 = _DiluentCalChart.calValue;
	l2 = _DiluentCalChart.calStand;
	l1 = (l1*l2)/_DILUENT_MIX_BASE_COEFF;
	DiluentCoff = (unsigned int)l1;
}
*/
// 2016-06-21 Save_DiluentCalChart 改为返回值函数，返回稀释校准因素
unsigned int Save_DiluentCalChart(unsigned int m)
{
	unsigned long l1, l2;
	// 检查仪器校准因数和法性
//#ifndef Puncture
	if(m == 0)		m = 9;
	else if(m > 13)	m = 13;
	if(_DiluentCalChart.calStand[m] < _DILUENT_MIX_BASE_COEFF_DOWN)
		_DiluentCalChart.calStand[m] = _DILUENT_MIX_BASE_COEFF_DOWN;
	if(_DiluentCalChart.calStand[m] > _DILUENT_MIX_BASE_COEFF_UP)
		_DiluentCalChart.calStand[m] = _DILUENT_MIX_BASE_COEFF_UP;
	// 检查稀释液泵校准因数和法性
	if(_DiluentCalChart.calValue < _DILUENT_PUMP_BASE_COEFF_DOWN)
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF_DOWN;
	if(_DiluentCalChart.calValue > _DILUENT_PUMP_BASE_COEFF_UP)
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF_UP;
	EEPROM_WRITE(EEP_ADD_CAL_DAT, _DiluentCalChart);
	// 重新计算当前稀释校准因数
	l1 = _DiluentCalChart.calValue;
	l2 = _DiluentCalChart.calStand[m];
	l1 = (l1*l2)/_DILUENT_MIX_BASE_COEFF;
	DiluentCoff[m] = (unsigned int)l1;
	return DiluentCoff[m];
/*
#else
{
	if(WithoutPuncture != 0)		// 无需穿刺
	{
		if(m == 0)		m = 9;
		else if(m > 13)	m = 13;
		if(_DiluentCalChart.calStand[m] < _DILUENT_MIX_BASE_COEFF_DOWN)
		_DiluentCalChart.calStand[m] = _DILUENT_MIX_BASE_COEFF_DOWN;
		if(_DiluentCalChart.calStand[m] > _DILUENT_MIX_BASE_COEFF_UP)
		_DiluentCalChart.calStand[m] = _DILUENT_MIX_BASE_COEFF_UP;
	// 检查稀释液泵校准因数和法性
	if(_DiluentCalChart.calValue < _DILUENT_PUMP_BASE_COEFF_DOWN)
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF_DOWN;
	if(_DiluentCalChart.calValue > _DILUENT_PUMP_BASE_COEFF_UP)
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF_UP;
	EEPROM_WRITE(EEP_ADD_CAL_DAT, _DiluentCalChart);
	// 重新计算当前稀释校准因数
	l1 = _DiluentCalChart.calValue;
	uart_Printf("*9922 $%4d\r\n", l1);
	l2 = _DiluentCalChart.calStand[m];
	l1 = (l1*l2)/_DILUENT_MIX_BASE_COEFF;
	DiluentCoff[m] = (unsigned int)l1;
	return DiluentCoff[m];
	}
	else
	{
		if(_DiluentCalChart.calStand[8] < _DILUENT_MIX_BASE_COEFF_DOWN)
		_DiluentCalChart.calStand[8] = _DILUENT_MIX_BASE_COEFF_DOWN;
	if(_DiluentCalChart.calStand[8] > _DILUENT_MIX_BASE_COEFF_UP)
		_DiluentCalChart.calStand[8] = _DILUENT_MIX_BASE_COEFF_UP;
	// 检查稀释液泵校准因数和法性
	if(_DiluentCalChart.calValue < _DILUENT_PUMP_BASE_COEFF_DOWN)
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF_DOWN;
	if(_DiluentCalChart.calValue > _DILUENT_PUMP_BASE_COEFF_UP)
		_DiluentCalChart.calValue = _DILUENT_PUMP_BASE_COEFF_UP;
	EEPROM_WRITE(EEP_ADD_CAL_DAT, _DiluentCalChart);
	// 重新计算当前稀释校准因数
	l1 = _DiluentCalChart.calValue;
	
	l2 = _DiluentCalChart.calStand[8];
	
	l1 = (l1*l2)/_DILUENT_MIX_BASE_COEFF;
	uart_Printf("*9933 $%4d\r\n", l1);
	DiluentCoff[8] = (unsigned int)l1;
	return DiluentCoff[8];
	}
}
#endif
*/
}
/***************************************************************/

void MemCopy(void *ps, void *pt, unsigned char n){
	unsigned char *p1, *p2;
	p1 = (unsigned char *)ps;
	p2 = (unsigned char *)pt;
	while(n){
		*(p2++) = *(p1++);
		n--;
		}
}

unsigned char StringMatching(_CONST char * str1, char * str2){
	// 字符串匹配
	while(*str1){
		if(*str2 == 0)
			return 0;
		if(*str1 == *str2){
			str1 ++;	str2 ++;
			}
		else
			return 0;
		}
	if(*str1 == *str2)
		return 1;
	else
		return 0;
}

unsigned int  StringToInt(const char * pStr){
	// 字符串转数值
	unsigned char i, neg=0;
	int n;
	if(*pStr=='-'){
		pStr ++;
		neg = 1;
		}
	n = 0;
	while(*pStr){
		if(*pStr>='0' && *pStr<='9'){
			n = n*10;
			n += (*pStr-0x30);
			}
		pStr ++;
		}
	if(neg)
		n = 0-n;
	return n;
}

unsigned int  StringToInt2(const char * pStr){
	// 字符串转数值
	unsigned char i;
	unsigned int n = 0;
	while(*pStr != '$'){
		if(*pStr>='0' && *pStr<='9'){
			n = n*10;
			n += (*pStr-0x30);
			}
		pStr ++;
		}
	return n;
}

unsigned int AbsDifference(unsigned int a, unsigned int b){
	unsigned int i;
	if(a>b)
		i = a-b;
	else
		i = b-a;
	return i;
}

