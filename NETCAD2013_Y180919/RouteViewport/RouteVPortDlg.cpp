// RouteVPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RouteVPortDlg.h"
#include "RouteInfo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Externally defined
///////////////////////////////////////////////////////////////////////////////////////////////////////
extern void sortRouteInfo (std:: vector <CRouteInfo> &routeInfo);
extern int  getRouteCount (std:: vector <CRouteInfo> &routeInfo, int iSheetNo);

// Undocumented
void ads_regen();

//////////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Function name: GetClosestPoint()
// Description  : 
/////////////////////////////////////////////////////
bool GetClosestPoint(ads_name enObject, AcGePoint3d gePtPick, AcGePoint3d &gePtOnCurve)
{
	AcDbObjectId objCable;
	if (acdbGetObjectId(objCable, enObject) != Acad::eOk) return false;

	AcDbEntity *pEntity;
	Acad:ErrorStatus es = acdbOpenObject(pEntity, objCable, AcDb::kForRead);

	/**/ if (AcDbPolyline::cast(pEntity))
	{
		AcDbPolyline *pCable = AcDbPolyline::cast(pEntity);
		es = pCable->getClosestPointTo(gePtPick, gePtOnCurve);
		if (es != Acad::eOk) { pCable->close(); return false; }
		pCable->close();
	}
	else if (AcDb2dPolyline::cast(pEntity))
	{
		AcDb2dPolyline *pCable = AcDb2dPolyline::cast(pEntity);
		es = pCable->getClosestPointTo(gePtPick, gePtOnCurve);
		if (es != Acad::eOk) { pCable->close(); return false; }
		pCable->close();
	}
	else { pEntity->close(); return false; }

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: UndoSegment()
// Description  : Removes the last vertex of the given route.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void UndoSegment(AcDbObjectId objRoute, ads_point ptPrev)
{
	// Undo the last segment drawn
	AcDbPolyline *pLine;
	if (acdbOpenObject(pLine, objRoute, AcDb::kForWrite) != Acad::eOk) return;

	// Check if there is anything to UNDO
	int iNoOfVerts = pLine->numVerts();
	if (iNoOfVerts > 1)
	{
		pLine->removeVertexAt(iNoOfVerts - 1);

		// Set the last point of the remaining route so that the remaining plan may be drawn from it
		if (iNoOfVerts >= 2) 
		{
			AcGePoint2d gePt;
			pLine->getPointAt((iNoOfVerts - 2), gePt);
			acutPolar(asDblArray(gePt), 0.0, 0.0, ptPrev);
		}
	}

	pLine->close();
}

//////////////////////////////////////////////////////////////////////////
// Function name: 
// Description  :
//////////////////////////////////////////////////////////////////////////
bool GetCustomParameters(AcDbObjectId eId, CString csParameter, CString &csValue)
{
	// Get the object Id of the entity
	AcDbEntity* pEnt = NULL;
	Acad::ErrorStatus es = acdbOpenObject(pEnt, eId, AcDb::kForRead);
	if (es != Acad::eOk) { acutPrintf(L"\nError opening entity[%s].", acadErrorStatusText(es)); return false; }
	if (pEnt->isA() != AcDbBlockReference::desc()) { pEnt->close();	return false; }

	AcDbBlockReference *pBlkRef = AcDbBlockReference::cast(pEnt);
	AcDbHandle handle = eId.handle();

	// Initialize a AcDbDynBlockReference from the object id of the block reference
	AcDbDynBlockReference* pDynBlkRef = new AcDbDynBlockReference(pBlkRef->objectId());

	// Don't forget to close the block reference here, otherwise you wont be able to modify properties
	pEnt->close();

	if (pDynBlkRef)
	{
		if (pDynBlkRef->isDynamicBlock())
		{
			// Check if dynamic block
			AcDbDynBlockReferencePropertyArray blkPropAry;
			pDynBlkRef->getBlockProperties(blkPropAry);

			Acad::ErrorStatus err;
			AcDbDynBlockReferenceProperty blkProp;

			for (long lIndex1 = 0L ; lIndex1 < blkPropAry.length() ; ++lIndex1)
			{
				blkProp = blkPropAry[lIndex1];

				// Get allowed values for this property
				AcDbEvalVariantArray evalAry;
				if ((err = blkProp.getAllowedValues(evalAry)) != Acad::eOk) continue;

				// This property does not have values
				if (evalAry.length() == 0)
				{
					if (blkProp.propertyName().kACharPtr() == csParameter)
					{ 
						AcDbEvalVariant eval = blkProp.value();
						/**/ if (eval.restype == RTSTR) csValue.Format(L"%s", eval.resval.rstring);
						else if (eval.restype == 40)    csValue.Format(L"%s", suppressZero(eval.resval.rreal));

						delete pDynBlkRef;
						return true;
					}

					continue;
				}
			}
		}
		else 
		{
			// If the handle is same as the table skip it
			TCHAR *pszHandle = new TCHAR[17]; 
			//handle.getIntoAsciiBuffer(pszHandle);
			handle.getIntoAsciiBuffer(pszHandle,sizeof(pszHandle));
			CString csHndl; csHndl.Format(L"%s", pszHandle);
			acutPrintf(L"\nNot a dynamic block this [%s].", pszHandle);
		}

		// Don't forget to delete this reference, otherwise you will have problems.
		delete pDynBlkRef;
	}
	else acutPrintf(L"\nNULL dynamic block.");
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : ChangeAttributeValueAndRelocate
// Arguments        : 1. The parent entity, as object id
//                    2. The new value, as string
// Description      : 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ChangeCustomParameters(AcDbObjectId eId, CString csParameter, CString csValue);
void ChangeAttributeValueAndRelocate(AcDbObjectId objInsertId, CString csTag, CString csAttValue, CString csSheet, bool bRelocate)
{
	// Get the attributes attached to this insert
	AcDbBlockReference *pInsert;
	acdbOpenObject(pInsert, objInsertId, AcDb::kForRead);
	AcDbObjectIterator *pIter = pInsert->attributeIterator();
	AcGeScale3d acgeScale			= pInsert->scaleFactors();
	ads_point ptInsert				= { pInsert->position().x, pInsert->position().y, 0.0 };
	pInsert->close();

	// Get the attribute tag specified and change the value specified
	AcDbAttribute *pAtt;
	AcDbObjectId objAttId;

	CString csValue;

	CString csLabel; 
	csLabel.Format(L"%s LENGTH",     csSheet); GetCustomParameters(objInsertId, csLabel, csValue); double dLength = _tstof(csValue);
	csLabel.Format(L"%s ANGLE",		   csSheet); GetCustomParameters(objInsertId, csLabel, csValue); double dAngle  = _tstof(csValue);
	csLabel.Format(L"LOCATION %s X", csSheet); GetCustomParameters(objInsertId, csLabel, csValue); 

	ads_point ptLocation; acutPolar(ptInsert, dAngle, dLength / 2.0, ptLocation);
	
	csLabel.Format(L"LOCATION %s X", csSheet); ChangeCustomParameters(objInsertId, csLabel, suppressZero(ptLocation[X] - ptInsert[X]));
	csLabel.Format(L"LOCATION %s Y", csSheet); ChangeCustomParameters(objInsertId, csLabel, suppressZero(ptLocation[Y] - ptInsert[Y]));

	for (pIter->start(); !pIter->done(); pIter->step())
	{
		objAttId = pIter->objectId();
		acdbOpenObject(pAtt, objAttId, AcDb::kForWrite);

		if (!csTag.CompareNoCase(pAtt->tag())) 
		{
			// Set the new text
			pAtt->setTextString(csAttValue);	

			if (bRelocate)
			{
				pAtt->setPosition(AcGePoint3d(ptLocation[X], ptLocation[Y], 0.0));
				pAtt->setAlignmentPoint(AcGePoint3d(ptLocation[X], ptLocation[Y], 0.0));
			}
		}

		pAtt->close();
	}

	delete pIter;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: AttachRouteXData()
// Description  :
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus AttachRouteXData(AcDbObjectId objInsert, CString csSheet, CString csSheetNo, CString csScale)
{
	Acad::ErrorStatus es;

	// Attach ROUTE XDATA
	AcDbEntity *pEntity;
	es = acdbOpenObject(pEntity, objInsert, AcDb::kForWrite);
	if (es != Acad::eOk) return es;

	// Add XDATA to identity the details of this route
	acdbRegApp(_T("ROUTE"));
	struct resbuf *rbpXData = acutBuildList(AcDb::kDxfRegAppName, _T("ROUTE"), 
		AcDb::kDxfXdAsciiString, csSheet + L" Layout", 
		AcDb::kDxfXdAsciiString, csScale, AcDb::kDxfXdAsciiString, L"1:1", NULL);
	pEntity->setXData(rbpXData);
	acutRelRb(rbpXData);

	acdbRegApp(csSheet + L" Layout");
	rbpXData = acutBuildList(AcDb::kDxfRegAppName, csSheet + L" Layout", AcDb::kDxfXdInteger16, 1, NULL);
	es = pEntity->setXData(rbpXData);
	acutRelRb(rbpXData);

	pEntity->close();

	// Change the layer on which the attribute sits
	ChangeAttributeValueAndRelocate(objInsert, _T("SHEET_NUMBER"), csSheetNo, csSheet, true);
	return es;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetNextSheetNumber()
// Description  : Determines the sheet number to be assigned to the route symbol placed.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetNextSheetNumber(CString csSheet)
{
	// Collect all the "ROUTE"s and get the maximum number from it
	int iSheetNo = 1;
	ads_name ssGet;
	struct resbuf *rbpFilt = acutBuildList(AcDb::kDxfRegAppName, L"ROUTE", NULL);
	if (acedSSGet(L"X", NULL, NULL, rbpFilt, ssGet) != RTNORM) { acutRelRb(rbpFilt); acedSSFree(ssGet); return iSheetNo; }
	acutRelRb(rbpFilt);

	// Get the attribute value corresponding to the tag "SHEETNUMBER"
	//long lLength = 0L;
	int lLength = 0L;
	ads_name enGet;
	AcDbObjectId objId;
	AcDbBlockReference *pBlkRef; 
	AcDbObjectIterator *pAttIter;
	acedSSLength(ssGet, &lLength);
	CString csLabel; 
	
	for (long lCtr = 0L; lCtr < lLength; lCtr++)
	{
		acedSSName(ssGet, lCtr, enGet);
		acdbGetObjectId(objId, enGet);

		if (acdbOpenObject(pBlkRef, objId, AcDb::kForRead) != Acad::eOk) continue;
		pAttIter = pBlkRef->attributeIterator();
		pBlkRef->close();
		
		for (pAttIter->start(); !pAttIter->done(); pAttIter->step())
		{
			AcDbAttribute *pAtt;
			if (acdbOpenObject(pAtt, pAttIter->objectId(), AcDb::kForRead) != Acad::eOk) continue;
			if (!_tcsicmp(pAtt->tag(), L"SHEET_NUMBER")) 
			{
				if (iSheetNo < _tstoi(pAtt->textString())) iSheetNo = _tstoi(pAtt->textString());
			}
			pAtt->close();
		}

		delete pAttIter;
	}

	return iSheetNo;
}

//////////////////////////////////////////////////////////////////////////
// Function name: DrawRoutePolyline()
// Description  : Adds a new vertex to the current route entity.
//////////////////////////////////////////////////////////////////////////
void DrawRoutePolyline(AcDbObjectId objRoute, ads_point ptNext)
{
	AcDbPolyline *pLine;
	if (acdbOpenObject(pLine, objRoute, AcDb::kForWrite) != Acad::eOk) return;

	int iNoOfVerts = pLine->numVerts();
	struct resbuf *rbpXD = pLine->xData(L"ROUTE_PLAN");

	if (!rbpXD)
	{
		// Modify the end point of the first segment as it was set to start point itself at the start
		pLine->setPointAt(1, AcGePoint2d(ptNext[X], ptNext[Y]));

		// Set the XDATA
		rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"ROUTE_PLAN", AcDb::kDxfXdInteger16, 0, NULL);
		acdbRegApp(L"ROUTE_PLAN");
		pLine->setXData(rbpXD);
	}
	else
	{
		// Simply add the new vertex
		pLine->addVertexAt(iNoOfVerts, AcGePoint2d(ptNext[X], ptNext[Y]));
	}

	pLine->setColorIndex(2);
	pLine->setConstantWidth(0.5);

	acutRelRb(rbpXD);
	pLine->close();
}

//////////////////////////////////////////////////////////////////////////
// Function name: RouteAlongPline()
// Description  : 
//////////////////////////////////////////////////////////////////////////
bool RouteAlongPline(AcDbObjectId objPline, ads_point ptPick, AcDbObjectId objRoute, ads_point ptPrev)
{
	// Determine the points to draw the route along the polyline
	AcDbPolyline *pPline;
	if (acdbOpenObject(pPline, objPline, AcDb::kForWrite) != Acad::eOk) return false;
	int iNoOfVertices = pPline->numVerts(); 

	// Decide the start point of the route on the selected polyline
	// Note: This will be the vertex closer to the selected point on the polyline
	AcGePoint3d geOnCurve; 
	if (pPline->getClosestPointTo(AcGePoint3d(ptPick[X], ptPick[Y], 0.0), geOnCurve) != Acad::eOk) return false;

	double dParam;
	int iVertexID = -1;
	double dAng1;
	double dAng2;

	for (int iV = 0; iV < iNoOfVertices; iV++)
	{
		// Determine the segment of which the vertex is.
		if (pPline->onSegAt(iV, AcGePoint2d(geOnCurve.x, geOnCurve.y), dParam) != Adesk::kTrue) continue;

		// Get the segment on which the selected point lies
		AcGePoint2d geV1; pPline->getPointAt(iV,     geV1);
		AcGePoint2d geV2; pPline->getPointAt(iV + 1, geV2);

		// Get the vertex ID on this segment that is close to the selected point
		double dDist1 = acutDistance(asDblArray(geV1), asDblArray(geOnCurve));
		double dDist2 = acutDistance(asDblArray(geV2), asDblArray(geOnCurve));

		if (dDist1 <= dDist2) iVertexID = iV; else iVertexID = iV + 1;
		break;
	}

	// If the vertex ID is -1, this implies that the vertex was not found
	if (iVertexID == -1) { pPline->close(); return false; }

	// To the route, add all the segments from the nearest vertex to the end of the polyline
	AcGePoint2d gePt;
	AcGePoint2dArray ptrxVertices;
	AcGePoint2dArray ptrxVerticesReversed;

	// Get the vertex of the polyline from the pick point to the last vertex on the polyline path. 
	for (int iV = iVertexID; iV >= 0; iV--)	{	pPline->getPointAt(iV, gePt);	ptrxVerticesReversed.append(gePt);  	}
	for (int iV = iVertexID; iV < iNoOfVertices; iV++) { pPline->getPointAt(iV, gePt); ptrxVertices.append(gePt); }

	pPline->close();

	int iRet = RTERROR;
	ads_point ptNext;
	ads_point ptStore = { ptPrev[X], ptPrev[Y], 0.0 };
	bool bRedrawRoute = true;

	// Lock document and begin transaction
	acDocManager->lockDocument(curDoc());
	acTransactionManagerPtr()->startTransaction();

	TCHAR result[10];
	bool bReverse = false;
	int iNoOfVerticesRoute = 0;

	// Get the number pf vertices of the existing route
	if (acdbOpenObject(pPline, objRoute, AcDb::kForWrite) == Acad::eOk)
	{
		iNoOfVerticesRoute = pPline->numVerts();
		pPline->close();
	}

	while (T)
	{
		// Draw the route if requested
		if (bRedrawRoute)
		{
			// Draw the new route
			if (bReverse == false)
			{
				// From start to end
				for (int iPt = 0; iPt < ptrxVertices.logicalLength(); iPt++)
				{
					acutPolar(asDblArray(ptrxVertices.at(iPt)), 0.0, 0.0, ptPrev);
					DrawRoutePolyline(objRoute, ptPrev);
				}
			}
			else
			{
				// From end to start
				for (int iPt = 0; iPt < ptrxVerticesReversed.logicalLength(); iPt++)
				{
					acutPolar(asDblArray(ptrxVerticesReversed.at(iPt)), 0.0, 0.0, ptPrev);
					DrawRoutePolyline(objRoute, ptPrev);
				}
			}
		}

		// Display the user options
		acedInitGet(NULL, L"Reverse Trim Delete");
		iRet = acedGetPoint(ptPrev, L"\nSpecify next point or [Reverse/Trim/Delete]: ", ptNext);

		if (iRet == RTNORM)
		{
			bRedrawRoute = false;
			DrawRoutePolyline(objRoute, ptNext);
			acutPolar(ptNext, 0.0, 0.0, ptPrev);
		}
		else if (iRet == RTNONE)
		{
			acDocManager->unlockDocument(curDoc());
			acTransactionManagerPtr()->endTransaction();
			return true;
		}
		else if (iRet == RTCAN)
		{
			acDocManager->unlockDocument(curDoc());
			acTransactionManagerPtr()->abortTransaction();
			acutPolar(ptStore, 0.0, 0.0, ptPrev);
			return true;
		}
		else if ((iRet == RTKWORD) && (acedGetInput(result) == RTNORM))
		{
			// Delete (Remove the route over this PLINE and the connecting points too)
			if (result[0] == 'D') 
			{
				acDocManager->unlockDocument(curDoc());
				acTransactionManagerPtr()->abortTransaction();
				acutPolar(ptStore, 0.0, 0.0, ptPrev);
				return true;
			}
			else if (result[0] == 'T') 
			{
				ads_point ptTrim;
				if (acedGetPoint(ptPrev, L"\nSpecify point to trim to: ", ptTrim) == RTNORM) 
				{
					// Determine the closest point on the route and also determined the segment on which the closest point is.
					Acad::ErrorStatus es;
					int iVertexID = -1;
					double dParam;
					int iNV = 0;

					if (acdbOpenObject(pPline, objRoute, AcDb::kForWrite) == Acad::eOk)
					{
						es = pPline->getClosestPointTo(AcGePoint3d(ptTrim[X], ptTrim[Y], 0.0), geOnCurve);
						iNV = pPline->numVerts();
						for (int iV = 0; iV < pPline->numVerts(); iV++)
						{
							if (pPline->onSegAt(iV, AcGePoint2d(geOnCurve.x, geOnCurve.y), dParam) != Adesk::kTrue) continue;
							iVertexID = iV;
							break;
						}

						// Remove the vertex following the trim point
						while (pPline->numVerts() > iVertexID + 1) pPline->removeVertexAt(pPline->numVerts() - 1);
						pPline->addVertexAt(iVertexID + 1, AcGePoint2d(geOnCurve.x, geOnCurve.y));
						acutPolar(asDblArray(geOnCurve), 0.0, 0.0, ptPrev);
					}

					pPline->close();
				}
			}
			else if (result[0] == 'R') 
			{
				bReverse = !bReverse;

				// This will recall the code to redraw the polyline in the reverse direction
				bRedrawRoute = true;

				// Remove all the vertex drawn during this session
				if (acdbOpenObject(pPline, objRoute, AcDb::kForWrite) == Acad::eOk)
				{
					// Remove all other vertex added during this session 
					while (pPline->numVerts() > iNoOfVerticesRoute) pPline->removeVertexAt(pPline->numVerts() - 1);

					// Get the vertex at the start of route on this polyline
					AcGePoint3d gePt; pPline->getPointAt(iNoOfVerticesRoute - 1, gePt);
					acutPolar(asDblArray(gePt), 0.0, 0.0, ptPrev);
					pPline->close();
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: SelectEntity()
// Description  : Enables selection of entities insertion points and vertices are used to define the route.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SelectEntity(AcDbObjectId objRoute, ads_point ptPrev)
{
	int iRet;
	ads_name enSelect;
	ads_point ptDummy;
	ads_point ptNext;
	AcDbPolyline *pLine;
	bool bAccept = true;

	while (T)
	{
		iRet = acedEntSel(L"\nSelect object: ", enSelect, ptDummy);

		/**/ if (iRet == RTCAN) return false;
		/**/ if (iRet == RTERROR) return false;
		else if (iRet != RTNORM) continue; 

		// Open the entity to check the entity type
		AcDbObjectId objSelect; acdbGetObjectId(objSelect, enSelect);
		AcDbEntity *pEntity;
		if (acdbOpenObject(pEntity, objSelect, AcDb::kForRead) != Acad::eOk) continue;

		// If the selected entity is a block reference
		if (pEntity->isKindOf(AcDbBlockReference::desc()))
		{
			AcDbBlockReference *pBlkRef = AcDbBlockReference::cast(pEntity);
			acutPolar(asDblArray(pBlkRef->position()), 0.0, 0.0, ptNext);
			pBlkRef->close();

			// Draw the route
			DrawRoutePolyline(objRoute, ptNext);

			// Elastic line point
			acutPolar(ptNext, 0.0, 0.0, ptPrev);
		}
		if (pEntity->isKindOf(AcDbLine::desc()))
		{
			// Add the two end points of the selected line to the route. 
			AcDbLine *pLine = AcDbLine::cast(pEntity);
			ads_point ptStart; acutPolar(asDblArray(pLine->startPoint()), 0.0, 0.0, ptStart);
			ads_point ptEnd;   acutPolar(asDblArray(pLine->endPoint()),   0.0, 0.0, ptEnd);
			pLine->close();

			// First point will be the end point closest to the pick point. Second point will be the other end point.
			if (acutDistance(ptDummy, ptStart) <= acutDistance(ptDummy, ptEnd))
			{
				DrawRoutePolyline(objRoute, ptStart);
				DrawRoutePolyline(objRoute, ptEnd);

				// Elastic line point
				acutPolar(ptEnd, 0.0, 0.0, ptPrev);
			}
			else
			{
				DrawRoutePolyline(objRoute, ptEnd);
				DrawRoutePolyline(objRoute, ptStart);

				// Elastic line point
				acutPolar(ptStart, 0.0, 0.0, ptPrev);
			}
		}
		else if (pEntity->isKindOf(AcDbPolyline::desc()))
		{
			pEntity->close();

			// Call the function to route along the selected polyline
			if (RouteAlongPline(objSelect, ptDummy, objRoute, ptPrev)) return true;
		}
		else { pEntity->close(); continue; }
	}
}

////////////////////////////////////////////////////////////////////////
// Function name: ReverseRoute()
// Description  : The vertex points of the given polyline is reversed.
////////////////////////////////////////////////////////////////////////
void ReverseRoute(AcDbObjectId objRoute, ads_point ptPrev)
{
	// Reverse the route plan end points
	AcDbPolyline *pLine;
	if (acdbOpenObject(pLine, objRoute, AcDb::kForWrite) != Acad::eOk) return;

	AcGePoint2d gePt; pLine->getPointAt(0, gePt);
	acutPolar(asDblArray(gePt), 0.0, 0.0, ptPrev);
	pLine->reverseCurve();
	pLine->close();

	// Set this point to the center of the view
	acedCommandS(RTSTR, L".ZOOM", RTSTR, L"C", RTPOINT, ptPrev, RTSTR, L"", NULL);
}

void getUcsToWcsMatrix(AcGeMatrix3d& m, AcDbDatabase* db)
{
	ASSERT (db != NULL);
	if (!acdbUcsMatrix(m, db)) { m.setToIdentity();	ASSERT(0);	}
}

AcGeVector3d ucsToWcs(AcGeVector3d& vec)
{
	AcDbDatabase* db = acdbHostApplicationServices()->workingDatabase();
	ASSERT(db != NULL);
	AcGeMatrix3d m;

	getUcsToWcsMatrix(m, acdbHostApplicationServices()->workingDatabase());

	AcGeVector3d newv = vec;
	newv.transformBy(m);

	return newv;
}

AcGePoint3d ucsToWcs(const AcGePoint3d& pt)
{
	AcDbDatabase* db = acdbHostApplicationServices()->workingDatabase();
	ASSERT(db != NULL);
	AcGeMatrix3d m;

	getUcsToWcsMatrix(m, db);
	return m * pt;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool makeUcs(CString symName, AcGePoint3d &geO, AcGePoint3d &geX, AcGePoint3d &geY, AcDbObjectId &objRecId)
{
	// Get the UCS table pointer
	AcDbUCSTable *pUcsTbl;
	acdbHostApplicationServices()->workingDatabase()->getUCSTable(pUcsTbl, AcDb::kForWrite);

	// If the UCS name is already there delete it.
	Acad::ErrorStatus es;
	AcDbUCSTableRecord* pUcsTblRec;
	if (pUcsTbl->has(symName))
	{
		pUcsTbl->getAt(symName, pUcsTblRec, AcDb::kForWrite);
		es = pUcsTblRec->erase();
		if (es != Acad::eOk) { acutPrintf(_T("\nError %d: %s...\n"), __LINE__, acadErrorStatusText(es)); pUcsTblRec->close(); pUcsTbl->close(); return false; }
		pUcsTblRec->close();
	}

	AcDbUCSTableRecord* pNewUcsTblRec = new AcDbUCSTableRecord();
	pNewUcsTblRec->setName(symName);

	AcGePoint3d originPt = ucsToWcs(geO);
	pNewUcsTblRec->setOrigin(originPt);

	AcGeVector3d xAxis = ucsToWcs(geX) - originPt;
	AcGeVector3d yAxis = ucsToWcs(geY) - originPt;

	AcGeVector3d zAxis = xAxis.crossProduct(yAxis);
	yAxis = zAxis.crossProduct(xAxis);

	pNewUcsTblRec->setXAxis(xAxis.normalize());
	pNewUcsTblRec->setYAxis(yAxis.normalize());

	objRecId = AcDbObjectId::kNull;
	es = pUcsTbl->add(objRecId, pNewUcsTblRec);
	if (es != Acad::eOk) { delete pNewUcsTblRec; pUcsTbl->close(); return false; }

	pNewUcsTblRec->close();
	pUcsTbl->close();

	return true;
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
bool ChangeVisibility(AcDbObjectId eId, CString csVisibilityState)
{
	// Get the object Id of the entity
	AcDbEntity* pEnt = NULL;
	Acad::ErrorStatus es = acdbOpenObject(pEnt, eId, AcDb::kForRead);
	if (es != Acad::eOk) { acutPrintf(L"\nError opening entity[%s].", acadErrorStatusText(es)); return false; }
	if (pEnt->isA() != AcDbBlockReference::desc()) { pEnt->close();	return false; }

	AcDbBlockReference *pBlkRef = AcDbBlockReference::cast(pEnt);
	AcDbHandle handle = eId.handle();

	// Initialize a AcDbDynBlockReference from the object id of the block reference
	AcDbDynBlockReference* pDynBlkRef = new AcDbDynBlockReference(pBlkRef->objectId());

	// Don't forget to close the block reference here, otherwise you wont be able to modify properties
	pEnt->close();

	if (pDynBlkRef)
	{
		if (pDynBlkRef->isDynamicBlock())
		{
			if (!csVisibilityState.CompareNoCase(L"Default")) pDynBlkRef->resetBlock(); 
			else
			{
				// Check if dynamic block
				AcDbDynBlockReferencePropertyArray blkPropAry;
				pDynBlkRef->getBlockProperties(blkPropAry);

				Acad::ErrorStatus err;
				AcDbDynBlockReferenceProperty blkProp;

				for (long lIndex1 = 0L ; lIndex1 < blkPropAry.length() ; ++lIndex1)
				{
					blkProp = blkPropAry[lIndex1];

					// Look for the relevant property
					if (wcscmp(blkProp.propertyName().kACharPtr(), L"SHEET SIZE") != 0) continue;

					// Get allowed values for property
					AcDbEvalVariantArray evalAry;
					if ((err = blkProp.getAllowedValues(evalAry)) == Acad::eOk )
					{
						for (int iVis = 0; iVis < evalAry.length(); iVis++)
						{
							AcDbEvalVariant eval = evalAry[iVis];
							CString csExistState; csExistState.Format(L"%s", eval.resval.rstring);

							if (csExistState.CompareNoCase(csVisibilityState)) continue; 

							if (!blkProp.readOnly())
							{
								if ((err = blkProp.setValue(eval)) != Acad::eOk) { acutPrintf(L"...Error setting property value..."); }
								break;
							}
						}
					}
				}
			}
		}
		else 
		{
			// If the handle is same as the table skip it
			TCHAR *pszHandle = new TCHAR[17]; 
			//handle.getIntoAsciiBuffer(pszHandle);
			handle.getIntoAsciiBuffer(pszHandle,sizeof(pszHandle));
			CString csHndl; csHndl.Format(L"%s", pszHandle);
		}

		// Don't forget to delete this reference, otherwise you will have problems.
		delete pDynBlkRef;
	}
	else acutPrintf(L"\nNULL dynamic block.");

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Function name: 
// Description  :
//////////////////////////////////////////////////////////////////////////
bool ChangeCustomParameters(AcDbObjectId eId, CString csParameter, CString csValue)
{
	// Get the object Id of the entity
	AcDbEntity* pEnt = NULL;
	Acad::ErrorStatus es = acdbOpenObject(pEnt, eId, AcDb::kForRead);
	if (es != Acad::eOk) { acutPrintf(L"\nError opening entity[%s].", acadErrorStatusText(es)); return false; }
	if (pEnt->isA() != AcDbBlockReference::desc()) { pEnt->close();	return false; }

	AcDbBlockReference *pBlkRef = AcDbBlockReference::cast(pEnt);
	AcDbHandle handle = eId.handle();

	// Initialize a AcDbDynBlockReference from the object id of the block reference
	AcDbDynBlockReference* pDynBlkRef = new AcDbDynBlockReference(pBlkRef->objectId());

	// Don't forget to close the block reference here, otherwise you wont be able to modify properties
	pEnt->close();

	if (pDynBlkRef)
	{
		if (pDynBlkRef->isDynamicBlock())
		{
			// Check if dynamic block
			AcDbDynBlockReferencePropertyArray blkPropAry;
			pDynBlkRef->getBlockProperties(blkPropAry);

			Acad::ErrorStatus err;
			AcDbDynBlockReferenceProperty blkProp;

			for (long lIndex1 = 0L ; lIndex1 < blkPropAry.length() ; ++lIndex1)
			{
				blkProp = blkPropAry[lIndex1];

				// Get allowed values for this property
				AcDbEvalVariantArray evalAry;
				if ((err = blkProp.getAllowedValues(evalAry)) != Acad::eOk) continue;

				// This property does not have values
				if (evalAry.length() == 0)
				{
					if (blkProp.propertyName().kACharPtr() == csParameter)
					{
						AcDbEvalVariant eval = blkProp.value();
						/**/ if (eval.restype == RTSTR)
							_tccpy(eval.resval.rstring, csValue);
						else if (eval.restype == 40)
							eval.resval.rreal  = _tstof(csValue);

						Acad::ErrorStatus es = blkProp.setValue(eval);
					}

					continue;
				}

				for (int iVis = 0; iVis < evalAry.length(); iVis++)
				{
					AcDbEvalVariant eval = evalAry[iVis];

					if (eval.restype == RTSTR)
					{
						CString csExistState; csExistState.Format(L"%s", eval.resval.rstring);
					}
				}
			}
		}
		else 
		{
			// If the handle is same as the table skip it
			TCHAR *pszHandle = new TCHAR[17]; 
			//handle.getIntoAsciiBuffer(pszHandle);
			handle.getIntoAsciiBuffer(pszHandle,sizeof(pszHandle));
			CString csHndl; csHndl.Format(L"%s", pszHandle);
			acutPrintf(L"\nNot a dynamic block this [%s].", pszHandle);
		}

		// Don't forget to delete this reference, otherwise you will have problems.
		delete pDynBlkRef;
	}
	else acutPrintf(L"\nNULL dynamic block.");
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Function name: getViewPortsInLayout()
// Arguments		: [IN]  The layout name
//                [OUT] The object id's of the viewports in the given layout.
// Description  :	Retrieves the VIEWPORTS in the given layout
//////////////////////////////////////////////////////////////////////////
void getViewPortsInLayout(CString csLayoutName, AcDbObjectIdArray &objVPortIDArray)
{
	// Get the layout manager pointer
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();

	// Get the layout name from the vector and get its block table record object id
	//Commented for ACAD 2018
	//AcDbLayout *pLayout = pLayoutMngr->findLayoutNamed(csLayoutName, true);
	AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(csLayoutName);
	AcDbLayout *pLayout = NULL;
	acdbOpenObject(pLayout, objLayoutId, AcDb::kForRead);
	AcDbObjectId objBlkTblRcdId = pLayout->getBlockTableRecordId();
	pLayout->close();

	// Open the block table record
	AcDbBlockTableRecord *pBlockTblRcd;
	Acad::ErrorStatus es = acdbOpenObject(pBlockTblRcd, objBlkTblRcdId, AcDb::kForRead);

	// Initialize a new block table record iterator
	AcDbBlockTableRecordIterator *pBlkTblRcdItr;
	pBlockTblRcd->newIterator(pBlkTblRcdItr);
	pBlockTblRcd->close();

	// For each entity in the database, check if it is an ATTDEF object
	AcDbEntity *pEnt;
	for (pBlkTblRcdItr->start(); !pBlkTblRcdItr->done(); pBlkTblRcdItr->step())
	{
		// Get the next entity in the drawing database. The check is in case the database is corrupted
		if (pBlkTblRcdItr->getEntity(pEnt, AcDb::kForRead) != Acad::eOk) { delete pBlkTblRcdItr; return; }

		// Cast the entity pointer as an AcDbBlockReference pointer. 
		if (AcDbViewport::cast(pEnt)) objVPortIDArray.append(pEnt->objectId());
		pEnt->close();
	}

	// Delete the block table record iterator
	delete pBlkTblRcdItr;
}

/////////////////////////////////////////////////////
// Function name: CreateSummarySheet
// Description  :
/////////////////////////////////////////////////////
void CreateSummarySheet(CString csLayout, CString csVPortScale, AcGePoint3d geVPortCen, double dWidth, double dHeight, ads_point ptMin, ads_point ptMax) 
{
	CString csSummarySheetName = L"Sheet 1";
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
	AcApLayoutManager *pLayoutMngrAcAp = (AcApLayoutManager *)acdbHostApplicationServices()->layoutManager();

	// Create the layout (if not present). If present delete this layout!
	AcDbLayout *pLayout;
	Acad::ErrorStatus es;
	//Commented for ACAD 2018
	//if (pLayout = pLayoutMngr->findLayoutNamed(csSummarySheetName)) pLayoutMngr->deleteLayout(csSummarySheetName); 
	if (AcDbObjectId objLytId = pLayoutMngr->findLayoutNamed(csSummarySheetName)) pLayoutMngr->deleteLayout(csSummarySheetName);

	// Copy the new layout from the reference layout selected while creating route!
	es = pLayoutMngr->copyLayout(csLayout, csSummarySheetName);
	if (es != Acad::eOk) { return; }

	// Delete all the existing viewport from this layout
	//Commented for ACAD 2018
	//pLayout = pLayoutMngr->findLayoutNamed(csSummarySheetName);
	//if (pLayout == NULL) return;
	AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(csSummarySheetName);
	if (!objLayoutId.isValid()) return;
	
	// Delete all the existing viewport from this layout
	AcDbObjectIdArray objectIds;
	es = pLayoutMngr->setCurrentLayout(csSummarySheetName);
	// Set the tab order of the layout
	//Commented for ACAD 2018
	//if (acdbOpenObject(pLayout, pLayout->objectId(), AcDb::kForWrite) == Acad::eOk){ pLayout->setTabOrder(pLayoutMngr->countLayouts() - 1); }
	if (acdbOpenObject(pLayout, objLayoutId, AcDb::kForWrite) == Acad::eOk) { pLayout->setTabOrder(pLayoutMngr->countLayouts() - 1); }
	
	pLayoutMngrAcAp->updateLayoutTabs();	
	objectIds = pLayout->getViewportArray();
	deleteArray(objectIds);
	objectIds.removeAll();

	pLayout->close();
	
	// Create a UCS
	ads_point ptOrigin = { (ptMin[X] + ptMax[X]) / 2.0, (ptMin[Y] + ptMax[Y]) / 2.0, 0.0 };
	AcDbObjectId objIdUcs;

	if (!makeUcs(csSummarySheetName, AcGePoint3d(ptOrigin[X], ptOrigin[Y], 0.0), AcGePoint3d(ptOrigin[X] + 1000.00, ptOrigin[Y], 0.0), AcGePoint3d(ptOrigin[X], ptOrigin[Y] + 1000.0, 0.0), objIdUcs)) { acutPrintf(_T("\nFailed to create ") + csSummarySheetName); return; }

	// Sheet corners b/w which the VPORTS are to be created
	ads_point ptFirst  = { geVPortCen.x - (dWidth / 2), geVPortCen.y - (dHeight / 2), geVPortCen.z };
	ads_point ptSecond = { geVPortCen.x + (dWidth / 2), geVPortCen.y + (dHeight / 2), geVPortCen.z };

	// Switch to paper space
	acdbHostApplicationServices()->workingDatabase()->setTilemode(0);

	// Create a viewport using acedCommand/MVIEW
	saveOSMode();
	acedCommandS(RTSTR, _T(".MVIEW"), RTPOINT, ptFirst, RTPOINT, ptSecond, NULL);
	restoreOSMode();

	ads_name enMView; acdbEntLast(enMView); 
	AcDbObjectId objMViewId; acdbGetObjectId(objMViewId, enMView); // objVPIds.append(objMViewId);

	// Use acedCommand/zoom-extents to ensure that the view was on the screen
	acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("E"), NULL);

	// Make sure that the viewport was turned on using acedCommand MVIEW/ON last. 
	acedCommandS(RTSTR, _T(".MVIEW"), RTSTR, _T("ON"), RTENAME, enMView, RTSTR, _T(""), NULL);

	// Activate the viewport created
	acedMspace();

	// Get the viewport number
	AcDbViewport *pVPort;
	es = acdbOpenObject(pVPort, objMViewId, AcDb::kForRead);
	if (es != Acad::eOk) { AfxMessageBox(_T("Can't open viewport.")); return; }

	// Set CVPORT to the number of your viewport to ensure that your viewport is active
	int iVPN = pVPort->number();
	pVPort->close();

	struct resbuf rbSetVar; rbSetVar.resval.rint = iVPN; acedSetVar(_T("CVPORT"), &rbSetVar);

	// Location of the next viewport on this sheet
	rbSetVar.resval.rint = iVPN; acedSetVar(_T("CVPORT"), &rbSetVar);

	// Go to MSPACE and set the zoom and scale
	acedCommandS(RTSTR, _T(".MSPACE"), NULL);
	acedCommandS(RTSTR, _T(".UCS"), RTSTR, _T("NA"), RTSTR, _T("R"), RTSTR, csSummarySheetName, NULL);
	acedCommandS(RTSTR, _T(".PLAN"), RTSTR, _T("U"), RTSTR, csSummarySheetName, NULL);
	acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("E"), NULL);
	acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("C"), RTSTR, _T("0,0"), RTSTR, _T(""), NULL);
	acedCommandS(RTSTR, _T(".PSPACE"), NULL);
	acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("E"), NULL);

	// struct resbuf *rbpXD = acutBuildList(AcDb::kDxfRegAppName, _T("VIEWSCALE"), AcDb::kDxfXdAsciiString, _T("SCALE_") + csViewScale.Mid(csViewScale.Find(_T(":")) + 1), NULL);
	if (acdbOpenObject(pVPort, objMViewId, AcDb::kForWrite) == Acad::eOk)
	{
		//double dScale = 1000.0 / _tstof(csVPortScale.Mid(csVPortScale.Find(_T(":")) + 1));
		//pVPort->setCustomScale(dScale);
		pVPort->setCustomScale(1.0);

		pVPort->setUcs(objIdUcs);
		pVPort->setUcsPerViewport(true);
		pVPort->setLocked();
		pVPort->updateDisplay();

		// Add the XDATA to help place the scale bars
		//pVPort->setXData(rbpXD); acutRelRb(rbpXD);

		es = acedSetCurrentVPort(pVPort);
		pVPort->close();
	}
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
void CreateSheets(CString csSheet, CString csViewScale, CString csVPortScale, CStringArray &csaSheetNos)
{
	CString csLayout; csLayout.Format(L"%s Layout", csSheet);

	// Zoom BIG
	acedCommandS(RTSTR, L".ZOOM", RTSTR, L"E", NULL);

	// Check if layout is present in the drawing
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
	AcApLayoutManager *pLayoutMngrAcAp = (AcApLayoutManager *)acdbHostApplicationServices()->layoutManager();

	//Commented for ACAD 2018
	//AcDbLayout *pLayout = pLayoutMngr->findLayoutNamed(csLayout);
	AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(csLayout);
	AcDbLayout *pLayout = NULL;
	acdbOpenObject(pLayout, objLayoutId, AcDb::kForRead);
	if (!pLayout) { acutPrintf(_T("\nLayout \"" + csLayout +  "\" not found. Create this sheet and try again!"));	return; }
	pLayoutMngr->setCurrentLayout(csLayout);

	// Get the view ports on this sheet
	AcDbObjectIdArray geObjIdArray = pLayout->getViewportArray();
	pLayout->close();

	// If there are more than one viewport, then message the user
	if (geObjIdArray.length() == 0) { acutPrintf(_T("\nThere are no viewports in ") + csLayout + _T(".")); return; }

	// Set the UCS to WORLD
	acedCommandS(RTSTR, _T(".UCS"), RTSTR, _T("W"), NULL);

	AcDbObjectId objIdVPort;
	AcDbViewport *pViewPort;
	AcGePoint3d geVPortCen;
	double dHeight; 
	double dWidth;
	Acad::ErrorStatus es;

	for (int iCtr = geObjIdArray.length() - 1; iCtr >= 0; iCtr--)
	{
		// Get the viewport object id
		objIdVPort = geObjIdArray.at(iCtr);

		// Get the location and the size of the viewport
		es = acdbOpenObject(pViewPort, objIdVPort, AcDb::kForRead);
		if (es != Acad::eOk) continue;

		geVPortCen = pViewPort->centerPoint();
		dHeight = pViewPort->height();
		dWidth  = pViewPort->width();

		pViewPort->close();
		break;
	}

	// Collect all the blocks with name "ROUTE_PLAN_VIEW"
	ads_name ssGet;
	struct resbuf *rbpFilt = acutBuildList(RTDXF0, _T("INSERT"), AcDb::kDxfRegAppName, csLayout, 67, 0, NULL);
	int iRet = acedSSGet(_T("X"), NULL, NULL, rbpFilt, ssGet);
	//long lLength = 0L; acedSSLength(ssGet, &lLength);
	int lLength = 0L; acedSSLength(ssGet, &lLength);
	if ((iRet != RTNORM) || (lLength == 0L))
	{
		acutPrintf(_T("\nThere are no valid routes defined in the drawing. Exiting command!\n"));
		acutRelRb(rbpFilt); acedSSFree(ssGet); return; 
	}
	acutRelRb(rbpFilt);

	// Get the route information and assign them to the route object
	ads_name enInsert;
	AcDbObjectId objId;
	AcDbBlockReference *pInsert;
	AcDbObjectIterator *pIter;
	ads_point ptInsert;
	AcDbAttribute *pAtt;
	AcDbObjectId objAttId;
	CString csTag;
	CString csSheetNo;

	// Height remaining in this sheet to place a viewport
	double dRemainingHt = dHeight;

	std::vector <CRouteInfo> routeinfo_Vector;
	routeinfo_Vector.clear();
	int iAsciiValue  = 64;
	int iPrevSheetNo = 0; 

	CString csLabel; 

	for (long lCtr = lLength - 1L; lCtr >= 0L; lCtr--)
	{
		// Get the entity name
		acedSSName(ssGet, lCtr, enInsert);

		// Get the object id
		acdbGetObjectId(objId, enInsert);

		// Open the block reference and get its attribute value, insertion point, geom extents and rotation angle
		es = acdbOpenObject(pInsert, objId, AcDb::kForRead);
		if (es != Acad::eOk) { acutPrintf(L"\nERROR %d: %s", __LINE__, acadErrorStatusText(es)); continue; }

		ptInsert[X] = pInsert->position().x; ptInsert[Y] = pInsert->position().y; ptInsert[Z] = 0.0;
		pIter = pInsert->attributeIterator();
		pInsert->close();

		// Get the sheet number from the attribute
		csSheetNo.Empty();
		
		csLabel.Format(L"SHEET_NUMBER", csSheet);
		for (pIter->start(); !pIter->done(); pIter->step())
		{
			objAttId = pIter->objectId();
			acdbOpenObject(pAtt, objAttId, AcDb::kForWrite);
			csTag.Format(_T("%s"), pAtt->tag());

			if (!csTag.CompareNoCase(_T("SHEET_NUMBER"))) 
			{
				csSheetNo.Format(_T("%s"), pAtt->textString()); 
				pAtt->close(); 
				break; 
			}

			pAtt->close();
		}

		delete pIter;

		// If the sheet number is empty, ignore it
		if (csSheetNo.IsEmpty()) continue;

		// If the sheet number is not to be processed in this command, ignore it
		CString csValue;
		if (GetParameterValue(csaSheetNos, csSheetNo, csValue, 0) == -1) continue;

		// Get them all into RouteInfo object
		CRouteInfo routeInfo;

		routeInfo.m_objId     = objId;
		routeInfo.m_iSheetNo  = _ttoi(csSheetNo);
		routeInfo.m_dWidth    = dWidth; // Width of the parent viewport

		// Get the ANGLE set in the VPORT
		csLabel.Format(L"%s ANGLE", csSheet);
		if (GetCustomParameters(objId, csLabel, csValue)) routeInfo.m_dRotation = _tstof(csValue);

		// See if this viewport can be accommodated in this sheet
		GetParameterValue(csaSheetNos, csSheetNo, csValue, 0);
		int iNoOfSheets = _tstoi(csValue);
		routeInfo.m_dHeight   = (dHeight / iNoOfSheets); 

		if (_ttoi(csSheetNo) != iPrevSheetNo)
		{
			routeInfo.m_bIsNewSheet = true;
			iAsciiValue							= 64;
			routeInfo.m_iAsciiNo    = 64;
			dRemainingHt						= dHeight;
		}
		else
			routeInfo.m_bIsNewSheet = false;

		// Insertion point of what
		// acutPolar(ptInsert, 0.0, 0.0, routeInfo.m_ptInsert);
		double dWidth = 0.0;
		csLabel.Format(L"%s LENGTH", csSheet);
		// if (GetCustomParameters(objId, L"A4 LENGTH", csValue)) dWidth = _tstof(csValue);
		if (GetCustomParameters(objId, csLabel, csValue)) dWidth = _tstof(csValue);
		acutPolar(ptInsert, routeInfo.m_dRotation, dWidth / 2, routeInfo.m_ptInsert);
		iPrevSheetNo = routeInfo.m_iSheetNo;
		routeinfo_Vector.push_back(routeInfo);
	}

	// Sort the information in the vector in ascending sheet number
	if (routeinfo_Vector.size()) sortRouteInfo(routeinfo_Vector);

	// Get the geometric extents of the routes collection
	ads_point ptMin = { 1000000000.0, 1000000000.0, 0.0 };
	ads_point ptMax = { -1000000000.0, -1000000000.0, 0.0 };

	AcDbBlockReference *pBlkRef;
	AcDbExtents extents;
	for (int iR = 0; iR < routeinfo_Vector.size(); iR++)
	{
		if (acdbOpenObject(pBlkRef, routeinfo_Vector.at(iR).m_objId, AcDb::kForRead) != Acad::eOk) continue;
		// if (pBlkRef->geomExtentsBestFit(extents) != Acad::eOk) { pBlkRef->close(); continue; }
		AcGePoint3d ptrx = pBlkRef->position();
		pBlkRef->close();

		if (ptrx.x < ptMin[X]) ptMin[X] = ptrx.x;
		if (ptrx.y < ptMin[Y]) ptMin[Y] = ptrx.y;

		if (ptrx.x > ptMax[X]) ptMax[X] = ptrx.x;
		if (ptrx.y > ptMax[Y]) ptMax[Y] = ptrx.y;
	}

	//////////////////////////////////////////////////////////////////////////
	// Create UCS to modify the views in the VPORTS
	//////////////////////////////////////////////////////////////////////////
	CString csSheetName;
	CString ucsName;
	CRouteInfo routeInfo;
	ads_point ptYAxis;
	ads_point ptXAxis;
	CString csAscii;

	for (int iCtr = 0; iCtr < routeinfo_Vector.size(); iCtr++)
	{
		routeInfo = routeinfo_Vector[iCtr];

		// Place/Replace the first sheet
		if (iCtr == 0) { CreateSummarySheet(csLayout, csVPortScale, geVPortCen, dWidth, dHeight, ptMin, ptMax); }
						
		if (routeInfo.m_iAsciiNo > 64)
		{
			// Form the sheet name
			routeinfo_Vector[iCtr].m_csSheetName.Format(_T("Sheet %d%c"), routeInfo.m_iSheetNo, char(routeInfo.m_iAsciiNo));

			// Form the UCS name
			routeinfo_Vector[iCtr].m_csUcsName.Format(_T("Sheet %d%c-%d"), routeInfo.m_iSheetNo, char(routeInfo.m_iAsciiNo), routeInfo.m_iVPN);
		}
		else
		{
			// Form the sheet name
			routeinfo_Vector[iCtr].m_csSheetName.Format(_T("Sheet %d"), routeInfo.m_iSheetNo);

			// Form the UCS name
			routeinfo_Vector[iCtr].m_csUcsName.Format(_T("Sheet %d-%d"), routeInfo.m_iSheetNo, routeInfo.m_iVPN);
		}

		// Create the layout (if not present). If present delete this layout
		//Commented for ACAD 2018
		//if (pLayout = pLayoutMngr->findLayoutNamed(routeinfo_Vector[iCtr].m_csSheetName)) {	pLayoutMngr->deleteLayout(routeinfo_Vector[iCtr].m_csSheetName); }
		if (AcDbObjectId objLytId = pLayoutMngr->findLayoutNamed(routeinfo_Vector[iCtr].m_csSheetName)) { pLayoutMngr->deleteLayout(routeinfo_Vector[iCtr].m_csSheetName); }

		// Copy the new layout from the reference layout selected while creating route
		es = pLayoutMngr->copyLayout(csLayout, routeinfo_Vector[iCtr].m_csSheetName);
		if (es != Acad::eOk) { continue; }

		// Delete all the existing viewport from this layout
		//Commented for ACAD 2018
		//if (pLayout = pLayoutMngr->findLayoutNamed(routeinfo_Vector[iCtr].m_csSheetName), true)
		AcDbObjectId objLayoutId;
		if (objLayoutId = pLayoutMngr->findLayoutNamed(routeinfo_Vector[iCtr].m_csSheetName))
		{
			//Commented for ACAD 2018
			//acdbOpenObject(pLayout, objLayoutId, AcDb::kForRead);
			AcDbObjectIdArray objectIds;
			AcApLayoutManager *pLayoutMan = (AcApLayoutManager*)acdbHostApplicationServices()->layoutManager(); 
			es = pLayoutMngr->setCurrentLayout(routeinfo_Vector[iCtr].m_csSheetName);
			//Commented for ACAD 2018
			//routeinfo_Vector[iCtr].m_objLayoutID = pLayout->objectId();
			//pLayout->close();
			routeinfo_Vector[iCtr].m_objLayoutID = objLayoutId;

			// Set the tab order of the layout
			if (acdbOpenObject(pLayout, routeinfo_Vector[iCtr].m_objLayoutID, AcDb::kForWrite) == Acad::eOk)
			{
				pLayout->setTabOrder(pLayoutMngr->countLayouts() - 1);
			}

			pLayoutMngrAcAp->updateLayoutTabs();
			objectIds = pLayout->getViewportArray();
			deleteArray(objectIds);
			objectIds.removeAll();

			pLayout->close();
		}

		// Create the UCS for this view
		if ((routeInfo.m_dRotation >= PIby2) && (routeInfo.m_dRotation <= (3 * PIby2)))
		{
			// To avoid upside down of contents in paper space viewport
			routeInfo.m_dRotation = routeInfo.m_dRotation + PI;
		}

		acutPolar(routeInfo.m_ptInsert, routeInfo.m_dRotation,				 100.0, ptXAxis);
		acutPolar(routeInfo.m_ptInsert, routeInfo.m_dRotation + PIby2, 100.0, ptYAxis);

		if (!makeUcs(routeinfo_Vector[iCtr].m_csUcsName, AcGePoint3d(routeInfo.m_ptInsert[X], routeInfo.m_ptInsert[Y], 0.0), AcGePoint3d(ptXAxis[X], ptXAxis[Y], ptXAxis[Z]), AcGePoint3d(ptYAxis[X], ptYAxis[Y], ptYAxis[Z]), routeinfo_Vector[iCtr].m_objIdUcs))
			acutPrintf(_T("\nFailed to create ") + routeinfo_Vector[iCtr].m_csSheetName);
	}

	// Sheet corners b/w which the VPORTS are to be created
	ads_point ptFirst  = { geVPortCen.x - (dWidth / 2), geVPortCen.y - (dHeight / 2), geVPortCen.z };
	ads_point ptSecond = { geVPortCen.x + (dWidth / 2), geVPortCen.y + (dHeight / 2), geVPortCen.z };

	// Create the required number of VPORTS on the sheets and set its view.
	AcDbObjectId layoutTblId;
	AcDbObjectId layoutId;
	AcDbObjectIdArray objVPortIds;
	CString csVPScale;

	ads_point ptFCorner, ptSCorner;

	AcDbObjectIdArray objVPIds;
	for (int iCtr = 0; iCtr < routeinfo_Vector.size(); )
	{
		// Get the number of VPORTS to be drawn in this layout
		int iNoOfVports = getRouteCount(routeinfo_Vector, routeinfo_Vector[iCtr].m_iSheetNo);

		double dCumulativeHt = 0.0;
		ads_name enMView;
		AcDbObjectId objMViewId;
		AcDbViewport *pVPort;

		// Set the required viewport current
		struct resbuf rbSetVar; rbSetVar.restype = RTSHORT; 

		acutPolar(ptFirst,  0.0, 0.0, ptFCorner);
		acutPolar(ptSecond, 0.0, 0.0, ptSCorner);

		// Register the VIEWSCALE XDATA
		acdbRegApp(_T("VIEWSCALE"));

		for (int iMView = 0; iMView < iNoOfVports; iMView++, iCtr++)
		{
			routeInfo = routeinfo_Vector[iCtr];

			if (routeinfo_Vector[iCtr].m_bIsNewSheet)
			{
				acutPolar(ptFirst,  0.0, 0.0, ptFCorner);
				acutPolar(ptSecond, 0.0, 0.0, ptSCorner);
			}

			// Set the layout as current
			es = pLayoutMngr->setCurrentLayout(routeInfo.m_csSheetName);
			pLayoutMngrAcAp->updateLayoutTabs();

			acutPrintf(_T("\rSetting sheet: %s..."), routeInfo.m_csSheetName);

			// Create a view in paper space
			ptSCorner[Y] = ptFCorner[Y] + routeInfo.m_dHeight;

			// Switch to paper space
			acdbHostApplicationServices()->workingDatabase()->setTilemode(0);

			// Create a viewport using acedCommand/MVIEW
			saveOSMode();
			acedCommandS(RTSTR, _T(".MVIEW"), RTPOINT, ptFCorner, RTPOINT, ptSCorner, NULL);
			restoreOSMode();

			ads_name enMView; acdbEntLast(enMView); acdbGetObjectId(objMViewId, enMView); 
			objVPIds.append(objMViewId);

			// Use acedCommand/zoom-extents to ensure that the view was on the screen
			acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("E"), NULL);

			// Make sure that the viewport was turned on using acedCommand MVIEW/ON last. 
			acedCommandS(RTSTR, _T(".MVIEW"), RTSTR, _T("ON"), RTENAME, enMView, RTSTR, _T(""), NULL);

			// Activate the viewport created
			acedMspace();

			// Get the viewport number
			es = acdbOpenObject(pVPort, objMViewId, AcDb::kForRead);
			if (es != Acad::eOk) { AfxMessageBox(_T("Can't open viewport.")); continue; }

			// Set CVPORT to the number of your viewport to ensure that your viewport is active
			int iVPN = pVPort->number();
			pVPort->close();

			rbSetVar.resval.rint = iVPN;
			acedSetVar(_T("CVPORT"), &rbSetVar);

			// Location of the next viewport on this sheet
			ptFCorner[Y] += routeInfo.m_dHeight;
			rbSetVar.resval.rint = iVPN; acedSetVar(_T("CVPORT"), &rbSetVar);

			// Go to MSPACE and set the zoom and scale
			acedCommandS(RTSTR, _T(".MSPACE"), NULL);
			acedCommandS(RTSTR, _T(".UCS"), RTSTR, _T("NA"), RTSTR, _T("R"), RTSTR, routeInfo.m_csUcsName, NULL);
			acedCommandS(RTSTR, _T(".PLAN"), RTSTR, _T("U"), RTSTR, routeInfo.m_csUcsName, NULL);
			acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("E"), NULL);
			acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("C"), RTSTR, _T("0,0"), RTSTR, _T(""), NULL);
			acedCommandS(RTSTR, _T(".PSPACE"), NULL);
			acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("E"), NULL);

			struct resbuf *rbpXD = acutBuildList(AcDb::kDxfRegAppName, _T("VIEWSCALE"), AcDb::kDxfXdAsciiString, _T("SCALE_") + csViewScale.Mid(csViewScale.Find(_T(":")) + 1), NULL);
			if (acdbOpenObject(pVPort, objMViewId, AcDb::kForWrite) == Acad::eOk)
			{
				double dScale = 1000.0 / _tstof(csVPortScale.Mid(csVPortScale.Find(_T(":")) + 1));
				pVPort->setCustomScale(dScale);
				pVPort->setUcs(routeInfo.m_objIdUcs);
				pVPort->setUcsPerViewport(true);
				pVPort->setLocked();
				pVPort->updateDisplay();

				// Add the XDATA to help place the scale bars
				pVPort->setXData(rbpXD);
				acutRelRb(rbpXD);

				es = acedSetCurrentVPort(pVPort);
				pVPort->close();
			}
		}
	}
} // csSheet

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetVPortDims()
// Description  : Get the width and height of the viewport in the layout selected.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
extern void getViewPortsInLayout(CString csLayoutName, AcDbObjectIdArray &objVPortIDArray);
bool GetVPortDims(CString csSheetSze, double &dVPortHeight, double &dVPortWidth)
{
	csSheetSze += L" Layout";

	// Get the view ports on the layout selected
	AcDbObjectIdArray geObjIdArray; getViewPortsInLayout(csSheetSze, geObjIdArray);

	// If there are more than one viewport, then message the user
	/**/ if (geObjIdArray.length() == 0) { acutPrintf(_T("\nThere are no viewports in ") + csSheetSze + _T(".")); return false; }
	// else if (geObjIdArray.length() > 1)  { acutPrintf(_T("\nThere seems to be more than one view port in the layout \"Sheet 1\".")); return; }

	AcDbObjectId objIdVPort;
	AcDbViewport *pViewPort;

	for (int iCtr = geObjIdArray.length() - 1; iCtr >= 0; iCtr--)
	{
		// Get the viewport object id
		objIdVPort = geObjIdArray.at(iCtr);

		// Get the location and the size of the viewport
		acdbOpenObject(pViewPort, objIdVPort, AcDb::kForRead);

		dVPortHeight = pViewPort->height();
		dVPortWidth  = pViewPort->width();

		pViewPort->close();
		break;
	}

	return true;
}

////////////////////////////////////////////////////////////////
// CRouteVPortDlg dialog
////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CRouteVPortDlg, CDialog)

CRouteVPortDlg::CRouteVPortDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRouteVPortDlg::IDD, pParent)
{
	m_iCalledFor = 0;

	m_csLength = L"0.0";
	m_csWidth = L"0.0";
	m_csOverlap = L"0.0";
	
	m_csSheet = L"";
	m_csScale = L"";
	m_csSelectedText = L"No object selected";

	m_dVPortLength = 0.0;
	m_dVPortWidth = 0.0;
}

CRouteVPortDlg::~CRouteVPortDlg()
{
}

void CRouteVPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RV_PICKLENGTH, m_btnPickLength);
	DDX_Control(pDX, IDC_RV_PICKWIDTH, m_btnPickWidth);
	DDX_Control(pDX, IDC_RV_PICKOVERLAP, m_btnPickOverlap);
	DDX_Text(pDX, IDC_STATIC_OBJSEL, m_csSelectedText);
	DDX_Text(pDX, IDC_RV_LENGTH, m_csLength);
	DDX_Text(pDX, IDC_RV_WIDTH, m_csWidth);
	DDX_Text(pDX, IDC_RV_OVERLAP, m_csOverlap);
	DDX_Control(pDX, IDC_RV_SHEETSIZE, m_cbSheetSize);
	DDX_CBString(pDX, IDC_RV_SHEETSIZE, m_csSheet);
	DDX_Control(pDX, IDC_RV_SCALE, m_cbScale);
	DDX_CBString(pDX, IDC_RV_SCALE, m_csScale);
	DDX_Control(pDX, IDC_EDITROUTE, m_btnEditRoute);
	DDX_Control(pDX, IDC_STATIC_IMAGE, m_btnInfo);
}

BEGIN_MESSAGE_MAP(CRouteVPortDlg, CDialog)
	ON_BN_CLICKED(IDC_RV_PICKLENGTH, &CRouteVPortDlg::OnBnClickedPickLength)
	ON_BN_CLICKED(IDC_RV_PICKWIDTH, &CRouteVPortDlg::OnBnClickedPickWidth)
	ON_BN_CLICKED(IDC_RV_PICKOVERLAP, &CRouteVPortDlg::OnBnClickedPickOverlap)
	ON_CBN_SELCHANGE(IDC_RV_SHEETSIZE, &CRouteVPortDlg::OnCbnSelchangeSheetSize)
	ON_BN_CLICKED(IDC_NEWROUTE, &CRouteVPortDlg::OnBnClickedNewRoute)
	ON_BN_CLICKED(IDC_EDITROUTE, &CRouteVPortDlg::OnBnClickedEditRoute)
	ON_CBN_SELCHANGE(IDC_RV_SCALE, &CRouteVPortDlg::OnCbnSelchangeScale)
	ON_BN_CLICKED(IDHELP, &CRouteVPortDlg::OnBnClickedHelp)
END_MESSAGE_MAP()

// CRouteVPortDlg message handlers
BOOL CRouteVPortDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the button bitmaps
	m_btnPickLength.SetSkin(IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, 1, 1, 1); 
	m_btnPickWidth.SetSkin(IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, 1, 1, 1);
	m_btnPickOverlap.SetSkin(IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, IDB_SELECT, 1, 1, 1);

	// Set the information ICON
	HICON hIcon = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_INFO), IMAGE_ICON, 16, 16, LR_SHARED);
	m_btnInfo.SetIcon(hIcon);

	// Populate layout names
	PopulateLayoutNames();

	// Sheet
	if (!m_csSheet.IsEmpty()) m_cbSheetSize.SelectString(-1, m_csSheet); else m_cbSheetSize.SetCurSel(0);

	// Scale
	if (!m_csScale.IsEmpty()) m_cbScale.SelectString(-1, m_csScale); else m_cbScale.SetCurSel(0);

	// Set the possible length and width in MSPACE
	if (m_csLength.IsEmpty()) m_csLength = L"200";
	if (m_csWidth.IsEmpty()) m_csLength = L"50";
	if (m_csOverlap.IsEmpty()) m_csOverlap = L"10";
	if ((_tstof(m_csLength) <= 0.0) && (_tstof(m_csWidth) <= 0.0)) OnCbnSelchangeSheetSize();

	// If the route is valid enable the "Edit route" button
	if (m_objRoute.isValid()) m_btnEditRoute.EnableWindow(TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
// Function name: CRouteVPortDlg::PopulateLayoutNames()
// Description  : Retrieves the layout names in the drawing.
//                Model space and names without "Layout" in it are ignored.
//////////////////////////////////////////////////////////////////////////
void CRouteVPortDlg::PopulateLayoutNames()
{
	// Get the block table of the drawing
	AcDbBlockTable *pBT; 
	Acad::ErrorStatus es;
	es = acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBT, AcDb::kForRead);
	if (es != Acad::eOk) return;

	// Create a new iterator
	AcDbBlockTableIterator* pIter; pBT->newIterator(pIter);

	// Close the block table
	pBT->close();

	// Loop through the iterator
	AcDbObjectId layoutId;
	for (pIter->start(); !pIter->done(); pIter->step())
	{
		// Get the block table record
		AcDbBlockTableRecord* pBTR; pIter->getRecord(pBTR, AcDb::kForRead);

		// If this is a layout
		if (pBTR->isLayout())
		{
			// Get the layout's object ID and from there its name
			layoutId = pBTR->getLayoutId();
			AcDbLayout *pLayout; acdbOpenAcDbObject((AcDbObject*&)pLayout, layoutId, AcDb::kForRead);
			ACHAR *pLayoutName; pLayout->getLayoutName(pLayoutName);
			CString csLayoutName; csLayoutName.Format(_T("%s"), pLayoutName);

			// If Model space layout or if the layout name doesn't have "Layout" in its name (not interested). Because, in the template we assume that
			// the sheet templates will be named "A1 Layout", "A2 Layout" etc.
			if (csLayoutName.CompareNoCase(_T("Model")) && (csLayoutName.Find(_T(" Layout")) != -1)) 
			{
				if (m_cbSheetSize.SelectString(-1, csLayoutName.Mid(0, 2)) == -1) m_cbSheetSize.AddString(csLayoutName.Mid(0, 2));
			}

			pLayout->close();
		}

		pBTR->close();
	}

	delete pIter;
}

/////////////////////////////////////////////////////////////////////////
// Function name: CRouteVPortDlg::CalculatePermissibleSizes
// Description  :
/////////////////////////////////////////////////////////////////////////
void CRouteVPortDlg::CalculatePermissibleSizes()
{
	// Get the sheet size and scale values
	if (m_cbSheetSize.GetCurSel() == CB_ERR) return;
	if (m_cbScale.GetCurSel() == CB_ERR) return;
	m_cbSheetSize.GetLBText(m_cbSheetSize.GetCurSel(), m_csSheet);
	m_cbScale.GetLBText(m_cbScale.GetCurSel(), m_csScale);

	// Determine the maximum Length and Width of the viewport for the selected paper size
	/**/ if (!m_csSheet.CompareNoCase(L"A4")) { m_dVPortLength = 273.0;  m_dVPortWidth = 165;    } 
	else if (!m_csSheet.CompareNoCase(L"A3")) { m_dVPortLength = 390;    m_dVPortWidth = 226;    }
	else if (!m_csSheet.CompareNoCase(L"A2")) { m_dVPortLength = 552.0;  m_dVPortWidth = 339.0;  }
	else if (!m_csSheet.CompareNoCase(L"A1")) { m_dVPortLength = 787.0;  m_dVPortWidth = 505.0;  }
	else if (!m_csSheet.CompareNoCase(L"A0")) { m_dVPortLength = 1113.0; m_dVPortWidth = 734.0;  } 

	// Reduction of the above sizes for margins
	m_dVPortLength -= 10.0;
	m_dVPortWidth  -= 10.0;
	
	CString csScale = m_csScale;
	double dScale   = _tstof(csScale.Mid(csScale.Find(_T(":")) + 1));
		
	// Get the permissible length
	// m_dVPortLength = m_dVPortLength * 1000 / dScale;
	// m_dVPortWidth  = m_dVPortWidth * 1000 / dScale;

	m_dVPortLength = m_dVPortLength * dScale / 1000;
	m_dVPortWidth  = m_dVPortWidth * dScale / 1000;

	m_csLength.Format(L"%s", suppressZero(m_dVPortLength));
	m_csWidth.Format(L"%s",suppressZero(m_dVPortWidth));  

	SetDlgItemText(IDC_RV_LENGTH, m_csLength);
	SetDlgItemText(IDC_RV_WIDTH,  m_csWidth);
}

/////////////////////////////////////////////////////////////////////////
// Function name: CRouteVPortDlg::OnCbnSelchangeSheetsize()
// Description  :
/////////////////////////////////////////////////////////////////////////
void CRouteVPortDlg::OnCbnSelchangeSheetSize() { CalculatePermissibleSizes(); }
void CRouteVPortDlg::OnCbnSelchangeScale()     { CalculatePermissibleSizes(); }

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
void CRouteVPortDlg::OnBnClickedPickLength()
{
	// Set the flag to enable the user to specify width
	m_iCalledFor = 3;

	// Close the dialog
	OnOK();
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
void CRouteVPortDlg::OnBnClickedPickWidth()
{
	// Set the flag to enable the user to specify width
	m_iCalledFor = 4;

	// Close the dialog
	OnOK();
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
void CRouteVPortDlg::OnBnClickedPickOverlap()
{
	// Set the flag to enable the user to specify width
	m_iCalledFor = 5;

	// Close the dialog
	OnOK();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CRouteVPortDlg::OnOK()
// Description  : Validates the inputs specified to draw the routes in drawing.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRouteVPortDlg::OnOK()
{
	UpdateData(TRUE);

	// Get sheet size
	if (m_cbSheetSize.GetCurSel() != CB_ERR) m_cbSheetSize.GetLBText(m_cbSheetSize.GetCurSel(), m_csSheet); 

	// Scale
	if (m_cbScale.GetCurSel() != CB_ERR) m_cbScale.GetLBText(m_cbScale.GetCurSel(), m_csScale);

	// Validate the inputs
	if (!m_iCalledFor)
	{
		// Polyline selection
		if (!m_objRoute.isValid()) { appMessage(L"Select route path.");	return; }

		// Length
		if (_tstof(m_csLength) <= 0.0) { ShowBalloon(L"Specify route length.", this, IDC_RV_LENGTH); return; }
		if (_tstof(m_csWidth) > m_dVPortLength)
		{
			CString csMsg; csMsg.Format(L"Maximum permissible route length is %s.", suppressZero(m_dVPortLength));
			ShowBalloon(csMsg, this, IDC_RV_LENGTH); 
			return; 
		}

		// Width
		if (_tstof(m_csWidth) <= 0.0) { ShowBalloon(L"Specify route width.", this, IDC_RV_LENGTH); return; }
		if (_tstof(m_csWidth) > m_dVPortLength)
		{
			CString csMsg; csMsg.Format(L"Maximum permissible woute width is %s.", suppressZero(m_dVPortWidth));
			ShowBalloon(csMsg, this, IDC_RV_WIDTH); 
			return; 
		}
				
		// Inputs validated and confirmed OK
		m_iCalledFor = 100;
	}
	
	CDialog::OnOK();
}

//////////////////////////////////////////////////////////////////
// Function name: CRouteVPortDlg::RemoveRoute()
// Description  : Removes the current route from the drawing.
//////////////////////////////////////////////////////////////////
void CRouteVPortDlg::RemoveRoute()
{
	if (!m_objRoute.isValid()) { m_objRoute.setNull(); return; }

	AcDbEntity *pEntity;
	if (acdbOpenObject(pEntity, m_objRoute, AcDb::kForWrite) != Acad::eOk) return;
	pEntity->erase();
	pEntity->close();

	m_objRoute.setNull();
}

//////////////////////////////////////////////////////////////////////////
// Function name: CRouteVPortDlg::DefineRoute()
// Description  : Enables selection of points to define a new route.
//////////////////////////////////////////////////////////////////////////
void CRouteVPortDlg::DefineRoute()
{
	TCHAR result[7];
	ads_point ptNext;
	ads_point ptPrev;
	AcDbPolyline *pLine = NULL;
	int iRet;

	if (!m_objRoute.isValid())
	{
		// Called when a new route has to be drawn
		ads_point ptStart;
		iRet = acedGetPoint(NULL, L"\nSpecify route start point: ", ptStart);
		if (iRet == RTCAN) return;

		// Draw a route pline with length zero at the moment
		pLine = new AcDbPolyline();
		pLine->addVertexAt(0, AcGePoint2d(ptStart[X], ptStart[Y]));
		pLine->addVertexAt(1, AcGePoint2d(ptStart[X], ptStart[Y]));
		pLine->setLayer(L"0");

		// Append it to the database
		acutPolar(ptStart, 0.0, 0.0, ptPrev);
		m_objRoute = appendEntityToDatabase(pLine, true);
		pLine->close();
	}
	else
	{
		// Called to edit the route already drawn. 
		if (acdbOpenObject(pLine, m_objRoute, AcDb::kForWrite) != Acad::eOk) return;
		AcGePoint2d gePt;
		pLine->getPointAt(pLine->numVerts() -1, gePt);
		pLine->close();

		// Set the last vertex of this route as the previous point.
		acutPolar(asDblArray(gePt), 0.0, 0.0, ptPrev);
	}

	while (T)
	{
		acedInitGet(NULL, L"Object Undo Change");
		iRet = acedGetPoint(ptPrev, L"\nSpecify next point or [Object/Undo/Change end]: ", ptNext);

		/**/ if (iRet == RTCAN) 
		{
			m_csSelectedText = L"1 object selected";
			return;

			//if (m_objRoute.isValid())
			//{
				// Erase the route drawn so far
				//if (acdbOpenObject(pLine, m_objRoute, AcDb::kForWrite) == Acad::eOk) { pLine->erase(); pLine->close(); }
			//}

			return;
		}
		else if (iRet == RTNORM)
		{
			// Draw the polyline
			DrawRoutePolyline(m_objRoute, ptNext);

			// Origin of the elastic line as the next input is queried
			acutPolar(ptNext, 0.0, 0.0, ptPrev);
		}
		else if ((iRet == RTKWORD) && (acedGetInput(result) == RTNORM))
		{
			/**/ if (result[0] == L'O') SelectEntity(m_objRoute, ptPrev);
			else if (result[0] == L'C') ReverseRoute(m_objRoute, ptPrev);
			else if (result[0] == L'U') UndoSegment (m_objRoute, ptPrev);
		}
		else if (iRet == RTNONE)
		{
			m_csSelectedText = L"1 object selected";
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CRouteVPortDlg::GenerateDesign()
// Description  : Along the selected polyline profile, the function places the ROUTE representations.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRouteVPortDlg::GenerateDesign(AcDbObjectIdArray &objVPorts, CStringArray &csaSheetNo)
{
	// Save the current setting in XRECORD
	struct resbuf *rbpXRec = acutBuildList(AcDb::kDxfText, m_csSheet, AcDb::kDxfText, m_csScale, AcDb::kDxfText, m_csLength, AcDb::kDxfText, m_csWidth, AcDb::kDxfText, m_csOverlap, AcDb::kDxfReal, m_dVPortLength, AcDb::kDxfReal, m_dVPortWidth, NULL);
	addXRecordToDictionary(L"eCapture", L"RouteVport", rbpXRec);
	acutRelRb(rbpXRec);
	
	AcDbPolyline *pCable;
	AcDb2dPolyline *pCable2D;
	Acad::ErrorStatus es;
	AcGePoint3d geptStart;

	// Get the length of the PLINE
	double dLength;
	double dStartDist = 0;
	double dEndDist = 0;

	// Open the route to get its start point (useful while making selection for command "LENGTHEN")
	AcDbEntity *pEntity;
	es = acdbOpenObject(pEntity, m_objRoute, AcDb::kForRead);
	if (es != Acad::eOk) { return; }

	/**/ if (AcDbPolyline::cast(pEntity))
	{
		pCable = AcDbPolyline::cast(pEntity);
		pCable->getPointAt(0, geptStart);
		pCable->close();
	}
	else { pEntity->close(); return; } 

	// Use the command to determine the length of the selected polyline
	ads_name enRoute; acdbGetAdsName(enRoute, m_objRoute);
	acedCommandS(RTSTR, L".LENGTHEN", RTLB, RTENAME, enRoute, RTPOINT, asDblArray(geptStart), RTLE, RTSTR, L"", NULL);
	struct resbuf rbSetVar; acedGetVar(L"PERIMETER", &rbSetVar);
	dLength = rbSetVar.resval.rreal;

	// Start placing the VPORTS from the Start point to End point
	es = acdbOpenObject(pCable, m_objRoute, AcDb::kForRead);
	if (es != Acad::eOk) { return; }

	double dCoveredLen = 0.0;
	AcGePoint3d geptInsert;
	double dAngle = -1000.0;
	AcDbObjectId objInsert;
	AcDbObjectIdArray aryObjIds;

	int iVportCnt = 0;
	int iSheetNo = GetNextSheetNumber(m_csSheet) + 1;
	CString csValue;
	CString csTmpVal;

	// The last point to enable calculation of angle
	AcGePoint3d geptLastInsert = geptStart;

	// Automatically calculate the no. of VPORTS that we can accommodate in the sheet selected
	int iNoOfVPortsPerSheet = (floor) (m_dVPortWidth / _tstof(m_csWidth));
	if (iNoOfVPortsPerSheet > 4) iNoOfVPortsPerSheet = 4;
	
	while (T) 
	{
		es = pCable->getPointAtDist(dCoveredLen, geptInsert);
		if (es != Acad::eOk) { pCable->close(); break; }

		// Place the route at the current location
		if (dAngle == -1000)
		{
			AcGePoint3d geAngle;
			es = pCable->getPointAtDist(1.0, geAngle);
			dAngle = acutAngle(asDblArray(geptLastInsert), asDblArray(geAngle));
		}
		else dAngle = acutAngle(asDblArray(geptLastInsert), asDblArray(geptInsert));

		// Get the block definition from the template (if not available locally)
		if (!acdbTblSearch(L"BLOCK", L"ROUTE_PLAN_VIEW", FALSE)) {	UpdateBlockFromStandards(L"ROUTE_PLAN_VIEW"); }
		if (!insertBlock(_T("ROUTE_PLAN_VIEW"), _T("0"), asDblArray(geptInsert), 1.0, 1.0, 0.0, 0.0, _T(""), objInsert, true)) 
		{
			acutPrintf(L"\nUnable to insert route block in layout.");
			pCable->close();
			return;
		}

		// Change the sheet size settings
		ChangeVisibility(objInsert, m_csSheet);

		// Set the length, width and angle of the VPORT
		/*
		ChangeCustomParameters(objInsert, L"A4 LENGTH", m_csLength);
		ChangeCustomParameters(objInsert, L"A4 WIDTH",  m_csWidth);
		ChangeCustomParameters(objInsert, L"A4 ANGLE", suppressZero(dAngle));
		*/

		CString csLabel; 
		
		csLabel.Format(L"%s LENGTH", m_csSheet); ChangeCustomParameters(objInsert, csLabel, m_csLength);
		csLabel.Format(L"%s WIDTH",  m_csSheet); ChangeCustomParameters(objInsert, csLabel, m_csWidth);
		csLabel.Format(L"%s ANGLE",  m_csSheet); ChangeCustomParameters(objInsert, csLabel, suppressZero(dAngle));

		// Add this to the array
		aryObjIds.append(objInsert);

		// Attach the ROUTE XDATA for placing these later on in PAPER SPACE
		if (iVportCnt + 1 > iNoOfVPortsPerSheet) { iSheetNo++; iVportCnt = 0; }
		AttachRouteXData(objInsert, m_csSheet, suppressZero(iSheetNo), m_csScale);
		iVportCnt++;

		dCoveredLen += _tstof(m_csLength);

		// dCoveredLen += (_tstof(dlgRVPort.m_csGap) * dScaleFactor / 1000);
		dCoveredLen -= (_tstof(m_csOverlap) * _tstof(m_csLength) / 100);
		geptLastInsert = geptInsert;

		// Save the count of VPORTS drawn for each sheet number
		int iIndex = GetParameterValue(csaSheetNo, suppressZero(iSheetNo), csValue, 0);
		if (iIndex != -1)
		{
			// If present we will add one more to it save it back
			csTmpVal.Format(L"%s#%d", suppressZero(iSheetNo), _tstoi(csValue) + 1);
			csaSheetNo.SetAt(iIndex, csTmpVal);
		}
		else
		{
			// New sheet number
			csTmpVal.Format(L"%s#1", suppressZero(iSheetNo));
			csaSheetNo.Add(csTmpVal);
		}

		// Append the object ID to array, for removing these if the "Settings" in the dialog are to be revisited
		objVPorts.append(objInsert);
	}

	pCable->close(); 

	// Ask the user whether wants to proceed with creating sheets
	if (!aryObjIds.length()) { appMessage(L"No valid routes defined in drawing to create sheets.", 0); return; }

	// Zoom BIG
	acedCommandS(RTSTR, L".ZOOM", RTSTR, L"E", NULL);
}

//////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedNewRoute
// DEscription  : Enables defining a route to place the viewports along it.
//////////////////////////////////////////////////////////////////////////
void CRouteVPortDlg::OnBnClickedNewRoute()
{
	// Dismiss the dialog
	m_iCalledFor = 1;
	OnOK();
}

//////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedEditRoute
// DEscription  : Exits the dialog when the clicks on route edit.
//////////////////////////////////////////////////////////////////////////
void CRouteVPortDlg::OnBnClickedEditRoute()
{
	// Dismiss the dialog
	m_iCalledFor = 2;
	OnOK();
}

/////////////////////////////////////////////////////
// Function name: OnBnClickedHelp
// Description  : Calls the help window.
/////////////////////////////////////////////////////
void CRouteVPortDlg::OnBnClickedHelp() { displayHelp((DWORD)_T("Route_View_Plan.htm")); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: Command_RouteVP
// Description  : Function to create route paths, place route view along the path and generate the route views in paper space.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_RoutePlan()
{
	// Switch off certain SYSVARS
	switchOff();

	CRouteVPortDlg dlgRVPort;
	AcDbObjectIdArray objVPorts;
	CStringArray csaSheetNo; 

	// Get current setting from XRECORD
	struct resbuf *rbpXRec = getXRecordFromDictionary(L"eCapture", L"RouteVport");
	if (rbpXRec)
	{
		dlgRVPort.m_csSheet        = rbpXRec->resval.rstring; rbpXRec = rbpXRec->rbnext;
		dlgRVPort.m_csScale        = rbpXRec->resval.rstring; rbpXRec = rbpXRec->rbnext;
		dlgRVPort.m_csLength       = rbpXRec->resval.rstring; rbpXRec = rbpXRec->rbnext;
		dlgRVPort.m_csWidth        = rbpXRec->resval.rstring; rbpXRec = rbpXRec->rbnext; 
		dlgRVPort.m_csOverlap      = rbpXRec->resval.rstring; rbpXRec = rbpXRec->rbnext;
		dlgRVPort.m_dVPortLength   = rbpXRec->resval.rreal;   rbpXRec = rbpXRec->rbnext;
		dlgRVPort.m_dVPortWidth    = rbpXRec->resval.rreal;  
		acutRelRb(rbpXRec);
	}

	while (T)
	{
		// Display the route VPORTS dialog
		dlgRVPort.m_iCalledFor = 0;
		if (dlgRVPort.DoModal() == IDCANCEL) 
		{
			// Remove the route and exit the function
			dlgRVPort.RemoveRoute();
			return;
		}

		// The dialog is exited for a number of reasons. Determine the reason and react.
		if (dlgRVPort.m_iCalledFor == 1)
		{
			//////////////////////////////////////////////////////////////////////////
			// User opted to create a new route
			//////////////////////////////////////////////////////////////////////////
			// Remove the existing route
			csaSheetNo.RemoveAll();
			dlgRVPort.RemoveRoute();

			// Call the function to enable definition of new route
			dlgRVPort.DefineRoute();
			continue;
		}
		else if (dlgRVPort.m_iCalledFor == 2)
		{
			//////////////////////////////////////////////////////////////////////////////////////////////////
			// User opted to edit the existing route
			//////////////////////////////////////////////////////////////////////////////////////////////////
			// Same function called for the new route too. If a route is present, it enables editing the same
			dlgRVPort.DefineRoute();
			continue;
		}
		else if (dlgRVPort.m_iCalledFor == 3)
		{
			///////////////////////////////////////////////////
			// User opted to specify the LENGTH of the route
			///////////////////////////////////////////////////
			double dLength = 0.0;
			while (T)
			{
				CString csPrompt; csPrompt.Format(_T("\nLength of route <%s>: "), dlgRVPort.m_csLength);
				acedInitGet(RSG_NONEG | RSG_NOZERO, _T(""));
				int iRet = acedGetDist(NULL, csPrompt, &dLength);
				/**/ if (iRet == RTCAN)  break;
				else if (iRet == RTNONE) dLength = _tstof(dlgRVPort.m_csLength);

				if (dLength > dlgRVPort.m_dVPortLength) acutPrintf(_T("\nLength specified must be less than %.2f."), dlgRVPort.m_dVPortLength); else break;
			}

			dlgRVPort.m_csLength.Format(L"%.2f", dLength);
			continue;
		}
		else if (dlgRVPort.m_iCalledFor == 4)
		{
			///////////////////////////////////////////////////
			// User opted to specify the WIDTH of the route
			///////////////////////////////////////////////////
			double dWidth = 0.0;
			while (T)
			{
				CString csPrompt; csPrompt.Format(_T("\nWidth of route <%s>: "), dlgRVPort.m_csWidth);
				acedInitGet(RSG_NONEG | RSG_NOZERO, _T(""));
				int iRet = acedGetDist(NULL, csPrompt, &dWidth);
				/**/ if (iRet == RTCAN)  break;
				else if (iRet == RTNONE) dWidth = _tstof(dlgRVPort.m_csWidth);

				if (dWidth > dlgRVPort.m_dVPortWidth) acutPrintf(_T("\nWidth specified must be less than %.2f."), dlgRVPort.m_dVPortWidth); else break;
			}

			dlgRVPort.m_csWidth.Format(L"%.2f", dWidth);
			continue;
		}
		else if (dlgRVPort.m_iCalledFor == 5)
		{
			/////////////////////////////////////////////////
			// User opted to specify the OVERLAP distance 
			/////////////////////////////////////////////////
			double dOverlap = 0.0;
			while (T)
			{
				CString csPrompt; csPrompt.Format(_T("\nDistance of overlap <%s>: "), dlgRVPort.m_csOverlap);
				acedInitGet(RSG_NONEG | RSG_NOZERO, _T(""));
				int iRet = acedGetDist(NULL, csPrompt, &dOverlap);
				/**/ if (iRet == RTCAN)  break;
				else if (iRet == RTNONE) dOverlap = _tstof(dlgRVPort.m_csOverlap);

				if (dOverlap > dlgRVPort.m_dVPortLength * 0.20) acutPrintf(_T("\nOverlap specified must be less than %.2f."), dlgRVPort.m_dVPortLength * 0.20); else break;
			}

			dlgRVPort.m_csOverlap.Format(L"%.2f", dOverlap);
			continue;
		}

		// Call the function to show the VPORTS along the route
		dlgRVPort.GenerateDesign(objVPorts, csaSheetNo);

		// Prompt the user to know the next course of action
		TCHAR result[10];
		acedInitGet(NULL, L"Settings Accept");
		int iRet = acedGetKword(L"\nEnter an option [Settings/Regen/aDd/Accept] <Accept>: ", result);
		
		/**/ if (iRet == RTNONE)
		{
			// Remove the existing route
			dlgRVPort.RemoveRoute();

			// Call the function to create the route sheets
			// CreateSheets(dlgRVPort.m_csSheet + L" Layout", dlgRVPort.m_csScale, dlgRVPort.m_csScale, csaSheetNo);
			CreateSheets(dlgRVPort.m_csSheet, dlgRVPort.m_csScale, dlgRVPort.m_csScale, csaSheetNo);
			return;
		}
		else if ((iRet == RTNORM) && (result[0] == 'A') && (result[1] == 'c'))
		{
			// Remove the existing route
			dlgRVPort.RemoveRoute();

			// Call the function to create the route sheets
			// CreateSheets(dlgRVPort.m_csSheet + L" Layout", dlgRVPort.m_csScale, dlgRVPort.m_csScale, csaSheetNo);
			CreateSheets(dlgRVPort.m_csSheet, dlgRVPort.m_csScale, dlgRVPort.m_csScale, csaSheetNo);
			break;
		}
		else if ((iRet == RTCAN) || (iRet == RTNORM) && (result[0] == 'S'))
		{
			//////////////////////////////////////////////////////////////////////////////////////////////////////////
			// "Settings" :- Deletes the Summary layout and returns to the dialog to allow settings to be adjusted
			//////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Erase all the VPORTS placed
			AcDbEntity *pEnt;
			for (int iObj = 0; iObj < objVPorts.logicalLength(); iObj++)
			{
				if (acdbOpenObject(pEnt, objVPorts.at(iObj), AcDb::kForWrite) == Acad::eOk)
				{
					pEnt->erase();
					pEnt->close();
				}
			}

			// Update the drawing
			ads_regen();
		}
	}
}

