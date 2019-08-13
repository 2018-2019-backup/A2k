// BalloonHelp.cpp : header file
// Copyright 2001, Joshua Heyer
//  You are free to use this code for whatever you want, provided you
// give credit where credit is due.
//  I'm providing this code in the hope that it is useful to someone, as i have
// gotten much use out of other peoples code over the years.
//  If you see value in it, make some improvements, etc., i would appreciate it 
// if you sent me some feedback.
//

#ifndef _BALLOON_HELP_H_INCLUDED_
#define _BALLOON_HELP_H_INCLUDED_

#include <afxmt.h>


class CBalloonHelp : public CWnd
{
public:
	CBalloonHelp();
	virtual ~CBalloonHelp();

   // options
   static const unsigned int unCLOSE_ON_LBUTTON_UP;   // closes window on WM_LBUTTON_UP
   static const unsigned int unCLOSE_ON_RBUTTON_UP;   // closes window on WM_RBUTTON_UP
   static const unsigned int unCLOSE_ON_MOUSE_MOVE;   // closes window when user moves mouse past threshhold
   static const unsigned int unCLOSE_ON_KEYPRESS;     // closes window on the next keypress message sent to this thread.    (!!! probably not thread safe !!!)
   static const unsigned int unDELETE_THIS_ON_CLOSE;  // deletes object when window is closed.  Used by LaunchBalloon(), use with care
   static const unsigned int unSHOW_CLOSE_BUTTON;     // shows close button in upper right
   static const unsigned int unSHOW_INNER_SHADOW;     // draw inner shadow in balloon
   static const unsigned int unSHOW_TOPMOST;          // place balloon above all other windows
   static const unsigned int unDISABLE_FADEIN;        // disable the fade-in effect (overrides system and user settings)
   static const unsigned int unDISABLE_FADEOUT;       // disable the fade-out effect (overrides system and user settings)
   static const unsigned int unDISABLE_FADE;          // disable the fade-in/fade-out effects (overrides system and user settings)

   BOOL Create(int iControlID,                  // id of the control for whom this message is generated
               const CString& strTitle,         // title of balloon
               const CString& strContent,       // content of balloon
               const CPoint& ptAnchor,          // anchor (tail position) of balloon
               unsigned int unOptions,          // options (see above)
               CWnd* pParentWnd = NULL,         // parent window (NULL == MFC main window)
               const CString strURL = "",       // URL to open (ShellExecute()) when clicked
               unsigned int unTimeout = 0,      // delay before closing automatically (milliseconds)
               HICON hIcon = NULL);             // icon to display

   // Show a help balloon on screen.
   static void LaunchBalloon(int iControlID, const CString& strTitle, const CString& strContent, 
               const CPoint& ptAnchor, 
               LPCTSTR szIcon = IDI_EXCLAMATION,
               unsigned int unOptions = unSHOW_CLOSE_BUTTON,
               CWnd* pParentWnd = NULL, 
               const CString strURL = "",
               unsigned int unTimeout = 10000);

   // Sets the font used for drawing the balloon title.  Deleted by balloon, do not use CFont* after passing to this function.
   void SetTitleFont(CFont* pFont);
   // Sets the font used for drawing the balloon content.  Deleted by balloon, do not use CFont* after passing to this function.
   void SetContentFont(CFont* pFont);
   // Sets the icon displayed in the top left of the balloon (pass NULL to hide icon)
   void SetIcon(HICON hIcon);
   // Sets the icon displayed in the top left of the balloon (pass NULL hBitmap to hide icon)
   void SetIcon(HBITMAP hBitmap, COLORREF crMask);
   // Sets the icon displayed in the top left of the balloon
   void SetIcon(HBITMAP hBitmap, HBITMAP hMask);
   // Set icon displayed in the top left of the balloon to image # nIconIndex from pImageList
   void SetIcon(CImageList* pImageList, int nIconIndex);
   // Sets the URL to be opened when balloon is clicked.  Pass "" to disable.
   void SetURL(const CString& strURL);
   // Sets the number of milliseconds the balloon can remain open.  Set to 0 to disable timeout.
   void SetTimeout(unsigned int unTimeout);
   // Sets the distance the mouse must move before the balloon closes when the unCLOSE_ON_MOUSE_MOVE option is set.
   void SetMouseMoveTolerance(int nTolerance);
   // Sets the point to which the balloon is "anchored"
   void SetAnchorPoint(CPoint ptAnchor);
   // Sets the title of the balloon
   void SetTitle(const CString& strTitle);
   // Sets the content of the balloon (plain text only)
   void SetContent(const CString& strContent);
   // Sets the forground (text and border) color of the balloon
   void SetForegroundColor(COLORREF crForeground);
   // Sets the background color of the balloon
   void SetBackgroundColor(COLORREF crBackground);
protected:
   // layout constants
   static const int nTIP_TAIL;
   static const int nTIP_MARGIN;
   // mouse move tolerance
   static const int nMOUSE_MOVE_TRIGGER;

   enum BALLOON_QUADRANT { BQ_TOPRIGHT, BQ_TOPLEFT, BQ_BOTTOMRIGHT, BQ_BOTTOMLEFT };
   BALLOON_QUADRANT GetBalloonQuadrant();

   // Calculate the dimensions and draw the balloon header
   virtual CSize DrawHeader(CDC* pDC, bool bDraw = TRUE);
   // Calculate the dimensions and draw the balloon contents
   virtual CSize DrawContent(CDC* pDC, int nTop, bool bDraw = TRUE);
   // Calculate the dimensions required to draw the balloon header
   CSize CalcHeaderSize(CDC* pDC) { return DrawHeader(pDC, FALSE); }
   // Calculate the dimensions required to draw the balloon content
   CSize CalcContentSize(CDC* pDC) { return DrawContent(pDC, 0, FALSE); }
   // Calculate the total size needed by the balloon window
	CSize CalcWindowSize();
   // Calculate the total size needed by the client area of the balloon window
	CSize CalcClientSize();
   // Size and position the balloon window on the screen.
	void PositionWindow();

   // Displays the balloon on the screen, performing fade-in if enabled.
   void ShowBalloon(void);
   // Removes the balloon from the screen, performing the fade-out if enabled
   void HideBalloon(void);

   // Returns the class ATOM for a BalloonHelp control.  Registers the class first, if necessary.
   static ATOM GetClassAtom();

   // message handlers
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg void OnLButtonDown(UINT, CPoint point);
	afx_msg void OnLButtonUp(UINT, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);

	
#ifdef WIN64
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#else
	afx_msg void OnTimer(UINT nIDEvent);
#endif

	//afx_msg void OnTimer(UINT_PTR nIDEvent);//64-bit
	////afx_msg void OnTimer(UINT nIDEvent);//Use this for 32-bit

   afx_msg void OnMouseMove(UINT, CPoint point);
   afx_msg void OnDestroy();
   afx_msg void OnRButtonUp(UINT, CPoint);
   afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()
private:
   // keyboard hook callback
   static LRESULT CALLBACK KeyboardProc( int code, WPARAM wParam, LPARAM lParam);
   // handle to the hook, if set
   static HHOOK            s_hKeyboardHook;
   // windows to close when user hits a key
   static CPtrArray        s_apKeyboardCloseWnds;
   // synchronization
   static CCriticalSection s_KeyboardHookSection;

   // Sets up the keyboard hook; adds this to list to close
   void SetKeyboardHook();
   // Removes this from the list of windows to close; removes hook if not in use.
   void RemoveKeyboardHook();

   // Wrapper for possibly-existing API call
   BOOL SetLayeredWindowAttributes(COLORREF crKey, int nAlpha, DWORD dwFlags);
   // layered window API, if available
   typedef BOOL (WINAPI* FN_SET_LAYERED_WINDOW_ATTRIBUTES)(HWND,COLORREF,BYTE,DWORD);
   FN_SET_LAYERED_WINDOW_ATTRIBUTES m_fnSetLayeredWindowAttributes;

   int            m_iControlID;
   unsigned int   m_unOptions;
   unsigned int   m_unTimeout;      // max time to show, in milliseconds
   CString        m_strContent;     // text to show in content area
   CString        m_strURL;         // url to open, if clicked.
   CPoint         m_ptAnchor;       // "anchor" (point of tail)
   CImageList     m_ilIcon;         // icon

   CFont*         m_pTitleFont;     // font to use for title
   CFont*         m_pContentFont;   // font to use for content
   
   COLORREF       m_crBackground;   // Background color for balloon   
   COLORREF       m_crForeground;   // Foreground color for balloon
   
   CRgn           m_rgnComplete;    // Clipping / Drawing region
   int            m_nAlpha;         // current alpha, for fade in / fade out
   CPoint         m_ptMouseOrig;    // original mouse position; for hiding on mouse move
   UINT           m_uCloseState;    // current state of the close button
   int            m_nMouseMoveTolerance;  // distance mouse has to move before balloon will close.

   static ATOM    m_ClassAtom;      // class atom
protected:
   virtual void PostNcDestroy();
};

#endif // _BALLOON_HELP_H_INCLUDED_