#pragma once
#include "afxcmn.h"


// CVRLayersDlg dialog

class CVRLayersDlg : public CDialog
{
	DECLARE_DYNAMIC(CVRLayersDlg)

public:
	CVRLayersDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVRLayersDlg();

  BOOL RestoreLayer(int iIndex);
  void WriteToReport(CStdioFile &cfLog);

// Dialog Data
	enum { IDD = IDD_VALIDATE_RESULTS_LAYERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnBnClickedLayersFix();
  afx_msg void OnBnClickedLayersSelectAll();
  afx_msg void OnBnClickedLayersClearAll();

	DECLARE_MESSAGE_MAP()

public:
  CListCtrl m_lcLayers;

  CStringArray m_csaActNames, m_csaActColors, m_csaStdColors, m_csaActLTypes, m_csaStdLTypes, m_csaActWeights, m_csaStdWeights, m_csaErrorNos;
};
