
/*
1.�����ַ�������
2.�ն��������������ȡ
*/
/*
����:void CommandExplain(unsigned char c)	�ڴ��������ж��е���
���:CONTROL_CMD ControlCmd					��������ж�ȡ������
*/

#include "Common.h"
/*
typedef struct _HT_COMMAND_STRING{
	_CONST char * pChar;
	_CONST unsigned int indx;
}HTCMD_STR;

typedef struct _HTCOMMAND_BUF{
	unsigned char itemPnt;			// ��ǰ��Ŀָ��
	unsigned char charPnt;			// ��ǰ��Ŀ�ַ�ָ��
	unsigned char cmdStr[35];		// �����ַ���
	unsigned char pamStr[6][10];	// ����
}HTCOMMAND_BUF;
*/
typedef struct _UICOMMAND_BUF{
	unsigned char itemPnt;			// ��ǰ���յ���Ŀ��
	unsigned char charPnt;			// ����ָ��
	unsigned char cmdStr[10];		// ���������ַ���
}UICOMMAND_BUF;
/*
// ����
_CONST char CStr0000[] 		= "echo";		// ���Կ���
_CONST HTCMD_STR HTCmd0000 	= {CStr0000, 0};
_CONST char CStr0001[] 		= "startrap01";		// ��������
_CONST HTCMD_STR HTCmd0001 	= {CStr0001, 1};
_CONST char CStr0002[] 		= "setwork";		// ���ù�������
_CONST HTCMD_STR HTCmd0002 	= {CStr0002, 2};
_CONST char CStr0003[] 		= "askwork";		// ѯ�ʵ�ǰ��������
_CONST HTCMD_STR HTCmd0003 	= {CStr0003, 3};
_CONST char CStr0004[] 		= "quit";		// �˳���ǰ��������
_CONST HTCMD_STR HTCmd0004 	= {CStr0004, 4};
_CONST char CStr0005[] 		= "suspend";				// 	��ͣ��������
_CONST HTCMD_STR HTCmd0005 	= {CStr0005, 5};
_CONST char CStr0010[] 		= "storedooropen";		// ����Ƭ��
_CONST HTCMD_STR HTCmd0010 	= {CStr0010, 10};
_CONST char CStr0011[] 		= "getstorehumiture";	// ��ȡƬ����ʪ��
_CONST HTCMD_STR HTCmd0011 	= {CStr0011, 11};
_CONST char CStr0099[] 		= "restart";	// ����
_CONST HTCMD_STR HTCmd0099 	= {CStr0099, 99};

_CONST HTCMD_STR * CmdString0[] = {
	&HTCmd0000, &HTCmd0001, &HTCmd0002, &HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0010, &HTCmd0011, &HTCmd0099, 0
};
// ��е�Լ�
_CONST HTCMD_STR * CmdString1[] = {
	&HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0099, 0
};
// Һ·�Լ�
_CONST HTCMD_STR * CmdString2[] = {
	&HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0099, 0
};
// ����
_CONST char CStr3001[] 		= "settestparamter";	// ���ò��Բ���
_CONST HTCMD_STR HTCmd3001 	= {CStr3001, 3001};
_CONST char CStr3002[] 		= "setworkstore";		// ���õ�ǰȡƬ���ֺ�
_CONST HTCMD_STR HTCmd3002 	= {CStr3002, 3002};
_CONST char CStr3003[] 		= "setdiluteratio";		// ����ϡ�ͱ���
_CONST HTCMD_STR HTCmd3003 	= {CStr3003, 3003};
_CONST char CStr3004[] 		= "setreadtime1";		// ���ö���ʱ��1
_CONST HTCMD_STR HTCmd3004 	= {CStr3004, 3004};
_CONST char CStr3005[] 		= "setreadtime2";		// ���ö���ʱ��2
_CONST HTCMD_STR HTCmd3005 	= {CStr3005, 3005};
_CONST char CStr3006[] 		= "setreadmodule";		// ���ö���ģ��
_CONST HTCMD_STR HTCmd3006 	= {CStr3006, 3006};
_CONST char CStr3007[] 		= "setdropvolume";		// ���õ�Һ����
_CONST HTCMD_STR HTCmd3007 	= {CStr3007, 3007};
_CONST char CStr3010[] 		= "setautotest";		// ����������������
_CONST HTCMD_STR HTCmd3010 	= {CStr3010, 3010};
_CONST char CStr3011[] 		= "askstorestate";		// ��ѯƬ��״̬
_CONST HTCMD_STR HTCmd3011 	= {CStr3011, 3011};
_CONST char CStr3012[] 		= "setlamplum";	// 
_CONST HTCMD_STR HTCmd3012 	= {CStr3012, 3012};
_CONST char CStr3013[] 		= "turnonlamp";			// �������Թ�Դ
_CONST HTCMD_STR HTCmd3013 	= {CStr3013, 3013};
_CONST char CStr3014[] 		= "turnofflamp";		// �رղ��Թ�Դ
_CONST HTCMD_STR HTCmd3014 	= {CStr3014, 3014};
_CONST char CStr3015[] 		= "getworkstore";		// ��ȡ��ǰ�����ֺ�
_CONST HTCMD_STR HTCmd3015 	= {CStr3015, 3015};
_CONST char CStr3020[] 		= "setdebugmode";		// 	���ò���״̬�µĵ���ģʽ��0:������1:����Һ����
_CONST HTCMD_STR HTCmd3020 	= {CStr3020, 3020};

_CONST char CStr3050[] 		= "sleep";				// 	��������
_CONST HTCMD_STR HTCmd3050 	= {CStr3050, 3050};
_CONST char CStr3051[] 		= "startup";			// 	�ָ�����
_CONST HTCMD_STR HTCmd3051 	= {CStr3051, 3051};
_CONST char CStr3052[] 		= "setsleeptime";		// 	��������ʱ��
_CONST HTCMD_STR HTCmd3052 	= {CStr3052, 3052};
_CONST char CStr3053[] 		= "samplingsw";			// 	ȡ�����ܿ���
_CONST HTCMD_STR HTCmd3053 	= {CStr3053, 3053};
_CONST char CStr3054[] 		= "rereadtest";			// 	���¶�ȡ���Կ�
_CONST HTCMD_STR HTCmd3054 	= {CStr3054, 3054};
_CONST char CStr3055[] 		= "setcleanmode";		// 	������ϴģʽ
_CONST HTCMD_STR HTCmd3055 	= {CStr3055, 3055};
_CONST char CStr3056[] 		= "setreadclose";		// 	���ö�������
_CONST HTCMD_STR HTCmd3056 	= {CStr3056, 3056};

_CONST char CStr3060[] 		= "manualprimedil";		// 	�ֶ���עϡ��Һ
_CONST HTCMD_STR HTCmd3060 	= {CStr3060, 3060};
_CONST char CStr3061[] 		= "manualprimeflu";		// 	�ֶ���ע��ϴҺ
_CONST HTCMD_STR HTCmd3061 	= {CStr3061, 3061};

//_CONST char CStr3090[] 		= "quit";			// �����˳�
//_CONST HTCMD_STR HTCmd3090 	= {CStr3090, 3090};

_CONST HTCMD_STR * CmdString3[] = {
	 &HTCmd0003, &HTCmd0004, &HTCmd0005, &HTCmd0010, &HTCmd0011, &HTCmd0099, 
	 &HTCmd3001, &HTCmd3002, &HTCmd3003, &HTCmd3004, &HTCmd3005, &HTCmd3006, &HTCmd3007, 
	 &HTCmd3010, &HTCmd3011, &HTCmd3012, 
	 &HTCmd3013, &HTCmd3014, &HTCmd3015, &HTCmd3020, 
	 &HTCmd3050, &HTCmd3051, &HTCmd3052, &HTCmd3053, &HTCmd3054, &HTCmd3055, &HTCmd3056,
	 &HTCmd3060, &HTCmd3061, 0
};
// ά��
	// λ�õ�������
_CONST char CStr4002[] 		= "setneedleonmixpos";		// ȡ����λ���ڻ��ȳر���λ�õ���
_CONST HTCMD_STR HTCmd4002 	= {CStr4002, 4002};
_CONST char CStr4004[] 		= "setdropheight";		// ȡ����߶ȵ���
_CONST HTCMD_STR HTCmd4004 	= {CStr4004, 4004};
_CONST char CStr4005[] 		= "cardloadstartadjust";	// װƬС����ʼλ����
_CONST HTCMD_STR HTCmd4005 	= {CStr4005, 4005};
_CONST char CStr4006[] 		= "cardloadendadjust";		// װƬС����ֹλ����
_CONST HTCMD_STR HTCmd4006 	= {CStr4006, 4006};
_CONST char CStr4007[] 		= "cardunloadstartadjust";	// жƬС����ʼλ����
_CONST HTCMD_STR HTCmd4007 	= {CStr4007, 4007};
_CONST char CStr4008[] 		= "cardunloadendadjust";	// жƬС����ֹλ����
_CONST HTCMD_STR HTCmd4008 	= {CStr4008, 4008};
_CONST char CStr4009[] 		= "liquidphotoadjust";	// 
_CONST HTCMD_STR HTCmd4009 	= {CStr4009, 4009};
_CONST char CStr4010[] 		= "cardstorephotoadjust";	// 
_CONST HTCMD_STR HTCmd4010 	= {CStr4010, 4010};
// ���Ե�Դ����
_CONST char CStr4011[] 		= "setlamplum";	// 
_CONST HTCMD_STR HTCmd4011 	= {CStr4011, 4011};
_CONST char CStr4012[] 		= "getlamplum";	// 
_CONST HTCMD_STR HTCmd4012 	= {CStr4012, 4012};
_CONST char CStr4013[] 		= "turnonlamp";	// 
_CONST HTCMD_STR HTCmd4013 	= {CStr4013, 4013};
_CONST char CStr4014[] 		= "turnofflamp";	// 
_CONST HTCMD_STR HTCmd4014 	= {CStr4014, 4014};

_CONST char CStr4015[] 		= "calibvalue";		// 		����У׼
_CONST HTCMD_STR HTCmd4015 	= {CStr4015, 4015};
_CONST char CStr4016[] 		= "calibtest";		// 		����У׼����, ��������
_CONST HTCMD_STR HTCmd4016 	= {CStr4016, 4016};
	// ��е��������
_CONST char CStr4020[] 		= "turnplatecheck";		// ת�̲���
_CONST HTCMD_STR HTCmd4020 	= {CStr4020, 4020};
_CONST char CStr4021[] 		= "needleturncheck";	// ȡ������ת����
_CONST HTCMD_STR HTCmd4021 	= {CStr4021, 4021};
_CONST char CStr4022[] 		= "needleupdowncheck";	// ȡ�����������в���
_CONST HTCMD_STR HTCmd4022 	= {CStr4022, 4022};
_CONST char CStr4023[] 		= "cardstoremovecheck";	// Ƭ��С���ƶ�����
_CONST HTCMD_STR HTCmd4023 	= {CStr4023, 4023};
_CONST char CStr4024[] 		= "cardtakehookcheck";		// ��Ƭ������
_CONST HTCMD_STR HTCmd4024 	= {CStr4024, 4024};
_CONST char CStr4025[] 		= "cardloadcheck";		// ��Ƭװ��С������
_CONST HTCMD_STR HTCmd4025 	= {CStr4025, 4025};
_CONST char CStr4026[] 		= "cardunloadcheck";	// ��Ƭж��С������
_CONST HTCMD_STR HTCmd4026 	= {CStr4026, 4026};
_CONST char CStr4029[] 		= "diluentpumpcheck";	// ϡ��Һ�ò���
_CONST HTCMD_STR HTCmd4029 	= {CStr4029, 4029};
_CONST char CStr4030[] 		= "leanerpumpcheck";	// ��ϴҺ�ò���
_CONST HTCMD_STR HTCmd4030 	= {CStr4030, 4030};
_CONST char CStr4031[] 		= "effluentpumpcheck";	// ��Һ�ò���
_CONST HTCMD_STR HTCmd4031 	= {CStr4031, 4031};
_CONST char CStr4032[] 		= "sampsyringecheck";	// ȡ��ע��������
_CONST HTCMD_STR HTCmd4032 	= {CStr4032, 4032};

_CONST char CStr4033[] 		= "liquidphotocheck";	// 
_CONST HTCMD_STR HTCmd4033 	= {CStr4033, 4033};
_CONST char CStr4034[] 		= "cardstorephotocheck";	// 
_CONST HTCMD_STR HTCmd4034 	= {CStr4034, 4034};

_CONST char CStr4035[] 		= "needleonmixsidecheck";	// 
_CONST HTCMD_STR HTCmd4035 	= {CStr4035, 4035};
_CONST char CStr4036[] 		= "dropheightcheck";	// 
_CONST HTCMD_STR HTCmd4036 	= {CStr4036, 4036};

_CONST char CStr4050[] 		= "diluentquantifytest";	// ϡ��Һ��������
_CONST HTCMD_STR HTCmd4050 	= {CStr4050, 4050};
_CONST char CStr4051[] 		= "leanerquantifytest";		// ��ϴҺ�ö�������
_CONST HTCMD_STR HTCmd4051 	= {CStr4051, 4051};
_CONST char CStr4052[] 		= "sampquantifytest";		// ȡ��ע������������
_CONST HTCMD_STR HTCmd4052 	= {CStr4052, 4052};
_CONST char CStr4055[] 		= "getsensor";		// ��ȡ�����ź״̬
_CONST HTCMD_STR HTCmd4055 	= {CStr4055, 4055};
_CONST char CStr4056[] 		= "getallsensor";		// ��ȡ�����ź״̬
_CONST HTCMD_STR HTCmd4056 	= {CStr4056, 4056};

	// ��������
_CONST char CStr4060[] 		= "motsetposition";		// ���õ��λ��
_CONST HTCMD_STR HTCmd4060 	= {CStr4060, 4060};
_CONST char CStr4061[] 		= "motinitcheck";		// ���λ�ó�ʼ��
_CONST HTCMD_STR HTCmd4061 	= {CStr4061, 4061};
_CONST char CStr4062[] 		= "motrun";				// �������
_CONST HTCMD_STR HTCmd4062 	= {CStr4062, 4062};
_CONST char CStr4063[] 		= "motrunto";			// ������е�
_CONST HTCMD_STR HTCmd4063 	= {CStr4063, 4063};
_CONST char CStr4064[] 		= "motruntosite";		// ������е�ָ��λ��
_CONST HTCMD_STR HTCmd4064 	= {CStr4064, 4064};
_CONST char CStr4065[] 		= "setmotparamter";		// ���õ�����в���
_CONST HTCMD_STR HTCmd4065 	= {CStr4065, 4065};
_CONST char CStr4066[] 		= "setmotbaseparamter";		// ���õ����������
_CONST HTCMD_STR HTCmd4066 	= {CStr4066, 4066};
_CONST char CStr4067[] 		= "setaddress";			// ����ģ���ַ
_CONST HTCMD_STR HTCmd4067 	= {CStr4067, 4067};
_CONST char CStr4068[] 		= "setevalve";			// ���õ�ŷ�
_CONST HTCMD_STR HTCmd4068 	= {CStr4068, 4068};
_CONST char CStr4069[] 		= "getliquidphosignal";	// ��ȡҺ·�����ź�
_CONST HTCMD_STR HTCmd4069 	= {CStr4069, 4069};
_CONST char CStr4070[] 		= "liquidphoadjust";	// Һ·����У׼
_CONST HTCMD_STR HTCmd4070 	= {CStr4070, 4070};
_CONST char CStr4071[] 		= "getstorestate";		// ��ȡƬ��״̬
_CONST HTCMD_STR HTCmd4071 	= {CStr4071, 4071};
_CONST char CStr4072[] 		= "cardstorecal";		// Ƭ�ֹ������
_CONST HTCMD_STR HTCmd4072 	= {CStr4072, 4072};
_CONST char CStr4073[] 		= "getstorephosignal";	// ��ȡƬ�ֹ����ź�
_CONST HTCMD_STR HTCmd4073 	= {CStr4073, 4073};
_CONST char CStr4074[] 		= "getliquiddetsignal";	// ��ȡҺ·̽���ź�
_CONST HTCMD_STR HTCmd4074 	= {CStr4074, 4074};

//_CONST char CStr4090[] 		= "quit";	// �ӹ����˳�
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

extern unsigned char WorkProcessStep;		// �������̺�
extern unsigned char EchoSW;

CONTROL_CMD ControlCmd;		// ��Ž����õ�����, ����ģ��ֱ�Ӷ�ȡ�˽ṹ��Ϣ, ��ȡ��������Ϣ���
/*
unsigned char HTCommandStrMatch(void){
	// �����ַ���ƥ��
	unsigned char i;
	_CONST HTCMD_STR ** CmdString;

	switch(WorkProcessStep){
		case 0:
		case 255:
			i = 0;
			while(CmdString0[i]){
				if(StringMatching(CmdString0[i]->pChar, HTCmdBuf.cmdStr)){
					ControlCmd.cmdIdx = CmdString0[i]->indx;		// ƥ��ɹ���������������
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
					ControlCmd.cmdIdx = CmdString0[i]->indx;		// ƥ��ɹ���������������
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
					ControlCmd.cmdIdx = CmdString0[i]->indx;		// ƥ��ɹ���������������
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
					ControlCmd.cmdIdx = CmdString3[i]->indx;		// ƥ��ɹ���������������
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
					ControlCmd.cmdIdx = CmdString4[i]->indx;		// ƥ��ɹ���������������
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
					ControlCmd.cmdIdx = CmdString0[i]->indx;		// ƥ��ɹ���������������
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
	// �������ת��
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
	// �����ն��������,���ղ�������ȷ���ַ�
	unsigned char i;
	unsigned char * pChar;
	if(ControlCmd.cmdState == 2)
		return 0;
	if(ControlCmd.cmdState == 0){	
		CommandClear();
		}
	// �ַ����պʹ���
	switch(HTCmdBuf.itemPnt)		// ���ݲ������ս�������ͬ����
	{
		case 0:		// ��ǰ���������ַ���
			if(c>='A' && c<='Z')
				c += 0x20;
			if((c>='a' && c<='z') || (c>='0' && c<='9'))	// �����ַ�����
			{
				ControlCmd.cmdState = 1;
				if(HTCmdBuf.charPnt < 33)
				{
					HTCmdBuf.cmdStr[HTCmdBuf.charPnt] = c;
					HTCmdBuf.charPnt ++;
					uart0SendChar(c);
				}
			}
			else if(c == ':')		// �����ַ�������
			{
				HTCmdBuf.cmdStr[HTCmdBuf.charPnt] = 0;		// �ַ���������ʶ
				HTCmdBuf.charPnt = 0;
				HTCmdBuf.itemPnt = 1;			// �����������
				uart0SendChar(':');
				// �����ַ���ƥ�䣬ƥ��ɹ����Բ�����ʽ�����������
				if(HTCommandStrMatch()==0)
				{
					// ����ƥ�����
					uart0SendChar(0x0d);
					uart0SendChar(0x0a);
					HTCmdBuf.itemPnt = 0;
					HTCmdBuf.charPnt = 0;
					//ControlCmd.cmdState == 0;
					ControlCmd.cmdState = 0;
					return 0;
				}
			}
			else if(c == 0x0d )		// ������ս���
			{
				if(HTCmdBuf.charPnt != 0)
				{
					HTCmdBuf.cmdStr[HTCmdBuf.charPnt] = 0;		// �ַ���������ʶ
					if(HTCommandStrMatch())	// ����ƥ��ɹ�
					{
						HTCmdBuf.charPnt = 0;
						ControlCmd.pamLen = 0;		// ���������
						ControlCmd.cmdState = 2;		// ���ý��յ������ʶ
					}	
					else	// ����ƥ�����
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
			else if(c == 0x08){		// �˸�
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
		case 1:		// ����0
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
			else if(c == ',')		// �����ָ���
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
					ControlCmd.cmdState = 2;			// �������������ɱ�ʶ
				}
				else		// ���մ���ɾ������
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
			else if(c == 0x08)			// �˸�
			{
				if(HTCmdBuf.charPnt != 0)
				{
					HTCmdBuf.charPnt --;
					HTCmdBuf.pamStr[HTCmdBuf.itemPnt-1][HTCmdBuf.charPnt] = 0;
					uart0SendChar(0x08);
					uart0SendChar(' ');
					uart0SendChar(0x08);
				}
				else			// �˻ص���һ������
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
					else		// ɾ������
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
	unsigned char itemPnt;			// ��ǰ���յ���Ŀ��
	unsigned char charPnt;			// ����ָ��
	unsigned char cmdStr[10];		// ���������ַ���
}UICOMMAND_BUF;
/************************************ �û������������ *******************************************/
extern unsigned char checkFlag;
extern unsigned char UartReceiveBuf[60];
unsigned char UICommandExplain(unsigned char c){
	unsigned char i;
	unsigned char * pChar;
	unsigned int cmd;
	static unsigned char checkSum = 0;
	if(ControlCmd.cmdState == 2)
		return 0;
	if(ControlCmd.cmdState == 0)		// ����
	{
		CommandClear();
	}
	switch(UICmdBuf.itemPnt)
	{
		case 0:
			if(c == '#')		// ���ʼ��־
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
			else if(c == '$')	// ���ֲ�������������������������
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
			else if(c == 0x0d)// �س��ţ��������
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
			else if(c == 0x08)		// �˸�
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
				else		// �������
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
			else if(c == '-')	// ����
			{
				if(UICmdBuf.charPnt==0)
				{
					UICmdBuf.cmdStr[0] = '-';
					UICmdBuf.charPnt ++;
					if(EchoSW)
						uart0SendChar(c);
				}
			}
			else if(c == 0x0d)	// �س��ţ��������
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
			else if(c == 0x08){		// �˸�
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
				else		// �������
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