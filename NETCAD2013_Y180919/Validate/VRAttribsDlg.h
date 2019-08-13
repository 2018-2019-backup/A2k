#pragma once
#include "afxcmn.h"


// CVRAttribsDlg dialog

class CVRAttribsDlg : public CDialog
{
	DECLARE_DYNAMIC(CVRAttribsDlg)

public:
	CVRAttribsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVRAttribsDlg();

  void WriteToReport(CStdioFile &cfLog);

// Dialog Data
	enum { IDD = IDD_VALIDATE_RESULTS_ATTRIBS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnZoomToObject(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedErrorAccept();

	DECLARE_MESSAGE_MAP()

public:
  CListCtrl m_lcAttribs;

  CStringArray m_csaMAHandles, m_csaMANames, m_csaMAAttribs, m_csaMALayers, m_csaErrorNos;
};
