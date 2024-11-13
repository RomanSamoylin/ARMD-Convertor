#pragma once
#include "afxwin.h"


// CFilterEventProcess1 dialog

class CFilterEventProcess1 : public CDialog
{
	DECLARE_DYNAMIC(CFilterEventProcess1)

public:
	CFilterEventProcess1(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFilterEventProcess1();

	// Dialog Data
	enum { IDD = IDD_EVENT_CHOICE_PROC1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CCheckListBox m_filtred_events;
	CFilterEventProcess1* all_filters;
	BYTE num_filtres_proc;
	BYTE curr_proc;
	//CListBox m_filtred_events;
	afx_msg void OnBnClickedSelectAll();
	afx_msg void OnBnClickedUnselectAll();
	afx_msg void OnBnClickedApplyToAllProcess();
	CBrush bg_brush;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CButton apply_to_all_process;
};
