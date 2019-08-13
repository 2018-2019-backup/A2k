#include "StdAfx.h"
#include "PoleInfo.h"
#include "OffsetInfo.h"

extern void sortConductorInfo   (std:: vector <CConductorInfo> &conductorInfo);
extern void reverseConductorInfo(std:: vector <CConductorInfo> &conductorInfo);
extern void appendPoleInfo			(AcDbObjectId objId, ads_point ptPole, std:: vector <CConductorInfo> conductorInfo, std:: vector <CPoleInfo> &poleIInfoVector);

double g_dPoleDia = 0.0;		

//////////////////////////////////////////////////////////////////////////
// Function name: DrawConductors
// Description  : 
//////////////////////////////////////////////////////////////////////////
bool DrawConductors(bool bModifySourceConductor, std::vector <CPoleInfo> &poleInfo_Vector, ads_point ptNewPole, std:: vector <CConductorInfo> &conductorInfo_Vector, std:: vector <COffsetInfo> offsetInfo_Vector, AcDbObjectIdArray &arObjIds)
{
  // Insertion point of the previous pole
  ads_point ptSelPole; acutPolar(poleInfo_Vector[poleInfo_Vector.size() - 1].m_ptInsert, 0.0, 0.0, ptSelPole);

  // Angle to draw the conductors
  double dAngle = acutAngle(ptSelPole, ptNewPole);

  ads_point ptStart, ptEnd;
  Acad::ErrorStatus es;
  double dOffset;

  for (int iRow = 0; iRow < conductorInfo_Vector.size(); iRow++)
  {
    // Offset point from center of the pole perpendicular to the conductor
    if (conductorInfo_Vector.size() > 1)
      dOffset = conductorInfo_Vector[iRow].m_dOffset;
    else
      dOffset = 0.0; // There is only one conductor, so offset not required

		// Start point of the conductor at the selected pole
    acutPolar(ptSelPole, dAngle + PIby2, dOffset, ptStart);
		
    // Get the offset along the conductor
    if (fabs(dOffset) < g_dPoleDia / 2)
    {
      double dOppSide = sqrt((g_dPoleDia * g_dPoleDia / 4) - (dOffset * dOffset));
      acutPolar(ptStart, dAngle, dOppSide, ptStart);
    }

    // End point of the conductor at offset from center of the new pole
    acutPolar(ptNewPole, dAngle + PIby2, dOffset, ptEnd);

    // Get the entity type of the conductor to duplicate
    AcDbEntity *pEntity;
    es = acdbOpenObject(pEntity, conductorInfo_Vector[iRow].m_objId, AcDb::kForRead);
    if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); acTransactionManagerPtr()->abortTransaction(); return false; }

    bool bIsLine = true;
    if (pEntity->isKindOf(AcDbLine::desc())) bIsLine = true; else bIsLine = false;

    // if (!bIsLine)
    {
      // Add the first and second vertex
      AcDbPolyline *pPline = new AcDbPolyline();
      pPline->addVertexAt(0, AcGePoint2d(ptStart[X], ptStart[Y]), 0.0);
      pPline->addVertexAt(1, AcGePoint2d(ptEnd[X], ptEnd[Y]), 0.0);

      // Set the layer
      pPline->setLayer(conductorInfo_Vector[iRow].m_csLayer);
      pPline->setLinetype(_T("BYLAYER"));
      AcCmColor color; color.setColor(AcCmEntityColor::kByLayer);
      pPline->setColor(color);

      // Append the entity for deleting just in case if a flip is required
      appendEntityToDatabase(pPline);

      // The new conductor ID is required for the continuity of the command
      conductorInfo_Vector[iRow].m_objIdNext = pPline->objectId();

      // This is done to erase them during a flip
      arObjIds.append(pPline->objectId());

			if (g_dPoleDia)
			{
				// Extend the lines to meet inside the pole
        AcGePoint3dArray geIntersectPts;
				es = pPline->intersectWith(pEntity, AcDb::kExtendBoth, geIntersectPts);
				if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); }
				else if (geIntersectPts.length() > 0)
				{
					// Modify the start point of the new conductors
					pPline->setPointAt(0, AcGePoint2d(geIntersectPts.at(0).x, geIntersectPts.at(0).y));
				}
				pEntity->close();
							        
				// Modify the start point of the source conductor
				if (bModifySourceConductor || (conductorInfo_Vector.size() > 1))
				{
					AcDbPolyline *pExtPline;
					if (conductorInfo_Vector[iRow].m_objId.isValid())
					{
						es = acdbOpenObject(pExtPline, conductorInfo_Vector[iRow].m_objId, AcDb::kForWrite);
						if (es == Acad::eOk)
						{
							AcGePoint3d geStartPt; pExtPline->getStartPoint(geStartPt);
							AcGePoint3d geEndPt;   pExtPline->getEndPoint(geEndPt);

							CString csDist1; csDist1.Format(_T("%.2f"), acutDistance(asDblArray(geStartPt), ptSelPole));
							CString csDist2; csDist2.Format(_T("%.2f"), acutDistance(asDblArray(geEndPt),   ptSelPole));
							
							if (_tstof(csDist1) <= _tstof(csDist2))
								pExtPline->setPointAt(0, AcGePoint2d(geIntersectPts.at(0).x, geIntersectPts.at(0).y));
							else
								pExtPline->setPointAt(pExtPline->numVerts() - 1, AcGePoint2d(geIntersectPts.at(0).x, geIntersectPts.at(0).y));
							pExtPline->close();
						}
					}
        }
      }
			else pEntity->close();

      // Get the entity name of this cable to use in extend command
      pPline->close();
    }

    // Replace the new object id with the new one
    conductorInfo_Vector[iRow].m_objId		 = conductorInfo_Vector[iRow].m_objIdNext;
    conductorInfo_Vector[iRow].m_objIdNext = AcDbObjectId::kNull;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name: selectPole()
// Description  : Enables selection of valid pole block.
// Arguments    : 1. AcDbObjectId &, Object ID of the selected pole.
//  				    : 2. ads_point,			 Insertion point of the pole.
//////////////////////////////////////////////////////////////////////////
bool selectPole(AcDbObjectId &objPole, ads_point ptPole, CStringArray &csaValidObjects, CStringArray &csaValidLayers)
{
  int iRet;
  ads_name enPole;
  ads_point ptDummy;
  Acad::ErrorStatus es;
  AcDbBlockReference *pInsert = NULL;
  while (TRUE)
  {
		iRet = acedEntSel(_T("\n\rSelect support or connection: "), enPole, ptDummy);
    /**/ if (iRet == RTCAN) return false;
    else if (iRet == RTNORM) 
    {
      // Check if the entity selected is a valid pole
      acdbGetObjectId(objPole, enPole);

      es = acdbOpenObject(pInsert, objPole, AcDb::kForRead);
      if ((es != Acad::eOk) && (es != Acad::eNotThatKindOfClass)) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); return false; }

      // Check if the selection is an insert
      if (pInsert == NULL) { acutPrintf(_T("\nNot a valid support or connection.\n")); continue; }
			
			// Get the insertion point
			ptPole[X] = pInsert->position().x; ptPole[Y] = pInsert->position().y;	ptPole[Z] = 0.0;

			// Get the name of the insert
			AcDbObjectId objTblRcd = pInsert->blockTableRecord();
			
			// Close the insert as it found an insert
			pInsert->close();

			AcDbBlockTableRecord *pBlkTblRcd = NULL;
			if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk) { return false; }

			ACHAR *pValue; 
			pBlkTblRcd->getName(pValue); 
			CString csName; csName.Format(_T("%s"), pValue); csName.MakeUpper();
			delete pValue;
			pBlkTblRcd->close();

			// Get the valid objects for selection from tblExtendObjects
			CQueryTbl tblExtendObjects;
			CString csSQL;
			csSQL.Format(L"SELECT [fldInstallation], [fldNewObject], [fldNewLayer] FROM tblExtendObjects WHERE [fldObject] = '%s' ORDER BY [ID]", csName);
			if (!tblExtendObjects.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return false;

			if (tblExtendObjects.GetRows() <= 0) { acutPrintf(L"\nSelected support or connection is invalid."); continue; }
			
			CString csValidObject;
			CString csValidLayer;

			for (int iRow = 0; iRow < tblExtendObjects.GetRows(); iRow++)
			{
				// New block name for the new object
				csValidObject.Format(L"%s#%s", tblExtendObjects.GetRowAt(iRow)->GetAt(0), tblExtendObjects.GetRowAt(iRow)->GetAt(1));
				csaValidObjects.Add(csValidObject);

				// Layer name for the new object
				csValidLayer.Format(L"%s#%s", tblExtendObjects.GetRowAt(iRow)->GetAt(0), tblExtendObjects.GetRowAt(iRow)->GetAt(2));
				csaValidLayers.Add(csValidLayer);
			}

      return true;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Function name: GetOffsetFromPoleCenter()
// Description  : Gets the offset distance from the center of the symbol.
//////////////////////////////////////////////////////////////////////////
double GetOffsetFromPoleCenter(AcDbEntity *pEntity, ads_point ptPole, double &dRefAngle /*, AcGeVector3d *pVector*/)
{
  double dOffset = -99999.9;
  
  // Cast the entity to a curve
  AcDbCurve *pCurve = (AcDbCurve *) pEntity;
  if (!pCurve) return dOffset;

  // If the entity is a LWPolyLine, temporarily create a LINE entity. What to do, the getClosestPoint() is not working with extend flag for LWPOLYLINE.
  AcGePoint3d geResult;
  Acad::ErrorStatus es;
  if (pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc()))
  {
    AcDbObjectIdArray aryObjIds;

    AcGePoint3d geStart, geEnd;
    pCurve->getStartPoint(geStart);
    pCurve->getEndPoint(geEnd);

    // Close the polyline curve
    pCurve->close();

    // Add a temporary line
    AcDbLine *pLine = new AcDbLine(geStart, geEnd);
    appendEntityToDatabase(pLine);
    AcDbObjectId objId = pLine->objectId(); 

    // Append the object Id to be deleted later
    aryObjIds.append(objId);

    // Now get the LINE's curve pointer
    pCurve = (AcDbCurve *) pLine;

    // Get the closest point
    es = pCurve->getClosestPointTo(AcGePoint3d(ptPole[X], ptPole[Y], 0.0), geResult, Adesk::kTrue);

    // Close the temporary line
    pLine->close();
    pCurve->close();

    // Remove the temporary line
    deleteArray(aryObjIds);

    if (es != Acad::eOk) { acutPrintf(_T("\nERROR @%d: %s"), __LINE__, acadErrorStatusText(es)); return dOffset; }
  }
  else
  {
    // Get the closest point
    es = pCurve->getClosestPointTo(AcGePoint3d(ptPole[X], ptPole[Y], 0.0), geResult, Adesk::kTrue);
    pCurve->close();
    if (es != Acad::eOk) { acutPrintf(_T("\nERROR @%d: %s"), __LINE__, acadErrorStatusText(es)); return dOffset; }
  }

  // pVector->set(ptPole[X] - asDblArray(geResult)[X], ptPole[Y] - asDblArray(geResult)[Y], 0.0);
  // dOffset = acutDistance(ptPole, asDblArray(geResult));
  // if (((ptPole[X] - asDblArray(geResult)[X]) < 0) && ((ptPole[Y] - asDblArray(geResult)[Y]) < 0)) dOffset *= -1;

  dOffset = acutDistance(ptPole, asDblArray(geResult));
  CString csRefAngle;  csRefAngle.Format(_T("%.2f"),  dRefAngle);
  CString csThisAngle; csThisAngle.Format(_T("%.2f"), RTD(acutAngle(ptPole, asDblArray(geResult))));
  if (csRefAngle == _T("-999.99"))
    dRefAngle = RTD(acutAngle(ptPole, asDblArray(geResult))); 
  else
    if (csRefAngle != csThisAngle) dOffset *= -1;

  return dOffset;
}

//////////////////////////////////////////////////////////////////////////
// Function name: selectConductors()
// Description  : Enables selection of valid conductors.
// Arguments    : 1. ads_point,			 Insertion point of the pole.
//                2. std:: vector <CConductorInfo> &, Conductor vector
//  				    : 3. std:: vector <COffsetInfo> &, Offset vector
//////////////////////////////////////////////////////////////////////////
bool selectConductors(ads_point ptPole, std:: vector <CConductorInfo> &conductorInfo_Vector, std:: vector <COffsetInfo> &offsetInfo_Vector, int &iInstallationType)
{
  Acad::ErrorStatus es;
  AcDbObjectId objId;
  ads_name enEntity;
  AcDbEntity *pEntity;
  //long lLength = 0L;
  int lLength = 0L;
  CString csLayerName;

  // Array that defines the order of drawing the conductors
  CStringArray csaCableSortOrder; 
  
	// Get the order of drawing circuits from tblExtendCicuits
	CQueryTbl tblExtendCircuits;
	CString csSQL;
	csSQL.Format(L"SELECT [fldCircuits],[fldLayer] FROM tblExtendCircuits ORDER BY [fldDrawSequence]");
	if (!tblExtendCircuits.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return false;

	CString csCircuitInfo;
	for (int iR = 0; iR < tblExtendCircuits.GetRows(); iR++) 
	{
		csCircuitInfo.Format(L"%s#%s", tblExtendCircuits.GetRowAt(iR)->GetAt(0), tblExtendCircuits.GetRowAt(iR)->GetAt(1));	
		csaCableSortOrder.Add(csCircuitInfo); 
	}
		
  // const TCHAR *prompts[2];
  // _tcsset(prompts, L"Select circuits: "); // , L"Remove circuits: " };
	acutPrintf(_T("\r\nSelect circuits connected to this pole...\n"));
  while (T)
  {
    ads_name ssGet;
		const TCHAR* sPrompts[] = { L"Select circuit: ", L"Remove circuit: " };
		int iRet = acedSSGet(L":$", sPrompts, NULL, NULL, ssGet);

    /**/ if (iRet == RTCAN)  return false;
    else if (iRet != RTNORM) continue;

    // Validate the entities selected
    acedSSLength(ssGet, &lLength);

    conductorInfo_Vector.clear();
    double dRefAngle = -999.99;
		iInstallationType = 0; // 0 for Invalid, 1 for OH and 2 for UG
    for (long lCtr = 0; lCtr < lLength; lCtr++)
    {
      // Get the entity name at the index
      acedSSName(ssGet, lCtr, enEntity);

      // Get the object ID for the entity
      acdbGetObjectId(objId, enEntity);

      // Get the entity pointer
      es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
      if (es != Acad::eOk) { acedSSFree(ssGet); return false; }

      // Check if the entity selected is a valid conductor i.e. LWPOLYLINE or a LINE
      if (!pEntity->isKindOf(AcDbPolyline::desc()) && !pEntity->isKindOf(AcDbLine::desc()) && !pEntity->isKindOf(AcDb2dPolyline::desc())) {	pEntity->close(); continue; }

      // Get the first two characters of the layer name. This will determine the circuit type for the conductor.
      csLayerName.Format(_T("%s"), pEntity->layer()); 
      csLayerName = csLayerName.MakeUpper();
      
      // The first two characters must have SV, LV, SL & TR and the layer name must have "_" in it.
      // if ((csLayerName.GetLength() < 2) || (csLayerName.Find(_T("_")) == -1)) {	pEntity->close(); continue; }

			// Get the circuit and installation part of the layer name
			CString csValidType;
			/**/ if (csLayerName.Find(L"OH") != -1) 
			{
				iInstallationType = 1;
				csValidType = csLayerName.Mid(0, csLayerName.Find(L"_OH") + 3);
			}
			else if (csLayerName.Find(L"UG") != -1)
			{
				iInstallationType = 2;
				csValidType = csLayerName.Mid(0, csLayerName.Find(L"_UG") + 3);
			}
			
			CString csNewLayerName;
			int iIndex = GetParameterValue(csaCableSortOrder, csValidType, csNewLayerName, 0);
			if (!iInstallationType || (iIndex== -1)) { acutPrintf(L"\nCircuit not suitable for support or connection."); break;	}

      // Create an instance of the offset information class
      COffsetInfo offsetInfo;

      // Get offset from pole center for this entity
      offsetInfo.m_dOffset = GetOffsetFromPoleCenter(pEntity, ptPole, dRefAngle /*&offsetInfo.m_geVector*/);
      pEntity->close();
      if (offsetInfo.m_dOffset == -99999.9) {	acutPrintf(_T("\nInvalid offset determined. The conductor must be connected to the pole selected.\n")); continue; }

      // Add this to the vector for sort
      offsetInfo_Vector.push_back(offsetInfo);

      //////////////////////////////////////////////////////////////////////////
      // Create an instance of the conductor information class
      //////////////////////////////////////////////////////////////////////////
      CConductorInfo conductorInfo;

      // This index determines the order at which the conductor has to be drawn
      conductorInfo.m_iIndex  = iIndex;
      conductorInfo.m_objId   = objId;
			
			conductorInfo.m_csLayer = csNewLayerName;
      conductorInfo.m_dOffset = offsetInfo.m_dOffset;
      createLayer(csNewLayerName, Adesk::kFalse, Adesk::kFalse);

      // Add this to the vector for sort
      conductorInfo_Vector.push_back(conductorInfo);
    }

    // Free the selection set
    acedSSFree(ssGet);

		// Check if the conductors selected are from valid installation types
		if (iInstallationType == 0) continue;

    // Check if there are any valid conductors selected
    if (conductorInfo_Vector.size() <= 0) { acutPrintf(_T("\nNo valid conductors selected.\nConductor must be a LINE/POLYLINE on layers with prefix \"SV/LV/SL/HV/TR\".\n"));	} else return true;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: JoinSegments
// Description  : Segments of the same circuit will be joined to one single cable
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void JoinSegments(std::vector <CPoleInfo> &poleInfo_Vector, int iNoOfCircuits)
{
	ads_name enPline; 
	switchOn();
	for (int iC = 0; iC < iNoOfCircuits; iC++)
	{
		// Get the entity representing this circuit between each pole to join them
		for (int iP = 1; iP < poleInfo_Vector.size(); iP++)
		{
			acdbGetAdsName(enPline, poleInfo_Vector.at(iP).m_conductorInfo_Vector.at(iC).m_objId);
			
			if (iP == 1)
				acedCommandS(RTSTR, L".PEDIT", RTENAME, enPline, RTSTR, L"J", NULL);
			else
				acedCommandS(RTENAME, enPline, NULL);
		}

		acedCommandS(RTSTR, L"", RTSTR, L"", NULL);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////	
// Function name: Command_ExtendPoleAndConductors()
// Description  : Defines the function that is called by "NE".
////////////////////////////////////////////////////////////////////////////////////////////
void Command_ExtendPoleAndConductors()
{
	switchOff();
	acutPrintf(L"\nVersion: V2.0");

  Acad::ErrorStatus es;

  // Allow user to an select existing pole and get insertion point too.
  AcDbObjectId objSelPole;
  ads_point ptSelPole;
  AcGePoint3dArray gePrvPoints;
	CStringArray csaValidObjects;
	CStringArray csaValidLayers;

  if (!selectPole(objSelPole, ptSelPole, csaValidObjects, csaValidLayers)) return;

  // Allow the user to select valid conductors
  std::vector <CConductorInfo> conductorInfo_Vector;
  std::vector <COffsetInfo>    offsetInfo_Vector;
  std::vector <CPoleInfo>			 poleInfo_Vector;	
	int iInstallationType = 0;
  if (!selectConductors(ptSelPole, conductorInfo_Vector, offsetInfo_Vector, iInstallationType)) return;

	// Get the block name and the layer name to insert the new symbol
	CString csBlkName;
	if (GetParameterValue(csaValidObjects, ((iInstallationType == 1) ? L"OH" : L"UG"), csBlkName, 0) == -1) {	acutPrintf(L"\nValid new block symbols not configured."); return; }

	CString csLayName; 
	if (GetParameterValue(csaValidLayers,  ((iInstallationType == 1) ? L"OH" : L"UG"), csLayName, 0) == -1) 
	{
		acutPrintf(L"\nValid layer name for new symbol is not configured. Assuming POLE_PROP to palce the new symbol."); 
		csLayName = L"POLE_PROP";
		return; 
	}

  // Sort vector array's
  sortConductorInfo(conductorInfo_Vector);
  
  // Create an instance of the pole info object and add it to the vector
  appendPoleInfo(objSelPole, ptSelPole, conductorInfo_Vector, poleInfo_Vector);

  // Check if the source conductors are to be modified while drawing new conductors
  bool bModifySourceConductor = false;
  TCHAR pszInput[5]; 
  acedInitGet(NULL, L"Yes No");
  int iRet = acedGetKword(L"Modify selected conductors? [Yes/No] <N>: ", pszInput);
  /**/ if (iRet == RTCAN) return;
  else if (!_tcsicmp(pszInput, _T("Yes"))) bModifySourceConductor = true;

  while (T)
  {
    // Allow user to pick insertion point
    ads_point ptNewPole, ptPrevPole; 
    if (acedGetPoint(ptSelPole, _T("\nInsertion point for new object: "), ptNewPole) == RTCAN) return;

    ads_point ptStart, ptEnd;
    double dOffset;

    AcCmColor color; color.setColor(AcCmEntityColor::kByLayer);

    // Insert the new pole
    AcDbObjectId objPrevPole = AcDbObjectId::kNull;
    AcDbObjectId objNewPole;

    int iRet = RTNORM;
    TCHAR result[10];			result[0]     = 'S';
    TCHAR prevResult[10];	prevResult[0] = 'S';

    // Create an instance of the pole information object
    CPoleInfo poleInfo;
    while (T)
    {
      // Start the transaction
      acTransactionManagerPtr()->startTransaction();
      if (iRet == RTNORM)
      {
        // Insert the new pole
				g_dPoleDia = 0.0;
				if (!csBlkName.IsEmpty())
				{
					if (!acdbTblSearch(L"BLOCK", csBlkName, FALSE)) {	UpdateBlockFromStandards(csBlkName); }
					if (!insertBlock(csBlkName, csLayName, ptNewPole, 0.0, 0.0, 0.0, 0.0, _T(""), objNewPole, true)) { acTransactionManagerPtr()->abortTransaction(); return; }
				
					// Get the geomExtents of the block so that the diameter of the block can be determined. 
					// The diameter of the block is taken to calculate the end points of the conductors on the pole.
					if (!g_dPoleDia)
					{
						AcDbExtents exBounds; if (!GetPoleExtents(objNewPole, exBounds)) { acTransactionManagerPtr()->abortTransaction(); return; }
						g_dPoleDia = fabs(exBounds.maxPoint().x - exBounds.minPoint().x);
					}
				}
					
        // Explode the newly inserted pole block
        if (objNewPole.isValid())
        {
					if (!csBlkName.CompareNoCase(L"POLE_NUMBERED_PROP"))
					{
						ads_name enNewPole; acdbGetAdsName(enNewPole, objNewPole);
						acedCommandS(RTSTR, L".EXPLODE", RTENAME, enNewPole, NULL);

						ads_name ssExplode; 
						if (acedSSGet(L"P", NULL,	NULL, NULL, ssExplode) == RTNORM)
						{
							// Get the object id of the new pole "POLE_PROP" block that results from EXPLODE
							ads_name enEntity; 
							AcDbObjectId objExplode;

							//long lLength = 0L; acedSSLength(ssExplode, &lLength);
							int lLength = 0L; acedSSLength(ssExplode, &lLength);
							for (long lCtr = 0L; lCtr < lLength; lCtr++)
							{
								if (acedSSName(ssExplode, lCtr, enEntity) != RTNORM) continue;
								if (acdbGetObjectId(objExplode, enEntity) != Acad::eOk) continue;

								// Get the block reference pointer and from it its name
								AcDbBlockReference *pInsert;
								if (acdbOpenObject(pInsert, objExplode, AcDb::kForWrite) != Acad::eOk) continue;

								// Get the object id of the block definition from the insert
								AcDbObjectId objTblRcd = pInsert->blockTableRecord();

								// Open the symbol table record for this id
								AcDbSymbolTableRecord *pBlkTblRcd = NULL;
								if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk) { pInsert->close(); continue; }
								pBlkTblRcd->close();

								AcString acsName; pBlkTblRcd->getName(acsName); 
								if (!acsName.matchNoCase(L"POLE_PROP")) { pInsert->close(); continue; }

								// Change the layer to POLE_PROP
								pInsert->setLayer(csLayName);
								pInsert->close();

								// We got the insert in it, go ahead and get its object id
								objNewPole = objExplode;
								break;
							}

							// Try and find out if the new object id is valid. Just in case the exploded information didn't have the POLE_PROP block in it :-)
							if (!objNewPole.isValid())
							{
								acutPrintf(L"\nThe \"POLE_NUMBERED_PROP\" block doesn't have \"POLE_PROP\" nested within it!"); 
								acTransactionManagerPtr()->abortTransaction(); 
								acedSSFree(ssExplode); 
								return; 
							}
						}

						// Free the selection set
						acedSSFree(ssExplode);
          }
        }
      }

      // Draw the new conductors
      AcDbObjectIdArray arObjIds;
      if (result[0] != 'U') 
      {
        // To draw new conductors from previous pole to the new pole, the information of conductors connected to the pole is passed as reference.
        // The draw function will draw the new set of conductors to the new pole and return the new set of conductor information that is appended
        // to the new pole.
        conductorInfo_Vector = poleInfo_Vector[poleInfo_Vector.size() - 1].m_conductorInfo_Vector;
        DrawConductors(bModifySourceConductor, poleInfo_Vector, ptNewPole, conductorInfo_Vector, offsetInfo_Vector, arObjIds);
        appendPoleInfo(objNewPole, ptNewPole, conductorInfo_Vector, poleInfo_Vector);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // All conductors drawn, check if the user wants to flip the side or Undo  what is done or Specify a new point or Exit the command.
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      while (T)
      {
        ads_point ptInput;
        acedInitGet(NULL, _T("Flip Undo Exit"));
        ads_point ptTempNewPoint;	
        iRet = acedGetPoint(poleInfo_Vector[poleInfo_Vector.size() - 1].m_ptInsert, _T("\nSpecify next point or [Flip/Undo/Exit] <Exit>: "), ptNewPole);

        prevResult[0] = result[0];
        if ((iRet == RTKWORD) && (acedGetInput(result) == RTNORM))
        {
          /**/ if (result[0] == _T('F')) 
          {
            // Remove the last set of conductors drawn as they are going to be drawn afresh.
            poleInfo_Vector.pop_back();

            // Reverse the conductors in the conductor information array. That should do the job of flipping.
            reverseConductorInfo(poleInfo_Vector[poleInfo_Vector.size() - 1].m_conductorInfo_Vector);

            // Undo what ever was done during the previous occasion
            acTransactionManagerPtr()->abortTransaction();

            // This to force an insert of pole again
            iRet = RTNORM; 
            break;
          }
          else if (result[0] == _T('U')) 
          {
            if ((prevResult[0] != 'U') && (poleInfo_Vector.size() > 1))
            {
              // Remove the last pole info from the vector
              poleInfo_Vector.pop_back();
              acTransactionManagerPtr()->abortTransaction();
              break;
            }

            // Nothing to undo dude!
            acutPrintf(_T("\nNothing to undo.\n"));
          }
          else if (result[0] == _T('E')) 
          {
            // End the command
            acTransactionManagerPtr()->endTransaction();

						// Join all PLINE segments to form a cable
						JoinSegments(poleInfo_Vector, conductorInfo_Vector.size());
            return;
          }
        }
        else if (iRet == RTNONE)
        {
          // End the command
          acTransactionManagerPtr()->endTransaction();

					// Join all PLINE segments to form a cable
					JoinSegments(poleInfo_Vector, conductorInfo_Vector.size());

          return;
        }
        else if (iRet == RTNORM)
        {
          // Confirm the placement of the new pole and conductors
          acTransactionManagerPtr()->endTransaction();

          prevResult[0] = result[0];
          result[0] = _T('S');
          break;
        }
        else if (iRet == RTCAN) 
        {
          acTransactionManagerPtr()->abortTransaction();
          return; 
        }
      }
    }
  }
}
