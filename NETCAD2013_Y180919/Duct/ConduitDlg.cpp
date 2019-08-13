// ConduitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConduitDlg.h"


// CConduitDlg dialog

IMPLEMENT_DYNAMIC(CConduitDlg, CDialog)

CConduitDlg::CConduitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConduitDlg::IDD, pParent)
{
}

CConduitDlg::~CConduitDlg()
{
}

void CConduitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_CONDUIT,				m_csConduit);
	DDX_CBString(pDX, IDC_CABLE,					m_csCable);
	DDX_Control(pDX,	IDC_CONDUIT,				m_cbConduit);
	DDX_Control(pDX,	IDC_CABLE,					m_cbCable);
	DDX_CBString(pDX, IDC_TRENCH_STATUS,  m_csTrenchStatus);
	DDX_Control(pDX,	IDC_TRENCH_STATUS,	m_cbTrenchStatus);
	DDX_CBString(pDX, IDC_CABLESTATUS,    m_csCableStatus);
	DDX_Control(pDX,  IDC_CABLESTATUS,    m_cbCableStatus);
}

BEGIN_MESSAGE_MAP(CConduitDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CConduitDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_CABLE, &CConduitDlg::OnCbnSelchangeCable)
	ON_BN_CLICKED(IDHELP, &CConduitDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


// CConduitDlg message handlers

BOOL CConduitDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the trench status
	m_cbTrenchStatus.SelectString(-1, m_csTrenchStatus);

	// Populate the Cable status combo and set the default value
	CQueryTbl tblDuctStatus;
	CString csSQL;

	if (!m_csTrenchStatus.CompareNoCase(L"Existing"))
	{
		m_cbCableStatus.EnableWindow(TRUE);

		// If the conduit size is known and is None, the Cable Status cannot be changed
		if (!m_csConduit.CompareNoCase(L"None")) m_cbCableStatus.EnableWindow(FALSE);
	}
	else 
	{
		m_cbCableStatus.SelectString(-1, L"Proposed");
		m_cbCableStatus.EnableWindow(FALSE);
	}
	if (!m_csCableStatus.IsEmpty()) m_cbCableStatus.SelectString(-1, m_csCableStatus);

	// Populate the conduit sizes combo based on the trench status
	CQueryTbl tblConduitSize;
	// csSQL.Format(L"SELECT fldDuctConduitSize, Default FROM tblDuctConduitSize ORDER BY fldSequence");
	csSQL.Format(L"SELECT fldConduitSize, Default FROM tblDuctConduit WHERE fldStatus = '%s' ORDER BY fldSequence", m_csTrenchStatus);
	if (!tblConduitSize.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return TRUE;
	for (int i = 0; i < tblConduitSize.GetRows(); i++)
	{
		m_cbConduit.AddString(tblConduitSize.GetRowAt(i)->GetAt(0));
		if (_tstoi(tblConduitSize.GetRowAt(i)->GetAt(1))) m_cbConduit.SelectString(-1, tblConduitSize.GetRowAt(i)->GetAt(0));
	}

	if (!m_csConduit.IsEmpty()) m_cbConduit.SelectString(-1, m_csConduit);
	

	// Populate the conduit types based on the status
	// None;HV;LV;SL;SV;AUX;TR132;TR66;TR33;HV Core;HV Trip;LV Quad;TR132 Trip;TR66 Trip;TR33 Trip;TR132 Core;TR66 Core;TR33 Core;
	CQueryTbl tblDuctTable;
	csSQL.Format(L"SELECT fldCableType FROM tblDuctCable WHERE fldStatus = '%s' ORDER BY fldSequence", m_csTrenchStatus);
	if (!tblDuctTable.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return TRUE;
	for (int iC = 0; iC < tblDuctTable.GetRows(); iC++) m_cbCable.AddString(tblDuctTable.GetRowAt(iC)->GetAt(0));

	if (!m_csCable.IsEmpty()) 	{ int iSel = m_cbCable.FindStringExact(-1, m_csCable); m_cbCable.SetCurSel(iSel); }

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
// Function name: CConduitDlg::OnCbnSelchangeCable()
// Description  : 
//////////////////////////////////////////////////////////////////////////
void CConduitDlg::OnCbnSelchangeCable()
{
	// Get the cable specified
	int iSel = m_cbCable.GetCurSel();
	if (iSel == CB_ERR) return;
	m_cbCable.GetLBText(iSel, m_csCable);

	// If the cable selected is not "NONE", then we will have to enable specifying the status of this cable, provided the trench status is "Existing"
	if (!m_csTrenchStatus.CompareNoCase(L"Existing") && m_csCable.CompareNoCase(L"None")) m_cbCableStatus.EnableWindow(TRUE); else	m_cbCableStatus.EnableWindow(FALSE);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CConduitDlg::OnBnClickedHelp()
// Description  : Calls the help window with the context displayed
//////////////////////////////////////////////////////////////////////////
void CConduitDlg::OnBnClickedHelp() { displayHelp((DWORD)_T("Conduit_data_dialog_box.htm")); }

//////////////////////////////////////////////////////////////////////////
// Function name: 
// Description  :
//////////////////////////////////////////////////////////////////////////
void CConduitDlg::OnBnClickedOk() { UpdateData(); OnOK(); }





