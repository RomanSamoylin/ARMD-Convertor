// ARMDFileList.cpp : implementation file
//

#include "stdafx.h"
#include "ARMD.h"
#include "ARMDFileList.h"


// CARMDFileList dialog

IMPLEMENT_DYNAMIC(CARMDFileList, CDialog)
CARMDFileList::CARMDFileList(CWnd* pParent /*=NULL*/)
	: CDialog(CARMDFileList::IDD, pParent)
{
	curr = NULL;
}

CARMDFileList::~CARMDFileList()
{
}

void CARMDFileList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, file_list);
}


BEGIN_MESSAGE_MAP(CARMDFileList, CDialog)
	ON_LBN_SELCHANGE(IDC_LIST1, OnLbnSelchangeList1)
END_MESSAGE_MAP()


// CARMDFileList message handlers

void CARMDFileList::OnLbnSelchangeList1()
{
	// TODO: Add your control notification handler code here
}


BOOL CARMDFileList::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	char* str = new char[256];
	while (curr != NULL)
	{
		sprintf(str, "%s День:%.2d Месяц:%.2d Год:%.4d  Часы:%.2d Минуты:%.2d Секунды:%.2d ",
			curr->file_info->name.GetFileName(NULL, 0), curr->file_info->time[2], curr->file_info->time[1],
			curr->file_info->time[0],
			curr->file_info->time[3], curr->file_info->time[4], curr->file_info->time[5]);
		file_list.AddString(str);
		curr = curr->next;
	}
	delete[]str;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
