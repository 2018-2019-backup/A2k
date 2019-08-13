// ConfirmUpdateDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Resource.h"
#include "ConfirmUpdateDlg.h"

// CConfirmUpdateDlg dialog
IMPLEMENT_DYNAMIC(CConfirmUpdateDlg, CDialog)

CConfirmUpdateDlg::CConfirmUpdateDlg(CWnd* pParent /*=NULL*/)	: CDialog(CConfirmUpdateDlg::IDD, pParent)
{
  m_bRemember = FALSE;
  m_csRegSection = _T("NETCAD\\Settings\\Download Updates");
}

CConfirmUpdateDlg::~CConfirmUpdateDlg()
{
}

void CConfirmUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_REMEMBER, m_bRemember);
}

// CConfirmUpdateDlg message handlers
BEGIN_MESSAGE_MAP(CConfirmUpdateDlg, CDialog)
  ON_BN_CLICKED(IDOK, &CConfirmUpdateDlg::OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL, &CConfirmUpdateDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


BOOL CConfirmUpdateDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Get the data in the registry and update it back to the dialog
  CWinApp *pApp = AfxGetApp();
  if (!pApp) { appError(__FILE__, _T("OnInitDialog"), __LINE__, _T("Unable to retrieve application pointer.")); OnCancel(); return TRUE; }
  m_bRemember = _tstoi(pApp->GetProfileString(m_csRegSection, _T("Update"), _T("0")));

  // If the value is TRUE, then we need not show this dialog
  if (m_bRemember == TRUE) OnOK();

  // Success
  return TRUE;
}

void CConfirmUpdateDlg::OnBnClickedOk()
{
  // Update
  UpdateData();

  // If we must remember this choice
  if (m_bRemember == TRUE)
  {
    // Store the data in the registry
    CWinApp *pApp = AfxGetApp();
    if (!pApp) { appError(__FILE__, _T("OnBnClickedOk"), __LINE__, _T("Unable to retrieve application pointer.")); return; }
    pApp->WriteProfileString(m_csRegSection, _T("Update"), _T("1"));
  }

  // Close the dialog
  OnOK();
}


void CConfirmUpdateDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
