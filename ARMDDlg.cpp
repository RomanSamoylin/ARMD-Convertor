// ARMDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ARMD.h"
#include "ARMDDlg.h"
#include "SaveMonErrorOpen.h"
#include "ARMDFileList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static UINT BASED_CODE indicators[] = { ID_STATUS_BAR };

extern WORD event_count;
extern EM EVENT_NAME[];
extern 	BYTE first_record;
//extern EM EVENT_NAME0[];
extern MonFileQueueInfo* BuildQueue(CPathOperations* file_name);


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

void DestroyQueue(MonFileQueueInfo* mon_file_qeueue_info)
{
	if (mon_file_qeueue_info == NULL || mon_file_qeueue_info->first == NULL) return;
	MonFileQueue* curr = mon_file_qeueue_info->first;
	while (curr != NULL)
	{
		MonFileQueue* tmp = curr;
		curr = curr->next;
		if (tmp != NULL)
		{
			if (tmp->file_info != NULL)
				delete tmp->file_info;
			delete tmp;
		}
	}
	delete mon_file_qeueue_info;
}

// CARMDDlg dialog



CARMDDlg::CARMDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CARMDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	strcpy(status_bar_msg.ready, "Готово");
	strcpy(status_bar_msg.confuguration_loaded, "Конфигурация загружена");
	strcpy(status_bar_msg.configuration_saved, "Конфигурация сохранена");
	strcpy(status_bar_msg.empty, "");
	strcpy(status_bar_msg.wait, "Подождите...операция может занять несколько минут");
	strcpy(status_bar_msg.file_saved, "Файл успешно сохранен");
	strcpy(status_bar_msg.error, "Ошибка");
	strcpy(status_bar_msg.queue_build, "Очередь построена");
	strcpy(status_bar_msg.queue_build_error, "Ошибка! Не удалось построить очередь АРМД файлов!");

	strcpy(message_box_msg.save_to_text_error.title, "Ошибка!");
	strcpy(message_box_msg.save_to_text_error.text, "Операция завершена некорректно! Возможно файл ARMD поврежден.");

	strcpy(message_box_msg.qeueu_build_error.title, "Ошибка!");
	strcpy(message_box_msg.qeueu_build_error.text, "Не удалось построить очередь АРМД файлов!");

	strcpy(message_box_msg.illegal_time_limits.title, "Ошибка!");
	strcpy(message_box_msg.illegal_time_limits.text, "Временные рамки фильтра выходят за временные рамки очереди АРМД!");

	strcpy(message_box_msg.io_error.title, "Ошибка!");
	strcpy(message_box_msg.io_error.text, "Отсутствует доступ к файлу АРМД!");

	strcpy(message_box_msg.ARMD_file_not_found.title, "Ошибка");
	strcpy(message_box_msg.ARMD_file_not_found.text, "Файлы АРМД не найдены!");
	filter_event_process = new CFilterEventProcess1[6];//process count
	memset(begin_time_limit, 0, 7 * sizeof(WORD));
	memset(end_time_limit, 0, 7 * sizeof(WORD));
	mon_file_qeueue_info = NULL;
}
CARMDDlg::~CARMDDlg()
{
	delete[] filter_event_process;
	//	free(mon_queue_file_name);
	//	free(save_mon_file_name);
	//	free(pr_name);
	DestroyQueue(mon_file_qeueue_info);
	//CDialog::~CDialog();
}

void CARMDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab_ctrl);
	DDX_Control(pDX, IDC_DATETIMEPICKER2, m_from_date);
	DDX_Control(pDX, IDC_DATETIMEPICKER4, m_from_time);
	DDX_Control(pDX, IDC_DATETIMEPICKER3, m_to_date);
	DDX_Control(pDX, IDC_DATETIMEPICKER5, m_to_time);
	//	DDX_Control(pDX, IDC_LIST1, m_loaded_mon_files);
	DDX_Control(pDX, IDC_CHECK1, separate_proc);
	DDX_Control(pDX, IDC_EDIT1, path_bar);
	DDX_Control(pDX, IDC_SAVE_TO_TEXT, save_to_text_ctrl);
	DDX_Control(pDX, IDC_EDIT2, begin_limit_ctrl);
	DDX_Control(pDX, IDC_EDIT3, end_limit_ctrl);
	DDX_Control(pDX, IDC_BUTTON1, armd_file_info_ctrl);
	DDX_Control(pDX, IDC_BUTTON2, m_data_synchro);
}

BEGIN_MESSAGE_MAP(CARMDDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTcnSelchangeTab1)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB1, OnTcnSelchangingTab1)
	ON_BN_CLICKED(IDC_SAVE_TO_TEXT, OnBnClickedSaveToText)
	ON_BN_CLICKED(IDC_OPEN_MON_FILE, OnBnClickedOpenMonFile)
	ON_NOTIFY(NM_KILLFOCUS, IDC_DATETIMEPICKER2, OnNMKillfocusDatetimepicker2)
	ON_NOTIFY(NM_KILLFOCUS, IDC_DATETIMEPICKER4, OnNMKillfocusDatetimepicker4)
	ON_NOTIFY(NM_KILLFOCUS, IDC_DATETIMEPICKER3, OnNMKillfocusDatetimepicker3)
	ON_NOTIFY(NM_KILLFOCUS, IDC_DATETIMEPICKER5, OnNMKillfocusDatetimepicker5)
	//ON_NOTIFY(NM_THEMECHANGED, IDC_DATETIMEPICKER2, OnNMThemeChangedDatetimepicker2)
	ON_WM_DESTROY()
	ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
	ON_COMMAND(ID_139, LoadConfiguration)
	ON_COMMAND(ID_140, SaveConfigurations)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CARMDDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CARMDDlg message handlers

BOOL CARMDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	save_to_text_ctrl.EnableWindow(FALSE);
	armd_file_info_ctrl.EnableWindow(FALSE);
	m_data_synchro.EnableWindow(FALSE);

	//m_loaded_mon_files.SetHorizontalExtent(1000);
	//m_loaded_mon_files.SetCurSel(0);

	status_bar.Create(this);
	status_bar.SetIndicators(indicators, 1);
	CRect rect;
	GetClientRect(&rect);
	status_bar.SetPaneInfo(0, ID_STATUS_BAR, SBPS_NORMAL, rect.Width());
	//RepositionBars(
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, ID_STATUS_BAR);
	status_bar.SetPaneText(0, status_bar_msg.ready);
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	TC_ITEM tci;
	tci.mask = TCIF_TEXT;
	tci.iImage = -1;
	tci.pszText = "Общий";
	m_tab_ctrl.InsertItem(0, &tci);

	tci.pszText = "Процесс 1";
	m_tab_ctrl.InsertItem(1, &tci);
	tci.pszText = "Процесс 2";
	m_tab_ctrl.InsertItem(2, &tci);
	tci.pszText = "Процесс 3";
	m_tab_ctrl.InsertItem(3, &tci);
	tci.pszText = "Процесс 4";
	m_tab_ctrl.InsertItem(4, &tci);
	tci.pszText = "Процесс 5";
	m_tab_ctrl.InsertItem(5, &tci);

	for (BYTE proc = 0; proc < 6; proc++)
	{
		tci.mask = TCIF_PARAM;
		tci.lParam = (LPARAM)&filter_event_process[proc];
		m_tab_ctrl.SetItem(proc, &tci);

		filter_event_process[proc].Create(CFilterEventProcess1::IDD, &m_tab_ctrl);
		RECT g;
		filter_event_process[proc].GetWindowRect(&g);
		//		filter_event_process[proc].SetWindowPos(NULL, 5, 25, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		//		filter_event_process[proc].SetWindowPos(NULL, 5, 25, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		//		filter_event_process[proc].SetWindowPos(NULL, 5, 25, g.right, g.bottom, SWP_NOSIZE | SWP_NOZORDER);
		filter_event_process[proc].SetWindowPos(NULL, ((rect.right - rect.left) - (g.right - g.left)) / 2, 25, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		//		filter_event_process[proc].SetWindowPos(NULL, 5, 25, rect.right+1, rect.bottom,/* SWP_NOSIZE |*/ SWP_NOZORDER);
		filter_event_process[proc].all_filters = filter_event_process;
		filter_event_process[proc].num_filtres_proc = 6;
		filter_event_process[proc].curr_proc = proc;
		filter_event_process[proc].ShowWindow(SW_HIDE);
		filter_event_process[proc].bg_brush.CreateSolidBrush(::GetSysColor(COLOR_WINDOW));

		if (proc == 0) { FillFilterEventListsProc0(&filter_event_process[proc]); filter_event_process[proc].apply_to_all_process.EnableWindow(FALSE); }
		else { FillFilterEventLists(&filter_event_process[proc]); filter_event_process[proc].apply_to_all_process.EnableWindow(TRUE); }
	}
	filter_event_process[0].ShowWindow(SW_SHOW);
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	BYTE load_param = 0;

	char* curr_path = new char[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, curr_path);
	ini_path.SetPath(curr_path);
	ini_path.SetFileName("armdtnsl.ini");
	if (LoadSysConfiguration(&ini_path) == 0)
	{
		status_bar.SetPaneText(0, status_bar_msg.confuguration_loaded);
		load_param = 1;
	}
	delete curr_path;
	///////////////////////////////////////////////////////////////////////////////////////////////////
		// TODO: Add extra initialization here
	SetWindowText("ARMD Converter");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CARMDDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CARMDDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CARMDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CARMDDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	int iTab = m_tab_ctrl.GetCurSel();
	TC_ITEM tci;
	tci.mask = TCIF_PARAM;
	m_tab_ctrl.GetItem(iTab, &tci);
	//CWnd* pWnd = (CWnd *)tci.lParam;
	CFilterEventProcess1* f = (CFilterEventProcess1*)tci.lParam;
	if (iTab == 0) f->apply_to_all_process.EnableWindow(FALSE);
	else f->apply_to_all_process.EnableWindow(TRUE);
	f->ShowWindow(SW_SHOW);
	//pWnd->ShowWindow(SW_SHOW); 

	*pResult = 0;
}

void CARMDDlg::OnTcnSelchangingTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	int iTab = m_tab_ctrl.GetCurSel();
	TC_ITEM tci;
	tci.mask = TCIF_PARAM;
	m_tab_ctrl.GetItem(iTab, &tci);
	CWnd* pWnd = (CWnd*)tci.lParam;
	pWnd->ShowWindow(SW_HIDE);

	*pResult = 0;
}

void CARMDDlg::FillFilterEventListsProc0(CFilterEventProcess1* filter_event_process)
{
	filter_event_process->m_filtred_events.AddString(EVENT_NAME[1].name);
	filter_event_process->m_filtred_events.AddString(EVENT_NAME[2].name);
	filter_event_process->m_filtred_events.AddString(EVENT_NAME[3].name);
	filter_event_process->m_filtred_events.AddString(EVENT_NAME[27].name);
}

void CARMDDlg::FillFilterEventLists(CFilterEventProcess1* filter_event_process)
{
	//for(DWORD i=1; i < event_count; i++)
	DWORD i = 1;
	while (EVENT_NAME[i].name[0] != 0)
	{
		if (i != 1 && i != 2 && i != 3 && i != 27)
			filter_event_process->m_filtred_events.AddString(EVENT_NAME[i].name);
		i++;

	}
}

signed char CARMDDlg::MonSaveToFile()
{
	DWORD curr_file_size = 0, max_index = 0, max_file_size = 0;
	DWORD OldFileLen;
	WORD control_flags = 0;
	MonBuf mon_buf;
	FILE** file_name;
	DWORD tmp;
	MonitorData** mon_data = NULL;
	HeaderMonInfo* header_mon_info = NULL;
	if (GetMonBuf(&mon_buf, 1000000 * sizeof(char)) == 0) return -1;

	file_name = (FILE**) new FILE[6];
	if (file_name == NULL) return -1;
	if (separate_proc.GetCheck() == 1)
	{
		for (BYTE file_i = 1; file_i < 6; file_i++)
		{
			file_name[file_i] = fopen(save_mon_file_name[file_i].GetFullPath(NULL, 0), "a");
			if (file_name[file_i] == NULL) { MessageBox(message_box_msg.io_error.text, message_box_msg.io_error.title, MB_OK | MB_ICONERROR); FreeMonBuf(&mon_buf); return -1; }
		}
	}
	else
	{
		file_name[0] = fopen(save_mon_file_name[0].GetFullPath(NULL, 0), "a");
		if (file_name[0] == NULL) { MessageBox(message_box_msg.io_error.text, message_box_msg.io_error.title, MB_OK | MB_ICONERROR); FreeMonBuf(&mon_buf); return -1; }
	}
	do {
		curr_file_size = 0;
		OldFileLen = 0;
		control_flags &= ~HEADER_LOADED;
		mon_buf.index = 0;
		control_flags |= WRITE_HEADER;
		do
		{
			//считывать файл через определенные промежутки времени

			OldFileLen += mon_buf.index;
			if (control_flags & NO_EVENT && OldFileLen >= (sizeof(long) + sizeof(WORD) + sizeof(BYTE) + sizeof(BYTE) + sizeof(short) + sizeof(short) + 1))//произошло событие "нет события"?
			{//переместить "указатель" в файле в позицию до события "нет события"
				OldFileLen -= sizeof(long) + sizeof(WORD) + sizeof(BYTE) + sizeof(BYTE) + sizeof(short) + sizeof(short) + 1;
			}
			if (ReadMonitorFile(&pr_name, &mon_buf, &OldFileLen, &tmp) < 0)
			{
				MessageBox(message_box_msg.io_error.text, message_box_msg.io_error.title, MB_OK | MB_ICONERROR);
				return -1;
			}

			max_index = tmp;
			mon_buf.index = 0;

			mon_data = (MonitorData**)calloc(1, sizeof(MonitorData*));
			mon_data[0] = (MonitorData*)calloc(1, sizeof(MonitorData));

			while (mon_buf.index < max_index)
			{
				memset(mon_data[0], 0, sizeof(MonitorData));
				if (!(control_flags & HEADER_LOADED))//заголовок не считывался?
				{//считываем заголовок
					if (LoadHeader(&header_mon_info, &mon_buf) < 0) break;
					time_interval.curr[0] = header_mon_info->SysTime.wYear;
					time_interval.curr[1] = header_mon_info->SysTime.wMonth;
					time_interval.curr[2] = header_mon_info->SysTime.wDay;

					max_file_size = header_mon_info->file_size;
					//					ShowHeader(header_mon_info);
										//printf("Header loaded. Press any key to continue...\n");
										//getch();
					control_flags |= HEADER_LOADED;
				}
				else
				{//считываем сообщения
					BYTE rezz;
					rezz = GetEventData(header_mon_info, mon_data[0], &mon_buf, &time_interval, &control_flags);
					if (rezz < 0)
					{
						FreeData(mon_data[0]);
						free(mon_data[0]);
						free(mon_data);
						FreeMonBuf(&mon_buf);
						FreeHeader(&header_mon_info);

						if (separate_proc.GetCheck() == 1)
						{
							fclose(file_name[1]);
							fclose(file_name[2]);
							fclose(file_name[3]);
							fclose(file_name[4]);
							fclose(file_name[5]);
						}
						else
							fclose(file_name[0]);
						return -1;
					}
					else
						if (rezz == 2)
						{
							break;
						}
					ConvertTime(&time_interval.curr[3], &time_interval.curr[4], &time_interval.curr[5], &time_interval.curr[6], mon_data[0]->time);
					if (WhichDateLater(time_interval.curr, time_interval.from) == DATE_SECOND_LATER)
					{
						FreeData(mon_data[0]);
						continue;
					}
					if (WhichDateLater(time_interval.curr, time_interval.to) == DATE_FIRST_LATER)
					{
						FreeData(mon_data[0]);
						free(mon_data[0]);
						free(mon_data);
						goto FINISH;
					}
					if (separate_proc.GetCheck() == 1) control_flags |= SEPARATE_PROC;
					FilterEvents(file_name, mon_data[0], header_mon_info, &mon_event_filter, &control_flags);
					FreeData(mon_data[0]);
				}
				//mon_data_index++;
			}
			curr_file_size += mon_buf.index;
			free(mon_data[0]);
			free(mon_data);
			//if(file_name != NULL)
			//file_name=NULL;
		} while (curr_file_size < max_file_size);
		//if(WhichDateLater(time_interval.curr,time_interval.to) == DATE_SECOND_LATER)
		{
			mon_file_qeueue_info->curr = mon_file_qeueue_info->curr->next;
			if (mon_file_qeueue_info->curr != NULL)
			{
				FreeHeader(&header_mon_info);
				pr_name.SetFullPath(mon_file_qeueue_info->curr->file_info->name.GetFullPath(NULL, 0));
			}

		}
		//	}while(	WhichDateLater(time_interval.curr, time_interval.to) == DATE_SECOND_LATER && mon_file_qeueue_info->curr != NULL);
		//	}while(mon_file_qeueue_info->curr->next != NULL);
	} while (mon_file_qeueue_info->curr != NULL);
FINISH:
	FreeMonBuf(&mon_buf);
	FreeHeader(&header_mon_info);
	if (separate_proc.GetCheck() == 1)
	{
		fclose(file_name[1]);
		fclose(file_name[2]);
		fclose(file_name[3]);
		fclose(file_name[4]);
		fclose(file_name[5]);
	}
	else
		fclose(file_name[0]);
	delete[] file_name;
	return 0;
}

DWORD GetBoxIndex(CFilterEventProcess1* filter_event_process, DWORD index, char* buf)
{
	DWORD i;
	filter_event_process->m_filtred_events.GetText(index, buf);
	i = 1;
	while (EVENT_NAME[i].name[0] != 0)
	{
		if (strcmpi(EVENT_NAME[i].name, buf) == 0)
		{
			return EVENT_NAME[i].event;
		}
		i++;
	}
	return 0;
}

const BYTE proc_count = 6;
void CARMDDlg::LoadFilter()
{
	DWORD count[proc_count] = { 0,0,0,0,0,0 }, event_counting;
	for (BYTE proc = 0; proc < proc_count; proc++)
	{
		count[proc] = 0;
		for (int i = 0; i < filter_event_process[proc].m_filtred_events.GetCount(); i++)
		{
			count[proc] += filter_event_process[proc].m_filtred_events.GetCheck(i);
		}
	}
	DWORD proc_i, event_i;
	mon_event_filter.max_proc = 6;

	mon_event_filter.proc_filter = (WORD**)calloc(mon_event_filter.max_proc, sizeof(WORD*));
	mon_event_filter.num_filter_events = (WORD*)calloc(mon_event_filter.max_proc, sizeof(WORD));

	char* box_str = new char[256];
	for (proc_i = 0; proc_i < mon_event_filter.max_proc; proc_i++)
	{
		mon_event_filter.proc_filter[proc_i] = (WORD*)calloc(count[proc_i], sizeof(WORD));
		event_counting = 0;
		for (event_i = 0; event_i < filter_event_process[proc_i].m_filtred_events.GetCount(); event_i++)
		{
			if (filter_event_process[proc_i].m_filtred_events.GetCheck(event_i) == 1)
			{
				mon_event_filter.proc_filter[proc_i][event_counting] = GetBoxIndex(&filter_event_process[proc_i], event_i, box_str);
				event_counting++;
			}
		}
		mon_event_filter.num_filter_events[proc_i] = event_counting;
	}
	delete box_str;

	SYSTEMTIME tmp;
	m_from_date.GetTime(&tmp);
	time_interval.from[0] = tmp.wYear;
	time_interval.from[1] = tmp.wMonth;
	time_interval.from[2] = tmp.wDay;

	m_from_time.GetTime(&tmp);
	time_interval.from[3] = tmp.wHour;
	time_interval.from[4] = tmp.wMinute;
	time_interval.from[5] = tmp.wSecond;
	time_interval.from[6] = 0;

	m_to_date.GetTime(&tmp);
	time_interval.to[0] = tmp.wYear;
	time_interval.to[1] = tmp.wMonth;
	time_interval.to[2] = tmp.wDay;

	m_to_time.GetTime(&tmp);
	time_interval.to[3] = tmp.wHour;
	time_interval.to[4] = tmp.wMinute;
	time_interval.to[5] = tmp.wSecond;
	time_interval.to[6] = 999;

}

void CARMDDlg::ReleaseFilter()
{
	int proc_i;
	for (proc_i = 0; proc_i < mon_event_filter.max_proc; proc_i++)
	{
		delete mon_event_filter.proc_filter[proc_i];
	}
	delete mon_event_filter.num_filter_events;
	delete mon_event_filter.proc_filter;
}


void CARMDDlg::OnBnClickedSaveToText()
{
	// TODO: Add your control notification handler code here
	int rez;
	if (mon_file_qeueue_info == NULL)
	{
		CSaveMonErrorOpen dlg;
		int rez = dlg.DoModal();
		return;
	}
	status_bar.SetPaneText(0, status_bar_msg.empty);
	CFileDialog* save_file_dlg = new CFileDialog(FALSE, NULL, NULL, 0, "", NULL);
	INT_PTR rez_save_dlg = save_file_dlg->DoModal();
	if (rez_save_dlg == IDOK)
	{
		//char a[MAX_PATH];
		//strcpy(save_mon_file_name,save_file_dlg->GetFileName().GetString());
		int date_correct = 0;
		char* str = new char[256];
		WORD len;
		if (separate_proc.GetCheck() == 1)
		{
			for (BYTE proc = 0; proc < 6; proc++)
			{
				save_mon_file_name[proc].SetFullPath((char*)save_file_dlg->GetPathName().GetString());
				strcpy(str, save_file_dlg->GetFileTitle().GetString());
				len = strlen(str);
				str[len] = '0' + proc;
				str[len + 1] = 0;
				strcat(str, ".");
				strcat(str, save_file_dlg->GetFileExt().GetString());
				save_mon_file_name[proc].SetFileName(str);
			}
		}
		else
			save_mon_file_name[0].SetFullPath((char*)save_file_dlg->GetPathName().GetString());

		status_bar.SetPaneText(0, status_bar_msg.wait);
		LoadFilter();

		if (WhichDateLater2(time_interval.from, time_interval.to) == DATE_FIRST_LATER)
		{
			MessageBox(message_box_msg.illegal_time_limits.text, message_box_msg.illegal_time_limits.title, MB_OK | MB_ICONERROR);
			date_correct = -1;
		}

		if (WhichDateLater2(time_interval.from, end_time_limit) == DATE_FIRST_LATER)
		{
			MessageBox(message_box_msg.illegal_time_limits.text, message_box_msg.illegal_time_limits.title, MB_OK | MB_ICONERROR);
			date_correct = -1;
		}

		if (WhichDateLater2(time_interval.to, begin_time_limit) == DATE_SECOND_LATER)
		{
			MessageBox(message_box_msg.illegal_time_limits.text, message_box_msg.illegal_time_limits.title, MB_OK | MB_ICONERROR);
			date_correct = -1;
		}

		if (WhichDateLater2(begin_time_limit, time_interval.from) == DATE_FIRST_LATER)
		{
			MessageBox(message_box_msg.illegal_time_limits.text, message_box_msg.illegal_time_limits.title, MB_OK | MB_ICONERROR);
			date_correct = -1;
		}
		if (WhichDateLater2(end_time_limit, time_interval.to) == DATE_SECOND_LATER)
		{
			MessageBox(message_box_msg.illegal_time_limits.text, message_box_msg.illegal_time_limits.title, MB_OK | MB_ICONERROR);
			date_correct = -1;
		}
		if (date_correct == 0)
		{
			if (FindMonFile(&pr_name, mon_file_qeueue_info, time_interval.from) >= 0)
			{
				rez = CARMDDlg::MonSaveToFile();
				ReleaseFilter();
				if (rez == 0)
					status_bar.SetPaneText(0, status_bar_msg.file_saved);
				else
				{
					status_bar.SetPaneText(0, status_bar_msg.error);
					MessageBox(message_box_msg.save_to_text_error.text, message_box_msg.save_to_text_error.title, MB_OK | MB_ICONERROR);
				}
			}
			else
				status_bar.SetPaneText(0, status_bar_msg.error);
		}
		delete[] str;
	}
	else
	{
		status_bar.SetPaneText(0, status_bar_msg.ready);
	}
	delete save_file_dlg;
	first_record = 0;
}
int CARMDDlg::GetTimeLimits()
{
	if (mon_file_qeueue_info == NULL || mon_file_qeueue_info->first == NULL ||
		mon_file_qeueue_info->first->file_info == NULL ||
		mon_file_qeueue_info->first->file_info->time == NULL || begin_time_limit == NULL) return -1;
	memcpy(begin_time_limit, mon_file_qeueue_info->first->file_info->time, 7 * sizeof(WORD));

	DWORD curr_file_size = 0, max_index = 0, max_file_size = 0;
	DWORD OldFileLen;
	WORD control_flags = 0;
	MonBuf mon_buf;
	FILE* file_name;
	DWORD tmp;
	DWORD curr_time;
	MonitorData** mon_data = NULL;
	HeaderMonInfo* header_mon_info = NULL;
	if (GetMonBuf(&mon_buf, 1000000 * sizeof(char)) == 0) return -1;

	curr_file_size = 0;
	OldFileLen = 0;
	control_flags &= ~HEADER_LOADED;
	mon_buf.index = 0;
	control_flags |= WRITE_HEADER;
	do
	{
		//считывать файл через определенные промежутки времени
		OldFileLen += mon_buf.index;
		if (control_flags & NO_EVENT && OldFileLen >= (sizeof(long) + sizeof(WORD) + sizeof(BYTE) + sizeof(BYTE) + sizeof(short) + sizeof(short) + 1))//произошло событие "нет события"?
		{//переместить "указатель" в файле в позицию до события "нет события"
			OldFileLen -= sizeof(long) + sizeof(WORD) + sizeof(BYTE) + sizeof(BYTE) + sizeof(short) + sizeof(short) + 1;
		}
		if (ReadMonitorFile(&mon_file_qeueue_info->last->file_info->name, &mon_buf, &OldFileLen, &tmp) < 0)
		{
			MessageBox(message_box_msg.io_error.text, message_box_msg.io_error.title, MB_OK | MB_ICONERROR);
		}
		max_index = tmp;
		mon_buf.index = 0;

		mon_data = (MonitorData**)calloc(1, sizeof(MonitorData*));
		mon_data[0] = (MonitorData*)calloc(1, sizeof(MonitorData));

		while (mon_buf.index < max_index)
		{
			memset(mon_data[0], 0, sizeof(MonitorData));
			if (!(control_flags & HEADER_LOADED))//заголовок не считывался?
			{//считываем заголовок
				if (LoadHeader(&header_mon_info, &mon_buf) < 0) return -1;

				time_interval.curr[0] = header_mon_info->SysTime.wYear;
				time_interval.curr[1] = header_mon_info->SysTime.wMonth;
				time_interval.curr[2] = header_mon_info->SysTime.wDay;

				max_file_size = header_mon_info->file_size;
				control_flags |= HEADER_LOADED;
			}
			else
			{//считываем сообщения
				if (GetEventData(header_mon_info, mon_data[0], &mon_buf, &time_interval, &control_flags) < 0)
				{
					FreeData(mon_data[0]); free(mon_data[0]); free(mon_data); FreeHeader(&header_mon_info); FreeMonBuf(&mon_buf);
					return -1;
				}

				curr_time = mon_data[0]->time;
				FreeData(mon_data[0]);
			}
		}
		curr_file_size += mon_buf.index;
		free(mon_data[0]);
		free(mon_data);
	} while (curr_file_size < max_file_size);
	end_time_limit[0] = time_interval.curr[0];
	end_time_limit[1] = time_interval.curr[1];
	end_time_limit[2] = time_interval.curr[2];

	ConvertTime(&end_time_limit[3], &end_time_limit[4], &end_time_limit[5], &end_time_limit[6], curr_time);
	FreeHeader(&header_mon_info);
	FreeMonBuf(&mon_buf);
	return 0;
}

void CARMDDlg::OnBnClickedOpenMonFile()
{
	// TODO: Add your control notification handler code here
	BROWSEINFO bi;
	char szDisplayName[MAX_PATH];
	char szTmpName[MAX_PATH];
	LPITEMIDLIST pidl;
	LPMALLOC pMalloc = NULL;
	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = NULL;
	bi.pszDisplayName = szDisplayName;
	bi.lpszTitle = TEXT("Выберите папку с файлами АРМД");
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	pidl = SHBrowseForFolder(&bi);
	status_bar.SetPaneText(0, status_bar_msg.empty);
	if (pidl)
	{
		HANDLE find_files;
		status_bar.SetPaneText(0, status_bar_msg.wait);
		SHGetPathFromIDList(pidl, szDisplayName);
		WIN32_FIND_DATA lpFindFileData;
		memset(&lpFindFileData, 0, sizeof(WIN32_FIND_DATA));
		strcpy(szTmpName, szDisplayName);
		strcat(szDisplayName, "\\*.mon");
		find_files = FindFirstFile(szDisplayName, &lpFindFileData);
		if (find_files != INVALID_HANDLE_VALUE)
		{
		CHECK_AGAIN:
			if (lpFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				memset(&lpFindFileData, 0, sizeof(WIN32_FIND_DATA));
				if (FindNextFile(find_files, &lpFindFileData) == 1)
				{
					goto CHECK_AGAIN;
				}
				else
				{
					MessageBox(message_box_msg.ARMD_file_not_found.text, message_box_msg.ARMD_file_not_found.title, MB_OK | MB_ICONERROR);
					status_bar.SetPaneText(0, status_bar_msg.ready);
					save_to_text_ctrl.EnableWindow(FALSE);
					armd_file_info_ctrl.EnableWindow(FALSE);
					m_data_synchro.EnableWindow(FALSE);

					return;
				}
			}
			else
			{
				if (lpFindFileData.cFileName[0] != 0)
				{
					mon_queue_file_name.SetPath(szTmpName);
					mon_queue_file_name.SetFileName(lpFindFileData.cFileName);
					open_file_name.SetPath(szTmpName);
					open_file_name.SetFileName(lpFindFileData.cFileName);

					if (mon_file_qeueue_info != NULL)
						DestroyQueue(mon_file_qeueue_info);

					path_bar.SetWindowText(mon_queue_file_name.GetPath(NULL, 0));
					mon_file_qeueue_info = BuildQueue(&mon_queue_file_name);
					if (mon_file_qeueue_info != NULL)
					{
						if (GetTimeLimits() < 0) return;
						status_bar.SetPaneText(0, status_bar_msg.queue_build);
						save_to_text_ctrl.EnableWindow(TRUE);
						armd_file_info_ctrl.EnableWindow(TRUE);
						m_data_synchro.EnableWindow(TRUE);

						char* text = new char[256];
						if (text != NULL)
						{
							sprintf(text, "%.2d.%.2d.%.4d  %.2d:%.2d:%.2d", begin_time_limit[2], begin_time_limit[1], begin_time_limit[0],
								begin_time_limit[3], begin_time_limit[4], begin_time_limit[5], begin_time_limit[6]);
							begin_limit_ctrl.SetWindowText(text);
							sprintf(text, "%.2d.%.2d.%.4d  %.2d:%.2d:%.2d", end_time_limit[2], end_time_limit[1], end_time_limit[0],
								end_time_limit[3], end_time_limit[4], end_time_limit[5], end_time_limit[6]);

							end_limit_ctrl.SetWindowText(text);
							delete[]text;
						}
					}
					else
					{
						status_bar.SetPaneText(0, status_bar_msg.error);
						save_to_text_ctrl.EnableWindow(FALSE);
						armd_file_info_ctrl.EnableWindow(FALSE);
						m_data_synchro.EnableWindow(FALSE);
					}
				}
				else
				{
					MessageBox(message_box_msg.ARMD_file_not_found.text, message_box_msg.ARMD_file_not_found.title, MB_OK | MB_ICONERROR);
					status_bar.SetPaneText(0, status_bar_msg.ready);
					return;
				}
			}
		}
		else
		{
			MessageBox(message_box_msg.ARMD_file_not_found.text, message_box_msg.ARMD_file_not_found.title, MB_OK | MB_ICONERROR);
			status_bar.SetPaneText(0, status_bar_msg.ready);
			return;
		}
	}
	else
	{
		status_bar.SetPaneText(0, status_bar_msg.ready);
	}

}


void CARMDDlg::OnNMKillfocusDatetimepicker2(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CARMDDlg::OnNMKillfocusDatetimepicker4(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CARMDDlg::OnNMKillfocusDatetimepicker3(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CARMDDlg::OnNMKillfocusDatetimepicker5(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CARMDDlg::OnNMThemeChangedDatetimepicker2(NMHDR* pNMHDR, LRESULT* pResult)
{
	// This feature requires Windows XP or greater.
	// The symbol _WIN32_WINNT must be >= 0x0501.
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

int CARMDDlg::LoadSysConfiguration(CPathOperations* path)
{
	SYSTEMTIME* st_from = new SYSTEMTIME;
	SYSTEMTIME* st_to = new SYSTEMTIME;
	m_from_date.GetTime(st_from);
	m_to_date.GetTime(st_to);
	DWORD tmp;
	BYTE exist = 1;
	char* tmp_str = new char[256];
	HANDLE H = CreateFile(path->GetFullPath(NULL, 0), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (H == INVALID_HANDLE_VALUE)
	{
		delete st_from;
		delete st_to;
		delete tmp_str;
		return -1;
	}
	SetFilePointer(H, 0, 0, FILE_BEGIN);
	ReadFile(H, tmp_str, 8 * sizeof(BYTE), &tmp, 0);
	if (memcmp(tmp_str, "ARMDTSR10", 8 * sizeof(BYTE)) != 0)
	{
		delete st_from;
		delete st_to;
		delete tmp_str;
		return -1;
	}

	SetFilePointer(H, DATE_FROM_EXIST, 0, FILE_BEGIN);
	ReadFile(H, &exist, sizeof(BYTE), &tmp, 0);
	if (exist == 1)
	{
		SetFilePointer(H, DATE_FROM_OFFSET, 0, FILE_BEGIN);
		ReadFile(H, &st_from->wYear, sizeof(WORD), &tmp, 0);
		ReadFile(H, &st_from->wMonth, sizeof(WORD), &tmp, 0);
		ReadFile(H, &st_from->wDay, sizeof(WORD), &tmp, 0);
	}

	SetFilePointer(H, TIME_FROM_EXIST, 0, FILE_BEGIN);
	ReadFile(H, &exist, sizeof(BYTE), &tmp, 0);
	if (exist == 1)
	{
		SetFilePointer(H, TIME_FROM_OFFSET, 0, FILE_BEGIN);
		ReadFile(H, &st_from->wHour, sizeof(WORD), &tmp, 0);
		ReadFile(H, &st_from->wMinute, sizeof(WORD), &tmp, 0);
		ReadFile(H, &st_from->wSecond, sizeof(WORD), &tmp, 0);
	}

	SetFilePointer(H, DATE_TO_EXIST, 0, FILE_BEGIN);
	ReadFile(H, &exist, sizeof(BYTE), &tmp, 0);
	if (exist == 1)
	{
		SetFilePointer(H, DATE_TO_OFFSET, 0, FILE_BEGIN);
		ReadFile(H, &st_to->wYear, sizeof(WORD), &tmp, 0);
		ReadFile(H, &st_to->wMonth, sizeof(WORD), &tmp, 0);
		ReadFile(H, &st_to->wDay, sizeof(WORD), &tmp, 0);
	}


	SetFilePointer(H, TIME_TO_EXIST, 0, FILE_BEGIN);
	ReadFile(H, &exist, sizeof(BYTE), &tmp, 0);
	if (exist == 1)
	{
		SetFilePointer(H, TIME_TO_OFFSET, 0, FILE_BEGIN);
		ReadFile(H, &st_to->wHour, sizeof(WORD), &tmp, 0);
		ReadFile(H, &st_to->wMinute, sizeof(WORD), &tmp, 0);
		ReadFile(H, &st_to->wSecond, sizeof(WORD), &tmp, 0);
	}

	SetFilePointer(H, PRIMARY_OFFSET, 0, FILE_BEGIN);
	BYTE total_proc = 0, curr_proc, set;
	DWORD total_event;
	DWORD len;
	char* str = new char[256];
	ReadFile(H, &total_proc, sizeof(BYTE), &tmp, 0);

	for (BYTE proc = 0; proc < total_proc; proc++)
	{
		ReadFile(H, &curr_proc, sizeof(BYTE), &tmp, 0);
		ReadFile(H, &total_event, sizeof(DWORD), &tmp, 0);
		char** loaded_events = new char* [total_event];

		for (DWORD ii = 0; ii < total_event; ii++)
		{
			loaded_events[ii] = new char[256];
		}

		BYTE* check_events = new BYTE[total_event];
		for (DWORD loaded_events_i = 0; loaded_events_i < total_event; loaded_events_i++)
		{
			ReadFile(H, &len, sizeof(DWORD), &tmp, 0);
			ReadFile(H, loaded_events[loaded_events_i], len * sizeof(char), &tmp, 0);
			loaded_events[loaded_events_i][len] = 0;
			ReadFile(H, &check_events[loaded_events_i], sizeof(BYTE), &tmp, 0);
		}

		int event = 1;
		for (event = 0; event < filter_event_process[proc].m_filtred_events.GetCount(); event++)
		{
			for (DWORD ev = 0; ev < total_event; ev++)
			{
				filter_event_process[proc].m_filtred_events.GetText(event, str);
				if (strcmpi(str, loaded_events[ev]) == 0)
				{
					filter_event_process[proc].m_filtred_events.SetCheck(event, check_events[ev]);
					break;
				}
			}
		}


		for (DWORD ii = 0; ii < total_event; ii++)
		{
			delete[] loaded_events[ii];
		}
		delete[] loaded_events;
		delete[] check_events;

	}
	ReadFile(H, &len, sizeof(DWORD), &tmp, 0);
	ReadFile(H, str, len * sizeof(char), &tmp, 0);
	str[len] = 0;
	if (len != 0)
	{
		open_file_name.SetFullPath(str);
		mon_queue_file_name.SetFullPath(str);
		path_bar.SetWindowText(mon_queue_file_name.GetPath(NULL, 0));
		mon_file_qeueue_info = BuildQueue(&mon_queue_file_name);
		if (mon_file_qeueue_info != NULL)
		{
			save_to_text_ctrl.EnableWindow(TRUE);
			armd_file_info_ctrl.EnableWindow(TRUE);
			m_data_synchro.EnableWindow(TRUE);
		}
		else
		{
			save_to_text_ctrl.EnableWindow(FALSE);
			armd_file_info_ctrl.EnableWindow(FALSE);
			m_data_synchro.EnableWindow(FALSE);
			MessageBox(message_box_msg.qeueu_build_error.text, message_box_msg.qeueu_build_error.title, MB_OK);
			//return -1;
		}
	}
	ReadFile(H, &set, sizeof(BYTE), &tmp, 0);
	separate_proc.SetCheck(set);

	filter_event_process[0].ShowWindow(SW_SHOW);


	m_from_date.SetTime(st_from);
	m_from_time.SetTime(st_from);

	m_to_date.SetTime(st_to);
	m_to_time.SetTime(st_to);

	CloseHandle(H);
	if (mon_file_qeueue_info != NULL)
	{
		if (GetTimeLimits() < 0)  return -1;

		char* text = new char[256];
		if (text != NULL)
		{
			sprintf(text, "%.2d.%.2d.%.4d  %.2d:%.2d:%.2d", begin_time_limit[2], begin_time_limit[1], begin_time_limit[0],
				begin_time_limit[3], begin_time_limit[4], begin_time_limit[5], begin_time_limit[6]);
			begin_limit_ctrl.SetWindowText(text);
			sprintf(text, "%.2d.%.2d.%.4d  %.2d:%.2d:%.2d", end_time_limit[2], end_time_limit[1], end_time_limit[0],
				end_time_limit[3], end_time_limit[4], end_time_limit[5], end_time_limit[6]);

			end_limit_ctrl.SetWindowText(text);
			delete[]text;
		}
	}
	delete str;
	delete st_from;
	delete st_to;
	delete tmp_str;
	return 0;
}

void CARMDDlg::SaveSysConfiguration(CPathOperations* path)
{
	SYSTEMTIME st;
	DWORD tmp;
	BYTE exist = 1;
	HANDLE H = CreateFile(path->GetFullPath(NULL, 0), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
	if (H != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(H, 0, 0, FILE_BEGIN);
		WriteFile(H, "ARMDTSR10", 8 * sizeof(BYTE), &tmp, 0);

		m_from_date.GetTime(&st);
		SetFilePointer(H, DATE_FROM_EXIST, 0, FILE_BEGIN);
		WriteFile(H, &exist, sizeof(BYTE), &tmp, 0);

		SetFilePointer(H, DATE_FROM_OFFSET, 0, FILE_BEGIN);
		WriteFile(H, &st.wYear, sizeof(WORD), &tmp, 0);
		WriteFile(H, &st.wMonth, sizeof(WORD), &tmp, 0);
		WriteFile(H, &st.wDay, sizeof(WORD), &tmp, 0);

		m_from_time.GetTime(&st);
		SetFilePointer(H, TIME_FROM_EXIST, 0, FILE_BEGIN);
		WriteFile(H, &exist, sizeof(BYTE), &tmp, 0);
		SetFilePointer(H, TIME_FROM_OFFSET, 0, FILE_BEGIN);
		WriteFile(H, &st.wHour, sizeof(WORD), &tmp, 0);
		WriteFile(H, &st.wMinute, sizeof(WORD), &tmp, 0);
		WriteFile(H, &st.wSecond, sizeof(WORD), &tmp, 0);

		m_to_date.GetTime(&st);
		SetFilePointer(H, DATE_TO_EXIST, 0, FILE_BEGIN);
		WriteFile(H, &exist, sizeof(BYTE), &tmp, 0);
		SetFilePointer(H, DATE_TO_OFFSET, 0, FILE_BEGIN);
		WriteFile(H, &st.wYear, sizeof(WORD), &tmp, 0);
		WriteFile(H, &st.wMonth, sizeof(WORD), &tmp, 0);
		WriteFile(H, &st.wDay, sizeof(WORD), &tmp, 0);

		m_to_time.GetTime(&st);
		SetFilePointer(H, TIME_TO_EXIST, 0, FILE_BEGIN);
		WriteFile(H, &exist, sizeof(BYTE), &tmp, 0);
		SetFilePointer(H, TIME_TO_OFFSET, 0, FILE_BEGIN);
		WriteFile(H, &st.wHour, sizeof(WORD), &tmp, 0);
		WriteFile(H, &st.wMinute, sizeof(WORD), &tmp, 0);
		WriteFile(H, &st.wSecond, sizeof(WORD), &tmp, 0);

		SetFilePointer(H, PRIMARY_OFFSET, 0, FILE_BEGIN);
		BYTE c;
		DWORD len;
		char* str = new char[256];
		c = 6;
		WriteFile(H, &c, sizeof(BYTE), &tmp, 0);
		for (BYTE i = 0; i < 6; i++)
		{
			WriteFile(H, &i, sizeof(BYTE), &tmp, 0);
			len = filter_event_process[i].m_filtred_events.GetCount();
			WriteFile(H, &len, sizeof(DWORD), &tmp, 0);
			for (DWORD j = 0; j < filter_event_process[i].m_filtred_events.GetCount(); j++)
			{
				len = filter_event_process[i].m_filtred_events.GetText(j, str);
				WriteFile(H, &len, sizeof(DWORD), &tmp, 0);
				WriteFile(H, str, len * sizeof(char), &tmp, 0);
				c = filter_event_process[i].m_filtred_events.GetCheck(j);
				WriteFile(H, &c, sizeof(BYTE), &tmp, 0);
			}
		}
		open_file_name.GetFullPath(str, 256);
		len = strlen(str);
		WriteFile(H, &len, sizeof(DWORD), &tmp, 0);
		WriteFile(H, str, len * sizeof(char), &tmp, 0);
		c = separate_proc.GetCheck();
		WriteFile(H, &c, sizeof(BYTE), &tmp, 0);
		CloseHandle(H);
		delete str;
	}
}

void CARMDDlg::OnDestroy()
{
	CDialog::OnDestroy();

	CARMDDlg::SaveSysConfiguration(&ini_path);
	// TODO: Add your message handler code here
}

void CARMDDlg::OnHelpAbout()
{
	// TODO: Add your command handler code here
	CAboutDlg* about = new CAboutDlg;
	about->DoModal();
	delete about;
}

void CARMDDlg::LoadConfiguration()
{
	CPathOperations file;
	CFileDialog* ldconfig = new CFileDialog(TRUE, NULL, "*.ini", NULL, "Config files (*.ini)");
	int rez = ldconfig->DoModal();
	if (rez == IDOK)
	{
		file.SetFullPath((char*)ldconfig->GetPathName().GetString());
		//m_loaded_mon_files.ResetContent();
		for (BYTE proc = 0; proc < 6; proc++)
		{
			CARMDDlg::filter_event_process[proc].m_filtred_events.ResetContent();
			if (proc == 0) FillFilterEventListsProc0(&filter_event_process[proc]);
			else FillFilterEventLists(&filter_event_process[proc]);
		}
		LoadSysConfiguration(&file);
		status_bar.SetPaneText(0, status_bar_msg.confuguration_loaded);
	}
	delete ldconfig;

	// TODO: Add your command handler code here
}

void CARMDDlg::SaveConfigurations()
{
	// TODO: Add your command handler code here
	CPathOperations file;
	CFileDialog* svconfig = new CFileDialog(FALSE, NULL, "*.ini", NULL, "Config files (*.ini)");
	int rez = svconfig->DoModal();
	if (rez == IDOK)
	{
		file.SetFullPath((char*)svconfig->GetPathName().GetString());
		SaveSysConfiguration(&file);
		status_bar.SetPaneText(0, status_bar_msg.configuration_saved);
	}
	delete svconfig;
}

void CARMDDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CARMDFileList* dlg = new CARMDFileList;
	//MonFileQueueInfo *mon_file_info;
	if (mon_file_qeueue_info != NULL && mon_file_qeueue_info->first != NULL)
	{
		dlg->curr = mon_file_qeueue_info->first;
		dlg->DoModal();
	}
	delete dlg;
}


void CARMDDlg::OnBnClickedButton2()
{
	SYSTEMTIME st;
	if (mon_file_qeueue_info != NULL)
	{
		st.wYear = begin_time_limit[0];
		st.wMonth = begin_time_limit[1];
		st.wDay = begin_time_limit[2];
		st.wHour = begin_time_limit[3];
		st.wMinute = begin_time_limit[4];
		st.wSecond = begin_time_limit[5];

		m_from_date.SetTime(st);
		m_from_time.SetTime(st);

		st.wYear = end_time_limit[0];
		st.wMonth = end_time_limit[1];
		st.wDay = end_time_limit[2];
		st.wHour = end_time_limit[3];
		st.wMinute = end_time_limit[4];
		st.wSecond = end_time_limit[5];

		m_to_date.SetTime(st);
		m_to_time.SetTime(st);
	}
	// TODO: добавьте свой код обработчика уведомлений
}
