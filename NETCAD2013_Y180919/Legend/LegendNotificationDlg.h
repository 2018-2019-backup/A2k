#pragma once
#include "afxwin.h"
#include "resource.h"
#include "afxcmn.h"

// CLegendNotificationDlg dialog

class CLegendNotificationDlg : public CDialog
{
	DECLARE_DYNAMIC(CLegendNotificationDlg)

public:
	CLegendNotificationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLegendNotificationDlg();

// Dialog Data
	enum { IDD = IDD_LEGENDNOTIFY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// int m_iIndex;
	CStringArray m_csaLayouts;
	CStringArray m_csaVports;
	CStringArray m_csaVportsCnt;
	CListBox m_lcValidationList;
	bool m_bSeparateLineAndBlock;
	CString m_csLayout;
	int m_iCurrentLayout;
	int m_iCalledFor;
	int m_iLegendType;

	CButton m_btnOnScreen;
	CButton m_btnSelect;
	CButton m_btnClear;

	CButton m_btnSelectAll;
	CButton m_btnClearAll;

	CListCtrl m_lcLayout;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedSelectAll();
	afx_msg void OnNMClickListlayout(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedClear();
	afx_msg void OnBnClickedOnscreen();
	afx_msg void OnBnClickedClearall();
	afx_msg void OnBnClickedSelect();
	afx_msg void OnBnClickedHelp();

	void SelecThisViewport(int iSel);
	void Highlight(int iSel, bool bHighlight);
	void EnableAllSuffixedButtons();
};
