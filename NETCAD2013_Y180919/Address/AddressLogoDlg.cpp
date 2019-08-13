////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : NETCAD
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : AddressLogoDlg.cpp
// Created          : 27th February 2008
// Created by       : S. Jaisimha
// Description      : Implementation for the address and logo selection dialog and command
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "AddressLogoDlg.h"

// CAddressLogoDlg dialog
IMPLEMENT_DYNAMIC(CAddressLogoDlg, CDialog)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CAddressLogoDlg::CAddressLogoDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAddressLogoDlg::CAddressLogoDlg(CWnd* pParent /*=NULL*/)	: CDialog(CAddressLogoDlg::IDD, pParent)
{
  m_csDepot      = _T("");
  m_csLogo       = _T("");
  m_csAIO        = _T("");
  m_csRegSection = _T("NETCAD\\Settings\\Address and Logo");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CAddressLogoDlg::~CAddressLogoDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAddressLogoDlg::~CAddressLogoDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CAddressLogoDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAddressLogoDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX,  IDC_DEPOT,        m_cbDepot);
  DDX_CBString(pDX, IDC_DEPOT,        m_csDepot);
  DDX_Control(pDX,  IDC_LOGO,         m_cbLogo);
  DDX_CBString(pDX, IDC_LOGO,         m_csLogo);
  DDX_Text(pDX,     IDC_EDIT_ADDRESS, m_csAddress);
  DDX_Text(pDX,     IDC_EDIT_PHONE,   m_csPhone);
  DDX_Text(pDX,     IDC_EDIT_FAX,     m_csFax);
  DDX_Text(pDX,     IDC_EDIT_MOBILE,  m_csMobile);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CAddressLogoDlg, CDialog)
  ON_CBN_SELCHANGE(IDC_DEPOT, &CAddressLogoDlg::OnSelectDepot)
  ON_BN_CLICKED(IDHELP,       &CAddressLogoDlg::OnBnClickedHelp)
  ON_BN_CLICKED(IDOK,         &CAddressLogoDlg::OnBnClickedOk)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CAddressLogoDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CAddressLogoDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  //XMLParser class Test Code
 // XPathNS::XPathParser* xPath= new XPathNS::XPathParser("c:\\BOOKS.XML",true);
  //std::vector<XPathNS::XMLNode> nodeList4 = xPath->selectNodes( "//book" ); 

  // Load the depot names from the database
  CString csSQL;
  CQueryTbl tblStd;
  CStringArray *pcsaData = NULL;
  csSQL.Format(_T("SELECT [Depot], [Address1], [Address2], [Phone], [Fax], [Mobile] FROM tblAddresses ORDER BY [Depot]"));
  if (!tblStd.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, _T("OnInitDialog"),true)) { OnCancel(); return TRUE; }
  if (tblStd.GetRows() <= 0) { appMessage(_T("No locations have been configured.\nPlease contact your Systems Administrator.")); OnCancel(); return TRUE; }
  for (int iCtr = 0; iCtr < tblStd.GetRows(); iCtr++)
  {
    pcsaData = tblStd.GetRowAt(iCtr);
    m_cbDepot.AddString(pcsaData->GetAt(0));
    m_csaAddress1.Add(pcsaData->GetAt(1));
    m_csaAddress2.Add(pcsaData->GetAt(2));
    m_csaPhone.Add(pcsaData->GetAt(3));
    m_csaFax.Add(pcsaData->GetAt(4));
    m_csaMobile.Add(pcsaData->GetAt(5));
  }


  // Load the logo names from the database
  csSQL.Format(_T("SELECT [LogoName] FROM tblLogos ORDER BY [LogoName]"));
  if (!tblStd.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, _T("OnInitDialog"),true)) { OnCancel(); return TRUE; }
  if (tblStd.GetRows() <= 0) { appMessage(_T("No logo names have been configured.\nPlease contact your Systems Administrator.")); OnCancel(); return TRUE; }
  for (int iCtr = 0; iCtr < tblStd.GetRows(); iCtr++) m_cbLogo.AddString(tblStd.GetRowAt(iCtr)->GetAt(0));

  // Select the first logo by default
  m_cbLogo.SetCurSel(0);
  // Get the data in the registry and update it back to the dialog
  CWinApp *pApp = AfxGetApp();
  if (!pApp) { appError(__FILE__, _T("OnInitDialog"), __LINE__, _T("Unable to retrieve application pointer.")); OnCancel(); return TRUE; }
  m_csDepot   = pApp->GetProfileString(m_csRegSection, _T("Depot"),   _T(""));
  m_csLogo    = pApp->GetProfileString(m_csRegSection, _T("Logo"),    _T(""));
  m_csAddress = pApp->GetProfileString(m_csRegSection, _T("Address"), _T(""));
  m_csPhone   = pApp->GetProfileString(m_csRegSection, _T("Phone"),   _T(""));
  m_csFax     = pApp->GetProfileString(m_csRegSection, _T("Fax"),     _T(""));
  m_csMobile  = pApp->GetProfileString(m_csRegSection, _T("Mobile"),  _T(""));
  UpdateData(FALSE);

  m_cbDepot.SetCurSel(0);
  OnSelectDepot();
  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CAddressLogoDlg::OnSelectDepot
// Description  : Called when the user selects a depot in the combo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAddressLogoDlg::OnSelectDepot()
{
  // Get the selection
  int iIndex = m_cbDepot.GetCurSel();
  if (iIndex == CB_ERR) return;

  // Set the other data from the arrays
  CString csAddress = m_csaAddress1.GetAt(iIndex); 
  if (!m_csaAddress2.GetAt(iIndex).IsEmpty()) csAddress.Format(_T("%s\r\n%s"), m_csaAddress1.GetAt(iIndex), m_csaAddress2.GetAt(iIndex));
  SetDlgItemText(IDC_EDIT_ADDRESS, csAddress);
  SetDlgItemText(IDC_EDIT_PHONE,   m_csaPhone.GetAt(iIndex));
  SetDlgItemText(IDC_EDIT_FAX,     m_csaFax.GetAt(iIndex));
  SetDlgItemText(IDC_EDIT_MOBILE,  m_csaMobile.GetAt(iIndex));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CAddressLogoDlg::OnBnClickedHelp
// Description  : Called when the user clicks on the "Help..." button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAddressLogoDlg::OnBnClickedHelp()
{
  // Display the appropriate topic
  displayHelp((DWORD)_T("Address.htm"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CAddressLogoDlg::OnBnClickedOk
// Description  : Called when the user clicks on the "Validate" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAddressLogoDlg::OnBnClickedOk()
{
  // Get the data entered by the user
  UpdateData();

  // Ensure that minimum data is specified
  if (m_csLogo.IsEmpty())    { ShowBalloon(_T("Logo must be specified."),    this, IDC_LOGO);         return; }
  if (m_csAddress.IsEmpty()) { ShowBalloon(_T("Address must be specified."), this, IDC_EDIT_ADDRESS); return; }

  // Store the data in the registry
  CWinApp *pApp = AfxGetApp();
  if (!pApp) { appError(__FILE__, _T("OnBnClickedOk"), __LINE__, _T("Unable to retrieve application pointer.")); return; }
  pApp->WriteProfileString(m_csRegSection, _T("Depot"),   m_csDepot);
  pApp->WriteProfileString(m_csRegSection, _T("Logo"),    m_csLogo);
  pApp->WriteProfileString(m_csRegSection, _T("Address"), m_csAddress);
  pApp->WriteProfileString(m_csRegSection, _T("Phone"),   m_csPhone);
  pApp->WriteProfileString(m_csRegSection, _T("Fax"),     m_csFax);
  pApp->WriteProfileString(m_csRegSection, _T("Mobile"),  m_csMobile);

  // Close the dialog
  OnOK();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : UpdateAddressLogo
// Called from  : Command_Address
// Description  : Changes the values in the various title blocks in the layouts.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void UpdateAddressLogo()
{
  // Get the data in the registry and update it back to the dialog
  CWinApp *pApp = AfxGetApp();
  CString csRegSection = _T("NETCAD\\Settings\\Address and Logo");
  if (!pApp) { appError(__FILE__, _T("Command_ContinueDXFIn"), __LINE__, _T("Unable to retrieve application pointer.")); return; }
  CString csDepot   = pApp->GetProfileString(csRegSection, _T("Depot"),   _T(""));
  CString csDynLogo = pApp->GetProfileString(csRegSection, _T("Logo"),    _T(""));
  CString csAddress = pApp->GetProfileString(csRegSection, _T("Address"), _T(""));
  CString csPhone   = pApp->GetProfileString(csRegSection, _T("Phone"),   _T(""));
  CString csFax     = pApp->GetProfileString(csRegSection, _T("Fax"),     _T(""));
  CString csMobile  = pApp->GetProfileString(csRegSection, _T("Mobile"),  _T(""));

  // If the logo is not specified, nothing else will be available
  if (csDynLogo.IsEmpty()) return;

  // Combine all the fields into one string
  CString csTemp, csAIO;
  csAIO = csAddress;
  if (!csPhone.IsEmpty())  { csTemp.Format(_T("\r\nP: %s"), csPhone);  csAIO += csTemp; }
  if (!csFax.IsEmpty())    { csTemp.Format(_T("\r\nF: %s"), csFax);    csAIO += csTemp; }
  if (!csMobile.IsEmpty()) { csTemp.Format(_T("\r\nM: %s"), csMobile); csAIO += csTemp; }

  // Get the block table of the drawing and create a new iterator
  AcDbBlockTable *pBT; acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBT, AcDb::kForRead);
  AcDbBlockTableIterator* pIter; pBT->newIterator(pIter);
  pBT->close();

  // Variables used within the loop
  ACHAR *pszLayout, *pszBlock;
  CString csLayout, csTag, csProp, csLogo;
  AcDbEntity *pLayoutEnt;
  AcDbLayout *pLayout;
  AcDbObjectId objLayout, objLayoutEnt, objLayoutBTR, objAtt;
  AcDbAttribute *pAtt;
  AcDbEvalVariant eval;
  AcDbObjectIterator *pAttIter;
  AcDbBlockReference *pLayoutRef;
  AcDbEvalVariantArray evalArray;
  AcDbDynBlockReference *pLayoutDynRef;
  AcDbBlockTableRecord* pBTR, *pLayoutBTR, *pRefBTR; 
  AcDbBlockTableRecordIterator* pLayoutIter; 
  AcDbDynBlockReferenceProperty dbrProp;
  AcDbDynBlockReferencePropertyArray dbrPropArray;

  // Loop through the iterator
  for ( ; !pIter->done(); pIter->step())
  {
    // Get the block table record
    if (pIter->getRecord(pBTR, AcDb::kForRead) != Acad::eOk) continue;

    // If this is a layout, get its object ID
    if (pBTR->isLayout() == false) { pBTR->close(); continue; }
    objLayout = pBTR->getLayoutId();
    pBTR->close();

    // Open the layout and get its name and associated block table record
    if (acdbOpenObject(pLayout, objLayout, AcDb::kForRead) != Acad::eOk) { pBTR->close(); continue; }
    pLayout->getLayoutName(pszLayout);
    objLayoutBTR = pLayout->getBlockTableRecordId();
    pLayout->close();
    csLayout = pszLayout;

    // If this is "Model", we are not interested
    if (csLayout == _T("Model")) continue;

    // Get the block table record for this layout
    if (acdbOpenObject(pLayoutBTR, objLayoutBTR, AcDb::kForRead) != Acad::eOk) continue;
    pLayoutBTR->newIterator(pLayoutIter);
    pLayoutBTR->close();

    // Loop through all entities on the layout
    for ( ; !pLayoutIter->done(); pLayoutIter->step())
    {
      // Get the block reference pointer
      if (pLayoutIter->getEntity(pLayoutEnt, AcDb::kForRead) != Acad::eOk) continue;
      if ((pLayoutRef = AcDbBlockReference::cast(pLayoutEnt)) == NULL) { pLayoutEnt->close(); continue; }
      pAttIter = pLayoutRef->attributeIterator();
      pLayoutRef->close();

      // Loop through the attributes
      for ( ; !pAttIter->done(); pAttIter->step())
      {
        // Get the attribute tag
        objAtt = pAttIter->objectId();
        if (acdbOpenObject(pAtt, objAtt, AcDb::kForWrite) != Acad::eOk) continue;
        csTag = pAtt->tag();

        // If the attribute is "NET_ADDRESS"
        if (csTag.CompareNoCase(_T("NET_ADDRESS")) == 0) pAtt->setTextString(csAIO);
        pAtt->close();
      }

      // Delete the iterator
      delete pAttIter;

      // Check if this is a dynamic block
      pLayoutIter->getEntityId(objLayoutEnt);
      if ((pLayoutDynRef = new AcDbDynBlockReference(objLayoutEnt)) == NULL) continue;

      // Get the dynamic block property array
      dbrPropArray.removeAll();
      pLayoutDynRef->getBlockProperties(dbrPropArray);
      for (long lIndex = 0; lIndex < dbrPropArray.length(); lIndex++)
      {
        dbrProp = dbrPropArray[lIndex];
        csProp = dbrProp.propertyName().kACharPtr();
        if (csProp.CompareNoCase(_T("Visibility")) == 0)
        {
          // Get the current value
          eval = dbrProp.value();

          // If the layout is "A4", set it to the truncated version (since the A4 block does not have the "no Assoc. drawing" and "and Assoc. drawing" values
          if (csLayout.Find(_T("A4")) != -1) csLogo = csDynLogo.Mid(0, 7); else csLogo = csDynLogo;

          // Reset the value to the selected one and update the property (this will do some black magic, since we are not explicitly updating the dynamic block)
          _tcscpy(eval.resval.rstring, csLogo);
          dbrProp.setValue(eval);
        }
      }

      // Delete the dynamic reference pointer
      delete pLayoutDynRef;
    }

    // Delete the iterator
    delete pLayoutIter;
  }

  // Delete the iterator
  delete pIter;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_Address
// Called from  : User, via the "NET_ADDRESS" command
// Description  : Displays the address and logo dialog.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_Address()
{
  // Display the dialog
  CAddressLogoDlg dlgAL;
  if (dlgAL.DoModal() == IDCANCEL) return;

  // Update the address and logo info in the drawing
  acutPrintf(_T("\nUpdating address and logo information in all sheets...\n"));
  UpdateAddressLogo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_AddressNoDialog
// Called from  : User, via the "NET_ADDRESSNODIALOG" command
// Description  : Updates the address and logo in the drawing without displaying the dialog.
//                This is used in "Command_ContinueDXFIn" method in DXFIn.cpp in Translate project.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_AddressNoDialog()
{
  // Check if the call is valid
  struct resbuf rbUserI5;
  acedGetVar(_T("USERI5"), &rbUserI5);
  if (rbUserI5.resval.rint != 1423) { acutPrintf(_T("Unknown command \"NET_ADDRESSNODIALOG\".  Press F1 for help.\n")); return; }

  // Update the address and logo info in the drawing
  UpdateAddressLogo();
}
