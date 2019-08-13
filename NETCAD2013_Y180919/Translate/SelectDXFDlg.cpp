////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : SelectDXFDlg.cpp
// Created          : 18th January 2008
// Created by       : S. Jaisimha
// Description      : Customized CFileDialog with an additional combo box to specify the input DXF scale
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "SelectDXFDlg.h"
#include "SelectTemplateDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implemented in ReadINI.cpp
extern BOOL UpdateINIFile(CString csSetting, CString csValue);

// Static members initialization
WNDPROC CSelectDXFDlg::m_wndProc;
CSelectDXFDlg *CSelectDXFDlg::m_pThis = NULL;

// CSelectDXFDlg
IMPLEMENT_DYNAMIC(CSelectDXFDlg, CFileDialog)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::CSelectDXFDlg
// Description  : Default constructor, simply passes the values to the underlying CFileDialog constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSelectDXFDlg::CSelectDXFDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName, DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd, BOOL bVistaStyle) 
             : CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, bVistaStyle)
{
  m_ofn.lpstrTitle = _T("NET CAD: Translate V2.0");

  m_pThis = this;

  // Set this flag to force usage of old style MFC CFileDialog, instead of the new Vista style dialog
  // This variable is a protected member in CFileDialog and will take effect only on Vista/7
  m_bVistaStyle = FALSE;

  // Set the flags
  m_bIntoCurrent = FALSE;
  m_bUseTemplate = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::~CSelectDXFDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSelectDXFDlg::~CSelectDXFDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSelectDXFDlg, CFileDialog)
  ON_WM_DESTROY()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::OnInitDialog
// Description  : Called by the MFC framework before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSelectDXFDlg::OnInitDialog()
{
  // Call the parent class implementation
  CFileDialog::OnInitDialog();

  // We need to enlarge standard CFileDialog to make space for the combo
  // Get a pointer to the original dialog box
  CWnd *wndDlg = GetParent();
  RECT Rect; wndDlg->GetWindowRect(&Rect);

  // Change the size of FileOpen dialog
  wndDlg->SetWindowPos(NULL, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top + 215, SWP_NOMOVE);

  // Standard CFileDialog control ID's are defined in dlgs.h 
  // Do not forget to include it in implementation file
  // cmb1 - standard file name combo box control
  CWnd *wndComboCtrl = wndDlg->GetDlgItem(cmb1);
  wndComboCtrl->GetWindowRect(&Rect);
  wndDlg->ScreenToClient(&Rect); // Remember it is a child control

  // Put our control(s) somewhere below HIDDEN check box and make space for 10 different scales
  Rect.top    += 30;
  Rect.bottom += 300;

  // Create the CComboBox object
  // IMPORTANT: We must put wndDlg here as hWndParent, not "this" as written in the Microsoft documentation example
  m_cbScale.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP, Rect, wndDlg, IDC_DXF_SCALE_COMBO);
  m_cbScale.SetFont(wndComboCtrl->GetFont(), TRUE);

  // We also need Static Control. Get coordinates from stc2 control
  CWnd *wndStaticCtrl = wndDlg->GetDlgItem(stc2);
  wndStaticCtrl->GetWindowRect(&Rect);
  wndDlg->ScreenToClient(&Rect);
  Rect.top    += 30;
  Rect.bottom += 80;

  // Create the static control
  wchar_t szText[120], szDefault[120];
  _stprintf(szText, _T("Import scale:"));
  m_stScale.Create(_T("Scale"), WS_CHILD | WS_VISIBLE, Rect, wndDlg, IDC_DXF_SCALE_STATIC);
  m_stScale.SetFont(wndComboCtrl->GetFont(), TRUE);
  wndDlg->SetDlgItemText(IDC_DXF_SCALE_STATIC, szText);

  // Create the "Use template" check box and edit control
  Rect.top += 20;
  m_btnUseTemplate.Create(_T("Use template:"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, Rect, wndDlg, IDC_DXF_USETEMPLATE);
  m_btnUseTemplate.SetFont(wndComboCtrl->GetFont(), TRUE);
  m_btnUseTemplate.SetCheck(TRUE);
  m_btnUseTemplate.EnableWindow(FALSE);

  Rect.top += 15;
  Rect.left += 94;
  Rect.right += 260;
  Rect.bottom += 30;
  _stprintf(szText, g_csLocalDWT);
  m_edTemplate2.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL, Rect, wndDlg, IDC_DXF_TEMPLATE_USED);
  m_edTemplate2.SetWindowText(g_csLocalDWT);
  m_edTemplate2.SetFont(wndComboCtrl->GetFont(), TRUE);

  // Create the "Translate into" check box and edit control
  Rect.left -= 94;
  Rect.right -= 260;
  Rect.top += 40;
  Rect.bottom += 55;
  m_btnImportInto.Create(_T("Translate into:"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, Rect, wndDlg, IDC_DXF_IMPORTINTO);
  m_btnImportInto.SetFont(wndComboCtrl->GetFont(), TRUE);

  Rect.left += 94;
  Rect.right += 260;
  Rect.top += 30;
  Rect.bottom += 20;
  struct resbuf rbDwgPrefix, rbDwgName;
  acedGetVar(_T("DwgPrefix"), &rbDwgPrefix);
  acedGetVar(_T("DwgName"), &rbDwgName);
  CString csImportInto; csImportInto.Format(_T("%s%s"), rbDwgPrefix.resval.rstring, rbDwgName.resval.rstring);
  m_edImportInto.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL, Rect, wndDlg, IDC_DXF_TEMPLATE_USED);
  m_edImportInto.SetWindowText(csImportInto);
  m_edImportInto.SetFont(wndComboCtrl->GetFont(), TRUE);

  // Create the "Remove duplicates" check box
  Rect.left -= 94;
  Rect.right -= 240;
  Rect.top += 40;
  Rect.bottom += 45;
  m_btnRunOverkill.Create(_T("Remove duplicates"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, Rect, wndDlg, IDC_DXF_RUNOVERKILL);
  m_btnRunOverkill.SetFont(wndComboCtrl->GetFont(), TRUE);

  // Get the "Cancel" button
  CWnd *wndCancel = wndDlg->GetDlgItem(IDCANCEL);
  if (wndCancel)
  {
    RECT rcCancel, rcHelp;
    wndCancel->GetWindowRect(&rcCancel);
    wndDlg->ScreenToClient(&rcCancel);
    rcHelp.left   = rcCancel.left;
    rcHelp.top    = rcCancel.bottom + 7;
    rcHelp.right  = rcCancel.right;
    rcHelp.bottom = rcHelp.top + 23;

    // Create the "Help..." button next to the combo box
    m_btnHelp.Create(_T("Help..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rcHelp, wndDlg, IDC_DXF_HELP);
    m_btnHelp.SetFont(wndComboCtrl->GetFont(), TRUE);

    // Create the "Template..." button below the Help button
    rcHelp.top += 35;
    rcHelp.bottom += 35;
    m_btnTemplate.Create(_T("Template..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, rcHelp, wndDlg, IDC_DXF_TEMPLATE);
    m_btnTemplate.SetFont(wndComboCtrl->GetFont(), TRUE);

    // Set the new window message handler
#ifdef WIN64
    m_wndProc = (WNDPROC)SetWindowLongPtr(wndDlg->GetSafeHwnd(), GWLP_WNDPROC, (LONG_PTR)CSelectDXFDlg::WindowProcNew);
#else
    m_wndProc = (WNDPROC)SetWindowLong(wndDlg->GetSafeHwnd(), GWL_WNDPROC, (LONG)CSelectDXFDlg::WindowProcNew);
#endif
  }

  // Read the scale data
  CString csSQL;
  CQueryTbl tblECap;
  csSQL.Format(_T("SELECT [Scale], [Default] FROM tblScales ORDER BY [Scale]"));
  if (!tblECap.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, _T("OnInitDialog"),true)) { OnCancel(); return TRUE; }
  if (tblECap.GetRows() <= 0) { appMessage(_T("DXF import scales have not been defined.\nPlease contact your System Administrator.")); OnCancel(); return TRUE; }

  // Add the available scales to the combo box
  for (int iCtr = 0; iCtr < tblECap.GetRows(); iCtr++)
  {
    _stprintf(szText, _T("1 : %s"), suppressZero(_tstof(tblECap.GetRowAt(iCtr)->GetAt(0))));
    wndDlg->SendDlgItemMessage(IDC_DXF_SCALE_COMBO, CB_INSERTSTRING, (WPARAM)(-1), (LPARAM)(szText));

    // If this is the default scale, store it
    if (tblECap.GetRowAt(iCtr)->GetAt(1) == _T("1")) wcscpy(szDefault, szText);
  }

  // Set default value
  wndDlg->SendDlgItemMessage(IDC_DXF_SCALE_COMBO, CB_SELECTSTRING, (WPARAM)(-1), (LPARAM)(szDefault));

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::WindowProcNew
// Description  : This replaces the default CallWindowProc message handler for the dialog. This is implemented since ON_BN_CLICKED message
//                will not be triggered for dialogs derived without a template from CFileDialog. 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CSelectDXFDlg::WindowProcNew(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  // Depending on the message
  switch(message)
  {
    // If this is a command message
    case WM_COMMAND:
    {
      /**/ if ((LONG)LOWORD(wParam) == IDC_DXF_HELP)        { m_pThis->OnBnClickedHelp();        return 0; } // If it was the "Help..." button
      else if ((LONG)LOWORD(wParam) == IDC_DXF_TEMPLATE)    { m_pThis->OnBnClickedTemplate();    return 0; } // If it was the "Template..." button
      else if ((LONG)LOWORD(wParam) == IDC_DXF_IMPORTINTO)  { m_pThis->OnBnClickedImportInto();  return 0; } // If it was the "Import into" check box
      else if ((LONG)LOWORD(wParam) == IDC_DXF_USETEMPLATE) { m_pThis->OnBnClickedUseTemplate(); return 0; } // If it was the "Use template" check box
      else if ((LONG)LOWORD(wParam) == IDC_DXF_RUNOVERKILL) { m_pThis->OnBnClickedRunOverkill(); return 0; } // If it was the "Remove duplicates" check box
    }
  }

  // In any other case, do the default
  return CallWindowProc(CSelectDXFDlg::m_wndProc, hwnd, message, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::OnBnClickedHelp
// Description  : Called when the user clicks on the "Help..." button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSelectDXFDlg::OnBnClickedHelp()
{
  // Display the associated help topic
  displayHelp((DWORD)_T("Translator.htm"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::OnBnClickedTemplate
// Description  : Called when the user clicks on the "Template..." button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSelectDXFDlg::OnBnClickedTemplate()
{
  // Display the template selection dialog
  CSelectTemplateDlg dlgST;
  dlgST.m_csStdPath = g_csStdTemplates;
  if (dlgST.DoModal() == IDCANCEL) return;

  // Copy the selected template name
  m_csTemplate = dlgST.m_csTemplate;

  // If the template does not have a "\" in it, prefix it with the standard path
  if (m_csTemplate.Find(_T('\\')) == -1) m_csTemplate = g_csStdTemplates + _T("\\") + m_csTemplate;

  // Update the template
  m_edTemplate2.SetWindowText(m_csTemplate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::OnBnClickedImportInto
// Description  : Called when the user clicks on the "Import into" check box
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSelectDXFDlg::OnBnClickedImportInto()
{
  // Get the check box state
  m_bIntoCurrent = m_btnImportInto.GetCheck();

  // Set the "Template" state
  m_btnTemplate.EnableWindow(TRUE);
  m_btnUseTemplate.SetCheck(TRUE);
  m_btnUseTemplate.EnableWindow(m_bIntoCurrent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::OnBnClickedUseTemplate
// Description  : Called when the user clicks on the "Use template" check box
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSelectDXFDlg::OnBnClickedUseTemplate()
{
  // Get the check box state
  m_bUseTemplate = m_btnUseTemplate.GetCheck();

  // Set the "Template" state
  m_btnTemplate.EnableWindow(m_bUseTemplate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::OnBnClickedRunOverkill
// Description  : Called when the user clicks on the "Remove duplicates" check box
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSelectDXFDlg::OnBnClickedRunOverkill()
{
  // Get the check box state
  m_bRunOverkill = m_btnRunOverkill.GetCheck();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CSelectDXFDlg::OnFileNameOK
// Description  : Called when the user clicks on the "Open" button (or double-clicks a file name in the list)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSelectDXFDlg::OnFileNameOK()
{
  // Get the file name
  CString csSelFileName = GetPathName();
  CString csSelFileExt  = csSelFileName.Right(4);

  // Check if the file name is OK and valid (in case the user types the name instead of picking it)
  if ((csSelFileExt.CompareNoCase(_T(".dxf")) != 0) && (csSelFileExt.CompareNoCase(_T(".dwg")) != 0)) { appMessage(_T("Please select a valid DXF or DWG to translate.")); return TRUE; }
  if (_taccess(csSelFileName, 00) == -1) { appMessage(_T("Unable to open selected file for reading.")); return TRUE; }

  // Check if this DXF has already been translated
  if (csSelFileExt.CompareNoCase(_T(".dxf")) == 0)
  {
    // Check if the selected DXF file size is greater than the allowed size
    CFileStatus fsDXF; CFile::GetStatus(csSelFileName, fsDXF);
    int iDXFSizeInMB = int(fsDXF.m_size / (1024 * 1024));
    if (iDXFSizeInMB > _tstoi(g_csMaxFileSize))
    {
      appMessage(GetFileName() + _T("\n\nFile size exceeds memory capacity."), MB_ICONINFORMATION);
      return TRUE; 
    }

    CString csCorDwg = csSelFileName; 
    csCorDwg.Replace(csCorDwg.Right(3), _T("dwg"));
    if (_taccess(csCorDwg, 00) != -1)
    {
      // Check if the drawing is open (using _taccess(csCorDwg, 02) does not work for some strange reason)
      bool bOpen = false;
      AcDbDatabase *pCorDb = new AcDbDatabase;
	  //Commented for ACAD 2018
      //if (pCorDb->readDwgFile(csCorDwg, _SH_DENYWR) != Acad::eOk) bOpen = true;
	  if (pCorDb->readDwgFile(csCorDwg, AcDbDatabase::OpenMode::kForReadAndReadShare) != Acad::eOk) bOpen = true;
      delete pCorDb;
      if (bOpen == true)
      {
        appMessage(_T("The selected DXF has already been translated\nand the drawing is open in the AutoCAD editor.\n\nPlease close the drawing and try again."));
        return TRUE;
      }
      // Otherwise, get a confirmation
      else if (getConfirmation(_T("That DXF already has a corresponding translated drawing.\nDo you want to overwrite the existing translation?")) == IDNO)
      {
        // Return without closing the dialog
        return TRUE;
      }
    }
  }

  // Get the selected template and verify if it is valid
  if (m_csTemplate.IsEmpty()) 
  { 
    appMessage(_T("A template must be selected."), MB_ICONINFORMATION);
    return TRUE; 
  }

  // Get the selected scale and verify if it is valid
  GetParent()->GetDlgItemText(IDC_DXF_SCALE_COMBO, m_csScale);
  if (m_csScale.IsEmpty()) 
  { 
    appMessage(_T("An import scale must be selected."), MB_ICONINFORMATION);
    return TRUE; 
  }

  // Remove the "1:" from the string
  m_csScale.Replace(_T("1 : "), _T(""));

  // Get the conversion factor from the table
  CString csSQL;
  CQueryTbl tblECap;
  csSQL.Format(_T("SELECT [Conversion] FROM tblScales WHERE [Scale] = %s"), m_csScale);
  if (!tblECap.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, _T("OnFileNameOK"),true)) return TRUE;
  if (tblECap.GetRows() <= 0) { appMessage(_T("Unable to retrieve conversion factor.\nPlease contact your System Administrator.")); return TRUE; }
  m_csConversion = tblECap.GetRowAt(0)->GetAt(0);

  // If the template is not the same as the one in Settings.ini
  if (m_csTemplate != g_csLocalDWT)
  {
    // Ask if user wants to update the setting
    if (getConfirmation(_T("The selected template is not the same as the one specified in Settings.ini for the LOCAL_STANDARD_DRAWING.\n\nDo you want to update the Settings.ini file to use the selected template in future?")) == IDYES)
    {
      // Update the INI file
      UpdateINIFile(_T("LOCAL_STANDARD_DRAWING"), m_csTemplate);
    }
  }

  // Close the dialog
  return CFileDialog::OnFileNameOK();
}
