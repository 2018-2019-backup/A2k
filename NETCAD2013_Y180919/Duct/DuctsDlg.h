#pragma once
#include "afxcmn.h"
#include "XListCtrl.h"
#include "afxwin.h"
#include "ConduitAndCableInfo.h"
#include "resource.h"

// CDuctsDlg dialog

class CDuctsDlg : public CDialog
{
	DECLARE_DYNAMIC(CDuctsDlg)

public:
	CDuctsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDuctsDlg();

// Dialog Data
	enum { IDD = IDD_DUCTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CXListCtrl m_lcConduits;
	CComboBox m_cbStatus;
	CComboBox m_cbCoverStrip;
	CComboBox m_cbCoverStripDepth;
	CComboBox m_cbCol;
	CComboBox m_cbRow;
	CComboBox m_cbTrenchWidth;
		
	CUIntArray m_uiaCableColor;
	ads_point m_ptPrev;
	ads_point m_ptNext;
	AcGePoint2dArray m_geDuctPath;
	
	AcDbObjectId m_objIdOffset1;
	AcDbObjectId m_objIdOffset2;
	AcDbObjectId m_objIdEnd1;
	AcDbObjectId m_objIdEnd2;
	AcDbObjectIdArray m_aryObjIds;

	int m_iCols;
	int m_iRows;
	int m_iDlgExitIndex;
	
	CString m_csTrenchWidth;
	CString m_csTrenchDepth;
	CString m_csLabel;
	CString m_csCols;
	CString m_csRows;
	CString m_csCoverStrip;
	CString m_csCoverStripDepth;
	CString m_csStatus;
	CString m_csLayer;
	CString m_csConduitStatus;

	bool m_bReference;
	bool m_bDrawTrench;
	CButton m_btnReference;
	CString m_csNotes;
	CButton m_btnDrawTrench;
	CEdit m_edNotes;
	CStatic m_stTrench;
	CStatic m_stPreview;
	CEdit m_edTrenchDepth;
	CStatic m_stWidth;
	CButton m_btnDrawFill;
	BOOL m_bDrawFill;
	std::vector <CConduitAndCableInfo> m_conduitConductorVectorInfo;

public:
	virtual BOOL OnInitDialog();
	// void DrawDuctCoverage();
	void DrawCrosssection();
	int DrawCrosssectionPreview(ads_point ptMid);
	void SpecifyConduit(int iRow, int iCol);

private:
	void BuildConduitGrid();
	void SetHeaderText(int iCol, CString csText);
	void SaveConfig(bool bClearArray);
	bool ValidateInputs();

	afx_msg void OnBnClickedDuctDrawcrosssection();
	afx_msg void OnBnClickedDuctRefresh();
	afx_msg void OnBnClickedDuctCoverage();
	afx_msg void OnCbnEditchangeDuctCoversdepth();
	afx_msg void OnCbnSelchangeDuctCoversdepth();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedClear();
	afx_msg void OnCbnSelchangeCols();
	afx_msg void OnCbnSelchangeRows();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedDuctDrawFill();
	void ShowImage();
	void LocateUB(int iRow, int iCol, double dScale, double &dOffsetX, double &dOffsetY);
	void DrawConduitsAndCables(ads_point ptSection, double dScale, bool bMSPace, AcDbObjectIdArray &aryXSectionObjIds);
	void DrawConduitsAndCablesForUBColumn(ads_point ptSection, double dScale, bool bMSPace, AcDbObjectIdArray &aryXSectionObjIds);
	void DrawConduitsAndCablesForUBRow(ads_point ptSection, double dScale, bool bMSPace, AcDbObjectIdArray &aryXSectionObjIds);
public:
	afx_msg void OnCbnSelchangeDuctStatus();
	afx_msg void OnBnClickedChelp();

	int m_iUBRow, m_iUBCol;
	double m_dMaxUnderBWidth;
	double m_dMaxUnderBDepth;

	//int m_iUBRow; // Row at which the max. under bore width was found
	//int m_iUBCol; // Column at which the max. under bore column was found
};
