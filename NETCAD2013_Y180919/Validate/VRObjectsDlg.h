#pragma once
#include "afxcmn.h"


// CVRObjectsDlg dialog

class CVRObjectsDlg : public CDialog
{
	DECLARE_DYNAMIC(CVRObjectsDlg)

public:
	CVRObjectsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVRObjectsDlg();

  BOOL RestoreObject(int iIndex);
  void WriteToReport(CStdioFile &cfLog);

// Dialog Data
	enum { IDD = IDD_VALIDATE_RESULTS_OBJECTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnZoomToObject(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedObjectsFix();
  afx_msg void OnBnClickedObjectsSelectAll();
  afx_msg void OnBnClickedObjectsClearAll();
  afx_msg void OnBnClickedErrorAccept();

	DECLARE_MESSAGE_MAP()

public:
  CListCtrl m_lcObjects;

  CStringArray m_csaObjTypes, m_csaObjHandles, m_csaObjActLayers, m_csaObjStdLayers, m_csaErrorNos, m_csaErrorHandles;
};
