#pragma once
#include "afxwin.h"
#include "resource.h"
#include "dcsfloatedit.h"


// CInsertPolesAndConductorsDlg dialog

class CInsertPolesAndConductorsDlg : public CDialog
{
	DECLARE_DYNAMIC(CInsertPolesAndConductorsDlg)

public:
	CInsertPolesAndConductorsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInsertPolesAndConductorsDlg();

// Dialog Data
	enum { IDD = IDD_POLESCONDUCTORS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_cbCircuit1;
	CComboBox m_cbCircuit2;
	CComboBox m_cbCircuit3;
	CComboBox m_cbCircuit4;

	CString m_csCircuit1;
	CString m_csCircuit2;
	CString m_csCircuit3;
	CString m_csCircuit4;

	CComboBox m_cbStatus;
	CComboBox m_cbSpacing;
	CString m_csSpacing;
	CString m_csOtherSpacing;
	CString m_csStatus;
	CEdit m_edOtherSpacing;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CString AssignLayerForType(CString csCircuit);
	afx_msg void OnBnClickedHelp();
	afx_msg void OnCbnSelchangeSpacing();
};
