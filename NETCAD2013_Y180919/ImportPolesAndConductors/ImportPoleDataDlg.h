#pragma once
#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "ConductorSpanInfo.h"


// CImportPoleDataDlg dialog

class CImportPoleDataDlg : public CDialog
{
	DECLARE_DYNAMIC(CImportPoleDataDlg)

public:
	CImportPoleDataDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImportPoleDataDlg();

// Dialog Data
	enum { IDD = IDD_IMPORTPOLEDATA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedOkOLD();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnCbnSelChangeOffset();
	CString AssignLayerForType(CString csCircuit, CString csLayerSuffix);
	void DrawConductorsCollected(CStringArray &csaCablesInFile, CStringArray &csaCableSortOrder, std:: vector <CConductorSpanInfo> &conductorSpan_vector, double dOffset);
		
	virtual BOOL OnInitDialog();
	CComboBox m_cbOffset;
	CString m_csOffset;
	CString m_csOtherSpacing;
	CEdit m_edOtherOffsetValue;
	
	CProgressCtrl m_Progress;
	CString m_csPathName;
	bool m_bPolesOnly;
	AcDbObjectIdArray m_objIds;
	CComboBox m_cbSide;
	CString m_csSide;

	CStringArray m_csaCircuits;
	CStringArray m_csaPropLayers;
	CStringArray m_csaExistLayers;
	afx_msg void OnBnClickedUndo();
	afx_msg void OnBnClickedHelp();
};
