#pragma once
#include "afxcmn.h"


// CSelectTemplateDlg dialog

class CSelectTemplateDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectTemplateDlg)

public:
	CSelectTemplateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectTemplateDlg();

// Dialog Data
	enum { IDD = IDD_SELECT_TEMPLATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnClickTemplateList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnDoubleClickTemplateList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedBrowse();
  afx_msg void OnBnClickedHelp();
  afx_msg void OnBnClickedOk();

	DECLARE_MESSAGE_MAP()

public:
  CString m_csStdPath;
  CListCtrl m_lcTemplates;
  CString m_csTemplate;
};
