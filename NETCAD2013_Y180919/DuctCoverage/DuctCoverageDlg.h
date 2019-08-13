#pragma once
#include "resource.h"
#include "xSkinButton.h"
#include "afxwin.h"

// CDuctCoverageDlg dialog

class CDuctCoverageDlg : public CDialog
{
	DECLARE_DYNAMIC(CDuctCoverageDlg)

public:
	CDuctCoverageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDuctCoverageDlg();

	int m_iCalledFor;
	CString m_csWidth;
	CString m_csOffset;
	CString m_csType;
	CString m_csSelectedText;
	CString m_csStatus;
	CString m_csPosition;

	AcDbObjectId m_objSel;
	AcDbObjectId m_objReference;
	ads_point m_ptPick;

	AcGePoint3dArray m_gePtArray;
	AcDbObjectIdArray m_objSelArray;
	
	int m_iOffsetFactor;
	CString m_csOldType;

	CxSkinButton m_btnPiclWidth;
	CxSkinButton m_btnSelectType;
	CxSkinButton m_btnOffset;
	CComboBox m_cbType;
	CComboBox m_cbStatus;
	CComboBox m_cbPosition;

// Dialog Data
	enum { IDD = IDD_DUCTCOVERAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedDcPickwidth();
	afx_msg void OnCbnSelchangeDcType();
	afx_msg void OnBnClickedRvSelecttype();
	afx_msg void OnBnClickedOk();
	
	afx_msg void OnBnClickedDcOn();
	afx_msg void OnBnClickedDcOff();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnKillfocusDcOffset();

	void ResetSelectedObject();
	CButton m_btnOnOff;
	afx_msg void OnBnClickedDcPickoffset();
	CStatic m_btnInfo;
	afx_msg void OnBnClickedHelp();
};
