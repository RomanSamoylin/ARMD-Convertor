#pragma once
#include "afxwin.h"
#include "MonitorBase.h"

// CARMDFileList dialog

class CARMDFileList : public CDialog
{
	DECLARE_DYNAMIC(CARMDFileList)

public:
	CARMDFileList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CARMDFileList();
	MonFileQueue* curr;

	// Dialog Data
	enum { IDD = IDD_ARMD_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox file_list;
	afx_msg void OnLbnSelchangeList1();
	virtual BOOL OnInitDialog();
};
