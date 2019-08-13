#pragma once
#include "afxcmn.h"


// CVRAcceptedDlg dialog

class CVRAcceptedDlg : public CDialog
{
	DECLARE_DYNAMIC(CVRAcceptedDlg)

public:
	CVRAcceptedDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVRAcceptedDlg();

// Dialog Data
	enum { IDD = IDD_VALIDATE_RESULTS_ACCEPT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnClickAcceptedList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnDoubleClickAcceptList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedAcceptSelectAll();
  afx_msg void OnBnClickedAcceptClearAll();
  afx_msg void OnBnClickedAcceptClear();

	DECLARE_MESSAGE_MAP()

public:
  CListCtrl m_lcAccepted;

  void UpdateAcceptedList();
  void WriteToReport(CStdioFile &cfLog);
};
