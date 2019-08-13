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
#include "VRBlocksDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Zoom helper from ValidateResultsDlg.cpp
extern void ZoomToSelection(CListCtrl& lcList, int iHandleCol, int iLayerCol);
extern void AcceptError(CListCtrl& lcList, int iRow, int iHandleCol, CString csErrorType);

// CVRBlocksDlg dialog
IMPLEMENT_DYNAMIC(CVRBlocksDlg, CDialog)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRBlocksDlg::CVRBlocksDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRBlocksDlg::CVRBlocksDlg(CWnd* pParent /*=NULL*/)	: CDialog(CVRBlocksDlg::IDD, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRBlocksDlg::~CVRBlocksDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRBlocksDlg::~CVRBlocksDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRBlocksDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRBlocksDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_BLOCKS_LIST, m_lcBlocks);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CVRBlocksDlg, CDialog)
  ON_NOTIFY(NM_CLICK, IDC_BLOCKS_LIST, &CVRBlocksDlg::OnClickObjectsList)
  ON_BN_CLICKED(IDC_ERROR_ACCEPT,      &CVRBlocksDlg::OnBnClickedErrorAccept)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRBlocksDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVRBlocksDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  // Initialize the list control
  m_lcBlocks.InsertColumn(0, _T("Error"),           LVCFMT_LEFT,  80);
  m_lcBlocks.InsertColumn(1, _T("Block reference"), LVCFMT_LEFT, 200);
  m_lcBlocks.InsertColumn(2, _T("Handle"),          LVCFMT_LEFT,   0);
  m_lcBlocks.InsertColumn(3, _T("Layer"),          LVCFMT_LEFT,  200);
  m_lcBlocks.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  // Add the values from the arrays
  for (int iCtr = 0; iCtr < m_csaNSBNames.GetSize(); iCtr++)
  {
    m_lcBlocks.InsertItem(iCtr, m_csaErrorNos.GetAt(iCtr));
    m_lcBlocks.SetItemText(iCtr, 1, m_csaNSBNames.GetAt(iCtr));
    m_lcBlocks.SetItemText(iCtr, 2, m_csaNSBHandles.GetAt(iCtr));
    m_lcBlocks.SetItemText(iCtr, 3, m_csaNSBLayers.GetAt(iCtr));
  }

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRBlocksDlg::OnClickObjectsList
// Description  : Called when the user clicks on an entry in the list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRBlocksDlg::OnClickObjectsList(NMHDR *pNMHDR, LRESULT *pResult)
{
  // Call the zoom function
  ZoomToSelection(m_lcBlocks, 2, 3);

  // Nullify the result
  *pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRBlocksDlg::OnBnClickedErrorAccept
// Description  : Called when the user clicks on the "Accept" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRBlocksDlg::OnBnClickedErrorAccept()
{
  // Get the selected item
  int iIndex = m_lcBlocks.GetSelectionMark();
  if (iIndex == LB_ERR) return;

  // Call the AcceptError method
  AcceptError(m_lcBlocks, iIndex, 2, _T("NON-STD BLOCK"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRBlocksDlg::WriteToReport
// Description  : Called to write to the already open report file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRBlocksDlg::WriteToReport(CStdioFile &cfLog)
{
  // If there is no data in the array, return now
  if (m_csaNSBNames.GetSize() == 0) return;

  // Write the tab header
  CString csLine; 
  csLine.Format(_T("\n\n-----------------------------------------\nBlocks with non-standard names: %d\n-----------------------------------------\n"), m_lcBlocks.GetItemCount()); 
  cfLog.WriteString(csLine);

  // Count number of different non-standard blocks
  int iIndex = 0;
  CString csQty;
  CStringArray csaNames, csaQtys;
  for (int iCtr = 0; iCtr < m_lcBlocks.GetItemCount(); iCtr++)
  {
    if (CheckForDuplication(m_lcBlocks.GetItemText(iCtr, 1), csaNames, iIndex) == TRUE)
    {
      csQty.Format(_T("%d"), _tstoi(csaQtys.GetAt(iIndex)) + 1);
      csaQtys.SetAt(iIndex, csQty);
    }
    else
    {
      csaNames.Add(m_lcBlocks.GetItemText(iCtr, 1));
      csaQtys.Add(_T("1"));
    }
  }

  // Write the details
  for (int iCtr = 0; iCtr < csaNames.GetSize(); iCtr++)
  {
    csLine.Format(_T("%d. %s blocks with name %s\n"), iCtr + 1, csaQtys.GetAt(iCtr), csaNames.GetAt(iCtr));
    cfLog.WriteString(csLine);
  }
}
