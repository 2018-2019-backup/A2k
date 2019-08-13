////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : ExtValidateDlg.cpp
// Created          : 19th February 2008
// Created by       : S. Jaisimha
// Description      : Implementation for the external validation dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "ValidateExtOptionsDlg.h"


// CValidateExtOptionsDlg dialog
IMPLEMENT_DYNAMIC(CValidateExtOptionsDlg, CDialog)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateExtOptionsDlg::CValidateExtOptionsDlg
// Description  : Default constructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CValidateExtOptionsDlg::CValidateExtOptionsDlg(CWnd* pParent /*=NULL*/)	: CDialog(CValidateExtOptionsDlg::IDD, pParent)
{
  m_bBlocks     = true;
  m_bLayers     = true;
  m_bTextStyles = true;
  m_bLineTypes  = true;
  m_bDimStyles  = true;
  m_csESPName   = _T("");
  m_iBlockCheckType = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateExtOptionsDlg::~CValidateExtOptionsDlg
// Description  : Default destructor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CValidateExtOptionsDlg::~CValidateExtOptionsDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateExtOptionsDlg::DoDataExchange
// Description  : Dialog to variable data transfer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateExtOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX,  IDC_ESPNAME,        m_cbESPName);
  DDX_CBString(pDX, IDC_ESPNAME,        m_csESPName);
  DDX_Control(pDX,  IDC_EXT_BLOCKS,     m_btnBlocks);
  DDX_Check(pDX,    IDC_EXT_BLOCKS,     m_bBlocks);
  DDX_Check(pDX,    IDC_EXT_LAYERS,     m_bLayers);
  DDX_Check(pDX,    IDC_EXT_TEXTSTYLES, m_bTextStyles);
  DDX_Check(pDX,    IDC_EXT_LINETYPES,  m_bLineTypes);
  DDX_Check(pDX,    IDC_EXT_DIMSTYLES,  m_bDimStyles);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CValidateExtOptionsDlg, CDialog)
  ON_BN_CLICKED(IDC_EXT_BLOCKS,         &CValidateExtOptionsDlg::OnBnClickedExtBlocks)
  ON_BN_CLICKED(IDC_EXT_BLOCKS_RENAME,  &CValidateExtOptionsDlg::OnBnClickedExtBlocksRename)
  ON_BN_CLICKED(IDC_EXT_BLOCKS_REPLACE, &CValidateExtOptionsDlg::OnBnClickedExtBlocksReplace)
  ON_BN_CLICKED(IDOK,                   &CValidateExtOptionsDlg::OnBnClickedOk)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateExtOptionsDlg::OnInitDialog
// Description  : Called before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CValidateExtOptionsDlg::OnInitDialog()
{
  // Call the parent implementation
  CDialog::OnInitDialog();

  // Set the default radio button
  CheckRadioButton(IDC_EXT_BLOCKS_RENAME, IDC_EXT_BLOCKS_REPLACE, IDC_EXT_BLOCKS_RENAME);

  // Load the configured external service providers into the combo
  CString csSQL, csESPName, csESPTable;
  CQueryTbl tblStd;
  csSQL = _T("SELECT [ESPName], [ESPTable] FROM tblESPNames ORDER BY [ESPName]");
  if (!tblStd.SqlRead(DSN_ECapture_Ext, csSQL, __LINE__, __FILE__, _T("OnInitDialog"),true)) { OnCancel(); return TRUE; }
  if (tblStd.GetRows() <= 0) { appMessage(_T("There are no configured External Service Providers.\nPlease contact your Systems Administrator.")); OnCancel(); return TRUE; }
  for (int iCtr = 0; iCtr < tblStd.GetRows(); iCtr++) 
  { 
    csESPName  = tblStd.GetRowAt(iCtr)->GetAt(0);
    csESPTable = tblStd.GetRowAt(iCtr)->GetAt(1);
    m_cbESPName.AddString(csESPName);
    m_csaESPTables.Add(csESPTable);
  }

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateExtOptionsDlg::OnBnClickedExtBlocks
// Description  : Called when the user clicks on the "Validate blocks" check box
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateExtOptionsDlg::OnBnClickedExtBlocks()
{
  // Get the selection and set the radio button status
  m_bBlocks = m_btnBlocks.GetCheck();
  GetDlgItem(IDC_EXT_BLOCKS_RENAME)->EnableWindow(m_bBlocks);
  GetDlgItem(IDC_EXT_BLOCKS_REPLACE)->EnableWindow(m_bBlocks);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateExtOptionsDlg::OnBnClickedExtBlocksRename
// Description  : Called when the user clicks on the "Rename blocks" radio button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateExtOptionsDlg::OnBnClickedExtBlocksRename()
{
  m_iBlockCheckType = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateExtOptionsDlg::OnBnClickedExtBlocksReplace
// Description  : Called when the user clicks on the "Replace blocks" radio button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateExtOptionsDlg::OnBnClickedExtBlocksReplace()
{
  m_iBlockCheckType = 2;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CValidateExtOptionsDlg::OnBnClickedOk
// Description  : Called when the user clicks on the "Validate" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CValidateExtOptionsDlg::OnBnClickedOk()
{
  // Get the data
  UpdateData();

  // If the ESP is not selected, say so
  if (m_csESPName.IsEmpty()) { ShowBalloon(_T("Select the External Service Provider."), this, IDC_ESPNAME); return; }

  // If all the options are off
  if (!m_bBlocks && !m_bLayers && !m_bTextStyles && !m_bLineTypes && !m_bDimStyles) { ShowBalloon(_T("At least one option must be on."), this, IDC_EXT_BLOCKS); return; }

  // Store the table associated with this ESP
  m_csESPTable = m_csaESPTables.GetAt(m_cbESPName.GetCurSel());

  // Close the dialog
  OnOK();
}
