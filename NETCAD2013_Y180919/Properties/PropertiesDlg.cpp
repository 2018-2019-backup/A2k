////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : PropertiesDlg.cpp
// Created          : 31st January 2008
// Created by       : S. Jaisimha
// Description      : Implementation for the properties dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "PropertiesDlg.h"
#include <comdef.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Undocumented
extern void ads_regen();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defined in DXFin.cpp
bool WildMatch(const TCHAR *pattern, const TCHAR *str);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals
CString g_csVaries = _T("*Varies*");

// CPropertiesDlg dialog
IMPLEMENT_DYNAMIC(CPropertiesDlg, CDialog)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::CPropertiesDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPropertiesDlg::CPropertiesDlg(CWnd* pParent /*=NULL*/)	: CDialog(CPropertiesDlg::IDD, pParent)
{
  m_bDrawName = TRUE;
  m_bDrawDesc = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::~CPropertiesDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPropertiesDlg::~CPropertiesDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_PROPS_OBJTYPE, m_cbType);
  DDX_Control(pDX, IDC_PROPS_LIST,    m_plList);
  DDX_Control(pDX, IDC_PROPS_APPLY,   m_btnApply);
  DDX_Control(pDX, IDC_PROPS_REMOVE,  m_btnRemove);

  DDX_Control(pDX, IDC_LINEAR_COND,       m_stCondLabel);
  DDX_Control(pDX, IDC_LINEAR_COND_LINE,  m_stCondLine);
  DDX_Control(pDX, IDC_LINEAR_MAND,       m_stMandLabel);
  DDX_Control(pDX, IDC_LINEAR_MAND_LINE,  m_stMandLine);
  DDX_Control(pDX, IDC_LINEAR_LABEL_USE,  m_stLabelUse);
  DDX_Control(pDX, IDC_LINEAR_USE,        m_cbUse);
  DDX_Control(pDX, IDC_LINEAR_LABEL_INS,  m_stLabelIns);
  DDX_Control(pDX, IDC_LINEAR_INS,        m_cbIns);
  DDX_Control(pDX, IDC_LINEAR_LABEL_CODE, m_stLabelCode);
  DDX_Control(pDX, IDC_LINEAR_CODE,       m_cbCode);
  DDX_Control(pDX, IDC_LINEAR_LABEL_DESC, m_stLabelDesc);
  DDX_Control(pDX, IDC_LINEAR_DESC,       m_cbDesc);
  DDX_Control(pDX, IDC_LINEAR_LABEL_NAME, m_stLabelName);
  DDX_Control(pDX, IDC_LINEAR_NAME,       m_cbName);
  DDX_Control(pDX, IDC_LINEAR_CHECK,      m_btnDrawDesc);
  DDX_Control(pDX, IDC_LINEAR_CHECK2,     m_btnDrawName);
  DDX_Control(pDX, IDC_LINEAR_RESET,      m_btnReset);

  DDX_CBString(pDX, IDC_LINEAR_USE,       m_csUse);
  DDX_CBString(pDX, IDC_LINEAR_INS,       m_csIns);
  DDX_CBString(pDX, IDC_LINEAR_CODE,      m_csCode);
  DDX_CBString(pDX, IDC_LINEAR_DESC,      m_csDesc);
  DDX_CBString(pDX, IDC_LINEAR_NAME,      m_csName);
  DDX_Check(pDX,    IDC_LINEAR_CHECK,     m_bDrawDesc);
  DDX_Check(pDX,    IDC_LINEAR_CHECK2,    m_bDrawName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPropertiesDlg, CDialog)
  ON_CBN_SELCHANGE(IDC_PROPS_OBJTYPE, &CPropertiesDlg::OnSelectObjectType)
  ON_CBN_SELCHANGE(IDC_LINEAR_USE,    &CPropertiesDlg::OnSelectUse)
  ON_CBN_SELCHANGE(IDC_LINEAR_INS,    &CPropertiesDlg::OnSelectIns)
  ON_CBN_SELCHANGE(IDC_LINEAR_CODE,   &CPropertiesDlg::OnSelectCode)
  ON_CBN_SELCHANGE(IDC_LINEAR_DESC,   &CPropertiesDlg::OnSelectDesc)
  ON_CBN_SELCHANGE(IDC_LINEAR_NAME,   &CPropertiesDlg::OnSelectName)
  ON_BN_CLICKED(IDC_LINEAR_RESET,     &CPropertiesDlg::OnBnClickedReset)
  ON_BN_CLICKED(IDC_PROPS_APPLY,      &CPropertiesDlg::OnBnClickedApply)
  ON_BN_CLICKED(IDC_PROPS_REMOVE,     &CPropertiesDlg::OnBnClickedRemove)
  ON_BN_CLICKED(IDC_LINEAR_CHECK,     &CPropertiesDlg::OnCheckDrawDesc)
  ON_BN_CLICKED(IDC_LINEAR_CHECK2,    &CPropertiesDlg::OnCheckDrawName)
  ON_BN_CLICKED(IDC_PROPS_HELP,       &CPropertiesDlg::OnBnClickedHelp)
  ON_BN_CLICKED(IDC_PROPS_SELECT,     &CPropertiesDlg::OnBnClickedSelect)
  ON_BN_CLICKED(IDC_PROPS_REGEN,      &CPropertiesDlg::OnBnClickedRegen)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_EVENTSINK_MAP
// Description  : Message handlers for the property list control
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENTSINK_MAP(CPropertiesDlg, CDialog)
  ON_EVENT(CPropertiesDlg, IDC_PROPS_LIST, 1 /* PropertyChanged */, OnPropertyChanged, VTS_BSTR VTS_VARIANT)
END_EVENTSINK_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::SetSelectedHandlesAndLayers
// Description  : Called to retrieve the selected handles and the distinct layer names
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::SetSelectedHandlesAndLayers(CString csType)
{
  // Clear the selected handles and layers array
  m_csaSelHandles.RemoveAll(); m_csaSelLayers.RemoveAll();

  // Retrieve the handles associated with this object type from the other arrays
  for (int iCtr = 0; iCtr < m_csaTypes.GetSize(); iCtr++) if (m_csaTypes.GetAt(iCtr) == csType) m_csaSelHandles.Add(m_csaHandles.GetAt(iCtr));

  // Get the distinct layers for the selected handles
  ads_name enEnt;
  CString csLayer;
  struct resbuf *rbpEnt = NULL;
  for (int iCtr = 0; iCtr < m_csaSelHandles.GetSize(); iCtr++)
  {
    // Get the layer name
    acdbHandEnt(m_csaSelHandles.GetAt(iCtr), enEnt);
    rbpEnt = acdbEntGet(enEnt);
    csLayer = Assoc(rbpEnt, 8)->resval.rstring;
    acutRelRb(rbpEnt);

    // Add it to array if it doesn't already exist
    if (CheckForDuplication(csLayer, m_csaSelLayers) == FALSE) m_csaSelLayers.Add(csLayer);
  }

  // If there is more than one layer
  if (m_csaSelLayers.GetSize() > 1) 
  {
    // Show a warning message
    // appMessage(_T("Selected objects are not on the same layer."));

    // Add a "Varies" string
    m_csaSelLayers.InsertAt(0, g_csVaries);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::SetLinearObjectHandles
// Description  : Called to retrieve the handles for the selected Use/Installation values of linear objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::SetLinearObjectHandles(CString csUse, CString csIns)
{
  ads_name enEnt;
  CString csLayer;
  struct resbuf *rbpEnt = NULL;

  // Clear the array
  m_csaSelLinearHandles.RemoveAll();

  // Get the layers for the selected handles
  for (int iCtr = 0; iCtr < m_csaSelHandles.GetSize(); iCtr++)
  {
    // Get the layer name
    acdbHandEnt(m_csaSelHandles.GetAt(iCtr), enEnt);
    rbpEnt = acdbEntGet(enEnt);
    csLayer = Assoc(rbpEnt, 8)->resval.rstring;
    acutRelRb(rbpEnt);

    // If this layer's use value is same ("HV_OH_EXIST" == "HV")
    if (csLayer.Mid(0, csUse.GetLength()) == csUse)
    {
      csLayer = csLayer.Mid(csUse.GetLength() + 1);

      // If this layer's installation value is same ("HV_OH_EXIST" == "OH")
      if (csLayer.Mid(0, csIns.GetLength()) == csIns)
      {
        // Add this to the linear handles array
        m_csaSelLinearHandles.Add(m_csaSelHandles.GetAt(iCtr));
      }
      // Specific for "TR" layers
      else if (csUse == _T("TR"))
      {
        // Skip the voltage string and check for installation ("TR_66_OH_EXIST" == "OH")
        csLayer = csLayer.Mid(csLayer.Find('_') + 1);
        if (csLayer.Mid(0, csIns.GetLength()) == csIns)
        {
          // Add this to the linear handles array
          m_csaSelLinearHandles.Add(m_csaSelHandles.GetAt(iCtr));
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::GetLinearPropertyValue
// Description  : Called to retrieve the value for the given property from the selected handles
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::GetLinearPropertyValue(CString csProp, CString& csValue)
{
  CString csTemp;
  ads_name enObject;
  struct resbuf *rbpXData = NULL;

  // Clear the value
  csValue.Empty();

  // For each selected handle
  for (int iCtr = 0; iCtr < m_csaSelHandles.GetSize(); iCtr++)
  {
    // Get the entity name from the handle
    if (acdbHandEnt(m_csaSelHandles.GetAt(iCtr), enObject) != RTNORM) continue;

    // Get the xdata for the given property
    if (rbpXData = getXDataFromEntity(enObject, csProp))
    {
      // Get the value and if even one is different, assign "*Varies*"
      csTemp = rbpXData->rbnext->resval.rstring;
      acutRelRb(rbpXData);
      if (csValue.IsEmpty()) csValue = csTemp;
      else if (csValue != csTemp) csValue = g_csVaries;
    }
    // If the xdata does not exists and there is already a value, assign "*Varies*"
    else if (!csValue.IsEmpty()) csValue = g_csVaries;
  }

  // Return the status
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::GetBlockPropertyValue
// Description  : Called to retrieve the value for the given property from the selected handles
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::GetBlockPropertyValue(CString csTag, CString& csValue)
{
  BOOL bFound = FALSE;
  CString csTemp;
  ads_name enObject;
  AcDbObjectId objRef, objAtt;
  AcDbAttribute *pAtt = NULL;
  AcDbBlockReference *pRef = NULL;
  AcDbObjectIterator *pIter = NULL;

  // Clear the value
  csValue.Empty();

  // For each selected handle
  for (int iCtr = 0; iCtr < m_csaSelHandles.GetSize(); iCtr++)
  {
    // Get the entity name from the handle
    if (acdbHandEnt(m_csaSelHandles.GetAt(iCtr), enObject) != RTNORM) continue;

    // Open the block reference for reading and get the attribute iterator
    if (acdbGetObjectId(objRef, enObject) != Acad::eOk) continue;
    if (acdbOpenObject(pRef, objRef, AcDb::kForRead) != Acad::eOk) continue;
    pIter = pRef->attributeIterator();
    pRef->close();

    // Loop through the iterator and find this property
    bFound = FALSE;
    for (pIter->start(); !pIter->done(); pIter->step())
    {
      objAtt = pIter->objectId();
      if (acdbOpenObject(pAtt, objAtt, AcDb::kForWrite) != Acad::eOk) continue;
      if (csTag.CompareNoCase(pAtt->tag()) == 0) 
      { 
        csTemp = pAtt->textString(); 
        pAtt->close();  
        bFound = TRUE; 
        break; 
      }

      pAtt->close();
    }

    // Assign the value if found
    if (bFound)
    {
      if (iCtr == 0) csValue = csTemp;
      else if (csValue != csTemp) csValue = g_csVaries;
    }
  }

  // Return the status
  return bFound;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::ResetArrays
// Description  : Called to reset all arrays
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::ResetArrays()
{
  // Clear the arrays
  m_uiaBlockQtys.RemoveAll(); 
  m_csaLayers.RemoveAll(); 
  m_csaBlockNames.RemoveAll();
  m_csaTypes.RemoveAll(); 
  m_csaHandles.RemoveAll(); 
  m_csaSelHandles.RemoveAll(); 
  m_csaSelLayers.RemoveAll(); 
  m_csaSelLinearHandles.RemoveAll();
  m_csaPropTags.RemoveAll(); 
  m_csaPropNames.RemoveAll(); 
  m_csaPropValues.RemoveAll(); 
  m_csaPropMand.RemoveAll(); 
  m_csaPropUnique.RemoveAll();
  m_csaCodes.RemoveAll(); 
  m_csaDescs.RemoveAll(); 
  m_csaNames.RemoveAll();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();
  //For ACAD 2018
  SetWindowText(_T("NETCAD 2018: Object Properties V1.0"));
  // Move the dialog to the left of the screen
  CRect rcDialog;
  GetWindowRect(rcDialog);
  int iWidth      = rcDialog.Width();
  int iHeight     = rcDialog.Height();
  rcDialog.left   = 10;
  rcDialog.top    = 100;
  rcDialog.right  = rcDialog.left + iWidth;
  rcDialog.bottom = rcDialog.top + iHeight;
  MoveWindow(rcDialog);

  // Remove the border on the property list
  m_plList.SetBorderStyle(0);

  // Set the font for the two initially hidden labels
  CFont *pFont = new CFont;
  pFont->CreatePointFont(80, _T("Arial Bold"));
  m_stCondLabel.SetFont(pFont);
  m_stMandLabel.SetFont(pFont);

  // Reset all arrays
  ResetArrays();

  // Loop through the array
  ads_name enPicked;
  int iInserts = 0, iLinear = 0, iText = 0, iIndex = 0;
  CString csPicked, csHandle, csBlock, csLayer;
  struct resbuf *rbpPicked = NULL;
  for (long lCtr = 0L; lCtr < m_csaPickedHandles.GetSize(); lCtr++)
  {
    // Get the entity from the handle and check if it is valid
    if(acdbHandEnt(m_csaPickedHandles.GetAt(lCtr), enPicked) != RTNORM) continue;
    rbpPicked = acdbEntGet(enPicked);
    if (!rbpPicked) continue;

    // Get the selected object's details
    csPicked  = Assoc(rbpPicked, 0)->resval.rstring;
    csHandle  = Assoc(rbpPicked, 5)->resval.rstring;
    csLayer   = Assoc(rbpPicked, 8)->resval.rstring;

    // If this is on an "_EXIST" layer, ignore it
    if (csLayer.Find(_T("_EXIST")) > 0) continue;

    // If it is a block reference
    if (csPicked == _T("INSERT")) 
    {
      // Increment the counter
      iInserts++;

      // Get the block name and see if it is already present
      csBlock = Assoc(rbpPicked, 2)->resval.rstring;
      if (CheckForDuplication(csBlock, m_csaBlockNames, iIndex) == FALSE)
      {
        m_csaBlockNames.Add(csBlock);
        m_uiaBlockQtys.Add(1);
      }
      else m_uiaBlockQtys.SetAt(iIndex, m_uiaBlockQtys.GetAt(iIndex) + 1);

      // Set the entity type to the block name
      csPicked = csBlock;
    }
    // If it is any of the below, it is considered as a linear object
    else if ((csPicked == _T("ARC")) || (csPicked == _T("CIRCLE")) || (csPicked == _T("LINE")) || (csPicked == _T("POLYLINE")) || (csPicked == _T("LWPOLYLINE")) || (csPicked == _T("SPLINE"))) 
    { 
      csPicked = _T("Linear objects"); 
      iLinear++; 
    }
    // Otherwise, we are not interested
    else { acutRelRb(rbpPicked); continue; }

    // Release the buffer
    acutRelRb(rbpPicked);

    // Add the type and handle to the array
    m_csaTypes.Add(csPicked);
    m_csaHandles.Add(csHandle);
  }

  // Add the proper values to the first combo
  if (iInserts > 0) 
  { 
    for (int iCtr = 0; iCtr < m_csaBlockNames.GetSize(); iCtr++)
    {
      csPicked.Format(_T("Block %s (%d)"), m_csaBlockNames.GetAt(iCtr), m_uiaBlockQtys.GetAt(iCtr));
      m_cbType.AddString(csPicked);
    }
  }
  if (iLinear > 0) { csPicked.Format(_T("Linear objects (%d)"), iLinear); m_cbType.AddString(csPicked); }
  if (iText   > 0) { csPicked.Format(_T("Text (%d)"), iText);             m_cbType.AddString(csPicked); }

  // If there is nothing in the combo, return
  if (m_cbType.GetCount() == 0) { OnCancel(); return TRUE; }

  // Collect all layer names into the array
  ACHAR *pszLayer = NULL;
  AcDbLayerTable *pLyrTbl; acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLyrTbl, AcDb::kForRead);
  AcDbLayerTableIterator *pLyrIter; pLyrTbl->newIterator(pLyrIter); pLyrTbl->close();
  AcDbLayerTableRecord *pLyrRcd;
  for (pLyrIter->start(); !pLyrIter->done(); pLyrIter->step())
  {
    pLyrIter->getRecord(pLyrRcd, AcDb::kForRead);
    pLyrRcd->getName(pszLayer);
    pLyrRcd->close();
    m_csaLayers.Add(pszLayer);
  }
  delete pLyrIter;

  // Set the selection to the first entry and call the reactor
  m_cbType.SetCurSel(0); OnSelectObjectType();

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::SetLinearVisibility
// Description  : Shows or hides the linear visibility controls
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::SetLinearVisibility(int iVis)
{
  m_stCondLabel.ShowWindow(iVis); m_stCondLine.ShowWindow(iVis);
  m_stMandLabel.ShowWindow(iVis); m_stMandLine.ShowWindow(iVis);
  
  m_stLabelUse.ShowWindow(iVis); 
  m_stLabelIns.ShowWindow(iVis);
  m_stLabelCode.ShowWindow(iVis); 
  m_stLabelDesc.ShowWindow(iVis); 
  m_stLabelName.ShowWindow(iVis);

  m_cbUse.ShowWindow(iVis); 
  m_cbIns.ShowWindow(iVis);
  m_cbCode.ShowWindow(iVis); 
  m_cbDesc.ShowWindow(iVis); 
  m_cbName.ShowWindow(iVis);
  m_btnReset.ShowWindow(iVis);
  m_btnDrawDesc.ShowWindow(iVis);
  m_btnDrawName.ShowWindow(iVis);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnSelectObjectType
// Description  : Called when the user changes the selection in the "Object type" combo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnSelectObjectType()
{
  bool bIsMandatory = false, bIsUnique = false, bIsEnabled = true;
  CString csSQL, csTag, csDesc, csLongDesc, csPropValue, csFunc = _T("OnSelectObjectType");
  CQueryTbl tblFeatures, tblValues;
  CStringArray *pcsaData = NULL;

  // Reset the property list and the value arrays
  m_plList.Clear(); 
  m_csaPropTags.RemoveAll();
  m_csaPropNames.RemoveAll();
  m_csaPropValues.RemoveAll();
  m_csaPropMand.RemoveAll();
  m_csaPropUnique.RemoveAll();

  // Disable the "Apply" button
  m_btnApply.EnableWindow(FALSE);
  
  // Reset the object type
  m_iObjectType = 0;

  // Get the selected object type and its quantity
  int iSel = m_cbType.GetCurSel(); if (iSel == CB_ERR) return;
  CString csObject, csType; m_cbType.GetLBText(iSel, csObject);
  CString csObjQty = csObject.Mid(csObject.Find(_T("(")) + 1);
  csObjQty.Replace(_T(")"), _T(""));
  int iObjQty = _tstoi(csObjQty);

  // If this is block references
  if (csObject.Mid(0, 5) == _T("Block"))
  {
    CString csMAtt = _T("Mandatory attributes"), csNMAtt = _T("Other attributes");

    // Set the selection type
    m_iObjectType = 1;

    // Show the property list and hide the linear control
    m_plList.ShowWindow(SW_SHOW);
    SetLinearVisibility(SW_HIDE);

    // Get the block name from the selection
    CString csBlock = csObject.Mid(6);
    csBlock = csBlock.Mid(0, csBlock.Find(_T(" (")));

    // Fill the selected handles array and their layers
    SetSelectedHandlesAndLayers(csBlock);

    // Get the attributes for this block
    csSQL.Format(_T("SELECT [Tag], [Desc], [Mandatory], [Unique] FROM tblBlockProps WHERE [Block] = '%s' ORDER BY [Mandatory], [Sequence], [Tag]"), csBlock);
    if (!tblFeatures.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, csFunc,true)) return;
    if (tblFeatures.GetRows() <= 0) return;

    // For each property
    for (int iCtr = 0; iCtr < tblFeatures.GetRows(); iCtr++)
    {
      // Get the tag, description, mandatory and unique flags
      pcsaData     = tblFeatures.GetRowAt(iCtr);
      csTag        = pcsaData->GetAt(0);
      csDesc       = pcsaData->GetAt(1);
      bIsMandatory = _tstoi(pcsaData->GetAt(2));
      bIsUnique    = _tstoi(pcsaData->GetAt(3));
      csLongDesc.Format(_T("%s (tag: %s)"), csDesc, csTag);

      //////////////////////////////////////////////////////////////////////////////
      // Removed on request, via mail from Darren McMonigal, dated 13.04.2011
      //
      // Decide whether this property should be enabled
      // bIsEnabled = (((bIsUnique == true) && (iObjQty > 1)) ? false : true);
      //
      //////////////////////////////////////////////////////////////////////////////

      // Get the value for this property from the selected objects
      if (GetBlockPropertyValue(csTag, csPropValue) == FALSE) continue;

      // Store the property name and its value
      m_csaPropTags.Add(csTag);
      m_csaPropNames.Add(csDesc);
      m_csaPropValues.Add(csPropValue);
      m_csaPropMand.Add(pcsaData->GetAt(2));
      m_csaPropUnique.Add(pcsaData->GetAt(3));

      // Get the possible values for this block/tag combination
      csSQL.Format(_T("SELECT [Data] FROM tblAttributeData WHERE [Block] = '%s' AND [Tag] = '%s' ORDER BY [Sequence]"), csBlock, csTag);
      if (!tblValues.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, csFunc,true)) return;

      // If there are no values
      if (tblValues.GetRows() <= 0)
      {
        // Create this as a edit box property and set its value
        m_plList.AddProperty((bIsMandatory ? csMAtt : csNMAtt), csDesc, COleVariant(_T("")), csLongDesc, 0, bIsEnabled);
        m_plList.SetValue(csDesc, COleVariant(csPropValue));
      }
      // Otherwise create it as a combo box
      else
      {
        int iValue = 0;
        COleSafeArray osaValues;
        BSTR bsValues[100];
        for (iValue = 0; iValue < tblValues.GetRows(); iValue++) bsValues[iValue] = ::SysAllocString(tblValues.GetRowAt(iValue)->GetAt(0));
        if (csPropValue == g_csVaries) bsValues[iValue++] = ::SysAllocString(g_csVaries);
        osaValues.CreateOneDim(VT_BSTR, iValue, bsValues);
        m_plList.AddProperty((bIsMandatory ? csMAtt : csNMAtt), csDesc, osaValues, csLongDesc, -1, bIsEnabled);
        m_plList.SetValue(csDesc, COleVariant(csPropValue));
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Code added 14.06.2011, If there are additional attributes in the block that are not in the table

    // Open the first block object in the array (since all of them should be the same block)
    ads_name enObject; if (acdbHandEnt(m_csaSelHandles.GetAt(0), enObject) != RTNORM) return;
    AcDbObjectId objRef; if (acdbGetObjectId(objRef, enObject) != Acad::eOk) return;
    AcDbBlockReference *pRef; if (acdbOpenObject(pRef, objRef, AcDb::kForRead) != Acad::eOk) return;
    AcDbObjectIterator *pIter = pRef->attributeIterator();
    pRef->close();

    // Loop through the iterator
    for (pIter->start(); !pIter->done(); pIter->step())
    {
      // Open the attribute for reading and get its tag
      AcDbObjectId objAtt = pIter->objectId();
      AcDbAttribute *pAtt; if (acdbOpenObject(pAtt, objAtt, AcDb::kForWrite) != Acad::eOk) continue;
      csTag = pAtt->tag();
      pAtt->close();

      // If this tag is already in the array
      if (CheckForDuplication(csTag, m_csaPropTags) == TRUE) continue;

      // Get its property value
      if (GetBlockPropertyValue(csTag, csPropValue) == FALSE) continue;

      // Create this as a edit box property and set its value
      m_plList.AddProperty(csNMAtt, csTag, COleVariant(_T("")), csTag, 0, bIsEnabled);
      m_plList.SetValue(csTag, COleVariant(csPropValue));

      // Store the property name and its value
      m_csaPropTags.Add(csTag);
      m_csaPropNames.Add(csTag);
      m_csaPropValues.Add(csPropValue);
      m_csaPropMand.Add(_T("0"));
      m_csaPropUnique.Add(_T("0"));
    }

    // End code added 14.06.2011
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  }
  // If this is linear objects
  else if (csObject.Mid(0, 6) == _T("Linear"))
  {
    // Set the selection type
    m_iObjectType = 2;

    // Hide the property list and show the linear controls
    m_plList.ShowWindow(SW_HIDE);
    SetLinearVisibility(SW_SHOW);

    // Reset all combos and variables
    m_cbUse.ResetContent();  m_csUse.Empty();
    m_cbIns.ResetContent();  m_csIns.Empty();
    m_cbCode.ResetContent(); m_csCode.Empty();
    m_cbDesc.ResetContent(); m_csDesc.Empty();
    m_cbName.ResetContent(); m_csName.Empty();

    // Fill the selected handles array and their layers
    SetSelectedHandlesAndLayers(_T("Linear objects"));

    // Populate the "Use" combo based on the layer names of the selected objects
    CString csUse, csIns, csCode;
    for (int iCtr = 0; iCtr < m_csaSelLayers.GetSize(); iCtr++) 
    {
      // Get the voltage level from the layer name ("HV_OH_EXIST" or "LV_UG_PROP" or similar)
      csUse = m_csaSelLayers.GetAt(iCtr);
      csUse = csUse.Mid(0, csUse.Find('_'));

      // If it isn't already in the combo, add it
      if ((csUse.IsEmpty() == FALSE) && (m_cbUse.FindStringExact(-1, csUse) == -1)) m_cbUse.AddString(csUse);
    }

    // If there is only one entry, select it and call the reactor
    if (m_cbUse.GetCount() == 1) { m_cbUse.SetCurSel(0); OnSelectUse(); }

    // Get the previously selected code from the selected objects
    GetLinearPropertyValue(_T("Code"), csCode);
    if (csCode != g_csVaries) { m_cbCode.SetCurSel(m_cbCode.FindStringExact(-1, csCode)); OnSelectCode(); }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnSelectUse
// Description  : Called when the user changes the selection in the "Use" combo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnSelectUse()
{
  // Reset the combo and variable
  m_cbIns.ResetContent(); m_csIns.Empty();

  // Get the selection
  int iIndex = m_cbUse.GetCurSel();
  if (iIndex == CB_ERR) return;
  m_cbUse.GetLBText(iIndex, m_csUse);

  // Populate the "Installation" combos based on the layer names of the selected objects and the selected "Use"
  CString csIns, csTRIns;
  for (int iCtr = 0; iCtr < m_csaSelLayers.GetSize(); iCtr++) 
  {
    csIns = m_csaSelLayers.GetAt(iCtr);

    // If this layer's use is same as the one selected
    if (csIns.Mid(0, m_csUse.GetLength()) == m_csUse)
    {
      // Get the installation from the layer name ("OH" or "UG")
      csIns.Replace(m_csUse + _T("_"), _T(""));
      csIns = csIns.Mid(0, csIns.Find('_'));

      // If it is not "OH" or "UG"
      if ((csIns != _T("OH")) && (csIns != _T("UG"))) 
      {
        // If the use is "TR"
        if (m_csUse == _T("TR"))
        {
          // Get the installation after the second "_"
          csTRIns = m_csaSelLayers.GetAt(iCtr);
          csTRIns.Replace(m_csUse +_T("_") + csIns + _T("_"), _T(""));
          csTRIns = csTRIns.Mid(0, csTRIns.Find('_'));

          // If it is still not "OH" or "UG", loop back
          if ((csTRIns != _T("OH")) && (csTRIns != _T("UG"))) continue;
          // Otherwise, copy the value
          else csIns = csTRIns;
        }
        // Otherwise, loop back
        else continue;
      }

      // If it isn't already in the combo, add it
      if ((csIns.IsEmpty() == FALSE) && (m_cbIns.FindStringExact(-1, csIns) == -1)) m_cbIns.AddString(csIns);
    }
  }

  // If there is only one entry, select it and call the reactor
  if (m_cbIns.GetCount() == 1) { m_cbIns.SetCurSel(0); OnSelectIns(); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnSelectIns
// Description  : Called when the user changes the selection in the "Installation" combo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnSelectIns()
{
  // Clear the dependent combos and variables
  m_cbCode.ResetContent(); m_csCode.Empty(); m_csaCodes.RemoveAll();
  m_cbDesc.ResetContent(); m_csDesc.Empty(); m_csaDescs.RemoveAll();
  m_cbName.ResetContent(); m_csName.Empty(); m_csaNames.RemoveAll();

  // If the use is not selected yet or the installation is not selected, return
  int iIndex = m_cbIns.GetCurSel();
  if ((m_csUse.IsEmpty() == TRUE) || (iIndex == CB_ERR)) return;

  // Get the selected value
  m_cbIns.GetLBText(iIndex, m_csIns);

  // Get the code, description and name values for the selected use and installation
  CString csSQL;
  CQueryTbl tblCodes;
  csSQL.Format(_T("SELECT [Code], [Description], [CName] FROM tblCodes WHERE %s = '%s' ORDER BY [Code]"), m_csUse, m_csIns);
  if (!tblCodes.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, _T("OnSelectInstallation"),true)) return;
  if (tblCodes.GetRows() <= 0) { appMessage(_T("No data has been configured for the selected use and installation.")); return; }

  // Add values to the combos
  CStringArray *pcsaData = NULL;
  for (int iCtr = 0; iCtr < tblCodes.GetRows(); iCtr++)
  {
    pcsaData = tblCodes.GetRowAt(iCtr);
    
    m_csaCodes.Add(pcsaData->GetAt(0));
    m_csaDescs.Add(pcsaData->GetAt(1));
    m_csaNames.Add(pcsaData->GetAt(2));

    if (m_cbCode.FindStringExact(-1, pcsaData->GetAt(0)) == -1) m_cbCode.AddString(pcsaData->GetAt(0)); 
    if (m_cbDesc.FindStringExact(-1, pcsaData->GetAt(1)) == -1) m_cbDesc.AddString(pcsaData->GetAt(1)); 
    if (m_cbName.FindStringExact(-1, pcsaData->GetAt(2)) == -1) m_cbName.AddString(pcsaData->GetAt(2)); 
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnSelectCode
// Description  : Called when the user changes the selection in the "Code" combo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnSelectCode()
{
  // Get the selected code
  int iIndex = m_cbCode.GetCurSel();
  if (iIndex == CB_ERR) return;
  m_cbCode.GetLBText(iIndex, m_csCode);

  // Auto-select the appropriate description and name
  for (int iCtr = 0; iCtr < m_csaCodes.GetSize(); iCtr++)
  {
    if (m_csaCodes.GetAt(iCtr) == m_csCode)
    {
      m_cbDesc.SelectString(-1, m_csaDescs.GetAt(iCtr));
      m_cbName.SelectString(-1, m_csaNames.GetAt(iCtr));
      break;
    }
  }

  // Enable the "Apply" button
  m_btnApply.EnableWindow(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnSelectDesc
// Description  : Called when the user changes the selection in the "Description" combo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnSelectDesc()
{
  // Get the selected description
  int iIndex = m_cbDesc.GetCurSel();
  if (iIndex == CB_ERR) return;
  m_cbDesc.GetLBText(iIndex, m_csDesc);

  // Auto-select the appropriate code and name
  for (int iCtr = 0; iCtr < m_csaDescs.GetSize(); iCtr++)
  {
    if (m_csaDescs.GetAt(iCtr) == m_csDesc)
    {
      m_cbCode.SelectString(-1, m_csaCodes.GetAt(iCtr));
      m_cbName.SelectString(-1, m_csaNames.GetAt(iCtr));
      break;
    }
  }

  // Enable the "Apply" button
  m_btnApply.EnableWindow(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnSelectName
// Description  : Called when the user changes the selection in the "Name" combo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnSelectName()
{
  // Get the selected name
  int iIndex = m_cbName.GetCurSel();
  if (iIndex == CB_ERR) return;
  m_cbName.GetLBText(iIndex, m_csName);

  // Reset the code and description 
  m_cbCode.ResetContent();
  m_cbDesc.ResetContent();

  // Populate with list of matching codes and descriptions
  for (int iCtr = 0; iCtr < m_csaNames.GetSize(); iCtr++)
  {
    if (m_csaNames.GetAt(iCtr) == m_csName)
    {
      m_cbCode.AddString(m_csaCodes.GetAt(iCtr));
      m_cbDesc.AddString(m_csaDescs.GetAt(iCtr));
    }
  }

  // If there is only one entry, auto-select it
  if (m_cbCode.GetCount() == 1) m_cbCode.SetCurSel(0);
  if (m_cbDesc.GetCount() == 1) m_cbDesc.SetCurSel(0);

  // Enable the "Apply" button, if there is only one code
  m_btnApply.EnableWindow(m_cbCode.GetCount() == 1 ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnBnClickedReset
// Description  : Called when the user changes the selection in the "Reset" combo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnBnClickedReset()
{
  // Reset all combo selections
  if (m_cbUse.GetCount()  > 1) m_cbUse.SetCurSel(-1);
  if (m_cbIns.GetCount()  > 1) m_cbIns.SetCurSel(-1);
  m_cbName.SetCurSel(-1);
  m_cbCode.SetCurSel(-1);
  m_cbDesc.SetCurSel(-1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnPropertyChanged
// Description  : Called when the user changes the value of a property (WARNING: This is called twice for some unknown reason)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnPropertyChanged(LPCTSTR PropertyName, const VARIANT FAR& NewValue) 
{
  CString csFunc = _T("OnPropertyChanged"), csMAtt = _T("Mandatory attributes"), csNMAtt = _T("Other attributes");

  // Get the selected property name and value
  CString csProp  = PropertyName;
  CString csValue = (LPCTSTR)(_bstr_t)_variant_t(NewValue);

  //////////////////////////////////////////////////////////////////////////
  // THIS CONDITION IS NO LONGER VALID, SINCE "Layer" HAS BEEN REMOVED
  //////////////////////////////////////////////////////////////////////////
  // If we are Linear Objects and the property is "Layer"
  if ((m_iObjectType == 2) && (csProp.CompareNoCase(_T("Layer")) == 0))
  {
    // Reset the property list and the value arrays
    m_plList.Clear(); 
    m_csaPropTags.RemoveAll();
    m_csaPropNames.RemoveAll();
    m_csaPropValues.RemoveAll();
    m_csaPropMand.RemoveAll();
    m_csaPropUnique.RemoveAll();

    // Disable the "Apply" button
    m_btnApply.EnableWindow(FALSE);

    // Add the layers to the combo
    int iValue = 0;
    BSTR bsValues[100];
    CString csLayer;
    COleSafeArray osaValues;
    for (iValue = 0; iValue < m_csaSelLayers.GetSize(); iValue++) bsValues[iValue] = ::SysAllocString(m_csaSelLayers.GetAt(iValue));
    osaValues.CreateOneDim(VT_BSTR, iValue, bsValues);
    m_plList.AddProperty(_T("General"), _T("Layer"), osaValues, _T(""), 0, true);

    // Set the value back to the combo
    m_plList.SetValue(_T("Layer"), COleVariant(csValue));

    // If the value is "*Varies*", return
    if (csValue == g_csVaries) return;

    // Select the distinct layers from the database
    CString csSQL;
    CQueryTbl tblStd;
    csSQL.Format(_T("SELECT DISTINCT [Layer] FROM tblLinearFeatures ORDER BY [Layer]"));
    if (!tblStd.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, csFunc),true) return;
    CStringArray csaDBlayers; tblStd.GetColumnAt(0, csaDBlayers);

    // For each value
    CString csPattern;
    for (int iCtr = 0; iCtr < csaDBlayers.GetSize(); iCtr++)
    {
      // If the selected layer matches the pattern
      csPattern.Format(_T("%s*"), csaDBlayers.GetAt(iCtr));
      if (WildMatch(csPattern, csValue) == true)
      {
        // Retrieve the properties to be displayed for the selected layer
        csSQL.Format(_T("SELECT [Field], [Mandatory] FROM tblLinearFeatures WHERE [Layer] = '%s' ORDER BY [Sequence]"), csaDBlayers.GetAt(iCtr));
        if (!tblStd.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, csFunc),true) return;
        CStringArray csaFields; tblStd.GetColumnAt(0, csaFields);
        CStringArray csaMandat; tblStd.GetColumnAt(1, csaMandat);

        // For each field
        CString csPropValue, csPropField;
        for (int iField = 0; iField < csaFields.GetSize(); iField++)
        {
          // Get the value for this property from the selected objects
          csPropField = csaFields.GetAt(iField);
          GetLinearPropertyValue(csPropField, csPropValue);

          // Add the property to the array
          m_csaPropTags.Add(_T("***"));
          m_csaPropNames.Add(csPropField);
          m_csaPropValues.Add(csPropValue);
          m_csaPropMand.Add(csaMandat.GetAt(iField));
          m_csaPropUnique.Add(_T("0"));

          // Get the data set for this property
          csSQL.Format(_T("SELECT [Data] FROM tblLinearData WHERE [Layer] = '%s' AND [Field] = '%s' ORDER BY [Sequence]"), csaDBlayers.GetAt(iCtr), csPropField);
          if (!tblStd.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, csFunc,true)) return;
          if (tblStd.GetRows() <= 0)
          {
            // Create this as a edit box property and set its value
            m_plList.AddProperty((csaMandat.GetAt(iField) == _T("1") ? csMAtt : csNMAtt), csPropField, COleVariant(_T("")), _T(""), 0, true);
            m_plList.SetValue(csPropField, COleVariant(csPropValue));
          }
          // Otherwise create it as a combo box
          else
          {
            int iValue = 0;
            COleSafeArray osaValues;
            BSTR bsValues[100];
            for (iValue = 0; iValue < tblStd.GetRows(); iValue++) bsValues[iValue] = ::SysAllocString(tblStd.GetRowAt(iValue)->GetAt(0));
            if (csPropValue == g_csVaries) bsValues[iValue++] = ::SysAllocString(g_csVaries);
            osaValues.CreateOneDim(VT_BSTR, iValue, bsValues);
            m_plList.AddProperty((csaMandat.GetAt(iField) == _T("1") ? csMAtt : csNMAtt), csPropField, osaValues, _T(""), -1, true);
            m_plList.SetValue(csPropField, COleVariant(csPropValue));
          }
        }
      }
    }
  }
  // If we are Linear Objects and the property is "Use (Voltage)"
  else if ((m_iObjectType == 2) && (csProp.CompareNoCase(_T("Use (Voltage)")) == 0))
  {
    // Add the layers to the combo
    int iValue = 0;
    BSTR bsValues[100];
    CString csLayer;
    COleSafeArray osaValues;
    for (iValue = 0; iValue < m_csaSelLayers.GetSize(); iValue++) bsValues[iValue] = ::SysAllocString(m_csaSelLayers.GetAt(iValue));
    osaValues.CreateOneDim(VT_BSTR, iValue, bsValues);
    m_plList.AddProperty(_T("General"), _T("Use (Voltage)"), osaValues, _T(""), 0, true);

    // Set the value back to the combo
    m_plList.SetValue(_T("Layer"), COleVariant(csValue));

    // Get the applicable installation from the 
  }
  // Otherwise
  else
  {
    // Enable the "Apply" button
    m_btnApply.EnableWindow(TRUE);

    // Update the corresponding value for this property in the array
    int iIndex = 0;
    if (CheckForDuplication(csProp, m_csaPropNames, iIndex) == TRUE) m_csaPropValues.SetAt(iIndex, csValue);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnBnClickedApply
// Description  : Called when the user clicks on the "Apply" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnBnClickedApply()
{
  // For "Blocks"
  if (m_iObjectType == 1)
  {
    ads_name enObject;
    AcDbObjectId objRef, objAtt;
    AcDbAttribute *pAtt;
    AcDbBlockReference *pRef;
    AcDbObjectIterator *pIter;
    struct resbuf *rbpXData = NULL;

    // For each selected handle
    for (int iCtr = 0; iCtr < m_csaSelHandles.GetSize(); iCtr++)
    {
      // Get the entity name
      if (acdbHandEnt(m_csaSelHandles.GetAt(iCtr), enObject) != RTNORM) continue;

      // Open the block reference and get its attribute iterator
      if (acdbGetObjectId(objRef, enObject) != Acad::eOk) continue;
      if (acdbOpenObject(pRef, objRef, AcDb::kForRead) != Acad::eOk) continue;
      pIter = pRef->attributeIterator();
      pRef->close();

      // For each property in the array
      for (int iProp = 0; iProp < m_csaPropNames.GetSize(); iProp++)
      {
        // If this isn't "*Varies*"
        if (m_csaPropValues.GetAt(iProp) != g_csVaries)
        {
          // If this is a mandatory attribute and it is not filled and not unique
          if ((m_csaPropMand.GetAt(iProp) == _T("1")) && m_csaPropValues.GetAt(iProp).IsEmpty())
          {
            displayMessage(_T("Mandatory property \"%s\" is not filled."), m_csaPropNames.GetAt(iProp));
            return;
          }

          ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          // Code commented 14.06.2011, since xdata should not be used for blocks
          //
          // Build and add the xdata to the entity
          // rbpXData = acutBuildList(AcDb::kDxfRegAppName, m_csaPropNames.GetAt(iProp), AcDb::kDxfXdAsciiString, m_csaPropValues.GetAt(iProp), NULL);
          // addXDataToEntity(enObject, rbpXData);
          // acutRelRb(rbpXData);
          //
          ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

          // Loop through the iterator and find this property
          for (pIter->start(); !pIter->done(); pIter->step())
          {
            objAtt = pIter->objectId();
            if (acdbOpenObject(pAtt, objAtt, AcDb::kForWrite) != Acad::eOk) continue;
            acutPrintf(_T("\nComparing %s with %s..."), pAtt->tag(), m_csaPropTags.GetAt(iProp));
            if (m_csaPropTags.GetAt(iProp).CompareNoCase(pAtt->tag()) == 0) 
            {
              acutPrintf(_T(" updated as %s.\n"), m_csaPropValues.GetAt(iProp));
              pAtt->setTextString(m_csaPropValues.GetAt(iProp));
              pAtt->close();
              break;
            }
            pAtt->close();
          }
        }
      }

      // Delete the iterator
      delete pIter;
    }
  }
  // For "Linear objects"
  else if (m_iObjectType == 2)
  {
    CString csEntType, csLayer, csAnnoLayer, csTextHandle, csJustify = _T("BL");
    double dTextAng = 0.0;
    ads_name enObject, enText;
    AcGePoint3d geText;
    AcDbObjectId objId;
    struct resbuf *rbpXData = NULL, *rbpTextXD = NULL;

    // Get the selected values
    UpdateData(TRUE);
    if (m_csUse.IsEmpty())  { ShowBalloon(_T("Use must be selected."),          this, IDC_LINEAR_USE);  return; }
    if (m_csIns.IsEmpty())  { ShowBalloon(_T("Installation must be selected."), this, IDC_LINEAR_INS);  return; }
    if (m_csName.IsEmpty()) { ShowBalloon(_T("Name must be selected."),         this, IDC_LINEAR_NAME); return; }
    if (m_csDesc.IsEmpty()) { ShowBalloon(_T("Description must be selected."),  this, IDC_LINEAR_DESC); return; }
    if (m_csCode.IsEmpty()) { ShowBalloon(_T("Code must be selected."),         this, IDC_LINEAR_CODE); return; }

    // Set the handles for the selected use and installation
    SetLinearObjectHandles(m_csUse, m_csIns);

    // Keep the xdata ready
    rbpXData = acutBuildList(AcDb::kDxfRegAppName, _T("CODE"), AcDb::kDxfXdAsciiString, m_csCode, NULL);

    // For each handle in the array
    for (int iCtr = 0; iCtr < m_csaSelLinearHandles.GetSize(); iCtr++)
    {
      // Get the entity name, its object ID and the layer
      if (acdbHandEnt(m_csaSelLinearHandles.GetAt(iCtr), enObject) != RTNORM) continue;
      if (acdbGetObjectId(objId, enObject) != Acad::eOk) continue;
      csLayer = Assoc(acdbEntGet(enObject), 8)->resval.rstring;

      // Add the xdata to the entity
      addXDataToEntity(enObject, rbpXData);

      // See if a text was previously attached to this object
      if (rbpTextXD = getXDataFromEntity(enObject, _T("DESC_TEXT")))
      {
        // Erase the text, if it still exists in the drawing
        if (acdbHandEnt(rbpTextXD->rbnext->resval.rstring, enText) == RTNORM) 
        {
          AcDbObjectId objText; 
          if (acdbGetObjectId(objText, enText) == Acad::eOk)
          {
            AcDbEntity *pDescText;
            if (acdbOpenObject(pDescText, objText, AcDb::kForWrite) == Acad::eOk)
            {
              pDescText->erase();
              pDescText->close();
            }
          }
        }

        acutRelRb(rbpTextXD);
      }

      // If we have to draw the text
      if ((m_bDrawDesc == TRUE) || (m_bDrawName == TRUE))
      {
        // Based on the entity type, calculate the text location and angle
        csEntType = Assoc(acdbEntGet(enObject), 0)->resval.rstring;

        // If it is a line, calculate the text point based on the angle of the line
        if (csEntType == _T("LINE"))
        {
          AcDbLine *pLine;
          if (acdbOpenObject(pLine, objId, AcDb::kForRead) != Acad::eOk) continue;
          AcGePoint3d geFrom = pLine->startPoint();
          AcGePoint3d geTo   = pLine->endPoint();
          pLine->close();

          double dLineAng = RTD(acutAngle(asDblArray(geFrom), asDblArray(geTo)));
          if ((dLineAng >=   0.0) && (dLineAng <  90.0)) { geText = geFrom; csJustify = _T("BL"); dTextAng = acutAngle(asDblArray(geFrom), asDblArray(geTo));   }
          if ((dLineAng >=  90.0) && (dLineAng < 180.0)) { geText = geFrom; csJustify = _T("BR"); dTextAng = acutAngle(asDblArray(geTo),   asDblArray(geFrom)); }
          if ((dLineAng >= 180.0) && (dLineAng < 270.0)) { geText = geFrom; csJustify = _T("BR"); dTextAng = acutAngle(asDblArray(geTo),   asDblArray(geFrom)); }
          if ((dLineAng >= 270.0) && (dLineAng < 360.0)) { geText = geFrom; csJustify = _T("BL"); dTextAng = acutAngle(asDblArray(geFrom), asDblArray(geTo));   }
        }
        // If it is an arc, get the start point
        else if (csEntType == _T("ARC"))
        {
          AcDbArc *pArc;
          if (acdbOpenObject(pArc, objId, AcDb::kForRead) != Acad::eOk) continue;
          AcGePoint3d geCen = pArc->center();
          double dRadius    = pArc->radius();
          double dStartAng  = pArc->startAngle();
          pArc->close();

          acutPolar(asDblArray(geCen), dStartAng, dRadius, asDblArray(geText));
        }
        // If it is a lwpolyline, get the start point
        else if (csEntType == _T("LWPOLYLINE"))
        {
          AcDbPolyline *pPline;
          if (acdbOpenObject(pPline, objId, AcDb::kForRead) != Acad::eOk) continue;
          AcGePoint2d geFirst;  pPline->getPointAt(0, geFirst);
          AcGePoint2d geSecond; pPline->getPointAt(1, geSecond);
          pPline->close();

          geText.x = geFirst.x;
          geText.y = geFirst.y;
          dTextAng = acutAngle(asDblArray(geFirst), asDblArray(geSecond));
        }
        // If it is an old style polyline, get the start point
        else if (csEntType == _T("POLYLINE"))
        {
          AcDb2dPolyline *pPline2d;
          if (acdbOpenObject(pPline2d, objId, AcDb::kForRead) != Acad::eOk) continue;
          AcDbObjectIterator *pIter = pPline2d->vertexIterator();
          pPline2d->close();

          AcDb2dVertex *pVert;
          AcGePoint3d geVert, geVert1, geVert2;
          for (int iCtr = 0; iCtr < 2; iCtr++, pIter->step())
          {
            if (acdbOpenObject(pVert, pIter->objectId(), AcDb::kForRead) != Acad::eOk) continue;
            geVert = pVert->position();
            pVert->close();

            if (iCtr == 0) geVert1 = geVert;
            else geVert2 = geVert;
          }

          delete pIter;

          geText.x = geVert1.x;
          geText.y = geVert1.y;
          dTextAng = acutAngle(asDblArray(geVert1), asDblArray(geVert2));
        }
        // If it is a SPLINE
        else if (csEntType == _T("SPLINE"))
        {
          AcDbSpline *pSpline;
          if (acdbOpenObject(pSpline, objId, AcDb::kForRead) != Acad::eOk) continue;
          AcGePoint3d geFirst;  pSpline->getControlPointAt(0, geFirst);
          AcGePoint3d geSecond; pSpline->getControlPointAt(1, geSecond);

          geText.x = geFirst.x;
          geText.y = geFirst.y;
          dTextAng = acutAngle(asDblArray(geFirst), asDblArray(geSecond));
        }
        // In any other case
        else continue;

        // Raise the text point a little
        acutPolar(asDblArray(geText), PIby2 + dTextAng, 0.25, asDblArray(geText));  

        // If the angle is between 90 & 270 degrees
        if ((csEntType != _T("LINE")) && ((RTD(dTextAng) > 90.0) && (RTD(dTextAng) < 270.0)))
        {
          // Adjust the angle
          dTextAng -= PI;

          // Right-justify the text
          csJustify = _T("BR");
        }

        // Adjust the point
        acutPolar(asDblArray(geText), dTextAng + PIby2, 0.5, asDblArray(geText));

        // Get the corresponding "ANNO" layer and check if it is present (for ex., "SV_OH_EXIST" should have "SV_OH_ANNO_EXIST")
        csAnnoLayer.Format(_T("%s_ANNO_%s"), csLayer.Mid(0, csLayer.ReverseFind(_T('_'))), csLayer.Mid(csLayer.ReverseFind(_T('_')) + 1));
        if (acdbTblSearch(_T("LAYER"), csAnnoLayer, 0) == NULL) 
        {
          acutPrintf(_T("Layer %s not found. Text will be placed on same layer.\n"), csAnnoLayer);
          csAnnoLayer = csLayer;
        }

        // Ensure that the "ISO" text style exists
        Acad::ErrorStatus es;
        AcDbTextStyleTable *pTxtTable; 
        AcDbTextStyleTableRecord *pTxtRec; 
        acdbHostApplicationServices()->workingDatabase()->getTextStyleTable(pTxtTable, AcDb::kForRead);
        es = pTxtTable->getAt(_T("ISO"), pTxtRec, AcDb::kForWrite);
        pTxtTable->close();
        if (es == Acad::eOk)
        {
          // Set its height to 0.0
          double dPrevSize = pTxtRec->textSize();
          pTxtRec->setTextSize(0.0);
          pTxtRec->close();

          // Draw the required text at the calculated location and move it to the entity's layer
          if (m_bDrawDesc) m_csWriteText = m_csDesc;
          if (m_bDrawName) m_csWriteText = m_csName;
          acedCommandS(RTSTR, _T(".TEXT"), RTSTR, _T("S"), RTSTR, _T("ARIAL"), RTSTR, _T("J"), RTSTR, csJustify, RTPOINT, asDblArray(geText), RTREAL, RTD(dTextAng), RTSTR, m_csWriteText, NULL);
          acdbEntLast(enText);
          acedCommandS(RTSTR, _T(".CHPROP"), RTENAME, enText, RTSTR, _T(""), RTSTR, _T("LA"), RTSTR, csAnnoLayer, RTSTR, _T(""), NULL);

          // Get the handle of the text and attach it to the entity
          csTextHandle = Assoc(acdbEntGet(enText), 5)->resval.rstring;
          rbpTextXD = acutBuildList(AcDb::kDxfRegAppName, _T("DESC_TEXT"), AcDb::kDxfXdHandle, csTextHandle, NULL);
          addXDataToEntity(enObject, rbpTextXD);
          acutRelRb(rbpTextXD);

          // Re-open the text style and reset its height
          pTxtTable->getAt(_T("ISO"), pTxtRec, AcDb::kForWrite);
          pTxtRec->setTextSize(dPrevSize);
          pTxtRec->close();
        }
      }
    }

    // Release the xdata buffer
    acutRelRb(rbpXData);
  }

  // Disable the apply button
  m_btnApply.EnableWindow(FALSE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnBnClickedRemove
// Description  : Called when the user clicks on the "Remove" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnBnClickedRemove()
{
  // Get a confirmation from the user
  CString csConfirm = _T("Do you want to clear values already assigned to the object?");
  if (m_csaSelHandles.GetSize() > 1) csConfirm = _T("Warning! Multiple objects are selected.\n\nDo you want to clear values assigned to all objects?");
  if (getConfirmation(csConfirm + _T("\nSelect Yes to remove all existing values.\nSelect No to return without clearing.")) == IDNO) return;

  // For "Blocks"
  if (m_iObjectType == 1)
  {
    ads_name enObject;
    AcDbObjectId objRef, objAtt;
    AcDbAttribute *pAtt;
    AcDbBlockReference *pRef;
    AcDbObjectIterator *pIter;
    struct resbuf *rbpXData = NULL;

    // For each selected handle
    for (int iCtr = 0; iCtr < m_csaSelHandles.GetSize(); iCtr++)
    {
      // Get the entity name
      if (acdbHandEnt(m_csaSelHandles.GetAt(iCtr), enObject) != RTNORM) continue;

      // For each property in the array
      for (int iProp = 0; iProp < m_csaPropNames.GetSize(); iProp++)
      {
        // Remove the xdata to the entity
        rbpXData = acutBuildList(AcDb::kDxfRegAppName, m_csaPropNames.GetAt(iProp), NULL);
        addXDataToEntity(enObject, rbpXData);
        acutRelRb(rbpXData);
      }
    }

    // Call OnSelectObjectType to reset the properties list
    OnSelectObjectType();
  }
  // For "Linear objects"
  if (m_iObjectType == 2)
  {
    CString csEntType, csLayer, csTextHandle;
    double dTextAng = 0.0;
    ads_name enObject, enText;
    AcGePoint3d geText;
    AcDbObjectId objId;
    struct resbuf *rbpXData = NULL, *rbpTextXDR = NULL, *rbpTextXD = NULL;

    // Set the handles for the selected use and installation
    SetLinearObjectHandles(m_csUse, m_csIns);

    // Keep the empty xdata ready
    rbpXData   = acutBuildList(AcDb::kDxfRegAppName, _T("CODE"), NULL);
    rbpTextXDR = acutBuildList(AcDb::kDxfRegAppName, _T("DESC_TEXT"), NULL);

    // For each handle in the array
    for (int iCtr = 0; iCtr < m_csaSelLinearHandles.GetSize(); iCtr++)
    {
      // Get the entity name, its object ID and the layer
      if (acdbHandEnt(m_csaSelLinearHandles.GetAt(iCtr), enObject) != RTNORM) continue;
      if (acdbGetObjectId(objId, enObject) != Acad::eOk) continue;

      // Add the xdata to the entity (this will actually remove it)
      addXDataToEntity(enObject, rbpXData);

      // See if a text was previously attached to this object
      if (rbpTextXD = getXDataFromEntity(enObject, _T("DESC_TEXT")))
      {
        // Erase the text, if it still exists in the drawing
        if (acdbHandEnt(rbpTextXD->rbnext->resval.rstring, enText) == RTNORM) 
        {
          AcDbObjectId objText; 
          if (acdbGetObjectId(objText, enText) == Acad::eOk)
          {
            AcDbEntity *pDescText;
            if (acdbOpenObject(pDescText, objText, AcDb::kForWrite) == Acad::eOk)
            {
              pDescText->erase();
              pDescText->close();
            }
          }
        }

        acutRelRb(rbpTextXD);
        addXDataToEntity(enObject, rbpTextXDR);
      }
    }

    // Release the buffers
    acutRelRb(rbpXData);
    acutRelRb(rbpTextXDR);

    // Call "Reset" to reset the combo boxes
    OnBnClickedReset();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnCheckDrawDesc
// Description  : Called when the user clicks on the "Draw description" check box
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnCheckDrawDesc()
{
  // Get the status of the description check box
  m_bDrawDesc = m_btnDrawDesc.GetCheck();

  // If checked, reset the name check box
  if (m_bDrawDesc == TRUE) { m_bDrawName = FALSE; m_btnDrawName.SetCheck(FALSE); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnCheckDrawName
// Description  : Called when the user clicks on the "Draw name" check box
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnCheckDrawName()
{
  // Get the status of the name check box
  m_bDrawName = m_btnDrawName.GetCheck();

  // If checked, reset the description check box
  if (m_bDrawName == TRUE) { m_bDrawDesc = FALSE; m_btnDrawDesc.SetCheck(FALSE); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnBnClickedSelect
// Description  : Called when the user clicks on the "Select" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnBnClickedSelect()
{
  // Close the dialog with OK
  CDialog::OnOK();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnCheckDrawName
// Description  : Called when the user clicks on the "Regenerate" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnBnClickedRegen()
{
  // Regenerate the drawing
  ads_regen();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CPropertiesDlg::OnBnClickedHelp
// Description  : Called when the user clicks on the "Help..." check box
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnBnClickedHelp()
{
  // Display the appropriate topic
  displayHelp((DWORD)_T("Attach_Attributes.htm"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_Properties
// Called from  : User, via the "EA_PROPERTIES" command
// Description  : Shows the properties of the selected objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_Properties()
{
	//long lPicked = 0L; 
	int lPicked = 0L;
	ads_name ssPickFirst;

	// Check if there are objects already selected
	if (acedSSGet(_T("I"), NULL, NULL, NULL, ssPickFirst) != RTNORM)
	{
		// If not, ask for selection
		if (acedSSGet(NULL, NULL, NULL, NULL, ssPickFirst) != RTNORM) return;
	}

	// Get the number of objects selected
	if ((acedSSLength(ssPickFirst, &lPicked) != RTNORM) || (lPicked == 0L)) return;

	// Get their handles
	ads_name enPicked;
	CString csHandle;
	struct resbuf *rbpPicked = NULL;
	CStringArray csaPickedHandles;
	for (long lCtr = 0L; lCtr < lPicked; lCtr++)
	{
		acedSSName(ssPickFirst, lCtr, enPicked);
		rbpPicked = acdbEntGet(enPicked);
		if (rbpPicked)
		{
			csHandle = Assoc(rbpPicked, 5)->resval.rstring;
			csaPickedHandles.Add(csHandle);
		}
	}

	// Free the selection set
	acedSSFree(ssPickFirst);

	// Switch "CMDECHO" off
	struct resbuf rbCmdEcho;
	rbCmdEcho.restype = RTSHORT;
	rbCmdEcho.resval.rint = 0;
	acedSetVar(_T("CMDECHO"), &rbCmdEcho);

	// Retrieve the "draw text" value from the local registry
	CWinApp *pApp = AfxGetApp();
	CString csRegSection = _T("eCapture\\Settings\\Properties");
	if (!pApp) { appError(__FILE__, _T("Command_Properties"), __LINE__, _T("Unable to retrieve application pointer.")); return; }
	CString csDrawDesc = pApp->GetProfileString(csRegSection, _T("DrawDesc"));
	CString csDrawName = pApp->GetProfileString(csRegSection, _T("DrawName"));
	if (csDrawDesc.IsEmpty()) csDrawDesc = _T("0");
	if (csDrawName.IsEmpty()) csDrawName = _T("0");

	// Display the properties dialog
	CPropertiesDlg dlgProps;
	dlgProps.m_bDrawDesc = (csDrawDesc == _T("1") ? TRUE : FALSE);
	dlgProps.m_bDrawName = (csDrawName == _T("1") ? TRUE : FALSE);
	dlgProps.m_csaPickedHandles.Copy(csaPickedHandles);

	// Put this in a loop, so that the user can exit only with Cancel
	
	while (dlgProps.DoModal() == IDOK)
	{
		// Code modified 16.06.2011, further to mail dated 02.06.2011
		// Clear the picked handles array, since we must now replace the selection set
		dlgProps.m_csaPickedHandles.RemoveAll();
		// End code modified 16.06.2011

		// We must add to the existing array
		//long lAdd = 0L;
		int lAdd = 0L;
		ads_name enAdd, ssAdd;
		if (acedSSGet(NULL, NULL, NULL, NULL, ssAdd) == RTNORM)
		{
			if ((acedSSLength(ssAdd, &lAdd) == RTNORM) && (lAdd > 0L))
			{
				for (long lCtr = 0L; lCtr < lAdd; lCtr++)
				{
					acedSSName(ssAdd, lCtr, enAdd);
					rbpPicked = acdbEntGet(enAdd);
					if (rbpPicked)
					{
						csHandle = Assoc(rbpPicked, 5)->resval.rstring;
						if (CheckForDuplication(dlgProps.m_csaPickedHandles, csHandle) == FALSE)
						{
							dlgProps.m_csaPickedHandles.Add(csHandle);
						}
					}
				}

				// Clear the additional set
				acedSSFree(ssAdd);
			}
		}
	}

  // Store the "draw text" value in the local registry
  csDrawDesc.Format(_T("%d"), dlgProps.m_bDrawDesc);
  csDrawName.Format(_T("%d"), dlgProps.m_bDrawName);
  pApp->WriteProfileString(csRegSection, _T("DrawDesc"), csDrawDesc);
  pApp->WriteProfileString(csRegSection, _T("DrawName"), csDrawName);
}
