// BOMNotifyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BOMNotifyDlg.h"


// CBOMNotifyDlg dialog

IMPLEMENT_DYNAMIC(CBOMNotifyDlg, CDialog)

CBOMNotifyDlg::CBOMNotifyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBOMNotifyDlg::IDD, pParent)
{
	m_iCurrentLayout = -1;
}

CBOMNotifyDlg::~CBOMNotifyDlg()
{
}

void CBOMNotifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOM_SELECTALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_BOM_CLEARALL, m_btnClearAll);
	DDX_Control(pDX, IDC_BOM_SELECT, m_btnSelect);
	DDX_Control(pDX, IDC_BOM_CLEAR, m_btnClear);
	DDX_Control(pDX, IDC_ONSCREEN, m_btnOnScreen);
	DDX_Control(pDX, IDC_LISTLAYOUT, m_lcLayout);
}


BEGIN_MESSAGE_MAP(CBOMNotifyDlg, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_LISTLAYOUT, &CBOMNotifyDlg::OnNMClickListlayout)
	ON_BN_CLICKED(IDCANCEL, &CBOMNotifyDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CBOMNotifyDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BOM_SELECTALL, &CBOMNotifyDlg::OnBnClickedSelectAll)
	ON_BN_CLICKED(IDC_BOM_CLEARALL, &CBOMNotifyDlg::OnBnClickedClearAll)
	ON_BN_CLICKED(IDC_BOM_SELECT, &CBOMNotifyDlg::OnBnClickedSelect)
	ON_BN_CLICKED(IDC_BOM_CLEAR, &CBOMNotifyDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_BOM_ONSCREEN, &CBOMNotifyDlg::OnBnClickedOnScreen)
	ON_BN_CLICKED(IDHELP, &CBOMNotifyDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


// CBOMNotifyDlg message handlers
/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
BOOL CBOMNotifyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// Define columns for the Layout list
	m_lcLayout.InsertColumn(0, L"Layout",					LVCFMT_LEFT, 150);
	m_lcLayout.InsertColumn(1, L"Viewports",      LVCFMT_LEFT, 120);
	m_lcLayout.InsertColumn(2, L"",								LVCFMT_LEFT, 0);
	m_lcLayout.InsertColumn(3, L"",								LVCFMT_LEFT, 0);
	m_lcLayout.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// Add the array information to the list control
	CString csVPort;
	for (int iCtr = 0; iCtr < m_csaLayouts.GetSize(); iCtr++)
	{
		m_lcLayout.InsertItem(iCtr,			m_csaLayouts.GetAt(iCtr));
		m_lcLayout.SetItemText(iCtr, 2, m_csaVports.GetAt(iCtr));
		
		// Set the no. of VPORTS in the previously selected
		csVPort = m_csaVports.GetAt(iCtr);
		int iCnt = 0;
		
		while (csVPort.Find(L";") != -1) 
		{
			iCnt++; 
			csVPort = csVPort.Mid(csVPort.Find(L";") + 1); 
		}
		csVPort.Format(L"%d of %s selected", iCnt, m_csaVportsCnt.GetAt(iCtr));

		m_lcLayout.SetItemText(iCtr, 1, csVPort);
		m_lcLayout.SetItemText(iCtr, 3, m_csaVportsCnt.GetAt(iCtr));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
void CBOMNotifyDlg::OnNMClickListlayout(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	int iSelLayout = m_lcLayout.GetSelectionMark();
	if (iSelLayout == LB_ERR) return;
		
	m_iCurrentLayout = iSelLayout;

	if (_tstoi(m_lcLayout.GetItemText(iSelLayout, 3)) > 0)
	{
		m_btnOnScreen.ShowWindow(SW_NORMAL);
		m_btnSelect.ShowWindow(SW_NORMAL);
		m_btnClear.ShowWindow(SW_NORMAL);

		m_btnSelectAll.ShowWindow(SW_HIDE);
		m_btnClearAll.ShowWindow(SW_HIDE);
	}
	else
	{
		m_btnOnScreen.ShowWindow(SW_HIDE);
		m_btnSelect.ShowWindow(SW_HIDE);
		m_btnClear.ShowWindow(SW_HIDE);

		m_btnSelectAll.ShowWindow(SW_NORMAL);
		m_btnClearAll.ShowWindow(SW_NORMAL);
	}

	// Display the selected layout
	CString csLayout = m_lcLayout.GetItemText(m_iCurrentLayout, 0);

	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
	//Commented for ACAD 2018
	//AcDbLayout *pLayout = pLayoutMngr->findLayoutNamed(csLayout, false);
	AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(csLayout);
	AcDbLayout *pLayout = NULL;
	acdbOpenObject(pLayout, objLayoutId, AcDb::kForRead);

	if (pLayout)
	{
		pLayoutMngr->setCurrentLayout(csLayout);
		pLayout->close();

		// Zoom big
		acedCommandS(RTSTR, L".ZOOM", RTSTR, L"E", NULL);
	}

	// Highlight the selected viewport
	Highlight(m_iCurrentLayout, true);
	*pResult = 0;
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
void CBOMNotifyDlg::OnBnClickedCancel() { OnCancel(); }

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
void CBOMNotifyDlg::OnBnClickedOk() 
{
	int iSel = m_lcLayout.GetSelectionMark();
	Highlight(iSel, false);

	// Check if at least one viewport in a layout is selected
	m_iCurrentLayout = -1;
	
	CString csSelTxt;
	for (int i = 0; i < m_lcLayout.GetItemCount(); i++)
	{
		if (m_lcLayout.GetItemText(i, 1)[0] != '0')
		{
			m_bSeparateLineAndBlock = IsDlgButtonChecked(IDC_SEPARATELINESBLOCKS);

			OnOK();
			return;
		}
	}

	appMessage(L"At least one viewport must be selected to generate legend.", 00);

	OnOK(); 
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::SelecThisViewport()
// Description  : Select all view ports from all the layouts
//////////////////////////////////////////////////////////////////////////
void CBOMNotifyDlg::SelecThisViewport(int iSel)
{
	// Get the name of the selected viewport and set it as current
	CString csLayout = m_lcLayout.GetItemText(iSel, 0);
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
	//Commented for ACAD 2018
	//AcDbLayout *pLayout = pLayoutMngr->findLayoutNamed(csLayout, false);
	AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(csLayout);
	AcDbLayout *pLayout = NULL;
	acdbOpenObject(pLayout, objLayoutId, AcDb::kForRead);

	CString csSelTxt; 
	if (pLayout)
	{
		Acad::ErrorStatus es = pLayoutMngr->setCurrentLayout(csLayout);
		// pLayout->close();
	}
	else
	{
		csSelTxt.Format(L"0 of %s selected", m_csaVportsCnt.GetAt(iSel));
		m_lcLayout.SetItemText(iSel, 1, csSelTxt);
		return;
	}

	// Get the view ports placed in this layout
	AcDbObjectIdArray geObjIdArray = pLayout->getViewportArray();
	pLayout->close();

	// If there are no valid view ports, then message the user
	/**/ if (geObjIdArray.length() == 0) 
	{
		csSelTxt.Format(L"0 of %s selected", m_csaVportsCnt.GetAt(iSel));
		m_lcLayout.SetItemText(iSel, 1, csSelTxt); 
		m_lcLayout.SetItemText(iSel, 2, L"");
		return; 
	}

	AcDbObjectId objIdVPort;
	AcDbViewport *pViewPort;
	Acad::ErrorStatus es;
	ads_name enVPort;
	struct resbuf *rbpGet;
	CStringArray csaHandles;
	for (int iCtr = geObjIdArray.length() - 1; iCtr >= 0; iCtr--)
	{
		// Get the viewport object id
		AcDbObjectId objIdVPort = geObjIdArray.at(iCtr);

		// Get the location and the size of the viewport
		es = acdbOpenObject(pViewPort, objIdVPort, AcDb::kForRead);
		if (es != Acad::eOk) continue;

		// Check if this viewport is visible
		bool bIsOn  = pViewPort->isOn();
		int iNumber = pViewPort->number();
		pViewPort->close();

		// If the viewport is not ON or when PAPERSPACE default skip it
		if (!bIsOn || (iNumber == 1)) continue;

		// Get the viewport handle and add it to the array
		acdbGetAdsName(enVPort, objIdVPort);
		rbpGet = acdbEntGet(enVPort);
		if (rbpGet)
		{
			csaHandles.Add(Assoc(rbpGet, 5)->resval.rstring);
			acutRelRb(rbpGet);
		}
	}

	// Add all the handles collected into one string
	CString csHandle; 
	for (int iVPort = 0; iVPort < csaHandles.GetSize(); iVPort++) csHandle.Format(L"%s;%s", csHandle, csaHandles.GetAt(iVPort));

	// Set the resultant handle to the array
	CString csValue; csValue.Format(L"%d of %s selected", csaHandles.GetSize(), m_csaVportsCnt.GetAt(iSel));
	m_lcLayout.SetItemText(iSel, 1, csValue);
	m_lcLayout.SetItemText(iSel, 2, csHandle);

	// Set the viewports to the selected handles
	m_csaVports.SetAt(iSel, csHandle);
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
void CBOMNotifyDlg::Highlight(int iSel, bool bHighlight)
{
	// Highlight the selected vports
	CString csVPort = m_lcLayout.GetItemText(iSel, 2);

	// Get the object ID of frozen layers in the VPORT selected
	CStringArray csaHandles;
	CString csHandle;
	while (csVPort.Find(L";") != -1) 
	{
		csVPort  = csVPort.Mid(csVPort.Find(L";") + 1); 
		csHandle = csVPort;
		if (csHandle.Find(L";") != -1) { csHandle = csHandle.Mid(0, csHandle.Find(L";")); }
		csaHandles.Add(csHandle);
	}

	ads_name enHandle;
	for (int i = 0; i < csaHandles.GetSize(); i++)
	{
		// Get the view port entity name for this handle
		acdbHandEnt(csaHandles.GetAt(i), enHandle);
		acedRedraw(enHandle, (bHighlight ? 3 : 4));
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: CBOMNotifyDlg::OnBnClickedClear()
// Description  : De-selects all view ports in the selected layout
//////////////////////////////////////////////////////////////////////////
void CBOMNotifyDlg::OnBnClickedClear()
{
	// Get selected index
	int iSel = m_lcLayout.GetSelectionMark();
	
	// If it has no view ports, exit the function
	if (m_lcLayout.GetItemText(iSel, 1)[0] == '0') return;

	CString csSelTxt; csSelTxt.Format(L"0 of %s selected", m_csaVportsCnt.GetAt(iSel));
	m_lcLayout.SetItemText(iSel, 1, csSelTxt);

	m_lcLayout.SetItemText(iSel, 2, L"");

	// Set the viewports to ""
	m_csaVports.SetAt(iSel, L"");

	// To remove the highlights
	acedCommandS(RTSTR, L".REGEN", NULL);
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
void CBOMNotifyDlg::OnBnClickedSelect()
{
	// Call the function to select the view ports
	int iSel = m_lcLayout.GetSelectionMark();
	if (iSel == LB_ERR) 
	{
		appMessage(L"Select a layout to select all viewports in it.", 00);
		return;
	}

	// Call the function to select the view ports
	SelecThisViewport(iSel);

	m_lcLayout.SetSelectionMark(iSel);
	Highlight(iSel, true);
	Highlight(iSel, true);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CBOMNotifyDlg::OnBnClickedClearall()
// Description  : De-selects all view ports in all layouts
//////////////////////////////////////////////////////////////////////////
void CBOMNotifyDlg::OnBnClickedClearAll()
{
	for (int iVPort = 0; iVPort < m_lcLayout.GetItemCount(); iVPort++)
	{
		// If it has no view ports, exit the function
		if (m_lcLayout.GetItemText(iVPort, 1)[0] == '0') continue;

		CString csSelTxt; csSelTxt.Format(L"0 of %s selected", m_csaVportsCnt.GetAt(iVPort));
		m_lcLayout.SetItemText(iVPort, 1, csSelTxt);

		m_lcLayout.SetItemText(iVPort, 2, L"");

		// Set the viewports to ""
		m_csaVports.SetAt(iVPort, L"");
	}

	// To remove the highlights
	acedCommandS(RTSTR, L".REGEN", NULL);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CBOMNotifyDlg::OnBnClickedSelectAll()
// Description  : Selects all view ports in all layouts
//////////////////////////////////////////////////////////////////////////
void CBOMNotifyDlg::OnBnClickedSelectAll()
{
	// Call the function to select the view ports of every layout
	for (int iVPort = 0; iVPort < m_lcLayout.GetItemCount(); iVPort++) { SelecThisViewport(iVPort); }
}

void CBOMNotifyDlg::OnBnClickedHelp() { displayHelp((DWORD)_T("Bill_Of_Materials.htm")); }

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnBnClickedOnscreen()
// Description  : Sets the layout selected as current
//////////////////////////////////////////////////////////////////////////
void CBOMNotifyDlg::OnBnClickedOnScreen()
{
	// If no entry in the list is selected
	int iSel = m_lcLayout.GetSelectionMark();
	if (iSel == CB_ERR) return;

	// Display the selected layout
	m_csLayout = m_lcLayout.GetItemText(iSel, 0);

	m_iCurrentLayout = iSel;
	OnOK();
}


