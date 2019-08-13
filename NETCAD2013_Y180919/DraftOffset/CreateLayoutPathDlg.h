#pragma once
#include "afxwin.h"
#include "resource.h"
#include "DCSFloatEdit.h"


// CCreateLayoutPathDlg dialog

class CCreateLayoutPathDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreateLayoutPathDlg)

public:
	CCreateLayoutPathDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCreateLayoutPathDlg();

// Dialog Data
	enum { IDD = IDD_CREATELAYOUTPATH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_cbOffset;
	CComboBox m_cbUsage;
	CComboBox m_cbVoltage;
	CComboBox m_cbStatus;
	CComboBox m_cbConstruction;
	CComboBox m_cbNumberOfCables;

	CStringArray m_csaOFSFor;
	CStringArray m_csaOFSPmpt1;
	CStringArray m_csaOFSPmpt2;
	CStringArray m_csaOFSLayer;

	CString m_csUsage;
	CString m_csOffset;
	CString m_csVoltage;
	CString m_csStatus;
	CString m_csConstruction;
	CString m_csLayer;
	CString m_csNoOfCables;

	bool m_bIsOverhead;

	afx_msg void OnCbnSelchangeOfsUsage();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedHelp();
};

