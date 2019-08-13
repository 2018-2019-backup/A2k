#if !defined(AFX_DCSFLOATEDIT_H__6923D770_6649_11D5_907A_000021227B4F__INCLUDED_)
#define AFX_DCSFLOATEDIT_H__6923D770_6649_11D5_907A_000021227B4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DCSFloatEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDCSFloatEdit window

class CDCSFloatEdit : public CEdit
{
// Construction
public:
	CDCSFloatEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDCSFloatEdit)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	CString csTemp;
  BOOL m_bAllowNegative;
	virtual ~CDCSFloatEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDCSFloatEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DCSFLOATEDIT_H__6923D770_6649_11D5_907A_000021227B4F__INCLUDED_)
