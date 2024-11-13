// ARMD.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define DATE_FROM_EXIST	11
#define TIME_FROM_EXIST	12
#define DATE_TO_EXIST	13
#define TIME_TO_EXIST	14

#define DATE_FROM_OFFSET 20
#define TIME_FROM_OFFSET 26
#define DATE_TO_OFFSET 32
#define TIME_TO_OFFSET 38
#define PRIMARY_OFFSET 44
// CARMDApp:
// See ARMD.cpp for the implementation of this class
//

class CARMDApp : public CWinApp
{
public:
	CARMDApp();

	// Overrides
public:
	virtual BOOL InitInstance();

	// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CARMDApp theApp;