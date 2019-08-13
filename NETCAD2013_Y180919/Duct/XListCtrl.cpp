// XListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "XListCtrl.h"
#include "DuctsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXListCtrl

CXListCtrl::CXListCtrl()
{
	m_bCustomDraw = TRUE;
	m_sel_row = -1;
	m_sel_col = -1;
	m_nNumberOfRows = 0;
	m_nNumberOfCols = 0;
}

CXListCtrl::~CXListCtrl()
{
}

BEGIN_MESSAGE_MAP(CXListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CXListCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
  ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnKeydown)
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,custom_draw_funtion)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXListCtrl message handlers

void CXListCtrl::custom_draw_funtion(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* nmcd=(NMLVCUSTOMDRAW*)pNMHDR;
	*pResult=CDRF_DODEFAULT;
		
	int row;
	int col;

	switch (nmcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			if (m_bCustomDraw) *pResult=CDRF_NOTIFYITEMDRAW; // else CDRF_DODEFAULT which stops notification messages
			return;

		case CDDS_ITEMPREPAINT:
			*pResult=CDRF_NOTIFYSUBITEMDRAW;
			return;
		case CDDS_SUBITEM|CDDS_ITEMPREPAINT:
		{
			*pResult=0;

			row = nmcd->nmcd.dwItemSpec;
			col = nmcd->iSubItem;
			
			CString str = GetItemText(row, col);
		
			CRect rect;
			CDC* pDC=CDC::FromHandle(nmcd->nmcd.hdc);
			
			if (col > 0)
				GetSubItemRect(row, col, LVIR_BOUNDS, rect);
			else
				GetItemRect(row, &rect, LVIR_LABEL);

			/*
			//////////////////////////////////////////////////////////////////////////
			// Added by KMK for IE_TB project
			//////////////////////////////////////////////////////////////////////////
			CDuctsDlg *pParent = (CDuctsDlg *)GetParent();
			if ((col > 0) && (col <= pParent->m_iCols))
			{
				HICON hIcon = AfxGetApp()->LoadIcon(IDI_EXISTING);
				int x = rect.TopLeft().x;
				int y = rect.top + rect.Height() / 4;
				DrawIconEx(pDC->m_hDC, x, y, hIcon, 8, 8, 0, NULL, DI_NORMAL);
			}
			//////////////////////////////////////////////////////////////////////////
			*/
			
			UINT uCode=DT_LEFT;

			if (row == m_sel_row && col == m_sel_col)
			{
				COLORREF kolor=0xaa00aa;

				if (GetFocus() == this) kolor=0xffefdb;
				
				CBrush brush(kolor);
				pDC->FillRect(&rect, &brush);
			}

			rect.OffsetRect(6,0);
			pDC->DrawText(str, &rect, uCode);
					
			*pResult=CDRF_SKIPDEFAULT;
			break;
		}
	}
}

void CXListCtrl::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMITEMACTIVATE* nm=(NMITEMACTIVATE*)pNMHDR;
	
	invalidate_grid(m_sel_row,m_sel_col);
	m_sel_row=nm->iItem;
	m_sel_col=nm->iSubItem;
	invalidate_grid(m_sel_row,m_sel_col);
	
	*pResult = 0;
}

void CXListCtrl::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NMITEMACTIVATE* nm = (NMITEMACTIVATE*)pNMHDR;
	invalidate_grid(m_sel_row, m_sel_col);
	
  m_sel_row = nm->iItem;
  m_sel_col = nm->iSubItem;
  invalidate_grid(m_sel_row, m_sel_col);
		
	// KMK: if (m_sel_col > 0)
	CDuctsDlg *pParent = (CDuctsDlg *)GetParent();
	if ((m_sel_row >= 0) && (m_sel_row < GetItemCount()) && (m_sel_col > 0) && (m_sel_col <= _tstoi(pParent->m_csCols))) pParent->SpecifyConduit(m_sel_row, m_sel_col);
	
  *pResult = 0;
}

void CXListCtrl::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLVKEYDOWN* nmkd = (NMLVKEYDOWN*)pNMHDR;
	
	switch(nmkd->wVKey)
	{
		case VK_LEFT: 
			m_sel_col--;
			if (m_sel_col < 0) m_sel_col = 0;
			invalidate_grid(m_sel_row, m_sel_col + 1);
			break;
		case VK_RIGHT:
			m_sel_col++;
			if (m_sel_col > m_nNumberOfCols - 1) m_sel_col = m_nNumberOfCols - 1;
			invalidate_grid(m_sel_row, m_sel_col - 1);
			break;
		
		case VK_UP:   
			m_sel_row--;
			if (m_sel_row < 0) m_sel_row = 0;
			invalidate_grid(m_sel_row + 1, m_sel_col);
			break;
		
		case VK_DOWN: 
			m_sel_row++;
			if(m_sel_row>m_nNumberOfRows-1)
				m_sel_row=m_nNumberOfRows-1;
			invalidate_grid(m_sel_row-1,m_sel_col);
			break;
		case VK_PRIOR:
			invalidate_grid(m_sel_row,m_sel_col);
			m_sel_row=0;
			break;
		case VK_NEXT:
			invalidate_grid(m_sel_row,m_sel_col);
			m_sel_row=m_nNumberOfRows-1;
			break;
		case VK_HOME:
			invalidate_grid(m_sel_row,m_sel_col);
			m_sel_col=0;

			if(GetKeyState(VK_CONTROL)<0)
				m_sel_row=0;

			SetItemState(m_sel_row,LVIS_FOCUSED,LVIS_FOCUSED);
			*pResult = CDRF_SKIPDEFAULT;
			invalidate_grid(m_sel_row,m_sel_col);
			return;
			break;
		case VK_END:
			invalidate_grid(m_sel_row,m_sel_col);
			m_sel_col=m_nNumberOfCols-1;
			if(GetKeyState(VK_CONTROL)<0)
				m_sel_row=m_nNumberOfRows-1;

			SetItemState(m_sel_row,LVIS_FOCUSED,LVIS_FOCUSED);

			*pResult=CDRF_SKIPDEFAULT;
			invalidate_grid(m_sel_row,m_sel_col);
			return;
	}
	
	*pResult = 0;
}

void CXListCtrl::invalidate_grid(int row, int col)
{
	CRect r;

	if (col == 0)
		GetItemRect(row, &r, LVIR_LABEL);
	else
		GetSubItemRect(row,col,LVIR_BOUNDS,r);

	InvalidateRect(&r);
}
