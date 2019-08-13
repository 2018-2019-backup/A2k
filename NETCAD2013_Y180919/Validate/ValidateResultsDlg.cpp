////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : ValidateResultsDlg.cpp
// Created          : 23rd January 2008
// Created by       : S. Jaisimha
// Description      : Implementation for the validation results dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "ValidateResultsDlg.h"
#include "AcceptErrorDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defined in ValidateInt.cpp
extern CString g_csError;
extern CValidateResultsDlg *g_pdlgVR;
extern CStringArray g_csaAccErrNos, g_csaAccErrTypes, g_csaAccErrHandles, g_csaAccErrLayers, g_csaAccErrReasons;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defined in ValidateInt.cpp
void CloseResultsDialog();


// CValidateResultsDlg dialog
IMPLEMENT_DYNAMIC(CValidateResultsDlg, CDialog)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateResultsDlg::CValidateResultsDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CValidateResultsDlg::CValidateResultsDlg(CWnd* pParent /*=NULL*/)	: CDialog(CValidateResultsDlg::IDD, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateResultsDlg::~CValidateResultsDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CValidateResultsDlg::~CValidateResultsDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateResultsDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateResultsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_VALIDATE_TABS, m_tabResults);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CValidateResultsDlg, CDialog)
  ON_NOTIFY(TCN_SELCHANGE, IDC_VALIDATE_TABS, &CValidateResultsDlg::OnSelectTab)
  ON_BN_CLICKED(IDCANCEL, &CValidateResultsDlg::OnBnClickedCancel)
  ON_BN_CLICKED(IDC_SAVE_RESULTS, &CValidateResultsDlg::OnBnClickedSaveResults)
  ON_BN_CLICKED(IDHELP, &CValidateResultsDlg::OnBnClickedHelp)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateResultsDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CValidateResultsDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  // Get the tab rectangle and size it properly
  CRect rcTab; 
  m_tabResults.GetClientRect(rcTab); 
  rcTab.left   += 6;
  rcTab.top    += 26;
  rcTab.right  -= 10;
  rcTab.bottom -= 10;

  int iTabIndex = 0;

  // If at least one layer is present in the array
  if (m_dlgLayers.m_csaActNames.GetSize() > 0)
  {
    m_tabResults.InsertItem(iTabIndex++, _T("Non-std layers"));
    m_dlgLayers.Create(IDD_VALIDATE_RESULTS_LAYERS, &m_tabResults); 
    m_dlgLayers.MoveWindow(rcTab); 
    m_dlgLayers.ShowWindow(iTabIndex == 1 ? SW_SHOW : SW_HIDE);
    //EnableThemeDialogTexture(m_dlgLayers.GetSafeHwnd(), ETDT_USETABTEXTURE);
  }

  // If at least one object is present in the array
  if (m_dlgObjects.m_csaObjTypes.GetSize() > 0)
  {
    m_tabResults.InsertItem(iTabIndex++, _T("Objects"));
    m_dlgObjects.Create(IDD_VALIDATE_RESULTS_OBJECTS, &m_tabResults); 
    m_dlgObjects.MoveWindow(rcTab); 
    m_dlgObjects.ShowWindow(iTabIndex == 1 ? SW_SHOW : SW_HIDE);
    //EnableThemeDialogTexture(m_dlgObjects.GetSafeHwnd(), ETDT_USETABTEXTURE);
  }

  // If at least one object is present in the array
  if (m_dlgBlocks.m_csaNSBNames.GetSize() > 0)
  {
    m_tabResults.InsertItem(iTabIndex++, _T("Non-std blocks"));
    m_dlgBlocks.Create(IDD_VALIDATE_RESULTS_BLOCKS, &m_tabResults); 
    m_dlgBlocks.MoveWindow(rcTab); 
    m_dlgBlocks.ShowWindow(iTabIndex == 1 ? SW_SHOW : SW_HIDE);
    //EnableThemeDialogTexture(m_dlgBlocks.GetSafeHwnd(), ETDT_USETABTEXTURE);
  }

  // If at least one object is present in the array
  if (m_dlgAttribs.m_csaMANames.GetSize() > 0)
  {
    m_tabResults.InsertItem(iTabIndex++, _T("Attributes"));
    m_dlgAttribs.Create(IDD_VALIDATE_RESULTS_ATTRIBS, &m_tabResults); 
    m_dlgAttribs.MoveWindow(rcTab); 
    m_dlgAttribs.ShowWindow(iTabIndex == 1 ? SW_SHOW : SW_HIDE);
    //EnableThemeDialogTexture(m_dlgAttribs.GetSafeHwnd(), ETDT_USETABTEXTURE);
  }

  // If at least one object is present in the array
  if (m_dlgLayer0.m_csa0Names.GetSize() > 0)
  {
    m_tabResults.InsertItem(iTabIndex++, _T("Layer 0"));
    m_dlgLayer0.Create(IDD_VALIDATE_RESULTS_LAYER0, &m_tabResults); 
    m_dlgLayer0.MoveWindow(rcTab); 
    m_dlgLayer0.ShowWindow(iTabIndex == 1 ? SW_SHOW : SW_HIDE);
    //EnableThemeDialogTexture(m_dlgLayer0.GetSafeHwnd(), ETDT_USETABTEXTURE);
  }

  // If at least one object is present in the array
  if (m_dlgMoved.m_csaMovNames.GetSize() > 0)
  {
    m_tabResults.InsertItem(iTabIndex++, _T("Moved assets"));
    m_dlgMoved.Create(IDD_VALIDATE_RESULTS_MOVEMENT, &m_tabResults); 
    m_dlgMoved.MoveWindow(rcTab); 
    m_dlgMoved.ShowWindow(iTabIndex == 1 ? SW_SHOW : SW_HIDE);
    //EnableThemeDialogTexture(m_dlgMoved.GetSafeHwnd(), ETDT_USETABTEXTURE);
  }

  // Add the "Accepted" tab anyway
  m_tabResults.InsertItem(iTabIndex++, _T("Accepted"));
  m_dlgAccepted.Create(IDD_VALIDATE_RESULTS_ACCEPT, &m_tabResults);
  m_dlgAccepted.MoveWindow(rcTab); 
  m_dlgAccepted.ShowWindow(iTabIndex == 1 ? SW_SHOW : SW_HIDE);
  //EnableThemeDialogTexture(m_dlgAccepted.GetSafeHwnd(), ETDT_USETABTEXTURE);

  // If tab index is 1, no tab was created
  if (iTabIndex == 1) { CDialog::OnCancel(); return TRUE; }

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateResultsDlg::OnSelectTab
// Description  : Called when the user changes the tab
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateResultsDlg::OnSelectTab(NMHDR *pNMHDR, LRESULT *pResult)
{
  // Nullify the result
  *pResult = 0;

  //  Get the selected tab item text
  ACHAR buffer[256] = {0};
  TCITEM tcItem;
  tcItem.pszText = buffer;
  tcItem.cchTextMax = 256;
  tcItem.mask = TCIF_TEXT;
  m_tabResults.GetItem(m_tabResults.GetCurSel(), &tcItem);
  CString csSelTab = tcItem.pszText;

  // Hide all dialogs
  m_dlgLayers.ShowWindow(SW_HIDE);
  m_dlgObjects.ShowWindow(SW_HIDE); 
  m_dlgBlocks.ShowWindow(SW_HIDE);  
  m_dlgAttribs.ShowWindow(SW_HIDE); 
  m_dlgLayer0.ShowWindow(SW_HIDE); 
  m_dlgMoved.ShowWindow(SW_HIDE); 
  m_dlgAccepted.ShowWindow(SW_HIDE);

  // Show appropriate dialog and hide the others
  /**/ if (csSelTab == _T("Non-std layers")) m_dlgLayers.ShowWindow(SW_SHOW);
  else if (csSelTab == _T("Objects"))        m_dlgObjects.ShowWindow(SW_SHOW);
  else if (csSelTab == _T("Non-std blocks")) m_dlgBlocks.ShowWindow(SW_SHOW);
  else if (csSelTab == _T("Attributes"))     m_dlgAttribs.ShowWindow(SW_SHOW); 
  else if (csSelTab == _T("Layer 0"))        m_dlgLayer0.ShowWindow(SW_SHOW); 
  else if (csSelTab == _T("Moved assets"))   m_dlgMoved.ShowWindow(SW_SHOW); 
  else if (csSelTab == _T("Accepted"))       m_dlgAccepted.ShowWindow(SW_SHOW); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateResultsDlg::OnBnClickedSaveResults
// Description  : Called when the user clicks on the "Save" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateResultsDlg::OnBnClickedSaveResults()
{
  // Get the drawing name and path and form the path to the ".log" file in the same name in the same folder
  CString csDwgPath, csDwgName, csLogFile;
  struct resbuf *rbpPath = acutNewRb(RTSTR); acedGetVar(_T("DWGPREFIX"), rbpPath); csDwgPath = rbpPath->resval.rstring; acutRelRb(rbpPath);
  struct resbuf *rbpName = acutNewRb(RTSTR); acedGetVar(_T("DWGNAME"),   rbpName); csDwgName = rbpPath->resval.rstring; acutRelRb(rbpName);
  csLogFile.Format(_T("%s%s"), csDwgPath, csDwgName); csLogFile.Replace(_T(".dwg"), _T(".log")); csLogFile.Replace(_T(".DWG"), _T(".log"));
  
  // If the report log file exists, ask the user whether it should be overwritten
  if ((_taccess(csLogFile, 00) != -1) && (getConfirmation(csLogFile + _T("\n\nFile already exists. Do you want to overwrite the old report with the new one?")) == IDNO)) return;

  // Create the file
  CStdioFile cfLog;
  CFileException exLog;
  if (cfLog.Open(csLogFile, CFile::modeCreate | CFile::modeWrite, &exLog) == FALSE)
  {
    TCHAR   szCause[255];
    CString strFormatted;
    exLog.GetErrorMessage(szCause, 255);
    strFormatted.Format(_T("%s\n\nThe log file could not be opened.\n%s"), csLogFile, szCause);
    appMessage(strFormatted, MB_ICONSTOP);
    return;
  }

  // Write the header lines
  CString csLine; 
  unsigned long lNameSize = 250L;
  TCHAR szCompName[MAX_PATH];
  CTime tmNow = CTime::GetCurrentTime();
  GetComputerName(szCompName, &lNameSize);
  csLine.Format(_T("NET CAD Validation Report\n--------------------------\n"));                 cfLog.WriteString(csLine);
  csLine.Format(_T("Report created on  : %s\n"),     tmNow.Format(_T("%d/%m/%Y at %H:%M:%S"))); cfLog.WriteString(csLine);
  csLine.Format(_T("Validated drawing  : %s%s\n"),   csDwgPath, csDwgName);                     cfLog.WriteString(csLine);
  csLine.Format(_T("Standards drawing  : %s\n"),     g_csLocalDWT);                             cfLog.WriteString(csLine);
  csLine.Format(_T("Standards database : %s\n"),     g_csLocalMDB);                             cfLog.WriteString(csLine);
  csLine.Format(_T("Validated by       : %s\n\n"),   szCompName);                               cfLog.WriteString(csLine);

  // Call each tab's report writer
  m_dlgLayers.WriteToReport(cfLog);
  m_dlgObjects.WriteToReport(cfLog);
  m_dlgBlocks.WriteToReport(cfLog);
  m_dlgAttribs.WriteToReport(cfLog);
  m_dlgLayer0.WriteToReport(cfLog);
  m_dlgMoved.WriteToReport(cfLog);
  m_dlgAccepted.WriteToReport(cfLog);

  // Close the log file
  cfLog.Close();

  // Show a message
  appMessage(csLogFile + _T("\nValidation report log file was saved successfully."));

  // Open the log file
  CString csRunThis; 
  csRunThis.Format(_T("\"%s\""), csLogFile);
  ShellExecute(this->m_hWnd, _T("open"), csRunThis, NULL, NULL, SW_SHOWNORMAL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateResultsDlg::OnBnClickedHelp
// Description  : Called when the user clicks on the "Help..." button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateResultsDlg::OnBnClickedHelp()
{
  // Display the appropriate topic
  displayHelp((DWORD)_T("Validate.htm"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateResultsDlg::OnBnClickedCancel
// Description  : Called when the user closes the dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateResultsDlg::OnBnClickedCancel()
{
  // If user wants to delete the validation error tags, clear everything on the validation errors layer
  if (getConfirmation(_T("Do you want to remove all remaining error tags?")) == IDYES) 
  {
    // Make the selection set of all "ERROR" blocks on the validation layer
    ads_name ssErase;
    if (acedSSGet(_T("X"), NULL, NULL, acutBuildList(RTDXF0, _T("INSERT"), 2, g_csError, 8, g_csError, NULL), ssErase) == RTNORM)
    {
      //long lErrors = 0L;
      //if ((acedSSLength(ssErase, &lErrors) != RTNORM) || (lErrors == 0L)) { acedSSFree(ssErase); return; }

		int lErrors = 0;
		if ((acedSSLength(ssErase, &lErrors) != RTNORM) || (lErrors == 0)) { acedSSFree(ssErase); return; }

      // Lock the current document
      acDocManager->lockDocument(curDoc());

      // For each entity in the selection set
      ads_name enErase;
      for (long lCtr = 0L; lCtr < lErrors; lCtr++)
      {
        // Erase the entity
        acedSSName(ssErase, lCtr, enErase);
        acdbEntDel(enErase);
      }
    }

    // Free the selection set
    acedSSFree(ssErase);

    // Unlock the current document
    acDocManager->unlockDocument(curDoc());
  }

  // Close the dialog
  OnCancel();

  // Call the closure function
  CloseResultsDialog();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ZoomToSelection
// Description  : Called to zoom to the selected object (forces layer to be visible and thawed)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ZoomToSelection(CListCtrl& lcList, int iHandleCol, int iLayerCol)
{
  // Get the handle and layer from the selection in the list
  int iIndex = lcList.GetSelectionMark(); if (iIndex == LB_ERR) return;
  CString csHandle = lcList.GetItemText(iIndex, iHandleCol);
  CString csLayer  = lcList.GetItemText(iIndex, iLayerCol);

  // Since we are doing this through the palette, we must lock the document
  acDocManager->lockDocument(curDoc());
  acTransactionManagerPtr()->startTransaction();

  // Get the entity's extents
  ads_name enObject;  if (acdbHandEnt(csHandle, enObject) != RTNORM) { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
  AcDbObjectId objId; if (acdbGetObjectId(objId, enObject) != Acad::eOk) { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
  AcDbEntity *pEnt;   if (acdbOpenObject(pEnt, objId, AcDb::kForRead) != Acad::eOk) { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
  AcDbExtents exEnt;  pEnt->getGeomExtents(exEnt);
  pEnt->close();

  // Ensure that the layer is visible and thawed
  AcDbLayerTable *pLayerTbl; acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead);
  if (pLayerTbl->has(csLayer) == false) { pLayerTbl->close(); acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
  AcDbLayerTableRecord *pLayerRec; pLayerTbl->getAt(csLayer, pLayerRec, AcDb::kForWrite);
  pLayerRec->setIsFrozen(false); pLayerRec->setIsOff(false);
  pLayerRec->close();
  pLayerTbl->close();

  /////////////////////////////////////////////////////////////////////////////////////////////
  // Standard ZOOM code from ADN (DevNote ID: TS8781)
  // Please report any errors in the following to Autodesk, not DeltaCADD

  AcDbViewTableRecord view;
  struct resbuf rb;
  struct resbuf wcs, dcs, ccs; // acedTrans co-ordinate system flags
  ads_point vpDir;
  ads_point wmin = { exEnt.minPoint().x, exEnt.minPoint().y, 0.0 };
  ads_point wmax = { exEnt.maxPoint().x, exEnt.maxPoint().y, 0.0 };
  ads_point wdcsmax, wdcsmin;   // windows corners in device coords
  AcGeVector3d viewDir;
  AcGePoint2d cenPt;

  ads_real lenslength, viewtwist, frontz, backz;
  ads_point target;

  int viewmode, tilemode, cvport;

  wcs.restype = RTSHORT; wcs.resval.rint = 0; // WORLD   co-ordinate system flag
  ccs.restype = RTSHORT; ccs.resval.rint = 1; // CURRENT co-ordinate system flag
  dcs.restype = RTSHORT; dcs.resval.rint = 2; // DEVICE  co-ordinate system flag

  // Get the 'VPOINT' direction vector
  acedGetVar(_T("VIEWDIR"), &rb); //VIEWDIR is reported in UCS
  acedTrans(rb.resval.rpoint, &ccs, &wcs, 0, vpDir);
  viewDir.set(vpDir[X], vpDir[Y], vpDir[Z]);

  // Convert upper right window corner to DCS
  acedTrans(wmax, &ccs, &dcs, 0, wdcsmax);

  // Convert lower left window corner to DCS
  acedTrans(wmin, &ccs, &dcs, 0, wdcsmin);

  // Calculate and set view center point
  cenPt.set(wdcsmin[X] + ((wdcsmax[X] - wdcsmin[X]) / 2.0), wdcsmin[Y] + ((wdcsmax[Y] - wdcsmin[Y]) / 2.0));
  view.setCenterPoint(cenPt);

  // Set view height and width and direction
  view.setHeight(fabs(wdcsmax[Y] - wdcsmin[Y]) * 4.0);
  view.setWidth(fabs(wdcsmax[X] - wdcsmin[X]) * 4.0);
  view.setViewDirection(viewDir);

  // Get other properties
  acedGetVar(_T("LENSLENGTH"), &rb); lenslength = rb.resval.rreal; 
  acedGetVar(_T("VIEWTWIST"),  &rb); viewtwist  = rb.resval.rreal; 
  acedGetVar(_T("FRONTZ"),     &rb); frontz     = rb.resval.rreal;
  acedGetVar(_T("BACKZ"),      &rb); backz      = rb.resval.rreal;
  acedGetVar(_T("VIEWMODE"),   &rb); viewmode   = rb.resval.rint;
  acedGetVar(_T("TILEMODE"),   &rb); tilemode   = rb.resval.rint;
  acedGetVar(_T("CVPORT"),     &rb); cvport     = rb.resval.rint;

  // Set the view properties
  view.setLensLength(lenslength);
  view.setViewTwist(viewtwist);
  view.setPerspectiveEnabled(viewmode & 1);
  view.setFrontClipEnabled(viewmode & 2);
  view.setBackClipEnabled(viewmode & 4);
  view.setFrontClipAtEye(!(viewmode & 16));

  // Paper space flag
  Adesk::Boolean paperspace = ((tilemode == 0) && (cvport == 1)) ? Adesk::kTrue : Adesk::kFalse;
  view.setIsPaperspaceView(paperspace);

  // If we are in Model Space
  if (Adesk::kFalse == paperspace)
  {
    view.setFrontClipDistance(frontz);
    view.setBackClipDistance(backz);
  }
  // If we are in Paper Space
  else
  {
    view.setFrontClipDistance(0.0);
    view.setBackClipDistance(0.0);
  }

  acedGetVar(_T("TARGET"), &rb);
  acedTrans(rb.resval.rpoint, &ccs, &wcs, 0, target);
  view.setTarget(AcGePoint3d(target[X], target[Y], target[Z]));

  // Update view (this will actually do the ZOOM)
  acedSetCurrentView(&view, NULL);

  // End standard ZOOM code from ADN (DevNote ID: TS8781)
  /////////////////////////////////////////////////////////////////////////////////////////////

  // Unlock the current document and end the transaction
  acDocManager->unlockDocument(curDoc());
  acTransactionManagerPtr()->endTransaction(); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : AcceptError
// Description  : Called to insert the accept block
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AcceptError(CListCtrl& lcList, int iRow, int iHandleCol, CString csErrorType)
{
  // Get the error count number
  CString csErrNo; csErrNo.Format(_T("%d"), g_pdlgVR->m_dlgAccepted.m_lcAccepted.GetItemCount() + 1);

  // Display the accept error dialog
  CAcModuleResourceOverride useMe;
  CAcceptErrorDlg dlgAE;
  dlgAE.m_csErrNo = csErrNo; //lcList.GetItemText(iRow, 0);
  dlgAE.m_csErrType = csErrorType;
  if (dlgAE.DoModal() == IDCANCEL) return;

  // Lock document and begin transaction
  acDocManager->lockDocument(curDoc());
  acTransactionManagerPtr()->startTransaction();

  // Get the parent handle of the error block
  CString csParentHandle, csParentLayer, csHandle = lcList.GetItemText(iRow, iHandleCol);
  ads_name enObject; if (acdbHandEnt(csHandle, enObject) != RTNORM) { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
  struct resbuf *rbpParent = getXDataFromEntity(enObject, _T("Parent"));
  if (rbpParent) { csParentHandle = rbpParent->rbnext->resval.rstring; acutRelRb(rbpParent); }

  // Erase the associated ERROR block
  AcDbObjectId objId;       if (acdbGetObjectId(objId, enObject) != Acad::eOk)             { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
  AcDbBlockReference *pErr; if (acdbOpenObject(pErr, objId, AcDb::kForWrite) != Acad::eOk) { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
  AcGePoint3d gePos = pErr->position();
  pErr->erase();
  pErr->close();

  // Insert the "ACCEPTED" block at the same place (this will insert without attribute values)
  AcDbObjectId objAccept;
  insertBlock(g_csAccept, g_csError, asDblArray(gePos), 0.0, 0.0, 0.0, 0.0, _T(""), objAccept, TRUE);

  // Re-open the block and get the handle and attribute iterator
  AcDbBlockReference *pAcc; if (acdbOpenObject(pAcc, objAccept, AcDb::kForRead) != Acad::eOk) { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
  AcDbHandle dbHandle; pAcc->getAcDbHandle(dbHandle);
  ACHAR szHandle[17]; dbHandle.getIntoAsciiBuffer(szHandle);
  CString csErrHandle = szHandle;
  AcDbObjectIterator *pIter = pAcc->attributeIterator();
  pAcc->close();

  // Update the attributes in the block
  for (pIter->start(); !pIter->done(); pIter->step())
  {
    AcDbObjectId objAtt = pIter->objectId();
    AcDbAttribute *pAtt; if (acdbOpenObject(pAtt, objAtt, AcDb::kForWrite) != Acad::eOk) { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
    CString csTag = pAtt->tag();
    /**/ if (csTag == _T("ERRNO"))        pAtt->setTextString(dlgAE.m_csErrNo);
    else if (csTag == _T("ERROR_TYPE"))   pAtt->setTextString(dlgAE.m_csErrType);
    else if (csTag == _T("ERROR_REASON")) pAtt->setTextString(dlgAE.m_csErrReason);
    pAtt->close();
  }

  // If the parent handle is valid
  if (!csParentHandle.IsEmpty())
  {
    // Add the "Accepted" xdata to it
    ads_name enParent;
    if (acdbHandEnt(csParentHandle, enParent) != RTNORM) { acDocManager->unlockDocument(curDoc()); acTransactionManagerPtr()->abortTransaction(); return; }
    csParentLayer = Assoc(acdbEntGet(enParent), 8)->resval.rstring;
    struct resbuf *rbpAccepted = acutBuildList(AcDb::kDxfRegAppName, XDATA_Accepted, AcDb::kDxfXdAsciiString, dlgAE.m_csErrNo, AcDb::kDxfXdAsciiString, dlgAE.m_csErrType, AcDb::kDxfXdAsciiString, csParentLayer, AcDb::kDxfXdAsciiString, dlgAE.m_csErrReason, AcDb::kDxfXdHandle, csErrHandle, NULL);
    addXDataToEntity(enParent, rbpAccepted);
    acutRelRb(rbpAccepted);

    // Add the "Parent" handle to the Accepted block
    ads_name enAccepted; acdbGetAdsName(enAccepted, objAccept);
    rbpParent = acutBuildList(AcDb::kDxfRegAppName, _T("Parent"), AcDb::kDxfXdHandle, csParentHandle, NULL);
    addXDataToEntity(enAccepted, rbpParent);
    acutRelRb(rbpParent);
  }

  // Unlock document and end transaction
  acDocManager->unlockDocument(curDoc());
  acTransactionManagerPtr()->endTransaction();

  // Remove the entry from the list
  lcList.DeleteItem(iRow);

  // Update the "Accepted" tab in the dialog
  if (g_pdlgVR) 
  {
    g_csaAccErrNos.Add(dlgAE.m_csErrNo);
    g_csaAccErrTypes.Add(dlgAE.m_csErrType);
    g_csaAccErrHandles.Add(csErrHandle);
    g_csaAccErrLayers.Add(csParentLayer);
    g_csaAccErrReasons.Add(dlgAE.m_csErrReason);

    g_pdlgVR->m_dlgAccepted.UpdateAcceptedList();
  }

  // Update the drawing screen
  acedGetAcadFrame()->SetFocus();
  acedGetAcadFrame()->RedrawWindow();
}
