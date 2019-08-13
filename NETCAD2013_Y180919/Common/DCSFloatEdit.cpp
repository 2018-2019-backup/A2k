// DCSFloatEdit.cpp : implementation file
//

#include "stdafx.h"

#include "DCSFloatEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDCSFloatEdit

CDCSFloatEdit::CDCSFloatEdit()
{
  m_bAllowNegative = TRUE;
}

CDCSFloatEdit::~CDCSFloatEdit()
{
}


BEGIN_MESSAGE_MAP(CDCSFloatEdit, CEdit)
	//{{AFX_MSG_MAP(CDCSFloatEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDCSFloatEdit message handlers

void CDCSFloatEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if ((( nChar > 47 ) && ( nChar < 58 ) ) || (nChar == 8) ) CEdit::OnChar(nChar, nRepCnt, nFlags); 
	else if (nChar == 46 ) 
	{
		GetWindowText(csTemp); 
		if (csTemp.Find(_T('.')) < 0) CEdit::OnChar(nChar, nRepCnt, nFlags); 
	} 
  else if ((nChar == 45) && m_bAllowNegative)
  {
    GetWindowText(csTemp); 
    if (csTemp.IsEmpty()) CEdit::OnChar(nChar, nRepCnt, nFlags); 
    else 
    {
      int iStart, iEnd;
      GetSel(iStart, iEnd);
      if (iStart == 0) CEdit::OnChar(nChar, nRepCnt, nFlags); 
    }
  }
}
