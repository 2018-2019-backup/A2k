#pragma once
#include "resource.h"
#include "xSkinButton.h"
#include "afxwin.h"

// CRouteVPortDlg dialog

class CRouteVPortDlg : public CDialog
{
	DECLARE_DYNAMIC(CRouteVPortDlg)

public:
	CRouteVPortDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRouteVPortDlg();

// Dialog Data
	enum { IDD = IDD_ROUTEVPORTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_iCalledFor;
	CString m_csSheet;
	CString m_csSelectedText;
	
	CString m_csLength;
	CString m_csWidth;
	CString m_csOverlap;

	CxSkinButton m_btnPickLength;
	CxSkinButton m_btnPickWidth;
	CxSkinButton m_btnPickOverlap;

	CComboBox m_cbSheetSize;
	CComboBox m_cbScale;
	CString m_csScale;
	CButton m_btnEditRoute;

	double m_dVPortLength;
	double m_dVPortWidth;
	AcDbObjectId m_objRoute;
	
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedNewRoute();
	afx_msg void OnBnClickedEditRoute();
	afx_msg void OnBnClickedSelectpline();
	afx_msg void OnBnClickedPickLength();
	afx_msg void OnBnClickedPickWidth();
	afx_msg void OnBnClickedPickOverlap();
	afx_msg void OnCbnSelchangeSheetSize();
	afx_msg void OnCbnSelchangeScale();
	
	void CalculatePermissibleSizes();
	void PopulateLayoutNames();
	
	void DefineRoute();
	void RemoveRoute();

	void GenerateDesign(AcDbObjectIdArray &objVPorts, CStringArray &csaSheetNo);
protected:
	virtual void OnOK();
public:
	afx_msg void OnBnClickedHelp();
	CStatic m_btnInfo;
};
