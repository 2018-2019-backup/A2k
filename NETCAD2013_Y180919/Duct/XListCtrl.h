#if !defined(AFX_XLISTCTRL_H__53D03F04_D1AB_4FEA_83C5_6C846A090E32__INCLUDED_)
#define AFX_XLISTCTRL_H__53D03F04_D1AB_4FEA_83C5_6C846A090E32__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// XListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CXListCtrl window

class CXListCtrl : public CListCtrl
{
// Construction
public:
	CXListCtrl();

// Attributes
public:

// Operations
public:
   BOOL m_bCustomDraw;
   int m_nNumberOfRows;
   int m_nNumberOfCols;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CXListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CXListCtrl)
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	void invalidate_grid(int row,int col);
	afx_msg void custom_draw_funtion(NMHDR *pNMHDR, LRESULT *pResult);
	int m_sel_row;
	int m_sel_col;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XLISTCTRL_H__53D03F04_D1AB_4FEA_83C5_6C846A090E32__INCLUDED_)
