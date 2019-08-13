#pragma once
#include "afxcmn.h"

#include "VRLayersDlg.h"
#include "VRObjectsDlg.h"
#include "VRBlocksDlg.h"
#include "VRAttribsDlg.h"
#include "VRLayer0Dlg.h"
#include "VRMovedDlg.h"
#include "VRAcceptedDlg.h"

// CValidateResultsDlg dialog

class CValidateResultsDlg : public CDialog
{
	DECLARE_DYNAMIC(CValidateResultsDlg)

public:
	CValidateResultsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CValidateResultsDlg();

// Dialog Data
	enum { IDD = IDD_VALIDATE_RESULTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnSelectTab(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedSaveResults();
  afx_msg void OnBnClickedHelp();
  afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()

public:
  CTabCtrl       m_tabResults;
  CVRLayersDlg   m_dlgLayers;
  CVRObjectsDlg  m_dlgObjects;
  CVRBlocksDlg   m_dlgBlocks;
  CVRAttribsDlg  m_dlgAttribs;
  CVRLayer0Dlg   m_dlgLayer0;
  CVRMovedDlg    m_dlgMoved;
  CVRAcceptedDlg m_dlgAccepted;
};
