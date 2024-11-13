// Monitor1.cpp : Defines the entry point for the console application.
//
//ДЛЯ ОТЛАДКИ!!! ПРИМЕР ЧТЕНИЯ ФАЙЛА МОНИТОРИНГА!!!
#include "stdafx.h"
/*
#include "windows.h"
#include <math.h>
#include <malloc.h>
#include <conio.h>
#include <stddef.h>
#include <time.h>
*/
#include "monitorarmd.h"

signed char ReadMonitorFile(char *pr_name,MonBuf *mon_buf,DWORD *OldFileLen,DWORD *BytesRead)
{
		HANDLE pFile;
		DWORD tmp;
		if(BytesRead == NULL) BytesRead=&tmp;
		mon_buf->index=0;
		//открывать файл лучше с флагом FILE_SHARE_READ, но здесь есть проблема с совместным доступом к файлу со стороны УЧПУ и данной программы
		//связано это или с сетью или с конфликтом операционных систем, т.к. УЧПУ управляет On-Time RTOS-32
		//для того, чтобы УЧПУ могло получить доступ к файлу как можно быстрее желательно открывать файл на небольшой промежуток времени
		//иначе может произойти переполнение буфера. При возможности лучше не открывать файл, с которым работает УЧПУ.
        pFile = CreateFile(pr_name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if (pFile == INVALID_HANDLE_VALUE)
		{
			return -1;
		} 
		//т.к. событие "нет события" затирается следующим событием причем это может быть повторное событие "нет события" 
		//с новым временем появления, которое возникает в системе, если не происходило никаких изменений в значениях мониторируемых переменных, 
		//то необходимо вернуться на позицию в файле до события "нет события". Период возникновения события "нет события"
		//(запись на диск накопленного буффера) устанавливается инструкцией RecordTime в файле MonCfg.ini.
		if(OldFileLen != NULL)
			SetFilePointer(pFile, (*OldFileLen), NULL, FILE_BEGIN);//переместиться на позицию, с которой будет считываться файл
		ReadFile(pFile, mon_buf->buf, mon_buf->buf_size,BytesRead,NULL) ; //чтение файла
		CloseHandle(pFile);
		return 0;
}

//----Начало секции поиска файла---------------------------------------------------------------------------------------------------------------//
void FindQueueBegin(char *file_name,MonBuf *mon_buf)
{
	HeaderMonInfo *mon_header=NULL;
	BYTE stop=0;
	do{
		ReadMonitorFile(file_name,mon_buf,NULL,NULL);
		LoadHeader(&mon_header,mon_buf);
		if(mon_header->prev_file_name[0] != 0) 
		{
			memcpy(file_name,mon_header->prev_file_name,MAX_MON_FILE_NAME);
			file_name[MAX_MON_FILE_NAME+1]=0;
		}
		else
		{
			stop=1;
		}
		FreeHeader(&mon_header);
	}while(stop == 0);

}

void GetFileInfo(char *file_name,MonBuf *mon_buf,MonFileInfo *file_info)
{
	HeaderMonInfo *mon_header=NULL;
	MonitorData *mon_data=(MonitorData*)calloc(1,sizeof(MonitorData));
	BYTE stop=0;
	WORD control_flags=0;

	ReadMonitorFile(file_name,mon_buf,NULL,NULL);
	LoadHeader(&mon_header,mon_buf);
	GetEventData(mon_data,mon_buf,&control_flags);
	strcpy(file_info->name,file_name);

	file_info->time[0]=mon_header->SysTime.wYear;
	file_info->time[1]=mon_header->SysTime.wMonth;
	file_info->time[2]=mon_header->SysTime.wDay;
	ConvertTime(&file_info->time[3],&file_info->time[4],&file_info->time[5],&file_info->time[6], mon_data->time);

	if(mon_header->next_file_name[0] != 0) 
	{
		memcpy(file_name,mon_header->next_file_name,MAX_MON_FILE_NAME);
		file_name[MAX_MON_FILE_NAME+1]=0;
	}
	else
		memset(file_name,0,MAX_MON_FILE_NAME+1);
	FreeHeader(&mon_header);
	FreeData(mon_data);
	free(mon_data);
}


MonFileQueueInfo *BuildQueue(char *file_name) 
{
	//MonFileQueue *mon_qeueue_first=NULL,*mon_qeueue_last=NULL,*mon_qeueue_curr=NULL;
	MonFileQueueInfo *mon_file_queue_info=NULL;
	HeaderMonInfo *mon_header=NULL;
	MonBuf mon_buf;
	MonitorData mon_data;
	WORD control_flag=0;
	MonFileInfo *file_info=NULL;
	mon_file_queue_info=(MonFileQueueInfo*)calloc(1,sizeof(MonFileQueueInfo));
	GetMonBuf(&mon_buf,100000);
	FindQueueBegin(file_name,&mon_buf);
	do{
		file_info=(MonFileInfo*)calloc(1,sizeof(MonFileInfo));
		GetFileInfo(file_name,&mon_buf,file_info);
		if(mon_file_queue_info->curr == NULL)
		{
			mon_file_queue_info->curr=(MonFileQueue*)calloc(1,sizeof(MonFileQueue));
			mon_file_queue_info->curr->file_info=file_info;
			mon_file_queue_info->first=mon_file_queue_info->curr;
		}
		else
		{
			if(mon_file_queue_info->curr->next == NULL)
			{
				mon_file_queue_info->curr->next=(MonFileQueue*)calloc(1,sizeof(MonFileQueue));
				mon_file_queue_info->curr->next->prev=mon_file_queue_info->curr;
				mon_file_queue_info->curr->next->file_info=file_info;
				mon_file_queue_info->curr=mon_file_queue_info->curr->next;
			}
		}
	}while(file_name[0] != 0);
	mon_file_queue_info->last=mon_file_queue_info->curr;
	return mon_file_queue_info;
}

#define DATE_EQUAL 0
#define DATE_FIRST_LATER	1
#define DATE_SECOND_LATER	2

signed char WhichDateLater(WORD *date1, WORD *date2)
{
	int i;
	for(i=0; i < 7; i ++)
	{
		if(date1[i] < date2[i])
		{
			return DATE_SECOND_LATER;
		}
		else
			if(date1[i] > date2[i])
				return DATE_FIRST_LATER;
	}
	if(date1[6] == date2[6]) 
		return DATE_EQUAL;
	return 0;
}
/*
signed char CmpDateLower(WORD *date1, WORD *date2)
{
	int i;
	for(i=0; i < 7; i ++)
	{
		if(date1[i] > date2[i])
		{
			return 1;
		}
		else
			if(date1[i] < date2[i])
				return 0;
	}
	if(date1[6] == date2[6]) return 2;
	return 0;
}
*/
void FindMonFile(char *file_name,MonFileQueueInfo *mon_file_queue_info, WORD *time)
{
	int i;
	BYTE tmp;
	BYTE find=0;
	MonFileQueue *mon_file_queue;
	DWORD e;
	mon_file_queue=mon_file_queue_info->first;
	do{
		tmp=WhichDateLater(mon_file_queue->file_info->time,time);
		if(tmp == DATE_SECOND_LATER)
		{
			if(mon_file_queue->prev != NULL)
				mon_file_queue_info->curr=mon_file_queue->prev;
			else
				mon_file_queue_info->curr=mon_file_queue;

			if(mon_file_queue->prev != NULL)
				strcpy(file_name,mon_file_queue->prev->file_info->name);
			else
				strcpy(file_name,mon_file_queue->file_info->name);
		}
		else
			if(tmp ==DATE_EQUAL)
			{
				if(mon_file_queue != NULL)	mon_file_queue_info->curr=mon_file_queue;

				if(mon_file_queue != NULL)	strcpy(file_name,mon_file_queue->file_info->name);
				else
					strcpy(file_name,mon_file_queue->file_info->name);
			}
			else
			{
				int gg=0;
			}
		mon_file_queue=mon_file_queue->next;
	}while(mon_file_queue != NULL);
	return;
}
//----Конец секции поиска файла----------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------------//
DWORD GetMonBuf(MonBuf *mon_buf,DWORD num_items)
{
	mon_buf->buf_size=num_items;
	mon_buf->buf=(char*)malloc(mon_buf->buf_size*sizeof(char));
	mon_buf->index=0;
	return mon_buf->buf_size;
}

void FreeMonBuf(MonBuf *mon_buf)
{
	free(mon_buf->buf);
	mon_buf->buf=NULL;
	mon_buf->buf_size=0;
	mon_buf->index=0;
}
//---------------------------------------------------------------------------------------------------------------------------------------------//
signed char ConvertTime(WORD *Hours, WORD *Minutes, WORD *Seconds, WORD *Milliseconds, DWORD time)
{
/*
	(*Hours)=time/(3600*1000*10);
	(*Minutes)=(time-(*Hours)*3600*1000*10)/(60 * 1000*10);
	(*Seconds)=(time-(*Hours)*3600*1000*10-(*Minutes)*60*1000*10)/ (1000*10);
	(*Milliseconds)=(time-(*Hours)*3600*1000*10-(*Minutes)*60*1000*10-(*Seconds)*1000*10)/10;
*/
//	if (Microseconds!= NULL) (*Microseconds)=(time%10)*100;
	time/=10;
	if (Milliseconds!= NULL) (*Milliseconds)=time%1000;
	time/=1000;
	if (Seconds != NULL)	(*Seconds)=time%60;
	time/=60;
	if (Minutes != NULL)(*Minutes)=time%60;
	time/=60;
	if (Hours != NULL)	(*Hours)=time%60;
	return 0;
}

typedef enum _control_flags{
	NO_EVENT			=	0x1,
	HEADER_LOADED		=	0x2,
	WRITE_HEADER		=	0x4
}ControlFlgs;

signed char GetValFromBuf(void *value, MonBuf *mon_buf, DWORD value_size)
{
	if(mon_buf->index >= mon_buf->buf_size) return -1;
	memcpy(value,&mon_buf->buf[mon_buf->index],value_size); 
	mon_buf->index+=value_size;
	return 0;
}
//#define str_num 10
//функция считывает заголовок и начальные значения переменных
signed char LoadHeader(HeaderMonInfo **header_mon_info_p,MonBuf *mon_buf)
{
	HeaderMonInfo *header_mon_info;
	header_mon_info=(*header_mon_info_p);

	if(header_mon_info == NULL)	header_mon_info=(HeaderMonInfo*)calloc(1,sizeof(HeaderMonInfo));
	GetValFromBuf(header_mon_info->identifier, mon_buf, IDENTIFIER_LEN);
	if(strcmp(header_mon_info->identifier,"BSMN") != 0) return -1;
	GetValFromBuf(header_mon_info->ver, mon_buf, VERSION_LEN);
	if(strcmp(header_mon_info->ver,"0000") != 0) return -1;
	GetValFromBuf(&header_mon_info->file_size, mon_buf, sizeof(header_mon_info->file_size));//точная текущая длина файла, 
												//функция GetEventData не должна обрабатывать больше байт, чем записано в данной переменной
												//т.к. в конце теоретически может появиться грязь
	GetValFromBuf(header_mon_info->prev_file_name, mon_buf, MAX_MON_FILE_NAME);//имя предыдущего файла истории, если нет поле пустое
	GetValFromBuf(header_mon_info->next_file_name, mon_buf, MAX_MON_FILE_NAME);//имя следующего файла истории, если нет поле пустое

	GetValFromBuf(&header_mon_info->software_version_len, mon_buf, sizeof(header_mon_info->software_version_len));
	header_mon_info->software_version=(char*)calloc(header_mon_info->software_version_len+1,sizeof(char));
	GetValFromBuf(header_mon_info->software_version, mon_buf, header_mon_info->software_version_len);//  "имя" станка

	GetValFromBuf(header_mon_info->machine_name, mon_buf, MAX_MON_FILE_NAME);//  "имя" станка
	GetValFromBuf(&header_mon_info->system_tick, mon_buf, sizeof(header_mon_info->system_tick));//системный тик (AXCFIL;инструкция TIM,мс)
	GetValFromBuf(&header_mon_info->record_time, mon_buf, sizeof(header_mon_info->record_time));//системный тик (AXCFIL;инструкция TIM,мс)
	GetValFromBuf(&header_mon_info->SysTime.wYear, mon_buf, sizeof(header_mon_info->SysTime.wYear));//год создания файла
	GetValFromBuf(&header_mon_info->SysTime.wMonth, mon_buf, sizeof(header_mon_info->SysTime.wMonth));//месяц
	GetValFromBuf(&header_mon_info->SysTime.wDay, mon_buf, sizeof(header_mon_info->SysTime.wDay));//день

	GetValFromBuf(&header_mon_info->num_proc, mon_buf, sizeof(header_mon_info->num_proc));//количество всех процессов участвующих в процессе мониторинга
	header_mon_info->proc_info=(ProcInfo*)calloc(header_mon_info->num_proc,sizeof(ProcInfo));
	for(short i=0; i < header_mon_info->num_proc;i++)
	{
		GetValFromBuf(&header_mon_info->proc_info[i].num_events, mon_buf, sizeof(header_mon_info->proc_info[i].num_events));//общее количество событий в процессе
		header_mon_info->proc_info[i].event_info=(SysMonInfo*)calloc(header_mon_info->proc_info[i].num_events,sizeof(SysMonInfo));
		for(short j=0; j < header_mon_info->proc_info[i].num_events; j++)
		{
			//параметры мониторинга
			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].event, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].event));
			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].account_type, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].account_type));
			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].account_param, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].account_param));
			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].units, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].units));
		}
	}

	//GetEventData(mon_data,mon_buf,control_flags);//считываем начальные параметры переменных
	(*header_mon_info_p)=header_mon_info;
	return 0;
}
//#undef str_num

signed char FreeHeader(HeaderMonInfo **header_mon_info_p)
{
	HeaderMonInfo *header_mon_info;
	header_mon_info=(*header_mon_info_p);

	free(header_mon_info->software_version);

	for(short i=0; i < header_mon_info->num_proc;i++)
		free(header_mon_info->proc_info[i].event_info);

	free(header_mon_info->proc_info);

	if(header_mon_info != NULL)	free(header_mon_info);
	header_mon_info=NULL;

	(*header_mon_info_p)=header_mon_info;
	return 0;
}


signed char ShowHeader(HeaderMonInfo *header_mon_info)
{
	printf("SOFTWARE_VERSION:%s ",header_mon_info->software_version);
	printf("MACHINE_NAME:%s SYSTEM_TICK:%d RECORD_TIME:%d ",header_mon_info->machine_name,header_mon_info->system_tick, header_mon_info->record_time);
	printf("TOTAL_PROC:%d ",header_mon_info->num_proc-1);
	printf("\n");
	for(short i=0; i < header_mon_info->num_proc;i++)
	{
		printf("\n");
		if(i!=0)printf("PROCESS:%d ",i);
		else	printf("PROCESS:%d (NO PROCESS) ",i);
		printf("NUMBER_OF_EVENTS_IN_PROCESS:%d ",header_mon_info->proc_info[i].num_events);
		for(short j=0; j < header_mon_info->proc_info[i].num_events; j++)
		{
			//параметры мониторинга
			printf("EVENT:%s ",EVENT_NAME[header_mon_info->proc_info[i].event_info[j].event]);
			printf("EVENT_TYPE:%s ",EVENT_TYPE[header_mon_info->proc_info[i].event_info[j].account_type]);
			printf("VALUE:%f ",header_mon_info->proc_info[i].event_info[j].account_param);
//			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].units, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].units));
			printf("\n");
		}
	}
	return 0;
}

signed char RecordHeader(FILE *file_name,HeaderMonInfo *header_mon_info)
{
	fprintf(file_name,"SOFTWARE_VERSION:%s ",header_mon_info->software_version);
	fprintf(file_name,"MACHINE_NAME:%s SYSTEM_TICK:%d RECORD_TIME:%d ",header_mon_info->machine_name,header_mon_info->system_tick, header_mon_info->record_time);
	fprintf(file_name,"TOTAL_PROC:%d ",header_mon_info->num_proc-1);
	fprintf(file_name,"\n");
	for(short i=0; i < header_mon_info->num_proc;i++)
	{
		fprintf(file_name,"\n");
		if(i!=0)fprintf(file_name,"PROCESS:%d ",i);
		else	fprintf(file_name,"PROCESS:%d (NO PROCESS) ",i);
		fprintf(file_name,"NUMBER_OF_EVENTS_IN_PROCESS:%d ",header_mon_info->proc_info[i].num_events);
		for(short j=0; j < header_mon_info->proc_info[i].num_events; j++)
		{
			//параметры мониторинга
			fprintf(file_name,"EVENT:%s ",EVENT_NAME[header_mon_info->proc_info[i].event_info[j].event]);
			fprintf(file_name,"EVENT_TYPE:%s ",EVENT_TYPE[header_mon_info->proc_info[i].event_info[j].account_type]);
			fprintf(file_name,"VALUE:%f ",header_mon_info->proc_info[i].event_info[j].account_param);
//			GetValFromBuf(&header_mon_info->proc_info[i].event_info[j].units, mon_buf, sizeof(header_mon_info->proc_info[i].event_info[j].units));
			fprintf(file_name,"\n");
		}
	}
	return 0;
}

//Функция считывает события из массива и распределяет их по структурам
signed char GetEventData(MonitorData *mon_data,MonBuf *mon_buf,WORD *control_flags)
{
	DWORD rem=0;
	if(mon_data == NULL) return -1;
	if(mon_buf->buf_size-mon_buf->index < sizeof(mon_data->time)+sizeof(mon_data->events_len)+sizeof(mon_data->num_proc)+sizeof(mon_data->proc_data[0].proc)+sizeof(mon_data->proc_data[0].event_data[0].event)+sizeof(mon_data->check))
	{
		return -2;
	}
	(*control_flags)&=~NO_EVENT;
	GetValFromBuf(&mon_data->time,mon_buf, sizeof(mon_data->time));
	GetValFromBuf(&mon_data->events_len,mon_buf, sizeof(mon_data->events_len));
	if(mon_data->events_len + sizeof(mon_data->check)+ mon_buf->index > mon_buf->buf_size)
	{
		mon_buf->index-=sizeof(mon_data->time)+sizeof(mon_data->events_len);
		return -2;
	}
	GetValFromBuf(&mon_data->num_proc,mon_buf, sizeof(mon_data->num_proc));
	mon_data->proc_data=(ProcMonData*)calloc(mon_data->num_proc,sizeof(ProcMonData));
	for(BYTE proc=0; proc <mon_data->num_proc;proc++)
	{
		GetValFromBuf(&mon_data->proc_data[proc].proc,mon_buf, sizeof(mon_data->proc_data[proc].proc));
		GetValFromBuf(&mon_data->proc_data[proc].num_event,mon_buf, sizeof(mon_data->proc_data[proc].num_event));
		mon_data->proc_data[proc].event_data=(EventMonData*)calloc(mon_data->proc_data[proc].num_event,sizeof(EventMonData));
		for(short event=0; event < mon_data->proc_data[proc].num_event; event++)
		{
			GetValFromBuf(&mon_data->proc_data[proc].event_data[event].event,mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].event));
			switch(mon_data->proc_data[proc].event_data[event].event)
			{
				case EVENT_NO_EVENT:
				{
					(*control_flags)|=NO_EVENT;
				}
				break;
				case EVENT_SYSTEM_START:
					mon_data->proc_data[proc].event_data[event].value.system_start_data=(SystemStartData*)calloc(1,sizeof(SystemStartData));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->Year,mon_buf,sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->Year));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->Month,mon_buf,sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->Month));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->Day,mon_buf,sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->Day));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->time,mon_buf,sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->time));
					//-----------------------------информация о файлах характеризации-----------------------------------------------------------//
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num,mon_buf,sizeof(mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num));
					mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data=(CharacterizationFileData*)calloc(mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num,sizeof(CharacterizationFileData));
					for(int i=0; i < mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num; i++)
					{
						BYTE len;
						GetValFromBuf(&len,mon_buf,sizeof(BYTE));
						mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name=(char*)calloc(len+1,sizeof(char));
						GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name,mon_buf,len);

						GetValFromBuf(&len,mon_buf,sizeof(BYTE));
						mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name=(char*)calloc(len+1,sizeof(char));
						GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name,mon_buf,len);

						GetValFromBuf(&len,mon_buf,sizeof(BYTE));
						mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination=(char*)calloc(len+1,sizeof(char));
						GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination,mon_buf,len);
					}
					//--------------------------------------------------------------------------------------------------------------------------//
				break;
				case  EVENT_NEW_DATE:
					mon_data->proc_data[proc].event_data[event].value.time=(WORD*) calloc(3,sizeof(WORD));
					GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.time,mon_buf,3*sizeof(WORD));
				break;
				case EVENT_WORK_MODE: case EVENT_SYSTEM_STATE: case EVENT_UAS: case EVENT_UVR:case EVENT_URL: case EVENT_COMU:
				case EVENT_CEFA: case EVENT_MUSP: case EVENT_REAZ: case EVENT_PART_FINISHED:case EVENT_RISP: case EVENT_CONP: 
				case EVENT_SPEPN_REQ: case EVENT_A_SPEPN: 
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Char,mon_buf, sizeof(char));
				break;
				case EVENT_FEED: case EVENT_SPINDLE_SPEED: case EVENT_CONTROL_PANEL_SWITCH_JOG:
				case EVENT_CONTROL_PANEL_SWITCH_FEED: case EVENT_CONTROL_PANEL_SWITCH_SPINDLE:
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Float,mon_buf, sizeof(float));
				break;
				case EVENT_MES_ERR_PROG:
				{
					mon_data->proc_data[proc].event_data[event].value.emergency_error=(EmergData*)calloc(1,sizeof(EmergData));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.emergency_error->error_code,mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.emergency_error->error_code));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len,mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len));
					if(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len > 0)
					{
						mon_data->proc_data[proc].event_data[event].value.emergency_error->msg=
									(char*)calloc(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len+1,sizeof(char));
						GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg,mon_buf, mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len);
					}
				}
				break;
				case EVENT_PROGRAM_NAME:
				{
					BYTE str_len=0,i;
					mon_data->proc_data[proc].event_data[event].value.progname=(ProgName*)calloc(1,sizeof(ProgName));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.progname->num,mon_buf, sizeof(BYTE));
					mon_data->proc_data[proc].event_data[event].value.progname->data=(ProgNameData*)calloc(mon_data->proc_data[proc].event_data[event].value.progname->num,sizeof(ProgNameData));
					for(i=0; i < mon_data->proc_data[proc].event_data[event].value.progname->num; i++)
					{
						GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer,mon_buf, sizeof(BYTE));
						GetValFromBuf(&str_len,mon_buf, sizeof(str_len));
						mon_data->proc_data[proc].event_data[event].value.progname->data[i].name=(char*)calloc(str_len+1,sizeof(char));
						GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.progname->data[i].name,mon_buf, str_len);
						GetValFromBuf(&str_len,mon_buf, sizeof(str_len));
						mon_data->proc_data[proc].event_data[event].value.progname->data[i].path=(char*)calloc(str_len+1,sizeof(char));
						GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.progname->data[i].path,mon_buf, str_len);
					}
				}
				break;
				case EVENT_BLOCK_NUMB_CTRL_PROG:
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Long,mon_buf,sizeof(long));
				break;
				case EVENT_TOOL_NUMBER: case EVENT_CORRECTOR_NUMBER:
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.Word,mon_buf,sizeof(WORD));
				break;
				case EVENT_MACHINE_IDLETIME_CAUSE:
					mon_data->proc_data[proc].event_data[event].value.machine_idletime=(MachineIdleTime*)calloc(1,sizeof(MachineIdleTime));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.machine_idletime->num,mon_buf,sizeof(char));
					mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle=(Idle*)calloc(mon_data->proc_data[proc].event_data[event].value.machine_idletime->num,sizeof(Idle));
					
					for(int ai=0; ai < mon_data->proc_data[proc].event_data[event].value.machine_idletime->num; ai++)
					{
						GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].action,mon_buf,sizeof(char));
						GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len,mon_buf,sizeof(char));
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len > 0)
						{
							mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group=
							(char*)calloc(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len+1,sizeof(char));
							GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group,mon_buf,
								mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len*sizeof(char));
						}
						GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len,mon_buf,sizeof(char));
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len > 0)
						{
							mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str=
								(char*)calloc(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len+1,sizeof(char));
							GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str,mon_buf,
								mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len*sizeof(char));
						}
					}
				break;
				case EVENT_PLC_ERR:
					mon_data->proc_data[proc].event_data[event].value.plc_error=(PlcError*)calloc(1,sizeof(PlcError));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log_len,mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log_len));
					mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log=(char*)calloc(mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log_len+1,sizeof(char));
					GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log,mon_buf,mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log_len);

					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log_len,mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log_len));
					mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log=(char*)calloc(mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log_len+1,sizeof(char));
					GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log,mon_buf,mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log_len);
				break;
				case EVENT_PROCESS_COMMAND_LINE: 
				case EVENT_PROCESS_BLOCK_LINE:
				case EVENT_COMMAND_LINE:
					mon_data->proc_data[proc].event_data[event].value.command_line=(CommandLine*)calloc(1,sizeof(CommandLine));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.command_line->len,mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.command_line->len));
					mon_data->proc_data[proc].event_data[event].value.command_line->str=(char*)calloc(mon_data->proc_data[proc].event_data[event].value.command_line->len+1,sizeof(char));
					GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.command_line->str,mon_buf, mon_data->proc_data[proc].event_data[event].value.command_line->len);
				break;
				case EVENT_G_FUNCTIONS:
					mon_data->proc_data[proc].event_data[event].value.g_functions=(GFunctions*)calloc(1,sizeof(GFunctions));
					GetValFromBuf(&mon_data->proc_data[proc].event_data[event].value.g_functions->num,mon_buf, sizeof(mon_data->proc_data[proc].event_data[event].value.g_functions->num));
					mon_data->proc_data[proc].event_data[event].value.g_functions->g=(BYTE*)calloc(mon_data->proc_data[proc].event_data[event].value.g_functions->num,sizeof(BYTE));
					GetValFromBuf(mon_data->proc_data[proc].event_data[event].value.g_functions->g,mon_buf, mon_data->proc_data[proc].event_data[event].value.g_functions->num*sizeof(BYTE));
				break;
			}
		}
	}
	BYTE check=0;
	for(DWORD ch=mon_buf->index-mon_data->events_len-sizeof(mon_data->time)-sizeof(mon_data->events_len); ch < mon_buf->index; ch++)
	{
		check^=mon_buf->buf[ch];
	}
	GetValFromBuf(&mon_data->check,mon_buf, sizeof(mon_data->check));
	if(mon_data->check != check) return -1;
	return 0;
}

// Функция вывода на экран считанных структур
signed char ShowVal(MonitorData *mon_data)
{
	WORD Hours, Minutes, Seconds, Milliseconds;
	HANDLE consoleOutput;
//	int i;
	// Получаем хэндл консоли 
	consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE); 
	ConvertTime(&Hours, &Minutes, &Seconds, &Milliseconds, mon_data->time);
	SetConsoleTextAttribute(consoleOutput, FOREGROUND_BLUE|FOREGROUND_RED |FOREGROUND_GREEN);
	printf("%.2d:%.2d:%.2d:%.4d ",Hours,Minutes,Seconds,Milliseconds);

	for(int proc=0; proc < mon_data->num_proc; proc++)
	{

		switch(mon_data->proc_data[proc].proc)	
		{
			case 0:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_GREEN |FOREGROUND_INTENSITY);									break;
			case 1:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_RED |FOREGROUND_INTENSITY);									break;
			case 2:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_BLUE|FOREGROUND_RED |FOREGROUND_INTENSITY);					break;
			case 3:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_RED |FOREGROUND_GREEN|FOREGROUND_INTENSITY);					break;
			case 4:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_BLUE |FOREGROUND_GREEN|FOREGROUND_INTENSITY);					break;
			case 5:	SetConsoleTextAttribute(consoleOutput, FOREGROUND_BLUE|FOREGROUND_RED |FOREGROUND_GREEN|FOREGROUND_INTENSITY);	break;
		}
		printf("PROCESS:%d ",mon_data->proc_data[proc].proc);
		for(int event=0; event<mon_data->proc_data[proc].num_event; event++)
		{
			switch(mon_data->proc_data[proc].event_data[event].event)
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
						printf("%d:%d:%d:%d \n",Hours,Minutes,Seconds,Milliseconds);

						printf("Characterization files: ");
						for(int i=0; i < mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num; i++)
						{
							printf("%s ",mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name);
							printf("%s",mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name);
							printf("%s, ",mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination);
						}

				break;
				case  EVENT_NEW_DATE:
					printf("NEW DATE:%.4d/%.2d/%.2d ",mon_data->proc_data[proc].event_data[event].value.time[0],
					mon_data->proc_data[proc].event_data[event].value.time[1],mon_data->proc_data[proc].event_data[event].value.time[2]);
				break;
				case EVENT_WORK_MODE:
					printf("WORK MODE:%s ",WORK_MODE_ST[mon_data->proc_data[proc].event_data[event].value.Char-1]);
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
					if(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len != 0)
						printf("MSG:%s ", mon_data->proc_data[proc].event_data[event].value.emergency_error->msg);
				break;
				case EVENT_PROGRAM_NAME:
					for(int i=0; i < mon_data->proc_data[proc].event_data[event].value.progname->num; i++)
					{
						if(mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == ROUTINE) printf("ROUTINE ");
						else
							if(mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == SUBROUTINE1) printf("SUBROUTINE 1 ");
							else
								if(mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == SUBROUTINE2) printf("SUBROUTINE 2 ");
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
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("UAS ENABLED ");
					else	printf("UAS DISABLED ");
				break;
				case EVENT_UVR:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("UVR ENABLED ");
					else	printf("UVR DISABLED ");
				break;
				case EVENT_URL:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("URL ENABLED ");
					else	printf("URL DISABLED ");
				break;
				case EVENT_COMU:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("COMU ENABLED ");
					else	printf("COMU DISABLED ");
				break;
				case EVENT_CEFA:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("CEFA ENABLED ");
					else	printf("CEFA DISABLED ");
				break;
				case EVENT_MUSP:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("MUSP ENABLED ");
					else	printf("MUSP DISABLED ");
				break;
				case EVENT_REAZ:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("REAZ ENABLED ");
					else	printf("REAZ DISABLED ");
				break;
				case EVENT_MACHINE_IDLETIME_CAUSE:
					for(int ai=0; ai < mon_data->proc_data[proc].event_data[event].value.machine_idletime->num; ai++)
					{
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].action == MACHINE_IDLETIME_SET)
							printf("MACHINE IDLE TIME SET: ");
						else
							printf("MACHINE IDLE TIME RESET: ");
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len > 0)
							printf("%s ",mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group);
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len > 0)
							printf("%s ",mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str);
					}
				break;
				case EVENT_PLC_ERR:
					printf("PLC_MESS:%s ",mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log);
					printf("PLC_ALARM:%s ",mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log);
				break;
				case EVENT_PROCESS_COMMAND_LINE:
					printf("COMMAND_FROM_PROCESS:%s ",mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
				case EVENT_PROCESS_BLOCK_LINE:
					printf("BLOCK_FROM_PROCESS:%s ",mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
				case EVENT_COMMAND_LINE:
					printf("COMMAND_LINE:%s ",mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
				case EVENT_PART_FINISHED:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("PART FINISHED  ");
				break;
				case EVENT_G_FUNCTIONS:
#define NO_G 0xFF
					printf("G Functions: ");
					for(int i=0; i < mon_data->proc_data[proc].event_data[event].value.g_functions->num; i++)
						if(mon_data->proc_data[proc].event_data[event].value.g_functions->g[i] != NO_G)
							printf("%d ",mon_data->proc_data[proc].event_data[event].value.g_functions->g[i]);
						else	printf("  ");
					printf(" ");
				break;
				case EVENT_RISP:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("RISP ENABLED |");
					else	printf("RISP DISABLED |");
				break;
				case EVENT_CONP:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("CONP ENABLED |");
					else	printf("CONP DISABLED |");
				break;
				case EVENT_SPEPN_REQ:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("SPEPNREQ ENABLED|");
					else	printf("SPEPNREQ DISABLED |");
				break;
				case EVENT_A_SPEPN:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		printf("ASPEPN ENABLED |");
					else	printf("ASPEPN DISABLED |");
				break;
			}
		}
	}
	printf("\n");
	return 0;
}


signed char FreeData(MonitorData *mon_data)
{
	DWORD rem=0;
	if(mon_data == NULL) return -1;
	for(BYTE proc=0; proc < mon_data->num_proc;proc++)
	{
		for(short event=0; event < mon_data->proc_data[proc].num_event; event++)
		{
			switch(mon_data->proc_data[proc].event_data[event].event)
			{
				case EVENT_NO_EVENT:
				break;
				case EVENT_SYSTEM_START:
					for(int i=0; i < mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num; i++)
					{
						free(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name);
						free(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name);
						free(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination);
					}
					free(mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data);
					free(mon_data->proc_data[proc].event_data[event].value.system_start_data);
				break;
				case  EVENT_NEW_DATE:
					free(mon_data->proc_data[proc].event_data[event].value.time);
				break;
				case EVENT_WORK_MODE: case EVENT_SYSTEM_STATE: case EVENT_UAS: case EVENT_UVR:case EVENT_URL: case EVENT_COMU:
				case EVENT_CEFA: case EVENT_MUSP: case EVENT_REAZ: case EVENT_PART_FINISHED:

				break;
				case EVENT_FEED: case EVENT_SPINDLE_SPEED: case EVENT_CONTROL_PANEL_SWITCH_JOG:
				case EVENT_CONTROL_PANEL_SWITCH_FEED: case EVENT_CONTROL_PANEL_SWITCH_SPINDLE:

				break;
				case EVENT_MES_ERR_PROG:
				{
					if(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len > 0)
					{
						free(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg);
					}
					free(mon_data->proc_data[proc].event_data[event].value.emergency_error);
				}
				break;
				case EVENT_PROGRAM_NAME:
				{
					for(i=0; i < mon_data->proc_data[proc].event_data[event].value.progname->num; i++)
					{
						free(mon_data->proc_data[proc].event_data[event].value.progname->data[i].name);
						free(mon_data->proc_data[proc].event_data[event].value.progname->data[i].path);
					}
					free(mon_data->proc_data[proc].event_data[event].value.progname->data);
					free(mon_data->proc_data[proc].event_data[event].value.progname);
				}
				break;
				case EVENT_BLOCK_NUMB_CTRL_PROG:
				break;
				case EVENT_TOOL_NUMBER: case EVENT_CORRECTOR_NUMBER:
				break;
				case EVENT_MACHINE_IDLETIME_CAUSE:
					
					for(int ai=0; ai < mon_data->proc_data[proc].event_data[event].value.machine_idletime->num; ai++)
					{
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len > 0)
						{
							free(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group);
						}
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len > 0)
						{
							free(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str);
						}
					}
					free(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle);
					free(mon_data->proc_data[proc].event_data[event].value.machine_idletime);
				break;
				case EVENT_PLC_ERR:
					free(mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log);
					free(mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log);

					free(mon_data->proc_data[proc].event_data[event].value.plc_error);
				break;
				case EVENT_PROCESS_COMMAND_LINE: 
				case EVENT_PROCESS_BLOCK_LINE:
				case EVENT_COMMAND_LINE:
					free(mon_data->proc_data[proc].event_data[event].value.command_line->str);
					free(mon_data->proc_data[proc].event_data[event].value.command_line);
				break;
				case EVENT_G_FUNCTIONS:
					free(mon_data->proc_data[proc].event_data[event].value.g_functions->g);
					free(mon_data->proc_data[proc].event_data[event].value.g_functions);
				break;
				case EVENT_RISP:
				break;
				case EVENT_CONP:
				break;
				case EVENT_SPEPN_REQ:
				break;
				case EVENT_A_SPEPN:
				break;
			}
		}
		free(mon_data->proc_data[proc].event_data);
	}
	free(mon_data->proc_data);
	return 0;
}

void LoadFilter(MonEventFilter *mon_event_filter)
{
	int i,j;
	mon_event_filter->max_proc=6;
	mon_event_filter->proc_filter=(WORD**) calloc(mon_event_filter->max_proc,sizeof(WORD*));
	mon_event_filter->num_filter_events=(WORD*) calloc(mon_event_filter->max_proc,sizeof(WORD));

	mon_event_filter->num_filter_events[0]=2;
	mon_event_filter->num_filter_events[1]=3;
	mon_event_filter->num_filter_events[2]=2;
	mon_event_filter->num_filter_events[3]=0;
	mon_event_filter->num_filter_events[4]=0;
	mon_event_filter->num_filter_events[5]=0;
/*
	for(i=0; i < mon_event_filter->max_proc; i++)
	{
		mon_event_filter->proc_filter[i]=calloc(mon_event_filter->proc_filter[i],sizeof(WORD));
		for(j=0; j < mon_event_filter->proc_filter[i])
		{
			mon_event_filter->proc_filter[i][j]=EVENT_NO_EVENT;
		}
	}
*/

	mon_event_filter->proc_filter[0]=(WORD*)calloc(mon_event_filter->num_filter_events[0],sizeof(WORD));
	mon_event_filter->proc_filter[0][0]=EVENT_NO_EVENT;
	mon_event_filter->proc_filter[0][1]=EVENT_NEW_DATE;

	mon_event_filter->proc_filter[1]=(WORD*)calloc(mon_event_filter->num_filter_events[1],sizeof(WORD));
	mon_event_filter->proc_filter[1][0]=EVENT_FEED;
	mon_event_filter->proc_filter[1][1]=EVENT_SPINDLE_SPEED;
	mon_event_filter->proc_filter[1][2]=EVENT_WORK_MODE;

	mon_event_filter->proc_filter[2]=(WORD*)calloc(mon_event_filter->num_filter_events[2],sizeof(WORD));
	mon_event_filter->proc_filter[1][0]=EVENT_FEED;
	mon_event_filter->proc_filter[1][1]=EVENT_SPINDLE_SPEED;

}
/*
unsigned char FindMonFile(WORD *time1, WORD *time2)
{
	int i;
	BYTE f=0;
	for(i=0; i < 7; i ++)
		if(time1[i] == time2[i]) f=1;
		else 
		{
			f=0;
			break;
		}
	if(f == 1) return 1;
	for(i=0; i < 7; i ++)
	{
		if(time1[i] > time2[i])
		{
			return 1;
		}
		else
			if(time1[i] < time2[i])
				return 2;
		}		
	return 2;
}

unsigned char FindMonFileR(WORD *time1, WORD *time2)
{
	int i;
	BYTE f=0;
	
	for(i=0; i < 7; i ++)
		if(time1[i] == time2[i]) f=1;
		else 
		{
			f=0;
			break;
		}
	if(f == 1) return 1;
	
	for(i=0; i < 7; i ++)
	{
		if(time1[i] < time2[i])
		{
			return 1;
		}
		else
			if(time1[i] > time2[i])
				return 2;
		}		
	return 2;
}
*/

signed char RecordVal(FILE *file_name,BYTE proc,DWORD event, MonitorData *mon_data,HeaderMonInfo *mon_header,BYTE *record_flag,WORD *curr_datetime,WORD *datetime,WORD *datetime_end,WORD *control_flags)
{
	WORD Hours, Minutes, Seconds, Milliseconds;
//	int i;
	// Получаем хэндл консоли 
	ConvertTime(&Hours, &Minutes, &Seconds, &Milliseconds, mon_data->time);
	curr_datetime[3]=Hours;
	curr_datetime[4]=Minutes;
	curr_datetime[5]=Seconds;
	curr_datetime[6]=Milliseconds;

//	if( FindMonFile(curr_datetime, datetime) != 1) return 1;
//	if( FindMonFileR(curr_datetime, datetime_end) != 1) return 1;
	if(WhichDateLater(curr_datetime, datetime) == DATE_SECOND_LATER) return 1;
	if(WhichDateLater(curr_datetime, datetime_end) == DATE_FIRST_LATER) return 1;
	if( (*control_flags)& WRITE_HEADER)
	{
		RecordHeader(file_name, mon_header);
		(*control_flags)&=~WRITE_HEADER;
	}
	if((*record_flag) == 0)
	{
		fprintf(file_name,"%.2d:%.2d:%.2d:%.4d ",Hours,Minutes,Seconds,Milliseconds);
	}
	fprintf(file_name,"PROCESS:%d ",mon_data->proc_data[proc].proc);

	switch(mon_data->proc_data[proc].event_data[event].event)
	{
		case EVENT_NO_EVENT:
			fprintf(file_name,"NO EVENT");
		break;
				case EVENT_SYSTEM_START:
						fprintf(file_name,"System Start:%.4d/%.2d/%.2d ",
							mon_data->proc_data[proc].event_data[event].value.system_start_data->Year,
							mon_data->proc_data[proc].event_data[event].value.system_start_data->Month,
							mon_data->proc_data[proc].event_data[event].value.system_start_data->Day);
						
						ConvertTime(&Hours, &Minutes, &Seconds, &Milliseconds, mon_data->time);
						fprintf(file_name,"%d:%d:%d:%d \n",Hours,Minutes,Seconds,Milliseconds);

						fprintf(file_name,"Characterization files: ");
						for(int i=0; i < mon_data->proc_data[proc].event_data[event].value.system_start_data->character_files_num; i++)
						{
							fprintf(file_name,"%s ",mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].logical_name);
							fprintf(file_name,"%s",mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].physical_name);
							fprintf(file_name,"%s, ",mon_data->proc_data[proc].event_data[event].value.system_start_data->file_data[i].destination);
						}

				break;
				case  EVENT_NEW_DATE:
					curr_datetime[0]=mon_data->proc_data[proc].event_data[event].value.time[0];
		curr_datetime[1]=mon_data->proc_data[proc].event_data[event].value.time[1];
		curr_datetime[2]=mon_data->proc_data[proc].event_data[event].value.time[2];

					fprintf(file_name,"NEW DATE:%.4d/%.2d/%.2d ",mon_data->proc_data[proc].event_data[event].value.time[0],
					mon_data->proc_data[proc].event_data[event].value.time[1],mon_data->proc_data[proc].event_data[event].value.time[2]);
				break;
				case EVENT_WORK_MODE:
					fprintf(file_name,"WORK MODE:%s ",WORK_MODE_ST[mon_data->proc_data[proc].event_data[event].value.Char-1]);
				break;
				case EVENT_FEED:
					fprintf(file_name,"FEED:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
				case EVENT_SPINDLE_SPEED:
					fprintf(file_name,"SPINDLE SPEED:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
				case EVENT_SYSTEM_STATE:
					fprintf(file_name,"SYS STATE:%s ", SYSTEM_STATE_ST[mon_data->proc_data[proc].event_data[event].value.Char]);
				break;
				case EVENT_MES_ERR_PROG:
					fprintf(file_name,"MesErrProg:%d ", mon_data->proc_data[proc].event_data[event].value.emergency_error->error_code);
					if(mon_data->proc_data[proc].event_data[event].value.emergency_error->msg_len != 0)
						fprintf(file_name,"MSG:%s ", mon_data->proc_data[proc].event_data[event].value.emergency_error->msg);
				break;
				case EVENT_PROGRAM_NAME:
					for(int i=0; i < mon_data->proc_data[proc].event_data[event].value.progname->num; i++)
					{
						if(mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == ROUTINE) fprintf(file_name,"ROUTINE ");
						else
							if(mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == SUBROUTINE1) fprintf(file_name,"SUBROUTINE 1 ");
							else
								if(mon_data->proc_data[proc].event_data[event].value.progname->data[i].layer == SUBROUTINE2) fprintf(file_name,"SUBROUTINE 2 ");
						fprintf(file_name,"NAME:%s ", mon_data->proc_data[proc].event_data[event].value.progname->data[i].name);
						fprintf(file_name,"PATH:%s ", mon_data->proc_data[proc].event_data[event].value.progname->data[i].path);
					}
				break;
				case EVENT_CONTROL_PANEL_SWITCH_JOG:
					fprintf(file_name,"JOG_SW:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
				case EVENT_CONTROL_PANEL_SWITCH_FEED:
					fprintf(file_name,"FEED_SW:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
				case EVENT_CONTROL_PANEL_SWITCH_SPINDLE:
					fprintf(file_name,"SPINDLE_SW:%f ", mon_data->proc_data[proc].event_data[event].value.Float);
				break;
				case EVENT_BLOCK_NUMB_CTRL_PROG:
				{
					fprintf(file_name,"NUM_BLOCK:%d ", mon_data->proc_data[proc].event_data[event].value.Long);
				}
				break;
				case EVENT_TOOL_NUMBER:
					fprintf(file_name,"NUM_TOOL:%d ", mon_data->proc_data[proc].event_data[event].value.Word);
				break;
				case EVENT_CORRECTOR_NUMBER:
					fprintf(file_name,"NUM_CORRECTOR:%d ", mon_data->proc_data[proc].event_data[event].value.Word);
				break;
				case EVENT_UAS:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"UAS ENABLED ");
					else	fprintf(file_name,"UAS DISABLED ");
				break;
				case EVENT_UVR:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"UVR ENABLED ");
					else	fprintf(file_name,"UVR DISABLED ");
				break;
				case EVENT_URL:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"URL ENABLED ");
					else	fprintf(file_name,"URL DISABLED ");
				break;
				case EVENT_COMU:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"COMU ENABLED ");
					else	fprintf(file_name,"COMU DISABLED ");
				break;
				case EVENT_CEFA:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"CEFA ENABLED ");
					else	fprintf(file_name,"CEFA DISABLED ");
				break;
				case EVENT_MUSP:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"MUSP ENABLED ");
					else	fprintf(file_name,"MUSP DISABLED ");
				break;
				case EVENT_REAZ:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"REAZ ENABLED ");
					else	fprintf(file_name,"REAZ DISABLED ");
				break;
				case EVENT_MACHINE_IDLETIME_CAUSE:
					for(int ai=0; ai < mon_data->proc_data[proc].event_data[event].value.machine_idletime->num; ai++)
					{
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].action == MACHINE_IDLETIME_SET)
							fprintf(file_name,"MACHINE IDLE TIME SET: ");
						else
							fprintf(file_name,"MACHINE IDLE TIME RESET: ");
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group_len > 0)
							fprintf(file_name,"%s ",mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].group);
						if(mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].len > 0)
							fprintf(file_name,"%s ",mon_data->proc_data[proc].event_data[event].value.machine_idletime->idle[ai].str);
					}
				break;
				case EVENT_PLC_ERR:
					fprintf(file_name,"PLC_MESS:%s ",mon_data->proc_data[proc].event_data[event].value.plc_error->mess_log);
					fprintf(file_name,"PLC_ALARM:%s ",mon_data->proc_data[proc].event_data[event].value.plc_error->alarm_log);
				break;
				case EVENT_PROCESS_COMMAND_LINE:
					fprintf(file_name,"COMMAND_FROM_PROCESS:%s ",mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
				case EVENT_PROCESS_BLOCK_LINE:
					fprintf(file_name,"BLOCK_FROM_PROCESS:%s ",mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
				case EVENT_COMMAND_LINE:
					fprintf(file_name,"COMMAND_LINE:%s ",mon_data->proc_data[proc].event_data[event].value.command_line->str);
				break;
				case EVENT_PART_FINISHED:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"PART FINISHED  ");
				break;
				case EVENT_G_FUNCTIONS:
#define NO_G 0xFF
					fprintf(file_name,"G Functions: ");
					for(int i=0; i < mon_data->proc_data[proc].event_data[event].value.g_functions->num; i++)
						if(mon_data->proc_data[proc].event_data[event].value.g_functions->g[i] != NO_G)
							fprintf(file_name,"%d ",mon_data->proc_data[proc].event_data[event].value.g_functions->g[i]);
						else	fprintf(file_name,"  ");
					fprintf(file_name," ");
				break;
				case EVENT_RISP:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"RISP ENABLED |");
					else	fprintf(file_name,"RISP DISABLED |");
				break;
				case EVENT_CONP:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"CONP ENABLED |");
					else	fprintf(file_name,"CONP DISABLED |");
				break;
				case EVENT_SPEPN_REQ:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"SPEPNREQ ENABLED|");
					else	fprintf(file_name,"SPEPNREQ DISABLED |");
				break;
				case EVENT_A_SPEPN:
					if(mon_data->proc_data[proc].event_data[event].value.Char == 1)		fprintf(file_name,"ASPEPN ENABLED |");
					else	fprintf(file_name,"ASPEPN DISABLED |");
				break;
			}
		
	
	fprintf(file_name,"\n");
	(*record_flag) = 1;
	return 0;
}


DWORD FindEvent(BYTE proc,WORD event,WORD num_event,MonEventFilter *mon_event_filter)
{
	int i;
	for(i=0; i < mon_event_filter->num_filter_events[proc]; i++)
	{
		if(num_event == mon_event_filter->proc_filter[proc][i]) 
			return num_event;
	}
	return 0xFFFFFFFF;
}

signed char FilterEvents(FILE *file_name,MonitorData *mon_data,HeaderMonInfo *mon_header, MonEventFilter *mon_event_filter,WORD *curr_datetime, WORD *datetime, WORD *datetime_end,WORD *control_flags)
{
	BYTE proc;
	DWORD event;
	BYTE record_event_flag=0;
	for(proc = 0; proc < mon_data->num_proc; proc++)
	{
		for(event=0; event < mon_data->proc_data[proc].num_event; event++)
		{
			if(FindEvent(mon_data->proc_data[proc].proc,event,mon_data->proc_data[proc].event_data[event].event,mon_event_filter) != 0xFFFFFFFF)
			{
				RecordVal(file_name,proc,event,mon_data,mon_header,&record_event_flag,curr_datetime,datetime,datetime_end,control_flags);
			}
		}
	}
	return 0;
}


//#define delay_time 20
//#define delay_time 0.01
char first_start=0;

typedef enum _cmd_settings{
	OPENMONFILE = 0x1,
	WRITETOFILE =0x2
}CmdSettings;

typedef struct _cmd_info{
	char	file_name[MAX_PATH];
	WORD	settings;
	DWORD	delay;
}CmdInfo;

char NextI(int *i,int argc)
{
	if((*i) +1 < argc){ (*i)++; return 0;}
	else return 1;
}

void LoadCmdLine(CmdInfo *cmd_info,int argc, _TCHAR* argv[])
{
	int i;
	memset(cmd_info,0,sizeof(CmdInfo));
	cmd_info->delay=1000;
	i=1;
	while(i < argc)
	{
		if(strstr(argv[i],".mon"))
		{
			strcpy(cmd_info->file_name,argv[i]);
			cmd_info->settings|=OPENMONFILE;
			if(NextI(&i,argc)) break;
		}
		else
		{
			if(strcmpi(argv[i],"delay"))
			{
				if(NextI(&i,argc)) break;
				cmd_info->delay=atof(argv[i]);
				if(NextI(&i,argc)) break;
			}
			else
			{
				if(strcmpi(argv[i],"WriteToFile"))
				{
					cmd_info->settings|=WRITETOFILE;
					if(NextI(&i,argc)) break;
				}
			}
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD OldFileLen=0,max_index=0,mon_data_index=0,mon_data_curr=0,tmp=0;
	HANDLE pFile;
	MonitorData **mon_data=NULL;
	HeaderMonInfo *header_mon_info=NULL;
	char *pr_name,buf_name[30],curr_open[30];
	//bool header_loaded=false;
	clock_t start,timer_i,delay_time=1;
	char *p=NULL;
	char read_info=0;
	WORD control_flags=0;
	MonBuf mon_buf;
	MonEventFilter *mon_event_filter;
	DWORD curr_file_size,max_file_size=0;

	pr_name=(char*)calloc(256,sizeof(char));
	strcpy(pr_name,"info.mon");
	start = clock();
	GetMonBuf(&mon_buf,3804);
	FILE *file_name;
	CmdInfo *cmd_info=(CmdInfo*)malloc(1*sizeof(CmdInfo));
	LoadCmdLine(cmd_info,argc,argv);
	mon_event_filter=(MonEventFilter*)calloc(1,sizeof(MonEventFilter));
	LoadFilter(mon_event_filter);
//#include <malloc.h>
	MonFileQueueInfo *mon_file_qeueue_info;
	SetCurrentDirectory("D:\\exch\\A\\MonitorFilter\\");
	char *mon_queue_file_name=(char*)calloc(MAX_PATH,sizeof(char*));
	strcpy(mon_queue_file_name,"20121018041150.mon");
	mon_file_qeueue_info=BuildQueue(mon_queue_file_name);
//	WORD timedate[7]={2012,10,18,04,12,00,00};
//	WORD timedate[7]={2012,9,18,04,12,00,00};
	WORD timedate[7]={2012,10,18,04,11,0,306};
	WORD timedate_end[7]={2012,10,18,04,12,40,00};
	WORD curr_datetime[7];
	FindMonFile(pr_name,mon_file_qeueue_info, timedate);
	
	do{
		curr_file_size=0;
		OldFileLen=0;
		control_flags&=~HEADER_LOADED;
		mon_buf.index=0;
		control_flags|=WRITE_HEADER;
		do
		{
			//считывать файл через определенные промежутки времени
			file_name=fopen("text.txt","a");
			if(file_name == NULL) {printf("open RRR!!"); continue;}

			OldFileLen+=mon_buf.index;
			if(control_flags&NO_EVENT && OldFileLen >= (sizeof(long)+sizeof(WORD)+sizeof(BYTE)+sizeof(BYTE)+sizeof(short)+sizeof(short)+1) )//произошло событие "нет события"?
			{//переместить "указатель" в файле в позицию до события "нет события"
				OldFileLen-=sizeof(long)+sizeof(WORD)+sizeof(BYTE)+sizeof(BYTE)+sizeof(short)+sizeof(short)+1;
			}
			if(ReadMonitorFile(pr_name,&mon_buf,&OldFileLen,&tmp) < 0)
			{
				printf("History file - open error! Retry...\n"); 
				continue;
			}
			strcpy(curr_open,pr_name);

			max_index=tmp;
			mon_buf.index=0;

			mon_data=(MonitorData**)calloc(1,sizeof(MonitorData*));
			mon_data[0]=(MonitorData*)calloc(1,sizeof(MonitorData));

			while(mon_buf.index < max_index)
			{
				memset(mon_data[0],0,sizeof(MonitorData));
				if(!(control_flags&HEADER_LOADED))//заголовок не считывался?
				{//считываем заголовок
					LoadHeader(&header_mon_info,&mon_buf);
					curr_datetime[0]=header_mon_info->SysTime.wYear;
					curr_datetime[1]=header_mon_info->SysTime.wMonth;
					curr_datetime[2]=header_mon_info->SysTime.wDay;
					max_file_size=header_mon_info->file_size;
					ShowHeader(header_mon_info);
					printf("Header loaded. Press any key to continue...\n");
					//getch();
					control_flags|=HEADER_LOADED;
				}
				else
				{//считываем сообщения
					if(GetEventData(mon_data[0],&mon_buf,&control_flags)  <0)
							break;
					ShowVal(mon_data[0]);
					FilterEvents(file_name,mon_data[0],header_mon_info,mon_event_filter,curr_datetime,timedate,timedate_end,&control_flags);
					FreeData(mon_data[0]);
				}
				//mon_data_index++;
			}
			curr_file_size+=mon_buf.index;
			free(mon_data[0]);
			free(mon_data);
			//if(file_name != NULL)
			fclose(file_name);
			file_name=NULL;
		}while(curr_file_size < max_file_size);
		if(WhichDateLater(curr_datetime, timedate_end) == DATE_SECOND_LATER)
		{
			strcpy(pr_name,mon_file_qeueue_info->curr->next->file_info->name);
			mon_file_qeueue_info->curr=mon_file_qeueue_info->curr->next;
		}
	}while(	WhichDateLater(curr_datetime, timedate_end) == DATE_SECOND_LATER && mon_file_qeueue_info->curr != NULL);
	return 0;
}

