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

	virtual BOOL OnInitDialog();
	CString m_csSpacing;
	afx_msg void OnBnClickedOk();
	CDCSFloatEdit m_edSpacing;
	CComboBox m_cbStatus;
	CString m_csStatus;
	CString AssignLayerForType(CString csCircuit);
	afx_msg void OnBnClickedHelp();
};
