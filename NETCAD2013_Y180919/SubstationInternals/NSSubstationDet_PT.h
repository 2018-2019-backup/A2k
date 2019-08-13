#pragma once
#include "resource.h"
#include "Substationdata.h"
#include "afxwin.h"

// CNSSubstationDet_PT dialog

class CNSSubstationDet_PT : public CDialog
{
	DECLARE_DYNAMIC(CNSSubstationDet_PT)
private:
	CSSData *m_pSSData;
	CString &m_strSSOptions;
	CString m_strMasterOptions;
	CString m_strSSSize;
	CString m_strSSPrefix;
	CString m_strSSNumber;
	CString m_strSSName;
	CString m_strSSType;
	CString m_strTXRating;
	CString m_strTxPoleLength;
	CString m_strTxPoleStrength;
	CComboBox m_cbPTLength;
	CComboBox m_CbPTStrength;
	CString m_strDetails[15];
	CComboBox m_CbItem[15];
	void ResetCBcontrol(CComboBox &CbControl,const std::vector<variant_t> &vData);	
	void updateSubstationOptions(int iCurItem);



public:
	CNSSubstationDet_PT(CString &strSSOptions, CSSData *pSSData,CWnd* pParent = NULL);   // standard constructor
	virtual ~CNSSubstationDet_PT();

// Dialog Data
	enum { IDD = Frm_SubstationDet_PT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeItem1(){ updateSubstationOptions(1);}
	afx_msg void OnCbnSelchangeItem2(){ updateSubstationOptions(2);}
	afx_msg void OnCbnSelchangeItem3(){ updateSubstationOptions(3);}
	afx_msg void OnCbnSelchangeItem4(){ updateSubstationOptions(4);}
	afx_msg void OnCbnSelchangeItem5(){ updateSubstationOptions(5);}
	afx_msg void OnCbnSelchangeItem6(){ updateSubstationOptions(6);}
	afx_msg void OnCbnSelchangeItem7(){ updateSubstationOptions(7);}
	afx_msg void OnCbnSelchangeItem8(){ updateSubstationOptions(8);}
	afx_msg void OnCbnSelchangeItem9(){ updateSubstationOptions(9);}
	afx_msg void OnCbnSelchangeItem10(){ updateSubstationOptions(10);}
	afx_msg void OnCbnSelchangeItem11(){ updateSubstationOptions(11);}
	afx_msg void OnCbnSelchangeItem12(){ updateSubstationOptions(12);}
	afx_msg void OnCbnSelchangeItem13(){ updateSubstationOptions(13);}
	afx_msg void OnCbnSelchangeItem14(){ updateSubstationOptions(14);}
	afx_msg void OnCbnSelchangeItem15(){ updateSubstationOptions(15);}
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnEditchangePTLength();
	afx_msg void OnCbnEditchangePTStrength();
	afx_msg void OnCbnSelchangePTLength();
	afx_msg void OnCbnSelchangePTStrength();
};
