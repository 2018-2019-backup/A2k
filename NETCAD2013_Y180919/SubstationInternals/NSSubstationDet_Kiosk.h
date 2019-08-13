#pragma once
#include "resource.h"
#include "Substationdata.h"
#include "afxwin.h"

// CNSSubstationDet_Kiosk dialog

class CNSSubstationDet_Kiosk : public CDialog
{
	DECLARE_DYNAMIC(CNSSubstationDet_Kiosk)

private:
	CSSData *m_pSSData;
	CString &m_strSSOptions;
	CString m_strSSSize;
	CString m_strSSPrefix;
	CString m_strSSNumber;
	CString m_strSSName;
	CString m_strSSType;
	CString m_strEA;

public:
	CNSSubstationDet_Kiosk(CString &strSSOptions, CSSData *pSSData,CWnd* pParent = NULL);   // standard constructor
	virtual ~CNSSubstationDet_Kiosk();

// Dialog Data
	enum { IDD = Frm_SubstationDet_Kiosk };

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);
public:
	virtual BOOL OnInitDialog();
	CComboBox m_CbKSSOption;
	afx_msg void OnCbnSelchangeKSSOption();
	afx_msg void OnBnClickedOk();
};
