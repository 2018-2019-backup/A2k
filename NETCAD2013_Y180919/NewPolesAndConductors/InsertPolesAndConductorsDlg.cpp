// InsertPolesAndConductorsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InsertPolesAndConductorsDlg.h"
#include "ConductorInfo.h"
#include <math.h>

/////////////////////////////////////////////////////
// Externally defined functions
/////////////////////////////////////////////////////
extern void sortConductorInfo   (std:: vector <CConductorInfo> &conductorInfo);

// Utility functions

//////////////////////////////////////////////////////////////////////////
// Function name: GetSegmentAngle()
// Description  : Retrieves the angle of the segment at the given point.
//////////////////////////////////////////////////////////////////////////
double GetSegmentAngle(AcDbEntity *pEntity, ads_point ptDummy)
{
	double dAngle = -1;
	if (!pEntity->isKindOf(AcDbPolyline::desc())) return dAngle;

	AcGePoint3d gePtOnCurve;
	double dParam;
	AcGeLineSeg2d pLineSeg;
	AcGePoint2d geStart, geEnd, geCen; 

	AcDbPolyline *pPLine = AcDbPolyline::cast(pEntity);

	// Get the point on the entity that is the closest to the pick point
	pPLine->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0), gePtOnCurve);

	// Get the end points of linear polyline
	for (int iSeg = 0; iSeg < pPLine->numVerts() - 1; iSeg++)
	{
		if (Adesk::kTrue == pPLine->onSegAt(iSeg, AcGePoint2d(gePtOnCurve.x, gePtOnCurve.y), dParam))
		{
			pPLine->getBulgeAt(iSeg, dParam);

			if (dParam != 0) 
			{
				pPLine->close();
				return dAngle;
			}
			else
			{
				// Define the line segment that has to be offset
				AcGeLineSeg2d lineSeg; pPLine->getLineSegAt(iSeg, lineSeg);
				geStart = lineSeg.startPoint();
				geEnd	  = lineSeg.endPoint();
				dAngle = acutAngle(asDblArray(geStart), asDblArray(geEnd));

				pPLine->close();
				return dAngle;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// If a closing segment is picked, it should come here
	//////////////////////////////////////////////////////////////////////////

	// Define the line segment that has to be offset
	AcGePoint3d geStart3d;
	AcGePoint3d geEnd3d;

	pPLine->getPointAt(0, geStart3d);
	pPLine->getPointAt(pPLine->numVerts() - 1, geEnd3d);
	dAngle = acutAngle(asDblArray(geStart3d), asDblArray(geEnd3d));

	pPLine->close();
	return dAngle;
}

// CInsertPolesAndConductorsDlg dialog
IMPLEMENT_DYNAMIC(CInsertPolesAndConductorsDlg, CDialog)

CInsertPolesAndConductorsDlg::CInsertPolesAndConductorsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInsertPolesAndConductorsDlg::IDD, pParent)
{
	m_csSpacing =_T("");
}

CInsertPolesAndConductorsDlg::~CInsertPolesAndConductorsDlg()
{
}

void CInsertPolesAndConductorsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPC_CIRCUIT1, m_cbCircuit1);
	DDX_Control(pDX, IDC_IPC_CIRCUIT2, m_cbCircuit2);
	DDX_Control(pDX, IDC_IPC_CIRCUIT3, m_cbCircuit3);
	DDX_Control(pDX, IDC_IPC_CIRCUIT4, m_cbCircuit4);
	DDX_CBString(pDX, IDC_IPC_CIRCUIT1, m_csCircuit1);
	DDX_CBString(pDX, IDC_IPC_CIRCUIT2, m_csCircuit2);
	DDX_CBString(pDX, IDC_IPC_CIRCUIT3, m_csCircuit3);
	DDX_CBString(pDX, IDC_IPC_CIRCUIT4, m_csCircuit4);
	DDX_Control(pDX, IDC_IPC_STATUS, m_cbStatus);
	DDX_Control(pDX, IDC_IPC_SPACING, m_cbSpacing);
	DDX_CBString(pDX, IDC_IPC_STATUS, m_csStatus);
	DDX_CBString(pDX, IDC_IPC_SPACING, m_csSpacing);
	DDX_Control(pDX, IDC_IPC_OTHERSPACING, m_edOtherSpacing);
	DDX_Text(pDX, IDC_IPC_OTHERSPACING, m_csOtherSpacing);
}

BEGIN_MESSAGE_MAP(CInsertPolesAndConductorsDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CInsertPolesAndConductorsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDHELP, &CInsertPolesAndConductorsDlg::OnBnClickedHelp)
	ON_CBN_SELCHANGE(IDC_IPC_SPACING, &CInsertPolesAndConductorsDlg::OnCbnSelchangeSpacing)
END_MESSAGE_MAP()


// CInsertPolesAndConductorsDlg message handlers
BOOL CInsertPolesAndConductorsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	////////////////////////////////////////////////////////////
	// Feb 2011: Get distinct Status and display in combo
	////////////////////////////////////////////////////////////
	CQueryTbl tblStatus;
	CString csSQL;
	csSQL.Format(L"SELECT [fldStatus] FROM tblStatus ORDER BY [fldSequence]");
	if (!tblStatus.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return TRUE;
	for (int iRow = 0; iRow < tblStatus.GetRows(); iRow++) m_cbStatus.AddString(tblStatus.GetRowAt(iRow)->GetAt(0));

	/////////////////////////////////////////////////////////////////////////
	// Get the distinct circuits and display in all "Circuit" combos
	/////////////////////////////////////////////////////////////////////////
	CQueryTbl tblCircuits;
	if (tblCircuits.SqlRead(DSN_DWGTOOLS, _T("SELECT [Circuits] FROM tblCircuits ORDER BY [DrawSequence]"), __LINE__, __FILE__, __FUNCTION__,true))
	{
		for (int iRow = 0; iRow < tblCircuits.GetRows(); iRow++) 
		{
			m_cbCircuit1.AddString(tblCircuits.GetRowAt(iRow)->GetAt(0));
			m_cbCircuit2.AddString(tblCircuits.GetRowAt(iRow)->GetAt(0));
			m_cbCircuit3.AddString(tblCircuits.GetRowAt(iRow)->GetAt(0));
			m_cbCircuit4.AddString(tblCircuits.GetRowAt(iRow)->GetAt(0));
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// Get the distinct circuits and display in all "Circuit" combos
	/////////////////////////////////////////////////////////////////////////
	CQueryTbl tblOffset;
	csSQL.Format(L"SELECT [fldOffsetValue] FROM tblOffset ORDER BY [fldSequence]");
	if (!tblOffset.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return TRUE;
	for (int iRow = 0; iRow < tblOffset.GetRows(); iRow++) m_cbSpacing.AddString(tblOffset.GetRowAt(iRow)->GetAt(0));

	// Set defaults
	m_cbStatus.SetCurSel(0);
	// SetDlgItemText(IDC_IPC_SPACING, _T("0.5")); //Feb 2011
	
	// Check if any previous values are to be set from a previous instance of this dialog
	struct resbuf *rbpPC = getXRecordFromDictionary(_T("eCapture"), _T("Poles and Conductors"));
	if (rbpPC)
	{
		m_cbStatus.SelectString(-1, rbpPC->resval.rstring);   rbpPC = rbpPC->rbnext;
		m_cbCircuit1.SelectString(-1, rbpPC->resval.rstring); rbpPC = rbpPC->rbnext; 
		m_cbCircuit2.SelectString(-1, rbpPC->resval.rstring); rbpPC = rbpPC->rbnext;
		m_cbCircuit3.SelectString(-1, rbpPC->resval.rstring); rbpPC = rbpPC->rbnext;
		m_cbCircuit4.SelectString(-1, rbpPC->resval.rstring);

		// Feb 2011: Spacing should be retrieved. Spacing values are now in a drop down rather than an edit box.
		if (rbpPC->rbnext)
		{
			rbpPC = rbpPC->rbnext;
			m_csSpacing = rbpPC->resval.rstring;
			m_cbSpacing.SelectString(-1, rbpPC->resval.rstring);

			if (!m_csSpacing.CompareNoCase(L"Other"))
			{
				rbpPC = rbpPC->rbnext;
				m_csOtherSpacing = rbpPC->resval.rstring;
			}
		}

		// Call the function to set the appropriate spacing
		OnCbnSelchangeSpacing();

		acutRelRb(rbpPC);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
// Function name: CInsertPolesAndConductorsDlg::AssignLayerForType()
// Description  : Calls the help window with the context displayed
//////////////////////////////////////////////////////////////////////////
CString CInsertPolesAndConductorsDlg::AssignLayerForType(CString csCircuit)
{
	if (csCircuit.IsEmpty()) return L"";

	// Get the layer to be assigned for this type
	CQueryTbl tblCircuits;
	CString csSQL;
	csSQL.Format(L"SELECT [Layer] FROM tblCircuits WHERE Circuits = '%s'", csCircuit);
	if (!tblCircuits.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return L"";

	CString csLayer;
	if (tblCircuits.GetRows())
	{
		// HV_OH_PROP
		csLayer = tblCircuits.GetRowAt(0)->GetAt(0);
		if (m_csStatus.CompareNoCase(L"Proposed")) csLayer.Replace(L"_PROP", L"_EXIST");
	}
	else
	{
		csCircuit.Replace(_T("TR"), _T("TR_"));
		csLayer.Format(_T("%s_OH_%s"), csCircuit, !m_csStatus.CompareNoCase(_T("Proposed")) ? _T("PROP") : _T("EXIST")); 
	}

	// If the layer does not exist, create it and set its property from the standard DWT
	UpdateLayerFromStandards(csLayer);
	
	return csLayer;
}

/////////////////////////////////////////////////////////////////////////
// Function name: CInsertPolesAndConductorsDlg::OnCbnSelchangeSpacing()
// Description  :
/////////////////////////////////////////////////////////////////////////
void CInsertPolesAndConductorsDlg::OnCbnSelchangeSpacing()
{
	int iSel = m_cbSpacing.GetCurSel();
	if (iSel == CB_ERR) return;

	m_cbSpacing.GetLBText(iSel, m_csSpacing);
	if (!m_csSpacing.CompareNoCase(L"Other"))
	{
		m_edOtherSpacing.ShowWindow(SW_NORMAL);
		m_edOtherSpacing.SetWindowText(m_csOtherSpacing);
	}
	else
	{
		//m_edOtherSpacing.EnableWindow(FALSE);
		m_edOtherSpacing.SetWindowText(L"0.0");
		m_edOtherSpacing.ShowWindow(SW_HIDE);
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: CInsertPolesAndConductorsDlg::OnBnClickedHelp()
// Description  : Calls the help window with the context displayed
//////////////////////////////////////////////////////////////////////////
void CInsertPolesAndConductorsDlg::OnBnClickedHelp() { displayHelp((DWORD)_T("New_Pole_and_Conductors.htm")); }

//////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedOk()
// Description  : 
//////////////////////////////////////////////////////////////////////////
void CInsertPolesAndConductorsDlg::OnBnClickedOk()
{
	UpdateData();

	////////////////////////////////
	// Selection of circuits
	////////////////////////////////
	// Changed: Feb 2011
	//int iSel = m_cbCircuit1.GetCurSel(); 
	//if (iSel == CB_ERR) {	ShowBalloon(_T("Select a circuit."), this, IDC_IPC_CIRCUIT1);	return;	}
	if (m_csCircuit1.IsEmpty()) {	ShowBalloon(_T("Select a circuit."), this, IDC_IPC_CIRCUIT1);	return;	}

	// Cable offset
	if (m_csSpacing.IsEmpty())			 { ShowBalloon(_T("Specify conductor spacing."), this, IDC_IPC_SPACING); return; }
	if (m_csSpacing.CompareNoCase(L"Other"))
	{
		if (_tstof(m_csSpacing) <= 0.0)  { ShowBalloon(_T("Specify valid conductor spacing."), this, IDC_IPC_SPACING); return; }
	}
	else
	{
		if (_tstof(m_csOtherSpacing) <= 0.0)  { ShowBalloon(_T("Specify valid conductor spacing."), this, IDC_IPC_SPACING); return; }
	}
	
	// Change request: Database Connectivity
	// Save the details selected for the next session
	struct resbuf *rbpPC = acutBuildList(AcDb::kDxfText, m_csStatus,
																			 AcDb::kDxfText, m_csCircuit1, 
																			 AcDb::kDxfText, m_csCircuit2, 
																			 AcDb::kDxfText, m_csCircuit3, 
																			 AcDb::kDxfText, m_csCircuit4, 
																			 AcDb::kDxfText, m_csSpacing,
																			 AcDb::kDxfText, m_csOtherSpacing,
																			 NULL); // Feb 2011: Save spacing too

	addXRecordToDictionary(_T("eCapture"), _T("Poles and Conductors"), rbpPC);
	acutRelRb(rbpPC);

	OnOK();
}

//////////////////////////////////////////////////////////////////////////
// Function name: Command_NewPoleAndConductors()
// Description  : Called on issuing NNPC command.
//////////////////////////////////////////////////////////////////////////
void Command_NewPoleAndConductors()
{
	// Display the insert pole and conductor dialog
	CInsertPolesAndConductorsDlg dlgInsert;
	if (dlgInsert.DoModal() == IDCANCEL) return;

	// Check if the block name is present in this drawing
	CString csBlkName = (!dlgInsert.m_csStatus.CompareNoCase(L"Proposed") ? L"POLE_NUMBERED_PROP" : L"POLE");
	if (!acdbTblSearch(L"BLOCK", csBlkName, FALSE)) {	UpdateBlockFromStandards(csBlkName); }

	// Set the spacing value to this local variable
	double dSpacing = _tstof(dlgInsert.m_csSpacing);
	if (!dlgInsert.m_csSpacing.CompareNoCase(L"Other")) dSpacing = _tstof(dlgInsert.m_csOtherSpacing);
	
	ads_point ptInsert;
	acedInitGet(RSG_NONULL, _T(""));
	if (acedGetPoint(NULL, _T("\nSpecify insertion point: "), ptInsert) == RTCAN) return;

	// Allow user to select boundary
	int iRet = 0;
	ads_name enBndy;
	ads_point ptPickPoint;
	Acad::ErrorStatus es;
	AcDbEntity *pEntity = NULL;
	bool bSelectProperty = true;

	while (TRUE)
	{
		if (bSelectProperty)
		  iRet = acedEntSel(_T("\r\nSelect property boundary or ENTER to pick a point: "), enBndy, ptPickPoint);
		else
			iRet = acedGetPoint(ptInsert, _T("\r\nPick a point or ENTER to select a property: "), ptPickPoint);

		if ((iRet == -5001) || (iRet == 5000))
		{
			bSelectProperty = !bSelectProperty;
			continue;
		}
		else if (iRet == RTCAN) return;
		else if (iRet == RTNORM) 
		{
			if (bSelectProperty)
			{
				// Check if the entity selected is a valid boundary
				AcDbObjectId objId; acdbGetObjectId(objId, enBndy);

				es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
				if (es != Acad::eOk) { acutPrintf(_T("Error: %s"), acadErrorStatusText(es)); return; }

				if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc()) || pEntity->isKindOf(AcDbSpline::desc())) { break; }
				pEntity->close();

				// Not a valid contour
				acutPrintf(_T("\nNot a valid property boundary.\n"));
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// User selects a point
				//////////////////////////////////////////////////////////////////////////
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Insert the pole block
	//////////////////////////////////////////////////////////////////////////
	AcDbObjectId objPole;	
	// Mail: 14.07.08: For New pole, when a new pole is created, the pole block is not placed on any layer. For proposed, the pole should reside on layer POLE_PROP and for existing POLE_EXIST
	// Mail: 16.07.08: The client has updated a the POLE_PROP block in the template, which is now called POLE_NUMBERED_PROP. This resides on the tool palette, and when placed it is exploded 
	//                 to POLE_PROP etc... to be placed on the correct layer. 
	//                 With this said, the NEW POLE command when used via the tool bar places the block on screen; however the 2 attributes contained within the block are not shown. 
	//                 Can you accommodate this?, ie, use the POLE_NUMBER_PROP, so that when it is placed it is exploded as above and displays the two attributes.

	// if (!insertBlock((!dlgInsert.m_csStatus.CompareNoCase(L"Proposed") ? L"POLE_PROP" : L"POLE"), _T(""), ptInsert, 0.0, 0.0, 0.0, 0.0, _T(""), objPole, TRUE)) 
	if (!insertBlock((!dlgInsert.m_csStatus.CompareNoCase(L"Proposed") ? L"POLE_NUMBERED_PROP" : L"POLE"), (!dlgInsert.m_csStatus.CompareNoCase(L"Proposed") ? L"POLE_PROP" : L"POLE_EXIST"), ptInsert, 0.0, 0.0, 0.0, 0.0, _T(""), objPole, TRUE)) 
	{
		if (pEntity) pEntity->close(); 
		return; 
	}

	//////////////////////////////////////////////////////////////////////////
	// For PROPOSED block, we will have to EXPLODE the resultant block. only
	// we will get the POLE_PROP block. 
	//////////////////////////////////////////////////////////////////////////
	if (!dlgInsert.m_csStatus.CompareNoCase(L"Proposed"))
	{
		ads_name enInsert; acdbGetAdsName(enInsert, objPole);
		acedCommandS(RTSTR, L".EXPLODE", RTENAME, enInsert, NULL);

		// Set the properties of POLE_PROP layer by retrieving them from the stadard DWT
		UpdateLayerFromStandards(L"POLE_PROP");
		
		// Collect all the resultant entities and put them on POLE_PROP layer
		ads_name ssExplode; 
		if (acedSSGet(L"P", NULL, NULL, NULL, ssExplode) ==  RTNORM) 
		{
			ads_name enEntity;
			AcDbObjectId objId; 
			AcDbEntity *pEntity;
			//long lLength = 0L; acedSSLength(ssExplode, &lLength);
			int lLength = 0L; acedSSLength(ssExplode, &lLength);
			for (long lCtr = 0L; lCtr < lLength; lCtr++)
			{
				// If the entity is an INSERT, change it to POLE_PROP layer
				acedSSName(ssExplode, lCtr, enEntity);
				acdbGetObjectId(objId, enEntity);
				if (acdbOpenObject(pEntity, objId, AcDb::kForWrite) == Acad::eOk) 
				{
					if (pEntity->isKindOf(AcDbBlockReference::desc())) pEntity->setLayer(L"POLE_PROP"); 
					pEntity->close(); 
				}
			}

			acedSSFree(ssExplode); 
		}
	}

	//////////////////////////////////////////////////////////////////////////
	double dAngle = 0.0;
	if (pEntity != NULL)
	{
		//////////////////////////////////////////////////////////////////////////
		// Get the direction to draw the cables from the property selected.
		//////////////////////////////////////////////////////////////////////////
		if (pEntity->isKindOf(AcDbLine::desc())) 
		{
			AcDbLine *pLine = AcDbLine::cast(pEntity);
			dAngle = acutAngle(asDblArray(pLine->startPoint()), asDblArray(pLine->endPoint()));
			pLine->close();
		}
		else if (pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbSpline::desc())) 
		{
			// Lines should be drawn the lines parallel to a tangent at the point of selection on the arc or spline
			dAngle = acutAngle(ptInsert, ptPickPoint) + PI / 2;
		}
		else if (pEntity->isKindOf(AcDbSpline::desc()))
		{
			dAngle = acutAngle(ptInsert, ptPickPoint) + PI / 2;
		}
		else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
		{
			AcDb2dPolyline *pPLine = AcDb2dPolyline::cast(pEntity);

			//////////////////////////////////////////////////////////////////////////
			// New code
			//////////////////////////////////////////////////////////////////////////
			// Get the end points of linear polyline
			AcGePoint3d gePtOnCurve;
			pPLine->getClosestPointTo(AcGePoint3d(ptPickPoint[X], ptPickPoint[Y], 0.0), gePtOnCurve);
			AcDbObjectIterator* vertexIter = pPLine->vertexIterator();
			pPLine->close();

			// For non-linear polyline, get the angle b/w pick point and insertion point of the pole. The angle perpendicular to this will be the angle
			// to draw the cables.
			dAngle = acutAngle(ptInsert, ptPickPoint) + PI / 2;

			ASSERT(vertexIter != NULL);
			if (vertexIter != NULL)
			{
				// Loop through the VERTEX and determine the segment angle picked by the user
				AcDb2dVertex* vertex;
				AcGePoint3d gePrevVertex;
				AcGePoint3d geThisVertex;
				bool bStartVertext = true;
				CString csAng1, csAng2;

				for (vertexIter->start(); !vertexIter->done(); vertexIter->step()) 
				{
					// Open the vertex data as you step through them
					if (acdbOpenObject(vertex, vertexIter->objectId(), AcDb::kForRead) == Acad::eOk) 
					{
						// If this is the start vertex continue to the top of the loop as we need to process further only after knowing the next vertex
						if (bStartVertext) { bStartVertext = false; gePrevVertex = vertex->position(); vertex->close(); continue; }

						if (vertex->bulge() != 0.0) 
						{
							// For non-linear polyline, get the angle b/w pick point and insertion point of the pole. The angle perpendicular to this will be the angle
							// to draw the cables.
							dAngle = acutAngle(ptInsert, ptPickPoint) + PI / 2;

							vertex->close(); 
							break;
						}

						// Get this vertex location
						geThisVertex = vertex->position(); 
					
						// Close the vertex
						vertex->close();

						// Check if the pick point lies b/w previous vertex and this vertex
						csAng1.Format(_T("%.2f"), RTD(acutAngle(asDblArray(gePrevVertex), asDblArray(gePtOnCurve))));
						csAng2.Format(_T("%.2f"), RTD(acutAngle(asDblArray(gePtOnCurve), asDblArray(geThisVertex))));
					
						if (csAng1 == csAng2)
						{
							dAngle = acutAngle(asDblArray(gePrevVertex), asDblArray(geThisVertex));
							break;
						}

						gePrevVertex = geThisVertex;
						vertex->close();
					}
				}

				delete vertexIter;
			}
		}
		else if (pEntity->isKindOf(AcDbPolyline::desc()))
		{
			dAngle = GetSegmentAngle(pEntity, ptPickPoint);
			if (dAngle == -1.0)	dAngle = acutAngle(ptInsert, ptPickPoint) + PI / 2;
		}
	
		// Close the entity pointer
		pEntity->close(); 
	}
	else
	{
		// No property selected, so the angle will be perpendicular to angle b/w insertion and pick point
		dAngle = acutAngle(ptInsert, ptPickPoint) + PI / 2;
	}

	// Get the geomExtents of the block so that the diameter of the block can be determined
	AcDbExtents exBounds;
	double dPoleDia = fabs(exBounds.maxPoint().x - exBounds.minPoint().x);

	// Array that defines the order of drawing the conductors
	CStringArray csaCableSortOrder; 

	/*
	Commented as per DC4 for AEC Projects issues Register - DC
	csaCableSortOrder.Add(_T("SV"));  csaCableSortOrder.Add(_T("SL"));   csaCableSortOrder.Add(_T("LV"));   
	csaCableSortOrder.Add(_T("HV"));  csaCableSortOrder.Add(_T("TR33")); csaCableSortOrder.Add(_T("TR66")); csaCableSortOrder.Add(_T("TR132"));
	*/

	csaCableSortOrder.Add(_T("SV"));   csaCableSortOrder.Add(_T("LV"));   csaCableSortOrder.Add(_T("SL"));  csaCableSortOrder.Add(_T("HV")); 
	csaCableSortOrder.Add(_T("TR33")); csaCableSortOrder.Add(_T("TR66")); csaCableSortOrder.Add(_T("TR132"));

	std::vector <CConductorInfo> conductorInfo_Vector;
	CConductorInfo conductorInfo;	

	int iNoOfSegs = 0;
	CString csLayer;
	if (!dlgInsert.m_csCircuit1.IsEmpty()) 
	{
		CheckForDuplication(csaCableSortOrder, dlgInsert.m_csCircuit1, conductorInfo.m_iIndex); 
		conductorInfo.m_csLayer = dlgInsert.AssignLayerForType(dlgInsert.m_csCircuit1);
		conductorInfo_Vector.push_back(conductorInfo); 
		iNoOfSegs++; 
	}

	if (!dlgInsert.m_csCircuit2.IsEmpty())
	{
		CheckForDuplication(csaCableSortOrder, dlgInsert.m_csCircuit2, conductorInfo.m_iIndex); 
		conductorInfo.m_csLayer = dlgInsert.AssignLayerForType(dlgInsert.m_csCircuit2);
		conductorInfo_Vector.push_back(conductorInfo); 
		iNoOfSegs++; 
	}

	if (!dlgInsert.m_csCircuit3.IsEmpty())
	{
		CheckForDuplication(csaCableSortOrder, dlgInsert.m_csCircuit3, conductorInfo.m_iIndex); 
		conductorInfo.m_csLayer = dlgInsert.AssignLayerForType(dlgInsert.m_csCircuit3);
		conductorInfo_Vector.push_back(conductorInfo); 
		iNoOfSegs++; 
	}

	if (!dlgInsert.m_csCircuit4.IsEmpty()) 
	{
		CheckForDuplication(csaCableSortOrder, dlgInsert.m_csCircuit4, conductorInfo.m_iIndex);
		conductorInfo.m_csLayer = dlgInsert.AssignLayerForType(dlgInsert.m_csCircuit4);
		conductorInfo_Vector.push_back(conductorInfo); 
		iNoOfSegs++; 
	}

	// Sorts the conductor collection based on the how the different conductor types are to be drawn.
	sortConductorInfo(conductorInfo_Vector);

	AcCmColor color; color.setColor(AcCmEntityColor::kByLayer);
	ads_point ptStart, ptEnd;
		
	double dAngleAway; 
	double dAngleClose; 
	ads_point ptOne; acutPolar(ptInsert, dAngle - (PI / 2), dSpacing * 0.5, ptOne);
	ads_point ptTwo; acutPolar(ptInsert, dAngle + (PI / 2), dSpacing * 0.5, ptTwo);
	if (acutDistance(ptOne, ptPickPoint) < acutDistance(ptTwo, ptPickPoint)) 
	{
		dAngleClose = dAngle - (PI / 2);
		dAngleAway  = dAngle + (PI / 2);
	}
	else
	{
		dAngleClose = dAngle + (PI / 2);
		dAngleAway = dAngle - (PI / 2);
	}

	for (int iRow = 0; iRow < conductorInfo_Vector.size(); iRow++)
	{
		// Draw a cable for 3M on either side of the inserted pole
		if (iNoOfSegs == 1)
		{
			acutPolar(ptInsert, 0.0, 0.0, ptStart); 
		}
		else if (iNoOfSegs == 2)
		{
					 if (iRow == 0) { acutPolar(ptInsert, dAngleClose, dSpacing * 0.5, ptStart); }
			else if (iRow == 1) { acutPolar(ptInsert, dAngleAway,  dSpacing * 0.5, ptStart); }
		}
		else if (iNoOfSegs == 3)
		{
					 if (iRow == 0) { acutPolar(ptInsert, dAngleClose, dSpacing * 0.5, ptStart); }
			else if (iRow == 1) { acutPolar(ptInsert, 0.0,                    0.0, ptStart); }
			else if (iRow == 2) { acutPolar(ptInsert, dAngleAway,  dSpacing * 0.5, ptStart); }
		}
		else if (iNoOfSegs == 4)
		{
					 if (iRow == 0) { acutPolar(ptInsert, dAngleClose, dSpacing * 1.5, ptStart); }
			else if (iRow == 1) { acutPolar(ptInsert, dAngleClose, dSpacing * 0.5, ptStart); }
			else if (iRow == 2) { acutPolar(ptInsert, dAngleAway,  dSpacing * 0.5, ptStart); }
			else if (iRow == 3) { acutPolar(ptInsert, dAngleAway,  dSpacing * 1.5, ptStart); }
		}
		
		// End point of the cable
		acutPolar(ptStart, dAngle + PI, 3.0, ptStart);
		acutPolar(ptStart, dAngle,	    6.0, ptEnd);
		
		// Add the first and second vertex
		AcDbPolyline *pPline = new AcDbPolyline();
		pPline->addVertexAt(0, AcGePoint2d(ptStart[X], ptStart[Y]), 0.0);
		pPline->addVertexAt(1, AcGePoint2d(ptEnd[X], ptEnd[Y]), 0.0);

		//////////////////////////////////////////////////////////////////////////
		// Set the layer (If it does not exist, create it with the properties
		// specified in the database.
		//////////////////////////////////////////////////////////////////////////
		createLayer(conductorInfo_Vector[iRow].m_csLayer, Adesk::kFalse, Adesk::kFalse);
		pPline->setLayer(conductorInfo_Vector[iRow].m_csLayer);
		pPline->setLinetype(_T("BYLAYER"));
		pPline->setColor(color);

		appendEntityToDatabase(pPline);
		pPline->close();
	}
}


