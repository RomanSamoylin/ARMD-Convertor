#pragma once
#include "afxwin.h"

#define EVENT_COUNT							33
//----�������----------------------------------------------------------------------------------------------------------------------------------//
#define EVENT_NO_EVENT						1
#define EVENT_SYSTEM_START					2
#define EVENT_NEW_DATE						3
#define EVENT_WORK_MODE						4
#define EVENT_FEED							5
#define EVENT_SPINDLE_SPEED					6
#define EVENT_SYSTEM_STATE					7	//EVENT_SYSTEM_STATE == EVENT_PROGRAM_STATE
#define EVENT_MES_ERR_PROG					8
#define EVENT_PROGRAM_NAME					9
#define EVENT_CONTROL_PANEL_SWITCH_JOG		10
#define EVENT_CONTROL_PANEL_SWITCH_FEED		11
#define EVENT_CONTROL_PANEL_SWITCH_SPINDLE	12
#define EVENT_BLOCK_NUMB_CTRL_PROG			13
#define EVENT_TOOL_NUMBER					14
#define EVENT_CORRECTOR_NUMBER				15
#define EVENT_UAS							16
#define EVENT_UVR							17
#define EVENT_URL							18
#define EVENT_COMU							19
#define EVENT_CEFA							20
#define EVENT_MUSP							21
#define EVENT_REAZ							22
#define EVENT_MACHINE_IDLETIME_CAUSE		23
#define EVENT_PLC_ERR						24
#define EVENT_PROCESS_COMMAND_LINE			25
#define EVENT_PROCESS_BLOCK_LINE			26
#define EVENT_COMMAND_LINE					27
#define EVENT_PART_FINISHED					28
#define EVENT_G_FUNCTIONS					29
#define EVENT_RISP							30
#define EVENT_CONP							31
#define EVENT_SPEPN_REQ						32
#define EVENT_A_SPEPN						33
//----��������� �������------------------------------------------------------------------------------------------------------------------------//
#define SYSTEM_EVENT_VAR	1			//���������	�������
#define TIME_EVENT_VAR		2			//��� �����:�������� ������� "�� ��������� �������" (����� n-�� ���-�� �����������) �� moncfg.ini
#define TICKCPU_EVENT_VAR	3			//��� �����:������ ��������, �������� ������� "������ ���" (1 ��� = n �����������) �� moncfg.ini
#define MEANVALUE_EVENT_VAR	4			//��� �����:������� ��������, "����� ����� ������� �� �������� �������" �� moncfg.ini
#define APPROX_EVENT_VAR	5			//��� �����:������������ ��������, �� moncfg.ini
//---------------------------------------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------------------------------------------------------------//
#define MAX_MON_FILE_NAME	25			//����������� ��������� ����� ����� ����� ����������� � �������� ������
#define IDENTIFIER_LEN		4			//����� ���������
#define VERSION_LEN			4			//����� ����, � ������� ��������� �������� "������ �������� ������ �����������"
//---------------------------------------------------------------------------------------------------------------------------------------------//

//---����� �������-----------------------------------------------------------------------------------------------------------------------------//
#define MACHINE_IDLETIME_SET 1
#define MACHINE_IDLETIME_RESET 2
//---------------------------------------------------------------------------------------------------------------------------------------------//

//---��� ���������\������������----------------------------------------------------------------------------------------------------------------//
#define ROUTINE 0						//��� ���������
#define SUBROUTINE1 1					//��� ������������ 1 ������ �����������
#define SUBROUTINE2	2					//��� ������������ 2 ������ �����������
//---------------------------------------------------------------------------------------------------------------------------------------------//

char WORK_MODE_ST[8][6]={"MDI","AUTO","STEP","MANU","MANJ","PROF","HOME","RESET"};
char SYSTEM_STATE_ST[10][5]={{"    "},{"IDLE"},{"RUN "},{"HOLD"},{"RUNH"},{"RESE"},{"ERRO"},{"WAIT"},{"INP "},{"RTCE"}};
//---------------------------------------------------------------------------------------------------------------------------------------------//
char EVENT_NAME[][60]=
{	"","NO_EVENT","SYSTEM_START","NEW_DATE","WORK_MODE","FEED", "SPINDLE_SPEED","SYSTEM_STATE","MES_ERR_PROG","PROGRAM_NAME",
	"CONTROL_PANEL_SWITCH_JOG","CONTROL_PANEL_SWITCH_FEED","CONTROL_PANEL_SWITCH_SPINDLE","BLOCK_NUMB_CTRL_PROG",
	"TOOL_NUMBER","CORRECTOR_NUMBER","UAS","UVR","URL","COMU","CEFA","MUSP","REAZ","MACHINE_IDLETIME_CAUSE",
	"PLC_ERR","PROCESS_COMMAND_LINE","PROCESS_BLOCK_LINE","COMMAND_LINE","PART_FINISHED","G_FUNCTIONS",
	"RISP","CONP","SPEPN_REQ","A_SPEPN"
};
char EVENT_TYPE[][60]=
{
	"","SYSTEM","TIMER","CPUTICK","MEANVALUE","APPROX"
};
//----��������� ����� �����������--------------------------------------------------------------------------------------------------------------//
typedef struct _sys_monitor_info{
	short			event;			//�������
	int				account_type;	//���(���) �����: ��� ����������� ������� (��������� ��������� �������, ����� ��� TIMER_EVENT_VAR TICKCPU_EVENT_VAR)
	float			account_param;	//�������� ���� ����� (� ������� ����� ������� ����������� ���������� ��������� �������� ��� TIMER_EVENT_VAR)
									//� ������� ���� � ����� ������������ ���������� ���������� �������� �������� ������ �� 1�=1000 �����������
									//(FEED,MEANVALUE,1000), �� ���������� event = 5(EVENT_FEED), 
									//account_type = 4(MEANVALUE_EVENT_VAR), account_param =1000 
	BYTE			units;			//������� ���������
}SysMonInfo;
//��������� � APPROX_EVENT_VAR
//���������� ���� ����� ��� ������������� �������� �������� �������� ���������� �������������� ����������
// �� ����������� �� ��������, ����������� � ����� �������. 
//�����, ���� �������� �������������� ���������� ����� ��������� ���������� ������� �� ����������, �� ������������ �� ������ �������� � ������ ������ �������. 
//� �������, � ����� ������������ �������� FEED,APPROXIMATION, 10
//�����, ���� �������� �������������� ���������� ������ ���������� ����� 100(���������� ���������� �������� =100), � �� ��������� +-10,�� �������� � ���� �� ������������. 
//��� ������ �� �������� ���������� 111 ��� 89, � ���� ������������ ������� ��������. 
//�.� ����� ����� ����� ����� �� ������� (111+10/2)/10 � �������� �� 10, ����� ���������� �� +-10 ����� ����������� �� ����� ������ �����, 
//��� ����, ���� ����� ��������� ��� ����� �������� ���������� �����-�� �����, �� � ���� ������������ ��� ������ ��������. 
//����� ��� ��������������� ���������� ������������ ��� ������������� ��������� ������������ ����������.

//� ���� ����� TICKCPU_EVENT_VAR ��� ���������

typedef struct _proc_info{
	BYTE proc;					//������� ����
	short num_events;			//���-�� ������� � �������� ����
	SysMonInfo *event_info;		
}ProcInfo;

typedef struct _header_monitor_info{
	char identifier[IDENTIFIER_LEN+1];			//������������� �����
	char ver[VERSION_LEN+1];					//������ ��������, ������������ ��� ������ �����
	unsigned long file_size;					//����� ����� �����������
	char prev_file_name[MAX_MON_FILE_NAME+1];	//��� ����������� �����
	char next_file_name[MAX_MON_FILE_NAME+1];	//��� ���������� �����
	BYTE software_version_len;					//����� ������ 
	char *software_version;						//������ ������������ ����������� �������������� �� ����
	char machine_name[MAX_MON_FILE_NAME+1];		//�������� ������
	long record_time;							//������ ������ ������� � ���� (��� ������� ������������ ������� "��� �������")
	SYSTEMTIME SysTime;							//��������� ���� � �������
	BYTE num_proc;								//���������� ���� ��������� �������������� �����������
	short system_tick;							//��� �������
	ProcInfo *proc_info;						//��������� ����������� ��� ������� �� ���������
}HeaderMonInfo;
//----��������� ����� �����������--------------------------------------------------------------------------------------------------------------//
typedef struct _plc_error{
	BYTE mess_log_len;
	char *mess_log;
	BYTE alarm_log_len;
	char *alarm_log;
}PlcError;

typedef struct _emerg_data{
	char error_code;
	char msg_len;
	char *msg;
}EmergData;

typedef struct _characterization_file_data{
	char *logical_name;
	char *physical_name;
	char *destination;
}CharacterizationFileData;

typedef struct _system_start_data{
	WORD Year;
	WORD Month;
	WORD Day;
	DWORD time;
	BYTE character_files_num;
	CharacterizationFileData *file_data;
}SystemStartData;

typedef struct _prog_name_data{
	BYTE layer;
	char *name;
	char *path;
}ProgNameData;

typedef struct _prog_name{
	BYTE num;
	ProgNameData *data;
}ProgName;
/*----------*/
typedef struct _idle{
	char action;
	BYTE len;
	BYTE group_len;
	char *group;
	char *str;
}Idle;

typedef struct _machine_idletime{
	BYTE num;
	Idle *idle;
}MachineIdleTime;
/*----------*/

typedef struct _command_line{
	BYTE len;
	char *str;
}CommandLine;

typedef struct _g_functions{
	BYTE num;
	BYTE *g;
}GFunctions;

typedef struct _event_mon_data{
	short event;					//�������
	union 
	{
		char	Char;
		short	Short;
		int		Int;
		float	Float;
		long	Long;
		WORD	Word;
		char	*str;
		void	*Void;
		WORD	*time;
		GFunctions		*g_functions;
		ProgName		*progname;
		EmergData		*emergency_error;
		MachineIdleTime	*machine_idletime;
		PlcError		*plc_error;
		SystemStartData	*system_start_data;
		CommandLine		*command_line;
	}value;							//�������� �������
}EventMonData;

typedef struct _proc_mon_data{
	BYTE proc;						//����� ��������, � ������� ��������� �������
	short num_event;				//���-�� ������� � ���������
	EventMonData *event_data;
}ProcMonData;

typedef struct _write_mon_struct{
	DWORD time;						//����� "���������" �������
	WORD events_len;				//����� ���������
	BYTE num_proc;					//����� ���������� ���������
	ProcMonData *proc_data;			
	BYTE check;						//����������� �����
}MonitorData;

typedef struct _mon_buf{
	char *buf;
	DWORD index;
	DWORD buf_size;
}MonBuf;

typedef struct _mon_event_filter{
	WORD max_proc;
	WORD *num_filter_events;
	WORD **proc_filter;
	char *filter_buf;
	DWORD num_filter_buf;
	DWORD max_num_filter_buf;
}MonEventFilter;

typedef struct _mon_queue_info{
	char name[MAX_PATH];
	WORD time[7];
}MonFileInfo;

typedef struct _mon_file_queue{
	struct _mon_file_queue *prev,*next;
	MonFileInfo *file_info;
}MonFileQueue;

typedef struct _mon_file_queue_info{
	MonFileQueue *first,*last,*curr;
}MonFileQueueInfo;

//---------------------------------------------------------------------------------------------------------------------------------------------//
signed char GetEventData(MonitorData *mon_data,MonBuf *mon_buf,WORD *control_flags);
signed char LoadHeader(HeaderMonInfo **header_mon_info_p,MonBuf *mon_buf);
signed char FreeHeader(HeaderMonInfo **header_mon_info_p);
signed char ShowVal(MonitorData *mon_data);
signed char GetValFromBuf(void *value, MonBuf *mon_buf, DWORD value_size);
signed char ConvertTime(WORD *Hours, WORD *Minutes, WORD *Seconds, WORD *Milliseconds, DWORD time);
signed char ReadMonitorFile(char *pr_name,MonBuf *mon_buf,DWORD *OldFileName,DWORD *BytesRead);
DWORD GetMonBuf(MonBuf *mon_buf,DWORD num_items);
void FreeMonBuf(MonBuf *mon_buf);
signed char FreeData(MonitorData *mon_data);
//---------------------------------------------------------------------------------------------------------------------------------------------//
