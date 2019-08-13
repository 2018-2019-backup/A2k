#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////////
extern AcCmColor g_ByLayerColor; 

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
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
    AcGePoint3d tmpPt = pts[0];        // used to be a bug in dynamic arrays (not sure if its still there??)
    pts.append(tmpPt);
    bulges.append(0.0);
  }
}


//////////////////////////////////////////////////////////////////////////
// Function name: getClosestPoint()
// Description  : Gets the closest point to given point and also gets
//                another point that is 0.1 units away from closest pt.
//////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus getClosestPoint(ads_name enPt, ads_point ptGiven, ads_point ptClose, ads_point ptNear)
{
  // Get the object id
  AcDbObjectId objId; 
  Acad::ErrorStatus es = acdbGetObjectId(objId, enPt);
  if (es != Acad::eOk) return es;

  // Get the entity pointer
  AcDbEntity *pEntity;
  if (acdbOpenObject(pEntity, objId, AcDb::kForRead) != Acad::eOk) { return es; }

  // Depending on the entity type call the function to get the closest point
  if (!pEntity->isKindOf(AcDbLine::desc()) && !pEntity->isKindOf(AcDbPolyline::desc()) && !pEntity->isKindOf(AcDb2dPolyline::desc()) && !pEntity->isKindOf(AcDbArc::desc()) && !pEntity->isKindOf(AcDbSpline::desc()))
  {
    pEntity->close();
    return Acad::eNotThatKindOfClass;
  }

  AcGePoint3d geGivenPt = AcGePoint3d(ptGiven[X], ptGiven[Y], 0.0 );
  AcGePoint3d geClosePt;

  AcDbCurve *pCurve;
  if (pEntity->isKindOf(AcDbLine::desc()))
  {
    AcDbLine *pLine = AcDbLine::cast(pEntity);

    es = pLine->getClosestPointTo(geGivenPt, geClosePt);
    if (es != Acad::eOk) { pLine->close(); return es; }

    pCurve = (AcDbCurve *) pLine;
    pLine->close();
  }
  else if (pEntity->isKindOf(AcDbPolyline::desc()))
  {
    AcDbPolyline *pPLine = AcDbPolyline::cast(pEntity);

    es = pPLine->getClosestPointTo(geGivenPt, geClosePt);
    if (es != Acad::eOk) { pPLine->close(); return es; }

    pCurve = (AcDbCurve *) pPLine;
    pPLine->close();
  }
  else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
  {
    AcDb2dPolyline *pPLine = AcDb2dPolyline::cast(pEntity);

    es = pPLine->getClosestPointTo(geGivenPt, geClosePt);
    if (es != Acad::eOk) { pPLine->close(); return es; }

    pCurve = (AcDbCurve *) pPLine;
    pPLine->close();
  }
  else if (pEntity->isKindOf(AcDbArc::desc()))
  {
    AcDbArc *pArc = AcDbArc::cast(pEntity);

    es = pArc->getClosestPointTo(geGivenPt, geClosePt);
    if (es != Acad::eOk) { pArc->close(); return es; }

    pCurve = (AcDbCurve *) pArc;
    pArc->close();
  }
  else if (pEntity->isKindOf(AcDbSpline::desc()))
  {
    AcDbSpline *pSpline = AcDbSpline::cast(pEntity);

    es = pSpline->getClosestPointTo(geGivenPt, geClosePt);
    if (es != Acad::eOk) { pSpline->close(); return es; }

    pCurve = (AcDbCurve *) pSpline;
    pSpline->close();
  }

  // Get the point that is 0.1 units away from the closest point
  double dDist;
  es = pCurve->getDistAtPoint(geClosePt, dDist);
  if (es != Acad::eOk) { return es; }

  AcGePoint3d geNearPt;
  es = pCurve->getPointAtDist((!dDist ? 0.1 : dDist - 0.1), geNearPt);
  if (es != Acad::eOk) { return es; }

  acutPolar(asDblArray(geClosePt), 0.0, 0.0, ptClose);
  acutPolar(asDblArray(geNearPt),  0.0, 0.0, ptNear);

  return Acad::eOk;
}

//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
bool ElaborateCoverage(AcDbObjectId objPolyId, double dWidth, CString csLayer, AcDbObjectIdArray &aryObjIds, ads_point ptEnd, CString csStatus, CString csPosition, CString csType, double dOffset)
{
	if (objPolyId == AcDbObjectId::kNull) { acutPrintf(L"\nError 1"); return false; }

	//////////////////////////
	// Length of the curve
	//////////////////////////
	//ads_name enObjID;
	//acdbGetAdsName(enObjID, objPolyId);
	//acedCommand(RTSTR, L".LENGTHEN", RTLB, RTENAME, enObjID, RTPOINT, ptEnd, RTLE, RTSTR, L"", NULL);
	//struct resbuf rbSetVar; acedGetVar(L"PERIMETER", &rbSetVar);
	//double dLength = rbSetVar.resval.rreal;
	
	
	/////////////////////////////////////////////////////////////////////////
	// Elaborate the coverage
	//////////////////////////////////////////////////////////////////////////
	AcDbEntity *pEntity;
	Acad::ErrorStatus es = acdbOpenObject(pEntity, objPolyId, AcDb::kForWrite);
	if (es != Acad::eOk) { acutPrintf(L"\nError 2 [%s]", acadErrorStatusText(es)); return false; }

	//////////////////////////////////////////////////////////////////////////
	// Offset axis polyline on +ve/-ve side of the axis for half trench width
	//////////////////////////////////////////////////////////////////////////
	AcDbVoidPtrArray ar_Offsets;
	AcGePoint3d geStart1, geEnd1;
	AcGePoint3d geStart2, geEnd2;

	deleteArray(aryObjIds);
	aryObjIds.removeAll();

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
		
		pEntity->close();
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
			aryObjIds.append(pOffset->objectId());

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

				// Join the ends
				AcDbLine *pEnd1 = new AcDbLine(geStart1, geStart2); pEnd1->setLayer(csLayer); pEnd1->setLinetype(_T("BYLAYER")); pEnd1->setColor(g_ByLayerColor); appendEntityToDatabase(pEnd1); objIdEnd1 = pEnd1->objectId(); pEnd1->close();
				AcDbLine *pEnd2 = new AcDbLine(geEnd1,   geEnd2);   pEnd2->setLayer(csLayer); pEnd2->setLinetype(_T("BYLAYER")); pEnd2->setColor(g_ByLayerColor); appendEntityToDatabase(pEnd2); objIdEnd2 = pEnd2->objectId(); pEnd2->close();

				dAngle = acutAngle(asDblArray(geStart1), asDblArray(geStart2));

				aryObjIds.append(objIdEnd1);
				aryObjIds.append(objIdEnd2);
			}

			// Close the curve
			pOffset->close();
		}
	}

	// The new start point of the next coverage
	ptEnd[X] = (geEnd1[X] + geEnd2[X]) / 2;
	ptEnd[Y] = (geEnd1[Y] + geEnd2[Y]) / 2;
	ptEnd[Z] = 0.0;

	if (!csStatus.CompareNoCase(L"Proposed"))
	{
		////////////////////////////////////////
		// Hatch the coverage for PROPOSED
		////////////////////////////////////////
		AcDbHatch *pHatch = new AcDbHatch();
					
		// Set hatch plane and pattern
		AcGeVector3d normal(0.0, 0.0, 1.0);
		pHatch->setNormal(normal);
		pHatch->setElevation(0.0);

		// Hatch with ANSI 131/ Scale 8 and angle = Angle of a line drawn between the midpoints of the two end lines of all the combined segments (NOT UNDERSTOOD!!)
		// pHatch->setPatternScale(acutDistance(asDblArray(geStartPt), asDblArray(geEndPt)) * 2);
		// pHatch->setPatternAngle(acutAngle(asDblArray(geStartPt), asDblArray(geEndPt)));
		pHatch->setPatternScale(dWidth * 2 * 10);
		// pHatch->setPatternScale(8);
		pHatch->setPatternAngle(dAngle - PIby2);
		pHatch->setPattern(AcDbHatch::kPreDefined, _T("ANSI31"));
		pHatch->setHatchStyle(AcDbHatch::kNormal);

		// Set Associativity
		pHatch->setAssociative(Adesk::kTrue);

		es = pHatch->appendLoop(AcDbHatch::kDefault, aryObjIds);
		if (es != Acad::eOk) { /*acutPrintf(_T("\nERROR: %s"), acadErrorStatusText(es)); delete pHatch; acutPrintf(L"\nError 3");*/  return false; }

		// Elaborate hatch lines
		pHatch->evaluateHatch();

		pHatch->setLayer(csLayer);
		pHatch->setColor(g_ByLayerColor);

		appendEntityToDatabase(pHatch);
		aryObjIds.append(pHatch->objectId());

		// Attach hatchId to all source boundary objects for notification.
		AcDbEntity *pEnt;
		AcDbObjectId objHatchId = pHatch->objectId();

		int numObjs = aryObjIds.length();
		for (int i = 0; i < numObjs; i++) 
		{
			es = acdbOpenAcDbEntity(pEnt, aryObjIds[i], AcDb::kForWrite);
			if (es == Acad::eOk) 
			{
				pEnt->addPersistentReactor(pHatch->objectId());
				pHatch->close();
				pEnt->close();
			}
		}

		pHatch->close();
	}

	// Close the entity if its already open
	if (pEntity != NULL) pEntity->close();

	// Erase the polyline at the axis
	es = acdbOpenObject(pEntity, objPolyId, AcDb::kForWrite);
	if (es != Acad::eOk) { /*acutPrintf(L"\nError 4"); */ return false; } 

	// If the position was "Left" or "Right" this axis is drawn by the application and hence must be removed. 
	// For "Center" this line is the selected entity only and hence should not be removed.
	if (dOffset || csPosition.CompareNoCase(L"Center") || !csType.CompareNoCase(L"Points")) pEntity->erase();
	pEntity->close();

	// Convert the hatch boundary to a polyline
	ads_name enPoly;
	for (int iCtr = 0; iCtr < aryObjIds.length(); iCtr++)
	{
		acdbGetAdsName(enPoly, aryObjIds.at(iCtr));
				
		if (!iCtr)
		{
			struct resbuf *rbpType = acdbEntGet(enPoly);
			if (!_tcsicmp(Assoc(rbpType, 0)->resval.rstring, L"LINE"))
				acedCommand(RTSTR, L".PEDIT", RTENAME, enPoly, RTSTR, L"Y", RTSTR, L"J", NULL);
			else
				acedCommand(RTSTR, L".PEDIT", RTENAME, enPoly, RTSTR, L"J", NULL);
			acutRelRb(rbpType);
		}
		else
			acedCommand(RTENAME, enPoly, NULL);
	}

	acedCommand(RTSTR, L"", RTSTR, L"", NULL);
	acedCommand(RTSTR, _T(".REGEN"), NULL);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
void Command_DuctCoverageOld()
// void Command_DuctCoverage()
{
	int iRet;
	TCHAR result[10];
	AcDbObjectIdArray aryObjIds;

	// Switch off certain variables
	switchOff();

	CString csLayer = L"BASE_DUCT_PROP";
	createLayer(_T("BASE_DUCT_PROP"),  Adesk::kFalse, Adesk::kFalse);
	while (T)
	{
		//////////////////////////////////////////////////////////////////////////
		// Get the cables b/w which the coverage has to be drawn
		//////////////////////////////////////////////////////////////////////////

		AcDbEntity *pEntity;
		AcDbObjectId objId;
		Acad::ErrorStatus es;

		ads_name enCable1; 
		ads_point ptCable1;

		while (T)
		{
			iRet = acedEntSel(_T("\rSelect the first outermost cable: "), enCable1, ptCable1);
			/**/ if (iRet == RTCAN) return;
			else if (iRet == RTNORM) 
			{
				// Check if the entity selected is a valid contour
				acdbGetObjectId(objId, enCable1);

				es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
				if (es != Acad::eOk) { /*acutPrintf(_T("\nError at %d: %s"), __LINE__, acadErrorStatusText(es));*/ return; }

				if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc())) break;
				pEntity->close();

				// Not a valid contour
				acutPrintf(_T("\nNot a valid cable.\n"));
			}
		}

		ads_name enCable2; 
		ads_point ptCable2;
		while (T)
		{
			iRet = acedEntSel(_T("\rSelect the second outermost cable: "), enCable2, ptCable2);
			/**/ if (iRet == RTCAN) return;
			else if (iRet == RTNORM) 
			{
				// Check if the entity selected is a valid contour
				acdbGetObjectId(objId, enCable2);

				es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
				if (es != Acad::eOk) { /*acutPrintf(_T("\nError at %d: %s"), __LINE__, acadErrorStatusText(es)); */ return; }

				if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc())) break;
				pEntity->close();

				// Not a valid contour
				acutPrintf(_T("\nNot a valid cable.\n"));
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Get the distance b/w them
		//////////////////////////////////////////////////////////////////////////
		struct resbuf rbOsnap; rbOsnap.restype = RTPOINT;  
		acutPolar(ptCable1, 0.0, 0.0, rbOsnap.resval.rpoint); acedSetVar(L"LASTPOINT", &rbOsnap);

		ads_point ptInters; 
		iRet = acedOsnap(ptCable2, L"PER", ptInters);
		if (iRet == RTCAN) return;
		if (iRet != RTNORM) continue;

		// The width of the coverage would then be
		double dOffset = acutDistance(ptCable1, ptInters);

		//////////////////////////////////////////////////////////////////////////
		// Draw a dummy POINT entity. This will be erased after the PLINE is drawn.
		acedCommand(RTSTR, _T(".POINT"), RTSTR, _T("0,0"), NULL);
		ads_name enPoint; acdbEntLast(enPoint); AcDbObjectId objPointId; acdbGetObjectId(objPointId, enPoint);

		// Allow user to use the PLINE command
		int iRet = RTNORM;

		// THe start of the PLINE will be the point at the axis
		ads_point ptStart = { (ptCable1[X] + ptCable2[X]) / 2, (ptCable1[Y] + ptCable2[Y]) / 2, 0.0 };

		saveOSMode();
		acutPrintf(_T("\nSpecify points along coverage axis...\n"));
		iRet = acedCommand(RTSTR, _T(".PLINE"), RTPOINT, ptStart, NULL); 

		// Switch on the CMDECHO variable to see the PLINE prompts
		restoreOSMode();
		struct resbuf rbSetvar; rbSetvar.restype = RTSHORT; rbSetvar.resval.rint = 1; acedSetVar(_T("CMDECHO"), &rbSetvar);

		if (iRet == RTCAN) { return; }
		while (T)
		{
			iRet = acedCommand (RTSTR, PAUSE, NULL);
			if (iRet == RTCAN) 
			{
				// Check if the last entity drawn is a valid PLINE
				ads_name enPoly; acdbEntLast(enPoly); AcDbObjectId objPolyId; acdbGetObjectId(objPolyId, enPoly);
				if (objPointId == objPolyId) 
				{
					// Erase the POINT
					acdbEntDel(enPoint);
				}
				else
				{
					// Erase the POINT and PLINE
					acdbEntDel(enPoint); 
					acdbEntDel(enPoly); 
					acedCommand(RTSTR, _T(".REGEN"), NULL);
				}
				return; 
			}

			// Get the value in CMDNAMES
			struct resbuf rbSetvar; acedGetVar(_T("CMDNAMES"), &rbSetvar);
			CString csCmdNames; csCmdNames.Format(_T("%s"), rbSetvar.resval.rstring);
			if (csCmdNames.Find(_T("PLINE")) == -1) break;
		}

		// Switch on the CMDECHO variable to see the PLINE prompts
		rbSetvar.resval.rint = 0; acedSetVar(_T("CMDECHO"), &rbSetvar);

		// Check if the last entity drawn is a valid PLINE
		ads_name enPoly; acdbEntLast(enPoly); AcDbObjectId objPolyId; acdbGetObjectId(objPolyId, enPoly);
		if (objPointId == objPolyId) { acdbEntDel(enPoint);	return; }	else acdbEntDel(enPoint); // Erase the POINT

		// Draw the coverage for the selected path
		ads_point ptDummy;
		ElaborateCoverage(objPolyId, (dOffset * 0.60), csLayer, aryObjIds, ptDummy, L"Existing", L"Invalid", L"Invalid", 0.0); // "Existing" must be changed

		// Check if the user wants to UNDO or ACCEPT
		acedInitGet(NULL, _T("Undo Accept"));
		iRet = acedGetKword(_T("Confirm coverage depicted [Undo/Accept] <Accept>: "), result);
		/**/ if (iRet == RTNONE) {  aryObjIds.removeAll(); return; }
		else if (iRet == RTNORM)
		{
			/**/ if (result[0] == _T('A')) { aryObjIds.removeAll(); return; }
			else if (result[0] == _T('U'))
			{
				// Remove previously drawn duct coverage
				deleteArray(aryObjIds);
				aryObjIds.removeAll();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: DefineCoverageAxis
// Description  : 
//////////////////////////////////////////////////////////////////////////
AcDbObjectId DefineCoverageAxis(ads_point ptStart, ads_point ptEnd, AcDbObjectId &objPolyId, bool &bTraceReverse)
{
	// Validate the reference ID
	AcDbObjectId objAxisId;
	if (!objPolyId.isValid()) { return objAxisId; }

	// Duplicate the copy of the reference line on itself and get its object id
	ads_name enPoly; acdbGetAdsName(enPoly, objPolyId);
	acedCommand(RTSTR, L".COPY", RTENAME, enPoly, RTSTR, L"", RTSTR, L"0,0", RTSTR, L"", NULL);
	ads_name enRefId; acdbEntLast(enRefId);
	AcDbObjectId objRefId; acdbGetObjectId(objRefId, enRefId);

	Acad::ErrorStatus es;
	AcGePoint3d geBrkStart;
	AcGePoint3d geBrkEnd;
	AcGePoint3d gePtOnCurveAtStart;
	AcGePoint3d gePtOnCurveAtEnd;
	bTraceReverse = false;
	if (objRefId != AcDbObjectId::kNull)
	{
		AcDbEntity *pEntity;
		if (acdbOpenObject(pEntity, objRefId, AcDb::kForRead) != Acad::eOk) { return objAxisId; }

		if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc()))
		{
			if (pEntity->isKindOf(AcDbLine::desc()))
			{
				AcDbLine *pLine = AcDbLine::cast(pEntity);

				geBrkStart = pLine->startPoint();
				geBrkEnd   = pLine->endPoint();

				// Default end point
				if ((ptEnd[X] == -99999.99) && (ptEnd[Y] == -99999.99)) { ptEnd[X] = geBrkEnd.x; ptEnd[Y] = geBrkEnd.y; ptEnd[Z] = 0.0; }

				// Get the closest point to the start point
				if (acutDistance(asDblArray(geBrkStart), ptStart) < acutDistance(asDblArray(geBrkStart), ptEnd))
				{
					es = pLine->getClosestPointTo(AcGePoint3d(ptStart[X], ptStart[Y], 0.0), gePtOnCurveAtStart);
					es = pLine->getClosestPointTo(AcGePoint3d(ptEnd[X], ptEnd[Y], 0.0), gePtOnCurveAtEnd);
				}
				else
				{
					// Not usual
					bTraceReverse = true;
					es = pLine->getClosestPointTo(AcGePoint3d(ptEnd[X], ptEnd[Y], 0.0), gePtOnCurveAtStart);
					es = pLine->getClosestPointTo(AcGePoint3d(ptStart[X], ptStart[Y], 0.0), gePtOnCurveAtEnd);

					// Swap start point as we are going in the reverse direction
					acutPolar(ptEnd, 0.0, 0.0, ptStart);
				}

				pLine->close();
			}
			else if (pEntity->isKindOf(AcDbPolyline::desc()))
			{
				AcDbPolyline *pPLine = AcDbPolyline::cast(pEntity);

				// Get the end points of the polyline
				pPLine->getPointAt(0, geBrkStart);
				pPLine->getPointAt(pPLine->numVerts() - 1, geBrkEnd);

				// Default end point
				if ((ptEnd[X] == -99999.99) && (ptEnd[Y] == -99999.99)) { ptEnd[X] = geBrkEnd.x; ptEnd[Y] = geBrkEnd.y; ptEnd[Z] = 0.0; }

				// Get points on PLINE that are closest to the start and end points 
				AcGePoint3d gePt1; es = pPLine->getClosestPointTo(AcGePoint3d(ptStart[X], ptStart[Y], 0.0), gePt1);
				AcGePoint3d gePt2; es = pPLine->getClosestPointTo(AcGePoint3d(ptEnd[X], ptEnd[Y], 0.0), gePt2);

				// We must determine which if the two points are closer to the start point of the reference line
				double dParam;
				bool bOnSegStart = false, bOnSegEnd = false;
				for (int iSeg = 0; iSeg < pPLine->numVerts(); iSeg++)
				{					
					// Check if the start point in close to the start vertex.
					if (Adesk::kTrue == pPLine->onSegAt(iSeg, AcGePoint2d(gePt1.x, gePt1.y), dParam)) bOnSegStart = true;

					// Check if the start point in close to the start vertex.
					if (Adesk::kTrue == pPLine->onSegAt(iSeg, AcGePoint2d(gePt2.x, gePt2.y), dParam)) bOnSegEnd = true;

					// If both the vertices's are found on this segment, we are forced to find out which is closest
					if (bOnSegStart && !bOnSegEnd)
					{
						gePtOnCurveAtStart = gePt1;
						gePtOnCurveAtEnd   = gePt2;
						break;
					}
					else if (!bOnSegStart && bOnSegEnd)
					{
						// Not usual
						bTraceReverse = true;

						gePtOnCurveAtStart = gePt2;
						gePtOnCurveAtEnd   = gePt1;

						// Swap start point as we are going in the reverse direction
						acutPolar(ptEnd, 0.0, 0.0, ptStart);
						break;
					}
					else if (bOnSegStart && bOnSegEnd)
					{
						AcGePoint3d geVertex; pPLine->getPointAt(iSeg, geVertex);
						if (acutDistance(asDblArray(geVertex), ptStart) < acutDistance(asDblArray(geVertex), ptEnd))
						{
							gePtOnCurveAtStart = gePt1;
							gePtOnCurveAtEnd = gePt2;
						}
						else
						{
							// Not usual
							bTraceReverse = true;

							gePtOnCurveAtStart = gePt2;
							gePtOnCurveAtEnd   = gePt1;

							// Swap start point as we are going in the reverse direction
							acutPolar(ptEnd, 0.0, 0.0, ptStart);
						}
						break;
					}
				}

				// acutPrintf(L"\nStart-%d End-%d", bOnSegStart, bOnSegEnd);
				pPLine->close();
			}
			else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
			{
				AcDb2dPolyline *pPLine = AcDb2dPolyline::cast(pEntity);

				// Get the closest point to the end point
				AcGePoint3dArray gePtsArray;
				AcGeDoubleArray geDblArray;
				collectVertices(pPLine, gePtsArray, geDblArray, true);

				// Get the end points of the reference line
				geBrkStart = gePtsArray.at(0);
				geBrkEnd = gePtsArray.at(gePtsArray.length() - 1);

				// // Default end point
				if ((ptEnd[X] == -99999.99) && (ptEnd[Y] == -99999.99)) { ptEnd[X] = geBrkEnd.x; ptEnd[Y] = geBrkEnd.y; ptEnd[Z] = 0.0; }

				// Get the closest point to the start point
				if (acutDistance(asDblArray(geBrkStart), ptStart) < acutDistance(asDblArray(geBrkStart), ptEnd))
				{
					es = pPLine->getClosestPointTo(AcGePoint3d(ptStart[X], ptStart[Y], 0.0), gePtOnCurveAtStart);
					es = pPLine->getClosestPointTo(AcGePoint3d(ptEnd[X], ptEnd[Y], 0.0), gePtOnCurveAtEnd);
				}
				else
				{
					// Not usual
					bTraceReverse = true;
					es = pPLine->getClosestPointTo(AcGePoint3d(ptEnd[X], ptEnd[Y], 0.0), gePtOnCurveAtStart);
					es = pPLine->getClosestPointTo(AcGePoint3d(ptStart[X], ptStart[Y], 0.0), gePtOnCurveAtEnd);

					// Swap start point as we are going in the reverse direction
					acutPolar(ptEnd, 0.0, 0.0, ptStart);
				}

				pPLine->close();

				/////////////////////////////////////////////////////////////////////////
				// Break the reference curve at start and end points
				//////////////////////////////////////////////////////////////////////////
				saveOSMode();
				acedCommand(RTSTR, L".BREAK", RTLB, RTENAME, enRefId, RTPOINT, asDblArray(geBrkStart), RTLE, RTPOINT, asDblArray(gePtOnCurveAtStart), NULL);
				acdbEntLast(enRefId);
				acedCommand(RTSTR, L".BREAK", RTLB, RTENAME, enRefId, RTPOINT, asDblArray(geBrkEnd), RTLE, RTPOINT, asDblArray(gePtOnCurveAtEnd), NULL);
				acdbEntLast(enRefId);
				acedCommand(RTSTR, L".MOVE", RTENAME, enRefId, RTSTR, L"", RTPOINT, asDblArray(gePtOnCurveAtStart), RTPOINT, ptStart, NULL);
				restoreOSMode();

				acdbGetObjectId(objRefId, enRefId);

				// The start point of the new segment is the end point of the old segment
				if (!bTraceReverse)
				{
					ptStart[X] = gePtOnCurveAtEnd[X] + (ptStart[X] - gePtOnCurveAtStart[X]);
					ptStart[Y] = gePtOnCurveAtEnd[Y] + (ptStart[Y] - gePtOnCurveAtStart[Y]);
					ptStart[Z] = 0.0;
				}
				else
				{
					ptStart[X] = gePtOnCurveAtStart[X] + (ptStart[X] - gePtOnCurveAtStart[X]);
					ptStart[Y] = gePtOnCurveAtStart[Y] + (ptStart[Y] - gePtOnCurveAtStart[Y]);
					ptStart[Z] = 0.0;
				}

				return objRefId;
			}

			/////////////////////////////////////////////////////////////////////////
			// Break the reference curve at start and end points
			//////////////////////////////////////////////////////////////////////////
			saveOSMode();
			acedCommand(RTSTR, L".BREAK", RTLB, RTENAME, enRefId, RTPOINT, asDblArray(geBrkStart), RTLE, RTPOINT, asDblArray(gePtOnCurveAtStart), NULL);
			acedCommand(RTSTR, L".BREAK", RTLB, RTENAME, enRefId, RTPOINT, asDblArray(geBrkEnd), RTLE, RTPOINT, asDblArray(gePtOnCurveAtEnd), NULL);
			acedCommand(RTSTR, L".MOVE", RTENAME, enRefId, RTSTR, L"", RTPOINT, asDblArray(gePtOnCurveAtStart), RTPOINT, ptStart, NULL);
			restoreOSMode();

			// The start point of the new segment is the end point of the old segment
			if (!bTraceReverse)
			{
				ptStart[X] = gePtOnCurveAtEnd[X] + (ptStart[X] - gePtOnCurveAtStart[X]);
				ptStart[Y] = gePtOnCurveAtEnd[Y] + (ptStart[Y] - gePtOnCurveAtStart[Y]);
				ptStart[Z] = 0.0;
			}
			else
			{
				ptStart[X] = gePtOnCurveAtStart[X] + (ptStart[X] - gePtOnCurveAtStart[X]);
				ptStart[Y] = gePtOnCurveAtStart[Y] + (ptStart[Y] - gePtOnCurveAtStart[Y]);
				ptStart[Z] = 0.0;
			}

			return objRefId;
		}
		else
		{
			pEntity->erase();
			pEntity->close();
		}
	}

	return objAxisId;
}

//////////////////////////////////////////////////////////////////////////
// Function name: Command_DuctCoverageNEW()
// Description  : 
//////////////////////////////////////////////////////////////////////////
void Command_DuctCoverageNew()
{
	int iRet;
	TCHAR result[10];
	AcDbObjectIdArray aryObjIds;

	CString csLayer = L"BASE_DUCT_PROP";

	double dOffset;
	ads_point ptStart;
	while (T)
	{
		//////////////////////////////////////////////////////////////////////////
		// Get the cables b/w which the coverage has to be drawn
		//////////////////////////////////////////////////////////////////////////

		AcDbEntity *pEntity;
		AcDbObjectId objId;
		Acad::ErrorStatus es;

		ads_name enCable1; 
		ads_point ptCable1;

		while (T)
		{
			iRet = acedEntSel(_T("\rSelect the first outermost cable: "), enCable1, ptCable1);
			/**/ if (iRet == RTCAN) return;
			else if (iRet == RTNORM) 
			{
				// Check if the entity selected is a valid contour
				acdbGetObjectId(objId, enCable1);

				es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
				if (es != Acad::eOk) { acutPrintf(_T("\nError at %d: %s"), __LINE__, acadErrorStatusText(es)); return; }

				if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc())) 
				{
					pEntity->close();
					break;
				}
				pEntity->close();

				// Not a valid contour
				acutPrintf(_T("\nNot a valid cable.\n"));
			}
		}

		ads_name enCable2; 
		ads_point ptCable2;
		while (T)
		{
			iRet = acedEntSel(_T("\rSelect the second outermost cable: "), enCable2, ptCable2);
			/**/ if (iRet == RTCAN) return;
			else if (iRet == RTNORM) 
			{
				// Check if the entity selected is a valid contour
				acdbGetObjectId(objId, enCable2);

				es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
				if (es != Acad::eOk) { acutPrintf(_T("\nError at %d: %s"), __LINE__, acadErrorStatusText(es)); return; }

				if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc()))
				{
					pEntity->close();
					break;
				}
				pEntity->close();

				// Not a valid contour
				acutPrintf(_T("\nNot a valid cable.\n"));
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Get the distance b/w them
		//////////////////////////////////////////////////////////////////////////
		struct resbuf rbOsnap; rbOsnap.restype = RTPOINT;  
		acutPolar(ptCable1, 0.0, 0.0, rbOsnap.resval.rpoint); acedSetVar(L"LASTPOINT", &rbOsnap);

		ads_point ptInters; 
		iRet = acedOsnap(ptCable2, L"PER", ptInters);
		if (iRet == RTCAN) return;
		if (iRet != RTNORM) continue;

		// The width and the start point of the coverage would then be...
		dOffset = acutDistance(ptCable1, ptInters);
		if (dOffset > 0.0)
		{
			dOffset *= 1.8; 

			ptStart[X] = (ptCable1[X] + ptCable2[X]) / 2;
			ptStart[Y] = (ptCable1[Y] + ptCable2[Y]) / 2;
			ptStart[Z] = 0.0;

			// Correct the width
			CString csMsg; csMsg.Format(L"\nSpecify width <%.2f>: ", dOffset);

			acedInitGet(RSG_NONEG | RSG_NOZERO, L"");
			iRet = acedGetDist(ptStart, csMsg, &dOffset);
			break;	
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Get the choice to draw the coverage
	//////////////////////////////////////////////////////////////////////////
	ads_point ptTempEndPt = { -99999.99, -99999.99, -99999.99}; 
	AcDbObjectId objPolyId;
	acTransactionManagerPtr()->startTransaction();

	// The user has selected a valid line and see if the end point has been specified
	AcDbObjectId objAxisId;
	AcDbObjectId objNewSegment;
	while (T)
	{
		acedInitGet(NULL, L"Line Points Exit");
		iRet = acedGetKword(L"\nSpecify coverage route by [Line/Points] <Exit>: ", result);
		/**/ if (iRet == RTCAN)  {  acTransactionManagerPtr()->abortTransaction(); return; }
		else if (iRet == RTNONE)
		{
			// Call the function to duplicate the entire line
			if (objAxisId.isValid()) ElaborateCoverage(objAxisId, (dOffset * 0.60), csLayer, aryObjIds, ptStart, L"Existing", L"Invalid", L"Invalid", 0.0); // "Existing" must be changed

			acTransactionManagerPtr()->endTransaction(); 
			return; 
		}
		else if (iRet == RTNORM)
		{
			if (result[0] == _T('E')) 
			{
				// Call the function to duplicate the entire line
				if (objAxisId.isValid()) ElaborateCoverage(objAxisId, (dOffset * 0.60), csLayer, aryObjIds, ptStart, L"Existing", L"Invalid", L"Invalid", 0.0); // "Existing" must be changed

				// Exit be save
				acTransactionManagerPtr()->endTransaction();
				return;
			}
			/**/ if (result[0] == _T('L')) 
			{
				// Allow the user to select a line
				AcDbEntity *pEntity;
				Acad::ErrorStatus es;

				AcGePoint3d geFillerStartPoint;
				AcGePoint3d geFillerEndPoint;

				ads_name enRefCable; 
				ads_point ptRefCable;

				iRet = acedEntSel(_T("\rSelect the reference line: "), enRefCable, ptRefCable);
				/**/ if (iRet == RTCAN) continue;
				else if (iRet == RTNORM) 
				{
					// Check if the entity selected is a valid contour
					acdbGetObjectId(objPolyId, enRefCable);

					es = acdbOpenObject(pEntity, objPolyId, AcDb::kForRead);
					if (es != Acad::eOk) { acutPrintf(_T("\nError at %d: %s"), __LINE__, acadErrorStatusText(es)); return; }

					if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc())) 
					{
						pEntity->close();

						// Get the end point of the coverage
						acedInitGet(RSG_NONULL, L"");
						iRet = acedGetPoint(ptStart, L"\nSpecify end point of coverage: ", ptTempEndPt);
						if (iRet == RTCAN) continue;

						// Call the function to retrieve the points from the reference line
						bool bTraceReverse;
						objNewSegment = DefineCoverageAxis(ptStart, ptTempEndPt, objPolyId, bTraceReverse);

						// Open the existing coverage and add the new points
						if (objAxisId.isValid())
						{
							AcDbEntity *pChkEntity;
							bool bAxisIsLine = false;
							AcGePoint3d gePt1;
							AcGePoint3d gePt2;

							// Get the entity name of the coverage
							ads_name enObjAxis; acdbGetAdsName(enObjAxis, objAxisId);

							//////////////////////////////////////////////////////////////////////////
							// Filler - Get the end point of the new segment
							//////////////////////////////////////////////////////////////////////////
							if (acdbOpenObject(pChkEntity, objNewSegment, AcDb::kForRead) == Acad::eOk)
							{
								if (pChkEntity->isKindOf(AcDbLine::desc())) 
								{
									AcDbLine *pChkLine = AcDbLine::cast(pChkEntity); 
									geFillerEndPoint   = (bTraceReverse ? pChkLine->endPoint() : pChkLine->startPoint());
									pChkLine->close(); 

									// Previous entity is a LINE we must covert it to a PLINE
									bAxisIsLine = true; 
								}
								else if (pChkEntity->isKindOf(AcDbPolyline::desc()))
								{
									AcDbPolyline *pChkPLine = AcDbPolyline::cast(pChkEntity); 
									if (!bTraceReverse)
										pChkPLine->getPointAt(0, geFillerEndPoint); 
									else
										pChkPLine->getPointAt(pChkPLine->numVerts() - 1, geFillerEndPoint); 

									pChkPLine->close(); 
								}
								else if (pChkEntity->isKindOf(AcDb2dPolyline::desc()))
								{
									// Collect all vertices's of the PLINE
									AcDb2dPolyline *pChkPLine = AcDb2dPolyline::cast(pChkEntity); 
									AcGePoint3dArray geChkPtsArray;
									AcGeDoubleArray geChkDblArray;
									collectVertices(pChkPLine, geChkPtsArray, geChkDblArray, true);

									if (!bTraceReverse)
										geFillerEndPoint = geChkPtsArray.at(0); 
									else
										geFillerEndPoint = geChkPtsArray.at(geChkPtsArray.length() - 1); 

									pChkPLine->close(); 
								}
								else pChkEntity->close();
							}

							//////////////////////////////////////////////////////////////////////////
							// Filler - Get the end point of the prevailing axis
							//////////////////////////////////////////////////////////////////////////

							if (acdbOpenObject(pChkEntity, objAxisId, AcDb::kForRead) == Acad::eOk)
							{
								if (pChkEntity->isKindOf(AcDbLine::desc())) 
								{
									AcDbLine *pChkLine = AcDbLine::cast(pChkEntity); 
									gePt1 = pChkLine->startPoint();
									gePt2 = pChkLine->endPoint();

									// Previous entity is a LINE we must covert it to a PLINE
									bAxisIsLine = true; 
								}
								else if (pChkEntity->isKindOf(AcDbPolyline::desc()))
								{
									AcDbPolyline *pChkPLine = AcDbPolyline::cast(pChkEntity); 
									pChkPLine->getPointAt(0, gePt1); 
									pChkPLine->getPointAt(pChkPLine->numVerts() - 1, gePt2); 
								}
								else if (pChkEntity->isKindOf(AcDb2dPolyline::desc()))
								{
									AcDb2dPolyline *pChkPLine = AcDb2dPolyline::cast(pChkEntity); 
									AcGePoint3dArray geChkPtsArray;
									AcGeDoubleArray geChkDblArray;
									collectVertices(pChkPLine, geChkPtsArray, geChkDblArray, true);

									gePt1 = geChkPtsArray.at(0); 
									gePt2 = geChkPtsArray.at(geChkPtsArray.length() - 1); 
								}

								if (acutDistance(asDblArray(geFillerEndPoint), asDblArray(gePt1)) < acutDistance(asDblArray(geFillerEndPoint), asDblArray(gePt2)))
									geFillerStartPoint = gePt1;
								else
									geFillerStartPoint = gePt2;

								// Close the axis entity
								pChkEntity->close();
							}

							// If the new start point and the actual start point do not match, we shall introduce one more segment b/w them
							ads_name enNewSegment; acdbGetAdsName(enNewSegment, objNewSegment);
							if (acutDistance(asDblArray(geFillerStartPoint), asDblArray(geFillerEndPoint)) <= 0.00001)
							{
								saveOSMode();
								if (bAxisIsLine)
								{
									acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enObjAxis, RTPOINT, ptStart, RTLE, RTSTR, L"Y", RTSTR, L"JOIN", RTENAME, enNewSegment, RTSTR, L"", RTSTR, L"", NULL);
								}
								else
								{
									acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enObjAxis, RTPOINT, ptStart, RTLE, RTSTR, L"JOIN", RTENAME, enNewSegment, RTSTR, L"", RTSTR, L"", NULL);
								}
								restoreOSMode();
							}
							else
							{
								//////////////////////////////////////////////////////////////////////////
								// Create the mid segment
								//////////////////////////////////////////////////////////////////////////
								AcDbLine *pFillerSeg = new AcDbLine(geFillerStartPoint, geFillerEndPoint);
								appendEntityToDatabase(pFillerSeg);
								AcDbObjectId objFiller = pFillerSeg->objectId();
								pFillerSeg->close();

								ads_name enFiller; acdbGetAdsName(enFiller, objFiller);

								saveOSMode();
								if (bAxisIsLine)
									acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enObjAxis, RTPOINT, ptStart, RTLE, RTSTR, L"Y", RTSTR, L"JOIN", RTENAME, enFiller, RTENAME, enNewSegment, RTENAME, RTSTR, L"", RTSTR, L"", NULL);
								else
									acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enObjAxis, RTPOINT, ptStart, RTLE, RTSTR, L"JOIN", RTENAME, enFiller, RTENAME, enNewSegment, RTSTR, L"", RTSTR, L"", NULL);
								restoreOSMode();
							}
						}
						else
							objAxisId = objNewSegment;
					}
				}
			}
			else if (result[0] == _T('P')) 
			{
				// Enable the user to select a few points to define the coverage
				acTransactionManagerPtr()->startTransaction();
				while (T)
				{
					int iRetPts;
					ads_point ptNext;

					// Get the next point
					iRetPts = acedGetPoint(ptStart, L"\nSpecify next point or ENTER to finish: ", ptNext);
					/**/ if (iRetPts == RTCAN)  { acTransactionManagerPtr()->abortTransaction(); break; }
					else if (iRetPts == RTNONE) { acTransactionManagerPtr()->endTransaction(); break; }

					// Draw the new LINE segment
					// AcDbLine *pPtsLine = new AcDbLine(AcGePoint3d(ptStart[X], ptStart[Y], 0.0), AcGePoint3d(ptNext[X], ptNext[Y], 0.0));
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
							acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enObjAxis, RTPOINT, ptStart, RTLE, RTSTR, L"Y", RTSTR, L"JOIN", RTENAME, enNewSegment, RTSTR, L"", RTSTR, L"", NULL);
						else
							acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enObjAxis, RTPOINT, ptStart, RTLE, RTSTR, L"JOIN", RTENAME, enNewSegment, RTSTR, L"", RTSTR, L"", NULL);
						restoreOSMode();

						// Get the object ID
						ads_name enAxis; acdbEntLast(enAxis);
						acdbGetObjectId(objAxisId, enAxis);
					}
					else
					{
						objAxisId = objNewSegment;
					}
				}//
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name	  : GetValueFromRegistry
// Description	    : Opens the Windows registry, searches for the given key and under it for the given query value. If found, returns the stored value.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString GetValueFromRegistry(CString csKeyName, CString csSubKey, DWORD dwType)
{
  HKEY hKey;
  LONG lRet;
  ACHAR szValue[501];
  DWORD dwBufLen = 501, dwValue;
  CString csValue;

  if (RegOpenKeyEx(HKEY_CURRENT_USER, csKeyName, 0, KEY_EXECUTE, &hKey) == ERROR_SUCCESS)
  {
    if (dwType == REG_SZ) lRet = RegQueryValueEx(hKey, csSubKey, NULL, NULL, (LPBYTE)szValue, &dwBufLen);
    else lRet = RegQueryValueEx(hKey, csSubKey, NULL, NULL, (LPBYTE)&dwValue, &dwBufLen);
    if (lRet == ERROR_SUCCESS)
    {
      if (dwType == REG_SZ) csValue = szValue;
      else csValue.Format(_T("%d"), dwValue);
    }
    RegCloseKey(hKey);
  }

  return csValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name	  : SetValueInRegistry
// Description	    : Opens the Windows registry, searches for the given key and under it for the given query value. If found, sets the given value.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetValueInRegistry(CString csKeyName, CString csSubKey, DWORD dwType, CString csValue)
{
  HKEY hKey;
  DWORD dwValue = _wtof(csValue);
  ACHAR szValue[_MAX_PATH]; wcscpy(szValue, csValue);

  if (RegOpenKeyEx(HKEY_CURRENT_USER, csKeyName, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
  {
    if (dwType == REG_SZ) RegSetValueEx(hKey, csSubKey, 0, REG_SZ, (LPBYTE)szValue, _MAX_PATH);
    else RegSetValueEx(hKey, csSubKey, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
    RegCloseKey(hKey);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : SetDWGToolsSettings
// Description  : 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetDWGToolsSettings(double dOffset, double dWidth, int iState)
{
	CString csOffset;	csOffset.Format(L"%.1f", dOffset);
	CString csWidth;	csWidth.Format(L"%.1f",  dWidth);
	CString csState;  csState.Format(L"%d",    iState); 

	// Get the current profile name
	struct resbuf rbCProfile;
	acedGetVar(_T("CPROFILE"), &rbCProfile);
	CString csProfile = rbCProfile.resval.rstring;

	// Get and set the path to the DWG Tools registry key in the current profile
	CString csAcadKey  = _T("Software\\Autodesk\\AutoCAD");
	CString csAcadVer  = GetValueFromRegistry(csAcadKey, _T("CurVer"), REG_SZ);
	CString csAcadProd = GetValueFromRegistry(csAcadKey + _T("\\") + csAcadVer, _T("CurVer"), REG_SZ);
	CString csAcadPSKey; csAcadPSKey.Format(_T("%s\\%s\\%s\\Profiles\\%s\\Dialogs\\DWGTools"), csAcadKey, csAcadVer, csAcadProd, csProfile);

	// Set the existing values
	SetValueInRegistry(csAcadPSKey, _T("DCOffset"), REG_SZ, csOffset); 
	SetValueInRegistry(csAcadPSKey, _T("DCWidth"),  REG_SZ, csWidth);
	SetValueInRegistry(csAcadPSKey, _T("DCState"),  REG_SZ, csState);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ReadDWGToolsSettings
// Description  : 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ReadDWGToolsSettings(CString &csDCOffset, CString &csDCWidth, CString &csDCState)
{
	// Get the current profile name
	struct resbuf rbCProfile;
	acedGetVar(_T("CPROFILE"), &rbCProfile);
	CString csProfile = rbCProfile.resval.rstring;

	// Get and set the path to the DWGTools registry key in the current profile
	CString csAcadKey  = _T("Software\\Autodesk\\AutoCAD");
	CString csAcadVer  = GetValueFromRegistry(csAcadKey, _T("CurVer"), REG_SZ);
	CString csAcadProd = GetValueFromRegistry(csAcadKey + _T("\\") + csAcadVer, _T("CurVer"), REG_SZ);
	CString csAcadPSKey; csAcadPSKey.Format(_T("%s\\%s\\%s\\Profiles\\%s\\Dialogs\\DWGTools"), csAcadKey, csAcadVer, csAcadProd, csProfile);

	// Get the existing values
	csDCOffset = GetValueFromRegistry(csAcadPSKey, _T("DCOffset"), REG_SZ); if (csDCOffset.IsEmpty()) csDCOffset = _T("0.2");
	csDCWidth  = GetValueFromRegistry(csAcadPSKey, _T("DCWidth"),  REG_SZ); if (csDCWidth.IsEmpty())  csDCWidth  = _T("1.0");
	csDCState  = GetValueFromRegistry(csAcadPSKey, _T("DCState"),  REG_SZ); if (csDCState.IsEmpty())  csDCState  = _T("0");
}

//////////////////////////////////////////////////////////////////////////
// Function name: OffsetSegment1ForSegment2
// Description  : Offsets the given segment and retrieves its end points.
//////////////////////////////////////////////////////////////////////////
void OffsetSegment1ForSegment2(ads_name enSegment1, ads_point ptOnSegment1, double dOffset, ads_point ptSide, ads_name enSegment2, AcGePoint3d &geptSeg2Start, AcGePoint3d &geptSeg2End)
{
	// Draw the second segment of the coverage
	ChangeProperties(false, enSegment1, 3, FALSE, FALSE, 1.0);
	acedCommand(RTSTR, L".OFFSET", RTREAL, dOffset, RTLB, RTENAME, enSegment1, RTPOINT, ptOnSegment1, RTLE, RTPOINT, ptSide, RTSTR, L"", NULL);
	acdbEntLast(enSegment2);

	// Get the closest point on the new segment
	AcDbEntity *pEntity;
	AcDbObjectId objId;
	acdbGetObjectId(objId, enSegment2);
	acdbOpenObject(pEntity, objId, AcDb::kForRead);

	if (pEntity->isKindOf(AcDbLine::desc()))
	{
		AcDbLine *pSegment = AcDbLine::cast(pEntity);

		pSegment->getStartPoint(geptSeg2Start);
		pSegment->getEndPoint(geptSeg2End);

		pSegment->close();
	}
	else if (pEntity->isKindOf(AcDbPolyline::desc()))
	{
		AcDbPolyline *pSegment = AcDbPolyline::cast(pEntity);

		pSegment->getPointAt(0, geptSeg2Start);
		pSegment->getPointAt(pSegment->numVerts() - 1, geptSeg2End);
		pSegment->close();
	}
	else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
	{
		AcDb2dPolyline *pSegment = AcDb2dPolyline::cast(pEntity);
		pSegment->close();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus getEndPoints(ads_name enSegment, AcGePoint3d &geStart, AcGePoint3d &geEnd)
{
	//////////////////////////////////////////////////////////////////////////
	// Open the first entity to get its end points
	//////////////////////////////////////////////////////////////////////////
	Acad::ErrorStatus es;
	AcDbEntity *pEntity;
	AcDbObjectId objId;

	// Get the object id
	es = acdbGetObjectId(objId, enSegment);
	if (es != Acad::eOk) return es;

	// Open the entity
	es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
	if (es != Acad::eOk) return es;

	if (pEntity->isKindOf(AcDbLine::desc()))
	{
		AcDbLine *pSegment = AcDbLine::cast(pEntity);

		pSegment->getStartPoint(geStart);
		pSegment->getEndPoint(geEnd);

		pSegment->close();
	}
	else if (pEntity->isKindOf(AcDbPolyline::desc()))
	{
		AcDbPolyline *pSegment = AcDbPolyline::cast(pEntity);

		pSegment->getPointAt(0, geStart);
		pSegment->getPointAt(pSegment->numVerts() - 1, geEnd);
		pSegment->close();
	}
	else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
	{
		AcDb2dPolyline *pSegment = AcDb2dPolyline::cast(pEntity);

		AcGePoint3dArray gePtsArray;
		AcGeDoubleArray geDblArray;
		collectVertices(pSegment, gePtsArray, geDblArray, true);
		pSegment->close();

		// Start and end points
		geStart = gePtsArray.at(0);
		geEnd   = gePtsArray.at(gePtsArray.length() - 1);
	}
	else
	{
		pEntity->close();
		return Acad::eNotThatKindOfClass;
	}

	return Acad::eOk;
}

//////////////////////////////////////////////////////////////////////////
// Function name: DrawPolygon
// Description  : Draws a single polygon defined from Segment 1 and 2
//////////////////////////////////////////////////////////////////////////
void DrawPolygon(ads_name enSegment1, ads_name enSegment2, ads_name enPolygon)
{
	AcGePoint3d geptSeg1Start;
	AcGePoint3d geptSeg1End;
	AcGePoint3d geptSeg2Start;
	AcGePoint3d geptSeg2End;

	// Get the end points of the first segment
	if (getEndPoints(enSegment1, geptSeg1Start, geptSeg1End) != Acad::eOk) return;

	// Get the end points of the first segment
	if (getEndPoints(enSegment2, geptSeg2Start, geptSeg2End) != Acad::eOk) return;

	// Draw the closing segments of the segments
	acedCommand(RTSTR, L".LINE", RTPOINT, asDblArray(geptSeg1Start), RTPOINT, asDblArray(geptSeg2Start), RTSTR, L"", NULL);
	ads_name enCloseSeg1; acdbEntLast(enCloseSeg1);

	acedCommand(RTSTR, L".LINE", RTPOINT, asDblArray(geptSeg1End), RTPOINT, asDblArray(geptSeg2End), RTSTR, L"", NULL);
	ads_name enCloseSeg2; acdbEntLast(enCloseSeg2);

	// Form the polygon
	acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enCloseSeg1, RTPOINT, asDblArray(geptSeg1Start), RTLE, RTSTR, L"Y", RTSTR, L"J", RTENAME, enSegment1, RTENAME, enSegment2, RTENAME, enCloseSeg2, RTSTR, L"", RTSTR, L"", NULL);
	acdbEntLast(enPolygon);

	// Add the vertices of the first and second closing segments
	acdbRegApp(L"ECPATURE_MERGEDATA");
	struct resbuf *rbpVertices = acutBuildList(AcDb::kDxfRegAppName, L"ECPATURE_MERGEDATA", AcDb::kDxfXdXCoord, asDblArray(geptSeg1Start),
		AcDb::kDxfXdXCoord, asDblArray(geptSeg1End),
		AcDb::kDxfXdXCoord, asDblArray(geptSeg2Start),
		AcDb::kDxfXdXCoord, asDblArray(geptSeg2End), NULL);

	addXDataToEntity(enPolygon, rbpVertices);
	acutRelRb(rbpVertices);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetActualIntersection(ads_name enSegment1, ads_point pt1, ads_name enSegment2, ads_point pt2, ads_point ptInters)
{
	// Open the entities and check if the given point is on it
	Acad::ErrorStatus es;
	AcDbEntity *pEntity;
	AcDbObjectId objId;

	int iReturn = 0;
	es = acdbGetObjectId(objId, enSegment1); if (es != Acad::eOk) { return -1; }
	es = acdbOpenObject(pEntity, objId, AcDb::kForRead); if (es != Acad::eOk) { return -1; }
	AcDbCurve *pCurve1 = (AcDbCurve *) pEntity; pEntity->close();
	double dParam1; es = pCurve1->getParamAtPoint(AcGePoint3d(pt1[X], pt1[Y], 0.0), dParam1); if (es != Acad::eOk) iReturn += 1;

	es = acdbGetObjectId(objId, enSegment2); if (es != Acad::eOk) { return -1; }
	es = acdbOpenObject(pEntity, objId, AcDb::kForRead); if (es != Acad::eOk) {  return -1; }
	AcDbCurve *pCurve2 = (AcDbCurve *) pEntity; pEntity->close();
	double dParam2; es = pCurve2->getParamAtPoint(AcGePoint3d(pt2[X], pt2[Y], 0.0), dParam2); if (es != Acad::eOk) iReturn += 2;

	// If one of the points don't lie on the segment specified, exit the function!
	if (iReturn) return iReturn;

	// Get the closest/nearest point on these two entities
	ads_point ptClose1, ptClose2;
	ads_point ptNear1, ptNear2;

	es = getClosestPoint(enSegment1, pt1, ptClose1, ptNear1); if (es != Acad::eOk) { return -1; }
	es = getClosestPoint(enSegment2, pt2, ptClose2, ptNear2); if (es != Acad::eOk) { return -1; }

	// Now try the inters to get the actual intersection
	ads_point ptRslt;
	if (acdbInters(ptClose1, ptNear1, ptClose2, ptNear2, 0, ptRslt) == RTNORM) { acutPolar(ptRslt, 0.0, 0.0, ptInters); }

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Function name: MergePolygons
// Description  : Merges the two polygons to one
//////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus MergePolygons(ads_name enPolygon1, ads_name enPolygon2)
{
	// Return Acad::eOk;
	AcDbPolyline *pEntity;
	AcDbObjectId objId;
	Acad::ErrorStatus es;

	//////////////////////////////////////////////////////////////////////////
	// Get the long vertices on either side of the first polygon
	//////////////////////////////////////////////////////////////////////////
	es = acdbGetObjectId(objId, enPolygon1);
	if (es != Acad::eOk) return es;

	es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
	if (es != Acad::eOk) return es;

	AcGePoint3d geptVertex;
	AcGePoint3dArray geptVertices;
	for (int i = 0; i < pEntity->numVerts(); i++)
	{
		pEntity->getPointAt(i, geptVertex);
		geptVertices.append(geptVertex);
	}

	int iNumVerPolygon1 = pEntity->numVerts();
	struct resbuf *rbpXD1 = pEntity->xData(L"ECPATURE_MERGEDATA");
	pEntity->close();

	AcGePoint3d gept10 = geptVertices.at(0); 
	AcGePoint3d gept11 = geptVertices.at(1); 

	AcGePoint3d gept12 = geptVertices.at(geptVertices.length() - 2); 
	AcGePoint3d gept13 = geptVertices.at(geptVertices.length() - 1); 

	//////////////////////////////////////////////////////////////////////////
	// Get the long vertices on either side of the second polygon
	//////////////////////////////////////////////////////////////////////////
	es = acdbGetObjectId(objId, enPolygon2);
	if (es != Acad::eOk) return es;

	es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
	if (es != Acad::eOk) return es;

	geptVertices.removeAll();
	for (int i = 0; i < pEntity->numVerts(); i++)
	{
		pEntity->getPointAt(i, geptVertex);
		geptVertices.append(geptVertex);
	}

	int iNumVerPolygon2 = pEntity->numVerts();

	// Get the XDATA attached to the PLINE
	struct resbuf *rbpXD2 = pEntity->xData(L"ECPATURE_MERGEDATA");
	pEntity->close();

	AcGePoint3d gept20 = geptVertices.at(0); 
	AcGePoint3d gept21 = geptVertices.at(1); 

	AcGePoint3d gept22 = geptVertices.at(geptVertices.length() - 2); 
	AcGePoint3d gept23 = geptVertices.at(geptVertices.length() - 1); 

	// Open the polygons
	acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enPolygon1, RTPOINT, asDblArray(gept10), RTLE, RTSTR, L"OPEN", RTSTR, L"", NULL);
	acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enPolygon2, RTPOINT, asDblArray(gept20), RTLE, RTSTR, L"OPEN", RTSTR, L"", NULL);

	// Add these two polygons to the selection set
	ads_name ssSel; 
	acedSSAdd(NULL, NULL, ssSel); 
	acedSSAdd(enPolygon1, ssSel, ssSel); 
	acedSSAdd(enPolygon2, ssSel, ssSel);

	ads_point ptPoly1_1;
	ads_point ptPoly1_2;
	ads_point ptPoly1_3;
	ads_point ptPoly1_4;

	ads_point ptPoly2_1;
	ads_point ptPoly2_2;
	ads_point ptPoly2_3;
	ads_point ptPoly2_4;

	// Put the closing segments for the new resultant PLINE in the making
	ads_name enJoin1, enJoin2;
	TCHAR result[10];

	// Break the PLINE at vertices from XDATA
	acedCommand(RTSTR, L".BREAK", RTLB, RTENAME, enPolygon1, RTPOINT, rbpXD1->rbnext->resval.rpoint, RTLE, RTPOINT, rbpXD1->rbnext->rbnext->rbnext->resval.rpoint, NULL);
	acdbEntLast(enJoin1); acedSSAdd(enJoin1, ssSel, ssSel);
	acedCommand(RTSTR, L".BREAK", RTLB, RTENAME, enPolygon2, RTPOINT, rbpXD2->rbnext->resval.rpoint, RTLE, RTPOINT, rbpXD2->rbnext->rbnext->rbnext->resval.rpoint, NULL);
	acdbEntLast(enJoin2); acedSSAdd(enJoin2, ssSel, ssSel);

	ads_point ptInters11, ptInters12, ptInters21, ptInters22;
	ads_point ptF1, ptF2, ptF3, ptF4;
	ads_point ptS1, ptS2, ptS3, ptS4;

	ads_point ptF1End; 
	ads_point ptF2End; 

	ads_point ptS1End; 
	ads_point ptS2End; 

	ads_point ptC11End, ptC12End;
	ads_point ptC21End, ptC22End;

	ads_name enJ1, enJ2, enJ3, enJ4, enJ5, enJ6;
	bool bFlip = false;
	bool bParallel = false;

	acutPolar(rbpXD2->rbnext->resval.rpoint, 0.0, 0.0, ptS1);
	acutPolar(rbpXD2->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptS2);
	acutPolar(rbpXD2->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptS3);
	acutPolar(rbpXD2->rbnext->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptS4);

	while (T)
	{
		if (!bFlip)
		{
			acutPolar(rbpXD1->rbnext->resval.rpoint, 0.0, 0.0, ptF1);
			acutPolar(rbpXD1->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF2);
			acutPolar(rbpXD1->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF3);
			acutPolar(rbpXD1->rbnext->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF4);
		}
		else
		{
			acutPolar(rbpXD1->rbnext->resval.rpoint, 0.0, 0.0, ptF3);
			acutPolar(rbpXD1->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF4);
			acutPolar(rbpXD1->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF1);
			acutPolar(rbpXD1->rbnext->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF2);
		}

		// if (acutDistance(ptF1, ptS1) <= acutDistance(ptF1, ptS2))	{	iRet1 = acdbInters(ptF1, ptF2, ptS1, ptS2, 0, ptInters11); }
		int iRet1 = acdbInters(ptF1, ptF2, ptS1, ptS2, 0, ptInters11);
		int iRet2 = acdbInters(ptF3, ptF4, ptS3, ptS4, 0, ptInters22);

		if (iRet1 == RTERROR) { bParallel = true; break;  }
		if (iRet2 == RTERROR) { bParallel = true; break;	}

		// First Line on Polygon 1
		if (acutDistance(ptF1, ptInters11) <= acutDistance(ptF2, ptInters11))	
		{
			acutPolar(ptF1, 0.0, 0.0, ptF1End);
			acutPolar(ptF2, 0.0, 0.0, ptC11End); 
		}
		else
		{
			acutPolar(ptF2, 0.0, 0.0, ptF1End);
			acutPolar(ptF1, 0.0, 0.0, ptC11End);
		}

		// Second Line on Polygon 1
		if (acutDistance(ptF3, ptInters22) <= acutDistance(ptF4, ptInters22))
		{
			acutPolar(ptF3, 0.0, 0.0, ptF2End);
			acutPolar(ptF4, 0.0, 0.0, ptC12End);
		}
		else	
		{
			acutPolar(ptF4, 0.0, 0.0, ptF2End);
			acutPolar(ptF3, 0.0, 0.0, ptC12End);
		}

		// First Line on Polygon 2
		if (acutDistance(ptS1, ptInters11) <= acutDistance(ptS2, ptInters11))
		{
			acutPolar(ptS1, 0.0, 0.0, ptS1End); 
			acutPolar(ptS2, 0.0, 0.0, ptC21End); 
		}
		else
		{
			acutPolar(ptS2, 0.0, 0.0, ptS1End);
			acutPolar(ptS1, 0.0, 0.0, ptC21End); 
		}

		if (acutDistance(ptS3, ptInters22) <= acutDistance(ptS4, ptInters22))
		{
			acutPolar(ptS3, 0.0, 0.0, ptS2End); 
			acutPolar(ptS4, 0.0, 0.0, ptC22End); 
		}
		else
		{
			acutPolar(ptS4, 0.0, 0.0, ptS2End);
			acutPolar(ptS3, 0.0, 0.0, ptC22End); 
		}

		// Calculate the actual intersection (This is important when the points retrieved are not on the same edges of the polygon)
		int iErrorInSide;
		iErrorInSide = GetActualIntersection(enJoin1, ptF1End, enJoin2, ptS1End, ptInters11); 
		/**/ if (iErrorInSide == 1)	{ iErrorInSide = GetActualIntersection(enPolygon1, ptF1End, enJoin2,		ptS1End, ptInters11); }
		else if (iErrorInSide == 2)	{ iErrorInSide = GetActualIntersection(enJoin1,    ptF1End, enPolygon2, ptS1End, ptInters11); }
		else if (iErrorInSide == 3)	{ iErrorInSide = GetActualIntersection(enPolygon1, ptF1End, enPolygon2, ptS1End, ptInters11); }

		iErrorInSide = GetActualIntersection(enJoin1, ptF2End, enJoin2, ptS2End, ptInters22);  
		/**/ if (iErrorInSide == 1)	{ iErrorInSide = GetActualIntersection(enPolygon1, ptF2End, enJoin2,		ptS2End, ptInters22); }
		else if (iErrorInSide == 2)	{ iErrorInSide = GetActualIntersection(enJoin1,    ptF2End, enPolygon2, ptS2End, ptInters22); }
		else if (iErrorInSide == 3)	{ iErrorInSide = GetActualIntersection(enPolygon1, ptF2End, enPolygon2, ptS2End, ptInters22); }

		////////////////
		// First Line on Polygon 1
		if (acutDistance(ptF1, ptInters11) <= acutDistance(ptF2, ptInters11))	
		{
			acutPolar(ptF1, 0.0, 0.0, ptF1End);
			acutPolar(ptF2, 0.0, 0.0, ptC11End); 
		}
		else
		{
			acutPolar(ptF2, 0.0, 0.0, ptF1End);
			acutPolar(ptF1, 0.0, 0.0, ptC11End);
		}

		// Second Line on Polygon 1
		if (acutDistance(ptF3, ptInters22) <= acutDistance(ptF4, ptInters22))
		{
			acutPolar(ptF3, 0.0, 0.0, ptF2End);
			acutPolar(ptF4, 0.0, 0.0, ptC12End);
		}
		else	
		{
			acutPolar(ptF4, 0.0, 0.0, ptF2End);
			acutPolar(ptF3, 0.0, 0.0, ptC12End);
		}

		// First Line on Polygon 2
		if (acutDistance(ptS1, ptInters11) <= acutDistance(ptS2, ptInters11))
		{
			acutPolar(ptS1, 0.0, 0.0, ptS1End); 
			acutPolar(ptS2, 0.0, 0.0, ptC21End); 
		}
		else
		{
			acutPolar(ptS2, 0.0, 0.0, ptS1End);
			acutPolar(ptS1, 0.0, 0.0, ptC21End); 
		}

		if (acutDistance(ptS3, ptInters22) <= acutDistance(ptS4, ptInters22))
		{
			acutPolar(ptS3, 0.0, 0.0, ptS2End); 
			acutPolar(ptS4, 0.0, 0.0, ptC22End); 
		}
		else
		{
			acutPolar(ptS4, 0.0, 0.0, ptS2End);
			acutPolar(ptS3, 0.0, 0.0, ptC22End); 
		}

		////////////////

		acedCommand(RTSTR, L".LINE", RTPOINT, ptF1End,    RTPOINT, ptInters11, RTSTR, L"", NULL); acdbEntLast(enJ1); ChangeProperties(false, enJ1, 1, NULL, NULL, 1);
		acedCommand(RTSTR, L".LINE", RTPOINT, ptInters11, RTPOINT, ptS1End,    RTSTR, L"", NULL); acdbEntLast(enJ2); ChangeProperties(false, enJ2, 2, NULL, NULL, 1);

		acedCommand(RTSTR, L".LINE", RTPOINT, ptF2End,    RTPOINT, ptInters22, RTSTR, L"", NULL); acdbEntLast(enJ3); ChangeProperties(false, enJ3, 3, NULL, NULL, 1);
		acedCommand(RTSTR, L".LINE", RTPOINT, ptInters22, RTPOINT, ptS2End,    RTSTR, L"", NULL); acdbEntLast(enJ4); ChangeProperties(false, enJ4, 4, NULL, NULL, 1);

		acedCommand(RTSTR, L".LINE", RTPOINT, ptC11End,   RTPOINT, ptC12End,   RTSTR, L"", NULL); acdbEntLast(enJ5); ChangeProperties(false, enJ5, 5, NULL, NULL, 1);
		acedCommand(RTSTR, L".LINE", RTPOINT, ptC21End,   RTPOINT, ptC22End,   RTSTR, L"", NULL); acdbEntLast(enJ6); ChangeProperties(false, enJ6, 6, NULL, NULL, 1);

		acedInitGet(NULL, _T("Flip Accept"));
		int iRet = acedGetKword(_T("\nConfirm resultant polygon [Flip/Accept] <Accept>: "), result);
		/**/ if (iRet == RTNONE)
		{
			acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enJ5, RTPOINT, ptC11End, RTLE, RTSTR, L"Y", RTSTR, L"J", 
				RTPICKS, ssSel, 
				RTENAME, enJ2, RTENAME, enJ3, RTENAME, enJ4, RTENAME, enJ1, 
				RTSTR, L"", RTSTR, L"", NULL);
			acdbEntLast(enPolygon1);

			// This is done to ensure that the closing segment is the one joined
			acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enPolygon1, RTPOINT, ptC11End, RTLE, RTSTR, L"J", RTENAME, enJ6,	RTSTR, L"", RTSTR, L"", NULL);
			acdbEntLast(enPolygon1);

			acedSSFree(ssSel);

			// Add the vertices of the first and second closing segments
			struct resbuf *rbpVertices = acutBuildList(AcDb::kDxfRegAppName, L"ECPATURE_MERGEDATA", 
				AcDb::kDxfXdXCoord, ptC11End,
				AcDb::kDxfXdXCoord, ptC21End,
				AcDb::kDxfXdXCoord, ptC12End,
				AcDb::kDxfXdXCoord, ptC22End, NULL);
			addXDataToEntity(enPolygon1, rbpVertices);
			acutRelRb(rbpVertices);

			return Acad::eOk; 
		}
		else if (iRet == RTNORM)
		{
			/**/ if (result[0] == _T('A')) 
			{
				acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enJ5, RTPOINT, ptC11End, RTLE, RTSTR, L"Y", RTSTR, L"J", 
					RTPICKS, ssSel, 
					RTENAME, enJ2, RTENAME, enJ3, RTENAME, enJ4, RTENAME, enJ1, 
					RTSTR, L"", RTSTR, L"", NULL);

				acdbEntLast(enPolygon1);

				// This is done to ensure that the closing segment is the one joined
				acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enPolygon1, RTPOINT, ptC11End, RTLE, RTSTR, L"J", RTENAME, enJ6,	RTSTR, L"", RTSTR, L"", NULL);
				acdbEntLast(enPolygon1);

				acedSSFree(ssSel);

				// Add the vertices of the first and second closing segments
				struct resbuf *rbpVertices = acutBuildList(AcDb::kDxfRegAppName, L"ECPATURE_MERGEDATA", 
					AcDb::kDxfXdXCoord, ptC11End,
					AcDb::kDxfXdXCoord, ptC21End,
					AcDb::kDxfXdXCoord, ptC12End,
					AcDb::kDxfXdXCoord, ptC22End, NULL);
				addXDataToEntity(enPolygon1, rbpVertices);
				acutRelRb(rbpVertices);

				return Acad::eOk; 
			}
			else if (result[0] == _T('F'))
			{
				// Delete the previously placed entities
				acdbEntDel(enJ1); acdbEntDel(enJ2); acdbEntDel(enJ3); acdbEntDel(enJ4); acdbEntDel(enJ5); acdbEntDel(enJ6);
				bFlip = !bFlip;
			}
		}
	}

	// Convert all the segments to one single PLINE
	bFlip = false;
	while (bParallel && T)
	{		
		if (!bFlip)
		{
			acutPolar(rbpXD1->rbnext->resval.rpoint, 0.0, 0.0, ptF1);
			acutPolar(rbpXD1->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF2);
			acutPolar(rbpXD1->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF3);
			acutPolar(rbpXD1->rbnext->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF4);
		}
		else
		{
			acutPolar(rbpXD1->rbnext->resval.rpoint, 0.0, 0.0, ptF3);
			acutPolar(rbpXD1->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF4);
			acutPolar(rbpXD1->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF1);
			acutPolar(rbpXD1->rbnext->rbnext->rbnext->rbnext->resval.rpoint, 0.0, 0.0, ptF2);
		}

		if (acutDistance(ptF1, ptS1) <= acutDistance(ptF2, ptS1))
		{
			acutPolar(ptF1, 0.0, 0.0, ptF1End);	
			acutPolar(ptF2, 0.0, 0.0, ptC11End);	
		}
		else 
		{
			acutPolar(ptF2, 0.0, 0.0, ptF1End);
			acutPolar(ptF1, 0.0, 0.0, ptC11End);	
		}

		if (acutDistance(ptF1End, ptS1) <= acutDistance(ptF1End, ptS2))
		{
			acutPolar(ptS1, 0.0, 0.0, ptS1End);	
			acutPolar(ptS2, 0.0, 0.0, ptC21End);	
		}
		else
		{
			acutPolar(ptS2, 0.0, 0.0, ptS1End);
			acutPolar(ptS1, 0.0, 0.0, ptC21End);	
		}

		if (acutDistance(ptF3, ptS3) <= acutDistance(ptF4, ptS3))
		{
			acutPolar(ptF3, 0.0, 0.0, ptF2End);	
			acutPolar(ptF4, 0.0, 0.0, ptC12End);
		}
		else
		{
			acutPolar(ptF4, 0.0, 0.0, ptF2End);
			acutPolar(ptF3, 0.0, 0.0, ptC12End);
		}

		if (acutDistance(ptF2End, ptS3) <= acutDistance(ptF2End, ptS4))
		{
			acutPolar(ptS3, 0.0, 0.0, ptS2End);	
			acutPolar(ptS4, 0.0, 0.0, ptC22End);
		}
		else
		{
			acutPolar(ptS4, 0.0, 0.0, ptS2End);
			acutPolar(ptS3, 0.0, 0.0, ptC22End);
		}

		acedCommand(RTSTR, L".LINE", RTPOINT, ptF1End,  RTPOINT, ptS1End, RTSTR, L"", NULL);  acdbEntLast(enJ1); ChangeProperties(false, enJ1, 1, NULL, NULL, 1);
		acedCommand(RTSTR, L".LINE", RTPOINT, ptF2End,  RTPOINT, ptS2End, RTSTR, L"", NULL);  acdbEntLast(enJ2); ChangeProperties(false, enJ2, 2, NULL, NULL, 1);
		acedCommand(RTSTR, L".LINE", RTPOINT, ptC11End, RTPOINT, ptC12End, RTSTR, L"", NULL); acdbEntLast(enJ3); ChangeProperties(false, enJ3, 3, NULL, NULL, 1);
		acedCommand(RTSTR, L".LINE", RTPOINT, ptC21End, RTPOINT, ptC22End, RTSTR, L"", NULL); acdbEntLast(enJ4); ChangeProperties(false, enJ4, 4, NULL, NULL, 1);

		acedInitGet(NULL, _T("Flip Accept"));
		int iRet = acedGetKword(_T("\nConfirm resultant polygon [Flip/Accept] <Accept>: "), result);
		/**/ if (iRet == RTNONE) 
		{
			acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enJ3, RTPOINT, ptC12End, RTLE, RTSTR, L"Y", RTSTR, L"J", 
				RTPICKS, ssSel, 
				RTENAME, enJ2, RTENAME, enJ1, 
				RTSTR, L"", RTSTR, L"", NULL);
			acdbEntLast(enPolygon1);

			acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enPolygon1, RTPOINT, ptC12End, RTLE, RTSTR, L"Y", RTSTR, L"J", RTENAME, enJ4, RTSTR, L"", RTSTR, L"", NULL); 
			acdbEntLast(enPolygon1);

			acedSSFree(ssSel);

			// Add the vertices of the first and second closing segments
			struct resbuf *rbpVertices = acutBuildList(AcDb::kDxfRegAppName, L"ECPATURE_MERGEDATA", 
				AcDb::kDxfXdXCoord, ptC11End,
				AcDb::kDxfXdXCoord, ptC21End,
				AcDb::kDxfXdXCoord, ptC12End,
				AcDb::kDxfXdXCoord, ptC22End, NULL);
			addXDataToEntity(enPolygon1, rbpVertices);
			acutRelRb(rbpVertices);

			return Acad::eOk; 
		}
		else if (iRet == RTNORM)
		{
			/**/ if (result[0] == _T('A')) 
			{
				acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enJ3, RTPOINT, ptC12End, RTLE, RTSTR, L"Y", RTSTR, L"J", 
					RTPICKS, ssSel, 
					RTENAME, enJ2, RTENAME, enJ1, 
					RTSTR, L"", RTSTR, L"", NULL);
				acdbEntLast(enPolygon1);

				acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enPolygon1, RTPOINT, ptC12End, RTLE, RTSTR, L"Y", RTSTR, L"J", RTENAME, enJ4, RTSTR, L"", RTSTR, L"", NULL); 
				acdbEntLast(enPolygon1);

				acedSSFree(ssSel);

				// Add the vertices of the first and second closing segments
				struct resbuf *rbpVertices = acutBuildList(AcDb::kDxfRegAppName, L"ECPATURE_MERGEDATA", 
					AcDb::kDxfXdXCoord, ptC11End,
					AcDb::kDxfXdXCoord, ptC21End,
					AcDb::kDxfXdXCoord, ptC12End,
					AcDb::kDxfXdXCoord, ptC22End, NULL);
				addXDataToEntity(enPolygon1, rbpVertices);
				acutRelRb(rbpVertices);

				return Acad::eOk; 
			}
			else if (result[0] == _T('F'))
			{
				// Delete the previously placed entities
				acdbEntDel(enJ1); acdbEntDel(enJ2); acdbEntDel(enJ3); acdbEntDel(enJ4); 
				bFlip = !bFlip;
			}
		}
	}

	return Acad::eOk;
}

//////////////////////////////////////////////////////////////////////////
// Function name: MergePolygonsOld
// Description  : Merges the two polygons to one
//////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus MergePolygonsOld(ads_name enPolygon1, ads_name enPolygon2)
{
	AcDbPolyline *pEntity;
	AcDbObjectId objId;
	Acad::ErrorStatus es;

	// Get the long vertices on either side of the first polygon
	es = acdbGetObjectId(objId, enPolygon1);
	if (es != Acad::eOk) return es;

	es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
	if (es != Acad::eOk) return es;

	AcGePoint3d geptVertex;
	AcGePoint3dArray geptVertices;
	for (int i = 0; i < pEntity->numVerts(); i++)
	{
		pEntity->getPointAt(i, geptVertex);
		geptVertices.append(geptVertex);
	}

	int iNumVerPolygon1 = pEntity->numVerts();
	pEntity->close();

	AcGePoint3d gept10 = geptVertices.at(0); 
	AcGePoint3d gept11 = geptVertices.at(1); 

	AcGePoint3d gept12 = geptVertices.at(geptVertices.length() - 2); 
	AcGePoint3d gept13 = geptVertices.at(geptVertices.length() - 1); 

	// Get the long vertices on either side of the second polygon
	es = acdbGetObjectId(objId, enPolygon2);
	if (es != Acad::eOk) return es;

	es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
	if (es != Acad::eOk) return es;

	AcGePoint3d gept20; es = pEntity->getPointAt(0, gept20); if (es != Acad::eOk) { pEntity->close(); return es; }
	AcGePoint3d gept21; es = pEntity->getPointAt(1, gept21); if (es != Acad::eOk) { pEntity->close(); return es; }

	AcGePoint3d gept22; es = pEntity->getPointAt(2, gept22); if (es != Acad::eOk) { pEntity->close(); return es; }
	AcGePoint3d gept23; es = pEntity->getPointAt(3, gept23); if (es != Acad::eOk) { pEntity->close(); return es; }

	int iNumVerPolygon2 = pEntity->numVerts();
	pEntity->close();

	// Open the polygons
	acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enPolygon1, RTPOINT, asDblArray(gept10), RTLE, RTSTR, L"OPEN", RTSTR, L"", NULL);
	acedCommand(RTSTR, L".PEDIT", RTLB, RTENAME, enPolygon2, RTPOINT, asDblArray(gept20), RTLE, RTSTR, L"OPEN", RTSTR, L"", NULL);

	// Find out if the second polygon has to be rotated or not. This is done to get the closing segment close to the closing segment of Polygon 1
	ads_name enPt;
	ads_point ptInters1;
	ads_point ptInters2;

	int iVert = 0;
	AcDbPolyline *pPolygon = new AcDbPolyline();

	if (acutDistance(asDblArray(gept20), asDblArray(gept10)) > acutDistance(asDblArray(gept21), asDblArray(gept10)))
	{
		ads_point ptRotate = { (gept20.x + gept22.x) / 2, (gept20.y + gept22.y) / 2, 0.0 };
		acedCommand(RTSTR, L".ROTATE", RTENAME, enPolygon2, RTSTR, L"", RTPOINT, ptRotate, RTREAL, 180.0, NULL);

		// Get the new vertices after rotation
		es = acdbGetObjectId(objId, enPolygon2);
		if (es != Acad::eOk) return es;

		es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
		if (es != Acad::eOk) return es;

		es = pEntity->getPointAt(0, gept20); if (es != Acad::eOk) { pEntity->close(); return es; }
		es = pEntity->getPointAt(1, gept21); if (es != Acad::eOk) { pEntity->close(); return es; }
		es = pEntity->getPointAt(2, gept22); if (es != Acad::eOk) { pEntity->close(); return es; }
		es = pEntity->getPointAt(3, gept23); if (es != Acad::eOk) { pEntity->close(); return es; }

		pEntity->close();

		int iRet = acdbInters(asDblArray(gept12), asDblArray(gept13), asDblArray(gept20), asDblArray(gept21), false, ptInters1);
		if (iRet != RTNORM) return Acad::eInvalidDxf3dPoint;

		iRet = acdbInters(asDblArray(gept10), asDblArray(gept11), asDblArray(gept22), asDblArray(gept23), false, ptInters2);
		if (iRet != RTNORM) return Acad::eInvalidDxf3dPoint;

		///////////////////////////////////////////////////////////////////////////////////////////
		// Draw a new resultant polygon with closing segment at the end of second polygon
		///////////////////////////////////////////////////////////////////////////////////////////

		// Add the vertex of the first polygon
		pPolygon->addVertexAt(iVert, AcGePoint2d(gept22.x, gept22.y)); iVert++;

		// Intersection of two polygons (one end)
		pPolygon->addVertexAt(iVert, AcGePoint2d(ptInters2[X], ptInters2[Y])); iVert++;

		// Add the vertex of the first polygon (skip the first and the last vertex as they will be replaced by intersections found)
		for (int i = 1; i < iNumVerPolygon1 - 1; i++) { pPolygon->addVertexAt(iVert, AcGePoint2d(geptVertices.at(i).x, geptVertices.at(i).y)); iVert++; }

		// Intersection of two polygons (other end)
		pPolygon->addVertexAt(iVert, AcGePoint2d(ptInters1[X], ptInters1[Y])); iVert++;

		// Add the last vertex of the second polygon
		pPolygon->addVertexAt(iVert, AcGePoint2d(gept21.x, gept21.y)); iVert++;
	}
	else
	{
		int iRet = acdbInters(asDblArray(gept10), asDblArray(gept11), asDblArray(gept20), asDblArray(gept21), false, ptInters1);
		if (iRet != RTNORM) return Acad::eInvalidDxf3dPoint;

		iRet = acdbInters(asDblArray(gept12), asDblArray(gept13), asDblArray(gept22), asDblArray(gept23), false, ptInters2);
		if (iRet != RTNORM) return Acad::eInvalidDxf3dPoint;

		//////////////////////////////////////////////////////////////////////////
		// Draw a new resultant polygon with closing segment at the end of second polygon
		//////////////////////////////////////////////////////////////////////////

		// Add the vertex of the first polygon
		pPolygon->addVertexAt(iVert, AcGePoint2d(gept22.x, gept22.y)); iVert++;

		// Intersection of two polygons (one end)
		pPolygon->addVertexAt(iVert, AcGePoint2d(ptInters2[X], ptInters2[Y])); iVert++;

		// Add the vertex of the first polygon (skip the first and the last vertex as they will be replaced by intersections found)
		for (int i = 1; i < iNumVerPolygon1 - 1; i++) { pPolygon->addVertexAt(iVert, AcGePoint2d(geptVertices.at(i).x, geptVertices.at(i).y)); iVert++; }

		// Intersection of two polygons (other end)
		pPolygon->addVertexAt(iVert, AcGePoint2d(ptInters1[X], ptInters1[Y])); iVert++;

		// Add the last vertex of the second polygon
		pPolygon->addVertexAt(iVert, AcGePoint2d(gept21.x, gept21.y)); iVert++;
	}

	// Append the new polygon to database
	pPolygon->setClosed(Adesk::kTrue);
	appendEntityToDatabase(pPolygon);

	objId = pPolygon->objectId();

	ads_name enPolygon; acdbGetAdsName(enPolygon, objId);
	pPolygon->close();

	// Erase the old polygons
	acdbEntDel(enPolygon1);
	acdbEntDel(enPolygon2);

	// The resultant polygon is the Polygon 1 from now on
	enPolygon1[0] = enPolygon[0];
	enPolygon1[1] = enPolygon[1];

	return Acad::eOk;
}

//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus ApplyStatus(ads_name enPolygon, int iState)
{
	CString csLayer = (!iState ? L"BASE_DUCT_PROP" : L"BASE_DUCT_EXIST");

	// Get their end points to determine the hatch angle
	AcDbPolyline *pPolygon;
	AcDbObjectId objId;
	Acad::ErrorStatus es;
	es = acdbGetObjectId(objId, enPolygon); if (es != Acad::eOk) return es;
	es = acdbOpenObject(pPolygon, objId, AcDb::kForWrite); if (es != Acad::eOk) return es;
	AcGePoint3d gePt1; pPolygon->getPointAt(0, gePt1);
	AcGePoint3d gePt2; pPolygon->getPointAt(pPolygon->numVerts() - 1, gePt2);
	pPolygon->setLayer(csLayer);
	pPolygon->setColor(g_ByLayerColor);
	pPolygon->close();

	// State is EXISTING, simply return
	if (iState) return Acad::eOk;

	//////////////////////////////////////////////////////////////////////////
	// Hatch the inside of the polygon (PROPOSED ONLY)
	//////////////////////////////////////////////////////////////////////////
	AcDbHatch *pHatch = new AcDbHatch();

	// Set hatch plane and pattern
	AcGeVector3d normal(0.0, 0.0, 1.0);
	pHatch->setNormal(normal);
	pHatch->setElevation(0.0);
	pHatch->setLayer(csLayer);
	pHatch->setPatternScale(8);
	pHatch->setPattern(AcDbHatch::kPreDefined, _T("ANSI31"));
	pHatch->setHatchStyle(AcDbHatch::kNormal);
	pHatch->setPatternAngle(acutAngle(asDblArray(gePt1), asDblArray(gePt2)));

	// Set Associativity
	pHatch->setAssociative(Adesk::kTrue);

	AcDbObjectIdArray aryObjIds;
	aryObjIds.append(objId);
	es = pHatch->appendLoop(AcDbHatch::kDefault, aryObjIds);
	if (es != Acad::eOk) { acutPrintf(_T("\nERROR: %s"), acadErrorStatusText(es)); delete pHatch; return es; }

	// Elaborate hatch lines
	pHatch->evaluateHatch();

	pHatch->setLayer(csLayer);
	pHatch->setColor(g_ByLayerColor);

	appendEntityToDatabase(pHatch);
	// m_aryObjIds.append(pHatch->objectId());

	// Attach hatchId to all source boundary objects for notification.
	AcDbEntity *pEnt;
	AcDbObjectId objHatchId = pHatch->objectId();

	int numObjs = aryObjIds.length();
	for (int i = 0; i < numObjs; i++) 
	{
		es = acdbOpenAcDbEntity(pEnt, aryObjIds[i], AcDb::kForWrite);
		if (es == Acad::eOk) 
		{
			pEnt->addPersistentReactor(pHatch->objectId());
			pHatch->close();
			pEnt->close();
		}
	}

	pHatch->close();
	return Acad::eOk;
}

//////////////////////////////////////////////////////////////////////////
// Function name: GetIndexAtPoint
// Description  : 
//////////////////////////////////////////////////////////////////////////
int GetIndexAtPoint(AcDbPolyline *pPolygon, AcGePoint3d geptOn)
{
	// Get the end points of linear polyline
	double dParam;
	int iSeg;
	for (iSeg = 0; iSeg < pPolygon->numVerts(); iSeg++)
	{
		if (Adesk::kTrue == pPolygon->onSegAt(iSeg, AcGePoint2d(geptOn.x, geptOn.y), dParam)) return iSeg; 
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////
// Function name: ShowNewPolygon
// Description  :
//////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus ShowNewPolygon(ads_name enPolygon, ads_point ptEnd)
{
	// Open the polygon and get the closest point
	Acad::ErrorStatus es;
	AcDbObjectId objId;
	AcDbPolyline *pPolygon;
	AcGePoint3d geptEnd1;

	es = acdbGetObjectId(objId, enPolygon); if (es != Acad::eOk) return es;
	es = acdbOpenObject(pPolygon, objId, AcDb::kForRead); if (es != Acad::eOk) return es;
	es = pPolygon->getClosestPointTo(AcGePoint3d(ptEnd[X], ptEnd[Y], 0.0), geptEnd1); if (es != Acad::eOk) { pPolygon->close(); return es; }

	// Determine which segment has the point 
	int iSeg1 = GetIndexAtPoint(pPolygon, geptEnd1);
	if (iSeg1 == -1) { pPolygon->close(); return Acad::eInvalidDxf3dPoint; }
	//displayMessage(L"Seg1 = %d", iSeg1);

	// Draw the new end segment for which the end point on the other edge has to be determined
	AcGePoint3d geVertex1; pPolygon->getPointAt(iSeg1,		 geVertex1);
	AcGePoint3d geVertex2; pPolygon->getPointAt(iSeg1 + 1, geVertex2);

	AcGePoint3d geVertex0; pPolygon->getPointAt(0, geVertex0);
	AcGePoint3d geVertexN; pPolygon->getPointAt(pPolygon->numVerts() - 1, geVertexN);

	double dOffset = acutDistance(asDblArray(geVertex0), asDblArray(geVertexN));
	double dAngle  = acutAngle(asDblArray(geVertex1),    asDblArray(geVertex2));

	// Calculate the point on the other side of the segment
	ads_point ptEnd2; acutPolar(asDblArray(geptEnd1), dAngle + PIby2, dOffset, ptEnd2);
	AcGePoint3d geptEnd2(ptEnd2[X], ptEnd2[Y], 0.0);
	int iSeg2 = GetIndexAtPoint(pPolygon, geptEnd2);
	if (iSeg2 == -1) 
	{
		// The new point on the other side
		acutPolar(asDblArray(geVertex1), dAngle - PIby2, dOffset, ptEnd2);

		geptEnd2.x = ptEnd2[X]; geptEnd2.y = ptEnd2[Y]; 
		iSeg2 = GetIndexAtPoint(pPolygon, geptEnd2);
		if (iSeg2 == -1) { pPolygon->close(); return Acad::eInvalidDxf3dPoint; }
	}
	// displayMessage(L"Seg2 = %d", iSeg2);

	// Close the polygon
	pPolygon->close();

	// Get the lesser index and the larger index in the ascending order
	int iSegMin;
	int iSegMax;
	AcGePoint3d geMin;
	AcGePoint3d geMax;

	if (iSeg1 < iSeg2)
	{
		iSegMin = iSeg1; geMin =  geptEnd1;
		iSegMax = iSeg2; geMax =  geptEnd2;
	}
	else
	{
		iSegMin = iSeg2; geMin =  geptEnd2;
		iSegMax = iSeg1; geMax =  geptEnd1;
	}

	// Draw the portion of the POLYGON that will be retained
	ads_name enNewPolygon; 
	bool bFlip = false;
	while (T)
	{
		int iVert = 0;
		AcDbPolyline *pNewPoly = new AcDbPolyline();
		AcGePoint3d geVertex;

		if (!bFlip)
		{
			// Starting vertex
			pNewPoly->addVertexAt(iVert++, AcGePoint2d(geMin.x, geMin.y));

			for (int iCtr = 0; iCtr < pPolygon->numVerts(); iCtr++)
			{
				if ((iCtr > iSegMin) && (iCtr <= iSegMax))
				{
					pPolygon->getPointAt(iCtr, geVertex);
					pNewPoly->addVertexAt(iVert++, AcGePoint2d(geVertex.x, geVertex.y));
				}
			}

			// Last vertex
			pNewPoly->addVertexAt(iVert++, AcGePoint2d(geMax.x, geMax.y));
		}
		else
		{
			// Starting vertex
			pNewPoly->addVertexAt(iVert++, AcGePoint2d(geMax.x, geMax.y));

			// Get all the vertices after the max vertex into the new polygon first
			for (int iCtr = 0; iCtr < pPolygon->numVerts(); iCtr++)
			{
				if (iCtr > iSegMax)
				{
					pPolygon->getPointAt(iCtr, geVertex);
					pNewPoly->addVertexAt(iVert++, AcGePoint2d(geVertex.x, geVertex.y));
				}
			}

			// Get all the leading vertices before the min vertex into the new polygon
			for (int iCtr = 0; iCtr < pPolygon->numVerts(); iCtr++)
			{
				if (iCtr <= iSegMin)
				{
					pPolygon->getPointAt(iCtr, geVertex);
					pNewPoly->addVertexAt(iVert++, AcGePoint2d(geVertex.x, geVertex.y));
				}
			}

			// Last vertex
			pNewPoly->addVertexAt(iVert++, AcGePoint2d(geMin.x, geMin.y));
		}

		pNewPoly->setColorIndex(1);
		pNewPoly->setClosed(Adesk::kTrue);
		appendEntityToDatabase(pNewPoly);
		AcDbObjectId objNewPoly = pNewPoly->objectId();
		acdbGetAdsName(enNewPolygon, objNewPoly);
		pNewPoly->close();

		// Draw the new polygon end line that is perpendicular to the side of the polygon from the selection point to the opposite side of the polygon.
		TCHAR result[10];
		acedInitGet(NULL, _T("Flip Accept"));
		int iRetTrim = acedGetKword(_T("\nConfirm selection [Flip/Accept] <Accept>: "), result);
		if ((iRetTrim == RTNONE) || ((iRetTrim == RTNORM) && (result[0] == 'A')))
		{
			// Delete the old polygon
			acdbEntDel(enPolygon);

			// Accept the location of the trim
			enPolygon[0] = enNewPolygon[0];
			enPolygon[1] = enNewPolygon[1];

			// Change the color to bylayer
			ChangeProperties(false, enPolygon, 254, NULL, NULL, 1);

			return Acad::eOk;
		}
		else if (iRetTrim == RTCAN)
		{
			acdbEntDel(enNewPolygon);
			return Acad::eInvalidDxf3dPoint;
		}
		else
		{
			// Flip the polygon drawn
			bFlip = !bFlip;
			acdbEntDel(enNewPolygon);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CheckPointOnEntity
// DEscription  : Checks if a point is on the entity
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CheckPointOnEntity(ads_name enEntity, AcGePoint3d gePoint)
{
	AcDbObjectId objId; acdbGetObjectId(objId, enEntity);
	if (!objId.isValid()) { return 0; }

	AcDbEntity *pEntity;
	if (acdbOpenObject(pEntity, objId, AcDb::kForWrite, false) != Acad::eOk) { return 0; }
	if (!pEntity->isKindOf(AcDbLine::desc()) && !pEntity->isKindOf(AcDbPolyline::desc()) && !pEntity->isKindOf(AcDb2dPolyline::desc()))
	{
		pEntity->erase(Adesk::kTrue);
		pEntity->close();

		return 0;
	}

	double dParam;
	AcDbCurve *pCurve = (AcDbCurve *)pEntity;

	if (!pCurve) { pEntity->erase(Adesk::kTrue); pEntity->close(); return 0; }

	Acad::ErrorStatus es = pCurve->getDistAtPoint(gePoint, dParam);
	if (es != Acad::eOk) 
	{
		acdbRegApp(L"EACAPTURE_DEL");
		struct resbuf *rbpErase = acutBuildList(AcDb::kDxfRegAppName, L"EACAPTURE_DEL", AcDb::kDxfXdInteger16, 1, NULL);
		pEntity->setXData(rbpErase);
		pEntity->close(); 
		return 0; 
	}
	pEntity->close();

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// Collect broken segments and erase them
//////////////////////////////////////////////////////////////////////////
void EraseSegments()
{
	ads_name ssErase; 
	ads_name enErase;

	struct resbuf *rbpFilt = acutBuildList(AcDb::kDxfRegAppName, L"EACAPTURE_DEL", NULL);
	if (acedSSGet(L"X", NULL, NULL, rbpFilt, ssErase) == RTNORM) { acedCommand(RTSTR, L".ERASE", RTPICKS, ssErase, RTSTR, L"", NULL); }

	acutRelRb(rbpFilt);
	acedSSFree(ssErase);
}

//////////////////////////////////////////////////////////////////////////
// Function name: Command_DuctCoverageLatest()
// Description  : 
//////////////////////////////////////////////////////////////////////////
// void Command_DuctCoverageLatest()
void Command_DuctCoverage()
{
	// Switch Off certain system variables
	switchOff();

	// Create the necessary layers
	createLayer(L"BASE_DUCT_EXIST", Adesk::kFalse, Adesk::kFalse);
	createLayer(L"BASE_DUCT_PROP", Adesk::kFalse, Adesk::kFalse);

	int iRet;
	AcDbObjectIdArray aryObjIds;

	// Read the default values from registry
	CString csOffset;
	CString csWidth;
	CString csState;
	ReadDWGToolsSettings(csOffset, csWidth, csState);

	double dOffset = _tstof(csOffset);
	double dWidth  = _tstof(csWidth);
	int iState     = _tstoi(csState);

	int iSegment = 0;
	int iSideTwo = 0;

	AcDbObjectId objId;
	Acad::ErrorStatus es;

	ads_name enCable1; 
	ads_point ptCable1;

	ads_point ptMark1;  
	ads_point ptMark2;  
	ads_name enSegment1;
	ads_name enSegment2;
	ads_name enPolygon1;
	ads_name enPolygon2;
	AcGePoint3d geptSeg1Start;
	AcGePoint3d geptSeg1End;
	AcGePoint3d geptSeg2Start;
	AcGePoint3d geptSeg2End;

	// Switch off certain variables
	switchOff();

	CString csLayer = L"BASE_DUCT_PROP";
	createLayer(_T("BASE_DUCT_PROP"),  Adesk::kFalse, Adesk::kFalse);

	while (T)
	{
		TCHAR result[10];
		acutPrintf(L"\nCurrent settings..[O = %.1f][W = %.1f][%s][SideTwo = %d][Segment = %d]", dOffset, dWidth, (!iState ? L"Proposed" : L"Existing"), iSideTwo, iSegment);
		acedInitGet(NULL, L"Offset Line Point End Width State Undo Accept");
		iRet = acedGetKword(L"\nSpecify [Offset/Line/Point/End/Width/State/Undo]<Accept>: ", result);

		// Default accepted
		// if ((iRet == RTNORM) && (result[0] == 'A')) iRet = RTNONE;

		/**/ if (iRet == RTCAN) 
		{
			// Save the coverage values to registry
			SetDWGToolsSettings(dOffset, dWidth, iState); 

			// Remove if only segments are drawn
			if (acdbEntGet(enSegment1))	acdbEntDel(enSegment1);
			return; 
		}
		else if (iRet == RTNORM)
		{
			if (result[0] == 'L')
			{
				while (T)
				{
					//////////////////////////////////////////////////////////////////////////
					// Allow selection of Line Arc Polyline Spline etc to be included
					//////////////////////////////////////////////////////////////////////////
					ads_name enSel;
					ads_point ptDummy;
					int iSelRet = acedEntSel(L"\nSelect object:", enSel, ptDummy);

					// Return to the prompts
					if ((iSelRet == RTCAN) ||(iSelRet == RTNONE)) break;

					// Check if a valid entity is selected
					AcDbEntity *pEntity;
					AcDbObjectId objId; acdbGetObjectId(objId, enSel);
					if (acdbOpenObject(pEntity, objId, AcDb::kForRead) != Acad::eOk) continue;
					if (!pEntity->isKindOf(AcDbLine::desc()) && !pEntity->isKindOf(AcDbPolyline::desc()) && !pEntity->isKindOf(AcDb2dPolyline::desc())) { pEntity->close(); continue; }
					pEntity->close();

					if (iSideTwo == 0)
					{
						// No line drawn or point placed for this segment
						ads_point ptSide; 
						int iRetSide = acedGetPoint(NULL, L"\nSelect side to offset: ", ptSide);

						// Return to the prompts
						/**/ if ((iRetSide == RTCAN) ||(iRetSide == RTNONE)) break;
						else if ((iRetSide != RTNONE) && (iRetSide != RTCAN))
						{
							// Draw the first segment of the coverage
							acedCommand(RTSTR, L".OFFSET", RTREAL, dOffset, RTLB, RTENAME, enSel, RTPOINT, ptDummy, RTLE, RTPOINT, ptSide, RTSTR, L"", NULL);
							ads_name enSegment; acdbEntLast(enSegment);

							AcDbObjectId objSegment; acdbGetObjectId(objSegment, enSegment);
							if (acdbOpenObject(pEntity, objSegment, AcDb::kForRead) != Acad::eOk) break;

							// Get the point close to the dummy point selected on the source entity
							AcGePoint3d geptClosest1;
							AcGePoint3d geptClosest2;
							AcGePoint3d geptSegStart;
							AcGePoint3d geptSegEnd;

							double dParam1;
							double dParam2;
							double dParam;
							Acad::ErrorStatus es;
							AcDbCurve *pCurve;

							if (pEntity->isKindOf(AcDbLine::desc()))
							{
								AcDbLine *pSegment = AcDbLine::cast(pEntity);
								es = pSegment->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0 ), geptClosest1);
								es = pSegment->getClosestPointTo(AcGePoint3d(ptSide[X], ptSide[Y], 0.0 ),   geptClosest2);

								pCurve = (AcDbCurve *) pSegment;
								pSegment->close();
							}
							else if (pEntity->isKindOf(AcDbPolyline::desc()))
							{
								AcDbPolyline *pSegment = AcDbPolyline::cast(pEntity);
								es = pSegment->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0 ), geptClosest1);
								es = pSegment->getClosestPointTo(AcGePoint3d(ptSide[X], ptSide[Y], 0.0 ),   geptClosest2);

								pCurve = (AcDbCurve *) pSegment;
								pSegment->close();
							}
							else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
							{
								// Collect the polyline vertices
								AcDb2dPolyline *pSegment = AcDb2dPolyline::cast(pEntity);

								es = pSegment->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0 ), geptClosest1);
								es = pSegment->getClosestPointTo(AcGePoint3d(ptSide[X], ptSide[Y], 0.0 ),   geptClosest2);

								pCurve = (AcDbCurve *) pSegment;
								pSegment->close();
							}

							// Get the distances on the entity b/w which the entity should remain
							bool bError = false;

							es = pCurve->getDistAtPoint(geptClosest1, dParam1);
							if (es != Acad::eOk) { acutPrintf(L"\n[Error @%d: %s. Try again!", __LINE__, acadErrorStatusText(es)); bError = true; }

							es = pCurve->getDistAtPoint(geptClosest2, dParam2);
							if (es != Acad::eOk) { acutPrintf(L"\n[Error @%d: %s. Try again!", __LINE__, acadErrorStatusText(es)); bError = true; }

							if (bError) { acdbEntDel(enSegment); continue; }
							// acutPrintf(L"\nDist 1 = [%.2f] @[%.2f,%.2f], Dist 2 = [%.2f] at [%.2f,%.2f]", dParam1, geptClosest1.x, geptClosest1.y, dParam2, geptClosest2.x, geptClosest2.y);

							pCurve->getStartPoint(geptSegStart);
							pCurve->getEndPoint(geptSegEnd);

							if (dParam1 > dParam2)
							{
								double dTemp = dParam2;
								dParam2 = dParam1;
								dParam1 = dTemp;
							}

							//////////////////////////////////////////////////////////////////////////
							// Reduce the length of the entity on both sides
							//////////////////////////////////////////////////////////////////////////
							acedCommand(RTSTR, L".LENGTHEN", RTLB, RTENAME, enSegment, RTPOINT, asDblArray(geptSegStart), RTLE, RTSTR, L"", NULL);
							struct resbuf rbSetVar; acedGetVar(L"PERIMETER", &rbSetVar);
							double dSrcLength = rbSetVar.resval.rreal;

							acedCommand(RTSTR, L".LENGTHEN", RTSTR, L"DE", RTREAL, -1 * dParam1, RTLB, RTENAME, enSegment, RTPOINT, asDblArray(geptSegStart), RTLE, RTSTR, L"", NULL);
							acdbEntLast(enSegment);
							acedCommand(RTSTR, L".LENGTHEN", RTSTR, L"DE", RTREAL, -1 * (dSrcLength - dParam2), RTLB, RTENAME, enSegment, RTPOINT, asDblArray(geptSegEnd), RTLE, RTSTR, L"", NULL);
							acdbEntLast(enSegment);

							enSegment1[0] = enSegment[0]; 
							enSegment1[1] = enSegment[1]; 

							// This is required to close the segments to make a polygon
							geptSeg1Start = geptClosest1;
							geptSeg1End   = geptClosest2;

							// Return to the prompts
							iSideTwo = 1;
							break;
						}
					}
					else if (iSideTwo == 1)
					{
						//////////////////////////////////////////////////////////////////////////
						// Get the distance b/w them
						//////////////////////////////////////////////////////////////////////////
						struct resbuf rbOsnap; rbOsnap.restype = RTPOINT;  
						acutPolar(asDblArray(geptSeg1Start), 0.0, 0.0, rbOsnap.resval.rpoint); acedSetVar(L"LASTPOINT", &rbOsnap);

						ads_point ptInters; 
						iRet = acedOsnap(ptDummy, L"PER", ptInters);
						if (iRet == RTCAN) { SetDWGToolsSettings(dOffset, dWidth, iState); return; }
						if (iRet != RTNORM) continue;

						// The width of the coverage would then be
						double dOffsetDst = acutDistance(asDblArray(geptSeg1Start), ptInters) + dOffset;

						// Offset Segment 1 to get the Segment 2
						OffsetSegment1ForSegment2(enSegment1, asDblArray(geptSeg1Start), dOffsetDst, ptDummy, enSegment2, geptSeg2Start, geptSeg2End);

						// Draw the polygon comprising the segments and its closing segments
						ads_name enPolygon;	DrawPolygon(enSegment1, enSegment2, enPolygon);

						// New polygons can now be drawn
						iSideTwo = 0;

						// If “Segment” is <0> set “Segment” to <1>
						/**/ if (iSegment == 0)
						{
							// This is the first segment
							iSegment = 1;

							// This polygon is the first one drawn
							enPolygon1[0] = enPolygon[0];
							enPolygon1[1] = enPolygon[1];

							break; 
						} 
						else 
						{
							// This is the second polygon drawn, so we can Extend/Trim this with the earlier polygon
							iSegment++; 

							// This polygon is the second one drawn
							enPolygon2[0] = enPolygon[0];
							enPolygon2[1] = enPolygon[1];

							// Merge the polygons
							MergePolygons(enPolygon1, enPolygon2);
							break;
						}
					}
					else if (iSideTwo == 2)
					{
						// Draw a polyline that is offset from the selected line but starting from the previously placed point in a direction toward 
						// where the line was selected.
						acedCommand(RTSTR, L".OFFSET", RTSTR, L"T", RTLB, RTENAME, enSel, RTPOINT, ptDummy, RTLE, RTPOINT, ptMark1, RTSTR, L"", NULL);
						ads_name enSegment; acdbEntLast(enSegment);

						// Get the length of the selected entity
						acedCommand(RTSTR, L".LENGTHEN", RTLB, RTENAME, enSel, RTPOINT, ptDummy, RTLE, RTSTR, L"", NULL);
						struct resbuf rbSetVar; acedGetVar(L"PERIMETER", &rbSetVar);
						double dSrcLength = rbSetVar.resval.rreal;

						AcDbObjectId objSegment; acdbGetObjectId(objSegment, enSegment);
						if (acdbOpenObject(pEntity, objSegment, AcDb::kForRead) != Acad::eOk) break;

						// Get the point close to the dummy point selected on the source entity
						AcGePoint3d geptClosest1;
						AcGePoint3d geptClosest2;
						AcGePoint3d geBreakStart;
						AcGePoint3d geBreakEnd;
						double dParam1;
						double dParam2;
						Acad::ErrorStatus es;
						if (pEntity->isKindOf(AcDbLine::desc()))
						{
							AcDbLine *pSegment = AcDbLine::cast(pEntity);
							es = pSegment->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0 ), geptClosest1);
							es = pSegment->getClosestPointTo(AcGePoint3d(ptMark1[X], ptMark1[Y], 0.0 ),   geptClosest2);

							pSegment->getParamAtPoint(geptClosest1, dParam1);
							pSegment->getParamAtPoint(geptClosest2, dParam2);

							pSegment->getPointAtParam(0.0,				geBreakStart);
							pSegment->getPointAtParam(dSrcLength, geBreakEnd);
							pSegment->close();
						}
						else if (pEntity->isKindOf(AcDbPolyline::desc()))
						{
							AcDbPolyline *pSegment = AcDbPolyline::cast(pEntity);

							es = pSegment->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0 ), geptClosest1);
							es = pSegment->getClosestPointTo(AcGePoint3d(ptMark1[X], ptMark1[Y], 0.0 ),   geptClosest2);

							pSegment->getParamAtPoint(geptClosest1, dParam1);
							pSegment->getParamAtPoint(geptClosest2, dParam2);

							es = pSegment->getPointAt(0, geBreakStart);
							es = pSegment->getPointAt(pSegment->numVerts() - 1, geBreakEnd);

							pSegment->close();
						}
						else if (pEntity->isKindOf(AcDbLine::desc()))
						{
							AcDb2dPolyline *pSegment = AcDb2dPolyline::cast(pEntity);

							es = pSegment->getClosestPointTo(AcGePoint3d(ptDummy[X], ptDummy[Y], 0.0 ), geptClosest1);
							es = pSegment->getClosestPointTo(AcGePoint3d(ptMark1[X], ptMark1[Y], 0.0 ),   geptClosest2);

							pSegment->getParamAtPoint(geptClosest1, dParam1);
							pSegment->getParamAtPoint(geptClosest2, dParam2);

							pSegment->getPointAtParam(0.0,				geBreakStart);
							pSegment->getPointAtParam(dSrcLength, geBreakEnd);
							pSegment->close();
						}

						// Swap the points if the points are selected in reverse direction
						if (dParam2 < dParam1)
						{
							AcGePoint3d geTemp = geptClosest2;
							geptClosest2 = geptClosest1;
							geptClosest1 = geTemp;
						}

						// Break this at point specified
						acedCommand(RTSTR, L".BREAK", RTLB, RTENAME, enSegment, RTPOINT, asDblArray(geBreakStart), RTLE, RTPOINT, asDblArray(geptClosest1), NULL);
						acedCommand(RTSTR, L".BREAK", RTLB, RTENAME, enSegment, RTPOINT, asDblArray(geptClosest2), RTLE, RTPOINT, asDblArray(geBreakEnd), NULL);
						acdbEntLast(enSegment1);

						// This is required to close the segments to make a polygon
						geptSeg1Start = geptClosest1;
						geptSeg1End   = geptClosest2;

						// Allow selection of side
						ads_point ptSide;
						int iRetOff = acedGetPoint(asDblArray(geptSeg1Start), L"\nSide to offset: ", ptSide);
						if ((iRetOff == RTNONE) || (iRetOff == RTCAN))
						{
							iSideTwo = 1;

							// Return to the options
							break;
						}
						else
						{
							// Offset Segment 1 to get the Segment 2
							OffsetSegment1ForSegment2(enSegment1, asDblArray(geptClosest1), dOffset, ptSide, enSegment2, geptSeg2Start, geptSeg2End);

							// Draw the polygon comprising the segments and its closing segments
							ads_name enPolygon;	DrawPolygon(enSegment1, enSegment2, enPolygon);

							// New polygons can now be drawn
							iSideTwo = 0;
							if (iSegment == 0)
							{
								// This is the first segment
								iSegment = 1;

								// This polygon is the first one drawn
								enPolygon1[0] = enPolygon[0];
								enPolygon1[1] = enPolygon[1];

								// Return to the options
								break;
							}
							else
							{
								// This is the second polygon drawn, so we can Extend/Trim this with the earlier polygon
								iSegment++;

								// This polygon is the second one drawn
								enPolygon2[0] = enPolygon[0];
								enPolygon2[1] = enPolygon[1];

								// Merge the polygons
								MergePolygons(enPolygon1, enPolygon2);

								// Return to the options
								break;
							}
						}
					}
				}
			}
			else if (result[0] == 'P')
			{
				// Allow to select a point
				acedInitGet(RSG_NONULL, L"");
				int iSelMark = acedGetPoint(((iSideTwo == 2) ? ptMark1 : NULL), L"\nPoint marker: ", ((iSideTwo == 2) ? ptMark2 : ptMark1));
				if (iSelMark != RTCAN) 
				{
					/**/ if (iSideTwo == 0) iSideTwo = 2; 
					else if (iSideTwo == 1) 
					{
						// The first segment has already been drawn and a second point has been placed. So copy the first polyline to the new point and close the ends
						// b/w the first and the second segment
						acedCommand(RTSTR, L".COPY", RTENAME, enSegment1, RTSTR, L"", RTPOINT, asDblArray(geptSeg1Start), RTPOINT, ptMark1, NULL);
						acdbEntLast(enSegment2);

						// Draw the polygon comprising the segments and its closing segments
						ads_name enPolygon;	DrawPolygon(enSegment1, enSegment2, enPolygon);

						// New polygons can now be drawn
						iSideTwo = 0;

						// If “Segment” is <0> set “Segment” to <1>
						/**/ if (iSegment == 0) 
						{
							iSegment++; 

							// This polygon is the first one drawn
							enPolygon1[0] = enPolygon[0];
							enPolygon1[1] = enPolygon[1];
						} 
						else 
						{
							// This is the second polygon drawn, so we can Extend/Trim this with the earlier polygon
							iSegment++; 

							// This polygon is the second one drawn
							enPolygon2[0] = enPolygon[0];
							enPolygon2[1] = enPolygon[1];

							// Merge the polygons
							MergePolygons(enPolygon1, enPolygon2);
						}
					}
					else if (iSideTwo == 2)
					{
						// A first point has been placed and a second point has now been placed
						AcDbPolyline *pPline = new AcDbPolyline(2);

						pPline->addVertexAt(0, AcGePoint2d(ptMark1[X], ptMark1[Y]));
						pPline->addVertexAt(1, AcGePoint2d(ptMark2[X], ptMark2[Y]));

						appendEntityToDatabase(pPline);
						objId = pPline->objectId();
						pPline->close();

						// Indicate first line has been drawn
						iSideTwo = 1;
						geptSeg1Start.x = ptMark1[X];
						geptSeg1Start.y = ptMark1[Y];
						acdbGetAdsName(enSegment1, objId);
					}
				}
			}
			else if (result[0] == 'E')
			{
				ads_point ptEnd;

				while (T)
				{
					acedInitGet(NULL, L"Accept");
					int iSelEnd = acedGetPoint(NULL, L"\nSelect end point on duct coverage or [Accept] <Accept>:", ptEnd);
					if ((iSelEnd == RTNONE) || (iSelEnd == RTKWORD) && (acedGetInput(result) == RTNORM))
					{
						// Exit the function
						if (!iSegment) { SetDWGToolsSettings(dOffset, dWidth, iState); return; }

						iSegment = 0;

						// Hatch with ANSI 131/ Scale 8 and angle = Angle of a line drawn between the midpoints of the two end lines of all the combined segments (NOT UNDERSTOOD!!)
						ApplyStatus(enPolygon1, iState);
						break;
					}
					else if (iSelEnd == RTNORM)
					{
						// End point of the coverage was selected. Check whether the end point of the coverage is on the polygon
						if ((es = ShowNewPolygon(enPolygon1, ptEnd)) == Acad::eOk) 
						{
							break;
						}
						else acutPrintf(L"\nError[%s]", acadErrorStatusText(es));
					}
				}
			}
			else if (result[0] == 'O')
			{
				double dInput;
				CString csPrompt;
				int iRetOfst;

				while (T)
				{
					acedInitGet(RSG_NOZERO | RSG_NONEG, L"");
					csPrompt.Format(L"\nSpecify offset distance <%.2f>: ", dOffset);
					iRetOfst = acedGetDist(NULL, csPrompt, &dInput);

					/**/ if (iRetOfst != RTCAN)
					{
						if (iRetOfst == RTNORM)
						{
							// If User enters a value within valid range update “Offset”, go to the prompts
							if ((dInput < 0.01) || (dInput > 2.0)) { acutPrintf(L"\nInvalid input! Specify value between 0.01 & 2.0!"); } else { dOffset = dInput; break; }
						}
						else if (iRetOfst == RTNONE) break;
					}
					else break;
				}
			}
			else if (result[0] == 'W')
			{
				double dInput;
				CString csPrompt;
				int iRetWidth;

				acedInitGet(RSG_NOZERO | RSG_NONEG, L"");
				csPrompt.Format(L"\nSpecify duct coverage width <%.1f>: ", dWidth);
				iRetWidth = acedGetDist(NULL, csPrompt, &dInput);

				/**/ if (iRetWidth != RTCAN)
				{
					if (iRetWidth == RTNORM) dWidth = dInput;

					// If User enters a value within valid range update “Width” 
					if ((dWidth < 0.1) || (dWidth > 10)) { acutPrintf(L"\nInvalid input! Specify value between 0.1 & 10."); }

					// If the specified width is valid
					if (iSideTwo == 0)
					{
						ads_point ptStart, ptEnd;
						int iRetStart = acedGetPoint(NULL, L"\nSelect start point:", ptStart);
						/**/ if ((iRetStart != RTCAN) && (iRetStart != RTNONE))
						{
							iSideTwo = 2;
							int iRetEnd = acedGetPoint(ptStart, L"\nSelect end point:", ptEnd);
							/**/ if ((iRetStart != RTCAN) && (iRetStart != RTNONE))
							{
								// This is segment 1 drawn
								acedCommand(RTSTR, L".LINE", RTPOINT, ptStart, RTPOINT, ptEnd, RTSTR, L"", NULL);

								acdbEntLast(enSegment1);
								geptSeg1Start = AcGePoint3d(ptStart[X], ptStart[Y], 0.0);
								geptSeg1End   = AcGePoint3d(ptEnd[X], ptEnd[Y], 0.0);

								iSideTwo = 1;

								// Get the side to offset segment 1 to draw the segment 2
								ads_point ptSide;
								int iRetSide = acedGetPoint(ptStart, L"\nPick side to offset: ", ptSide);
								/**/ if ((iRetStart != RTCAN) && (iRetStart != RTNONE))
								{
									// Draw the second segment of the coverage
									acedCommand(RTSTR, L".OFFSET", RTREAL, dWidth, RTLB, RTENAME, enSegment1, RTPOINT, asDblArray(geptSeg1Start), RTLE, RTPOINT, ptSide, RTSTR, L"", NULL);
									acdbEntLast(enSegment2);

									// Get the closest point on the new segment
									AcDbEntity *pEntity;
									acdbGetObjectId(objId, enSegment2);
									acdbOpenObject(pEntity, objId, AcDb::kForRead);

									if (pEntity->isKindOf(AcDbLine::desc()))
									{
										AcDbLine *pSegment = AcDbLine::cast(pEntity);

										pSegment->getStartPoint(geptSeg2Start);
										pSegment->getEndPoint(geptSeg2End);

										pSegment->close();
									}
									else if (pEntity->isKindOf(AcDbPolyline::desc()))
									{
										AcDbPolyline *pSegment = AcDbPolyline::cast(pEntity);

										pSegment->getPointAt(0, geptSeg2Start);
										pSegment->getPointAt(pSegment->numVerts() - 1, geptSeg2End);
										pSegment->close();
									}
									else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
									{
										AcDb2dPolyline *pSegment = AcDb2dPolyline::cast(pEntity);

										AcGePoint3dArray gePtsArray;
										AcGeDoubleArray geDblArray;
										collectVertices(pSegment, gePtsArray, geDblArray, true);
										pSegment->close();

										// Get the end points of the reference line
										geptSeg2Start = gePtsArray.at(0);
										geptSeg2End = gePtsArray.at(gePtsArray.length() - 1);
									}

									// Close the segments drawn to form a polygon
									//acedCommand(RTSTR, L".LINE", RTPOINT, asDblArray(geptSeg1Start), RTPOINT, asDblArray(geptSeg2Start), RTSTR, L"", NULL);
									//acedCommand(RTSTR, L".LINE", RTPOINT, asDblArray(geptSeg1End), RTPOINT, asDblArray(geptSeg2End), RTSTR, L"", NULL);

									// Draw the polygon comprising the segments and its closing segments
									ads_name enPolygon;	DrawPolygon(enSegment1, enSegment2, enPolygon);

									/*
									// Return to the prompts
									iSideTwo = 0;

									if (iSegment == 0) iSegment = 1;
									else if (iSegment > 0) { iSegment++; ApplyStatus(enPolygon, iState); }
									*/

									// If “Segment” is <0> set “Segment” to <1>
									/**/ if (iSegment == 0)
									{
										// This is the first segment
										iSegment = 1;

										// This polygon is the first one drawn
										enPolygon1[0] = enPolygon[0];
										enPolygon1[1] = enPolygon[1];
									} 
									else 
									{
										// This is the second polygon drawn, so we can Extend/Trim this with the earlier polygon
										iSegment++; 

										// This polygon is the second one drawn
										enPolygon2[0] = enPolygon[0];
										enPolygon2[1] = enPolygon[1];

										// Merge the polygons
										MergePolygons(enPolygon1, enPolygon2);
									}
								}
							}
						}
					}
				}
			}
			else if (result[0] == 'S')
			{
				int iRetState;
				TCHAR sStateRslt[17];
				acedInitGet(NULL, L"Proposed Existing");
				CString csPrompt; csPrompt.Format(L"\nSpecify state [Existing/Proposed] <%c>: ", (!iState ? 'P' : 'E'));
				iRetState = acedGetKword(csPrompt, sStateRslt);
				if (iRetState == RTNORM) {	if (sStateRslt[0] == 'P') iState = 0;	else iState = 1; }
			}
			else if (result[0] == 'U')
			{
				// Delete last entered point, line or segment.
				if (acdbEntGet(enSegment1))	acdbEntDel(enSegment1);

				// If “Side Two” is <0> Set “Segment” to “Segment” - 1
				if (iSideTwo == 0) iSegment -= 1;
				iSideTwo = 0;
			}
			else if (result[0] == 'A')
			{
				// Exit the function
				if (!iSegment) { SetDWGToolsSettings(dOffset, dWidth, iState); return; }

				iSegment = 0;

				// Hatch with ANSI 131/ Scale 8 and angle = Angle of a line drawn between the midpoints of the two end lines of all the combined segments (NOT UNDERSTOOD!!)
				ApplyStatus(enPolygon1, iState);
			}
		}
		else if (iRet == RTNONE)
		{
			// Exit the function
			if (!iSegment) { SetDWGToolsSettings(dOffset, dWidth, iState); return; }

			iSegment = 0;

			// Hatch with ANSI 131/ Scale 8 and angle = Angle of a line drawn between the midpoints of the two end lines of all the combined segments (NOT UNDERSTOOD!!)
			ApplyStatus(enPolygon1, iState);
		}
		else if (iRet == RTNORM) 
		{
			// Check if the entity selected is a valid contour
			acdbGetObjectId(objId, enCable1);

			AcDbEntity *pEntity;
			es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
			if (es != Acad::eOk) { acutPrintf(_T("\nError at %d: %s"), __LINE__, acadErrorStatusText(es)); SetDWGToolsSettings(dOffset, dWidth, iState); return; }

			if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc())) 
			{
				pEntity->close();
				break;
			}
			pEntity->close();

			// Not a valid contour
			acutPrintf(_T("\nNot a valid selection.\n"));
		}
	}
}













