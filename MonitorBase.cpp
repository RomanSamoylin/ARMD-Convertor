// Monitor1.cpp : Defines the entry point for the console application.
//
//ДЛЯ ОТЛАДКИ!!! ПРИМЕР ЧТЕНИЯ ФАЙЛА МОНИТОРИНГА!!!
#include "stdafx.h"
#include "MonitorBASE.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
WORD event_count = 33;

BYTE first_record = 0;

char WORK_MODE_ST[8][6] = { "MDI","AUTO","STEP","MANU","MANJ","PROF","HOME","RESET" };
char SYSTEM_STATE_ST[10][5] = { {"    "},{"IDLE"},{"RUN "},{"HOLD"},{"RUNH"},{"RESE"},{"ERRO"},{"WAIT"},{"INP "},{"RTCE"} };
//---------------------------------------------------------------------------------------------------------------------------------------------//

EM EVENT_NAME[] = {
	{"",0},
	{"NO_EVENT (НЕТ СОБЫТИЯ)"							,1},
	{"SYSTEM_START (ВКЛЮЧЕНИЕ СИСТЕМЫ)"					,2},
	{"NEW_DATE (ДАТА)"									,3},
	{"WORK_MODE (РЕЖИМ РАБОТЫ)"							,4},
	{"FEED (ПОДАЧА)"									,5},
	{"SPINDLE_SPEED (СКОРОСТЬ ШПИНДЕЛЯ)"				,6},
	{"SYSTEM_STATE (СОСТОЯНИЕ СИСТЕМЫ)"					,7},
	{"MES_ERR_PROG (СООБЩЕНИЯ ОБ ОШИБКАХ)"				,8},
	{"PROGRAM_NAME (ИМЯ УП)"							,9},
	{"CONTROL_PANEL_SWITCH_JOG (ПЕРЕКЛЮЧАТЕЛЬ JOG)"		,10},
	{"CONTROL_PANEL_SWITCH_FEED (ПЕРЕКЛЮЧАТЕЛЬ F)"		,11},
	{"CONTROL_PANEL_SWITCH_SPINDLE (ПЕРЕКЛЮЧАТЕЛЬ S)"	,12},
	{"BLOCK_NUMB_CTRL_PROG (НОМЕР КАДРА УП)"			,13},
	{"TOOL_NUMBER (НОМЕР ИНСТРУМЕНТА)"					,14},
	{"CORRECTOR_NUMBER (НОМЕР КОРРЕКТОРА)"				,15},
	{"UAS (СОСТОЯНИЕ UAS)"								,16},
	{"UVR (СОСТОЯНИЕ UVR)"								,17},
	{"URL (СОСТОЯНИЕ URL)"								,18},
	{"COMU (СОСТОЯНИЕ COMU)"							,19},
	{"CEFA (СОСТОЯНИЕ CEFA)"							,20},
	{"MUSP (СОСТОЯНИЕ MUSP)"							,21},
	{"REAZ (СОСТОЯНИЕ REAZ)"							,22},
	{"MACHINE_IDLETIME_CAUSE (ПРИЧИНЫ ПРОСТОЯ)"			,23},
	{"PLC_ALARM (ОШИБКИ ОТ PLC)"						,24},
	{"PLC_MESSAGE (СООБЩЕНИЯ ОТ PLC)"					,25},
	{"PROCESS_COMMAND_LINE (КОМАНДА ИЗ СТРОКИ ПРОЦЕССА)",26},
	{"PROCESS_BLOCK_LINE (КАДР ИЗ СТРОКИ ПРОЦЕССА)"		,27},
	{"COMMAND_LINE (КОМАНДА ИЗ СТРОКИ КОМАНДА)"			,28},
	{"PART_FINISHED (КОНЕЦ ОБРАБОТКИ ДЕТАЛИ)"			,29},
	{"G_FUNCTIONS (G ФУНКЦИИ)"							,30},
	{"RISP (СОСТОЯНИЕ RISP)"							,31},
	{"CONP (СОСТОЯНИЕ COMP)"							,32},
	{"SPEPN_REQ (СОСТОЯНИЕ SPEPENREQ)"					,33},
	{"A_SPEPN (СОСТОЯНИЕ ASPEPN)"						,34 },
	{"WNCMT (СОСТОЯНИЕ WNCMT)"							,35 },
	{ "WNPRT (СОСТОЯНИЕ WNPRT)"							,36 },
	{ "WPROG (СОСТОЯНИЕ WPROG)"							,37 },
	{ "WIZKD (СОСТОЯНИЕ WIZKD)"							,38 },
	{ "TIME_SYNCH (СИНХРОНИЗАЦИЯ ВРЕМЕНИ)"				,39 },
	{ "SPINDLE_POWER (НАГРУЗКА НА ШПИНДЕЛЬ)"			,40 },
	{ "ARMD_SERVICE (СЕРВИСНЫЕ СООБЩЕНИЯ)"				,41 },
	{""													,0} };

/*
char EVENT_NAME[][60]=
{	"","NO_EVENT","SYSTEM_START","NEW_DATE","WORK_MODE","FEED", "SPINDLE_SPEED","SYSTEM_STATE","MES_ERR_PROG","PROGRAM_NAME",
	"CONTROL_PANEL_SWITCH_JOG","CONTROL_PANEL_SWITCH_FEED","CONTROL_PANEL_SWITCH_SPINDLE","BLOCK_NUMB_CTRL_PROG",
	"TOOL_NUMBER","CORRECTOR_NUMBER","UAS","UVR","URL","COMU","CEFA","MUSP","REAZ","MACHINE_IDLETIME_CAUSE",
	"PLC_ERR","PROCESS_COMMAND_LINE","PROCESS_BLOCK_LINE","COMMAND_LINE","PART_FINISHED","G_FUNCTIONS",
	"RISP","CONP","SPEPN_REQ","A_SPEPN"
};
*/
char EVENT_TYPE[][60] =
{
	"","SYSTEM","TIMER","CPUTICK","MEANVALUE","APPROX"
};

/*
#include "windows.h"
#include <math.h>
#include <malloc.h>
#include <conio.h>
#include <stddef.h>
#include <time.h>
*/
//#include "monitorarmd.h"

signed char ReadMonitorFile(CPathOperations* pr_name, MonBuf* mon_buf, DWORD* OldFileLen, DWORD* BytesRead)
{
	HANDLE pFile;
	DWORD tmp;
	char* tmp_name = new char[MAX_PATH];
	if (BytesRead == NULL) BytesRead = &tmp;
	mon_buf->index = 0;
	//открывать файл лучше с флагом FILE_SHARE_READ, но здесь есть проблема с совместным доступом к файлу со стороны УЧПУ и данной программы
	//связано это или с сетью или с конфликтом операционных систем, т.к. УЧПУ управляет On-Time RTOS-32
	//для того, чтобы УЧПУ могло получить доступ к файлу как можно быстрее желательно открывать файл на небольшой промежуток времени
	//иначе может произойти переполнение буфера. При возможности лучше не открывать файл, с которым работает УЧПУ.
	pr_name->GetFullPath(tmp_name, MAX_PATH);
	pFile = CreateFile(tmp_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (pFile == INVALID_HANDLE_VALUE)
	{
		delete tmp_name;
		return -1;
	}
	//т.к. событие "нет события" затирается следующим событием причем это может быть повторное событие "нет события" 
	//с новым временем появления, которое возникает в системе, если не происходило никаких изменений в значениях мониторируемых переменных, 
	//то необходимо вернуться на позицию в файле до события "нет события". Период возникновения события "нет события"
	//(запись на диск накопленного буффера) устанавливается инструкцией RecordTime в файле MonCfg.ini.
	if (OldFileLen != NULL)
		SetFilePointer(pFile, (*OldFileLen), NULL, FILE_BEGIN);//переместиться на позицию, с которой будет считываться файл
	ReadFile(pFile, mon_buf->buf, mon_buf->buf_size, BytesRead, NULL); //чтение файла
	CloseHandle(pFile);
	delete tmp_name;
	return 0;
}

//----Начало секции поиска файла---------------------------------------------------------------------------------------------------------------//
int FindQueueBegin(CPathOperations* file_name, MonBuf* mon_buf)
{
	HeaderMonInfo* mon_header = NULL;
	if (file_name == NULL || mon_buf == NULL) return -1;
	BYTE stop = 0;
	do {
		if (ReadMonitorFile(file_name, mon_buf, NULL, NULL) < 0) return -1;
		if (LoadHeader(&mon_header, mon_buf) < 0) { FreeHeader(&mon_header); return -1; }
		if (mon_header != NULL)
		{
			if (mon_header->prev_file_name[0] != 0)
			{
				file_name->SetFileName(mon_header->prev_file_name);
			}
			else
			{
				stop = 1;
			}
			FreeHeader(&mon_header);
		}
		else
		{
			return -1;
		}
	} while (stop == 0);
	return 0;
}

int GetFileInfo(CPathOperations* file_name, MonBuf* mon_buf, MonFileInfo* file_info)
{
	HeaderMonInfo* mon_header = NULL;
	MonitorData* mon_data = (MonitorData*)calloc(1, sizeof(MonitorData));
	if (mon_data == NULL) return -1;
	BYTE stop = 0;
	WORD control_flags = 0;
	if (file_name == NULL || mon_buf == NULL || file_info == NULL)
	{
		free(mon_data); return -1;
	}
	if (ReadMonitorFile(file_name, mon_buf, NULL, NULL) < 0)
	{
		free(mon_data); return -1;
	}
	if (LoadHeader(&mon_header, mon_buf) < 0)
	{
		free(mon_data); return -1;
	}
	if (GetEventData(mon_header, mon_data, mon_buf, NULL, &control_flags) < 0)
	{
		free(mon_data); return -1;
	}
	//	strcpy(file_info->name,file_name);
	file_info->name.SetFullPath(file_name->GetFullPath(NULL, 0));

	file_info->time[0] = mon_header->SysTime.wYear;
	file_info->time[1] = mon_header->SysTime.wMonth;
	file_info->time[2] = mon_header->SysTime.wDay;
	ConvertTime(&file_info->time[3], &file_info->time[4], &file_info->time[5], &file_info->time[6], mon_data->time);

	if (mon_header->next_file_name[0] != 0)
	{
		file_name->SetFileName(mon_header->next_file_name);
	}
	else
		file_name->SetFileName("");
	FreeHeader(&mon_header);
	FreeData(mon_data);
	free(mon_data);
	return 0;
}

MonFileQueueInfo* BuildQueue(CPathOperations* file_name)
{
	//MonFileQueue *mon_qeueue_first=NULL,*mon_qeueue_last=NULL,*mon_qeueue_curr=NULL;
	MonFileQueueInfo* mon_file_queue_info = NULL;
	HeaderMonInfo* mon_header = NULL;
	MonBuf mon_buf;
	//	MonitorData mon_data;
	WORD control_flag = 0;
	MonFileInfo* file_info = NULL;
	//	mon_file_queue_info=(MonFileQueueInfo*)calloc(1,sizeof(MonFileQueueInfo));
	mon_file_queue_info = new MonFileQueueInfo;
	if (mon_file_queue_info == NULL) return NULL;
	memset(mon_file_queue_info, 0, sizeof(MonFileQueueInfo));
	//	if(GetMonBuf(&mon_buf,100000) == 0) return NULL;
	if (GetMonBuf(&mon_buf, 1000000) == 0) return NULL;
	if (FindQueueBegin(file_name, &mon_buf) < 0) { FreeMonBuf(&mon_buf); delete mon_file_queue_info; return NULL; }
	do {
		//file_info=(MonFileInfo*)calloc(1,sizeof(MonFileInfo));
		file_info = new MonFileInfo;
		if (GetFileInfo(file_name, &mon_buf, file_info) < 0)
		{
			char err[MAX_PATH + 50];
			sprintf_s(err, MAX_PATH + 50, "Файл очереди %s не найден! Ошибка при построении очереди.", file_name->GetFullPath(NULL, 0));
			MessageBoxA(NULL, err, "Ошибка", MB_OK);
			//return NULL;
			break;
		}
		if (mon_file_queue_info->curr == NULL)
		{
			mon_file_queue_info->curr = new MonFileQueue;
			if (mon_file_queue_info->curr == NULL)
				return NULL;
			memset(mon_file_queue_info->curr, 0, sizeof(MonFileQueue));
			mon_file_queue_info->curr->file_info = file_info;
			mon_file_queue_info->first = mon_file_queue_info->curr;
		}
		else
		{
			if (mon_file_queue_info->curr->next == NULL)
			{
				//				mon_file_queue_info->curr->next=(MonFileQueue*)calloc(1,sizeof(MonFileQueue));
				mon_file_queue_info->curr->next = new MonFileQueue;
				if (mon_file_queue_info->curr->next == NULL)
					return NULL;
				memset(mon_file_queue_info->curr->next, 0, sizeof(MonFileQueue));
				mon_file_queue_info->curr->next->prev = mon_file_queue_info->curr;
				mon_file_queue_info->curr->next->file_info = file_info;
				mon_file_queue_info->curr = mon_file_queue_info->curr->next;
			}
		}
	} while ((char*)file_name->GetFileName(NULL, 0)[0] != 0);
	mon_file_queue_info->last = mon_file_queue_info->curr;
	FreeMonBuf(&mon_buf);
	return mon_file_queue_info;
}


signed char WhichDateLater(WORD* date1, WORD* date2)
{
	int i;
	for (i = 0; i < 7; i++)
	{
		if (date1[i] < date2[i])
		{
			return DATE_SECOND_LATER;
		}
		else
			if (date1[i] > date2[i])
				return DATE_FIRST_LATER;
	}
	if (date1[6] == date2[6])
		return DATE_EQUAL;
	return 0;
}

signed char WhichDateLater2(WORD* date1, WORD* date2)
{
	int i;
	for (i = 0; i < 6; i++)
	{
		if (date1[i] < date2[i])
		{
			return DATE_SECOND_LATER;
		}
		else
			if (date1[i] > date2[i])
				return DATE_FIRST_LATER;
	}
	if (date1[5] == date2[5])
		return DATE_EQUAL;
	return 0;
}

int FindMonFile(CPathOperations* file_name, MonFileQueueInfo* mon_file_queue_info, WORD* time)
{
	int i;
	BYTE tmp;
	BYTE find = 0;
	MonFileQueue* mon_file_queue;
	DWORD e;
	if (file_name == NULL || mon_file_queue_info == NULL || mon_file_queue_info->first == NULL || time == NULL)
		return -1;
	mon_file_queue = mon_file_queue_info->first;
	file_name->SetPath(mon_file_queue->file_info->name.GetPath(NULL, 0));
	do {
		tmp = WhichDateLater(mon_file_queue->file_info->time, time);
		if (tmp == DATE_SECOND_LATER)
		{
			if (mon_file_queue->prev != NULL)
			{
				mon_file_queue_info->curr = mon_file_queue->prev;
				file_name->SetFileName(mon_file_queue->prev->file_info->name.GetFileName(NULL, 0));
			}
			else
			{
				mon_file_queue_info->curr = mon_file_queue;
				file_name->SetFileName(mon_file_queue->file_info->name.GetFileName(NULL, 0));
			}
		}
		else
			if (tmp == DATE_EQUAL)
			{
				if (mon_file_queue != NULL)
				{
					mon_file_queue_info->curr = mon_file_queue;
					file_name->SetFileName(mon_file_queue->file_info->name.GetFileName(NULL, 0));
				}
			}
			else
			{
				if (mon_file_queue != NULL)
				{
					mon_file_queue_info->curr = mon_file_queue_info->first;
					file_name->SetFileName(mon_file_queue_info->first->file_info->name.GetFileName(NULL, 0));
				}
			}
		if (mon_file_queue != NULL)
			mon_file_queue = mon_file_queue->next;
	} while (mon_file_queue != NULL);
	return 0;
}
//----Конец секции поиска файла----------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------------//
DWORD GetMonBuf(MonBuf* mon_buf, DWORD num_items)
{
	mon_buf->buf_size = num_items;
	mon_buf->buf = (char*)malloc(mon_buf->buf_size * sizeof(char));
	if (mon_buf->buf == NULL) return 0;
	mon_buf->index = 0;
	return mon_buf->buf_size;
}

void FreeMonBuf(MonBuf* mon_buf)
{
	free(mon_buf->buf);
	mon_buf->buf = NULL;
	mon_buf->buf_size = 0;
	mon_buf->index = 0;
}
//---------------------------------------------------------------------------------------------------------------------------------------------//
signed char ConvertTime(WORD* Hours, WORD* Minutes, WORD* Seconds, WORD* Milliseconds, DWORD time)
{
	//	if (Microseconds!= NULL) (*Microseconds)=(time%10)*100;
	time /= 10;
	if (Milliseconds != NULL) (*Milliseconds) = time % 1000;
	time /= 1000;
	if (Seconds != NULL)	(*Seconds) = time % 60;
	time /= 60;
	if (Minutes != NULL)(*Minutes) = time % 60;
	time /= 60;
	if (Hours != NULL)	(*Hours) = time % 60;
	return 0;
}

signed char GetValFromBuf(void* value, MonBuf* mon_buf, DWORD value_size)
{
	if (mon_buf->index >= mon_buf->buf_size)
	{
		MessageBox(NULL, "Ошибка доступа к памяти.", "Ошибка", MB_OK | MB_ICONERROR);
		return -1;
	}
	memcpy(value, &mon_buf->buf[mon_buf->index], value_size);
	mon_buf->index += value_size;
	return 0;
}
//#define str_num 10
//функция считывает заголовок и начальные значения переменных
signed char LoadHeader(HeaderMonInfo** header_mon_info_p, MonBuf* mon_buf)
{
	HeaderMonInfo* header_mon_info;
	header_mon_info = (*header_mon_info_p);

	if (header_mon_info == NULL)	header_mon_info = (HeaderMonInfo*)calloc(1, sizeof(HeaderMonInfo));
	GetValFromBuf(header_mon_info->identifier, mon_buf, IDENTIFIER_LEN);
	if (strcmp(header_mon_info->identifier, "BSMN") != 0) return -1;
	GetValFromBuf(header_mon_info->ver, mon_buf, VERSION_LEN);
	if (strcmp(header_mon_info->ver, "0000") != 0) return -1;
	GetValFromBuf(&header_mon_info->file_size, mon_buf, sizeof(header_mon_info->file_size));//точная текущая длина файла, 
												//функция GetEventData не должна обрабатывать больше байт, чем записано в данной переменной
												//т.к. в конце теоретически может появиться грязь
	GetValFromBuf(header_mon_info->prev_file_name, mon_buf, MAX_MON_FILE_NAME);//имя предыдущего файла истории, если нет поле пустое
	GetValFromBuf(header_mon_info->next_file_name, mon_buf, MAX_MON_FILE_NAME);//имя следующего файла истории, если нет поле пустое

	GetValFromBuf(&header_mon_info->withdraw, mon_buf, sizeof(header_mon_info->withdraw));//им¤ следующего файла истории, если нет поле пустое

	GetValFromBuf(&header_mon_info->software_version_len, mon_buf, sizeof(header_mon_info->software_version_len));
	header_mon_info->software_version = (char*)calloc(header_mon_info->software_version_len + 1, sizeof(char));
	if (header_mon_info->software_version == NULL) return -1;
	GetValFromBuf(header_mon_info->software_version, mon_buf, header_mon_info->software_version_len);//  "имя" станка

	GetValFromBuf(header_mon_info->machine_name, mon_buf, MAX_MON_FILE_NAME);//  "имя" станка
	GetValFromBuf(&header_mon_info->system_tick, mon_buf, sizeof(header_mon_info->system_tick));//системный тик (AXCFIL;инструкция TIM,мс)
	GetValFromBuf(&header_mon_info->record_time, mon_buf, sizeof(header_mon_info->record_time));//системный тик (AXCFIL;инструкция TIM,мс)
	GetValFromBuf(&header_mon_info->SysTime.wYear, mon_buf, sizeof(header_mon_info->SysTime.wYear));//год создания файла
	GetValFromBuf(&header_mon_info->SysTime.wMonth, mon_buf, sizeof(header_mon_info->SysTime.wMonth));//месяц
	GetValFromBuf(&header_mon_info->SysTime.wDay, mon_buf, sizeof(header_mon_info->SysTime.wDay));//день
	GetValFromBuf(&header_mon_info->local, mon_buf, sizeof(BYTE));
	GetValFromBuf(&header_mon_info->reserved, mon_buf, 12 * sizeof(BYTE));

	GetValFromBuf(&header_mon_info->num_proc, mon_buf, sizeof(header_mon_info->num_proc));//количество всех процессов участвующих в процессе мониторинга
	header_mon_info->proc_info = (ProcInfo*)calloc(header_mon_info->num_proc, sizeof(ProcInfo));
	if (header_mon_info->proc_info == NULL) return -1;
	for (short i = 0; i < header_mon_info->num_proc; i++)
	{
		GetValFromBuf(&header_mon_info->proc_info[i].num_events, mon_buf, sizeof(header_mon_info->proc_info[i].num_events));//общее количество событий в процессе
		header_mon_info->proc_info[i].event_info = (SysMonInfo*)calloc(header_mon_info->proc_info[i].num_events, sizeof(SysMonInfo));
		if (header_mon_info->proc_info[i].event_info == NULL) return -1;
		for (WORD j = 0; j < header_mon_info->proc_info[i].num_events; j++)
		{
			//параметры мониторинга
			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].event, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].event));
			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].account_type, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].account_type));
			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].account_param, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].account_param));
			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].units, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].units));
		}
	}

	//GetEventData(mon_data,mon_buf,control_flags);//считываем начальные параметры переменных
	(*header_mon_info_p) = header_mon_info;
	return 0;
}
//#undef str_num

signed char FreeHeader(HeaderMonInfo** header_mon_info_p)
{
	HeaderMonInfo* header_mon_info;
	if (header_mon_info_p == NULL) return -1;
	header_mon_info = (*header_mon_info_p);
	if (header_mon_info == NULL) return -1;

	if (header_mon_info->software_version != NULL)
		free(header_mon_info->software_version);
	if (header_mon_info->proc_info != NULL)
	{
		for (short i = 0; i < header_mon_info->num_proc; i++)
		{
			if (header_mon_info->proc_info[i].event_info != NULL)
				free(header_mon_info->proc_info[i].event_info);
		}
		free(header_mon_info->proc_info);
	}
	free(header_mon_info);
	header_mon_info = NULL;

	(*header_mon_info_p) = header_mon_info;
	return 0;
}


signed char ShowHeader(HeaderMonInfo* header_mon_info)
{
	printf("SOFTWARE_VERSION:%s ", header_mon_info->software_version);
	printf("MACHINE_NAME:%s SYSTEM_TICK:%d RECORD_TIME:%d ", header_mon_info->machine_name, header_mon_info->system_tick, header_mon_info->record_time);
	printf("TOTAL_PROC:%d ", header_mon_info->num_proc - 1);
	printf("\n");
	for (short i = 0; i < header_mon_info->num_proc; i++)
	{
		printf("\n");
		if (i != 0)printf("PROCESS:%d ", i);
		else	printf("PROCESS:%d (NO PROCESS) ", i);
		printf("NUMBER_OF_EVENTS_IN_PROCESS:%d ", header_mon_info->proc_info[i].num_events);
		for (short j = 0; j < header_mon_info->proc_info[i].num_events; j++)
		{
			//параметры мониторинга
			printf("EVENT:%s ", EVENT_NAME[header_mon_info->proc_info[i].event_info[j].event]);
			printf("EVENT_TYPE:%s ", EVENT_TYPE[header_mon_info->proc_info[i].event_info[j].account_type]);
			printf("VALUE:%f ", header_mon_info->proc_info[i].event_info[j].account_param);
			//			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].units, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].units));
			printf("\n");
		}
	}
	return 0;
}

signed char RecordHeader(FILE* file_name, HeaderMonInfo* header_mon_info, WORD* status)
{
	if (first_record == 1)
	{
		fprintf(file_name, "\n");
		fprintf(file_name, "\n");
	}
	fprintf(file_name, "----------------------НОВЫЙ ФАЙЛ-----------------------------------------------\n");
	fprintf(file_name, "ВЕРСИЯ ПРО:%s ", header_mon_info->software_version);
	fprintf(file_name, "ИМЯ ЧПУ :%s ТИК ЧПУ:%d ПЕРИОД ЗАПИСИ В ФАЙЛ:%d ", header_mon_info->machine_name, header_mon_info->system_tick, header_mon_info->record_time);
	fprintf(file_name, "КОЛИЧЕСТВО ВСЕХ ПРОЦЕССОВ:%d ", header_mon_info->num_proc - 1);
	return 0;
}
signed char RecordProcHeader(FILE* file_name, BYTE proc, HeaderMonInfo* header_mon_info, MonEventFilter* mon_event_filter, WORD* status)
{
	BYTE find_event;
	fprintf(file_name, "\n");
	fprintf(file_name, "ПРОЦЕСС:%d ", proc);
	fprintf(file_name, "КОЛИЧЕСТВО СОБЫТИЙ В ПРОЦЕССЕ:%d ", header_mon_info->proc_info[proc].num_events);
	for (short j = 0; j < header_mon_info->proc_info[proc].num_events; j++)
	{
		//параметры мониторинга
		find_event = 0;
		for (DWORD ii = 0; ii < mon_event_filter->num_filter_events[proc]; ii++)
		{
			if (header_mon_info->proc_info[proc].event_info[j].event == mon_event_filter->proc_filter[proc][ii])
			{
				find_event = 1;
				break;
			}
		}
		if (find_event == 1)
		{
			fprintf(file_name, "\n");
			fprintf(file_name, "СОБЫТИЕ:%s ", EVENT_NAME[header_mon_info->proc_info[proc].event_info[j].event].name);
			fprintf(file_name, "ТИП СОБЫТИЯ:%s ", EVENT_TYPE[header_mon_info->proc_info[proc].event_info[j].account_type]);
			fprintf(file_name, "ЗНАЧЕНИЕ:%f ", header_mon_info->proc_info[proc].event_info[j].account_param);
		}
	}
	fprintf(file_name, "\n");
	return 0;
}

//Функция считывает события из массива и распределяет их по структурам
signed char GetEventData(HeaderMonInfo* header_mon_info, MonitorData* mon_data, MonBuf* mon_buf, TimeInterval* time_interval, WORD* control_flags)
{
	short mon_inf_pos;
	if (mon_data == NULL || mon_buf == NULL) return -1;
	if (mon_buf->buf_size - mon_buf->index < sizeof(mon_data->time) + sizeof(mon_data->events_len) + sizeof(mon_data->num_proc) + sizeof(mon_data->proc_data[0].proc) + sizeof(mon_data->proc_data[0].event_data[0].event) + sizeof(mon_data->check))
	{
		return 2;
	}
	if (control_flags != NULL) (*control_flags) &= ~NO_EVENT;
	if (GetValFromBuf(&mon_data->time, mon_buf, sizeof(mon_data->time)) < 0) return -1;
	if (GetValFromBuf(&mon_data->events_len, mon_buf, sizeof(mon_data->events_len)) < 0) return -1;
	if (mon_data->events_len + sizeof(mon_data->check) + mon_buf->index > mon_buf->buf_size)
	{
		mon_buf->index -= sizeof(mon_data->time) + sizeof(mon_data->events_len);
		return 2;
	}
	if (GetValFromBuf(&mon_data->num_proc, mon_buf, sizeof(mon_data->num_proc)) < 0) return -1;
	mon_data->proc_data = (ProcMonData*)calloc(mon_data->num_proc, sizeof(ProcMonData));
	//	mon_data->proc_data=new ProcMonData[mon_data->num_proc];
	if (mon_data->proc_data == NULL) return -1;
	for (BYTE proc = 0; proc < mon_data->num_proc; proc++)
	{
		if (GetValFromBuf(&mon_data->proc_data[proc].proc, mon_buf, sizeof(mon_data->proc_data[proc].proc)) < 0)
			return -1;
		if (GetValFromBuf(&mon_data->proc_data[proc].num_event, mon_buf, sizeof(mon_data->proc_data[proc].num_event)) < 0)
			return -1;
		mon_data->proc_data[proc].event_data = (EventMonData*)calloc(mon_data->proc_data[proc].num_event, sizeof(EventMonData));
		if (mon_data->proc_data[proc].event_data == NULL)
			return -1;
		for (WORD event = 0; event < mon_data->proc_data[proc].num_event; event++)
		{
			GetValFromBuf(&mon_inf_pos, mon_buf, sizeof(short));
			mon_data->proc_data[proc].event_data[event].event = header_mon_info->proc_info[mon_data->proc_data[proc].proc].event_info[mon_inf_pos].event;
			//			if(GetValFromBuf(&mon_data->proc_data[proc].event_data[event].event,mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].event)) < 0)
			//				return -1;
			switch (mon_data->proc_data[proc].event_data[event].event)
			{
			case EVENT_NO_EVENT:
			{
				if (control_flags != NULL) (*control_flags) |= NO_EVENT;
			}
			break;
			case EVENT_SYSTEM_START:
				mon_data->proc_data[proc].event_data[event].value.system_start_data = (SystemStartData*)calloc(1, sizeof(SystemStartData));
				if (mon_data->proc_data[proc].event_data[event].value.system_start_data == NULL) return -1;
				if (GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->Year, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->Year)) < 0) return -1;
				if (GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->Month, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->Month)) < 0) return -1;
				if (GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->Day, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->Day)) < 0) return -1;
				if (GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->time, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->time)) < 0) return -1;
				//-----------------------------информация о файлах характеризации-----------------------------------------------------------//
				if (GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num)) < 0) return -1;
				mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data = (CharacterizationFileData*)calloc(mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num, sizeof(CharacterizationFileData));
				if (mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data == NULL) return -1;
				for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num; i++)
				{
					BYTE len;
					if (GetValFromBuf(&len, mon_buf, sizeof(BYTE)) < 0) return -1;
					mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name = (char*)calloc(len + 1, sizeof(char));
					if (mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name == NULL) return -1;
					if (GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name, mon_buf, len) < 0) return -1;

					if (GetValFromBuf(&len, mon_buf, sizeof(BYTE)) < 0) return -1;
					mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name = (char*)calloc(len + 1, sizeof(char));
					if (mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name == NULL) return -1;
					if (GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name, mon_buf, len) < 0) return -1;

					if (GetValFromBuf(&len, mon_buf, sizeof(BYTE)) < 0) return -1;
					mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination = (char*)calloc(len + 1, sizeof(char));
					if (mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination == NULL) return -1;
					if (GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination, mon_buf, len) < 0) return -1;
				}
				//--------------------------------------------------------------------------------------------------------------------------//
				if (time_interval != NULL)
				{
					time_interval->curr[0] = mon_data->proc_data[proc].event_data[event].value.system_start_data->Year;
					time_interval->curr[1] = mon_data->proc_data[proc].event_data[event].value.system_start_data->Month;
					time_interval->curr[2] = mon_data->proc_data[proc].event_data[event].value.system_start_data->Day;
				}
				break;
			case  EVENT_NEW_DATE:

				mon_data->proc_data[proc].event_data[event].value.time = (WORD*)calloc(3, sizeof(WORD));
				if (mon_data->proc_data[proc].event_data[event].value.time == NULL) return -1;
				GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.time, mon_buf, 3 * sizeof(WORD));
				if (time_interval != NULL)
				{
					time_interval->curr[0] = mon_data->proc_data[proc].event_data[event].value.time[0];
					time_interval->curr[1] = mon_data->proc_data[proc].event_data[event].value.time[1];
					time_interval->curr[2] = mon_data->proc_data[proc].event_data[event].value.time[2];
				}
				break;
			case EVENT_WORK_MODE: case EVENT_SYSTEM_STATE: case EVENT_UAS: case EVENT_UVR:case EVENT_URL: case EVENT_COMU:
			case EVENT_CEFA: case EVENT_MUSP: case EVENT_REAZ: case EVENT_PART_FINISHED:case EVENT_RISP: case EVENT_CONP:
			case EVENT_SPEPN_REQ: case EVENT_A_SPEPN:
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Char, mon_buf, sizeof(char));
				break;
			case EVENT_FEED: case EVENT_SPINDLE_SPEED: case EVENT_CONTROL_PANEL_SWITCH_JOG:
			case EVENT_CONTROL_PANEL_SWITCH_FEED: case EVENT_CONTROL_PANEL_SWITCH_SPINDLE: case EVENT_SPINDLE_POWER:
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Float, mon_buf, sizeof(float));
				break;
			case EVENT_MES_ERR_PROG:
			{
				mon_data->proc_data[proc].event_data[event].value.emergency_error = (EmergData*)calloc(1, sizeof(EmergData));
				if (mon_data->proc_data[proc].event_data[event].value.emergency_error == NULL) return -1;
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.emergency_error->error_code, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.emergency_error->error_code));
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len));
				if (mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len > 0)
				{
					mon_data->proc_data[proc].event_data[event].value.emergency_error->msg =
						(char*)calloc(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len + 1, sizeof(char));
					if (mon_data->proc_data[proc].event_data[event].value.emergency_error->msg == NULL) return -1;
					GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg, mon_buf, mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len);
				}
			}
			break;
			case EVENT_PROGRAM_NAME:
			{
				BYTE str_len = 0, i;
				mon_data->proc_data[proc].event_data[event].value.progname = (ProgName*)calloc(1, sizeof(ProgName));
				if (mon_data->proc_data[proc].event_data[event].value.progname == NULL) return -1;
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.progname->num, mon_buf, sizeof(BYTE));
				mon_data->proc_data[proc].event_data[event].value.progname->data = (ProgNameData*)calloc(mon_data->proc_data[proc].event_data[event].value.progname->num, sizeof(ProgNameData));
				if (mon_data->proc_data[proc].event_data[event].value.progname->data == NULL) return -1;
				for (i = 0; i < mon_data->proc_data[proc].event_data[event].value.progname->num; i++)
				{
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer, mon_buf, sizeof(BYTE));
					GetValFromBuf(&str_len, mon_buf, sizeof(str_len));
					mon_data->proc_data[proc].event_data[event].value.progname->data[i].name = (char*)calloc(str_len + 1, sizeof(char));
					if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].name == NULL) return -1;
					GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.progname->data[i].name, mon_buf, str_len);
					GetValFromBuf(&str_len, mon_buf, sizeof(str_len));
					mon_data->proc_data[proc].event_data[event].value.progname->data[i].path = (char*)calloc(str_len + 1, sizeof(char));
					if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].path == NULL) return -1;
					GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.progname->data[i].path, mon_buf, str_len);
				}
			}
			break;
			case EVENT_BLOCK_NUMB_CTRL_PROG:
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Long, mon_buf, sizeof(long));
				break;
			case EVENT_TOOL_NUMBER: case EVENT_CORRECTOR_NUMBER:
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Word, mon_buf, sizeof(WORD));
				break;
			case EVENT_MACHINE_IDLETIME_CAUSE:
				mon_data->proc_data[proc].event_data[event].value.machine_idletime = (MachineIdleTime*)calloc(1, sizeof(MachineIdleTime));
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.machine_idletime->num, mon_buf, sizeof(char));
				mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle = (Idle*)calloc(mon_data->proc_data[proc].event_data[event].value.machine_idletime->num, sizeof(Idle));

				for (int ai = 0; ai < mon_data->proc_data[proc].event_data[event].value.machine_idletime->num; ai++)
				{
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].action, mon_buf, sizeof(char));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len, mon_buf, sizeof(char));
					if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len > 0)
					{
						mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group =
							(char*)calloc(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len + 1, sizeof(char));
						if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group == NULL) return -1;
						GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group, mon_buf,
							mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len * sizeof(char));
					}
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len, mon_buf, sizeof(char));
					if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len > 0)
					{
						mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str =
							(char*)calloc(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len + 1, sizeof(char));
						if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str == NULL) return -1;
						GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str, mon_buf,
							mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len * sizeof(char));
					}
				}
				break;
			case EVENT_ALARM_PLC_ERR:
				mon_data->proc_data[proc].event_data[event].value.alarm_plc_error = (PlcError*)calloc(1, sizeof(PlcError));
				if (mon_data->proc_data[proc].event_data[event].value.alarm_plc_error == NULL) return -1;
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log_len, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log_len));
				mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log = (char*)calloc(mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log_len + 1, sizeof(char));
				if (mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log == NULL) return -1;
				GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log, mon_buf, mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log_len);
				break;
			case EVENT_MESS_PLC_ERR:
				mon_data->proc_data[proc].event_data[event].value.mess_plc_error = (PlcError*)calloc(1, sizeof(PlcError));
				if (mon_data->proc_data[proc].event_data[event].value.mess_plc_error == NULL) return -1;
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log_len, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log_len));
				mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log = (char*)calloc(mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log_len + 1, sizeof(char));
				if (mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log == NULL) return -1;
				GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log, mon_buf, mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log_len);
				break;
			case EVENT_PROCESS_COMMAND_LINE:
			case EVENT_PROCESS_BLOCK_LINE:
			case EVENT_COMMAND_LINE:
				mon_data->proc_data[proc].event_data[event].value.command_line = (CommandLine*)calloc(1, sizeof(CommandLine));
				if (mon_data->proc_data[proc].event_data[event].value.command_line == NULL) return -1;
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.command_line->len, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.command_line->len));
				mon_data->proc_data[proc].event_data[event].value.command_line->str = (char*)calloc(mon_data->proc_data[proc].event_data[event].value.command_line->len + 1, sizeof(char));
				if (mon_data->proc_data[proc].event_data[event].value.command_line->str == NULL) return -1;
				GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.command_line->str, mon_buf, mon_data->proc_data[proc].event_data[event].value.command_line->len);
				break;
			case EVENT_G_FUNCTIONS:
				mon_data->proc_data[proc].event_data[event].value.g_functions = (GFunctions*)calloc(1, sizeof(GFunctions));
				if (mon_data->proc_data[proc].event_data[event].value.g_functions == NULL) return -1;
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.g_functions->num, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.g_functions->num));
				mon_data->proc_data[proc].event_data[event].value.g_functions->g = (BYTE*)calloc(mon_data->proc_data[proc].event_data[event].value.g_functions->num, sizeof(BYTE));
				if (mon_data->proc_data[proc].event_data[event].value.g_functions->g == NULL) return -1;
				GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.g_functions->g, mon_buf, mon_data->proc_data[proc].event_data[event].value.g_functions->num * sizeof(BYTE));
				break;
			case EVENT_WNCMT: case EVENT_WNPRT: case EVENT_WPROG: case EVENT_WIZKD:
				mon_data->proc_data[proc].event_data[event].value.subroutine_info = (SubroutineInfo*)calloc(1, sizeof(SubroutineInfo));
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.subroutine_info->len, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.subroutine_info->len));
				mon_data->proc_data[proc].event_data[event].value.subroutine_info->str = (char*)calloc(mon_data->proc_data[proc].event_data[event].value.subroutine_info->len + 1, sizeof(BYTE));
				GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.subroutine_info->str, mon_buf, mon_data->proc_data[proc].event_data[event].value.subroutine_info->len);
				break;
			case EVENT_TIME_SYNCH:
				mon_data->proc_data[proc].event_data[event].value.Char = 1;
				break;
			case EVENT_ARMD_SERVICE:
				GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Char, mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.Char));
				break;
			}
		}
	}
	BYTE check = 0;
	for (DWORD ch = mon_buf->index - mon_data->events_len - sizeof(mon_data->time) - sizeof(mon_data->events_len); ch < mon_buf->index; ch++)
	{
		check ^= mon_buf->buf[ch];
	}
	GetValFromBuf(&mon_data->check, mon_buf, sizeof(mon_data->check));
	if (mon_data->check != check)
		return -1;
	return 0;
}

// Функция вывода на экран считанных структур
signed char ShowVal(MonitorData* mon_data)
{
	WORD Hours, Minutes, Seconds, Milliseconds;
	HANDLE consoleOutput;
	//	int i;
		// Получаем хэндл консоли 
	consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	ConvertTime(&Hours, &Minutes, &Seconds, &Milliseconds, mon_data->time);
	SetConsoleTextAttribute(consoleOutput, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
	printf("%.2d:%.2d:%.2d:%.4d ", Hours, Minutes, Seconds, Milliseconds);

	for (int proc = 0; proc < mon_data->num_proc; proc++)
	{

		switch (mon_data->proc_data[proc].proc)
		{
		case 0:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);									break;
		case 1:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);									break;
		case 2:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);					break;
		case 3:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);					break;
		case 4:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);					break;
		case 5:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);	break;
		}
		printf("PROCESS:%d ", mon_data->proc_data[proc].proc);
		for (int event = 0; event < mon_data->proc_data[proc].num_event; event++)
		{
			switch (mon_data->proc_data[proc].event_data[event].event)
			{
			case EVENT_NO_EVENT:
				printf("NO EVENT");
				break;
			case EVENT_SYSTEM_START:
				printf("System Start:%.4d/%.2d/%.2d ",
					mon_data->proc_data[proc].event_data[event].value.system_start_data->Year,
					mon_data->proc_data[proc].event_data[event].value.system_start_data->Month,
					mon_data->proc_data[proc].event_data[event].value.system_start_data->Day);

				ConvertTime(&Hours, &Minutes, &Seconds, &Milliseconds, mon_data->time);
				printf("%d:%d:%d:%d \n", Hours, Minutes, Seconds, Milliseconds);

				printf("Characterization files: ");
				for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num; i++)
				{
					printf("%s ", mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name);
					printf("%s", mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name);
					printf("%s, ", mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination);
				}

				break;
			case  EVENT_NEW_DATE:
				printf("NEW DATE:%.4d/%.2d/%.2d ", mon_data->proc_data[proc].event_data[event].value.time[0],
					mon_data->proc_data[proc].event_data[event].value.time[1], mon_data->proc_data[proc].event_data[event].value.time[2]);
				break;
			case EVENT_WORK_MODE:
				printf("WORK MODE:%s ", WORK_MODE_ST[mon_data->proc_data[proc].event_data[event].value.Char - 1]);
				break;
			case EVENT_FEED:
				printf("FEED:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
			case EVENT_SPINDLE_SPEED:
				printf("SPINDLE SPEED:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
			case EVENT_SYSTEM_STATE:
				printf("SYS STATE:%s ", SYSTEM_STATE_ST[mon_data->proc_data[proc].event_data[event].value.Char]);
				break;
			case EVENT_MES_ERR_PROG:
				printf("MesErrProg:%d ", mon_data->proc_data[proc].event_data[event].value.emergency_error->error_code);
				if (mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len != 0)
					printf("MSG:%s ", mon_data->proc_data[proc].event_data[event].value.emergency_error->msg);
				break;
			case EVENT_PROGRAM_NAME:
				for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.progname->num; i++)
				{
					if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == ROUTINE) printf("ROUTINE ");
					else
						if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == SUBROUTINE1) printf("SUBROUTINE 1 ");
						else
							if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == SUBROUTINE2) printf("SUBROUTINE 2 ");
					printf("NAME:%s ", mon_data->proc_data[proc].event_data[event].value.progname->data[i].name);
					printf("PATH:%s ", mon_data->proc_data[proc].event_data[event].value.progname->data[i].path);
				}
				break;
			case EVENT_CONTROL_PANEL_SWITCH_JOG:
				printf("JOG_SW:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
			case EVENT_CONTROL_PANEL_SWITCH_FEED:
				printf("FEED_SW:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
			case EVENT_CONTROL_PANEL_SWITCH_SPINDLE:
				printf("SPINDLE_SW:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
			case EVENT_BLOCK_NUMB_CTRL_PROG:
			{
				printf("NUM_BLOCK:%d ", mon_data->proc_data[proc].event_data[event].value.Long);
			}
			break;
			case EVENT_TOOL_NUMBER:
				printf("NUM_TOOL:%d ", mon_data->proc_data[proc].event_data[event].value.Word);
				break;
			case EVENT_CORRECTOR_NUMBER:
				printf("NUM_CORRECTOR:%d ", mon_data->proc_data[proc].event_data[event].value.Word);
				break;
			case EVENT_UAS:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("UAS ВКЛЮЧЕН ");
				else	printf("UAS DISABLED ");
				break;
			case EVENT_UVR:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("UVR ВКЛЮЧЕН ");
				else	printf("UVR DISABLED ");
				break;
			case EVENT_URL:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("URL ВКЛЮЧЕН ");
				else	printf("URL DISABLED ");
				break;
			case EVENT_COMU:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("COMU ВКЛЮЧЕН ");
				else	printf("COMU DISABLED ");
				break;
			case EVENT_CEFA:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("CEFA ВКЛЮЧЕН ");
				else	printf("CEFA DISABLED ");
				break;
			case EVENT_MUSP:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("MUSP ВКЛЮЧЕН ");
				else	printf("MUSP DISABLED ");
				break;
			case EVENT_REAZ:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("REAZ ВКЛЮЧЕН ");
				else	printf("REAZ DISABLED ");
				break;
			case EVENT_MACHINE_IDLETIME_CAUSE:
				for (int ai = 0; ai < mon_data->proc_data[proc].event_data[event].value.machine_idletime->num; ai++)
				{
					if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].action == MACHINE_IDLETIME_SET)
						printf("MACHINE IDLE TIME SET: ");
					else
						printf("MACHINE IDLE TIME RESET: ");
					if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len > 0)
						printf("%s ", mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group);
					if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len > 0)
						printf("%s ", mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str);
				}
				break;
			case EVENT_ALARM_PLC_ERR:
				printf("PLC_ALARM:%s ", mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log);
				break;
			case EVENT_MESS_PLC_ERR:
				printf("PLC_MESS:%s ", mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log);
				break;
			case EVENT_PROCESS_COMMAND_LINE:
				printf("COMMAND_FROM_PROCESS:%s ", mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
			case EVENT_PROCESS_BLOCK_LINE:
				printf("BLOCK_FROM_PROCESS:%s ", mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
			case EVENT_COMMAND_LINE:
				printf("COMMAND_LINE:%s ", mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
			case EVENT_PART_FINISHED:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("PART FINISHED  ");
				break;
			case EVENT_G_FUNCTIONS:
#define NO_G 0xFF
				printf("G Functions: ");
				for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.g_functions->num; i++)
					if (mon_data->proc_data[proc].event_data[event].value.g_functions->g[i] != NO_G)
						printf("%d ", mon_data->proc_data[proc].event_data[event].value.g_functions->g[i]);
					else	printf("  ");
				printf(" ");
				break;
			case EVENT_RISP:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("RISP ВКЛЮЧЕН ");
				else	printf("RISP DISABLED ");
				break;
			case EVENT_CONP:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("CONP ВКЛЮЧЕН ");
				else	printf("CONP DISABLED ");
				break;
			case EVENT_SPEPN_REQ:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("SPEPNREQ ВКЛЮЧЕН ");
				else	printf("SPEPNREQ DISABLED ");
				break;
			case EVENT_A_SPEPN:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("ASPEPN ВКЛЮЧЕН ");
				else	printf("ASPEPN DISABLED ");
				break;
			case EVENT_WNCMT:
				printf("WNCMT:%s ", mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
				break;
			case EVENT_WNPRT:
				printf("WPRT:%s ", mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
				break;
			case EVENT_WPROG:
				printf("WPROG:%s ", mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
				break;
			case EVENT_WIZKD:
				printf("WIZKD:%s ", mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
				break;
			case EVENT_TIME_SYNCH:
				if (mon_data->proc_data[proc].event_data[event].value.Char == 1)
					printf("TIME SYNCH ");
				break;
			case EVENT_SPINDLE_POWER:
				printf("SPINDLE POWER:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
			case EVENT_ARMD_SERVICE:
				printf("SERVICE MSG CODE:%d ", mon_data->proc_data[proc].event_data[event].value.Char);
				break;

			}
		}
	}
	printf("\n");
	return 0;
}


signed char FreeData(MonitorData* mon_data)
{
	if (mon_data == NULL) return -1;
	if (mon_data->proc_data == NULL) return -1;
	for (BYTE proc = 0; proc < mon_data->num_proc; proc++)
	{
		if (mon_data->proc_data[proc].event_data != NULL)
		{
			for (short event = 0; event < mon_data->proc_data[proc].num_event; event++)
			{
				switch (mon_data->proc_data[proc].event_data[event].event)
				{
				case EVENT_NO_EVENT:
					break;
				case EVENT_SYSTEM_START:
					if (mon_data->proc_data[proc].event_data[event].value.system_start_data != NULL)
					{
						if (mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data != NULL)
						{
							for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num; i++)
							{
								free(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name);
								free(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name);
								free(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination);
							}
							free(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data);
						}
						free(mon_data->proc_data[proc].event_data[event].value.system_start_data);
					}
					break;
				case  EVENT_NEW_DATE:
					if (mon_data->proc_data[proc].event_data[event].value.time != NULL) free(mon_data->proc_data[proc].event_data[event].value.time);
					break;
				case EVENT_WORK_MODE: case EVENT_SYSTEM_STATE: case EVENT_UAS: case EVENT_UVR:case EVENT_URL: case EVENT_COMU:
				case EVENT_CEFA: case EVENT_MUSP: case EVENT_REAZ: case EVENT_PART_FINISHED:

					break;
				case EVENT_FEED: case EVENT_SPINDLE_SPEED: case EVENT_CONTROL_PANEL_SWITCH_JOG:
				case EVENT_CONTROL_PANEL_SWITCH_FEED: case EVENT_CONTROL_PANEL_SWITCH_SPINDLE:case EVENT_SPINDLE_POWER:

					break;
				case EVENT_MES_ERR_PROG:
					if (mon_data->proc_data[proc].event_data[event].value.emergency_error)
					{
						if (mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len > 0)
						{
							if (mon_data->proc_data[proc].event_data[event].value.emergency_error->msg != NULL)
								free(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg);
						}
						free(mon_data->proc_data[proc].event_data[event].value.emergency_error);
					}
					break;
				case EVENT_PROGRAM_NAME:
				{
					if (mon_data->proc_data[proc].event_data[event].value.progname != NULL)
					{
						if (mon_data->proc_data[proc].event_data[event].value.progname->data != NULL)
						{
							for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.progname->num; i++)
							{
								if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].name != NULL)
									free(mon_data->proc_data[proc].event_data[event].value.progname->data[i].name);
								if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].path != NULL)
									free(mon_data->proc_data[proc].event_data[event].value.progname->data[i].path);
							}
							free(mon_data->proc_data[proc].event_data[event].value.progname->data);
						}
						free(mon_data->proc_data[proc].event_data[event].value.progname);
					}
				}
				break;
				case EVENT_BLOCK_NUMB_CTRL_PROG:
					break;
				case EVENT_TOOL_NUMBER: case EVENT_CORRECTOR_NUMBER:
					break;
				case EVENT_MACHINE_IDLETIME_CAUSE:
					if (mon_data->proc_data[proc].event_data[event].value.machine_idletime != NULL)
					{
						if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle != NULL)
						{
							for (int ai = 0; ai < mon_data->proc_data[proc].event_data[event].value.machine_idletime->num; ai++)
							{
								if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len > 0)
								{
									if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group != NULL)
										free(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group);
								}
								if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len > 0)
								{
									if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str != NULL)
										free(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str);
								}
							}
							free(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle);
						}
						free(mon_data->proc_data[proc].event_data[event].value.machine_idletime);
					}
					break;
				case EVENT_ALARM_PLC_ERR:
					if (mon_data->proc_data[proc].event_data[event].value.alarm_plc_error != NULL)
					{
						if (mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log != NULL)
							free(mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log);
						free(mon_data->proc_data[proc].event_data[event].value.alarm_plc_error);
					}
					break;
				case EVENT_MESS_PLC_ERR:
					if (mon_data->proc_data[proc].event_data[event].value.mess_plc_error != NULL)
					{
						if (mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log != NULL)
							free(mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log);
						free(mon_data->proc_data[proc].event_data[event].value.mess_plc_error);
					}
					break;
				case EVENT_PROCESS_COMMAND_LINE:
				case EVENT_PROCESS_BLOCK_LINE:
				case EVENT_COMMAND_LINE:
					if (mon_data->proc_data[proc].event_data[event].value.command_line != NULL)
					{
						if (mon_data->proc_data[proc].event_data[event].value.command_line->str != NULL)
							free(mon_data->proc_data[proc].event_data[event].value.command_line->str);
						free(mon_data->proc_data[proc].event_data[event].value.command_line);
					}
					break;
				case EVENT_G_FUNCTIONS:
					if (mon_data->proc_data[proc].event_data[event].value.g_functions != NULL)
					{
						if (mon_data->proc_data[proc].event_data[event].value.g_functions->g != NULL)
							free(mon_data->proc_data[proc].event_data[event].value.g_functions->g);
						free(mon_data->proc_data[proc].event_data[event].value.g_functions);
					}
					break;
				case EVENT_RISP:
					break;
				case EVENT_CONP:
					break;
				case EVENT_SPEPN_REQ:
					break;
				case EVENT_A_SPEPN:
					break;
				case EVENT_WNCMT: case EVENT_WNPRT: case EVENT_WPROG: case EVENT_WIZKD:
					free(mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
					free(mon_data->proc_data[proc].event_data[event].value.subroutine_info);
					break;
				case EVENT_TIME_SYNCH:
					break;
				case EVENT_ARMD_SERVICE:
					break;

				}
			}
			free(mon_data->proc_data[proc].event_data);
		}
	}
	free(mon_data->proc_data);
	return 0;
}

signed char RecordVal(FILE* file_name, BYTE proc, DWORD event, MonitorData* mon_data, HeaderMonInfo* mon_header, BYTE* record_flag, int* rec_proc, WORD* control_flags)
{
	WORD Hours, Minutes, Seconds, Milliseconds;

	ConvertTime(&Hours, &Minutes, &Seconds, &Milliseconds, mon_data->time);
	if ((*record_flag) == 0)
	{
		fprintf(file_name, "\n");
		fprintf(file_name, "%.2d:%.2d:%.2d:%.4d ", Hours, Minutes, Seconds, Milliseconds);
	}
	if ((*rec_proc) != mon_data->proc_data[proc].proc)
	{
		fprintf(file_name, "ПРОЦЕСС:%d ", mon_data->proc_data[proc].proc);
		(*rec_proc) = mon_data->proc_data[proc].proc;
	}

	switch (mon_data->proc_data[proc].event_data[event].event)
	{
	case EVENT_NO_EVENT:
		fprintf(file_name, "НЕТ СОБЫТИЯ");
		break;
	case EVENT_SYSTEM_START:
		fprintf(file_name, "ВЛЮЧЕНИЕ СИСТЕМЫ:%.4d/%.2d/%.2d ",
			mon_data->proc_data[proc].event_data[event].value.system_start_data->Year,
			mon_data->proc_data[proc].event_data[event].value.system_start_data->Month,
			mon_data->proc_data[proc].event_data[event].value.system_start_data->Day);

		ConvertTime(&Hours, &Minutes, &Seconds, &Milliseconds, mon_data->time);
		fprintf(file_name, "%d:%d:%d:%d \n", Hours, Minutes, Seconds, Milliseconds);

		fprintf(file_name, "ФАЙЛЫ ХАРАКТЕРИЗАЦИИ: ");
		for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num; i++)
		{
			fprintf(file_name, "%s ", mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name);
			fprintf(file_name, "%s", mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name);
			fprintf(file_name, "%s, ", mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination);
		}

		break;
	case  EVENT_NEW_DATE:
		fprintf(file_name, "НОВАЯ ДАТА:%.4d/%.2d/%.2d ", mon_data->proc_data[proc].event_data[event].value.time[0],
			mon_data->proc_data[proc].event_data[event].value.time[1], mon_data->proc_data[proc].event_data[event].value.time[2]);
		break;
	case EVENT_WORK_MODE:
		fprintf(file_name, "РЕЖИМ РАБОТЫ:%s ", WORK_MODE_ST[mon_data->proc_data[proc].event_data[event].value.Char - 1]);
		break;
	case EVENT_FEED:
		fprintf(file_name, "ПОДАЧА:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
		break;
	case EVENT_SPINDLE_SPEED:
		fprintf(file_name, "СКОРОСТЬ ШПИНДЕЛЯ:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
		break;
	case EVENT_SYSTEM_STATE:
		fprintf(file_name, "СОСТОЯНИЕ СИСИТЕМЫ:%s ", SYSTEM_STATE_ST[mon_data->proc_data[proc].event_data[event].value.Char]);
		break;
	case EVENT_MES_ERR_PROG:
		fprintf(file_name, "СООБЩЕНИЯ ОБ ОШИБКАХ:%d ", mon_data->proc_data[proc].event_data[event].value.emergency_error->error_code);
		if (mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len != 0)
			fprintf(file_name, "СООБЩЕНИЕ:%s ", mon_data->proc_data[proc].event_data[event].value.emergency_error->msg);
		break;
	case EVENT_PROGRAM_NAME:
		for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.progname->num; i++)
		{
			if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == ROUTINE) fprintf(file_name, "ПРОГРАММА ");
			else
				if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == SUBROUTINE1) fprintf(file_name, "ПОДПРОГРАММА 1 ");
				else
					if (mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == SUBROUTINE2) fprintf(file_name, "ПОДПРОГРАММА 2 ");
			fprintf(file_name, "ИМЯ УП:%s ", mon_data->proc_data[proc].event_data[event].value.progname->data[i].name);
			fprintf(file_name, "ПУТЬ:%s ", mon_data->proc_data[proc].event_data[event].value.progname->data[i].path);
		}
		break;
	case EVENT_CONTROL_PANEL_SWITCH_JOG:
		fprintf(file_name, "ПЕРЕКЛЮЧАТЕЛЬ JOG:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
		break;
	case EVENT_CONTROL_PANEL_SWITCH_FEED:
		fprintf(file_name, "ПЕРЕКЛЮЧАТЕЛЬ F:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
		break;
	case EVENT_CONTROL_PANEL_SWITCH_SPINDLE:
		fprintf(file_name, "ПЕРЕКЛЮЧАТЕЛЬ S:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
		break;
	case EVENT_BLOCK_NUMB_CTRL_PROG:
	{
		fprintf(file_name, "НОМЕР КАДРА УП:%d ", mon_data->proc_data[proc].event_data[event].value.Long);
	}
	break;
	case EVENT_TOOL_NUMBER:
		fprintf(file_name, "НОМЕР ИНСТРУМЕНТА:%d ", mon_data->proc_data[proc].event_data[event].value.Word);
		break;
	case EVENT_CORRECTOR_NUMBER:
		fprintf(file_name, "НОМЕР КОРРЕКТОРА:%d ", mon_data->proc_data[proc].event_data[event].value.Word);
		break;
	case EVENT_UAS:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "UAS ВКЛЮЧЕН ");
		else	fprintf(file_name, "UAS ВЫКЛЮЧЕН ");
		break;
	case EVENT_UVR:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "UVR ВКЛЮЧЕН ");
		else	fprintf(file_name, "UVR ВЫКЛЮЧЕН ");
		break;
	case EVENT_URL:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "URL ВКЛЮЧЕН ");
		else	fprintf(file_name, "URL ВЫКЛЮЧЕН ");
		break;
	case EVENT_COMU:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "COMU ВКЛЮЧЕН ");
		else	fprintf(file_name, "COMU ВЫКЛЮЧЕН ");
		break;
	case EVENT_CEFA:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "CEFA ВКЛЮЧЕН ");
		else	fprintf(file_name, "CEFA ВЫКЛЮЧЕН ");
		break;
	case EVENT_MUSP:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "MUSP ВКЛЮЧЕН ");
		else	fprintf(file_name, "MUSP ВЫКЛЮЧЕН ");
		break;
	case EVENT_REAZ:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "REAZ ВКЛЮЧЕН ");
		else	fprintf(file_name, "REAZ ВЫКЛЮЧЕН ");
		break;
	case EVENT_MACHINE_IDLETIME_CAUSE:
		for (int ai = 0; ai < mon_data->proc_data[proc].event_data[event].value.machine_idletime->num; ai++)
		{
			if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].action == MACHINE_IDLETIME_SET)
				fprintf(file_name, "ПРИЧИНА ПРОСТОЯ СТАНКА УСТАНОВЛЕНА: ");
			else
				fprintf(file_name, "ПРИЧИНА ПРОСТОЯ СТАНКА СНЯТА: ");
			if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len > 0)
				fprintf(file_name, "%s ", mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group);
			if (mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len > 0)
				fprintf(file_name, "%s ", mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str);
		}
		break;
	case EVENT_ALARM_PLC_ERR:
		fprintf(file_name, "СООБЩШЕНИЯ PLC:%s ", mon_data->proc_data[proc].event_data[event].value.alarm_plc_error->log);
		break;
	case EVENT_MESS_PLC_ERR:
		fprintf(file_name, "PLC_ALARM:%s ", mon_data->proc_data[proc].event_data[event].value.mess_plc_error->log);
		break;
	case EVENT_PROCESS_COMMAND_LINE:
		fprintf(file_name, "КОМАНДА ИЗ СТРОКИ ПРОЦЕССА:%s ", mon_data->proc_data[proc].event_data[event].value.command_line->str);
		break;
	case EVENT_PROCESS_BLOCK_LINE:
		fprintf(file_name, "КАДР ИЗ СТРОКИ ПРОЦЕССА:%s ", mon_data->proc_data[proc].event_data[event].value.command_line->str);
		break;
	case EVENT_COMMAND_LINE:
		fprintf(file_name, "КОМАНДА ИЗ СТРОКИ КОМАНДА:%s ", mon_data->proc_data[proc].event_data[event].value.command_line->str);
		break;
	case EVENT_PART_FINISHED:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "ДЕТАЛЬ ВЫПОЛНЕНА ");
		break;
	case EVENT_G_FUNCTIONS:
#define NO_G 0xFF
		fprintf(file_name, "G ФУНКЦИИ: ");
		for (int i = 0; i < mon_data->proc_data[proc].event_data[event].value.g_functions->num; i++)
			if (mon_data->proc_data[proc].event_data[event].value.g_functions->g[i] != NO_G)
				fprintf(file_name, "%d ", mon_data->proc_data[proc].event_data[event].value.g_functions->g[i]);
			else	fprintf(file_name, "  ");
		fprintf(file_name, " ");
		break;
	case EVENT_RISP:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "RISP ВКЛЮЧЕН ");
		else	fprintf(file_name, "RISP ВЫКЛЮЧЕН ");
		break;
	case EVENT_CONP:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "CONP ВКЛЮЧЕН ");
		else	fprintf(file_name, "CONP ВЫКЛЮЧЕН ");
		break;
	case EVENT_SPEPN_REQ:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "SPEPNREQ ВКЛЮЧЕН ");
		else	fprintf(file_name, "SPEPNREQ ВЫКЛЮЧЕН ");
		break;
	case EVENT_A_SPEPN:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name, "ASPEPN ВКЛЮЧЕН ");
		else	fprintf(file_name, "ASPEPN ВЫКЛЮЧЕН ");
		break;
	case EVENT_WNCMT:
		fprintf(file_name, "WNCMT:%s ", mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
		break;
	case EVENT_WNPRT:
		fprintf(file_name, "WPRT:%s ", mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
		break;
	case EVENT_WPROG:
		fprintf(file_name, "WPROG:%s ", mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
		break;
	case EVENT_WIZKD:
		fprintf(file_name, "WIZKD:%s ", mon_data->proc_data[proc].event_data[event].value.subroutine_info->str);
		break;
	case EVENT_TIME_SYNCH:
		if (mon_data->proc_data[proc].event_data[event].value.Char == 1)
			fprintf(file_name, "СИНХРОНИЗАЦИЯ ВРЕМЕНИ ");
		break;
	case EVENT_SPINDLE_POWER:
		fprintf(file_name, "МОЩНОСТЬ ШПИНДЕЛЯ:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
		break;
	case EVENT_ARMD_SERVICE:
		fprintf(file_name, "СЕРВИСНЫЙ КОД АРМД:%d ", mon_data->proc_data[proc].event_data[event].value.Char);
		break;

	}
	(*record_flag) = 1;
	return 0;
}


DWORD FindEvent(BYTE proc, WORD event, WORD num_event, MonEventFilter* mon_event_filter)
{
	int i;
	for (i = 0; i < mon_event_filter->num_filter_events[proc]; i++)
	{
		if (num_event == mon_event_filter->proc_filter[proc][i])
			return num_event;
	}
	return 0xFFFFFFFF;
}



BYTE FindNullProc(MonitorData* mon_data)
{
	for (BYTE i = 0; i < mon_data->num_proc; i++)
		if (mon_data->proc_data[i].proc == 0) return i;
	return 0xFF;
}

signed char FilterEvents(FILE** file_name_in, MonitorData* mon_data, HeaderMonInfo* mon_header, MonEventFilter* mon_event_filter, WORD* control_flags)
{
	BYTE proc;
	DWORD event;
	int rec_prog;
	BYTE record_event_flag = 0;
	FILE* file_name;
	BYTE find_proc;
	if ((*control_flags) & SEPARATE_PROC)
	{
		for (proc = 0; proc < mon_data->num_proc; proc++)
		{
			find_proc = FindNullProc(mon_data);
			if (find_proc == proc) continue;
			file_name = file_name_in[mon_data->proc_data[proc].proc];
			if (file_name == NULL) return -1;
			record_event_flag = 0;
			if ((*control_flags) & WRITE_HEADER)
			{
				RecordHeader(file_name, mon_header, control_flags);
				RecordProcHeader(file_name, 0, mon_header, mon_event_filter, control_flags);
				RecordProcHeader(file_name, mon_data->proc_data[proc].proc, mon_header, mon_event_filter, control_flags);
			}
			if (find_proc != 0xFF)
			{
				rec_prog = -1;
				for (event = 0; event < mon_data->proc_data[find_proc].num_event; event++)
				{
					if (FindEvent(mon_data->proc_data[find_proc].proc, event, mon_data->proc_data[find_proc].event_data[event].event, mon_event_filter) != 0xFFFFFFFF)
					{
						RecordVal(file_name, find_proc, event, mon_data, mon_header, &record_event_flag, &rec_prog, control_flags);
					}
				}
			}
			rec_prog = -1;
			for (event = 0; event < mon_data->proc_data[proc].num_event; event++)
			{
				if (FindEvent(mon_data->proc_data[proc].proc, event, mon_data->proc_data[proc].event_data[event].event, mon_event_filter) != 0xFFFFFFFF)
				{
					RecordVal(file_name, proc, event, mon_data, mon_header, &record_event_flag, &rec_prog, control_flags);
				}
			}
		}
		first_record = 1;
		if ((*control_flags) & WRITE_HEADER)
			(*control_flags) &= ~WRITE_HEADER;

	}
	else
	{
		file_name = file_name_in[0];
		for (proc = 0; proc < mon_data->num_proc; proc++)
		{
			rec_prog = -1;
			for (event = 0; event < mon_data->proc_data[proc].num_event; event++)
			{
				if (FindEvent(mon_data->proc_data[proc].proc, event, mon_data->proc_data[proc].event_data[event].event, mon_event_filter) != 0xFFFFFFFF)
				{
					if ((*control_flags) & WRITE_HEADER)
					{
						RecordHeader(file_name, mon_header, control_flags);
						//	RecordProcHeader(file_name,proc,mon_header,control_flags);
						fprintf(file_name, "\n----------------------ПАРАМЕТРЫ АРМД-------------------------------------------");
						for (int ii = 0; ii < mon_data->num_proc; ii++)
						{
							RecordProcHeader(file_name, mon_data->proc_data[ii].proc, mon_header, mon_event_filter, control_flags);
						}
						fprintf(file_name, "----------------------СОБЫТИЯ--------------------------------------------------");
						(*control_flags) &= ~WRITE_HEADER;
					}
					RecordVal(file_name, proc, event, mon_data, mon_header, &record_event_flag, &rec_prog, control_flags);
				}
			}
		}
		first_record = 1;
	}
	return 0;
}
