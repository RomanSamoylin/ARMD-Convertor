#pragma once

#include "PathOperations.h"


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
#define EVENT_ALARM_PLC_ERR					24
#define EVENT_MESS_PLC_ERR					25
#define EVENT_PROCESS_COMMAND_LINE			26
#define EVENT_PROCESS_BLOCK_LINE			27
#define EVENT_COMMAND_LINE					28
#define EVENT_PART_FINISHED					29
#define EVENT_G_FUNCTIONS					30
#define EVENT_RISP							31
#define EVENT_CONP							32
#define EVENT_SPEPN_REQ						33
#define EVENT_A_SPEPN						34
#define EVENT_WNCMT							35
#define EVENT_WNPRT							36
#define EVENT_WPROG							37
#define EVENT_WIZKD							38
#define EVENT_TIME_SYNCH					39
#define EVENT_SPINDLE_POWER					40
#define EVENT_ARMD_SERVICE					41

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
typedef struct _events_mnemo {
	char name[256];
	WORD event;
}EM;
//----��������� ����� �����������--------------------------------------------------------------------------------------------------------------//
typedef struct _sys_monitor_info {
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

typedef struct _proc_info {
	BYTE proc;					//������� ����
	short num_events;			//���-�� ������� � �������� ����
	SysMonInfo* event_info;
}ProcInfo;

typedef struct _header_monitor_info {
	char identifier[IDENTIFIER_LEN + 1];			//������������� �����
	char ver[VERSION_LEN + 1];					//������ ��������, ������������ ��� ������ �����
	unsigned long file_size;					//����� ����� �����������
	char prev_file_name[MAX_MON_FILE_NAME + 1];	//��� ����������� �����
	char next_file_name[MAX_MON_FILE_NAME + 1];	//��� ���������� �����
	BYTE withdraw;
	BYTE software_version_len;					//����� ������ 
	char* software_version;						//������ ������������ ����������� �������������� �� ����
	char machine_name[MAX_MON_FILE_NAME + 1];		//�������� ������
	long record_time;							//������ ������ ������� � ���� (��� ������� ������������ ������� "��� �������")
	SYSTEMTIME SysTime;							//��������� ���� � �������
	BYTE local;
	char reserved[12];
	BYTE num_proc;								//���������� ���� ��������� �������������� �����������
	short system_tick;							//��� �������
	ProcInfo* proc_info;						//��������� ����������� ��� ������� �� ���������
}HeaderMonInfo;
//----��������� ����� �����������--------------------------------------------------------------------------------------------------------------//
typedef struct _plc_error {
	BYTE log_len;
	char* log;
	//	BYTE alarm_log_len;
	//	char *alarm_log;
}PlcError;

typedef struct _emerg_data {
	char error_code;
	char msg_len;
	char* msg;
}EmergData;

typedef struct _characterization_file_data {
	char* logical_name;
	char* physical_name;
	char* destination;
}CharacterizationFileData;

typedef struct _system_start_data {
	WORD Year;
	WORD Month;
	WORD Day;
	DWORD time;
	BYTE character_files_num;
	CharacterizationFileData* file_data;
}SystemStartData;

typedef struct _prog_name_data {
	BYTE layer;
	char* name;
	char* path;
}ProgNameData;

typedef struct _prog_name {
	BYTE num;
	ProgNameData* data;
}ProgName;
/*----------*/
typedef struct _idle {
	char action;
	BYTE len;
	BYTE group_len;
	char* group;
	char* str;
}Idle;

typedef struct _machine_idletime {
	BYTE num;
	Idle* idle;
}MachineIdleTime;
/*----------*/

typedef struct _command_line {
	BYTE len;
	char* str;
}CommandLine;

typedef struct _g_functions {
	BYTE num;
	BYTE* g;
}GFunctions;

typedef struct VARIABLE1 {
	BYTE len;
	char* str;
}SubroutineInfo;

typedef struct _event_mon_data {
	short event;					//�������
	union
	{
		char	Char;
		short	Short;
		int		Int;
		float	Float;
		long	Long;
		WORD	Word;
		char* str;
		void* Void;
		WORD* time;
		GFunctions* g_functions;
		ProgName* progname;
		EmergData* emergency_error;
		MachineIdleTime* machine_idletime;
		PlcError* alarm_plc_error;
		PlcError* mess_plc_error;
		SystemStartData* system_start_data;
		CommandLine* command_line;
		SubroutineInfo* subroutine_info;
	}value;							//�������� �������
}EventMonData;

typedef struct _proc_mon_data {
	BYTE proc;						//����� ��������, � ������� ��������� �������
	WORD num_event;				//���-�� ������� � ���������
	EventMonData* event_data;
}ProcMonData;

typedef struct _write_mon_struct {
	DWORD time;						//����� "���������" �������
	WORD events_len;				//����� ���������
	BYTE num_proc;					//����� ���������� ���������
	ProcMonData* proc_data;
	BYTE check;						//����������� �����
}MonitorData;

typedef struct _mon_buf {
	char* buf;
	DWORD index;
	DWORD buf_size;
}MonBuf;

typedef struct _mon_event_filter {
	WORD max_proc;
	WORD* num_filter_events;
	WORD** proc_filter;
	char* filter_buf;
	DWORD num_filter_buf;
	DWORD max_num_filter_buf;
}MonEventFilter;

typedef struct _mon_queue_info {
	//	char name[MAX_PATH];
	CPathOperations name;
	WORD time[7];
}MonFileInfo;

typedef struct _mon_file_queue {
	struct _mon_file_queue* prev, * next;
	MonFileInfo* file_info;
}MonFileQueue;

typedef struct _mon_file_queue_info {
	MonFileQueue* first, * last, * curr;
}MonFileQueueInfo;

typedef struct _time_interval {
	WORD curr[7];
	WORD from[7];
	WORD to[7];
}TimeInterval;

typedef enum _control_flags {
	NO_EVENT = 0x1,
	HEADER_LOADED = 0x2,
	WRITE_HEADER = 0x4,
	SEPARATE_PROC = 0x8,
	WRITE_HEADER_PROC = 0x10
}ControlFlgs;

#define DATE_EQUAL 0
#define DATE_FIRST_LATER	1
#define DATE_SECOND_LATER	2


//---------------------------------------------------------------------------------------------------------------------------------------------//
signed char GetEventData(HeaderMonInfo* header_mon_info, MonitorData* mon_data, MonBuf* mon_buf, TimeInterval* time_interval, WORD* control_flags);
signed char LoadHeader(HeaderMonInfo** header_mon_info_p, MonBuf* mon_buf);
signed char FreeHeader(HeaderMonInfo** header_mon_info_p);
signed char ShowVal(MonitorData* mon_data);
signed char GetValFromBuf(void* value, MonBuf* mon_buf, DWORD value_size);
signed char ConvertTime(WORD* Hours, WORD* Minutes, WORD* Seconds, WORD* Milliseconds, DWORD time);
signed char ReadMonitorFile(CPathOperations* pr_name, MonBuf* mon_buf, DWORD* OldFileName, DWORD* BytesRead);
DWORD GetMonBuf(MonBuf* mon_buf, DWORD num_items);
void FreeMonBuf(MonBuf* mon_buf);
signed char FreeData(MonitorData* mon_data);
int FindMonFile(CPathOperations* file_name, MonFileQueueInfo* mon_file_queue_info, WORD* time);
DWORD GetMonBuf(MonBuf* mon_buf, DWORD num_items);
void FreeMonBuf(MonBuf* mon_buf);
signed char FilterEvents(FILE** file_name, MonitorData* mon_data, HeaderMonInfo* mon_header, MonEventFilter* mon_event_filter, WORD* control_flags);
signed char WhichDateLater(WORD* date1, WORD* date2);
signed char WhichDateLater2(WORD* date1, WORD* date2);
//---------------------------------------------------------------------------------------------------------------------------------------------//
