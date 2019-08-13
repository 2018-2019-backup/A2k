#pragma once 

#include "resource.h"
#include "Substation.h"
#include "SubstationData.h"
#include "afxcmn.h"
#include "afxwin.h"

#include "map"
#include "NSDirectives.h"

//typedef std::map<NSSTRING, NSSTRING> MYSTRINGSTRINGMAP;
//MYSTRINGSTRINGMAP mapOfAttributes;

//typedef std::map<NSSTRING, NSSTRING> mapOfAttributes;
//typedef std::map<NSSTRING, NSSTRING>::iterator m_AttMapIterator;



// CEditDlg dialog

class CEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditDlg)

public:
	CEditDlg(CString &strRename, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditDlg();

// Dialog Data
	enum { IDD = IDD_RENAMEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	

	DECLARE_MESSAGE_MAP()
private:
	CString &m_renameString;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};




class CSSTabCtrl :
	public CTabCtrl
{
public:
	CSSTabCtrl(CWnd* pParent = NULL){}
	~CSSTabCtrl(void);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};




// CNSMainSubstaionDlg dialog


class CNSMainSubstaionDlg : public CAcUiDialog
{
private:
	DECLARE_DYNAMIC(CNSMainSubstaionDlg)
	
	CString m_strSSPrefix;
	CString m_strSSNumber;
	CString m_strSSName;
	CString m_strHVCSideA;
	CString m_strHVCSideB;
	CString m_strHVCNumber;
	CString m_strSSOptions;
	bool m_bInitFromRegistry;
	int m_iHVCEfiB;
	int m_iHVCEfiA;
	CFont m_labelFont;
	CWnd* m_pParent;
	CSSData *m_pSSData;
	CSSTabCtrl m_TabDistributor;
	CComboBox m_CbSSType;
	CComboBox m_CbSSSize;
	CComboBox m_CbSSDescription;
	CComboBox m_CbSSStockType;
	CComboBox m_CbHVCEquipment;
	CButton m_BtnSSDetails;
	CComboBox m_CbHVCType;
	CComboBox m_CbHVCRating;
	CComboBox m_CbTXRating;
	CEdit m_EditTXVoltage;
	CEdit m_EditTXPhases;
	CComboBox m_CbTXTapRatio;
	CEdit m_EditTXTapSetting;
	CComboBox m_CbLVNoOfDist;
	CEdit m_EditTXCTRatio;
	CEdit m_EditTXKFactor;
	CComboBox m_CbLVCFuseA;
	CComboBox m_CbLVCFuse;
	CEdit m_EditLVCPanelRating;
	CEdit m_EditLVCDistName;
	CButton m_ChkLVCWDNO;
	CButton m_ChkLVCIDLINK;
	CEdit m_CbSSOptions;
	CEdit m_EditTXLength;
	CEdit m_EditTxStrength;

	MYSTRINGSTRINGMAP mapOfAttributes;
	

	void InitSSControls(const CString &strSSType = "", const CString &strSSSize = "");
	void  ResetCBcontrol(CComboBox &CbControl,const std::vector<variant_t> &vData);
	void UpdateSubstation();
	void InitHVCControls(const CString &strHVCEquip = "", const CString &strHVCType = "");	
	void InitTXControls(const CString &strTXRating, const CString &strTXTapRatio);	
	void InitLVCControls(int iCurTabSelected = 0);
	int PopulateAttributeMapfromUI();
	void Sub_Details_PT();
	void Sub_Details_Kiosk();
	void SetSubstationData();
	void GetSubstationData();
	void resetDescnStockType();


	



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL PreTranslateMessage(MSG* pMsg); 

	DECLARE_MESSAGE_MAP()
public:
	//CNSMainSubstaionDlg(CWnd* pParent = NULL );  // standard constructor
	CNSMainSubstaionDlg(MYSTRINGSTRINGMAP mapOfAttributesFromBlock,CWnd* pParent = NULL );  
	CNSMainSubstaionDlg(MYSTRINGSTRINGMAP mapOfAttributesFromBlock,AcDbHandle handle,CWnd* pParent = NULL );  // edit mode
	virtual ~CNSMainSubstaionDlg();
	NSSTRING getAttributeValue(NSSTRING attName);
	NSSTRING getXDATA(NSSTRING attName,std::map<NSSTRING, NSSTRING> mapXData);
	void setUIControlFromAttributeValues();

// Dialog Data
	enum { IDD = Frm_Substation };

	AcDbHandle m_handle;
	bool IsInEditMode;
	//AcGePoint3d pt;

	afx_msg void OnBnClickedSchematic();
	afx_msg void OnBnClickedSubstation();
	afx_msg void OnBnClickedSSdetails();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeSStype();
	afx_msg void OnCbnSelchangeSSsize();
	afx_msg void OnCbnSelchangeSSdescription();
	afx_msg void OnCbnSelchangeSSStockType();	
	afx_msg void OnCbnSelchangeHVCEquip();
	afx_msg void OnCbnSelchangeHVCType();
	afx_msg void OnCbnSelchangeTXRating();
	afx_msg void OnCbnSelchangeTXTapRatio();
	afx_msg void OnCbnSelchangeHVCRating();
	afx_msg void OnTcnSelchangeLVC(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeLVCFuseA();
	afx_msg void OnCbnSelchangeLVCFuse();
	afx_msg void OnEnChangeLVCDistName();
	afx_msg void OnChkclickedLVCWDNO();
	afx_msg void OnChkClickedLVCIDLINK();
	afx_msg void OnCbnSelchangeLVNoOfDist();
	afx_msg void OnCbnEditchangeHVCType();
	afx_msg void OnCbnEditchangeHVCEquip();
	afx_msg void OnCbnEditchangeHVCRating();
	afx_msg void OnCbnEditchangeTXRating();
	afx_msg void OnEnChangeTXVoltage();
	afx_msg void OnEnChangeTXPhases();
	afx_msg void OnEnChangeTXTapsetting();
	afx_msg void OnCbnEditchangeLVCFuseA();
	afx_msg void OnCbnEditchangeLVCFuse();
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedButton1();
};



