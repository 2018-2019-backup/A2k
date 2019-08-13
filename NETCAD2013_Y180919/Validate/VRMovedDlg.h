#pragma once
#include "afxcmn.h"


// CVRMovedDlg dialog

class CVRMovedDlg : public CDialog
{
	DECLARE_DYNAMIC(CVRMovedDlg)

public:
	CVRMovedDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVRMovedDlg();

  void WriteToReport(CStdioFile &cfLog);

// Dialog Data
	enum { IDD = IDD_VALIDATE_RESULTS_MOVEMENT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnClickObjectsList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedErrorAccept();

	DECLARE_MESSAGE_MAP()

public:
  CListCtrl m_lcMoved;

  CStringArray m_csaMovHandles, m_csaMovNames, m_csaMovLayers, m_csaMovType, m_csaErrorNos;
};
