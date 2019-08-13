#if !defined(AFX_MYB_H__3832DDEF_0C12_11D5_B6BE_00E07D8144D0__INCLUDED_)
#define AFX_MYB_H__3832DDEF_0C12_11D5_B6BE_00E07D8144D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CxSkinButton.h : header file
/* 15/03/2001 v1.00
 * ing.davide.pizzolato@libero.it
 * 29/03/2001 v1.10
 * Milan.Gardian@LEIBINGER.com
 * - mouse tracking
 * 02/04/2001 v1.20
 * Davide Pizzolato
 * - new CreateRgnFromBitmap
 */

/////////////////////////////////////////////////////////////////////////////
// CxSkinButton window
class CxSkinButton : public CButton
{
// Construction
public:
	CxSkinButton();

// Attributes
private:

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CxSkinButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	//}}AFX_VIRTUAL
// Implementation
public:
	void SetToolTipText(CString s);
	COLORREF SetTextColor(COLORREF new_color);
	void SetSkin(UINT normal,UINT down, UINT over=NULL, UINT disabled=NULL,UINT mask=NULL,
				short drawmode=1,short border=1,short margin=4);
	virtual ~CxSkinButton();
	// Generated message map functions
protected:
    bool m_tracking;
    bool m_button_down;
	void RelayEvent(UINT message, WPARAM wParam, LPARAM lParam);
	CToolTipCtrl m_tooltip;
	CBitmap m_bNormal,m_bDown,m_bDisabled,m_bMask,m_bOver; //skin bitmaps
	short	m_FocusRectMargin;		//dotted margin offset
	COLORREF m_TextColor;			//button text color
	HRGN	hClipRgn;				//clipping region
	BOOL	m_Border;				//0=flat; 1=3D;
	short	m_DrawMode;				//0=normal; 1=stretch; 2=tiled;
	HRGN	CreateRgnFromBitmap(HBITMAP hBmp, COLORREF color);
	void	FillWithBitmap(CDC* dc, HBITMAP hbmp, RECT r);
	void	DrawBitmap(CDC* dc, HBITMAP hbmp, RECT r, int DrawMode);
	int		GetBitmapWidth (HBITMAP hBitmap);
	int		GetBitmapHeight (HBITMAP hBitmap);
	//{{AFX_MSG(CxSkinButton)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnClicked();
	//}}AFX_MSG
	afx_msg void OnMouseLeave(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_MYB_H__3832DDEF_0C12_11D5_B6BE_00E07D8144D0__INCLUDED_)
