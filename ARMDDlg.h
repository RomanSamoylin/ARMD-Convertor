// ARMDDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "FilterEventProcess1.h"
#include "MonitorBase.h"
#include "afxdtctl.h"
#include "afxwin.h"
#include "PathOperations.h"

// CARMDDlg dialog
class CARMDDlg : public CDialog
{
	// Construction
public:
	CARMDDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ARMD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void FillFilterEventLists(CFilterEventProcess1* filter_event_processf);
	void FillFilterEventListsProc0(CFilterEventProcess1* filter_event_processd);
	void LoadFilter();
	MonEventFilter mon_event_filter;
	MonFileQueueInfo* mon_file_qeueue_info;
	CPathOperations mon_queue_file_name;
	CPathOperations save_mon_file_name[6];
	signed char MonSaveToFile();
	CPathOperations pr_name;
	CPathOperations ini_path;
	CPathOperations open_file_name;
	void ReleaseFilter();
	int LoadSysConfiguration(CPathOperations* path);
	void SaveSysConfiguration(CPathOperations* path);
	CStatusBar status_bar;
	WORD begin_time_limit[7], end_time_limit[7];
	int GetTimeLimits();
	//char a[4256];
public:

	struct _status_bar_msg {
		char ready[256];
		char confuguration_loaded[256];
		char configuration_saved[256];
		char empty[256];
		char wait[256];
		char file_saved[256];
		char error[256];
		char queue_build[256];
		char queue_build_error[256];
	}status_bar_msg;

	typedef struct _msg_box_struct {
		char title[256];
		char text[256];
	}msg_box_struct;

	struct _message_box_msg {
		msg_box_struct save_to_text_error;
		msg_box_struct qeueu_build_error;
		msg_box_struct illegal_time_limits;
		msg_box_struct io_error;
		msg_box_struct ARMD_file_not_found;
	}message_box_msg;

	TimeInterval time_interval;
	~CARMDDlg();

	CTabCtrl m_tab_ctrl;
	CFilterEventProcess1* filter_event_process;
	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTcnSelchangingTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSaveToText();
	afx_msg void OnBnClickedOpenMonFile();
	afx_msg void OnNMKillfocusDatetimepicker2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMKillfocusDatetimepicker4(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMKillfocusDatetimepicker3(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMKillfocusDatetimepicker5(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMThemeChangedDatetimepicker2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnHelpAbout();
	afx_msg void LoadConfiguration();
	afx_msg void SaveConfigurations();
	CDateTimeCtrl m_from_date;
	CDateTimeCtrl m_from_time;
	CDateTimeCtrl m_to_date;
	CDateTimeCtrl m_to_time;
	CListBox m_loaded_mon_files;
	CButton separate_proc;
	CEdit path_bar;
	CButton save_to_text_ctrl;
	CEdit begin_limit_ctrl;
	CEdit end_limit_ctrl;
	afx_msg void OnBnClickedButton1();
	CButton armd_file_info_ctrl;
	afx_msg void OnBnClickedButton2();
	CButton m_data_synchro;
};
