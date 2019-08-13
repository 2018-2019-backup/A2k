// ImportPoleDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImportPoleDataDlg.h"
#include "io.h"
#include "ConductorInfo.h"
#include "ConductorSpanInfo.h"
#include "CableJoin.h"
#include "OPFileInfo.h"
#include "CSpreadSheet.h"

/////////////////////////////////////////////////////
// Externally defined functions
/////////////////////////////////////////////////////

extern void sortConductorInfo  (std::vector <CConductorInfo> &conductorInfo);
extern void sortOPFileInfo(std::vector <COPFileInfo> &OPFileInfo_Vector);
extern void TrimCable(std::vector <CCableJoin> &cableJoinVec);
extern void SnapLineEndPoints();

extern int CreateHatchBoundary(AcDbObjectIdArray &aryObjIds);

// Utility Functions

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetMaterial()
// Description  : From standard tables retrieves the material name for the given code.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
CString GetMaterial(CString csMaterialCode)
{
	CString csMaterial = csMaterialCode;

	CQueryTbl tblPoleImportMatl;
	CString csSQL;
	// csSQL.Format(L"SELECT Data FROM tblAttributeData WHERE Block = 'POLE_PROP' AND fldCode = '%s'", csMaterialCode);
	csSQL.Format(L"SELECT fldDescription FROM tblImport_Material WHERE fldCode = '%s'", csMaterialCode);
	if (!tblPoleImportMatl.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return csMaterial; 
	if (tblPoleImportMatl.GetRows() > 0) csMaterial = tblPoleImportMatl.GetRowAt(0)->GetAt(0);

	return csMaterial;
}

// CImportPoleDataDlg dialog
IMPLEMENT_DYNAMIC(CImportPoleDataDlg, CDialog)

CImportPoleDataDlg::CImportPoleDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportPoleDataDlg::IDD, pParent)
{
	m_csOffset = _T("");
	m_csOtherSpacing = _T("0.0");
	m_bPolesOnly = false;
}

CImportPoleDataDlg::~CImportPoleDataDlg()
{
}

void CImportPoleDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP_OFFSET, m_cbOffset);
	DDX_CBString(pDX, IDC_IP_OFFSET, m_csOffset);
	DDX_Control(pDX, IDC_IP_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_IP_OTHERVALUE, m_edOtherOffsetValue);
	DDX_Text(pDX, IDC_IP_OTHERVALUE, m_csOtherSpacing);
	DDX_Control(pDX, IDC_IP_SIDE, m_cbSide);
	DDX_CBString(pDX, IDC_IP_SIDE, m_csSide);
}

BEGIN_MESSAGE_MAP(CImportPoleDataDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CImportPoleDataDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_IP_BROWSE, &CImportPoleDataDlg::OnBnClickedBrowse)
	ON_CBN_SELCHANGE(IDC_IP_OFFSET, &CImportPoleDataDlg::OnCbnSelChangeOffset)
	ON_BN_CLICKED(IDC_UNDO, &CImportPoleDataDlg::OnBnClickedUndo)
	ON_BN_CLICKED(IDHELP, &CImportPoleDataDlg::OnBnClickedHelp)
END_MESSAGE_MAP()

// CImportPoleDataDlg message handlers
/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
BOOL CImportPoleDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Populate the offset values from tables
	CQueryTbl tblOffset;
	CString csSQL;
	csSQL.Format(L"SELECT fldOffsetValue FROM tblOffset ORDER BY fldSequence");
	if (!tblOffset.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return TRUE;
	for (int i = 0; i < tblOffset.GetRows(); i++) m_cbOffset.AddString(tblOffset.GetRowAt(i)->GetAt(0));
	
	m_csPathName = "";

	//////////////////////////
	// Set the defaults
	//////////////////////////
	// Offset
	if (m_csOffset.IsEmpty()) m_cbOffset.SetCurSel(0); else m_cbOffset.SelectString(-1, m_csOffset);
	OnCbnSelChangeOffset();

	// Side
	if (!m_csSide.IsEmpty()) m_cbSide.SelectString(-1, m_csSide); else m_cbSide.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedBrowse()
// Description  : Displays the file open dialog to enable selection of XLS file to import.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CImportPoleDataDlg::OnBnClickedBrowse()
{
	// Call the file dialog to select XLS file
	CString csPath, csBrowsedPath;
	static TCHAR BASED_CODE szFilter[] = _T("Microsoft Office Excel (.xls)|*.xls||");
	CFileDialog dlgBrowse(TRUE, _T("xls"), NULL, OFN_HIDEREADONLY, szFilter, this);
	if (dlgBrowse.DoModal() != IDOK) return;

	// Set the file name selected in the edit control for file name
	SetDlgItemText(IDC_IP_FILENAME, dlgBrowse.GetPathName());
	m_csPathName = dlgBrowse.GetPathName();
	GetDlgItem(IDOK)->EnableWindow(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CImportPoleDataDlg::AssignLayerForType()
// Description  : Retrieves the LAYER suffixes to be used for various circuits in the XLS file.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CImportPoleDataDlg::AssignLayerForType(CString csCircuit, CString csLayerSuffix)
{
	if (csCircuit.IsEmpty()) return L"";

	CString csLayer = L""; 
	// csCircuit.Replace(_T("TR"), _T("TR_"));
	// csLayer.Format(_T("%s_OH_%s"), csCircuit, csLayerSuffix);

	if (!m_csaCircuits.GetCount())
	{
		CQueryTbl tblCircuits;
		CString csSQL = L"SELECT [Circuits], [Layer], [Layer_EXIST] FROM tblCircuits ORDER BY ID";
		if (!tblCircuits.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) { return csLayer; }
		if (tblCircuits.GetRows())
		{
			tblCircuits.GetColumnAt(0, m_csaCircuits);
			tblCircuits.GetColumnAt(1, m_csaPropLayers);
			tblCircuits.GetColumnAt(2, m_csaExistLayers);
		}
	}

	for (int iC = 0; iC < m_csaCircuits.GetSize(); iC++)
	{
		if (!m_csaCircuits.GetAt(iC).CompareNoCase(csCircuit) && !csLayerSuffix.CompareNoCase(L"PROP")) 
		{
			csLayer =  m_csaPropLayers.GetAt(iC);
			break;
		}
		else if (!m_csaCircuits.GetAt(iC).CompareNoCase(csCircuit) && !csLayerSuffix.CompareNoCase(L"EXIST")) 
		{
			csLayer =  m_csaExistLayers.GetAt(iC);
			break;
		}
	}

	if (csLayer.IsEmpty()) csLayer.Format(_T("%s_OH_%s"), csCircuit, csLayerSuffix);

	// If the layer does not exist, create it and set its property from the standard DWT
	UpdateLayerFromStandards(csLayer);
	return csLayer;
}

/////////////////////////////////////////////////////
// Function name: OnCbnSelChangeOffset()
// Description  :
/////////////////////////////////////////////////////
void CImportPoleDataDlg::OnCbnSelChangeOffset()
{
	int iSel = m_cbOffset.GetCurSel();
	if (iSel == CB_ERR) return;
	m_cbOffset.GetLBText(iSel, m_csOffset);

	// Enable/Disable the offset value edit control depending on whether the spacing selected in "Other" or otherwise
	if (m_csOffset.CompareNoCase(L"Other")) 
	{
		m_edOtherOffsetValue.EnableWindow(FALSE);
		m_csOtherSpacing = L"0.0";
		m_edOtherOffsetValue.SetWindowText(m_csOtherSpacing);
	}
	else
	{
		m_edOtherOffsetValue.EnableWindow(TRUE);
		m_edOtherOffsetValue.SetWindowText(m_csOtherSpacing);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetColumnIndex()
// Called by    : Get the value of the parameters based on its suggested column name(s)
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetColumnIndex(CStringArray &csaColNames, CString csSuggestedName1, CString csSuggestedName2)
{
	int iIndex;
	CString csValue;

	// Get the column index to fetch value
	iIndex = GetParameterValue(csaColNames, csSuggestedName1.MakeUpper(), csValue, 0);
	if ((iIndex == -1) && !csSuggestedName2.IsEmpty()) { iIndex = GetParameterValue(csaColNames, csSuggestedName2.MakeUpper(), csValue, 0); }
	
	return iIndex;
}

///////////////////////////////////////////////////////////////////////////
// Function name: DrawConductorsCollected()
// Description  : Draws the circuits between each given pair of pole data.
///////////////////////////////////////////////////////////////////////////
void CImportPoleDataDlg::DrawConductorsCollected(CStringArray &csaCablesInFile, CStringArray &csaCableSortOrder, std:: vector <CConductorSpanInfo> &conductorSpan_vector, double dOffset)
{
	ads_point ptInsert, ptPrevInsert;

	///////////////////////////////////////////////////////////////
	// Place the lines indicating circuits at various voltages
	///////////////////////////////////////////////////////////////
	for (int iSpan = 0; iSpan < conductorSpan_vector.size(); iSpan++)
	{
		// If no circuits are specified, exit the function
		if (!conductorSpan_vector.at(iSpan).m_CircuitInfoVetor.size()) { continue; }

		CConductorInfo conductorInfo;	
		std::vector <CConductorInfo> conductorInfo_Vector;

		int iNoOfSegs = 0;
		CString csLayer;

		for (int iC = 0; iC < conductorSpan_vector.at(iSpan).m_CircuitInfoVetor.size(); iC++)
		{
			if (m_csSide.CompareNoCase(L"None"))
			{
				// acutPrintf(L"\nCond Vector = %s [%s]", conductorSpan_vector.at(iSpan).m_CircuitInfoVetor.at(iC).m_csCircuit, conductorSpan_vector.at(iSpan).m_CircuitInfoVetor.at(iC).m_csLayerSuffix);
				if (!CheckForDuplication(csaCableSortOrder, conductorSpan_vector.at(iSpan).m_CircuitInfoVetor.at(iC).m_csCircuit, conductorInfo.m_iIndex)) conductorInfo.m_iIndex = 100;
			}
			else conductorInfo.m_iIndex = 1;

			conductorInfo.m_csLayer = AssignLayerForType(conductorSpan_vector.at(iSpan).m_CircuitInfoVetor.at(iC).m_csCircuit, conductorSpan_vector.at(iSpan).m_CircuitInfoVetor.at(iC).m_csLayerSuffix);
			conductorInfo_Vector.push_back(conductorInfo); 
			iNoOfSegs++; 
		}

		// Sort the conductors to ORDER based on the information in tblCircuits
		if (m_csSide.CompareNoCase(L"None")) sortConductorInfo(conductorInfo_Vector);

		AcCmColor color; color.setColor(AcCmEntityColor::kByLayer);
		ads_point ptStart, ptEnd;

		double dAngle = 0.0;
		double dDistance = 0.0; 

		if (iSpan != conductorSpan_vector.size() - 1)
		{
			dAngle = acutAngle(conductorSpan_vector.at(iSpan).m_ptInsert, conductorSpan_vector.at(iSpan + 1).m_ptInsert);
			dDistance = acutDistance(conductorSpan_vector.at(iSpan).m_ptInsert, conductorSpan_vector.at(iSpan + 1).m_ptInsert);

			acutPolar(conductorSpan_vector.at(iSpan).m_ptInsert,     0.0, 0.0, ptInsert);
		}
		else
		{
			dAngle = acutAngle(conductorSpan_vector.at(iSpan - 1).m_ptInsert, conductorSpan_vector.at(iSpan).m_ptInsert);
			dDistance = acutDistance(conductorSpan_vector.at(iSpan - 1).m_ptInsert, conductorSpan_vector.at(iSpan).m_ptInsert);

			acutPolar(conductorSpan_vector.at(iSpan).m_ptInsert, 0.0,    0.0, ptPrevInsert);
			acutPolar(ptPrevInsert,                              dAngle, 3.0, ptInsert);
		}

		double dAngleAway; 
		double dAngleClose; 

		ads_point ptOne; acutPolar(ptInsert, dAngle - (PI / 2), dOffset * 0.5, ptOne);
		ads_point ptTwo; acutPolar(ptInsert, dAngle + (PI / 2), dOffset * 0.5, ptTwo);

		dAngleClose = dAngle - (PI / 2);
		dAngleAway  = dAngle + (PI / 2);
		
		// Distance factor of the last segment from the center line
		double dFactor = ((iNoOfSegs % 2 == 0) ? (iNoOfSegs / 2) - 0.5 : (iNoOfSegs - 1) / 2); 
		for (int iSeg = 0; iSeg < conductorInfo_Vector.size(); iSeg++)
		{
			// Start point of the circuit calculated from the point on the center line
			acutPolar(ptInsert, ((iSeg < (iNoOfSegs / 2)) ? dAngleClose : dAngleAway), dOffset * fabs(dFactor - iSeg), ptStart);
		
			// End point of the cable
			acutPolar(ptStart, dAngle, dDistance, ptEnd);

			// Add the first and second vertex
			AcDbPolyline *pPline = new AcDbPolyline();
			pPline->addVertexAt(0, AcGePoint2d(ptStart[X], ptStart[Y]), 0.0);
			pPline->addVertexAt(1, AcGePoint2d(ptEnd[X], ptEnd[Y]), 0.0);

			// Set the layer (If it does not exist, create it with the properties specified in the database.
			createLayer(conductorInfo_Vector[iSeg].m_csLayer, Adesk::kFalse, Adesk::kFalse);
			pPline->setLayer(conductorInfo_Vector[iSeg].m_csLayer);
			pPline->setLinetype(_T("BYLAYER"));
			pPline->setColor(color);

			appendEntityToDatabase(pPline);
			pPline->close();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedUndo()
// Description  : Removes all modifications to the drawing performed by previous import.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CImportPoleDataDlg::OnBnClickedUndo()
{
	// Removes the action done by the import
	struct resbuf rbVar; acedGetVar(L"UNDOMARKS", &rbVar);
	if (rbVar.resval.rint > 0) acedCommandS(RTSTR, L".UNDO", RTSTR, L"B", NULL);
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
void CImportPoleDataDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	
	// Check if the file name specified exists
	if (m_csPathName.IsEmpty()) { ShowBalloon(L"Select a spreadsheet file to import data.", this, IDC_IP_FILENAME); return; }
	if (_taccess(m_csPathName, 00)) { ShowBalloon(L"File not found!", this, IDC_IP_FILENAME); return; }

	// Check if offset value is specified
	if (m_cbOffset.GetCurSel() == CB_ERR) { ShowBalloon(L"Specify offset value.", this, IDC_IP_OFFSET); return; }
	m_cbOffset.GetLBText(m_cbOffset.GetCurSel(), m_csOffset);

	// Check if the offset value is specified for "Other"
	if (m_edOtherOffsetValue.IsWindowEnabled() && (_tstof(m_csOtherSpacing) <= 0.0)) { ShowBalloon(L"Specify valid conductor spacing.", this, IDC_IP_OTHERVALUE); return; } 
	double dOffset = ((_tstof(m_csOffset) <= 0.0) ? _tstof(m_csOtherSpacing) : _tstof(m_csOffset));

	// Find out if only poles are to be drawn or the conductors are to be drawn too
	m_bPolesOnly = IsDlgButtonChecked(IDC_IP_POLEONLY);

	// Store the specified values so that they may be retrieved during the next instance of this command
	struct resbuf *rbpDefault = acutBuildList(AcDb::kDxfText, m_csOffset, AcDb::kDxfText, m_csOtherSpacing, NULL);
	addXRecordToDictionary(_T("eCapture"), _T("NIPC Defaults"), rbpDefault);	

	// Read the Status and Pole block names to be used apart from the default POLE_PROP
	CString csState;
	CStringArray csaStates;
	CQueryTbl tblPoleImport_State;
	CString csSQL = L"SELECT [State], [Block] FROM tblPoleImport_State";
	if (!tblPoleImport_State.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;
	for (int iS = 0; iS < tblPoleImport_State.GetRows(); iS++)
	{
		csState.Format(L"%s#%s", tblPoleImport_State.GetRowAt(iS)->GetAt(0), tblPoleImport_State.GetRowAt(iS)->GetAt(1));
		csaStates.Add(csState);
	}

	// Removes the action done by the import
	acedCommandS(RTSTR, L".UNDO", RTSTR, L"M", NULL);

	/////////////////////////////////////////////////////////////////////////
	// Read the order in which circuits are to be drawn from tblCircuits
	/////////////////////////////////////////////////////////////////////////
	CQueryTbl tblCircuits;
	CString csOrder = L"ASC";
	if (!m_csSide.CompareNoCase(L"Left")) csOrder = L"DESC";
	csSQL.Format(L"SELECT [Circuits] FROM tblCircuits WHERE [Circuits] <> '' ORDER BY [DrawSequence] %s", csOrder);
	
	if (!tblCircuits.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;
	if (tblCircuits.GetRows() <= 0) return;

	CStringArray csaCableSortOrder; 
	tblCircuits.GetColumnAt(0, csaCableSortOrder);
	
	// Check for records in the XLS
	CSpreadSheet SS(m_csPathName, L"CAD Import", false);
	int iNumRecs = SS.GetTotalRows();
	if (!iNumRecs) { appMessage(L"\nCheck if the selected file has \"CAD Import\" worksheet.");	return; }
	
	m_Progress.SetRange(0, iNumRecs);
	m_Progress.SetStep(1);

	ads_point ptInsert, ptPrevInsert;
	AcDbObjectId objPole;
	CString csMsg; 

	// Collect the circuit details in the following 
	SetDlgItemText(IDC_IP_STATICTEXT, m_csPathName);
	SetDlgItemText(IDC_STATIC_FILEHEADER, L"Reading file for pole records:");

	CString csHeader;
	int iConstructionIndex = -1;
	CStringArray csaColNames;
	CString csColParam;

	CString csPrefix, csNumber;
	CString csLength, csStrength, csMaterialCode, csMaterial, csDiameter, csDepth;
	CString csX, csY;
	int iIndex;
	CString csValue;
	CStringArray csaCablesInFile;
	CStringArray csaConductorData;

	int iLayerSuffixIndex;
	std::vector <CConductorSpanInfo> conductorSpan_vector;

	// Acceptable pole headers
	CStringArray csaValidPoleHeaders;
	csaValidPoleHeaders.Add(L"PREFIX");
	csaValidPoleHeaders.Add(L"NUMBER");
	csaValidPoleHeaders.Add(L"EASTING");
	csaValidPoleHeaders.Add(L"X");
	csaValidPoleHeaders.Add(L"NORTHING"); 
	csaValidPoleHeaders.Add(L"Y"); 
	csaValidPoleHeaders.Add(L"SIZE m");
	csaValidPoleHeaders.Add(L"LENGTH");
	csaValidPoleHeaders.Add(L"STRENGTH kN");
	csaValidPoleHeaders.Add(L"STRENGTH");
	csaValidPoleHeaders.Add(L"TYPE T/C/S");
	csaValidPoleHeaders.Add(L"MATERIAL");
	csaValidPoleHeaders.Add(L"BUTT DIAM mm");
	csaValidPoleHeaders.Add(L"DIAMETER");
	csaValidPoleHeaders.Add(L"E/R/N");
	csaValidPoleHeaders.Add(L"STATE");
	csaValidPoleHeaders.Add(L"DEPTH m");
	csaValidPoleHeaders.Add(L"DEPTH");

	m_csaCircuits.RemoveAll();
	m_csaExistLayers.RemoveAll();
	m_csaPropLayers.RemoveAll();
	
	// Tables
	CStdioFile fOut;
	CString csOPPath; csOPPath.Format(L"%s", m_csPathName);
	csOPPath = csOPPath.MakeUpper();
	csOPPath.Replace(L".XLS", L".LOG");
	
	if (_taccess(csOPPath, 00) != -1) DeleteFile(csOPPath);
	if (!fOut.Open(csOPPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText)) { acutPrintf(L"\nError creating output file!"); return; }

	CString csOut;
	std::vector <COPFileInfo> OPFileInfo_Vector;
	int iNoOfBlankRows = 0;

	for (int iRow = 1; iRow <= iNumRecs; iRow++)
	{
		m_Progress.StepIt(); m_Progress.UpdateWindow();
		csMsg.Format(L"%d of %d", iRow, iNumRecs);
		SetDlgItemText(IDC_IP_STATICPROGRESS, csMsg);

		CStringArray csaRowValues; 
		if (!SS.ReadRow(csaRowValues, iRow)) continue;
		
		// Check to eliminate the first few rows before the column headers
		if (!csaCablesInFile.GetSize() && csaRowValues.GetAt(1).CompareNoCase(L"NUMBER") && csaRowValues.GetAt(0).CompareNoCase(L"PREFIX")) 
		{
			if (!csaRowValues.GetAt(0).IsEmpty()) fOut.WriteString(L"\nNon standard:" + csaRowValues.GetAt(0));
			continue;
		}
		
		// Get the pole and cable headers from the file
		if (!csaCablesInFile.GetSize())
		{
			CString csHeader = L"\nHeading row:";
			CString csRowValue;
			for (int iN = 0; iN < csaRowValues.GetSize(); iN++) 
			{
				bool bIsPoleHeader = false; 
				for (int iP = 0; iP < csaValidPoleHeaders.GetSize(); iP++)
				{
					csRowValue = csaRowValues.GetAt(iN);
					csRowValue = csRowValue.Trim();
					csRowValue.MakeUpper();

					if (!csRowValue.CompareNoCase(csaValidPoleHeaders.GetAt(iP)))
					{
						csColParam.Format(L"%s#%d", csRowValue.MakeUpper(), iN); 
						csaColNames.Add(csColParam);	
						csHeader += (L"\t" + csRowValue);
						bIsPoleHeader = true;
						break;
					}
				}

				if (!bIsPoleHeader)
				{
					csHeader = csaRowValues.GetAt(iN);
					csaCablesInFile.Add(csHeader); 
					if (iConstructionIndex == -1) iConstructionIndex = iN;
				}
			}
			
			fOut.WriteString(csHeader.MakeUpper());
			continue;
		}

		// Location
		iIndex = GetColumnIndex(csaColNames, L"EASTING", L"X");
		if (iIndex != -1) 
		{
			csX = csaRowValues.GetAt(iIndex); 

			COPFileInfo OPFileInfo; 
			OPFileInfo.m_iIndex = iIndex; OPFileInfo.m_csOut = csX; OPFileInfo_Vector.push_back(OPFileInfo);
		}

		iIndex = GetColumnIndex(csaColNames, L"NORTHING", L"Y");
		if (iIndex != -1) 
		{
			csY = csaRowValues.GetAt(iIndex);

			COPFileInfo OPFileInfo; 
			OPFileInfo.m_iIndex = iIndex; OPFileInfo.m_csOut = csY; OPFileInfo_Vector.push_back(OPFileInfo);
		} 

		// Empty rows
		if ((_tstof(csX) <= 0.0) && (_tstof(csY) <= 0.0))  
		{
			acutPrintf(L"\nCompleted processing a group at [%d]...", iRow + 1);
		
			// Draw the conductors spanning all the poles (across groups)
			if (conductorSpan_vector.size() > 0) DrawConductorsCollected(csaCablesInFile, csaCableSortOrder, conductorSpan_vector, dOffset);

			iConstructionIndex = -1;
			csaCablesInFile.RemoveAll();
			csaColNames.RemoveAll();
			m_csaCircuits.RemoveAll();
			m_csaExistLayers.RemoveAll();
			m_csaPropLayers.RemoveAll();

			conductorSpan_vector.clear();
			OPFileInfo_Vector.clear();
			continue;
		}

		ptInsert[X] = _tstof(csX); ptInsert[Y] = _tstof(csY); ptInsert[Z] = 0.0;

		// Pole attribute values from XLS
		int iPrefixIndex = -1, iNumberIndex = -1;
		iIndex = GetColumnIndex(csaColNames, L"PREFIX", L"");
		if (iIndex != -1) 
		{
			iPrefixIndex = iIndex;
			csPrefix = csaRowValues.GetAt(iIndex);
		}

		iIndex = GetColumnIndex(csaColNames, L"NUMBER", L"");
		if (iIndex != -1) 
		{
			iNumberIndex = iIndex;
			csNumber = csaRowValues.GetAt(iIndex);
		}

		if (csPrefix.Find(L"-") != -1)
		{
			int iNumber = _tstoi(csPrefix.Mid(csPrefix.Find(L"-") + 1));
			if (iNumber != 0)
			{
				// There is no number after "-" so take the content from the NUMBER field only
				csNumber = csPrefix.Mid(csPrefix.Find(L"-") + 1);
			}
			
			csPrefix = csPrefix.Mid(0, csPrefix.Find(L"-"));
		}
		else if (csNumber.Find(L"-") != -1)
		{
			csPrefix = csNumber.Mid(0, csNumber.Find(L"-"));
			csNumber = csNumber.Mid(csNumber.Find(L"-") + 1);
		}

		COPFileInfo OPFileInfoPrefix; 
		OPFileInfoPrefix.m_iIndex = iPrefixIndex; OPFileInfoPrefix.m_csOut = csPrefix; OPFileInfo_Vector.push_back(OPFileInfoPrefix);

		COPFileInfo OPFileInfoNumber; 
		OPFileInfoNumber.m_iIndex = iNumberIndex; OPFileInfoNumber.m_csOut = csNumber; OPFileInfo_Vector.push_back(OPFileInfoNumber);

		// Length
		iIndex = GetColumnIndex(csaColNames, L"SIZE m", L"LENGTH");
		if (iIndex != -1) 
		{
			csLength = csaRowValues.GetAt(iIndex);

			COPFileInfo OPFileInfo; 
			OPFileInfo.m_iIndex = iIndex; OPFileInfo.m_csOut = csLength; OPFileInfo_Vector.push_back(OPFileInfo);
		}

		// Strength
		iIndex = GetColumnIndex(csaColNames, L"STRENGTH kN", L"STRENGTH");
		if (iIndex != -1)
		{
			csStrength = csaRowValues.GetAt(iIndex);

			COPFileInfo OPFileInfo; 
			OPFileInfo.m_iIndex = iIndex; OPFileInfo.m_csOut = csStrength; OPFileInfo_Vector.push_back(OPFileInfo);
		}

		// Material
		iIndex = GetColumnIndex(csaColNames, L"TYPE T/C/S", L"MATERIAL");
		if (iIndex != -1) 
		{
			csMaterialCode = csaRowValues.GetAt(iIndex);
			csMaterial     = GetMaterial(csMaterialCode);

			COPFileInfo OPFileInfo; 
			OPFileInfo.m_iIndex = iIndex; OPFileInfo.m_csOut = csMaterialCode; OPFileInfo_Vector.push_back(OPFileInfo);
		}

		// Diameter
		iIndex = GetColumnIndex(csaColNames, L"BUTT DIAM mm", L"DIAMETER");
		if (iIndex != -1) 
		{
			csDiameter = csaRowValues.GetAt(iIndex);

			COPFileInfo OPFileInfo; 
			OPFileInfo.m_iIndex = iIndex; OPFileInfo.m_csOut = csDiameter; OPFileInfo_Vector.push_back(OPFileInfo);
		}

		// State
		iIndex = GetColumnIndex(csaColNames, L"E/R/N", L"STATE");
		if (iIndex != -1) 
		{
			csState = csaRowValues.GetAt(iIndex);

			COPFileInfo OPFileInfo; 
			OPFileInfo.m_iIndex = iIndex; OPFileInfo.m_csOut = csState; OPFileInfo_Vector.push_back(OPFileInfo);
		}

		// Depth
		iIndex = GetColumnIndex(csaColNames, L"DEPTH m", L"DEPTH");
		if (iIndex != -1) 
		{
			csDepth = csaRowValues.GetAt(iIndex);

			COPFileInfo OPFileInfo; 
			OPFileInfo.m_iIndex = iIndex; OPFileInfo.m_csOut = csDepth; OPFileInfo_Vector.push_back(OPFileInfo);
		}

		// Format the string for file out
		sortOPFileInfo(OPFileInfo_Vector);
		csOut = L"\nPole Data:";
		for (int iO = 0; iO < OPFileInfo_Vector.size(); iO++)	{ csOut += (L"\t" + OPFileInfo_Vector.at(iO).m_csOut); }
		OPFileInfo_Vector.clear();
		fOut.WriteString(csOut);

		///////////////////////////////////////////////////////
		// Decide the pole block name to insert and its layer
		///////////////////////////////////////////////////////
		CString csValue;
		bool bIsProposed      = true;
		CString csPoleBlkName = L"POLE_NUMBERED_PROP";
		CString csPoleLayer   = L"POLE_PROP";

		GetParameterValue(csaStates, csState, csValue, 0);
		if (!csValue.IsEmpty()) { csPoleBlkName = csValue;  csPoleLayer = L"POLE_EXIST"; bIsProposed = false; }

		/////////////////////////////////////////////////
		// Insert the block and 
		/////////////////////////////////////////////////
		if (!acdbTblSearch(L"BLOCK", csPoleBlkName, FALSE)) {	UpdateBlockFromStandards(csPoleBlkName); }
		if (!insertBlock(csPoleBlkName, csPoleLayer, ptInsert, 0.0, 0.0, 0.0, 0.0, _T(""), objPole, TRUE)) { fOut.Close(); return; }

		// For a proposed block the inserted block has to be exploded. Nor for others.
		AcDbBlockReference *pInsert;
		CString csName;
		if (bIsProposed)
		{
			// Explode the block and move all its contents to POLE_PROP layer.
			ads_name enInsert; acdbGetAdsName(enInsert, objPole);
			acedCommandS(RTSTR, L".EXPLODE", RTENAME, enInsert, NULL);

			// Collect all the resultant entities and put them on POLE_PROP layer
			ads_name ssExplode; 
			if (acedSSGet(L"P", NULL, NULL, NULL, ssExplode) ==  RTNORM)
			{
				ChangeProperties(true, ssExplode, 0, csPoleLayer);	

				// Within this selection, get the block reference with POLE_PROP as its block name
				AcDbObjectId objId;
				ads_name enEntity;
				//long lLength = 0L; acedSSLength(ssExplode, &lLength);
				int lLength = 0L; acedSSLength(ssExplode, &lLength);
				for (long lCtr = 0L; lCtr < lLength; lCtr++)
				{
					// Get the object ID
					acedSSName(ssExplode, lCtr, enEntity);
					acdbGetObjectId(objId, enEntity);

					// Check if block reference
					AcDbEntity *pEntity;
					if (acdbOpenObject(pEntity, objId, AcDb::kForWrite) != Acad::eOk) { continue; }
					if (!pEntity->isKindOf(AcDbBlockReference::desc())) { pEntity->close(); continue; }
					pInsert = AcDbBlockReference::cast(pEntity);

					// Get the object id of the block definition from the insert
					AcDbObjectId objTblRcd;
					objTblRcd = pInsert->blockTableRecord();

					// Open the symbol table record for this id
					AcDbSymbolTableRecord *pBlkTblRcd;
					if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk) { pInsert->close(); continue; }
					AcString acsName;	pBlkTblRcd->getName(acsName); 

					pBlkTblRcd->close();

					// Check for block name same as POLE_PROP
					csName.Format(_T("%s"), acsName.kTCharPtr()); csName.MakeUpper();
					if (csName.CompareNoCase(L"POLE_PROP")) continue;
					objPole = objId;
					break;
				}

				// Free the EXPLODED selection set
				acedSSFree(ssExplode);
			}
			else { AfxMessageBox(L"none found after explode"); continue; }
		}
		else
		{
			// Get the POLE Insert pointer
			if (acdbOpenObject(pInsert, objPole, AcDb::kForWrite) != Acad::eOk) { fOut.Close(); return; }
		}

		// Modify the attribute values for PREFIX, NUMBER, MATERIAL, LENGTH, DEPTH, STRENGTH and DIAMETER
		AcDbObjectIterator *pIter = pInsert->attributeIterator();

		// Get the attribute tag specified and change the value specified
		AcDbAttribute *pAtt;
		AcDbObjectId objAttId;
		CString csTag;

		Acad::ErrorStatus es;
		for (pIter->start(); !pIter->done(); pIter->step())
		{
			objAttId = pIter->objectId();

			es = acdbOpenObject(pAtt, objAttId, AcDb::kForWrite);
			if (es != Acad::eOk) { acutPrintf(L"\nError: %s", acadErrorStatusText(es)); continue; }
			csTag = pAtt->tag();

			if (!csTag.CompareNoCase(L"PREFIX"))		pAtt->setTextString(csPrefix);
			else if (!csTag.CompareNoCase(L"NUMBER")) 	pAtt->setTextString(csNumber);	
			else if (!csTag.CompareNoCase(L"MATERIAL")) pAtt->setTextString(csMaterial);	
			else if (!csTag.CompareNoCase(L"LENGTH"))   pAtt->setTextString(csLength);	
			else if (!csTag.CompareNoCase(L"DEPTH"))    pAtt->setTextString(csDepth);	
			else if (!csTag.CompareNoCase(L"STRENGTH")) pAtt->setTextString(csStrength);	

			pAtt->close();
		}

		delete(pIter);

		// For trimming
		m_objIds.append(pInsert->objectId());

		// Attach XDATA to the pole block so that these attribute values may be modified via Attach attributes
		struct resbuf *rbpXD;
		rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"PREFIX", AcDb::kDxfXdAsciiString, csPrefix, NULL); 
		pInsert->setXData(rbpXD); acutRelRb(rbpXD);

		rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"NUMBER", AcDb::kDxfXdAsciiString, csNumber, NULL); 
		pInsert->setXData(rbpXD); acutRelRb(rbpXD);

		rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"MATERIAL", AcDb::kDxfXdAsciiString, csMaterial, NULL); 
		pInsert->setXData(rbpXD); acutRelRb(rbpXD);

		rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"LENGTH", AcDb::kDxfXdAsciiString, csLength, NULL);
		pInsert->setXData(rbpXD); acutRelRb(rbpXD);

		rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"DEPTH", AcDb::kDxfXdAsciiString, csDepth, NULL);
		pInsert->setXData(rbpXD); acutRelRb(rbpXD);

		rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"STRENGTH", AcDb::kDxfXdAsciiString, csStrength, NULL);
		pInsert->setXData(rbpXD); acutRelRb(rbpXD);

		pInsert->close();

		// If only poles are required to be imported, we can skip the rest of the code
		if (m_bPolesOnly) continue;

		// Collect conductor details to draw from this pole
		CConductorSpanInfo conductorSpanInfo;		
		for (int iCol = 0; iCol < csaCablesInFile.GetSize(); iCol++)
		{
			// If the cell is 0 it implies that this circuit is not required. If "1", it is proposed and "2" implies existing
			iLayerSuffixIndex = _tstoi(csaRowValues.GetAt(iConstructionIndex + iCol));

			if (iLayerSuffixIndex == 0) continue;

			CCircuitInfo circuitInfo;
			circuitInfo.m_csCircuit = csaCablesInFile.GetAt(iCol);
			circuitInfo.m_csLayerSuffix = ((iLayerSuffixIndex == 1) ? L"PROP" : L"EXIST");
			conductorSpanInfo.m_CircuitInfoVetor.push_back(circuitInfo);
		}

		// Insertion point
		conductorSpanInfo.m_ptInsert[X] = ptInsert[X];
		conductorSpanInfo.m_ptInsert[Y] = ptInsert[Y];
		conductorSpanInfo.m_ptInsert[Z] = 0.0;
		conductorSpan_vector.push_back(conductorSpanInfo);
	}

	fOut.Close(); 
	OnOK();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedHelp()
// Description  : Calls the topic specified from NET CAD.CHM file
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CImportPoleDataDlg::OnBnClickedHelp() { displayHelp((DWORD)_T("Import_Pole_and_Conductors.htm")); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
// Function name: Command_ImportPoleAndConductors()
// Description  : Reads data for new poles from external XLS file and inserts them completing its circuit.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
void Command_ImportPoleAndConductors()
{
	switchOff();

	while (T)
	{
		// Display the import pole data dialog
		CImportPoleDataDlg dlgImport;

		//////////////////////////////////////////////////////////////////////////
		// Defaults for controls
		//////////////////////////////////////////////////////////////////////////
		struct resbuf *rbpDefault = getXRecordFromDictionary(_T("eCapture"), _T("NIPC Defaults"));
		if (rbpDefault) 
		{
			// Default the value selected during the previous instance of this command
			dlgImport.m_csOffset			 = rbpDefault->resval.rstring;
			dlgImport.m_csOtherSpacing = rbpDefault->rbnext->resval.rstring;

			acutRelRb(rbpDefault);
		} 

		// Display the commands dialog
		if (dlgImport.DoModal() == IDCANCEL) return;
		if (dlgImport.m_bPolesOnly == true) continue;

		// Post process all the poles drawn so that the cables connected to it are filleted with radius 0.0
		Acad::ErrorStatus es;
		AcDbObjectIdArray aryConductors;
		
		for (int iC = 0; iC < dlgImport.m_objIds.length(); iC++)
		{
			AcDbObjectId objPole = dlgImport.m_objIds.at(iC);
			AcDbBlockReference *pInsert = NULL;

			es = acdbOpenObject(pInsert, objPole, AcDb::kForRead);
			if (es != Acad::eOk) { acutPrintf(acadErrorStatusText(es)); continue; }
						
			// Get the geometric extents
			AcDbExtents extents;
			pInsert->getGeomExtents(extents);
			AcGePoint3d position = pInsert->position();
			pInsert->close();

			// Zoom to the EXTENTS
			acedCommandS(RTSTR, L".ZOOM", RTSTR, L"W", RTPOINT, asDblArray(extents.minPoint()), RTPOINT, asDblArray(extents.maxPoint()), NULL);
			acedCommandS(RTSTR, L".ZOOM", RTSTR, L"S", RTSTR, L"0.9X", NULL);
		
			// Use the extents to collect all cables connected to it
			ads_name ssCables;
			if (acedSSGet(L"C", asDblArray(extents.minPoint()), asDblArray(extents.maxPoint()), NULL, ssCables) != RTNORM) { /*acutPrintf(L"\nNo cables");*/ continue; }
		
			// Process the cables connected to the pole and trim them
			//long lCables = 0L;
			int lCables = 0L;
			TCHAR handleStr[256];
			acedSSLength(ssCables, &lCables);

			std::vector <CCableJoin> cableJoinVec;
			for (long lCnt = 0L; lCnt < lCables; lCnt++)
			{
				ads_name enCable;
				acedSSName(ssCables, lCnt, enCable);

				AcDbPolyline *pCable = NULL;
				AcDbObjectId objCable;
				acdbGetObjectId(objCable, enCable);
				es = acdbOpenObject(pCable, objCable, AcDb::kForRead); 
				if (es != Acad::eOk) { /*acutPrintf(L"\nError : %s", acadErrorStatusText(es));*/ continue; }

				// Required to join them to one single conductor
				aryConductors.append(objCable);

				int iNumVertices = pCable->numVerts();
				ACHAR *pLayerName = pCable->layer();
				AcGePoint3d geStart; pCable->getPointAt(0, geStart);
				AcGePoint3d geEnd;   pCable->getPointAt(iNumVertices - 1, geEnd);

				CCableJoin cableJoin;
				cableJoin.objId = pCable->objectId();
				pCable->close();
			
				cableJoin.csLayer = pLayerName;
				if (acutDistance(asDblArray(geStart), asDblArray(position)) <= acutDistance(asDblArray(geEnd), asDblArray(position)))
				{
					acutPolar(asDblArray(geStart), 0.0, 0.0, cableJoin.ptTrim); 
					acutPolar(asDblArray(geEnd), 0.0, 0.0, cableJoin.ptOther); 
					cableJoin.iVertexAt = 0;
				}
				else
				{
					acutPolar(asDblArray(geStart), 0.0, 0.0, cableJoin.ptOther); 
					acutPolar(asDblArray(geEnd), 0.0, 0.0, cableJoin.ptTrim); 
					cableJoin.iVertexAt = pCable->numVerts() - 1;
				}
							
				cableJoin.csLayer = pLayerName;
				cableJoinVec.push_back(cableJoin);
			}

			acedSSFree(ssCables);

			// Get the cable pairs from the same layer
			TrimCable(cableJoinVec);
		}

		// Set the ZOOM to view the drawn POLES and CONDUCTORS
		acedCommandS(RTSTR, L".ZOOM", RTSTR, L"E", NULL);
	}
}

