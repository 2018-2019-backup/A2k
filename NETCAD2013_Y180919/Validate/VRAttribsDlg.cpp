////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : VRAttribsDlg.cpp
// Created          : 24th January 2008
// Created by       : S. Jaisimha
// Description      : Implementation for the "Attributes" tab of the validation results dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "VRAttribsDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Zoom helper from ValidateResultsDlg.cpp
extern void ZoomToSelection(CListCtrl& lcList, int iHandleCol, int iLayerCol);
extern void AcceptError(CListCtrl& lcList, int iRow, int iHandleCol, CString csErrorType);


// CVRAttribsDlg dialog
IMPLEMENT_DYNAMIC(CVRAttribsDlg, CDialog)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAttribsDlg::CVRAttribsDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRAttribsDlg::CVRAttribsDlg(CWnd* pParent /*=NULL*/)	: CDialog(CVRAttribsDlg::IDD, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAttribsDlg::~CVRAttribsDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRAttribsDlg::~CVRAttribsDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAttribsDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAttribsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_ATTRIBS_LIST, m_lcAttribs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CVRAttribsDlg, CDialog)
  ON_NOTIFY(NM_CLICK, IDC_ATTRIBS_LIST, &CVRAttribsDlg::OnZoomToObject)
  ON_BN_CLICKED(IDC_ERROR_ACCEPT,       &CVRAttribsDlg::OnBnClickedErrorAccept)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAttribsDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVRAttribsDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  // Initialize the list control
  m_lcAttribs.InsertColumn(0, _T("Error"),           LVCFMT_LEFT,   80);
  m_lcAttribs.InsertColumn(1, _T("Block / Object"),  LVCFMT_LEFT,  200);
  m_lcAttribs.InsertColumn(2, _T("Handle"),          LVCFMT_LEFT,    0);
  m_lcAttribs.InsertColumn(3, _T("Layer"),           LVCFMT_LEFT,  200);
  m_lcAttribs.InsertColumn(4, _T("Unfilled"),        LVCFMT_LEFT,  200);
  m_lcAttribs.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  // Add the values from the arrays
  for (int iCtr = 0; iCtr < m_csaMANames.GetSize(); iCtr++)
  {
    m_lcAttribs.InsertItem(iCtr, m_csaErrorNos.GetAt(iCtr));
    m_lcAttribs.SetItemText(iCtr, 1, m_csaMANames.GetAt(iCtr));
    m_lcAttribs.SetItemText(iCtr, 2, m_csaMAHandles.GetAt(iCtr));
    m_lcAttribs.SetItemText(iCtr, 3, m_csaMALayers.GetAt(iCtr));
    m_lcAttribs.SetItemText(iCtr, 4, m_csaMAAttribs.GetAt(iCtr));
  }

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAttribsDlg::OnZoomToObject
// Description  : Called when the user clicks on an entry in the list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAttribsDlg::OnZoomToObject(NMHDR *pNMHDR, LRESULT *pResult)
{
  // Call the zoom function
  ZoomToSelection(m_lcAttribs, 2, 3);

  // Nullify the result
  *pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRMovedDlg::OnBnClickedErrorAccept
// Description  : Called when the user clicks on the "Accept" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAttribsDlg::OnBnClickedErrorAccept()
{
  // Get the selected item
  int iIndex = m_lcAttribs.GetSelectionMark();
  if (iIndex == LB_ERR) return;

  // Call the AcceptError method
  AcceptError(m_lcAttribs, iIndex, 2, _T("UNFILLED ATTRIBUTE"));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAttribsDlg::WriteToReport
// Description  : Called to write to the already open report file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAttribsDlg::WriteToReport(CStdioFile &cfLog)
{
  // If there is no data in the array, return now
  if (m_csaMANames.GetSize() == 0) return;

  // Write the tab header
  CString csLine, csTemp; 
  csLine.Format(_T("\n-----------------------------------------\nBlocks with un-filled mandatory attributes: %d\n-----------------------------------------\n"), m_lcAttribs.GetItemCount()); 
  cfLog.WriteString(csLine);

  // Write the details
  for (int iCtr = 0; iCtr < m_lcAttribs.GetItemCount(); iCtr++)
  {
    csLine.Format(_T("%d. [E%s] Block %s on layer %s, unfilled %s\n"), iCtr + 1, m_lcAttribs.GetItemText(iCtr, 0), m_lcAttribs.GetItemText(iCtr, 1), m_lcAttribs.GetItemText(iCtr, 3), m_lcAttribs.GetItemText(iCtr, 4));
    cfLog.WriteString(csLine);
  }
}
