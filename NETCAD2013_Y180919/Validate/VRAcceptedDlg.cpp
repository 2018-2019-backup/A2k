////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : VRAcceptedDlg.cpp
// Created          : 18th February 2011
// Created by       : S. Jaisimha
// Description      : Implementation for the "Accepted" tab of the validation results dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "VRAcceptedDlg.h"
#include "AcceptErrorDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defined in ValidateInt.cpp
extern CString g_csError, g_csAccept;
extern CStringArray g_csaAccErrNos, g_csaAccErrTypes, g_csaAccErrHandles, g_csaAccErrLayers, g_csaAccErrReasons;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Zoom helper from ValidateResultsDlg.cpp
extern void ZoomToSelection(CListCtrl& lcList, int iHandleCol, int iLayerCol);


// CVRAcceptedDlg dialog
IMPLEMENT_DYNAMIC(CVRAcceptedDlg, CDialog)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::CVRAcceptedDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRAcceptedDlg::CVRAcceptedDlg(CWnd* pParent /*=NULL*/)	: CDialog(CVRAcceptedDlg::IDD, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::~CVRAcceptedDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRAcceptedDlg::~CVRAcceptedDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAcceptedDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_ACCEPT_LIST, m_lcAccepted);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CVRAcceptedDlg, CDialog)
  ON_NOTIFY(NM_CLICK, IDC_ACCEPT_LIST,  &CVRAcceptedDlg::OnClickAcceptedList)
  ON_NOTIFY(NM_DBLCLK, IDC_ACCEPT_LIST, &CVRAcceptedDlg::OnDoubleClickAcceptList)
  ON_BN_CLICKED(IDC_ACCEPT_SELECT_ALL, &CVRAcceptedDlg::OnBnClickedAcceptSelectAll)
  ON_BN_CLICKED(IDC_ACCEPT_CLEAR_ALL, &CVRAcceptedDlg::OnBnClickedAcceptClearAll)
  ON_BN_CLICKED(IDC_ACCEPT_CLEAR, &CVRAcceptedDlg::OnBnClickedAcceptClear)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVRAcceptedDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  // Initialize the list control
  m_lcAccepted.InsertColumn(0, _T("Error"),      LVCFMT_LEFT,   80);
  m_lcAccepted.InsertColumn(1, _T("Error Type"), LVCFMT_LEFT,  150);
  m_lcAccepted.InsertColumn(2, _T("Handle"),     LVCFMT_LEFT,    0);
  m_lcAccepted.InsertColumn(3, _T("ELayer"),     LVCFMT_LEFT,    0);
  m_lcAccepted.InsertColumn(4, _T("Layer"),      LVCFMT_LEFT,  200);
  m_lcAccepted.InsertColumn(5, _T("Reason"),     LVCFMT_LEFT,  300);
  m_lcAccepted.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);

  // Update the list
  UpdateAcceptedList();

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::UpdateAcceptedList
// Description  : Called to update the accepted error list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAcceptedDlg::UpdateAcceptedList()
{
  // Delete all items in the list
  m_lcAccepted.DeleteAllItems();

  // Add the values from the arrays
  for (int iCtr = 0; iCtr < g_csaAccErrNos.GetSize(); iCtr++)
  {
    m_lcAccepted.InsertItem(iCtr,     _T("A") + g_csaAccErrNos.GetAt(iCtr));  
    m_lcAccepted.SetItemText(iCtr, 1, g_csaAccErrTypes.GetAt(iCtr));
    m_lcAccepted.SetItemText(iCtr, 2, g_csaAccErrHandles.GetAt(iCtr));
    m_lcAccepted.SetItemText(iCtr, 3, g_csError);
    m_lcAccepted.SetItemText(iCtr, 4, g_csaAccErrLayers.GetAt(iCtr));
    m_lcAccepted.SetItemText(iCtr, 5, g_csaAccErrReasons.GetAt(iCtr));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::OnClickObjectsList
// Description  : Called when the user clicks on an entry in the list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAcceptedDlg::OnClickAcceptedList(NMHDR *pNMHDR, LRESULT *pResult)
{
  // Call the zoom function
  ZoomToSelection(m_lcAccepted, 2, 3);

  // Nullify the result
  *pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::OnDoubleClickAcceptList
// Description  : Called when the user double-clicks on an entry in the list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAcceptedDlg::OnDoubleClickAcceptList(NMHDR *pNMHDR, LRESULT *pResult)
{
  // Get the selected item
  LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
  int iIndex = pNMItemActivate->iItem;
  if (iIndex == -1) return;

  // Display the accept error dialog
  CAcceptErrorDlg dlgAE;
  dlgAE.m_bEditing    = TRUE;
  dlgAE.m_csErrNo     = m_lcAccepted.GetItemText(iIndex, 0);
  dlgAE.m_csErrType   = m_lcAccepted.GetItemText(iIndex, 1);
  dlgAE.m_csErrReason = m_lcAccepted.GetItemText(iIndex, 5);
  if (dlgAE.DoModal() == IDCANCEL) return;

  // Nullify the result
  *pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::OnBnClickedAcceptSelectAll
// Description  : Called when the clicks on the "Select all" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAcceptedDlg::OnBnClickedAcceptSelectAll()
{
  // Check all items in the list
  for (int iCtr = 0; iCtr < m_lcAccepted.GetItemCount(); iCtr++) m_lcAccepted.SetCheck(iCtr, TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::OnBnClickedAcceptClearAll
// Description  : Called when the clicks on the "Clear all" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAcceptedDlg::OnBnClickedAcceptClearAll()
{
  // Un-check all items in the list
  for (int iCtr = 0; iCtr < m_lcAccepted.GetItemCount(); iCtr++) m_lcAccepted.SetCheck(iCtr, FALSE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::OnBnClickedAcceptClear
// Description  : Called when the clicks on the "Clear selected" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAcceptedDlg::OnBnClickedAcceptClear()
{
  BOOL bChecked = FALSE, bCleared = FALSE;

  // Ensure that something is checked in the list
  for (int iCtr = 0; iCtr < m_lcAccepted.GetItemCount(); iCtr++) if (m_lcAccepted.GetCheck(iCtr)) bChecked = TRUE;
  if (bChecked == FALSE) { appMessage(_T("At least one accepted error must be selected.")); return; }

  // Get a confirmation
  if (getConfirmation(_T("This will reset the selected errors in the list.\nDo you want to continue?")) == IDNO) return;

  // Lock document and begin transaction
  acDocManager->lockDocument(curDoc());
  acTransactionManagerPtr()->startTransaction();

  // For each item in the list (in reverse order)
  for (int iCtr = m_lcAccepted.GetItemCount() - 1; iCtr >= 0; iCtr--) 
  {
    // If it is not checked, continue
    if (m_lcAccepted.GetCheck(iCtr) == FALSE) continue;

    // Get the error block handle and the entity name
    CString csErrHandle = m_lcAccepted.GetItemText(iCtr, 2);
    ads_name enError; if (acdbHandEnt(csErrHandle, enError) != RTNORM) continue;

    // Get the "Parent" xdata
    struct resbuf *rbpParent = getXDataFromEntity(enError, _T("Parent"));
    if (!rbpParent) continue;

    // Get the handle of the parent entity and from there its entity name
    CString csParentHandle = rbpParent->rbnext->resval.rstring;
    ads_name enParent; if (acdbHandEnt(csParentHandle, enParent) != RTNORM) continue;

    // Remove the "Accepted" xdata from the parent
    struct resbuf *rbpAccepted = acutBuildList(AcDb::kDxfRegAppName, XDATA_Accepted, NULL);
    addXDataToEntity(enParent, rbpAccepted);

    // Delete the error block
    acdbEntDel(enError);

    // Remove the entry from the list
    m_lcAccepted.DeleteItem(iCtr);

    // Set the flag
    bCleared = TRUE;
  }

  // Unlock document and end transaction
  acDocManager->unlockDocument(curDoc());
  acTransactionManagerPtr()->endTransaction();

  // If the flag is set
  if (bCleared == TRUE)
  {
    // Update the drawing screen
    acedGetAcadFrame()->SetFocus();
    acedGetAcadFrame()->RedrawWindow();

    // Display a message
    appMessage(_T("Please re-run the validation command to display the\naccepted errors again in their appropriate tabs."));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRAcceptedDlg::WriteToReport
// Description  : Called to write to the already open report file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRAcceptedDlg::WriteToReport(CStdioFile &cfLog)
{
  // If there is no data in the list, return now
  if (m_lcAccepted.GetItemCount() == 0) return;

  // Write the tab header
  CString csLine; 
  csLine.Format(_T("\n\n-----------------------------------------\nAccepted errors: %d\n-----------------------------------------\n"), m_lcAccepted.GetItemCount()); 
  cfLog.WriteString(csLine);

  // Output the movement of assets
  for (int iCtr = 0; iCtr < m_lcAccepted.GetItemCount(); iCtr++)
  {
    csLine.Format(_T("%d. [A%s] %s on layer %s  -  %s\n"), iCtr + 1, m_lcAccepted.GetItemText(iCtr, 0), m_lcAccepted.GetItemText(iCtr, 1), m_lcAccepted.GetItemText(iCtr, 4), m_lcAccepted.GetItemText(iCtr, 5));
    cfLog.WriteString(csLine);
  }
}
