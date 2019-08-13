#pragma once
#include "afxcmn.h"


// CVRLayer0Dlg dialog

class CVRLayer0Dlg : public CDialog
{
	DECLARE_DYNAMIC(CVRLayer0Dlg)

public:
	CVRLayer0Dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVRLayer0Dlg();

  void WriteToReport(CStdioFile &cfLog);

// Dialog Data
	enum { IDD = IDD_VALIDATE_RESULTS_LAYER0 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnZoomToObject(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedErrorAccept();

	DECLARE_MESSAGE_MAP()

public:
  CListCtrl m_lcLayer0;

  CStringArray m_csa0Handles, m_csa0Names, m_csaErrorNos, m_csaErrHandles;
};
