// ARMD.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ARMD.h"
#include "ARMDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CARMDApp

BEGIN_MESSAGE_MAP(CARMDApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CARMDApp construction

CARMDApp::CARMDApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CARMDApp object

CARMDApp theApp;


// CARMDApp initialization

BOOL CARMDApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	CARMDDlg* dlg = new CARMDDlg;

	m_pMainWnd = dlg;
	INT_PTR nResponse = dlg->DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Add your control notification handler code here

		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	delete dlg;
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
