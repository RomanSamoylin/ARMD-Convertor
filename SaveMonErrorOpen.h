#pragma once


// CSaveMonErrorOpen dialog

class CSaveMonErrorOpen : public CDialog
{
	DECLARE_DYNAMIC(CSaveMonErrorOpen)

public:
	CSaveMonErrorOpen(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSaveMonErrorOpen();

	// Dialog Data
	enum { IDD = IDD_MON_OPEN_ERROR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
