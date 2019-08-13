#pragma once

#include "resource.h"

// CLayOutSelDlg dialog

class CLayOutSelDlg : public CDialog
{
	DECLARE_DYNAMIC(CLayOutSelDlg)

public:
	CLayOutSelDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLayOutSelDlg();
	virtual BOOL OnInitDialog();
	int PopulateLayOutNames();

// Dialog Data
	enum { IDD = IDD_LAYOUT_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString strAvailableLayOutNames;
	CComboBox m_AvailableLayOutNames;

	CString strSeletedLayOutName;
  afx_msg void OnBnClickedOk();
};
