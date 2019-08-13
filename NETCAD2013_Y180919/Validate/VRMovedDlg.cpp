////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : VRBlocksDlg.cpp
// Created          : 23rd January 2008
// Created by       : S. Jaisimha
// Description      : Implementation for the "Non-std blocks" tab of the validation results dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "VRMovedDlg.h"
#include "AcceptErrorDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Zoom helper from ValidateResultsDlg.cpp
extern void ZoomToSelection(CListCtrl& lcList, int iHandleCol, int iLayerCol);
extern void AcceptError(CListCtrl& lcList, int iRow, int iHandleCol, CString csErrorType);


// CVRMovedDlg dialog
IMPLEMENT_DYNAMIC(CVRMovedDlg, CDialog)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRMovedDlg::CVRMovedDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRMovedDlg::CVRMovedDlg(CWnd* pParent /*=NULL*/)	: CDialog(CVRMovedDlg::IDD, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRMovedDlg::~CVRMovedDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRMovedDlg::~CVRMovedDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRMovedDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRMovedDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_MOVEMENT_LIST, m_lcMoved);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CVRMovedDlg, CDialog)
  ON_NOTIFY(NM_CLICK, IDC_MOVEMENT_LIST, &CVRMovedDlg::OnClickObjectsList)
  ON_BN_CLICKED(IDC_ERROR_ACCEPT,        &CVRMovedDlg::OnBnClickedErrorAccept)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRMovedDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVRMovedDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  // Initialize the list control
  m_lcMoved.InsertColumn(0, _T("Error"),         LVCFMT_LEFT,   80);
  m_lcMoved.InsertColumn(1, _T("Object"),        LVCFMT_LEFT,  150);
  m_lcMoved.InsertColumn(2, _T("Handle"),        LVCFMT_LEFT,    0);
  m_lcMoved.InsertColumn(3, _T("Layer"),         LVCFMT_LEFT,  200);
  m_lcMoved.InsertColumn(4, _T("Movement type"), LVCFMT_LEFT,  300);
  m_lcMoved.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  // Add the values from the arrays
  for (int iCtr = 0; iCtr < m_csaMovNames.GetSize(); iCtr++)
  {
    m_lcMoved.InsertItem(iCtr,     m_csaErrorNos.GetAt(iCtr));  
    m_lcMoved.SetItemText(iCtr, 1, m_csaMovNames.GetAt(iCtr));
    m_lcMoved.SetItemText(iCtr, 2, m_csaMovHandles.GetAt(iCtr));
    m_lcMoved.SetItemText(iCtr, 3, m_csaMovLayers.GetAt(iCtr));
    m_lcMoved.SetItemText(iCtr, 4, m_csaMovType.GetAt(iCtr));
  }

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRMovedDlg::OnClickObjectsList
// Description  : Called when the user clicks on an entry in the list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRMovedDlg::OnClickObjectsList(NMHDR *pNMHDR, LRESULT *pResult)
{
  // Call the zoom function
  ZoomToSelection(m_lcMoved, 2, 3);

  // Nullify the result
  *pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRMovedDlg::OnBnClickedErrorAccept
// Description  : Called when the user clicks on the "Accept" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRMovedDlg::OnBnClickedErrorAccept()
{
  // Get the selected item
  int iIndex = m_lcMoved.GetSelectionMark();
  if (iIndex == LB_ERR) return;

  // Call the AcceptError method
  AcceptError(m_lcMoved, iIndex, 2, _T("MOVED ASSET"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRMovedDlg::WriteToReport
// Description  : Called to write to the already open report file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRMovedDlg::WriteToReport(CStdioFile &cfLog)
{
  // If there is no data in the array, return now
  if (m_csaMovNames.GetSize() == 0) return;

  // Write the tab header
  CString csLine; 
  csLine.Format(_T("\n\n-----------------------------------------\nMoved assets: %d\n-----------------------------------------\n"), m_lcMoved.GetItemCount()); 
  cfLog.WriteString(csLine);

  // Output the movement of assets
  for (int iCtr = 0; iCtr < m_lcMoved.GetItemCount(); iCtr++)
  {
    csLine.Format(_T("%d. [E%s] %s on layer %s  -  %s\n"), iCtr + 1, m_lcMoved.GetItemText(iCtr, 0), m_lcMoved.GetItemText(iCtr, 1), m_lcMoved.GetItemText(iCtr, 3), m_lcMoved.GetItemText(iCtr, 4));
    cfLog.WriteString(csLine);
  }
}
