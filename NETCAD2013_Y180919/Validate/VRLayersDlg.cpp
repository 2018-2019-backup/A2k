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
#include "VRLayersDlg.h"


// CVRLayersDlg dialog
IMPLEMENT_DYNAMIC(CVRLayersDlg, CDialog)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::CVRLayersDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRLayersDlg::CVRLayersDlg(CWnd* pParent /*=NULL*/)	: CDialog(CVRLayersDlg::IDD, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::~CVRLayersDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVRLayersDlg::~CVRLayersDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRLayersDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LAYERS_LIST, m_lcLayers);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CVRLayersDlg, CDialog)
  ON_BN_CLICKED(IDC_LAYERS_FIX,        &CVRLayersDlg::OnBnClickedLayersFix)
  ON_BN_CLICKED(IDC_LAYERS_SELECT_ALL, &CVRLayersDlg::OnBnClickedLayersSelectAll)
  ON_BN_CLICKED(IDC_LAYERS_CLEAR_ALL,  &CVRLayersDlg::OnBnClickedLayersClearAll)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVRLayersDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  // Initialize the list control
  m_lcLayers.InsertColumn(0, _T("Layer"),       LVCFMT_LEFT,   250);
  m_lcLayers.InsertColumn(1, _T("Colour"),      LVCFMT_CENTER,  50);
  m_lcLayers.InsertColumn(2, _T("Std"),         LVCFMT_CENTER,  50);
  m_lcLayers.InsertColumn(3, _T("Linetype"),    LVCFMT_CENTER,  80);
  m_lcLayers.InsertColumn(4, _T("Std"),         LVCFMT_CENTER,  80);
  m_lcLayers.InsertColumn(5, _T("Line weight"), LVCFMT_CENTER,  80);
  m_lcLayers.InsertColumn(6, _T("Std"),         LVCFMT_CENTER,  80);
  m_lcLayers.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);

  // Add the values from the arrays
  int iIndex = 0;
  for (int iCtr = 0; iCtr < m_csaActNames.GetSize(); iCtr++)
  {
    iIndex = m_lcLayers.InsertItem(iCtr, m_csaActNames.GetAt(iCtr));
    m_lcLayers.SetItemText(iIndex, 1, m_csaActColors.GetAt(iCtr));
    m_lcLayers.SetItemText(iIndex, 2, m_csaStdColors.GetAt(iCtr));
    m_lcLayers.SetItemText(iIndex, 3, m_csaActLTypes.GetAt(iCtr));
    m_lcLayers.SetItemText(iIndex, 4, m_csaStdLTypes.GetAt(iCtr));
    m_lcLayers.SetItemText(iIndex, 5, m_csaActWeights.GetAt(iCtr));
    m_lcLayers.SetItemText(iIndex, 6, m_csaStdWeights.GetAt(iCtr));
  }

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::OnBnClickedLayersFix
// Description  : Called when the user clicks on the "Fix" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRLayersDlg::OnBnClickedLayersFix()
{
  // Lock the document and being a transaction
  acDocManager->lockDocument(curDoc());
  acTransactionManagerPtr()->startTransaction();

  // Loop through the list backwards and fix the selected entries
  for (int iCtr = m_lcLayers.GetItemCount() - 1; iCtr > -1; iCtr--)
  {
    if (m_lcLayers.GetCheck(iCtr) == TRUE)
    {
      if (RestoreLayer(iCtr) == TRUE)
      {
        m_lcLayers.DeleteItem(iCtr);
      }
    }
  }

  // Unlock the current document and end the transaction
  acTransactionManagerPtr()->endTransaction(); 
  acDocManager->unlockDocument(curDoc());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::OnBnClickedLayersSelectAll
// Description  : Called when the user clicks on the "Select all" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRLayersDlg::OnBnClickedLayersSelectAll()
{
  // Loop through the list and select all entries
  for (int iCtr = 0; iCtr < m_lcLayers.GetItemCount(); iCtr++)
  {
    m_lcLayers.SetCheck(iCtr, TRUE);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::OnBnClickedLayersClearAll
// Description  : Called when the user clicks on the "Clear all" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRLayersDlg::OnBnClickedLayersClearAll()
{
  // Loop through the list and unselect all entries
  for (int iCtr = 0; iCtr < m_lcLayers.GetItemCount(); iCtr++)
  {
    m_lcLayers.SetCheck(iCtr, FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::RestoreLayer
// Description  : Called to restore a layer to its standard state
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CVRLayersDlg::RestoreLayer(int iIndex)
{
  // Get the layer's standard data
  CString csLayerName  = m_lcLayers.GetItemText(iIndex, 0);
  CString csStdColour  = m_lcLayers.GetItemText(iIndex, 2);
  CString csStdLType   = m_lcLayers.GetItemText(iIndex, 4);
  CString csStdLWeight = m_lcLayers.GetItemText(iIndex, 6);

  // Get the layer table record for the selected layer
  AcDbLayerTable *pLayerTbl; 
  AcDbLayerTableRecord *pLayerTblRecord; 
  if (acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead) != Acad::eOk) return FALSE;
  if (pLayerTbl->getAt(csLayerName, pLayerTblRecord, AcDb::kForWrite) != Acad::eOk) { pLayerTbl->close(); return FALSE; }

  // If this is a non-standard layer and is not prefixed with "XT_"
  if ((csStdColour == _T("N.A.")) && (csLayerName.Mid(0, 3) != _T("XT_")))
  {
    // Change the layer name
    CString csNewLayerName;
    csNewLayerName.Format(_T("XT_%s"), csLayerName);
    pLayerTblRecord->setName(csNewLayerName);
  }
  // Otherwise
  else
  {
    // Set the layer properties
    AcCmColor cmColor; cmColor.setColorIndex(_tstoi(csStdColour)); pLayerTblRecord->setColor(cmColor);
    AcDbObjectId objLType = GetLineTypeObjectId(csStdLType); if (objLType.isValid()) pLayerTblRecord->setLinetypeObjectId(objLType);
    pLayerTblRecord->setLineWeight(AcDb::LineWeight(int(_tstof(csStdLWeight) * 100.0)));
  }

  // Close the layer
  pLayerTblRecord->close();
  pLayerTbl->close();

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CVRLayersDlg::WriteToReport
// Description  : Called to write to the already open report file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVRLayersDlg::WriteToReport(CStdioFile &cfLog)
{
  // If there is no data in the array, return now
  if (m_csaActNames.GetSize() == 0) return;

  // Write the tab header
  CString csLine, csTemp; 
  csLine.Format(_T("\n-----------------------------------------\nLayers with non-standard properties: %d\n-----------------------------------------\n"), m_lcLayers.GetItemCount()); 
  cfLog.WriteString(csLine);

  // Write the details
  bool bComma = false;
  for (int iCtr = 0; iCtr < m_lcLayers.GetItemCount(); iCtr++)
  {
    bComma = false;
    csLine.Format(_T("%d. "), iCtr + 1);
    if (m_lcLayers.GetItemText(iCtr, 1) != m_lcLayers.GetItemText(iCtr, 2)) { if (bComma) csLine += _T(", "); csLine += _T("Colour");      bComma = true; }
    if (m_lcLayers.GetItemText(iCtr, 3) != m_lcLayers.GetItemText(iCtr, 4)) { if (bComma) csLine += _T(", "); csLine += _T("Line type");   bComma = true; }
    if (m_lcLayers.GetItemText(iCtr, 5) != m_lcLayers.GetItemText(iCtr, 6)) { if (bComma) csLine += _T(", "); csLine += _T("Line weight"); bComma = true; }
    csTemp.Format(_T(" not standard for %s\n"), m_lcLayers.GetItemText(iCtr, 0));
    csLine += csTemp;
    cfLog.WriteString(csLine);
  }
}
