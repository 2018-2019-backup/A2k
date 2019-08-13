////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : VRLayersDlg.cpp
// Created          : 23rd January 2008
// Created by       : S. Jaisimha
// Description      : Implementation for the "Layers" tab of the validation results dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "VRObjectsDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Zoom helper from ValidateResultsDlg.cpp
extern void ZoomToSelection(CListCtrl& lcList, int iHandleCol, int iLayerCol);
extern void AcceptError(CListCtrl& lcList, int iRow, int iHandleCol, CString csErrorType);


// CVRObjectsDlg dialog
IMPLEMENT_DYNAMIC(CVRObjectsDlg, CDialog)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::CVRObjectsDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRObjectsDlg::CVRObjectsDlg(CWnd* pParent /*=NULL*/)	: CDialog(CVRObjectsDlg::IDD, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::~CVRObjectsDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRObjectsDlg::~CVRObjectsDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRObjectsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_OBJECTS_LIST, m_lcObjects);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CVRObjectsDlg, CDialog)
  ON_NOTIFY(NM_CLICK, IDC_OBJECTS_LIST, &CVRObjectsDlg::OnZoomToObject)
  ON_BN_CLICKED(IDC_OBJECTS_FIX,        &CVRObjectsDlg::OnBnClickedObjectsFix)
  ON_BN_CLICKED(IDC_OBJECTS_SELECT_ALL, &CVRObjectsDlg::OnBnClickedObjectsSelectAll)
  ON_BN_CLICKED(IDC_OBJECTS_CLEAR_ALL,  &CVRObjectsDlg::OnBnClickedObjectsClearAll)
  ON_BN_CLICKED(IDC_ERROR_ACCEPT,       &CVRObjectsDlg::OnBnClickedErrorAccept)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVRObjectsDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  // Initialize the list control
  m_lcObjects.InsertColumn(0, _T("Error"),  LVCFMT_LEFT,  80);
  m_lcObjects.InsertColumn(1, _T("Object"), LVCFMT_LEFT, 120);
  m_lcObjects.InsertColumn(2, _T("Handle"), LVCFMT_LEFT,   0);
  m_lcObjects.InsertColumn(3, _T("Layer"),  LVCFMT_LEFT, 200);
  m_lcObjects.InsertColumn(4, _T("Std"),    LVCFMT_LEFT, 200);
  m_lcObjects.InsertColumn(5, _T("EHdl"),   LVCFMT_LEFT,   0);
  m_lcObjects.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);

  // Add the values from the arrays
  for (int iCtr = 0; iCtr < m_csaObjTypes.GetSize(); iCtr++)
  {
    m_lcObjects.InsertItem(iCtr, m_csaErrorNos.GetAt(iCtr));
    m_lcObjects.SetItemText(iCtr, 1, m_csaObjTypes.GetAt(iCtr));
    m_lcObjects.SetItemText(iCtr, 2, m_csaObjHandles.GetAt(iCtr));
    m_lcObjects.SetItemText(iCtr, 3, m_csaObjActLayers.GetAt(iCtr));
    m_lcObjects.SetItemText(iCtr, 4, m_csaObjStdLayers.GetAt(iCtr));
    m_lcObjects.SetItemText(iCtr, 5, m_csaErrorHandles.GetAt(iCtr));
  }

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::OnZoomToObject
// Description  : Called when the user clicks on an entry in the list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRObjectsDlg::OnZoomToObject(NMHDR *pNMHDR, LRESULT *pResult)
{
  // Call the zoom function
  ZoomToSelection(m_lcObjects, 5, 3);

  // Nullify the result
  *pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::OnBnClickedObjectsFix
// Description  : Called when the user clicks on the "Fix" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRObjectsDlg::OnBnClickedObjectsFix()
{
  // Lock the document and being a transaction
  acDocManager->lockDocument(curDoc());
  acTransactionManagerPtr()->startTransaction();

  // Loop through the list backwards and fix the selected entries
  for (int iCtr = m_lcObjects.GetItemCount() - 1; iCtr > -1; iCtr--)
  {
    if (m_lcObjects.GetCheck(iCtr) == TRUE)
    {
      if (RestoreObject(iCtr) == TRUE)
      {
        m_lcObjects.DeleteItem(iCtr);
      }
    }
  }

  // Unlock the current document and end the transaction
  acTransactionManagerPtr()->endTransaction(); 
  acDocManager->unlockDocument(curDoc());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::OnBnClickedObjectsSelectAll
// Description  : Called when the user clicks on the "Select all" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRObjectsDlg::OnBnClickedObjectsSelectAll()
{
  // Loop through the list and select all entries
  for (int iCtr = 0; iCtr < m_lcObjects.GetItemCount(); iCtr++)
  {
    m_lcObjects.SetCheck(iCtr, TRUE);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::OnBnClickedObjectsClearAll
// Description  : Called when the user clicks on the "Clear all" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRObjectsDlg::OnBnClickedObjectsClearAll()
{
  // Loop through the list and unselect all entries
  for (int iCtr = 0; iCtr < m_lcObjects.GetItemCount(); iCtr++)
  {
    m_lcObjects.SetCheck(iCtr, FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::OnBnClickedErrorAccept
// Description  : Called when the user clicks on the "Accept" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRObjectsDlg::OnBnClickedErrorAccept()
{
  // Get the selected item
  int iIndex = m_lcObjects.GetSelectionMark();
  if (iIndex == LB_ERR) return;

  // Call the AcceptError method
  AcceptError(m_lcObjects, iIndex, 5, _T("CHANGED LAYER"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::RestoreObject
// Description  : Called to restore an object to its original layer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVRObjectsDlg::RestoreObject(int iIndex)
{
  // Get the object's details from the list
  CString csHandle    = m_lcObjects.GetItemText(iIndex, 2);
  CString csLayer     = m_lcObjects.GetItemText(iIndex, 4);
  CString csErrHandle = m_lcObjects.GetItemText(iIndex, 5);

  // Open the object and set its layer
  ads_name enObj;     if (acdbHandEnt(csHandle, enObj)  != RTNORM)    return FALSE;
  AcDbObjectId objId; if (acdbGetObjectId(objId, enObj) != Acad::eOk) return FALSE;
  AcDbEntity *pEnt;   if (acTransactionManagerPtr()->getObject((AcDbObject*&)pEnt, objId, AcDb::kForWrite) != Acad::eOk) return FALSE;
  if (pEnt->setLayer(csLayer) != Acad::eOk) { pEnt->close(); return FALSE; }
  pEnt->close();

  // Delete the error block associated with this object
  if (acdbHandEnt(csErrHandle, enObj) != RTNORM)    return FALSE;
  if (acdbGetObjectId(objId, enObj)   != Acad::eOk) return FALSE;
  if (acTransactionManagerPtr()->getObject((AcDbObject*&)pEnt, objId, AcDb::kForWrite) != Acad::eOk) return FALSE;
  if (pEnt->erase() != Acad::eOk) { pEnt->close(); return FALSE; }
  pEnt->close();

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRObjectsDlg::WriteToReport
// Description  : Called to write to the already open report file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRObjectsDlg::WriteToReport(CStdioFile &cfLog)
{
  // If there is no data in the array, return now
  if (m_csaObjTypes.GetSize() == 0) return;

  // Write the tab header
  CString csLine; 
  csLine.Format(_T("\n\n-----------------------------------------\nObjects not on original layers: %d\n-----------------------------------------\n"), m_lcObjects.GetItemCount()); 
  cfLog.WriteString(csLine);

  // Write the details
  for (int iCtr = 0; iCtr < m_lcObjects.GetItemCount(); iCtr++)
  {
    csLine.Format(_T("%d. [E%s] %s is on layer %s, should be on layer %s\n"), iCtr + 1, m_lcObjects.GetItemText(iCtr, 0), m_lcObjects.GetItemText(iCtr, 1), m_lcObjects.GetItemText(iCtr, 3), m_lcObjects.GetItemText(iCtr, 4));
    cfLog.WriteString(csLine);
  }
}
