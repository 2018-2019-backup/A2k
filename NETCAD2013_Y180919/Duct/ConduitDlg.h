#pragma once
#include "resource.h"
#include "afxwin.h"


// CConduitDlg dialog

class CConduitDlg : public CDialog
{
	DECLARE_DYNAMIC(CConduitDlg)

public:
	CConduitDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConduitDlg();

// Dialog Data
	enum { IDD = IDD_CONDUIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	CString m_csCable;
	CString m_csConduit;
	CString m_csCableStatus;
	CString m_csTrenchStatus;

	CComboBox m_cbConduit;
	CComboBox m_cbCable;
	CComboBox m_cbTrenchStatus;
	CComboBox m_cbCableStatus;
	
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnCbnSelchangeCable();
	afx_msg void OnBnClickedHelp();
	
};
