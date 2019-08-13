// BalloonHelp.cpp : implementation file
// Copyright 2001, Joshua Heyer
//  You are free to use this code for whatever you want, provided you
// give credit where credit is due.
//  I'm providing this code in the hope that it is useful to someone, as i have
// gotten much use out of other peoples code over the years.
//  If you see value in it, make some improvements, etc., i would appreciate it 
// if you sent me some feedback.
//
 
#include "stdafx.h"
#include "BalloonHelp.h" 
#include <atlbase.h>   

// debug helpers
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
// constants that may not be defined if you don't have the latest SDK
// (but i like to use them anyway)
//

#ifndef DFCS_HOT
#define DFCS_HOT 0x1000
#endif

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED   0x00080000
#define LWA_COLORKEY    0x00000001
#define LWA_ALPHA       0x00000002
#endif

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW   0x00020000
#endif

#ifndef SPI_GETDROPSHADOW
#define SPI_GETDROPSHADOW  0x1024
#endif

#ifndef SPI_GETTOOLTIPANIMATION
#define SPI_GETTOOLTIPANIMATION 0x1016
#endif

#ifndef SPI_GETTOOLTIPFADE
#define SPI_GETTOOLTIPFADE 0x1018
#endif

/////////////////////////////////////////////////////////////////////////////
// CBalloonHelp

// these could go in resources, if they make you nervous
// if someone knows a better way to deal with timers than to use
// constant IDs, pls let me know?
#define ID_TIMER_SHOW      3333
#define ID_TIMER_HIDE      3334
#define ID_TIMER_CLOSE     3335
#define ID_TIMER_HOTTRACK  3336

// option constants (bits)
const unsigned int   CBalloonHelp::unCLOSE_ON_LBUTTON_UP = 0x001;
const unsigned int   CBalloonHelp::unCLOSE_ON_RBUTTON_UP = 0x002;
const unsigned int   CBalloonHelp::unCLOSE_ON_MOUSE_MOVE = 0x004;
const unsigned int   CBalloonHelp::unCLOSE_ON_KEYPRESS   = 0x008;
const unsigned int   CBalloonHelp::unDELETE_THIS_ON_CLOSE= 0x010;
const unsigned int   CBalloonHelp::unSHOW_CLOSE_BUTTON   = 0x020;
const unsigned int   CBalloonHelp::unSHOW_INNER_SHADOW   = 0x040;
const unsigned int   CBalloonHelp::unSHOW_TOPMOST        = 0x080;
const unsigned int   CBalloonHelp::unDISABLE_FADEIN      = 0x100;
const unsigned int   CBalloonHelp::unDISABLE_FADEOUT     = 0x200;
const unsigned int   CBalloonHelp::unDISABLE_FADE        = CBalloonHelp::unDISABLE_FADEIN|CBalloonHelp::unDISABLE_FADEOUT;

// layout constants (should prolly be configurable)
const int            CBalloonHelp::nTIP_TAIL             = 20;
const int            CBalloonHelp::nTIP_MARGIN           = 8;
// class atom
ATOM                 CBalloonHelp::m_ClassAtom           = NULL;
// synchronization for keyboard hook
CCriticalSection     CBalloonHelp::s_KeyboardHookSection;
// windows using keyboard hook
CPtrArray            CBalloonHelp::s_apKeyboardCloseWnds;
// handle to the keyboard hook, if set
HHOOK                CBalloonHelp::s_hKeyboardHook       = NULL;

CBalloonHelp::CBalloonHelp()
:  m_fnSetLayeredWindowAttributes(NULL),
   m_unOptions(0),
   m_unTimeout(0),
   m_strURL(_T("")),
   m_ptAnchor(0,0),
   m_strContent(_T("")),
   m_nAlpha(255),
   m_nMouseMoveTolerance(3),
   m_ptMouseOrig(0,0),
   m_uCloseState(0),
   m_pTitleFont(NULL),
   m_pContentFont(NULL),
   m_crForeground(::GetSysColor(COLOR_INFOTEXT)),
   m_crBackground(::GetSysColor(COLOR_INFOBK))
{
   // retrieve layered window API if available
   HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
   m_fnSetLayeredWindowAttributes = (FN_SET_LAYERED_WINDOW_ATTRIBUTES)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
}

CBalloonHelp::~CBalloonHelp()
{
   if ( NULL != m_pTitleFont )
      delete m_pTitleFont;
   m_pTitleFont = NULL;
   if ( NULL != m_pContentFont )
      delete m_pContentFont;
   m_pContentFont = NULL;
}



//
// Show a help balloon on screen
// Parameters:
//    iControlID  |  ID of the control for whom this message is generated
//    strTitle    |  Title of balloon
//    strContent  |  Content of balloon
//    ptAnchor    |  point tail of balloon will be "anchor"ed to
//    szIcon      |  One of:
//                   IDI_APPLICATION
//                   IDI_INFORMATION IDI_ASTERISK (same)
//                   IDI_ERROR IDI_HAND (same)
//                   IDI_EXCLAMATION IDI_WARNING (same)
//                   IDI_QUESTION
//                   IDI_WINLOGO
//                   NULL (no icon)
//    unOptions   |  One or more of:
//                :     unCLOSE_ON_LBUTTON_UP   |  closes window on WM_LBUTTON_UP
//                :     unCLOSE_ON_RBUTTON_UP   |  closes window on WM_RBUTTON_UP
//                :     unCLOSE_ON_MOUSE_MOVE   |  closes window when user moves mouse past threshhold
//                :     unCLOSE_ON_KEYPRESS     |  closes window on the next keypress message sent to this thread.  (!!! probably not thread safe !!!)
//                :     unSHOW_CLOSE_BUTTON     |  shows close button in upper right
//                :     unSHOW_INNER_SHADOW     |  draw inner shadow in balloon
//                :     unSHOW_TOPMOST          |  place balloon above all other windows
//                :     unDISABLE_FADE          |  disable the fade-in/fade-out effects (overrides system and user settings)
//                :     unDISABLE_FADEIN        |  disable the fade-in effect
//                :     unDISABLE_FADEOUT       |  disable the fade-out effect
//    pParentWnd  |  Parent window.  If NULL will be set to AfxGetMainWnd()
//    strURL      |  If not empty, when the balloon is clicked ShellExecute() will
//                |  be called, with strURL passed in.
//    unTimeout   |  If not 0, balloon will automatically close after unTimeout milliseconds.
//
void CBalloonHelp::LaunchBalloon(int iControlID, const CString& strTitle, const CString& strContent, 
               const CPoint& ptAnchor, 
               LPCTSTR szIcon /*= IDI_EXCLAMATION*/,
               unsigned int unOptions /*= unSHOW_CLOSE_BUTTON*/,
               CWnd* pParentWnd /*= NULL*/,
               const CString strURL /*= ""*/,
               unsigned int unTimeout /*= 10000*/)
{
   CBalloonHelp* pbh = new CBalloonHelp;
   if ( NULL != szIcon )
   {
      HICON hIcon = (HICON)::LoadImage(NULL, szIcon, IMAGE_ICON, 16,16, LR_SHARED);
      if (NULL != hIcon)
      {
         // Use a scaled standard icon (looks very good on Win2k, XP, not so good on Win9x)
         CDC dc;
         CDC dcTmp1;
         CDC dcTmp2;
         CBitmap bmpIcon;
         CBitmap bmpIconSmall;
         dc.Attach(::GetDC(NULL));
         dcTmp1.CreateCompatibleDC(&dc);
         dcTmp2.CreateCompatibleDC(&dc);
         bmpIcon.CreateCompatibleBitmap(&dc, 32,32);
         bmpIconSmall.CreateCompatibleBitmap(&dc, 16,16);
         ::ReleaseDC(NULL, dc.Detach());
         
         // i now have two device contexts and two bitmaps.
         // i will select a bitmap in each device context,
         // draw the icon into the larger one,
         // scale it into the smaller one,
         // and set the small one as the balloon icon.
         // This is a rather long process to get a small icon,
         // but ensures maximum compatibility between different
         // versions of Windows, while producing the best possible
         // results on each version.
         CBitmap* pbmpOld1 = dcTmp1.SelectObject(&bmpIcon);
         CBitmap* pbmpOld2 = dcTmp2.SelectObject(&bmpIconSmall);
         dcTmp1.FillSolidRect(0,0,32,32, pbh->m_crBackground);
         ::DrawIconEx(dcTmp1, 0,0,hIcon,32,32,0,NULL,DI_NORMAL);
         dcTmp2.SetStretchBltMode(HALFTONE);
         dcTmp2.StretchBlt(0,0,16,16,&dcTmp1, 0,0,32,32,SRCCOPY);
         dcTmp1.SelectObject(pbmpOld1);
         dcTmp2.SelectObject(pbmpOld2);

         pbh->SetIcon(bmpIconSmall, pbh->m_crBackground);
      }
   }

   if (!pbh->Create(iControlID, strTitle, strContent, ptAnchor, unOptions|unDELETE_THIS_ON_CLOSE, 
               pParentWnd, strURL, unTimeout, NULL))
   {
     ::MessageBox(::GetActiveWindow(), strContent, strTitle, MB_OK | MB_ICONINFORMATION);
   }
}


// Sets the font used for drawing the balloon title.  Deleted by balloon, do not use CFont* after passing to this function.
void CBalloonHelp::SetTitleFont(CFont* pFont)
{
   if ( NULL != m_pTitleFont )
      delete m_pTitleFont;
   m_pTitleFont = pFont;
   // if already visible, resize & move
   if ( NULL != m_hWnd )
      PositionWindow();
}

// Sets the font used for drawing the balloon content.  Deleted by balloon, do not use CFont* after passing to this function.
void CBalloonHelp::SetContentFont(CFont* pFont)
{
   if ( NULL != m_pContentFont )
      delete m_pContentFont;
   m_pContentFont = pFont;
   // if already visible, resize & move
   if ( NULL != m_hWnd )
      PositionWindow();
}

// Sets the icon displayed in the top left of the balloon (pass NULL to hide icon)
void CBalloonHelp::SetIcon(HICON hIcon)
{
   if ( NULL != m_ilIcon.m_hImageList )
      m_ilIcon.DeleteImageList();
   ICONINFO iconinfo;
   if ( NULL != hIcon && ::GetIconInfo(hIcon, &iconinfo) )
   {
      SetIcon(iconinfo.hbmColor, iconinfo.hbmMask);
      ::DeleteObject(iconinfo.hbmColor);
      ::DeleteObject(iconinfo.hbmMask);
   }
   // if already visible, resize & move (icon size may have changed)
   if ( NULL != m_hWnd )
      PositionWindow();
}

// Sets the icon displayed in the top left of the balloon (pass NULL hBitmap to hide icon)
void CBalloonHelp::SetIcon(HBITMAP hBitmap, COLORREF crMask)
{
   if ( NULL != m_ilIcon.m_hImageList )
      m_ilIcon.DeleteImageList();

   if ( NULL != hBitmap )
   {
      BITMAP bm;
      if (::GetObject(hBitmap, sizeof(bm),(LPVOID)&bm))
      {
         m_ilIcon.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR24|ILC_MASK,1,0);
         m_ilIcon.Add(CBitmap::FromHandle(hBitmap), crMask);
      }
   }
   // if already visible, resize & move (icon size may have changed)
   if ( NULL != m_hWnd )
      PositionWindow();
}

// Sets the icon displayed in the top left of the balloon
void CBalloonHelp::SetIcon(HBITMAP hBitmap, HBITMAP hMask)
{
   if ( NULL != m_ilIcon.m_hImageList )
      m_ilIcon.DeleteImageList();

   ASSERT(NULL != hBitmap);
   ASSERT(NULL != hMask);

   BITMAP bm;
   if (::GetObject(hBitmap, sizeof(bm),(LPVOID)&bm))
   {
      m_ilIcon.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR24|ILC_MASK,1,0);
      m_ilIcon.Add(CBitmap::FromHandle(hBitmap), CBitmap::FromHandle(hMask));
   }
   // if already visible, resize & move (icon size may have changed)
   if ( NULL != m_hWnd )
      PositionWindow();
}

// Set icon displayed in the top left of the balloon to image # nIconIndex from pImageList
void CBalloonHelp::SetIcon(CImageList* pImageList, int nIconIndex)
{
   // sanity checks
   ASSERT_VALID(pImageList);
   ASSERT(nIconIndex >= 0 && nIconIndex < pImageList->GetImageCount() );

   HICON hIcon = NULL;
   if ( NULL != pImageList && nIconIndex >= 0 && nIconIndex < pImageList->GetImageCount() )
      hIcon = pImageList->ExtractIcon(nIconIndex);
   SetIcon(hIcon);
   if ( NULL != hIcon )
      ::DestroyIcon(hIcon);
   // if already visible, resize & move (icon size may have changed)
   if ( NULL != m_hWnd )
      PositionWindow();
}

// Sets the URL to be opened when balloon is clicked.  Pass "" to disable.
void CBalloonHelp::SetURL(const CString& strURL)
{
   m_strURL = strURL;
}

// Sets the number of milliseconds the balloon can remain open.  Set to 0 to disable timeout.
void CBalloonHelp::SetTimeout(unsigned int unTimeout)
{
   m_unTimeout = unTimeout;
   // if timer is already set, reset.
   if ( NULL != m_hWnd )
   {
      if ( m_unTimeout > 0 )
         SetTimer(ID_TIMER_CLOSE, m_unTimeout, NULL);
      else
         KillTimer(ID_TIMER_CLOSE);
   }
}

// Sets the point to which the balloon is "anchored"
void CBalloonHelp::SetAnchorPoint(CPoint ptAnchor)
{
   m_ptAnchor = ptAnchor;

   // if already visible, move
   if ( NULL != m_hWnd )
   {
      PositionWindow();
   }
}

// Sets the title of the balloon
void CBalloonHelp::SetTitle(const CString& strTitle)
{
   SetWindowText(strTitle);
   // if already visible, resize & move
   if ( NULL != m_hWnd )
      PositionWindow();
}

// Sets the content of the balloon (plain text only)
void CBalloonHelp::SetContent(const CString& strContent)
{
   m_strContent = strContent;
   // if already visible, resize & move
   if ( NULL != m_hWnd )
      PositionWindow();
}

// Sets the forground (text and border) color of the balloon
void CBalloonHelp::SetForegroundColor(COLORREF crForeground)
{
   m_crForeground = crForeground;
   // repaint if visible
   if ( NULL != m_hWnd )
      Invalidate(FALSE);
}

// Sets the background color of the balloon
void CBalloonHelp::SetBackgroundColor(COLORREF crBackground)
{
   m_crBackground = crBackground;
   // repaint if visible
   if ( NULL != m_hWnd )
      Invalidate(FALSE);
}

// Sets the distance the mouse must move before the balloon closes when the unCLOSE_ON_MOUSE_MOVE option is set.
void CBalloonHelp::SetMouseMoveTolerance(int nTolerance)
{
   m_nMouseMoveTolerance = nTolerance;
}

//
// creates a new balloon window
// Parameters:
//    iControlID  |  ID of the control for whom this message is generated
//    strTitle    |  Title of balloon
//    strContent  |  Content of balloon
//    ptAnchor    |  point tail of balloon will be "anchor"ed to
//    unOptions   |  One or more of:
//                :     unCLOSE_ON_LBUTTON_UP   |  closes window on WM_LBUTTON_UP
//                :     unCLOSE_ON_RBUTTON_UP   |  closes window on WM_RBUTTON_UP
//                :     unCLOSE_ON_MOUSE_MOVE   |  closes window when user moves mouse past threshhold
//                :     unCLOSE_ON_KEYPRESS     |  closes window on the next keypress message sent to this thread.    (!!! probably not thread safe !!!)
//                :     unDELETE_THIS_ON_CLOSE  |  deletes object when window is closed.  Used by LaunchBalloon(), use with care
//                :     unSHOW_CLOSE_BUTTON     |  shows close button in upper right
//                :     unSHOW_INNER_SHADOW     |  draw inner shadow in balloon
//                :     unSHOW_TOPMOST          |  place balloon above all other windows
//                :     unDISABLE_FADE          |  disable the fade-in/fade-out effects (overrides system and user settings)
//                :     unDISABLE_FADEIN        |  disable the fade-in effect
//                :     unDISABLE_FADEOUT       |  disable the fade-out effect
//    pParentWnd  |  Parent window.  If NULL will be set to AfxGetMainWnd()
//    strURL      |  If not empty, when the balloon is clicked ShellExecute() will
//                |  be called, with strURL passed in.
//    unTimeout   |  If not 0, balloon will automatically close after unTimeout milliseconds.
//    hIcon       |  If not NULL, the icon indicated by hIcon will be displayed at top-left of the balloon.
//
// Returns:
//    TRUE if successful, else FALSE
//
BOOL CBalloonHelp::Create(int iControlID, const CString& strTitle, const CString& strContent, 
               const CPoint& ptAnchor, unsigned int unOptions,
               CWnd* pParentWnd /*=NULL*/,
               const CString strURL /*= ""*/,
               unsigned int unTimeout /*= 0*/,
               HICON hIcon /*= NULL*/)
{
   m_iControlID   = iControlID;
   m_strContent   = strContent;
   m_ptAnchor     = ptAnchor;
   m_unOptions    = unOptions;
   m_strURL       = strURL;
   m_unTimeout    = unTimeout;

   if ( NULL != hIcon )
      SetIcon(hIcon);
   if (NULL == pParentWnd)
      pParentWnd = AfxGetMainWnd();
   if (NULL == pParentWnd)
      return FALSE;
   // if no fonts set, use defaults
   if ( NULL == m_pContentFont )
   {
      m_pContentFont = new CFont;
      if ( !m_pContentFont->CreateStockObject(DEFAULT_GUI_FONT) )
         return FALSE;
   }
   // title font defaults to bold version of content font
   if ( NULL == m_pTitleFont )
   {
      m_pTitleFont = new CFont;
      LOGFONT LogFont;
      m_pContentFont->GetLogFont(&LogFont);
      LogFont.lfWeight = FW_BOLD;
      if ( !m_pTitleFont->CreateFontIndirect(&LogFont) )
         return FALSE;
   }

   // This part of the code was modified by SSR, 07.02.02, to enable unloading and re-loading of ARX modules
   //if ( !GetClassAtom() ) return FALSE; // couldn't register class
   GetClassAtom();
   // End modification
      
   // check system settings: if fade effects are disabled, disable here too
   BOOL bFade = FALSE;
   ::SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &bFade, 0);
   if (bFade)
      ::SystemParametersInfo(SPI_GETTOOLTIPFADE, 0, &bFade, 0);
   if (!bFade || NULL == m_fnSetLayeredWindowAttributes)
      m_unOptions |= unDISABLE_FADE;

   // create invisible at arbitrary position; then position, set region, and finally show

   // the idea with WS_EX_TOOLWINDOW is, you can't switch to this using alt+tab
   DWORD dwExStyle = WS_EX_TOOLWINDOW|WS_EX_LAYERED;
   if ( m_unOptions&unSHOW_TOPMOST )      // make topmost, if requested
      dwExStyle |= WS_EX_TOPMOST;
   if ( !CreateEx(dwExStyle, _T("BalloonHelpClass"), strTitle, WS_POPUP, CRect(0,0,10,10), pParentWnd, 0, NULL) )
      return FALSE;
   PositionWindow();

   // show, possibly fading
   ShowBalloon();

   if ( m_unOptions & unCLOSE_ON_MOUSE_MOVE )
   {
      ::GetCursorPos(&m_ptMouseOrig);
      ScreenToClient(&m_ptMouseOrig);
      SetTimer(ID_TIMER_HOTTRACK, 100, NULL);
   }

   if ( (m_unOptions & unCLOSE_ON_LBUTTON_UP) || (m_unOptions & unCLOSE_ON_RBUTTON_UP) )
      SetCapture();

   if ( m_unOptions&unCLOSE_ON_KEYPRESS )
      SetKeyboardHook();


   // Code modified by SSR, 07.02.02, so that the parent dialog is no longer active
   // show, but don't take focus away from parent (or whatever)
   ShowWindow(SW_SHOWNOACTIVATE);
   //ShowWindow(SW_SHOWNORMAL);
   //GetParent()->EnableWindow(FALSE);
   // End modification

   return TRUE;
}

// Wrapper for possibly-existing API call
BOOL CBalloonHelp::SetLayeredWindowAttributes(COLORREF crKey, int nAlpha, DWORD dwFlags)
{
   if ( NULL != m_fnSetLayeredWindowAttributes )
      return (*m_fnSetLayeredWindowAttributes)(m_hWnd, crKey, (BYTE)nAlpha, dwFlags);
   return FALSE;
}

// calculates the area of the screen the balloon falls into
// this determins which direction the tail points
CBalloonHelp::BALLOON_QUADRANT CBalloonHelp::GetBalloonQuadrant()
{
   CRect rectDesktop;
   ::GetWindowRect(::GetDesktopWindow(), &rectDesktop);
   
   if ( m_ptAnchor.y < rectDesktop.Height()/2 )
   {
      if ( m_ptAnchor.x < rectDesktop.Width()/2 )
      {
         return BQ_TOPLEFT;
      }
      else
      {
         return BQ_TOPRIGHT;
      }
   }
   else
   {
      if ( m_ptAnchor.x < rectDesktop.Width()/2 )
      {
         return BQ_BOTTOMLEFT;
      }
      else
      {
         return BQ_BOTTOMRIGHT;
      }
   }

   // unreachable
}

// Calculate the dimensions and draw the balloon header
CSize CBalloonHelp::DrawHeader(CDC* pDC, bool bDraw)
{
   CSize sizeHdr(0,0);
   CRect rectClient;
   GetClientRect(&rectClient);   // use this for positioning when drawing
                                 // else if content is wider than title, centering wouldn't work

   // calc & draw icon
   if ( NULL != m_ilIcon.m_hImageList )
   {
      int x = 0;
      int y = 0;
      ImageList_GetIconSize(m_ilIcon, &x, &y);
      sizeHdr.cx += x;
      sizeHdr.cy = max(sizeHdr.cy, y);
      m_ilIcon.SetBkColor(m_crBackground);
      if (bDraw)
         m_ilIcon.Draw(pDC, 0, CPoint(0,0), ILD_NORMAL);//ILD_TRANSPARENT);
      rectClient.left += x;
   }

   // calc & draw close button
   if ( m_unOptions & unSHOW_CLOSE_BUTTON )
   {
      // if something is already in the header (icon) leave space
      if ( sizeHdr.cx > 0 )
         sizeHdr.cx += nTIP_MARGIN;
      sizeHdr.cx += 16;
      sizeHdr.cy = max(sizeHdr.cy, 16);
      if (bDraw)
         pDC->DrawFrameControl(CRect(rectClient.right-16,0,rectClient.right,16), DFC_CAPTION, DFCS_CAPTIONCLOSE|DFCS_FLAT);
      rectClient.right -= 16;
   }

   // calc title size
   CString strTitle;
   GetWindowText(strTitle);
   if ( !strTitle.IsEmpty() )
   {
      CFont* pOldFont = pDC->SelectObject(m_pTitleFont);

      // if something is already in the header (icon or close button) leave space
      if ( sizeHdr.cx > 0 ) 
         sizeHdr.cx += nTIP_MARGIN;
      CRect rectTitle(0,0,0,0);
      pDC->DrawText(strTitle, &rectTitle, DT_CALCRECT | DT_NOPREFIX | DT_EXPANDTABS | DT_SINGLELINE);
      sizeHdr.cx += rectTitle.Width();
      sizeHdr.cy = max(sizeHdr.cy, rectTitle.Height());

      // draw title
      if ( bDraw )
      {
         pDC->SetBkMode(TRANSPARENT);
         pDC->SetTextColor(m_crForeground);
         pDC->DrawText(strTitle, &rectClient, DT_CENTER | DT_NOPREFIX  | DT_EXPANDTABS | DT_SINGLELINE);
      }

      // cleanup
      pDC->SelectObject(pOldFont);
   }

   return sizeHdr;
}

// Calculate the dimensions and draw the balloon contents
CSize CBalloonHelp::DrawContent(CDC* pDC, int nTop, bool bDraw)
{
   CRect rectContent;
   ::GetClientRect(::GetDesktopWindow(), &rectContent);

   rectContent.top = nTop;

   // limit to half screen width
   rectContent.right -= rectContent.Width()/2;

   // calc size
   CFont* pOldFont = pDC->SelectObject(m_pContentFont);
   if ( !m_strContent.IsEmpty() )
      pDC->DrawText(m_strContent, &rectContent, DT_CALCRECT | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS | DT_WORDBREAK);
   else
      rectContent.SetRectEmpty();   // don't want to leave half the screen for empty strings ;)
   
   // draw
   if (bDraw)
   {
      pDC->SetBkMode(TRANSPARENT);
      pDC->SetTextColor(m_crForeground);
      pDC->DrawText(m_strContent, &rectContent, DT_LEFT | DT_NOPREFIX | DT_WORDBREAK | DT_EXPANDTABS);
   }

   // cleanup
   pDC->SelectObject(pOldFont);

   return rectContent.Size();
}

// calculates the client size necessary based on title and content
CSize CBalloonHelp::CalcClientSize()
{
   ASSERT(NULL != m_hWnd);
   CWindowDC dc(this);

   CSize sizeHeader = CalcHeaderSize(&dc);
   CSize sizeContent = CalcContentSize(&dc);

   return CSize(max(sizeHeader.cx,sizeContent.cx), sizeHeader.cy + nTIP_MARGIN + sizeContent.cy);
}

// calculates the size for the entire window based on content size
CSize CBalloonHelp::CalcWindowSize()
{
   CSize size = CalcClientSize();
   size.cx += nTIP_MARGIN*2;
   size.cy += nTIP_TAIL+nTIP_MARGIN*2;
   //size.cx = max(size.cx, nTIP_MARGIN*2+nTIP_TAIL*4);
   return size;
}


// this routine calculates the size and position of the window relative
// to it's anchor point, and moves the window accordingly.  The region is also
// created and set here.
void CBalloonHelp::PositionWindow()
{
   CSize sizeWnd = CalcWindowSize();

   CPoint ptTail[3];
   CPoint ptTopLeft(0,0);
   CPoint ptBottomRight(sizeWnd.cx, sizeWnd.cy);

   switch (GetBalloonQuadrant())
   {
   case BQ_TOPLEFT:
      ptTopLeft.y = nTIP_TAIL;
      ptTail[0].x = (sizeWnd.cx-nTIP_TAIL)/4 + nTIP_TAIL;
      ptTail[0].y = nTIP_TAIL+1;
      ptTail[2].x = (sizeWnd.cx-nTIP_TAIL)/4;
      ptTail[2].y = ptTail[0].y;
      ptTail[1].x = ptTail[2].x;
      ptTail[1].y = 1;
      break;
   case BQ_TOPRIGHT:
      ptTopLeft.y = nTIP_TAIL;
      ptTail[0].x = (sizeWnd.cx-nTIP_TAIL)/4*3;
      ptTail[0].y = nTIP_TAIL+1;
      ptTail[2].x = (sizeWnd.cx-nTIP_TAIL)/4*3 + nTIP_TAIL;
      ptTail[2].y = ptTail[0].y;
      ptTail[1].x = ptTail[2].x;
      ptTail[1].y = 1;
      break;
   case BQ_BOTTOMLEFT:
      ptBottomRight.y = sizeWnd.cy-nTIP_TAIL;
      ptTail[0].x = (sizeWnd.cx-nTIP_TAIL)/4 + nTIP_TAIL;
      ptTail[0].y = sizeWnd.cy-nTIP_TAIL-2;
      ptTail[2].x = (sizeWnd.cx-nTIP_TAIL)/4;
      ptTail[2].y = ptTail[0].y;
      ptTail[1].x = ptTail[2].x;
      ptTail[1].y = sizeWnd.cy-2;
      break;
   case BQ_BOTTOMRIGHT:
      ptBottomRight.y = sizeWnd.cy-nTIP_TAIL;
      ptTail[0].x = (sizeWnd.cx-nTIP_TAIL)/4*3;
      ptTail[0].y = sizeWnd.cy-nTIP_TAIL-2;
      ptTail[2].x = (sizeWnd.cx-nTIP_TAIL)/4*3 + nTIP_TAIL;
      ptTail[2].y = ptTail[0].y;
      ptTail[1].x = ptTail[2].x;
      ptTail[1].y = sizeWnd.cy-2;
      break;
   }

   // adjust for very narrow balloons
   if ( ptTail[0].x < nTIP_MARGIN )
      ptTail[0].x = nTIP_MARGIN;
   if ( ptTail[0].x > sizeWnd.cx - nTIP_MARGIN )
      ptTail[0].x = sizeWnd.cx - nTIP_MARGIN;
   if ( ptTail[1].x < nTIP_MARGIN )
      ptTail[1].x = nTIP_MARGIN;
   if ( ptTail[1].x > sizeWnd.cx - nTIP_MARGIN )
      ptTail[1].x = sizeWnd.cx - nTIP_MARGIN;
   if ( ptTail[2].x < nTIP_MARGIN )
      ptTail[2].x = nTIP_MARGIN;
   if ( ptTail[2].x > sizeWnd.cx - nTIP_MARGIN )
      ptTail[2].x = sizeWnd.cx - nTIP_MARGIN;

   // get window position
   CPoint ptOffs(m_ptAnchor.x - ptTail[1].x, m_ptAnchor.y - ptTail[1].y);

   // adjust position so all is visible
   CRect rectScreen;
   ::GetWindowRect(::GetDesktopWindow(), &rectScreen);
   int nAdjustX = 0;
   int nAdjustY = 0;
   if ( ptOffs.x < 0 )
      nAdjustX = -ptOffs.x;
   else if ( ptOffs.x + sizeWnd.cx >= rectScreen.right )
      nAdjustX = rectScreen.right - (ptOffs.x + sizeWnd.cx);
   if ( ptOffs.y < 0 )
      nAdjustY = -ptOffs.y;
   else if ( ptOffs.y + sizeWnd.cy >= rectScreen.bottom )
      nAdjustY = rectScreen.bottom - (ptOffs.y + sizeWnd.cy);

   // reposition tail
   // uncomment two commented lines below to move entire tail 
   // instead of just anchor point

   //ptTail[0].x -= nAdjustX;
   ptTail[1].x -= nAdjustX;
   //ptTail[2].x -= nAdjustX;
   ptOffs.x    += nAdjustX;
   ptOffs.y    += nAdjustY;

   // place window
   MoveWindow(ptOffs.x, ptOffs.y, sizeWnd.cx, sizeWnd.cy, TRUE);

   // apply region
   BOOL bRegionChanged = TRUE;
   CRgn region;
   CRgn regionRound;
   CRgn regionComplete;
   region.CreatePolygonRgn(&ptTail[0], 3, ALTERNATE);
   regionRound.CreateRoundRectRgn(ptTopLeft.x,ptTopLeft.y,ptBottomRight.x,ptBottomRight.y,nTIP_MARGIN*3,nTIP_MARGIN*3);
   regionComplete.CreateRectRgn(0,0,1,1);
   regionComplete.CombineRgn(&region, &regionRound, RGN_OR);
   if ( NULL == m_rgnComplete.m_hObject )
      m_rgnComplete.CreateRectRgn(0,0,1,1);
   else if ( m_rgnComplete.EqualRgn(&regionComplete) )
      bRegionChanged = FALSE;
   m_rgnComplete.CopyRgn(&regionComplete);
   
   SetWindowRgn((HRGN)regionComplete.Detach(), TRUE);

   // There is a bug with layered windows and NC changes in Win2k
   // As a workaround, redraw the entire window if the NC area changed.
   // Changing the anchor point is the ONLY thing that will change the
   // position of the client area relative to the window during normal
   // operation.
   if ( bRegionChanged )
      RedrawWindow(NULL, NULL, RDW_UPDATENOW| RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
}



// Returns the class ATOM for a BalloonHelp control.  Registers the class first, if necessary.
ATOM CBalloonHelp::GetClassAtom()
{
   if ( NULL == m_ClassAtom )
   {
      WNDCLASSEX wcx; 

      // Fill in the window class structure with parameters 
      // that describe the main window. 

   	// So AfxGetInstanceHandle() can work - ATL specific
      // Courtesy third
      if ( NULL == afxCurrentInstanceHandle && NULL != _pModule )
      {
         afxCurrentInstanceHandle = _pModule->GetModuleInstance();
      }

      wcx.cbSize = sizeof(wcx);                 // size of structure 
      wcx.style = CS_DBLCLKS|CS_SAVEBITS
         |CS_DROPSHADOW;                        // notify of double clicks, save screen under, show dropshadow
      wcx.lpfnWndProc = AfxWndProc;             // points to window procedure 
      wcx.cbClsExtra = 0;                       // no extra class memory 
      wcx.cbWndExtra = 0;                       // no extra window memory 
      wcx.hInstance = AfxGetInstanceHandle();   // handle to instance 
      wcx.hIcon = NULL;                         // no app. icon 
      wcx.hCursor = LoadCursor(NULL,IDC_ARROW); // predefined arrow 
      wcx.hbrBackground = ::GetSysColorBrush(COLOR_WINDOW);                 // no background brush 
      wcx.lpszMenuName =  NULL;                 // no menu resource 
      wcx.lpszClassName = _T("BalloonHelpClass");   // name of window class 
      wcx.hIconSm = NULL;                       // no small class icon

      // Register the window class. 
      m_ClassAtom = RegisterClassEx(&wcx);
      
      // didn't work? try not using dropshadow (may not be supported)
      if ( NULL == m_ClassAtom )
      {
         wcx.style &= ~CS_DROPSHADOW;
         m_ClassAtom = RegisterClassEx(&wcx);
      }
   }

   return m_ClassAtom;
}


// Displays the balloon on the screen, performing fade-in if enabled.
void CBalloonHelp::ShowBalloon(void)
{
   m_nAlpha = 5;
   SetLayeredWindowAttributes(0, m_nAlpha, LWA_ALPHA);
   SetTimer(ID_TIMER_SHOW, 25, NULL);
}

// Removes the balloon from the screen, performing the fade-out if enabled
void CBalloonHelp::HideBalloon(void)
{
   m_nAlpha = 250;
   SetTimer(ID_TIMER_HIDE, 25, NULL);
}

// Sets up the keyboard hook; adds this to list to close
void CBalloonHelp::SetKeyboardHook()
{
   CSingleLock singleLock(&s_KeyboardHookSection);
   singleLock.Lock();
   // if hook is not yet set, set
   if ( NULL == s_hKeyboardHook )
   {
      s_hKeyboardHook = ::SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardProc, NULL, ::GetCurrentThreadId());
   }
   s_apKeyboardCloseWnds.Add(m_hWnd);
}


// Removes this from the list of windows to close; removes hook if not in use.
void CBalloonHelp::RemoveKeyboardHook()
{
   CSingleLock singleLock(&s_KeyboardHookSection);
   singleLock.Lock();
   INT_PTR i;
   for (i=0; i<s_apKeyboardCloseWnds.GetSize(); ++i)
   {
      if ( s_apKeyboardCloseWnds.GetAt(i) == m_hWnd )
         s_apKeyboardCloseWnds.RemoveAt(i--);
   }

   // if no more windows need to be notified, get rid of hook
   if ( s_apKeyboardCloseWnds.GetSize() == 0 && NULL != s_hKeyboardHook )
   {
      ::UnhookWindowsHookEx(s_hKeyboardHook);
      s_hKeyboardHook = NULL;
   }
}

/////////////////////////////////////////////////////////////////////////////
// CBalloonHelp message handlers

BEGIN_MESSAGE_MAP(CBalloonHelp, CWnd)
   ON_WM_ERASEBKGND()
   ON_WM_PAINT()
   ON_WM_NCPAINT()
   ON_WM_LBUTTONDOWN()
   ON_WM_LBUTTONUP()
   ON_WM_NCCALCSIZE()
   ON_WM_TIMER()
   ON_WM_MOUSEMOVE()
   ON_WM_DESTROY()
   ON_WM_RBUTTONUP()
   ON_WM_CLOSE()
END_MESSAGE_MAP()

// Erase client area of balloon
BOOL CBalloonHelp::OnEraseBkgnd(CDC* pDC) 
{
   CRect rect;
   GetClientRect(&rect);
   pDC->FillSolidRect(&rect, m_crBackground);
   return TRUE;
}

// draw balloon client area (title & contents)
void CBalloonHelp::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
   CSize sizeHeader = DrawHeader(&dc);
   DrawContent(&dc, sizeHeader.cy+nTIP_MARGIN);
}

// draw balloon shape & boarder
void CBalloonHelp::OnNcPaint() 
{
   CWindowDC dc(this);
   CRect rect;
   GetWindowRect(&rect);
   ScreenToClient(&rect);
   CRect rectClient;
   GetClientRect(&rectClient);
   rectClient.OffsetRect(-rect.left, -rect.top);
   rect.OffsetRect(-rect.left, -rect.top);
   dc.ExcludeClipRect(&rectClient);
   dc.FillSolidRect(&rect, m_crBackground);

   ASSERT(NULL != m_rgnComplete.m_hObject);
   CBrush   brushFg;
   brushFg.CreateSolidBrush(m_crForeground);
   if ( m_unOptions & unSHOW_INNER_SHADOW )
   {
      CBrush   brushHL;
      // slightly lighter color
      int red = 170 + GetRValue(m_crBackground)/3;
      int green = 170 + GetGValue(m_crBackground)/3;
      int blue = 170 + GetBValue(m_crBackground)/3;
      brushHL.CreateSolidBrush(RGB(red,green,blue));
      m_rgnComplete.OffsetRgn(1,1);
      dc.FrameRgn(&m_rgnComplete, &brushHL, 2, 2);
      // slightly darker color
      red = GetRValue(m_crForeground)/3 + GetRValue(m_crBackground)/3*2;
      green = GetGValue(m_crForeground)/3 + GetGValue(m_crBackground)/3*2;
      blue = GetBValue(m_crForeground)/3 + GetBValue(m_crBackground)/3*2;
      brushHL.DeleteObject();
      m_rgnComplete.OffsetRgn(-2,-2);
      brushHL.CreateSolidBrush(RGB(red,green,blue));
      dc.FrameRgn(&m_rgnComplete, &brushHL, 2, 2);
      m_rgnComplete.OffsetRgn(1,1);
   }
   // outline
   dc.FrameRgn(&m_rgnComplete, &brushFg, 1, 1);
}

// Close button handler
void CBalloonHelp::OnLButtonDown(UINT, CPoint point) 
{
   if (m_unOptions & unSHOW_CLOSE_BUTTON)
   {
      CRect rect;
      GetClientRect(&rect);
      rect.left = rect.right-16;
      rect.bottom = rect.top+16;
      if ( rect.PtInRect(point) )
      {
         m_uCloseState |= DFCS_PUSHED;
         SetCapture();
         OnMouseMove(0, point);
      }
   }
}

// Close button handler,
// close on LButton up handler
void CBalloonHelp::OnLButtonUp(UINT, CPoint point) 
{
   if ( (m_unOptions & unSHOW_CLOSE_BUTTON) && (m_uCloseState & DFCS_PUSHED) )
   {
      ReleaseCapture();
      m_uCloseState &= ~DFCS_PUSHED;
      CRect rect;
      GetClientRect(&rect);
      rect.left = rect.right-16;
      rect.bottom = rect.top+16;
      if ( rect.PtInRect(point) )
         HideBalloon();
   }
   else if ( m_unOptions & unCLOSE_ON_LBUTTON_UP )
   {
      ReleaseCapture();
      HideBalloon();
   }
   else if ( !m_strURL.IsEmpty() )
   {
      HideBalloon();
      ::ShellExecute(NULL, NULL, m_strURL, NULL, NULL, SW_SHOWNORMAL);
   }
}

// Close on RButton up checking
void CBalloonHelp::OnRButtonUp(UINT, CPoint)
{
   if ( m_unOptions & unCLOSE_ON_RBUTTON_UP )
   {
      ReleaseCapture();
      HideBalloon();
   }
}

//
// do mouse tracking:
//   Close on mouse move;
//   Tracking for close button;
//
void CBalloonHelp::OnMouseMove(UINT, CPoint point)
{
   if (m_unOptions & unSHOW_CLOSE_BUTTON)
   {
      CRect rect;
      GetClientRect(&rect);
      rect.left = rect.right-16;
      rect.bottom = rect.top+16;
      CClientDC dc(this);
      UINT uState = DFCS_CAPTIONCLOSE;
      BOOL bPushed = m_uCloseState&DFCS_PUSHED;
      m_uCloseState &= ~DFCS_PUSHED;
      if ( rect.PtInRect(point) )
      {
         uState |= DFCS_HOT;
         if ( bPushed )
            uState |= DFCS_PUSHED;
         SetTimer(ID_TIMER_HOTTRACK, 500, NULL);
      }
      else
      {
         uState |= DFCS_FLAT;
      }
      if ( uState != m_uCloseState )
      {
         dc.DrawFrameControl(&rect, DFC_CAPTION, uState);
         m_uCloseState = uState;
      }
      if ( bPushed )
         m_uCloseState |= DFCS_PUSHED;
   }

   if ( m_unOptions & unCLOSE_ON_MOUSE_MOVE )
   {
      if ( m_nAlpha == 255 && (abs(point.x-m_ptMouseOrig.x) > m_nMouseMoveTolerance || abs(point.y-m_ptMouseOrig.y) > m_nMouseMoveTolerance) )
         HideBalloon();
      else
         SetTimer(ID_TIMER_HOTTRACK, 100, NULL);
   }
}

// Ensures client area is the correct size relative to window size,
// presearves client contents if possible when moving.
void CBalloonHelp::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
   // nTIP_MARGIN pixel margin on all sides
   ::InflateRect(&lpncsp->rgrc[0], -nTIP_MARGIN,-nTIP_MARGIN);

   // nTIP_TAIL pixel "tail" on side closest to anchor
   switch ( GetBalloonQuadrant() )
   {
   case BQ_TOPRIGHT:
   case BQ_TOPLEFT:
      lpncsp->rgrc[0].top += nTIP_TAIL;
      break;
   case BQ_BOTTOMRIGHT:
   case BQ_BOTTOMLEFT:
      lpncsp->rgrc[0].bottom -= nTIP_TAIL;
      break;
   }

   // sanity: ensure rect does not have negative size
   if ( lpncsp->rgrc[0].right < lpncsp->rgrc[0].left )
      lpncsp->rgrc[0].right = lpncsp->rgrc[0].left;
   if ( lpncsp->rgrc[0].bottom < lpncsp->rgrc[0].top )
      lpncsp->rgrc[0].bottom = lpncsp->rgrc[0].top;

   if ( bCalcValidRects )
   {
      // determine if client position has changed relative to the window position
      // if so, don't bother presearving anything.
      if ( !::EqualRect(&lpncsp->rgrc[0], &lpncsp->rgrc[2]) )
      {
         ::SetRectEmpty(&lpncsp->rgrc[2]);
      }
   }
}

// handler for various timers
#ifdef WIN64
void CBalloonHelp::OnTimer(UINT_PTR nIDEvent) 
#else
void CBalloonHelp::OnTimer(UINT nIDEvent) 
#endif
{
   switch (nIDEvent)
   {
   case ID_TIMER_SHOW:
      m_nAlpha += 25;
      if ( m_nAlpha > 255 || m_unOptions&unDISABLE_FADEIN)
      {
         m_nAlpha = 255;
         KillTimer(ID_TIMER_SHOW);
         SetTimeout(m_unTimeout);     // start close timer
      }
         
      SetLayeredWindowAttributes(0, m_nAlpha, LWA_ALPHA);
      break;
   case ID_TIMER_HIDE:
      // just in case...
      KillTimer(ID_TIMER_SHOW);

      m_nAlpha -= 25;
      if ( m_nAlpha < 0 || m_unOptions&unDISABLE_FADEOUT )
      {
         m_nAlpha = 0;
         KillTimer(ID_TIMER_HIDE);
         DestroyWindow();
      }
      else
      {
         SetLayeredWindowAttributes(0, m_nAlpha, LWA_ALPHA);
      }
      break;
   case ID_TIMER_CLOSE:
      KillTimer(ID_TIMER_CLOSE);
      HideBalloon();
      break;
   case ID_TIMER_HOTTRACK:
      {
         CPoint point;
         KillTimer(ID_TIMER_HOTTRACK);
         ::GetCursorPos(&point);
         ScreenToClient(&point);
         OnMouseMove(0, point);
         break;
      }
   default:
      CWnd::OnTimer(nIDEvent);
      break;
   }
}

// Called as the window is being destroyed.  Completes destruction after removing keyboard hook.
void CBalloonHelp::OnDestroy()
{
   // remove from list, if set
   RemoveKeyboardHook();

   // Code added by SJ, 07.02.02, so that the parent window is enabled
   //GetParent()->EnableWindow(TRUE);   
   //GetParent()->BringWindowToTop();
   if (m_iControlID > 0) GetParent()->GetDlgItem(m_iControlID)->SetFocus();
   else GetParent()->SetFocus();
   // End code addition

   // destroy
   CWnd::OnDestroy();
}

// close the balloon, performing any set transition effect.
void CBalloonHelp::OnClose()
{
   HideBalloon();
}

// Keyboard hook; used to implement the unCLOSE_ON_KEYPRESS option
LRESULT CALLBACK CBalloonHelp::KeyboardProc( int code, WPARAM wParam, LPARAM lParam)
{
   // Skip if the key was released or if it's a repeat
   if (code >= 0 && !(lParam & 0x80000000))
   {
      CSingleLock singleLock(&CBalloonHelp::s_KeyboardHookSection);
      singleLock.Lock();
      INT_PTR nNumWnds = CBalloonHelp::s_apKeyboardCloseWnds.GetSize();
      INT_PTR i;
      for (i=0; i<nNumWnds; ++i)
         ::PostMessage((HWND)CBalloonHelp::s_apKeyboardCloseWnds.GetAt(i), WM_CLOSE, 0, 0);
      CBalloonHelp::s_apKeyboardCloseWnds.RemoveAll();
   }
   return ::CallNextHookEx(CBalloonHelp::s_hKeyboardHook, code, wParam, lParam);
}

// Called after window has been destroyed.  Destroys the object if option is set.
void CBalloonHelp::PostNcDestroy()
{
   CWnd::PostNcDestroy();
  
   // free object if requested
   // be careful with this one :D
   if ( m_unOptions & unDELETE_THIS_ON_CLOSE )
      delete this;
}
