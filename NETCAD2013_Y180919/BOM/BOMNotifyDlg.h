#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "resource.h"


// CBOMNotifyDlg dialog

class CBOMNotifyDlg : public CDialog
{
	DECLARE_DYNAMIC(CBOMNotifyDlg)

public:
	CBOMNotifyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBOMNotifyDlg();

// Dialog Data
	enum { IDD = IDD_BOMNOTIFY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_btnSelectAll;
	CButton m_btnClearAll;
	CButton m_btnSelect;
	CButton m_btnClear;
	CButton m_btnOnScreen;
	
	virtual BOOL OnInitDialog();
	
	afx_msg void OnBnClickedSelectAll();
	afx_msg void OnBnClickedClearAll();

	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedClear();

	afx_msg void OnBnClickedOnScreen();
	afx_msg void OnNMClickListlayout(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();

	CListCtrl m_lcLayout;
	CStringArray m_csaLayouts;
	CStringArray m_csaVports;
	CStringArray m_csaVportsCnt;
	CString m_csLayout;
	int m_iCurrentLayout;
	bool m_bSeparateLineAndBlock;
	int m_iCalledFor;

	void SelecThisViewport(int iSel);
	void Highlight(int iSel, bool bHighlight);
	afx_msg void OnBnClickedHelp();
};
