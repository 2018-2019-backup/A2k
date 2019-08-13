#pragma once
#include "afxcmn.h"


// CVRBlocksDlg dialog

class CVRBlocksDlg : public CDialog
{
	DECLARE_DYNAMIC(CVRBlocksDlg)

public:
	CVRBlocksDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVRBlocksDlg();

  void WriteToReport(CStdioFile &cfLog);

// Dialog Data
	enum { IDD = IDD_VALIDATE_RESULTS_BLOCKS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnClickObjectsList(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedErrorAccept();

	DECLARE_MESSAGE_MAP()

public:
  CListCtrl m_lcBlocks;

  CStringArray m_csaNSBNames, m_csaNSBLayers, m_csaNSBHandles, m_csaErrorNos;
};
