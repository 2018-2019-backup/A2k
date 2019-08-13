// CreateLayoutPathDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CreateLayoutPathDlg.h"
#include "resource.h"

// CCreateLayoutPathDlg dialog

IMPLEMENT_DYNAMIC(CCreateLayoutPathDlg, CDialog)

CCreateLayoutPathDlg::CCreateLayoutPathDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateLayoutPathDlg::IDD, pParent)
{
	m_csNoOfCables = L"1";
}

CCreateLayoutPathDlg::~CCreateLayoutPathDlg()
{
}

void CCreateLayoutPathDlg::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OFS_USAGE, m_cbUsage);
	DDX_Control(pDX, IDC_OFS_VOLTAGE, m_cbVoltage);
	DDX_Control(pDX, IDC_OFS_STATUS, m_cbStatus);
	DDX_Control(pDX, IDC_OFS_OFFSET, m_cbOffset);
	DDX_CBString(pDX, IDC_OFS_OFFSET, m_csOffset);
	DDX_CBString(pDX, IDC_OFS_USAGE, m_csUsage);
	DDX_CBString(pDX, IDC_OFS_VOLTAGE, m_csVoltage);
	DDX_CBString(pDX, IDC_OFS_STATUS, m_csStatus);
	DDX_CBString(pDX, IDC_OFS_CONSTRUCTION, m_csConstruction);
	DDX_CBString(pDX, IDC_OFS_NOOFCABLES, m_csNoOfCables);
	DDX_Control(pDX, IDC_OFS_CONSTRUCTION, m_cbConstruction);
	DDX_Control(pDX, IDC_OFS_NOOFCABLES, m_cbNumberOfCables);
}


BEGIN_MESSAGE_MAP(CCreateLayoutPathDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_OFS_USAGE, &CCreateLayoutPathDlg::OnCbnSelchangeOfsUsage)
	ON_BN_CLICKED(IDOK, &CCreateLayoutPathDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDHELP, &CCreateLayoutPathDlg::OnBnClickedHelp)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CCreateLayoutPathDlg message handlers
//////////////////////////////////////////////////////////////////////////
BOOL CCreateLayoutPathDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// Get the standard values for the controls in the dialog
	CQueryTbl tblOFSPrompts;
	if (!tblOFSPrompts.SqlRead(DSN_DWGTOOLS, _T("SELECT For, Prompt1, Prompt2, LayerName_Cmp FROM tblOFSPrompts ORDER BY ID"), __LINE__, __FILE__, "",true)) return TRUE;
	tblOFSPrompts.GetColumnAt(0, m_csaOFSFor);
	tblOFSPrompts.GetColumnAt(1, m_csaOFSPmpt1);
	tblOFSPrompts.GetColumnAt(2, m_csaOFSPmpt2);
	tblOFSPrompts.GetColumnAt(3, m_csaOFSLayer);

	// Populate number of cable information
	CQueryTbl tblMaxCables;
	CString csSQL;
	csSQL.Format(L"SELECT fldNumberOfCables FROM tblMaxCables ORDER BY ID");
	if (!tblMaxCables.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, L"CCreateLayoutPathDlg::OnInitDialog()",true)) return TRUE;
	for (int i = 0; i < tblMaxCables.GetRows(); i++) m_cbNumberOfCables.AddString(tblMaxCables.GetRowAt(i)->GetAt(0));
	m_cbNumberOfCables.SelectString(-1, m_csNoOfCables);

	// Populate offset values
	CQueryTbl tblOffset;
	csSQL.Format(L"SELECT fldOffsetValue FROM tblOffset ORDER BY fldSequence");
	if (!tblOffset.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, L"CCreateLayoutPathDlg::OnInitDialog()",true)) return TRUE;
	for (int i = 0; i < tblOffset.GetRows(); i++) m_cbOffset.AddString(tblOffset.GetRowAt(i)->GetAt(0));
	
	// Populate relevant controls with the values retrieved
	for (int iRow = 0; iRow < m_csaOFSFor.GetCount(); iRow++) 
	{
		/**/ if (!m_csaOFSFor.GetAt(iRow).CompareNoCase(_T("UseAndVolt")))
		{
			// Only DISTINCT values are populated
			if (m_cbUsage.FindStringExact(-1, m_csaOFSPmpt1.GetAt(iRow)) == -1)	m_cbUsage.AddString(m_csaOFSPmpt1.GetAt(iRow));
		}
		else if (!m_csaOFSFor.GetAt(iRow).CompareNoCase(_T("Construction"))) m_cbConstruction.AddString(m_csaOFSPmpt1.GetAt(iRow));
		else if (!m_csaOFSFor.GetAt(iRow).CompareNoCase(_T("Status")))       m_cbStatus.AddString(m_csaOFSPmpt1.GetAt(iRow));
	}

	//////////////////////////////////////////////////////////////////////////
	// Defaults for controls
	//////////////////////////////////////////////////////////////////////////
	struct resbuf *rbpDefault = getXRecordFromDictionary(_T("eCapture"), _T("OFS Defaults"));
	if (rbpDefault) 
	{
		// No. of cables
		m_csNoOfCables = rbpDefault->resval.rstring; 
		SetDlgItemText(IDC_OFS_NOOFCABLES, m_csNoOfCables); 
		m_cbNumberOfCables.SelectString(-1, m_csNoOfCables);
		
		// Offset
		rbpDefault = rbpDefault->rbnext; 
		m_csOffset = rbpDefault->resval.rstring; 
		m_cbOffset.SelectString(-1, m_csOffset);        
		
		// Usage  
		rbpDefault = rbpDefault->rbnext;
		m_csUsage = rbpDefault->resval.rstring;
		m_cbUsage.SelectString(-1, m_csUsage); OnCbnSelchangeOfsUsage(); 

		// Voltage
		rbpDefault = rbpDefault->rbnext; 
		m_csVoltage = rbpDefault->resval.rstring;
		m_cbVoltage.SelectString(-1, m_csVoltage); 

		// Construction
		rbpDefault = rbpDefault->rbnext; 
		m_csConstruction = rbpDefault->resval.rstring;
		m_cbConstruction.SelectString(-1, m_csConstruction);  
		
		// Status
		rbpDefault = rbpDefault->rbnext; 
		m_csStatus = rbpDefault->resval.rstring;
		m_cbStatus.SelectString(-1, m_csStatus); 

		acutRelRb(rbpDefault);
	} 
	else
	{
		m_cbUsage.SetCurSel(0);  OnCbnSelchangeOfsUsage();
		m_cbStatus.SetCurSel(1);
		m_cbOffset.SetCurSel(0);
		m_cbConstruction.SetCurSel(0);
	}
				
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateLayoutPathDlg::OnCbnSelchangeOfsUsage()
{
	// Get usage selected
	int iSel = m_cbUsage.GetCurSel();
	if (iSel == CB_ERR) return;

	m_cbUsage.GetLBText(iSel, m_csUsage);
		
	// Populate the voltage combo for this usage
	m_cbVoltage.ResetContent();
	for (int iCtr = 0; iCtr < m_csaOFSPmpt1.GetCount(); iCtr++)
	{
		if (!m_csaOFSPmpt1.GetAt(iCtr).CompareNoCase(m_csUsage) && !m_csaOFSPmpt2.GetAt(iCtr).IsEmpty()) m_cbVoltage.AddString(m_csaOFSPmpt2.GetAt(iCtr));
	}

	// Enable the voltage button (if selection exists)
	if (m_cbVoltage.GetCount()) m_cbVoltage.EnableWindow(TRUE); else m_cbVoltage.EnableWindow(FALSE); m_cbVoltage.SetCurSel(0);
}

void CCreateLayoutPathDlg::OnBnClickedOk()
{
	// Validate the dialog inputs
	UpdateData(); 

	// No. of cables
	if (m_cbNumberOfCables.GetCurSel() == CB_ERR) { ShowBalloon(_T("Specify number of cables"), this, IDC_OFS_NOOFCABLES); return; }
	m_cbNumberOfCables.GetLBText(m_cbNumberOfCables.GetCurSel(), m_csNoOfCables);
	
	//////////////////////////////////////////////////////////////////////////
	// Layer for offsets
	//////////////////////////////////////////////////////////////////////////
	// Usage and Voltage
	int iCtr;
	for (iCtr = 0; iCtr < m_csaOFSPmpt1.GetCount(); iCtr++)
	{
		// Usage
		if (m_csaOFSPmpt1.GetAt(iCtr).CompareNoCase(m_csUsage)) continue;
		
		m_csLayer.Empty();
		m_csLayer = m_csaOFSLayer.GetAt(iCtr);

		// Voltage
		if (!m_cbVoltage.IsWindowEnabled()) break;

		if (m_csaOFSPmpt2.GetAt(iCtr).CompareNoCase(m_csVoltage)) continue;

		m_csLayer.Empty();
		m_csLayer = m_csaOFSLayer.GetAt(iCtr);
		break;
	}

	// Construction
	for (; iCtr < m_csaOFSPmpt1.GetCount(); iCtr++)
	{
		if (m_csaOFSPmpt1.GetAt(iCtr).CompareNoCase(m_csConstruction)) continue;
		m_csLayer.Format(_T("%s%s"), m_csLayer, m_csaOFSLayer.GetAt(iCtr));
		break;
	}

	// Status
	for (; iCtr < m_csaOFSPmpt1.GetCount(); iCtr++)
	{
		if (m_csaOFSPmpt1.GetAt(iCtr).CompareNoCase(m_csStatus)) continue;
		m_csLayer.Format(_T("%s%s"), m_csLayer, m_csaOFSLayer.GetAt(iCtr));
		acutPrintf(_T("\nLayer for new conductors \"%s\"\n"), m_csLayer);
		break;
	}

	// Check if the layer exists
	AcDbLayerTable *pLayerTbl;
	AcDbObjectId objLayer;

	// Get this drawing's layer table pointer
	acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead);
	if (!(pLayerTbl->has(m_csLayer)))
	{
		pLayerTbl->close();

		// Create the layer to place the offsets
		createLayer(m_csLayer, Adesk::kFalse, Adesk::kFalse);
	}
	pLayerTbl->close();

	// Add the values selected to the dictionary
	struct resbuf *rbpDefault = acutBuildList(AcDb::kDxfText, m_csNoOfCables, 
																						AcDb::kDxfText, m_csOffset, 
																						AcDb::kDxfText, m_csUsage,
																						AcDb::kDxfText, m_csVoltage,
  																					AcDb::kDxfText, m_csConstruction,
																						AcDb::kDxfText, m_csStatus, NULL);

	acdbRegApp(L"OFS Defaults");
	addXRecordToDictionary(L"eCapture", L"OFS Defaults", rbpDefault);
	
	OnOK();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: append()
// Description  : Adds the given entity to drawing database. The added entity is placed of the given layer before appending to database.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool append(AcDbEntity* pEntity, CString csLayer)
{
	// Get the current document pointer
	AcDbBlockTable *pBlockTable;
	AcApDocument* pDoc = acDocManager->curDocument();

	// Lock the document to append the entity
	Acad::ErrorStatus es = acDocManager->lockDocument(pDoc);
	if (es != Acad::eOk) { acutPrintf(_T("\nERROR: %s"), _T("Failed to lock the document."));	return false; }

	// Check if the layer exists
	if (!acdbTblSearch(L"LAYER", csLayer, FALSE)) UpdateLayerFromStandards(csLayer);

	// Get the database pointer
	AcDbDatabase* pDb = pDoc->database();
	es = pDb->getBlockTable(pBlockTable, AcDb::kForRead);
	if (es != Acad::eOk) { acutPrintf(_T("\nERROR: %s"), _T("Failed to get block table.")); return false;	}

	// Get the block table record pointer
	AcDbBlockTableRecord *pBlockRec;
	es = pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockRec, AcDb::kForWrite);
	if (es != Acad::eOk) { acutPrintf(_T("\nERROR: %s"), _T("Failed to get block table record."));	pBlockTable->close();	return false; }

	// Append the entity to the block table
	pEntity->setLayer(csLayer);

	AcCmColor color; color.setColor(AcCmEntityColor::kByLayer);
	pEntity->setColor(color);
	pEntity->setLineWeight(AcDb::kLnWtByLayer);
	pEntity->setLinetype(L"BYLAYER");

	es = pBlockRec->appendAcDbEntity(pEntity);
	if (es != Acad::eOk) { acutPrintf(_T("\nERROR: %s"), _T("Failed to append entity."));	pBlockTable->close();	pBlockRec->close();	delete pEntity;	return false; }
	
	pBlockRec->close();
	pBlockTable->close();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: getSideFactorForSegment()
// Description  : Determine if a positive or negative offset is required to put it close to the "Side".
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int getSideFactorForSegment(AcDbCurve *pCurve, double dOffset, ads_point ptSide)
{
	int iSideFactor = 1;

	// Append the resultant entity to the database
	AcDbCurve *pOffsetPositive = NULL;
	AcDbCurve *pOffsetNegative = NULL;

	AcGePoint3d geStartPositive;
	AcGePoint3d geStartNegative;

	AcGePoint3d geEndPositive;
	AcGePoint3d geEndNegative;
	
	AcGePoint3d geClosePositive;
	AcGePoint3d geCloseNegative;

	AcDbVoidPtrArray ar_Positives;
	AcDbVoidPtrArray ar_Negatives;

	Acad::ErrorStatus es;
	if (pCurve)
	{		
		es = pCurve->getOffsetCurves(dOffset, ar_Positives);
		if (es != Acad::eOk) return 1;
		if (ar_Positives.length() != 1) return 1;

		es = pCurve->getOffsetCurves(-1 * dOffset, ar_Negatives);
		if (es != Acad::eOk) return 1;
		if (ar_Negatives.length() != 1) return 1;

		pOffsetPositive = (AcDbCurve*)(ar_Positives[0]);
		pOffsetNegative = (AcDbCurve*)(ar_Negatives[0]);

		pOffsetPositive->getStartPoint(geStartPositive);
		pOffsetNegative->getStartPoint(geStartNegative);
		
		if (acutDistance(asDblArray(geStartPositive), ptSide) > acutDistance(asDblArray(geStartNegative), ptSide)) { return -1; }
	}
	
	return 1;
}

////////////////////////////////////////////////////////////////////////
// Function name: collectVertices()
// Description  : Collects the vertices of the given polyline.
////////////////////////////////////////////////////////////////////////
void collectVertices(const AcDb2dPolyline* pline, AcGePoint3dArray& pts, AcGeDoubleArray& bulges, bool asWcsPts)
{
	ASSERT (pline != NULL);
	ASSERT (pts.isEmpty() && bulges.isEmpty());

	AcDbObjectIterator* vertexIter = pline->vertexIterator();
	ASSERT(vertexIter != NULL);
	if (vertexIter == NULL)	return;

	AcDb2dVertex* vertex;
	for (; !vertexIter->done(); vertexIter->step()) 
	{
		if (acdbOpenObject(vertex, vertexIter->objectId(), AcDb::kForRead) == Acad::eOk) 
		{
			if (vertex->vertexType() != AcDb::k2dSplineCtlVertex) 
			{
				if (asWcsPts)
					pts.append(pline->vertexPosition(*vertex)); // returns WCS
				else
					pts.append(vertex->position());             // returns ECS
				bulges.append(vertex->bulge());
			}

			vertex->close();
		}
	}

	delete vertexIter;

	ASSERT(pts.isEmpty() == Adesk::kFalse);

	if (pline->isClosed()) 
	{
		AcGePoint3d tmpPt = pts[0]; // used to be a bug in dynamic arrays (not sure if its still there??)
		pts.append(tmpPt);
		bulges.append(0.0);
	}
}

////////////////////////////////////////////////////////////////////////
// Function name: collectVerticesForSpline()
// Description  : Collects the vertices of the given SPLINE.
////////////////////////////////////////////////////////////////////////
void collectVerticesForSpline(const AcDbSpline* pline, AcGePoint3dArray& pts)
{
	ASSERT (pline != NULL);
	ASSERT (pts.isEmpty());
	
	AcGePoint3d gePt;
	for (int iInd = 0; iInd < pline->numControlPoints(); iInd++) 
	{
		pline->getControlPointAt(iInd, gePt);
		pts.append(gePt);
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: getSideFactor
// Description  :
//////////////////////////////////////////////////////////////////////////
int getSideFactor(AcDbCurve *pCurve, double dOffset, ads_point ptPickPoint)
{
	int iSideFactor = 0;
	AcGePoint3d gePtOnCurve;
			
	//////////////////////////////////////////////////////////////////////////
	// Get the curve with a positive offset
	//////////////////////////////////////////////////////////////////////////
	AcDbVoidPtrArray ar_Positives;
	Acad::ErrorStatus es = pCurve->getOffsetCurves(dOffset, ar_Positives);
	if (es != Acad::eOk) { acutPrintf(L"\nError @%d[%s]", __LINE__, acadErrorStatusText(es)); return 0; }
	
	//////////////////////////////////////////////////////////////////////////
	// Get the curve with a negative offset
	//////////////////////////////////////////////////////////////////////////
	AcDbVoidPtrArray ar_Negatives;
	es = pCurve->getOffsetCurves(dOffset * -1, ar_Negatives);
	if (es != Acad::eOk) { acutPrintf(L"\nError @%d[%s]", __LINE__, acadErrorStatusText(es)); return 0; }
	
	AcDbCurve *pCurvePositive = (AcDbCurve *)ar_Positives.at(0);
	AcDbCurve *pCurveNegative = (AcDbCurve *)ar_Negatives.at(0);

	//////////////////////////////////////////////////////////////////////////////////////
	// Get the distance b/w pick point and point on both positive/negative offsets.
	// The curve that gives the shortest distance will decide the side factor to offset.
	//////////////////////////////////////////////////////////////////////////////////////

	es = pCurvePositive->getClosestPointTo(AcGePoint3d(ptPickPoint[X], ptPickPoint[Y], 0.0), gePtOnCurve);
	if (es != Acad::eOk) 
	{
		deleteArray(ar_Positives);
		deleteArray(ar_Negatives);

		if (es != Acad::eOk) { acutPrintf(L"\nError @%d[%s]", __LINE__, acadErrorStatusText(es)); return 0; }
		return 0;
	}
	double dPositive = acutDistance(asDblArray(gePtOnCurve), ptPickPoint);

	es = pCurveNegative->getClosestPointTo(AcGePoint3d(ptPickPoint[X], ptPickPoint[Y], 0.0), gePtOnCurve);
	if (es != Acad::eOk) 
	{
		deleteArray(ar_Positives);
		deleteArray(ar_Negatives);

		if (es != Acad::eOk) { acutPrintf(L"\nError @%d[%s]", __LINE__, acadErrorStatusText(es)); return 0; }
		return 0.0;
	}
	double dNegative = acutDistance(asDblArray(gePtOnCurve), ptPickPoint);

	deleteArray(ar_Positives);
	deleteArray(ar_Negatives);

	if (dPositive <= dNegative) iSideFactor = 1; else iSideFactor = -1;
	return iSideFactor;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Function name: getCurveToOffset()
// Description  : Returns the curve representing the given entity at offset specified.
///////////////////////////////////////////////////////////////////////////////////////////////
AcDbCurve *getCurveToOffset(AcDbEntity *pEntity, double dOffset, ads_point ptDummy, ads_point ptSide, int &iSideFactor)
{
	// Check for a LINE/ARC
	iSideFactor = 0;
	AcGePoint3d gePtOnCurve;
	AcDbCurve *pCurve = (AcDbCurve *) pEntity;

	if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbSpline::desc()))
	{
		// LINE/ARC entities are to be offset completely, so just determine the side to offset and exit the function
		pEntity->close();

		iSideFactor = getSideFactor(pCurve, dOffset, ptSide);
		return pCurve;
	}
	else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
	{
		AcDb2dPolyline *pPLine = AcDb2dPolyline::cast(pEntity);
		pEntity->close();

		// Determine the side factor to offset the curve
		iSideFactor = getSideFactor(pCurve, dOffset, ptSide);

		if (!pPLine->isClosed())
		{
			// POLYLINE that is OPEN are to be offset completely, so just determine the side to offset and exit the function
			pPLine->close();
			return pCurve;
		}

		//////////////////////////////////////////////////////////////////////////
		// For CLOSED POLYLINE we must determine the segment selected to offset
		//////////////////////////////////////////////////////////////////////////
		AcGePoint3dArray gePtsArray;
		AcGeDoubleArray geDblArray;
		collectVertices(pPLine, gePtsArray, geDblArray, true);

		// Get the end points of linear polyline
		pPLine->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0), gePtOnCurve);
		pPLine->close();
		AcGePoint3d geStart;
		AcGePoint3d geEnd;

		CString csAng1; 
		CString csAng2;
		for (int iSeg = 0; iSeg < gePtsArray.length() - 1; iSeg++)
		{
			if (!geDblArray.at(iSeg))
			{
				//////////////////////////////////////////////////////////////////////////
				// Handling LINE segments of the PLOYLINE
				//////////////////////////////////////////////////////////////////////////
				// Get the start and the end vertex of this segment
				geStart = gePtsArray.at(iSeg);
				geEnd = gePtsArray.at(iSeg + 1);

				// Get the angle that this makes with the pick point
				csAng1.Format(_T("%.2f"), RTD(acutAngle(asDblArray(geStart), asDblArray(gePtOnCurve))));
				csAng2.Format(_T("%.2f"), RTD(acutAngle(asDblArray(gePtOnCurve), asDblArray(geEnd))));

				// If they are same, this segment must is the one selected
				if (csAng1 == csAng2)
				{
					AcDbLine* pLine = new AcDbLine(geStart, geEnd);
					AcDbCurve *pLCurve = (AcDbCurve *) pLine;
					iSideFactor = getSideFactorForSegment(pLine, dOffset, ptSide);
					
					pCurve = (AcDbCurve *) pLine;
					pLine->close();
					return pCurve;
				}
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// Handling ARC segments of the POLYLINE
				//////////////////////////////////////////////////////////////////////////
				// Determine the start and the end points of this segment
				AcGePoint3d geStartPt(gePtsArray.at(iSeg).x, gePtsArray.at(iSeg).y, 0.0);
				AcGePoint3d geEndPt(gePtsArray.at(iSeg + 1).x, gePtsArray.at(iSeg + 1).y, 0.0);
				
				// Get the radius of the arc segment
				double chord = sqrt(pow(abs(geStartPt.x - geEndPt.x), 2) + pow(abs(geStartPt.y - geEndPt.y), 2));
				double s = chord / 2 * geDblArray.at(iSeg);
				double radius = (pow(chord / 2, 2) + pow(s, 2)) / (2 * s);

				// Draw the ARC
				if (geDblArray.at(iSeg) > 0)
					acedCommandS(RTSTR, L".ARC", RTPOINT, asDblArray(geStartPt), RTSTR, L"E", RTPOINT, asDblArray(geEndPt), RTSTR, L"R", RTREAL, fabs(radius), NULL);
				else
					acedCommandS(RTSTR, L".ARC", RTPOINT, asDblArray(geEndPt), RTSTR, L"E", RTPOINT, asDblArray(geStartPt), RTSTR, L"R", RTREAL, fabs(radius), NULL);

				ads_name enArc; acdbEntLast(enArc); AcDbObjectId objId; acdbGetObjectId(objId, enArc);

				// Get the arc's curve pointer to determine if this is the segment selected for offset
				if (acdbOpenObject(pCurve, objId, AcDb::kForRead) == Acad::eOk) 
				{
					// Determine if this is the segment to offset
					double dParam = 0.0;
					Acad::ErrorStatus es = pCurve->getParamAtPoint(gePtOnCurve, dParam);
					if (es == Acad::eOk) return pCurve;
				}

				// Erase the ARC
				acdbEntDel(enArc);
			}
		}

		return pCurve;
	}
	else if (pEntity->isKindOf(AcDbPolyline::desc()))
	{
		// Determine the side factor to offset the curve
		iSideFactor = getSideFactor(pCurve, dOffset, ptSide);

		AcDbPolyline *pPLine = AcDbPolyline::cast(pEntity);
		pEntity->close();

		if (!pPLine->isClosed())
		{
			// POLYLINE that is OPEN are to be offset completely, so just determine the side to offset and exit the function
			pPLine->close();
			return pCurve;
		}

		double dParam;
		AcGeLineSeg2d pLineSeg;
		AcGePoint2d geStart, geEnd, geCen; 
				
		// Get the point on the entity that is the closest to the pick point
		pPLine->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0), gePtOnCurve);

		// Get the end points of linear polyline
		for (int iSeg = 0; iSeg < pPLine->numVerts(); iSeg++)
		{
			if (Adesk::kFalse == pPLine->onSegAt(iSeg, AcGePoint2d(gePtOnCurve.x, gePtOnCurve.y), dParam)) continue;
			
			pPLine->getBulgeAt(iSeg, dParam);
			if (dParam != 0)
			{
				// Define the arc segment that has to be offset
				AcGeCircArc2d arcSeg; pPLine->getArcSegAt(iSeg, arcSeg);

				// Arc segment instance
				geStart = arcSeg.startPoint();
				geEnd	  = arcSeg.endPoint();
				geCen   = arcSeg.center();

				double dStartAng = acutAngle(asDblArray(geCen), asDblArray(geStart));
				double dEndAng	 = acutAngle(asDblArray(geCen), asDblArray(geEnd));

				AcDbArc *pArc = new AcDbArc();
				pArc->setCenter(AcGePoint3d(geCen.x, geCen.y, 0.0));
				pArc->setRadius(arcSeg.radius());

				if (!arcSeg.isClockWise())
				{
					pArc->setStartAngle(dStartAng);
					pArc->setEndAngle(dEndAng);
				}
				else
				{
					pArc->setStartAngle(dEndAng);
					pArc->setEndAngle(dStartAng);
				}
										
				pCurve = (AcDbCurve *) pArc;

				// Determine the side factor to offset the curve
				iSideFactor = getSideFactor(pCurve, dOffset, ptSide);

				pArc->close();
				pPLine->close();
				
				return pCurve;
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// Define the line segment that has to be offset
				//////////////////////////////////////////////////////////////////////////
				AcGeLineSeg2d lineSeg; pPLine->getLineSegAt(iSeg, lineSeg);
				geStart = lineSeg.startPoint();
				geEnd	  = lineSeg.endPoint();
								
				AcDbLine *pLine = new AcDbLine(AcGePoint3d(geStart.x, geStart.y, 0.0), AcGePoint3d(geEnd.x, geEnd.y, 0.0));
				pCurve = (AcDbCurve *) pLine;
				pLine->close();
				pPLine->close();

				// Determine the side factor to offset the curve
				iSideFactor = getSideFactor(pCurve, dOffset, ptSide);
				return pCurve;
			}
		}
		
		pPLine->close();

		// Determine the side factor to offset the curve
		iSideFactor = getSideFactor(pCurve, dOffset, ptSide);

		return pCurve;
	}
	
	pEntity->close();
	return pCurve;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Function name: getSegmentSelected()
// Description  : Gets the object ID of the segment on which the given point lies.
/////////////////////////////////////////////////////////////////////////////////////////////
AcDbObjectId getSegmentSelected(AcDbEntity *pEntity, ads_point ptDummy)
{
	AcDbObjectId objResult;

	// Check for a LINE/ARC
	AcGePoint3d gePtOnCurve;
	AcDbEntity *pSegment = NULL;
	AcGeMatrix3d matrix;
	Acad::ErrorStatus es;
	matrix.setToTranslation(AcGeVector3d(0.0, 0.0, 0.0));
	if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbSpline::desc()))
	{
		// LINE/ARC entities are to be offset completely
		es = pEntity->getTransformedCopy(matrix, pSegment);
		pEntity->close();
		if (es == Acad::eOk) 
		{
			appendEntityToDatabase(pSegment);
			objResult = pSegment->objectId();
			pSegment->close();
			return objResult;
		}
		else return AcDbObjectId::kNull;
	}
	else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
	{
		AcDb2dPolyline *pPLine = AcDb2dPolyline::cast(pEntity);

		if (!pPLine->isClosed())
		{
			// POLYLINE that is OPEN are to be offset completely
			es = pEntity->getTransformedCopy(matrix, pSegment);
			pEntity->close();

			if (es == Acad::eOk) 
			{
				appendEntityToDatabase(pSegment);
				objResult = pSegment->objectId();
				pSegment->close();
				return objResult;
			}
			else return AcDbObjectId::kNull;
		}

		pEntity->close();

		//////////////////////////////////////////////////////////////////////////
		// For CLOSED POLYLINE we must determine the segment selected to offset
		//////////////////////////////////////////////////////////////////////////
		AcGePoint3dArray gePtsArray;
		AcGeDoubleArray geDblArray;
		collectVertices(pPLine, gePtsArray, geDblArray, true);

		// Get the end points of linear polyline
		pPLine->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0), gePtOnCurve);
		pPLine->close();
		AcGePoint3d geStart;
		AcGePoint3d geEnd;

		CString csAng1; 
		CString csAng2;
		for (int iSeg = 0; iSeg < gePtsArray.length() - 1; iSeg++)
		{
			if (!geDblArray.at(iSeg))
			{
				//////////////////////////////////////////////////////////////////////////
				// Handling LINE segments of the PLOYLINE
				//////////////////////////////////////////////////////////////////////////
				// Get the start and the end vertex of this segment
				geStart = gePtsArray.at(iSeg);
				geEnd = gePtsArray.at(iSeg + 1);

				// Get the angle that this makes with the pick point
				csAng1.Format(_T("%.2f"), RTD(acutAngle(asDblArray(geStart), asDblArray(gePtOnCurve))));
				csAng2.Format(_T("%.2f"), RTD(acutAngle(asDblArray(gePtOnCurve), asDblArray(geEnd))));

				// If they are same, this segment must is the one selected
				if (csAng1 == csAng2)
				{
					AcDbLine* pLine = new AcDbLine(geStart, geEnd);
					pLine->setLayer(L"0");
					appendEntityToDatabase(pLine);

					pSegment = (AcDbEntity *) pLine;
					pLine->close();
					
					objResult = pSegment->objectId();
					pSegment->close();
					return objResult;
				}
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// Handling ARC segments of the POLYLINE
				//////////////////////////////////////////////////////////////////////////
				// Determine the start and the end points of this segment
				AcGePoint3d geStartPt(gePtsArray.at(iSeg).x, gePtsArray.at(iSeg).y, 0.0);
				AcGePoint3d geEndPt(gePtsArray.at(iSeg + 1).x, gePtsArray.at(iSeg + 1).y, 0.0);

				// Get the radius of the arc segment
				double chord = sqrt(pow(abs(geStartPt.x - geEndPt.x), 2) + pow(abs(geStartPt.y - geEndPt.y), 2));
				double s = chord / 2 * geDblArray.at(iSeg);
				double radius = (pow(chord / 2, 2) + pow(s, 2)) / (2 * s);

				// Draw the ARC
				if (geDblArray.at(iSeg) > 0)
					acedCommandS(RTSTR, L".ARC", RTPOINT, asDblArray(geStartPt), RTSTR, L"E", RTPOINT, asDblArray(geEndPt), RTSTR, L"R", RTREAL, fabs(radius), NULL);
				else
					acedCommandS(RTSTR, L".ARC", RTPOINT, asDblArray(geEndPt), RTSTR, L"E", RTPOINT, asDblArray(geStartPt), RTSTR, L"R", RTREAL, fabs(radius), NULL);
				ads_name enArc; acdbEntLast(enArc); AcDbObjectId objId; acdbGetObjectId(objId, enArc);

				// Get the arc's curve pointer to determine if this is the segment selected for offset
				AcDbCurve *pCurve;
				if (acdbOpenObject(pCurve, objId, AcDb::kForRead) == Acad::eOk) 
				{
					// Determine if this is the segment to offset
					double dParam = 0.0;
					Acad::ErrorStatus es = pCurve->getParamAtPoint(gePtOnCurve, dParam);
					if (es == Acad::eOk)
					{
						pSegment = (AcDbEntity *) pCurve;
						
						objResult = pSegment->objectId();
						pCurve->close();
						pSegment->close();
						return objResult;
					}
					else 
					{
						pCurve->close();
					}
				}

				// Erase the ARC
				acdbEntDel(enArc);
			}
		}

		return AcDbObjectId::kNull;
	}
	else if (pEntity->isKindOf(AcDbPolyline::desc()))
	{
		AcDbPolyline *pPLine = AcDbPolyline::cast(pEntity);
		pEntity->close();

		if (!pPLine->isClosed())
		{
			// POLYLINE that is OPEN are to be offset completely, so just determine the side to offset and exit the function
			es = pEntity->getTransformedCopy(matrix, pSegment);
			pEntity->close();
			
			if (es == Acad::eOk)
			{
				appendEntityToDatabase(pSegment);
				objResult = pSegment->objectId();
				pSegment->close();
				return objResult;
			}
			else AcDbObjectId::kNull;
		}

		pEntity->close();

		double dParam;
		AcGeLineSeg2d pLineSeg;
		AcGePoint2d geStart, geEnd, geCen; 

		// Get the point on the entity that is the closest to the pick point
		pPLine->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0), gePtOnCurve);

		// Get the end points of linear polyline
		for (int iSeg = 0; iSeg < pPLine->numVerts(); iSeg++)
		{
			if (Adesk::kFalse == pPLine->onSegAt(iSeg, AcGePoint2d(gePtOnCurve.x, gePtOnCurve.y), dParam)) continue;

			pPLine->getBulgeAt(iSeg, dParam);
			if (dParam != 0)
			{
				// Define the arc segment that has to be offset
				AcGeCircArc2d arcSeg; pPLine->getArcSegAt(iSeg, arcSeg);

				//////////////////////////////////////////////////////////////////////////
				// Arc segment instance
				//////////////////////////////////////////////////////////////////////////
				geStart = arcSeg.startPoint();
				geEnd	  = arcSeg.endPoint();
				geCen   = arcSeg.center();

				double dStartAng = acutAngle(asDblArray(geCen), asDblArray(geStart));
				double dEndAng	 = acutAngle(asDblArray(geCen), asDblArray(geEnd));

				AcDbArc *pArc = new AcDbArc();
				pArc->setCenter(AcGePoint3d(geCen.x, geCen.y, 0.0));
				pArc->setRadius(arcSeg.radius());

				if (!arcSeg.isClockWise())
				{
					pArc->setStartAngle(dStartAng);
					pArc->setEndAngle(dEndAng);
				}
				else
				{
					pArc->setStartAngle(dEndAng);
					pArc->setEndAngle(dStartAng);
				}

				appendEntityToDatabase(pArc);
				pArc->close();
				pPLine->close();

				pSegment = (AcDbEntity *) pArc;

				objResult = pSegment->objectId();
				pSegment->close();
				return objResult;
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// Define the line segment that has to be offset
				//////////////////////////////////////////////////////////////////////////
				AcGeLineSeg2d lineSeg; pPLine->getLineSegAt(iSeg, lineSeg);
				geStart = lineSeg.startPoint();
				geEnd	  = lineSeg.endPoint();

				AcDbLine *pLine = new AcDbLine(AcGePoint3d(geStart.x, geStart.y, 0.0), AcGePoint3d(geEnd.x, geEnd.y, 0.0));
				pLine->setLayer(L"0");
				appendEntityToDatabase(pLine);

				pSegment = (AcDbCurve *) pLine;
				pLine->close();
				pPLine->close();
				
				objResult = pSegment->objectId();
				pSegment->close();
				return objResult;
			}
		}

		pPLine->close();
		return AcDbObjectId::kNull;
	}

	pEntity->close();
	return AcDbObjectId::kNull;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: ChangeThickness
// Description  : Temporarily modifies the width of the contour selected to visualize the selection made.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ChangeThickness(ads_name enPline, double dThick)
{
	AcDbObjectId objID;
	acdbGetObjectId(objID, enPline);

	AcDbPolyline *pLine;
	if (acdbOpenObject(pLine, objID, AcDb::kForWrite) != Acad::eOk) return;

	pLine->setConstantWidth(dThick);
	pLine->close();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Function name: SelectContour
// Description  : Enables selection of a contour and other entities to join it.
//////////////////////////////////////////////////////////////////////////////////////////
int SelectContour(ads_point ptDummy, AcDbObjectId &objContour)
{
	// Allow user to select contour
	int iRet = 0;
	ads_name enContour;
	Acad::ErrorStatus es;
	AcDbEntity *pEntity = NULL;

	bool bIsSpline = false;
	while (T)
	{
		iRet = acedEntSel(_T("\rSelect existing contour: "), enContour, ptDummy);
		/**/ if (iRet == RTCAN)	return iRet;
		else if (iRet == RTNORM) 
		{
			// Check if the entity selected is a valid contour
			acdbGetObjectId(objContour, enContour);

			es = acdbOpenObject(pEntity, objContour, AcDb::kForRead);
			if (es != Acad::eOk) { acutPrintf(_T("\nError at %d: %s"), __LINE__, acadErrorStatusText(es)); return RTCAN; }

			if (!pEntity->isKindOf(AcDbLine::desc()) && !pEntity->isKindOf(AcDbSpline::desc()) && !pEntity->isKindOf(AcDbArc::desc()) && !pEntity->isKindOf(AcDbPolyline::desc()) && !pEntity->isKindOf(AcDb2dPolyline::desc()))
			{
				pEntity->close();

				// Not a valid contour
				acutPrintf(_T("\nNot a valid contour.\n"));
				continue;
			}

			// Make a copy of the selected contour and convert it to PLINE if required
			objContour = getSegmentSelected(pEntity, ptDummy);

			if (pEntity->isKindOf(AcDbSpline::desc())) bIsSpline = true;
			pEntity->close();
			break;
		}
	}

	// Add the contour selected to the database
	if (!objContour.isValid()) { return RTCAN; }
	
	// Open this entity to find out if it has to be converted to a PLINE
	AcDbEntity *pContour;

	acdbGetAdsName(enContour, objContour);
	acdbOpenObject(pContour, objContour, AcDb::kForRead);
	int iColor = pContour->colorIndex();

	if (pContour->isKindOf(AcDbLine::desc()) || pContour->isKindOf(AcDbArc::desc()))
	{
		pContour->close();

		// Modify the entity to a single contour
		acedCommandS(RTSTR, L".PEDIT", RTLB, RTENAME, enContour, RTPOINT, ptDummy, RTLE, RTSTR, L"Y", RTSTR, L"", NULL);
		acdbEntLast(enContour);

		// Get the combined entities object ID
		acdbGetObjectId(objContour, enContour);
		acdbOpenObject(pContour, objContour, AcDb::kForRead);
	}

	pContour->close();

	// Highlight the contour selected
	acedRedraw(enContour, 1);
	ChangeProperties(FALSE, enContour, ((iColor != 1) ? 1 : 2));
	ChangeThickness(enContour, 0.5);
	
	//////////////////////////////////////////////////////////////////////////
	// Select other entities to join to this contour
	//////////////////////////////////////////////////////////////////////////
	ads_name enSegment;
	ads_point ptSegment;
	AcDbObjectId objSegment;
	if (!bIsSpline) acutPrintf(L"\nSelect segments to join...");
	
	while (!bIsSpline)
	{
		// Highlight the entity resulting from a join. This statement immediately after a join is NOT working, hence we put it here.
		acdbGetObjectId(objSegment, enSegment);
		if (objSegment.isValid()) 
		{
			acedRedraw(enContour, 3);
			ChangeProperties(FALSE, enContour, ((iColor != 1) ? 1 : 2));
		}

		iRet = acedEntSel(_T("\nSelect segment:"), enSegment, ptSegment);
		
		/**/ if (iRet == RTCAN) return RTCAN;
		else if (iRet == RTNORM)
		{
			// Check if the entity selected is a valid contour
			acdbGetObjectId(objSegment, enSegment);
			es = acdbOpenObject(pEntity, objSegment, AcDb::kForRead);
			iColor = pEntity->colorIndex();
			if (es != Acad::eOk) { acutPrintf(_T("\nError at %d: %s"), __LINE__, acadErrorStatusText(es)); return RTCAN; }

			// Check if the selected entity is VALID
			if (!pEntity->isKindOf(AcDbLine::desc()) && !pEntity->isKindOf(AcDbSpline::desc()) && !pEntity->isKindOf(AcDbArc::desc()) && !pEntity->isKindOf(AcDbPolyline::desc()) && !pEntity->isKindOf(AcDb2dPolyline::desc()))
			{
				pEntity->close();

				// Not a valid contour
				acutPrintf(_T("\nNot a valid contour.\n"));
				continue;
			}

			// Make a copy of the selected contour and convert it to PLINE if required
			objSegment = getSegmentSelected(pEntity, ptSegment);
			pEntity->close();

			if (!objSegment.isValid()) continue;

			// Join the selected contour with already selected set of entities
			acdbGetAdsName(enSegment, objSegment);
			int iRetJoin = acedCommandS(RTSTR, L".PEDIT", RTLB, RTENAME, enContour, RTPOINT, ptDummy, RTLE, RTSTR, L"J", RTENAME, enSegment, RTSTR, L"", RTSTR, L"", NULL);
										
			// If the segment exists, then probably the segment exists as individual entity
			if (acdbEntGet(enSegment)) { acutPrintf(L"\r\nSelected segment cannot be joined!");	acdbEntDel(enSegment);	continue;	}
			
			// The selected segment has joined. Let us check if they have formed a closed polyline. If Yes, then we will have to open it
			acdbEntLast(enContour);
			acdbGetObjectId(objContour, enContour);

			AcDbPolyline *pPLine;
			if (acdbOpenObject(pPLine, objContour, AcDb::kForWrite) == Acad::eOk)
			{
				if (pPLine->isClosed())	pPLine->setClosed(Adesk::kFalse);
				pPLine->close();
			}

			// Highlight the entity selected
			acedRedraw(enContour, 3);
			ChangeProperties(FALSE, enContour, ((iColor != 1) ? 1 : 2));
			ChangeThickness(enContour, 0.5);
			
		}
		else if (iRet == RTERROR) 
		{
			AcDbPolyline *pPLine;
			if (acdbOpenObject(pPLine, objContour, AcDb::kForWrite) == Acad::eOk) pPLine->close();
			return RTNORM; 
		}
	}

	return RTNORM;
}

//////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedHelp()
// Description  : Calls the help window with the context displayed
//////////////////////////////////////////////////////////////////////////
void CCreateLayoutPathDlg::OnBnClickedHelp() { displayHelp((DWORD)_T("Draft_Offfset.htm")); }

//////////////////////////////////////////////////////////////////////////
// Function name: Command_OFS()
// Description  : Defines the DT_OFS command
//////////////////////////////////////////////////////////////////////////
void Command_OFS()
{
	// Switch off certain system variables
	switchOff();

	// Display the create layout path dialog
	CCreateLayoutPathDlg dlgOFS;
	if (dlgOFS.DoModal() == IDCANCEL) return;

	AcDbObjectId objContour;
	ads_name enContour;
	ads_point ptDummy;
		
	// Allow user to specify the offset distance (in case the offset in the dialog is "Other")
	int iRet;
	double dOffset = _tstof(dlgOFS.m_csOffset);
	if (!dlgOFS.m_csOffset.CompareNoCase(_T("Other")) || !dOffset)
	{
		while (T)
		{
			acedInitGet(RSG_NOZERO + RSG_NONEG, _T(""));

			iRet = acedGetReal(_T("\nSpecify offset distance <1>: "), &dOffset);
			/**/ if (iRet == RTCAN)  { acdbEntDel(enContour); return; }
			else if (iRet == RTNONE) { dOffset = 1.0; break; }
			else if (iRet == RTNORM) break;
		} 
	}
	
	// Allow the user to select the contour
	Acad::ErrorStatus es;
	AcDbEntity *pEntity;
	if (SelectContour(ptDummy, objContour) == RTCAN)
	{
		es = acdbOpenObject(pEntity, objContour, AcDb::kForWrite);
		if (es == Acad::eOk) { pEntity->erase(); pEntity->close(); }
		return; 
	}
	acdbGetAdsName(enContour, objContour);

	// Change the thickness assigned while selecting the contour and its sgements
	ChangeThickness(enContour, 0.0);
	
	// Allow user to pick a point to specify the side to offset
	ads_point ptSide;
	if (acedGetPoint(NULL, _T("\nPick side to offset: "), ptSide) == RTCAN) { acdbEntDel(enContour); return; }
	
	// Open the entity for processing
	if (acdbOpenObject(pEntity, objContour, AcDb::kForRead) != Acad::eOk) {	return; }
	
	// Offset the selected entity
	AcDbVoidPtrArray ar_Positives;
	AcDbCurve *pCurve = (AcDbCurve *) pEntity; 
		
	TCHAR result[10];
	iRet = RTNORM;

	AcDbCurve *pOffset = NULL;
	while (T)
	{
		AcDbObjectIdArray arObjIds;
		int iSideFactor = 1;
		
		//////////////////////////////////////////////////////////////////////////
		// Get the curve to offset
		//////////////////////////////////////////////////////////////////////////
		if (iRet != RTERROR) 
		{
			if (result[0] != 'F') { pCurve = getCurveToOffset(pEntity, dOffset, ptDummy, ptSide, iSideFactor); }
			pEntity->close();

			if (iSideFactor == 0)
			{
				// Abort all transactions
				while (acTransactionManagerPtr()->numActiveTransactions() > 0) acTransactionManagerPtr()->abortTransaction();
				if (acdbOpenObject(pEntity, objContour, AcDb::kForWrite) == Acad::eOk) { pEntity->erase(); pEntity->close(); }
				acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es));
				return; 
			}

			// Offset the selected curve for number of cable input specified
			acTransactionManagerPtr()->startTransaction();
			for (int iCtr = 0; iCtr < _tstoi(dlgOFS.m_csNoOfCables); iCtr++)
			{
				// Get the resultant curves of the offset
				ar_Positives.removeAll();
				es = pCurve->getOffsetCurves(iSideFactor * dOffset * (iCtr + 1), ar_Positives);
				pCurve->close();

				if (es != Acad::eOk) 
				{
					// Abort all transactions
					while (acTransactionManagerPtr()->numActiveTransactions() > 0) acTransactionManagerPtr()->abortTransaction();
					acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es));
					return; 
				}

				for (int iSeg = 0; iSeg < ar_Positives.length(); iSeg++)
				{
					// Append the resultant entity to the database
					pOffset = (AcDbCurve*)(ar_Positives[iSeg]);

					// Get the object id and append it to the temporary array
					append(pOffset, dlgOFS.m_csLayer);
					arObjIds.append(pOffset->objectId());
					pOffset->close();
				}
			}
		}
							
		// Check if the user wants to flip the side
		if (iRet != RTERROR)
		{
			acedInitGet(NULL, L"Flip Next Undo Exit");
			iRet = acedGetKword(L"\rSpecify [Next/Flip/Undo/Exit] <Exit>: ", result);
		}
		else
		{
			acedInitGet(NULL, L"Next Exit");
			iRet = acedGetKword(L"\rSpecify [Next/Exit] <Exit>: ", result);
		}

		if (iRet == RTNONE) { iRet = RTNORM; result[0] = 'E'; }
		if (iRet == RTNORM)
		{
			/**/ if (result[0] == 'N')
			{
				// Delete the previously selected contour
				if (acdbOpenObject(pEntity, objContour, AcDb::kForWrite) == Acad::eOk) { pEntity->erase(); pEntity->close(); }

				// End the previous transaction and start a new one
				acTransactionManagerPtr()->endTransaction();
				
				// Get the new contour
				if (SelectContour(ptDummy, objContour) != RTCAN)
				{
					// Open the entity for processing
					acdbGetAdsName(enContour, objContour);
					if (acdbOpenObject(pEntity, objContour, AcDb::kForRead) != Acad::eOk) return;

					// Allow user to pick a point to specify the side to offset
					if (acedGetPoint(NULL, _T("\nPick side to offset: "), ptSide) == RTCAN) 
					{
						if (pEntity->upgradeOpen() == Acad::eOk) { pEntity->erase(); pEntity->close(); }
						return;
					}
				}
			}
			else if (result[0] == 'F')
			{
				// Just change the sign of offset and everything else will fall in place
				dOffset *= -1;

				// Delete the existing contour and allow selection of a new contour
				acTransactionManagerPtr()->abortTransaction();
				if ((es = acdbOpenObject(pEntity, objContour, AcDb::kForRead)) != Acad::eOk) { acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es)); return; }
				// acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es));
			}
			else if (result[0] == 'U')
			{
				// Abort the previous offset and start a new one
				acTransactionManagerPtr()->abortTransaction();
				if (acdbOpenObject(pEntity, objContour, AcDb::kForWrite) == Acad::eOk) { pEntity->erase(); pEntity->close(); }
				iRet = RTERROR;
			}
			else if (result[0] == 'E')
			{
				// End all transactions
				acTransactionManagerPtr()->endTransaction();
				if (acdbOpenObject(pEntity, objContour, AcDb::kForWrite) == Acad::eOk) { pEntity->erase(); pEntity->close(); }
				return;
			}
		}
		else if (iRet == RTERROR) 
		{
			// The user pressed enter to Exit. End all transactions now.
			while (acTransactionManagerPtr()->numActiveTransactions() > 0) acTransactionManagerPtr()->endTransaction();
			return;
		}
		else if (iRet == RTCAN) 
		{
			// Abort all transactions
			while (acTransactionManagerPtr()->numActiveTransactions() > 0) acTransactionManagerPtr()->abortTransaction();
			es = acdbOpenObject(pEntity, objContour, AcDb::kForWrite);
			if (es == Acad::eOk) { pEntity->erase(); pEntity->close(); } 
			return; 
		}
	}
}
	

