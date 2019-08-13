#pragma once
#include "afxwin.h"


// CAddressLogoDlg dialog

class CAddressLogoDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddressLogoDlg)

public:
	CAddressLogoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddressLogoDlg();

// Dialog Data
	enum { IDD = IDD_ADDRESS_LOGO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnSelectDepot();
  afx_msg void OnBnClickedHelp();
  afx_msg void OnBnClickedOk();
	DECLARE_MESSAGE_MAP()

public:
  CComboBox m_cbDepot;
  CComboBox m_cbLogo;
  CString m_csDepot;
  CString m_csLogo;
  CString m_csAddress;
  CString m_csPhone;
  CString m_csFax;
  CString m_csMobile;

  CString m_csRegSection;
  CString m_csAIO;
  CStringArray m_csaAddress1, m_csaAddress2, m_csaPhone, m_csaFax, m_csaMobile;
};
