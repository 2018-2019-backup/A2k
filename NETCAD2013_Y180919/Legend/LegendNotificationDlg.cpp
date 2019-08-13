// LegendNotificationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LegendNotificationDlg.h"

// CLegendNotificationDlg dialog
IMPLEMENT_DYNAMIC(CLegendNotificationDlg, CDialog)

CLegendNotificationDlg::CLegendNotificationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLegendNotificationDlg::IDD, pParent)
{
	m_iCurrentLayout = -1;
	m_iLegendType = 0;

	m_csaLayouts.RemoveAll();
	m_csaVports.RemoveAll();
	m_csaVportsCnt.RemoveAll();

	m_bSeparateLineAndBlock = false;
}

CLegendNotificationDlg::~CLegendNotificationDlg()
{
}

void CLegendNotificationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELECTALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_CLEARALL, m_btnClearAll);
	DDX_Control(pDX, IDC_SELECT, m_btnSelect);
	DDX_Control(pDX, IDC_CLEAR, m_btnClear);
	DDX_Control(pDX, IDC_ONSCREEN, m_btnOnScreen);
	DDX_Control(pDX, IDC_LISTLAYOUT, m_lcLayout);
}

BEGIN_MESSAGE_MAP(CLegendNotificationDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CLegendNotificationDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_SELECTALL, &CLegendNotificationDlg::OnBnClickedSelectAll)
	ON_NOTIFY(NM_CLICK, IDC_LISTLAYOUT, &CLegendNotificationDlg::OnNMClickListlayout)
	ON_BN_CLICKED(IDC_CLEAR, &CLegendNotificationDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_ONSCREEN, &CLegendNotificationDlg::OnBnClickedOnscreen)
	ON_BN_CLICKED(IDC_CLEARALL, &CLegendNotificationDlg::OnBnClickedClearall)
	ON_BN_CLICKED(IDC_SELECT, &CLegendNotificationDlg::OnBnClickedSelect)
	ON_BN_CLICKED(IDHELP, &CLegendNotificationDlg::OnBnClickedHelp)
END_MESSAGE_MAP()

// CLegendNotificationDlg message handlers
//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnInitDialog()
// Description  :
//////////////////////////////////////////////////////////////////////////
BOOL CLegendNotificationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Define columns for the Layout list
	m_lcLayout.InsertColumn(0, L"Layout",					LVCFMT_LEFT, 150);
	m_lcLayout.InsertColumn(1, L"Viewports",      LVCFMT_LEFT, 120);
	m_lcLayout.InsertColumn(2, L"",								LVCFMT_LEFT, 0);
	m_lcLayout.InsertColumn(3, L"",								LVCFMT_LEFT, 0);
	m_lcLayout.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// Set the type of legend, together or separate (for editing)
	if (m_iCalledFor == 2)
	{
		CheckDlgButton(IDC_SEPARATELINESBLOCKS, m_iLegendType); 
		GetDlgItem(IDC_SEPARATELINESBLOCKS)->EnableWindow(FALSE);

		m_bSeparateLineAndBlock = (!m_iLegendType ? false : true);
	}

	// Display the list of Layout names and its VPORT details
	CString csVPort;
	for (int iCtr = 0; iCtr < m_csaLayouts.GetSize(); iCtr++)
	{
		m_lcLayout.InsertItem(iCtr,			m_csaLayouts.GetAt(iCtr));
		m_lcLayout.SetItemText(iCtr, 2, m_csaVports.GetAt(iCtr));
		
		// Get the count of VPORTS in this layout
		int iCnt = 0;
		
		csVPort = m_csaVports.GetAt(iCtr);
		while (csVPort.Find(L";") != -1) { iCnt++; csVPort = csVPort.Mid(csVPort.Find(L";") + 1); }
		csVPort.Format(L"%d of %s selected", iCnt, m_csaVportsCnt.GetAt(iCtr));

		m_lcLayout.SetItemText(iCtr, 1, csVPort);
		m_lcLayout.SetItemText(iCtr, 3, m_csaVportsCnt.GetAt(iCtr));
	}
			
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::SelecThisViewport()
// Description  : Select all view ports from all the layouts
//////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::SelecThisViewport(int iSel)
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

	// Update the no. of VPORTS selected for this layout in the dialog
	if (geObjIdArray.length() == 0) 
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
	
	// Set the view ports to the selected handles
	m_csaVports.SetAt(iSel, csHandle);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnBnClickedSelect()
// Description  : Selects all view ports in the selected layout
//////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::OnBnClickedSelect()
{
	// Get selected index
	int iSel = m_lcLayout.GetSelectionMark();
	if (iSel == LB_ERR) { appMessage(L"Select a layout to select all view ports in it.", 00); return; }

	// If it has no view ports, exit the function
	if (_tstoi(m_lcLayout.GetItemText(iSel, 3)) <= 0) { appMessage(L"Select a layout with atleast one view port in it."); return; }
	
	// Call the function to select the view ports
	SelecThisViewport(iSel);
	m_lcLayout.SetSelectionMark(iSel);

	Highlight(iSel, true);
	Highlight(iSel, true);

	// Remove the selection and enable the "All" suffixed buttons
	EnableAllSuffixedButtons();
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnBnClickedSelectAll()
// Description  : Selects all view ports in all layouts
//////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::OnBnClickedSelectAll()
{
	// Call the function to select the view ports of every layout
	for (int iVPort = 0; iVPort < m_lcLayout.GetItemCount(); iVPort++) { SelecThisViewport(iVPort); }
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnBnClickedClear()
// Description  : De-selects all view ports in the selected layout
//////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::OnBnClickedClear()
{
	// Get selected index
	int iSel = m_lcLayout.GetSelectionMark();
	if (iSel == LB_ERR) { appMessage(L"Select a layout to clear selected view ports from it.", 00); return; }

	// If it has no view ports, exit the function
	if (_tstoi(m_lcLayout.GetItemText(iSel, 3)) <= 0) { appMessage(L"Select a layout with atleast one view port in it."); return; }
		
	CString csSelTxt; csSelTxt.Format(L"0 of %s selected", m_csaVportsCnt.GetAt(iSel));
	m_lcLayout.SetItemText(iSel, 1, csSelTxt);

	m_lcLayout.SetItemText(iSel, 2, L"");
	
	// Set the view ports to ""
	m_csaVports.SetAt(iSel, L"");

	// Remove the selection and enable the "All" suffixed buttons
	EnableAllSuffixedButtons();

	// To remove the highlights
	acedCommandS(RTSTR, L".REGEN", NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: EnableAllSuffixedCButtons()
// Description  : Removes the selection mark of the selected layout and modifies the visibility of certain buttons
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::EnableAllSuffixedButtons()
{
	// Get the selected layout
	int iSel = m_lcLayout.GetSelectionMark();
	if (iSel == CB_ERR) return;

	// Deselect it
	m_lcLayout.SetItemState(iSel, 0, LVIS_SELECTED);

	// Enable/Disable controls in the dialog for this
	m_btnOnScreen.ShowWindow(SW_HIDE);
	m_btnSelect.ShowWindow(SW_HIDE);
	m_btnClear.ShowWindow(SW_HIDE);

	m_btnSelectAll.ShowWindow(SW_NORMAL);
	m_btnClearAll.ShowWindow(SW_NORMAL);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnBnClickedClearall()
// Description  : De-selects all view ports in all layouts
//////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::OnBnClickedClearall()
{
	int iSel = m_lcLayout.GetSelectionMark();

	for (int iVPort = 0; iVPort < m_lcLayout.GetItemCount(); iVPort++)
	{
		// If it has no view ports, exit the function
		if (m_lcLayout.GetItemText(iVPort, 1)[0] == '0') continue;

		CString csSelTxt; csSelTxt.Format(L"0 of %s selected", m_csaVportsCnt.GetAt(iVPort));
		m_lcLayout.SetItemText(iVPort, 1, csSelTxt);

		m_lcLayout.SetItemText(iVPort, 2, L"");

		// Set the view ports to ""
		m_csaVports.SetAt(iVPort, L"");
	}

	m_iCurrentLayout = iSel;
			
	// To remove the highlights
	acedCommandS(RTSTR, L".REGEN", NULL);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnNMClickListlayout()
// Description  : Sets the layout selected as current
//////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::OnNMClickListlayout(NMHDR *pNMHDR, LRESULT *pResult)
{
	// If no entry in the list is selected
	int iSel = m_lcLayout.GetSelectionMark();
	if (iSel == LB_ERR) return;
	
	m_iCurrentLayout = iSel;

	// Enable or Disable appropriate controls
	m_btnOnScreen.ShowWindow(SW_NORMAL);
	m_btnSelect.ShowWindow(SW_NORMAL);
	m_btnClear.ShowWindow(SW_NORMAL);

	m_btnSelectAll.ShowWindow(SW_HIDE);
	m_btnClearAll.ShowWindow(SW_HIDE);
	
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

	// TODO: Add your control notification handler code here
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: Highlight()
// Description  : Highlights (graphically) the selected VPORTS for a layout
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::Highlight(int iSel, bool bHighlight)
{
	// Get the string containing the VPORT handles selected for this layout
	CString csVPort = m_lcLayout.GetItemText(iSel, 2);

	// From the string determine the individual VPORT handles
	CStringArray csaHandles;
	CString csHandle;

	while (csVPort.Find(L";") != -1) 
	{
		csVPort  = csVPort.Mid(csVPort.Find(L";") + 1); 
		csHandle = csVPort;
		if (csHandle.Find(L";") != -1) { csHandle = csHandle.Mid(0, csHandle.Find(L";")); }
		csaHandles.Add(csHandle);
	}

	// Highlight the VPORT graphically for each handle in the collection
	ads_name enHandle;
	for (int i = 0; i < csaHandles.GetSize(); i++)
	{
		// Get the view port entity name for this handle
		acdbHandEnt(csaHandles.GetAt(i), enHandle);
		acedRedraw(enHandle, (bHighlight ? 3 : 4));
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnBnClickedOnscreen()
// Description  : Sets the layout selected as current
//////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::OnBnClickedOnscreen()
{
	// If no entry in the list is selected
	int iSel = m_lcLayout.GetSelectionMark();
	if (iSel == CB_ERR) return;

	// Display the selected layout
	m_csLayout = m_lcLayout.GetItemText(iSel, 0);

	m_iCurrentLayout = iSel;
	OnOK();
}

//////////////////////////////////////////////////////////////////////////
// Function name: CLegendNotificationDlg::OnBnClickedHelp()
// Description  : Calls the help window with the context displayed
//////////////////////////////////////////////////////////////////////////
void CLegendNotificationDlg::OnBnClickedHelp() { displayHelp((DWORD)_T("Dynamic_Legend.htm")); }
void CLegendNotificationDlg::OnBnClickedOk()
{
	int iSel = m_lcLayout.GetSelectionMark();
	Highlight(iSel, false);

	// Adding 1000 to the selected layout INDEX will enable us to get the VPORT handles already selected for this layout
	m_iCurrentLayout = -1;

	// Check if at least one viewport in a layout is selected
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
}

