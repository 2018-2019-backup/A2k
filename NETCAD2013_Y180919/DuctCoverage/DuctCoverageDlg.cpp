// DuctCoverageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DuctCoverageDlg.h"
extern int CreateHatchBoundary(AcDbObjectIdArray &aryObjIds);

//////////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////////
AcCmColor g_ByLayerColor; 
extern CString g_csProposedPattern;
extern double g_dProposedScale;
extern CString g_csUnderborePattern;
extern double g_dUnderboreScale; 

/////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Function name: append()
// Description  : Adds the given entity to drawing database.
//////////////////////////////////////////////////////////////////////////
bool append(AcDbEntity* pEntity, CString csLayer)
{
  // Get the current document pointer
  AcDbBlockTable *pBlockTable;
  AcApDocument* pDoc = acDocManager->curDocument();

  // Lock the document to append the entity
  Acad::ErrorStatus es = acDocManager->lockDocument(pDoc);
  if (es != Acad::eOk) { acutPrintf(_T("\nERROR: %s"), _T("Failed to lock the document."));	return false; }

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

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
bool ReferenceByPoints(AcDbObjectId &objAxisId, ads_point ptStart)
{
	// Enable the user to select a few points to define the coverage
	acTransactionManagerPtr()->startTransaction();

	AcDbObjectId objNewSegment;
	ptStart[X] = -99999.99;
	ptStart[Y] = -99999.99;
	ptStart[Z] = 0.0;

	while (T)
	{
		int iRetPts;
		ads_point ptNext;

		// Get the next point
		if ((ptStart[X] == -99999.99) && (ptStart[Y] == -99999.99))
		{
			acedInitGet(RSG_NONULL, L"");
			iRetPts = acedGetPoint(NULL, L"\nSpecify start point: ", ptStart);
			/**/ if (iRetPts == RTCAN) { acTransactionManagerPtr()->abortTransaction(); return false; }
			else continue;
		}
		else
			iRetPts = acedGetPoint(ptStart, L"\nSpecify next point or ENTER to finish: ", ptNext);

		/**/ if (iRetPts == RTCAN)  { acTransactionManagerPtr()->abortTransaction(); return false; }
		else if (iRetPts == RTNONE)
		{
			acTransactionManagerPtr()->endTransaction(); 
			break; 
		}

		// Draw the new LINE segment
		AcDbPolyline *pPtsLine = new AcDbPolyline();
		pPtsLine->addVertexAt(0, AcGePoint2d(ptStart[X], ptStart[Y]));
		pPtsLine->addVertexAt(1, AcGePoint2d(ptNext[X], ptNext[Y]));
		appendEntityToDatabase(pPtsLine);
		objNewSegment = pPtsLine->objectId();
		pPtsLine->close();

		// Set the next point as the new start point
		acutPolar(ptNext, 0.0, 0.0, ptStart);

		// Open the existing coverage and add the new points
		if (objAxisId.isValid())
		{
			// Get the entity name of the coverage
			ads_name enObjAxis; acdbGetAdsName(enObjAxis, objAxisId);

			// If the previous entity is a LINE we must covert it to a PLINE
			AcDbLine *pLine; 
			bool bAxisIsLine = false;
			Acad::ErrorStatus es  = acdbOpenObject(pLine, objAxisId, AcDb::kForRead);
			if (es != Acad::eNotThatKindOfClass) { bAxisIsLine = true; pLine->close(); }

			ads_name enNewSegment; acdbGetAdsName(enNewSegment, objNewSegment);
			saveOSMode();
			if (bAxisIsLine)
				acedCommandS(RTSTR, L".PEDIT", RTLB, RTENAME, enObjAxis, RTPOINT, ptStart, RTLE, RTSTR, L"Y", RTSTR, L"JOIN", RTENAME, enNewSegment, RTSTR, L"", RTSTR, L"", NULL);
			else
				acedCommandS(RTSTR, L".PEDIT", RTLB, RTENAME, enObjAxis, RTPOINT, ptStart, RTLE, RTSTR, L"JOIN", RTENAME, enNewSegment, RTSTR, L"", RTSTR, L"", NULL);
			restoreOSMode();

			// Get the object ID
			ads_name enAxis; acdbEntLast(enAxis);
			acdbGetObjectId(objAxisId, enAxis);
		}
		else
		{
			objAxisId = objNewSegment;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
bool SelectEntities(CString csType, AcDbObjectIdArray &aryObjIDs, AcGePoint3dArray &aryPickPts)
{
	aryObjIDs.removeAll();
	aryPickPts.removeAll();

	CString csPrompt;
	AcDbObjectId objSel;
	ads_point ptPick;
	ads_name enEntity;
	int iRet;

	while (T)
	{
		if (!csType.CompareNoCase(L"POINTS"))	
		{
			////////////////////////////
			// POINTS
			////////////////////////////
			// Allow user to select points to define the duct axis
			if (!ReferenceByPoints(objSel, ptPick)) return false;

			// Confirm whether the selected points are valid and has resulted in a valid entity type
			if (!objSel.isNull() && objSel.isValid() && !objSel.isErased())
			{
				aryObjIDs.append(objSel);
				aryPickPts.append(AcGePoint3d(ptPick[X], ptPick[Y], 0.0));
				return TRUE;
			}

			continue; 
		}

		////////////////////////////////////////////////////
		// LINE/POLYLINE/ARC/SPLINE/LWPOLYLINE
		////////////////////////////////////////////////////
		// If there are PRESELECTED Lines we can check for their validity and exit the function
		ads_name ssLines;
		int iRetI = acedSSGet(_T("I"), NULL, NULL, NULL, ssLines);
		if (iRetI == RTNORM)
		{
			//long lLength = 0L;
			int lLength = 0L;
			acedSSLength(ssLines, &lLength);

			for (long lCtr = 0L; lCtr < lLength; lCtr++)
			{
				// Open the object and verify whether it is a valid entity and type
				acedSSName(ssLines, lCtr, enEntity);
				if (acdbGetObjectId(objSel, enEntity) != Acad::eOk) continue;

				AcDbEntity *pEntity;
				if (acdbOpenObject(pEntity, objSel, AcDb::kForWrite) != Acad::eOk) continue;

				/**/ if (!csType.CompareNoCase(L"Line") && (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc()) || pEntity->isKindOf(AcDbSpline::desc())))
				{
					pEntity->close(); 

					// Check if the selected entity is already selected
					if (aryObjIDs.contains(objSel)) { acutPrintf(L"...%d line(s) selected (1 duplicate).", aryObjIDs.length()); continue; }

					aryObjIDs.append(objSel);
					aryPickPts.append(AcGePoint3d(ptPick[X], ptPick[Y], 0.0));

					acedRedraw(enEntity, 3);
					acutPrintf(L"...%d line(s) selected.", aryObjIDs.length());
					continue;
				}
				else pEntity->close();
			}

			if (aryObjIDs.length() > 0L) return TRUE;
		}

		while (T)
		{
			// Form the prompt to be displayed
			csPrompt.Format(L"\nSelect %s or ENTER to confirm: ", csType);

			// Enable selection
			int iRet = acedEntSel(csPrompt, enEntity, ptPick);
			/**/ if (iRet == RTCAN) return FALSE;
			else if ((iRet == -5001) || (iRet == 5000)) return TRUE;

			// Open the object and verify whether it is a valid entity and type
			if (acdbGetObjectId(objSel, enEntity) != Acad::eOk) continue;

			AcDbEntity *pEntity;
			if (acdbOpenObject(pEntity, objSel, AcDb::kForWrite) != Acad::eOk) continue;

			/**/ if (!csType.CompareNoCase(L"Line") && (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc()) || pEntity->isKindOf(AcDbSpline::desc())))
			{
				pEntity->close(); 

				// Check if the selected entity is already selected
				if (aryObjIDs.contains(objSel)) { acutPrintf(L"...%d line(s) selected (1 duplicate).", aryObjIDs.length()); continue; }

				aryObjIDs.append(objSel);
				aryPickPts.append(AcGePoint3d(ptPick[X], ptPick[Y], 0.0));

				acedRedraw(enEntity, 3);
				acutPrintf(L"...%d line(s) selected.", aryObjIDs.length());
				continue;
			}
			else pEntity->close();

			// Invalid selection
			acutPrintf(L"\nNot a valid entity.\n");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: ElaborateCoverage()
// Description  :
//////////////////////////////////////////////////////////////////////////
bool ElaborateCoverage(AcDbObjectId objPolyId, double dWidth, CString csLayer, AcDbObjectIdArray &aryObjIds, ads_point ptEnd, CString csStatus, CString csPosition, CString csType, double dOffset)
{
	if (objPolyId == AcDbObjectId::kNull) { acutPrintf(L"\nError 1"); return false; }

	/////////////////////////////////////////////////////////////////////////
	// Elaborate the coverage
	//////////////////////////////////////////////////////////////////////////
	AcDbEntity *pEntity;
	Acad::ErrorStatus es = acdbOpenObject(pEntity, objPolyId, AcDb::kForWrite);
	if (es != Acad::eOk) { acutPrintf(L"\nError 2 [%s]", acadErrorStatusText(es)); return false; }

	/*
	// Check if the entity selected is CLOSED
	bool bIsClosed = false;
	AcDb2dPolyline *p2DPline = AcDb2dPolyline::cast(pEntity);
	AcDbPolyline *pLWPline = AcDbPolyline::cast(pEntity);
	if (p2DPline) { bIsClosed = p2DPline->isClosed(); }
	if (pLWPline) { bIsClosed = pLWPline->isClosed(); }
	*/

	//////////////////////////////////////////////////////////////////////////
	// Offset axis polyline on +ve/-ve side of the axis for half trench width
	//////////////////////////////////////////////////////////////////////////
	AcDbVoidPtrArray ar_Offsets;
	AcGePoint3d geStart1, geEnd1;
	AcGePoint3d geStart2, geEnd2;
	AcDbObjectIdArray aryObjIdCollection;

	AcDbObjectId objIdOffset1;
	AcDbObjectId objIdOffset2;
	AcDbObjectId objIdEnd1;
	AcDbObjectId objIdEnd2;

	AcGePoint3d geStartPt;
	AcGePoint3d geEndPt;

	double dAngle = 0;
	for (int iCtr = 0; iCtr < 2; iCtr++)
	{
		// Get the curve pointer for the polyline
		AcDbCurve *pCurve = (AcDbCurve *) pEntity;

		// Remove existing offsets from the previous iteration
		ar_Offsets.removeAll();

		// Get the offsets for the polyline 
		es = pCurve->getOffsetCurves((!iCtr ? 1 : -1) * dWidth, ar_Offsets);

		// pEntity->close();
		pCurve->close();

		if (es == Acad::eOk) 
		{
			// Append the resultant entity to the database
			AcDbCurve *pOffset;
			pOffset = (AcDbCurve*)(ar_Offsets[0]);

			pOffset->getStartPoint(geStartPt);
			pOffset->getEndPoint(geEndPt);

			pOffset->setLayer(csLayer);
			pOffset->setColor(g_ByLayerColor); 
			pOffset->setLinetype(_T("BYLAYER"));

			appendEntityToDatabase(pOffset);
			aryObjIdCollection.append(pOffset->objectId());

			if (!iCtr) 
			{
				// Get the object Id of the first offset
				objIdOffset1 = pOffset->objectId(); 

				// Get the end points of the first offset
				pOffset->getStartPoint(geStart1); 
				pOffset->getEndPoint(geEnd1); 
			}
			else if (iCtr == 1)
			{
				// Get the object Id of the second offset
				objIdOffset2 = pOffset->objectId();

				// Get the end points of the first offset 
				pOffset->getStartPoint(geStart2); 
				pOffset->getEndPoint(geEnd2); 

				// Join the ends. This has to be done only for open entities
				// if (!bIsClosed)
				{
					AcDbLine *pEnd1 = new AcDbLine(geStart1, geStart2); pEnd1->setLayer(csLayer); pEnd1->setLinetype(_T("BYLAYER")); pEnd1->setColor(g_ByLayerColor); appendEntityToDatabase(pEnd1); objIdEnd1 = pEnd1->objectId(); pEnd1->close();
					AcDbLine *pEnd2 = new AcDbLine(geEnd1,   geEnd2);   pEnd2->setLayer(csLayer); pEnd2->setLinetype(_T("BYLAYER")); pEnd2->setColor(g_ByLayerColor); appendEntityToDatabase(pEnd2); objIdEnd2 = pEnd2->objectId(); pEnd2->close();

					dAngle = acutAngle(asDblArray(geStart1), asDblArray(geStart2));

					aryObjIdCollection.append(objIdEnd1);
					aryObjIdCollection.append(objIdEnd2);
				}
			}

			// Close the curve
			pOffset->close();
		}
		else
			acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es));
	}

	// The new start point of the next coverage
	ptEnd[X] = (geEnd1[X] + geEnd2[X]) / 2;
	ptEnd[Y] = (geEnd1[Y] + geEnd2[Y]) / 2;
	ptEnd[Z] = 0.0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Convert the hatch boundaries to a single polyline. This will enable the user to remove these by selecting it once, in the event of ERASE.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Check if the collection has splines. Splines as path cannot be exploded, hence the duct boundaries will remain as individual segments
	bool bHasSplines = false;
	for (int iCtr = 0; iCtr < aryObjIdCollection.logicalLength(); iCtr++)
	{
		AcDbEntity *pEntity;
		if (acdbOpenObject(pEntity, aryObjIdCollection.at(iCtr), AcDb::kForWrite) == Acad::eOk)
		{
			if (pEntity->isKindOf(AcDbSpline::desc())) 
			{
				pEntity->close(); bHasSplines = true; 
				break; 
			}
			pEntity->close();
		}
	}
	if (!bHasSplines) bHasSplines = !CreateHatchBoundary(aryObjIdCollection);
	
	////////////////////////////////////////
	// Hatch the coverage for PROPOSED
	////////////////////////////////////////
	if (!csStatus.CompareNoCase(L"Proposed") || !csStatus.CompareNoCase(L"Under Boring"))
	{
		AcDbHatch *pHatch = new AcDbHatch();

		// Set hatch plane and pattern 
		AcGeVector3d normal(0.0, 0.0, 1.0);
		pHatch->setNormal(normal);
		pHatch->setElevation(0.0);
		pHatch->setHatchStyle(AcDbHatch::kNormal);		
		pHatch->setAssociative(Adesk::kTrue);

		// Hatch with ANSI 131/ Scale 8 and angle = Angle of a line drawn between the midpoints of the two end lines of all the combined segments (NOT UNDERSTOOD!!)
		// pHatch->setPatternScale(dWidth * 2 * 10);
		// pHatch->setPattern(AcDbHatch::kPreDefined, _T("ANSI31"));

		if (!csStatus.CompareNoCase(L"Proposed"))
		{
			pHatch->setPatternScale(g_dProposedScale);
			pHatch->setPattern(AcDbHatch::kPreDefined, g_csProposedPattern);
			pHatch->setPatternAngle(dAngle - DTR(45.0));
		}
		else
		{
			pHatch->setPatternScale(g_dUnderboreScale);
			pHatch->setPattern(AcDbHatch::kPreDefined, g_csUnderborePattern);
			pHatch->setPatternAngle(dAngle - DTR(90.0));

		}

		es = pHatch->appendLoop(AcDbHatch::kDefault, aryObjIdCollection);
		if (es != Acad::eOk) { /*acutPrintf(_T("\nERROR: %s"), acadErrorStatusText(es)); delete pHatch; acutPrintf(L"\nError 3");*/  return false; }

		// Elaborate hatch lines
		pHatch->evaluateHatch();

		pHatch->setLayer(csLayer);
		pHatch->setColor(g_ByLayerColor);

		appendEntityToDatabase(pHatch);
		aryObjIdCollection.append(pHatch->objectId());

		// Attach hatchId to all source boundary objects for notification.
		AcDbEntity *pEnt;
		AcDbObjectId objHatchId = pHatch->objectId();

		int numObjs = aryObjIdCollection.length();
		for (int i = 0; i < numObjs; i++) 
		{
			es = acdbOpenAcDbEntity(pEnt, aryObjIdCollection[i], AcDb::kForWrite);
			if (es == Acad::eOk) 
			{
				pEnt->addPersistentReactor(pHatch->objectId());
				pHatch->close();
				// pEnt->setLayer(csLayer);
				pEnt->close();
			}
		}

		pHatch->close();
	}
	
	// Erase the entity if its already open
	if (pEntity != NULL) { pEntity->close(); }

	// Append the boundaries and hatch to all entities array
	for (int iCtr = 0; iCtr < aryObjIdCollection.logicalLength(); iCtr++) 
	{
		AcDbEntity *pEntity;
		if (acdbOpenObject(pEntity, aryObjIdCollection.at(iCtr), AcDb::kForWrite) == Acad::eOk)
		{
			pEntity->setLayer(csLayer);
			pEntity->close();
		}

		aryObjIds.append(aryObjIdCollection.at(iCtr)); 
	}

	///////////////////////////////////////
	// Erase the polyline at the axis
	///////////////////////////////////////
	es = acdbOpenObject(pEntity, objPolyId, AcDb::kForWrite);
	if (es != Acad::eOk) { /*acutPrintf(L"\nError 4"); */ return false; } 

	// If the position was "Side" this axis is drawn by the application and hence must be removed. For "Center" this line is the selected entity only and hence should not be removed.
	if (dOffset || csPosition.CompareNoCase(L"Center") || !csType.CompareNoCase(L"Points")) pEntity->erase();
	pEntity->close();

	acedCommandS(RTSTR, _T(".REGEN"), NULL);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CreateCoverage()
// Description  : Draws the reference objects to represent the coverage.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CreateCoverage(AcDbObjectId objSel, ads_point ptPick, CString csType, double dOffset, double dWidth, CString csPosition, CString csLayer, CString csStatus, AcDbObjectIdArray &aryObjIds)
{
	// Define the new reference object
	if (objSel.isNull() || !objSel.isValid() || objSel.isErased()) return FALSE;	

	// The reference object for duct coverage will be determined by the offset specified
	AcDbObjectId objReference;
	AcDbEntity *pEntity;
	Acad::ErrorStatus es;
	if (dOffset)
	{
		// Get the new curve at the offset distance specified
		if (acdbOpenObject(pEntity, objSel, AcDb::kForWrite) == Acad::eOk)
		{
			AcDbCurve *pCurve = (AcDbCurve *) pEntity;
			AcDbVoidPtrArray ar_Offsets;
			es = pCurve->getOffsetCurves(dOffset, ar_Offsets);
			if (es != Acad::eOk) { acutPrintf(L"\nError @%d[%s]", __LINE__, acadErrorStatusText(es)); pEntity->close(); return FALSE; }

			AcDbCurve *pCurvePositive = (AcDbCurve *)ar_Offsets.at(0);
			if (append(pCurvePositive, L"BASE_DUCT_PROP"))
			{
				objReference = pCurvePositive->objectId();
				pCurvePositive->close();

				// If the type is POINTS objSel was a dummy line defined as points were picked. This can be deleted. ******** LOOK 
				// if (!csType.CompareNoCase(L"Points")) pEntity->erase(); FEB 

				// Close the parent entity
				pEntity->close();
			}
		}
	}
	else
	{
		// Get the new curve at the offset distance specified
		ads_name enSel; acdbGetAdsName(enSel, objSel);
		acedCommandS(RTSTR, L".COPY", RTENAME, enSel, RTSTR, L"", RTSTR, L"0,0", RTSTR, L"0,0", NULL);
		ads_name enReference; acdbEntLast(enReference);
		acdbGetObjectId(objReference, enReference);
	}

	if (objReference.isNull() || !objReference.isValid() || objReference.isErased()) return FALSE;		

	AcDbObjectId objPolyId = objReference;
	if (csPosition.CompareNoCase(L"Center"))
	{
		// Get the curve offset to LEFT/RIGHT
		if (acdbOpenObject(pEntity, objReference, AcDb::kForWrite) == Acad::eOk)
		{
			AcDbCurve *pCurve = (AcDbCurve *) pEntity;
			AcDbVoidPtrArray ar_Offsets;
			int iSideFactor = ((csPosition.CompareNoCase(L"Left")) ? 1 : -1);
			es = pCurve->getOffsetCurves(dWidth * iSideFactor, ar_Offsets);

			if (es != Acad::eOk) { acutPrintf(L"\nError @%d[%s]", __LINE__, acadErrorStatusText(es)); pEntity->close(); return FALSE; }

			AcDbCurve *pCurvePositive = (AcDbCurve *)ar_Offsets.at(0);
			if (append(pCurvePositive, L"BASE_DUCT_PROP"))
			{
				// Get the new reference at offset specified
				objPolyId = pCurvePositive->objectId();
				pCurvePositive->close();

				pEntity->erase();
				pEntity->close();
			}
		}
	}

	// Call the function to do the hatch
	ads_point ptDummy;
	if (!ElaborateCoverage(objPolyId, dWidth, csLayer, aryObjIds, ptPick, csStatus, csPosition, csType, dOffset)) return FALSE;

	// Delete the reference object
	AcDbEntity *pEnt;
	if (acdbOpenObject(pEnt, objPolyId, AcDb::kForWrite) == Acad::eOk) { pEnt->erase(); pEnt->close(); }

	return TRUE;
}

// CDuctCoverageDlg dialog

IMPLEMENT_DYNAMIC(CDuctCoverageDlg, CDialog)

CDuctCoverageDlg::CDuctCoverageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDuctCoverageDlg::IDD, pParent)
{
	m_csSelectedText = L"No object selected";
	m_iCalledFor = 0;
	m_csWidth = L"1";
	m_csOffset = L"0.0";
	m_csType = L"";
	m_csStatus = L"";
	m_csPosition = L"";
	m_iOffsetFactor = 1;
}

CDuctCoverageDlg::~CDuctCoverageDlg()
{
}

void CDuctCoverageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DC_PICKWIDTH, m_btnPiclWidth);
	DDX_Control(pDX, IDC_RV_SELECTTYPE, m_btnSelectType);
	DDX_Text(pDX, IDC_DC_WIDTH, m_csWidth);
	DDX_Text(pDX, IDC_DC_OFFSET, m_csOffset);
	DDX_Control(pDX, IDC_DC_TYPE, m_cbType);
	DDX_CBString(pDX, IDC_DC_TYPE, m_csType);
	DDX_Text(pDX, IDC_STATIC_OBJSEL, m_csSelectedText);
	DDX_Control(pDX, IDC_DC_STATUS, m_cbStatus);
	DDX_CBString(pDX, IDC_DC_STATUS, m_csStatus);
	DDX_Control(pDX, IDC_DC_POSITION, m_cbPosition);
	DDX_CBString(pDX, IDC_DC_POSITION, m_csPosition);
	DDX_Control(pDX, IDC_ONOFF, m_btnOnOff);
	DDX_Control(pDX, IDC_DC_PICKOFFSET, m_btnOffset);
	DDX_Control(pDX, IDC_STATIC_IMAGE, m_btnInfo);
}


BEGIN_MESSAGE_MAP(CDuctCoverageDlg, CDialog)
	ON_BN_CLICKED(IDC_DC_PICKWIDTH, &CDuctCoverageDlg::OnBnClickedDcPickwidth)
	ON_CBN_SELCHANGE(IDC_DC_TYPE, &CDuctCoverageDlg::OnCbnSelchangeDcType)
	ON_BN_CLICKED(IDC_RV_SELECTTYPE, &CDuctCoverageDlg::OnBnClickedRvSelecttype)
	ON_BN_CLICKED(IDOK, &CDuctCoverageDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_DC_PICKOFFSET, &CDuctCoverageDlg::OnBnClickedDcPickoffset)
	ON_BN_CLICKED(IDHELP, &CDuctCoverageDlg::OnBnClickedHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////
// CDuctCoverageDlg message handlers
/////////////////////////////////////////////////////////
BOOL CDuctCoverageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Set the button bitmaps
	m_btnPiclWidth.SetSkin(IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, 1, 1, 1); 
	m_btnSelectType.SetSkin(IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, 1, 1, 1);
	m_btnOffset.SetSkin(IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, 1, 1, 1);

	// Set the information ICON
	HICON hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_INFO), IMAGE_ICON, 16, 16, LR_SHARED);
	m_btnInfo.SetIcon(hIcon);
	
	// Defaults
	if (m_csPosition.IsEmpty()) m_cbPosition.SetCurSel(0); else m_cbPosition.SelectString(-1, m_csPosition);

	// Status
	if (m_csStatus.IsEmpty()) m_csStatus = L"Proposed"; m_cbStatus.SelectString(-1, m_csStatus);

	if (m_csType.IsEmpty()) m_cbType.SetCurSel(0); else m_cbType.SelectString(-1, m_csType);
	
	// Modify the static text based on the selection
	CString csType = m_csType;
	SetDlgItemText(IDC_STATIC_TYPEMSG, L"Select " + csType.MakeLower());
	if (m_csStatus.IsEmpty()) m_cbStatus.SetCurSel(0); else m_cbStatus.SelectString(-1, m_csStatus);

	// Offset type
	CheckDlgButton(IDC_ONOFF, m_iOffsetFactor);

	// If any reference object is already created, delete it
	ResetSelectedObject();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctCoverageDlg::OnBnClickedDcPickwidth()
// Description  : Exits the dialog when the user opts to specify width on screen.
////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctCoverageDlg::OnBnClickedDcPickwidth()
{
	// Set the flag to enable the user to specify height
	m_iCalledFor = 1;

	// Close the dialog
	OnBnClickedOk();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctCoverageDlg::OnBnClickedRvSelecttype()
// Description  : Exits the dialog when the user opts to select entity on screen.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctCoverageDlg::OnBnClickedRvSelecttype()
{
	// Set the flag to enable the user to specify height
	m_iCalledFor = 2;

	// Close the dialog
	OnBnClickedOk();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctCoverageDlg::OnBnClickedDcPickoffset()
// Description  : Exits the dialog when user opts to specify offset on screen.
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctCoverageDlg::OnBnClickedDcPickoffset()
{
	// Set the flag to enable the user to specify height
	m_iCalledFor = 3;

	// Close the dialog
	OnBnClickedOk();
}

//////////////////////////////////////////////////////////////////////////////
// Function name: CDuctCoverageDlg::ResetSelectedObject()
// Description  : 
//////////////////////////////////////////////////////////////////////////////
void CDuctCoverageDlg::ResetSelectedObject()
{
	// If Yes, reset the object id and delete the object
	if (m_csType.CompareNoCase(m_csOldType))
	{
		// Delete the object (if drawn by using POINTS option)
		if (!m_csOldType.CompareNoCase(L"POINTS"))
		{
			AcDbEntity *pEntity;
			if (acdbOpenObject(pEntity, m_objSel, AcDb::kForWrite) == Acad::eOk)
			{
				pEntity->erase();
				pEntity->close();
			}
		}
		
		m_objSel.setNull(); 
		SetDlgItemText(IDC_STATIC_OBJSEL, L"No object selected");
		m_csOldType = m_csType;
	}
}

//////////////////////////////////////////////////////////////////////
// Function name: CDuctCoverageDlg::OnCbnSelchangeDcType()
// Description  : 
//////////////////////////////////////////////////////////////////////
void CDuctCoverageDlg::OnCbnSelchangeDcType()
{
	int iSel = m_cbType.GetCurSel();
	if (iSel == CB_ERR) return;
	m_cbType.GetLBText(iSel, m_csType);

	// Modify the static text based on the selection
	CString csType = m_csType;
	SetDlgItemText(IDC_STATIC_TYPEMSG, L"Select " + csType.MakeLower());

	// If any reference object is already created, delete it
	ResetSelectedObject();
}

//////////////////////////////////////////////////////////////////////
// Function name: CDuctCoverageDlg::OnBnClickedCancel()
// Description  : Validates the inputs and exits the dialog.
//////////////////////////////////////////////////////////////////////
void CDuctCoverageDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	// Get the ID selected
	// if ((m_iCalledFor == 2) || !m_iCalledFor)

	m_iOffsetFactor = IsDlgButtonChecked(IDC_ONOFF);
	if (!m_iCalledFor)
	{
		// Width
		if (_tstof(m_csWidth) < 0.0) { ShowBalloon(L"Specify width.", this, IDC_DC_WIDTH); return; }

		// Entity selected check
		if (m_objSelArray.length() <= 0) { CString csMsg; csMsg.Format(L"Select %s.", m_csType); ShowBalloon(csMsg, this, IDC_RV_SELECTTYPE); return; }
				
		// Inputs validated and confirmed OK
		m_iCalledFor = 100;
	}
		
	OnOK();
}

//////////////////////////////////////////////////////////////////////
// Function name: CDuctCoverageDlg::OnBnClickedCancel()
// Description  :
//////////////////////////////////////////////////////////////////////
void CDuctCoverageDlg::OnBnClickedCancel()
{
	//////////////////////////////////////////
	// Remove the selected object
	//////////////////////////////////////////
	if (!m_csType.CompareNoCase(L"POINTS") && !m_objSel.isNull() && m_objSel.isValid() && !m_objSel.isErased())
	{
		AcDbEntity *pEntity;
		if (acdbOpenObject(pEntity, m_objSel, AcDb::kForWrite) == Acad::eOk)
		{
			pEntity->erase();
			pEntity->close();
		}
	}

	//////////////////////////////////////////
	// Remove the reference object selected
	//////////////////////////////////////////
	if (!m_objReference.isNull() && m_objReference.isValid() && !m_objReference.isErased())
	{
		AcDbEntity *pEntity;
		if (acdbOpenObject(pEntity, m_objReference, AcDb::kForWrite) == Acad::eOk)
		{
			pEntity->erase();
			pEntity->close();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedHelp()
// Description  : Calls the topic specified from NET CAD.CHM file
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctCoverageDlg::OnBnClickedHelp() { displayHelp((DWORD)_T("Duct_Coverage.htm")); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: Command_DuctCoverage
// Description  : 
// Arguments    : 
// Called by    :
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_DuctCoverage()
{
	// Switch off certain system variables
	// switchOff();

  g_ByLayerColor.setColor(AcCmEntityColor::kByLayer);

	CDuctCoverageDlg dlgDuctCoverage;
	dlgDuctCoverage.m_iCalledFor = 0;

	// Check if there are any previously saved values for the dialog
	struct resbuf *rbpValues = getXRecordFromDictionary(_T("eCapture"), _T("DuctCoverage"));
	if (rbpValues)
	{
		dlgDuctCoverage.m_csPosition     = rbpValues->resval.rstring; rbpValues = rbpValues->rbnext;
		dlgDuctCoverage.m_csWidth        = rbpValues->resval.rstring; rbpValues = rbpValues->rbnext;
		dlgDuctCoverage.m_csOffset       = rbpValues->resval.rstring; rbpValues = rbpValues->rbnext;
		dlgDuctCoverage.m_iOffsetFactor  = rbpValues->resval.rint;    rbpValues = rbpValues->rbnext;
		dlgDuctCoverage.m_csType				 = rbpValues->resval.rstring; rbpValues = rbpValues->rbnext;
		dlgDuctCoverage.m_csStatus       = rbpValues->resval.rstring; rbpValues = rbpValues->rbnext;
		acutRelRb(rbpValues);
	}

	while (T)
	{
		// Display the duct coverage dialog
		dlgDuctCoverage.m_iCalledFor = 0;
		dlgDuctCoverage.m_csOldType = dlgDuctCoverage.m_csType;
		if (dlgDuctCoverage.DoModal() == IDCANCEL) return;

		if (dlgDuctCoverage.m_iCalledFor == 1)
		{
			///////////////////////////////////////////
			// User wants to pick two points for width
			///////////////////////////////////////////
			double dWidth;
			while (T)
			{
				acedInitGet(RSG_NONEG | RSG_NOZERO, _T(""));
				int iRet = acedGetDist(NULL, L"\nWidth: ", &dWidth);
				/**/ if (iRet == RTCAN)  break;
				else if (iRet == RTNORM) break;
			}

			dlgDuctCoverage.m_csWidth.Format(L"%.2f", dWidth);
		}
		else if (dlgDuctCoverage.m_iCalledFor == 2)
		{
			///////////////////////////////////////////
			// User wants to select the entities
			///////////////////////////////////////////
			if (SelectEntities(dlgDuctCoverage.m_csType, dlgDuctCoverage.m_objSelArray, dlgDuctCoverage.m_gePtArray)) 
			{
				dlgDuctCoverage.m_csSelectedText.Format(L"%d objects selected.", dlgDuctCoverage.m_objSelArray.length());
			}

			// Removes the highlights
			acedCommandS(RTSTR, L".REGEN", NULL);
		}
		else if (dlgDuctCoverage.m_iCalledFor == 3)
		{
			///////////////////////////////////////////
			// User wants to pick two points for offset
			///////////////////////////////////////////
			double dOffset;
			while (T)
			{
				acedInitGet(RSG_NONEG | RSG_NOZERO, _T(""));
				int iRet = acedGetDist(NULL, L"\nOffset: ", &dOffset);
				/**/ if (iRet == RTCAN)  break;
				else if (iRet == RTNORM) break;
			}

			dlgDuctCoverage.m_csOffset.Format(L"%.2f", dOffset);
		}
		else if (dlgDuctCoverage.m_iCalledFor == 100) 
		{
			///////////////////////////////////////////
			// User wants to draw the duct coverage
			///////////////////////////////////////////
			// Layer on which to place the coverage
			CString csLayer = L"BASE_DUCT_PROP";
			if (!dlgDuctCoverage.m_csStatus.CompareNoCase(L"Existing")) csLayer = L"BASE_DUCT_EXIST";
			createLayer(csLayer,  Adesk::kFalse, Adesk::kFalse);

			CString csPrompt;
			TCHAR pszInput[10]; 
			AcDbObjectIdArray objIdArray;
			CString csSide = (!dlgDuctCoverage.m_csPosition.CompareNoCase(L"Side") ? L"Left" : dlgDuctCoverage.m_csPosition);
			while (T)
			{
				// Call the function to draw the coverage
				for (int iCtr = 0; iCtr < dlgDuctCoverage.m_objSelArray.length(); iCtr++)
				{
					CreateCoverage(dlgDuctCoverage.m_objSelArray.at(iCtr), asDblArray(dlgDuctCoverage.m_gePtArray.at(iCtr)), dlgDuctCoverage.m_csType, _tstof(dlgDuctCoverage.m_csOffset) * (!dlgDuctCoverage.m_iOffsetFactor ? -1 : 1), _tstof(dlgDuctCoverage.m_csWidth) / 2, csSide, csLayer, dlgDuctCoverage.m_csStatus, objIdArray);
				}

				// Ask for the next intent of the user
				if (csSide.CompareNoCase(L"Center"))
				{
					csPrompt.Format(_T("\nAccept or [Flip]: "));
					acedInitGet(NULL, _T("Accept Flip"));
				}
				else
				{
					csPrompt.Format(_T("\nAccept: "));
					acedInitGet(NULL, _T("Accept"));
				}

				int iRet = acedGetKword(csPrompt, pszInput);
				/**/ if (iRet == RTCAN) 
				{
					// Erase the coverage that was drawn
					for (int iObj = 0; iObj < objIdArray.logicalLength(); iObj++) 
					{	
						AcDbEntity *pEntity;
						if (acdbOpenObject(pEntity, objIdArray.at(iObj), AcDb::kForWrite) == Acad::eOk) 
						{
							pEntity->erase();	
							pEntity->close();	
						}
					}

					// Remove the 
					dlgDuctCoverage.m_csSelectedText = L"No object selected";
					dlgDuctCoverage.m_objSelArray.removeAll();

					break;
				}
				else if ((iRet == RTNONE) || !_tcsicmp(pszInput, _T("Accept")))
				{
					// This is equivalent to "Accept" option. Display the dialog again to make the next duct coverage.
					dlgDuctCoverage.m_csSelectedText = L"No object selected";
					dlgDuctCoverage.m_objSelArray.removeAll();

					// Delete the entity drawn by picking points for specifying "Points" type
					if (!dlgDuctCoverage.m_csType.CompareNoCase(L"Points"))
					{
						AcDbEntity *pPtPline;
						if (acdbOpenObject(pPtPline, dlgDuctCoverage.m_objSelArray.at(0), AcDb::kForWrite) == Acad::eOk)
						{
							pPtPline->erase();
							pPtPline->close();
						}
					}

					break;
				}
								
				if (!_tcsicmp(pszInput, _T("Flip"))) 
				{
					// The user wants to swap the side of duct coverage
					csSide = (!csSide.CompareNoCase(L"Left") ? L"Right" : L"Left");

					// Delete the coverage already drawn
					for (int iObj = 0; iObj < objIdArray.logicalLength(); iObj++)
					{
						AcDbEntity *pEntity;
						if (acdbOpenObject(pEntity, objIdArray.at(iObj), AcDb::kForWrite) == Acad::eOk)
						{
							pEntity->erase();
							pEntity->close();
						}
					}

					objIdArray.removeAll();
				}
			}
		}
	}
	
	// Save the selected values for future retrieval
	rbpValues = acutBuildList(AcDb::kDxfText, dlgDuctCoverage.m_csPosition,
													  AcDb::kDxfText, dlgDuctCoverage.m_csWidth,
													  AcDb::kDxfText, dlgDuctCoverage.m_csOffset,
													  AcDb::kDxfInt16, dlgDuctCoverage.m_iOffsetFactor,
													  AcDb::kDxfText, dlgDuctCoverage.m_csType,
													  AcDb::kDxfText, dlgDuctCoverage.m_csStatus, 
													  NULL
													 );

	addXRecordToDictionary(_T("eCapture"), _T("DuctCoverage"), rbpValues);
	acutRelRb(rbpValues);

	// Switch off certain system variables
	switchOn();
}















