// FilterEventProcess1.cpp : implementation file
//

#include "stdafx.h"
#include "ARMD.h"
#include "FilterEventProcess1.h"


// CFilterEventProcess1 dialog

IMPLEMENT_DYNAMIC(CFilterEventProcess1, CDialog)
CFilterEventProcess1::CFilterEventProcess1(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterEventProcess1::IDD, pParent)
{
}

CFilterEventProcess1::~CFilterEventProcess1()
{
}

void CFilterEventProcess1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_filtred_events);
	DDX_Control(pDX, IDC_APPLY_TO_ALL_PROCESS, apply_to_all_process);
}


BEGIN_MESSAGE_MAP(CFilterEventProcess1, CDialog)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnBnClickedSelectAll)
	ON_BN_CLICKED(IDC_UNSELECT_ALL, OnBnClickedUnselectAll)
	ON_BN_CLICKED(IDC_APPLY_TO_ALL_PROCESS, OnBnClickedApplyToAllProcess)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CFilterEventProcess1 message handlers

void CFilterEventProcess1::OnBnClickedSelectAll()
{
	// TODO: Add your control notification handler code here
	for (int i = 0; i < m_filtred_events.GetCount(); i++)
	{
		m_filtred_events.SetCheck(i, 1);
	}
}

void CFilterEventProcess1::OnBnClickedUnselectAll()
{
	// TODO: Add your control notification handler code here
	for (int i = 0; i < m_filtred_events.GetCount(); i++)
	{
		m_filtred_events.SetCheck(i, 0);
	}
}

void CFilterEventProcess1::OnBnClickedApplyToAllProcess()
{
	// TODO: Add your control notification handler code here
	if (curr_proc == 0) return;
	for (DWORD i = 1; i < num_filtres_proc; i++)
	{
		for (DWORD c = 0; c < m_filtred_events.GetCount(); c++)
		{
			all_filters[i].m_filtred_events.SetCheck(c, m_filtred_events.GetCheck(c));
		}
	}
}

HBRUSH CFilterEventProcess1::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	//HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return bg_brush;
}
