#include "StdAfx.h"
#include "joinsegments.h"

extern double gFuzz;

/////////////////////////////////////////////////////
// Function name: appendExplodedEntityToInfoArray()
// Description  :
/////////////////////////////////////////////////////
bool appendExplodedEntityToInfoArray(AcDbObjectId objPrntId, AcDbObject *pChildObj, VAsdkBasicEntityInfo &mArrEntInfo)
{
	// Get the current UCS MATRIX
	AcGeVector3d mNorm;
	AcGeMatrix3d mCurUCSMat;	
	AcGeMatrix3d mWCS2UCSMat;
	acdbUcsMatrix(mCurUCSMat);
	mWCS2UCSMat = mCurUCSMat.inverse(); // This required to get the coordinates in current UCS

	double mZValue = gFuzz;

	// Create a temporary object that we can add to the entityInfo array
	CAsdkBasicEntityInfo mTempEntInfo;

	// Get an object id from the ename supplied from the selection set
	if (!objPrntId.isValid()) { return true; }
	mTempEntInfo.m_objectId = objPrntId;

	// AcDbLine and AcDbArc are both derived from AcDbCurve so we'll get the common data first
	AcDbCurve *pCurve = AcDbCurve::cast (pChildObj);
	
	// If it is a curve then get the common data out of it
	if (pCurve != NULL)
	{
		// Get the from (start point) of the line
		pCurve->getStartPoint (mTempEntInfo.m_from);
		mTempEntInfo.m_from.transformBy(mWCS2UCSMat);	//latest changes

		// Get the to (end point) of the line
		pCurve->getEndPoint (mTempEntInfo.m_to);
		mTempEntInfo.m_to.transformBy(mWCS2UCSMat);		//latest changes
	}
	else
	{
		return false;
	}

	// See if the entity is an Arc
	AcDbArc *pArc = AcDbArc::cast (pChildObj);
	AcDbLine *pLine = AcDbLine::cast(pChildObj);

	// If it is a line or arc check for z value. 
	if ((mTempEntInfo.m_from.z == mTempEntInfo.m_to.z) )
	{
		// Get the elevation
		/**/ if (mZValue == gFuzz) 
			mZValue = mTempEntInfo.m_from.z;
		else if (mZValue != mTempEntInfo.m_from.z)
		{
			// acutPrintf(_T("\nEntity is not at the same elevation as first entity.\tObject ID : %u"), pObj->objectId().asOldId());
			return false;
		}
	}
	else
	{
		// acutPrintf(_T("\nEntity is not parallel to UCS.\tObject ID : %u"), pObj->objectId().asOldId());
		return false;
	}

	// If it is an arc then get the data out of it
	if (pArc != NULL)
	{
		// Now work out the bulge factor i.e. the angle at which the arc goes off at angle of the arc - 90 degrees
		ads_real startAngle = pArc->startAngle ();
		ads_real endAngle = pArc->endAngle ();
		ads_real angle = 0.0;

		if (startAngle > endAngle)
		{
			angle = startAngle-(2.0 * PI);
		}
		else
		{
			angle = startAngle;
		} 

		// Get the bulge value
		mTempEntInfo.m_bulge = tan ((endAngle-angle) / 4.0);

		// See which direction the arc is going in relative to the zAxis
		if (Adesk::kFalse == pArc->normal().transformBy(mWCS2UCSMat).isCodirectionalTo(AcGeVector3d::kZAxis))
		{
			// If not isCodirectionalTo the zaxis then minus the bulge
			mTempEntInfo.m_bulge = -mTempEntInfo.m_bulge;
		}
	}			

	// Now add the data to the array for scanning in a minute
	mArrEntInfo.push_back (mTempEntInfo);
	
	return true;
}

/////////////////////////////////////////////////////
// Function name: appendToEntityInfoarray()
// Description  :
/////////////////////////////////////////////////////
bool appendToEntityInfoarray(AcDbObjectId objId, VAsdkBasicEntityInfo &mArrEntInfo)
{
	// Get the current UCS MATRIX
	AcGeVector3d mNorm;
	AcGeMatrix3d mCurUCSMat;	
	AcGeMatrix3d mWCS2UCSMat;
	acdbUcsMatrix(mCurUCSMat);
	mWCS2UCSMat = mCurUCSMat.inverse(); // This required to get the coordinates in current UCS

	double mZValue = gFuzz;

	// Create a temporary object that we can add to the entityInfo array
	CAsdkBasicEntityInfo mTempEntInfo;

	// Get an object id from the ename supplied from the selection set
	if (!objId.isValid()) { return true; }
	mTempEntInfo.m_objectId = objId;
	
	// Open the object because we want to extract some info out of it like the start and end points etc
	AcDbObject *pObj = NULL;
	Acad::ErrorStatus es = acdbOpenObject (pObj, mTempEntInfo.m_objectId, AcDb::kForRead);

	// If it opened ok
	if (es != Acad::eOk) { return false; }

	// Take extra care and make sure we have a valid pointer
	if (pObj == NULL) return false;

	// AcDbLine and AcDbArc are both derived from AcDbCurve so we'll get the common data first
	AcDbCurve *pCurve = AcDbCurve::cast (pObj);
	if (pObj->isKindOf(AcDbPolyline::desc()) || pObj->isKindOf(AcDb2dPolyline::desc())) 
	{
		// Close the parent POLYLINE
		pObj->close();

		//////////////////////////////////////////////////////////////////////////
		// KMK: IF THE CURVE IS A LWPOLYLINE OR AN OLD POLYLINE, WE MUST PROCESS
		//			THEIR SEGMENTS. HENCE THE BELOW CODE.
		//////////////////////////////////////////////////////////////////////////

		// Get the segment info for this object
		AcDbVoidPtrArray ar_Explodes; 
		es = pCurve->explode(ar_Explodes);
		
		// For each entity in the explode, process the entity information
		AcDbEntity *pEntity;
		AcDbObjectId objEntity;
		for (int iCtr = 0; iCtr < ar_Explodes.length(); iCtr++)
		{
			// Get the object ID of the segment
			pEntity = (AcDbEntity *) ar_Explodes.at(iCtr);

			// Add the segment information to the entity information array.
			// NOTE: Here the parent object ID is appended along with entity segment information. So that the parent gets deleted once a segment in it is joined.
			appendExplodedEntityToInfoArray(objId, pEntity, mArrEntInfo);

			// Close the entity
			pEntity->close();
		}

		return true;
	}

	// If it is a curve then get the common data out of it
	if (pCurve != NULL)
	{
		// Get the from (start point) of the line
		pCurve->getStartPoint (mTempEntInfo.m_from);
		mTempEntInfo.m_from.transformBy(mWCS2UCSMat);	//latest changes

		// Get the to (end point) of the line
		pCurve->getEndPoint (mTempEntInfo.m_to);
		mTempEntInfo.m_to.transformBy(mWCS2UCSMat);		//latest changes
	}
	else
	{
		pObj->close();
		return false;
	}

	// See if the entity is an Arc
	AcDbArc *pArc = AcDbArc::cast (pObj);
	AcDbLine *pLine = AcDbLine::cast(pObj);

	// If it is a line or arc check for z value. 
	if ((mTempEntInfo.m_from.z == mTempEntInfo.m_to.z) )
	{
		// Get the elevation
		/**/ if (mZValue == gFuzz) 
			mZValue = mTempEntInfo.m_from.z;
		else if (mZValue != mTempEntInfo.m_from.z)
		{
			// acutPrintf(_T("\nEntity is not at the same elevation as first entity.\tObject ID : %u"), pObj->objectId().asOldId());
			pObj->close();
			return false;
		}
	}
	else
	{
		// acutPrintf(_T("\nEntity is not parallel to UCS.\tObject ID : %u"), pObj->objectId().asOldId());
		pObj->close();
		return false;
	}

	// If it is an arc then get the data out of it
	if (pArc != NULL)
	{
		// Now work out the bulge factor i.e. the angle at which the arc goes off at angle of the arc - 90 degrees
		ads_real startAngle = pArc->startAngle ();
		ads_real endAngle = pArc->endAngle ();
		ads_real angle = 0.0;

		if (startAngle > endAngle)
		{
			angle = startAngle-(2.0 * PI);
		}
		else
		{
			angle = startAngle;
		} 

		// Get the bulge value
		mTempEntInfo.m_bulge = tan ((endAngle-angle) / 4.0);

		// See which direction the arc is going in relative to the zAxis
		if (Adesk::kFalse == pArc->normal().transformBy(mWCS2UCSMat).isCodirectionalTo(AcGeVector3d::kZAxis))
		{
			// If not isCodirectionalTo the zaxis then minus the bulge
			mTempEntInfo.m_bulge = -mTempEntInfo.m_bulge;
		}
	}			

	// Now add the data to the array for scanning in a minute
	mArrEntInfo.push_back (mTempEntInfo);

	// close the entity
	pObj->close();

	return true;
}	

//////////////////////////////////////////////////////////////////////////
// Function name: GatherSelectionSetDetails()
// Description  : Gets the start and end point into a vector array
//////////////////////////////////////////////////////////////////////////
Adesk::Boolean GatherSelectionSetDetails (VAsdkBasicEntityInfo &mArrEntInfo, ads_name ss)
{
	//long mLength = 0L;
	int mLength = 0L;
	
	// Get the number of entities in the selection set
	if (acedSSLength (ss, &mLength) != RTNORM) return Adesk::kFalse;
	
	// Loop the selection set extracting the entities out of the selection set and processing them
	AcDbObjectId objId;
	Acad::ErrorStatus es;

	for (long i = 0L; i < mLength; i++)
	{
		// Extract the ith ename from the selection set
		ads_name ename;	if (acedSSName(ss, i, ename) != RTNORM) continue;
		acdbGetObjectId(objId, ename);
		if (!objId.isValid()) continue;
		
		appendToEntityInfoarray(objId, mArrEntInfo);
	}

	return Adesk::kTrue;
}

//////////////////////////////////////////////////////////////////////////
// Function name: DeleteEntsInSS()
// Description  : Loops a selection set and erases entities within.
//////////////////////////////////////////////////////////////////////////
Adesk::Boolean DeleteEntsInSS (ads_name &ssSel, VAsdkBasicEntityInfo mArrEntityInfo)
{
	//long lLength;
	int lLength;
	bool mNotinArrFlag;
	AcDbObjectId objId;
	CAsdkBasicEntityInfo mEntInfo;
	ads_name enEntity;

	// Loop through and erase only those entities that have been converted to pline 
	acedSSLength(ssSel, &lLength);
	for (long lCnt = 0; lCnt < lLength; lCnt++)
	{
		if (acedSSName(ssSel, lCnt, enEntity) == RTNORM)
		{
			acdbGetObjectId(objId, enEntity);
			mNotinArrFlag = true;

			for (long lCtr = 0; lCtr < mArrEntityInfo.size(); lCtr++)
			{
				mEntInfo = mArrEntityInfo.at(lCtr);

				// If entity is present in the array, that means it is not a part of PLINE and hence do not delete
				if (mEntInfo.m_objectId == objId) mNotinArrFlag = false;
			}

			if (mNotinArrFlag) acdbEntDel(enEntity);
		}
	}

	return (Adesk::kTrue);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CreatePline
// Description  : 
//////////////////////////////////////////////////////////////////////////
Adesk::Boolean CreatePline(VAsdkBasicEntityInfo &mArrEntInfo, CString csLayerName)
{
	// Create the 'will be' sorted vector array of entity information
	CAsdkBasicEntityInfo mEntInfo;
	long mCtr = 0;
	long mFoundAt;
	long mVertexCtr = 0;
	const double mZeroBulg = 0.0;
	double mBulge = 0.0;

	Adesk::Boolean mRetVal;
	AcGePoint3d mSearchPt;
	AcGePoint3d mTempPt;
	AcGeMatrix3d mUCS2WCSMat;
	AcDbPolyline *pPoly = new AcDbPolyline();

	if (0 == mArrEntInfo.size()) return Adesk::kFalse;

	// Get the matrix to transform the pline to current UCS.
	acdbUcsMatrix(mUCS2WCSMat);

	// Try to get the first point. This means get a point which is not shared by two entities
	for (mCtr = 0; mCtr < mArrEntInfo.size(); mCtr++)
	{
		mEntInfo = mArrEntInfo.at(mCtr);

		// Test with start point first
		mSearchPt = mEntInfo.m_from;
		if (-1 == fGetNextPoint(mSearchPt, mArrEntInfo, mBulge, mCtr))
		{
			mBulge =  mEntInfo.m_bulge;
			mTempPt = mEntInfo.m_to; // This is the next point
			break;
		}

		// If not found start with end point
		mSearchPt = mEntInfo.m_to;
		if (-1 == fGetNextPoint(mSearchPt, mArrEntInfo, mBulge, mCtr))
		{
			mBulge =  -1 * mEntInfo.m_bulge;//change the bulge direction as we are going against the arc direction
			mTempPt = mEntInfo.m_from; // This is the next point
			break;
		}
	}
		
	// If the loop is closed then there will no start point. Pick the last entity and start processing
	if (mCtr == mArrEntInfo.size())
	{
		// You will reach here if the entities form a loop and are not open ended 
		--mCtr; // Pick the last entity from the list
		mEntInfo = mArrEntInfo.at(mCtr);
		mSearchPt = mEntInfo.m_from;
		mTempPt = mEntInfo.m_to;
		mBulge = mEntInfo.m_bulge;
	}
	
	// Add the first vertex which is the search point itself
	pPoly->addVertexAt(mVertexCtr++, AcGePoint2d(mSearchPt.x, mSearchPt.y), mBulge);
	mArrEntInfo.erase(mArrEntInfo.begin() + mCtr);

	mSearchPt = mTempPt;

	// Add the vertex to the pline. Search for the continuous points
	do
	{
		mFoundAt = fGetNextPoint(mSearchPt, mArrEntInfo, mBulge);

		//this means we have hit the last vertex
		if (-1 == mFoundAt )break;

		mEntInfo = mArrEntInfo.at(mFoundAt);
		pPoly->addVertexAt(mVertexCtr++, AcGePoint2d(mTempPt.x,mTempPt.y), mBulge);		

		// Temp point. Required to add correct bulge
		mTempPt = mSearchPt; 

		// Delete the entity info from array once the vertex is added
		mArrEntInfo.erase(mArrEntInfo.begin() + mFoundAt); 
	} while (true);

	// Add the last vertex and always has zero bulge
	pPoly->addVertexAt(mVertexCtr++, AcGePoint2d(mTempPt.x,mTempPt.y), mZeroBulg);

	// Set the elevation
	pPoly->setElevation(mTempPt.z);

	// Add to the database
	AcCmColor color; color.setColor(AcCmEntityColor::kByLayer);
	mRetVal = fPostToMs(pPoly,acdbHostApplicationServices()->workingDatabase());	

	// Transform it to the current UCS
	pPoly->transformBy(mUCS2WCSMat);

	pPoly->setLayer(csLayerName);
	pPoly->setColor(color);
	pPoly->setLinetype(_T("BYLAYER"));
	
	// Close the pline
	pPoly->close();

	return mRetVal;
}

//////////////////////////////////////////////////////////////////////////
// Function name: fGetNextPoint()
// Description  : Function to get next point in VAsdkBasicEntityInfo given
//                a search point.
//////////////////////////////////////////////////////////////////////////
long fGetNextPoint(AcGePoint3d &mSearchPt, const VAsdkBasicEntityInfo &mArrEntInfo, double &mBulge, long mSkip)
{
	CAsdkBasicEntityInfo mEntInfo;
	double mDist = 0.0;

	for (long mCtr = 0; mCtr < mArrEntInfo.size(); mCtr++)
	{
		if (mCtr == mSkip) continue;
		mEntInfo = mArrEntInfo.at(mCtr);

		// Check against the start point
		mDist = mSearchPt.distanceTo(mEntInfo.m_from);
		if (mDist <= gFuzz)
		{
			mSearchPt = mEntInfo.m_to;
			mBulge = mEntInfo.m_bulge;
			return mCtr;
		}

		// if it doesn't match check against the end point
		mDist = mSearchPt.distanceTo(mEntInfo.m_to);
		if (mDist <= gFuzz )
		{
			mSearchPt = mEntInfo.m_from;
			//change the bulge direction as we are going against the arc direction
			mBulge =  -1 * mEntInfo.m_bulge;
			return mCtr;
		}
	}

	return -1;
}


///////////////////////////////////////////////////////////////////////////////
// Function name: fPostToMs()
// Description  : Add entity to the model space
//////////////////////////////////////////////////////////////////////////
Adesk::Boolean fPostToMs (AcDbEntity *pEnt, AcDbDatabase *pDb)
{
	AcDbBlockTable* pBT = NULL;
	if (Acad::eOk != pDb->getBlockTable(pBT, AcDb::kForRead))	return Adesk::kFalse;

	AcDbBlockTableRecord* pBTR;
	Acad::ErrorStatus es = pBT->getAt( ACDB_MODEL_SPACE, pBTR, AcDb::kForWrite);
	pBT->close();

	if (Acad::eOk != es) return Adesk::kFalse;

	es = pBTR->appendAcDbEntity( pEnt );
	pBTR->close();

	if (es != Acad::eOk) return Adesk::kFalse; else return Adesk::kTrue;
}

/////////////////////////////////////////////////////
// Function name: ConvertToPline()
// Description  : 
/////////////////////////////////////////////////////
bool ConvertToPline(CString csLayerName, ads_name enJoinSrc, CString csLayerSuffix, CStringArray &csaApplicableLayerSuffix)
{
	// Get component layer names Prefix, Mid and Suffix (Here the Suffix will be the STATUS) 
	// TR_33_OH_ANNO_EXIST
	
	//////////////////////////////////
	// Get the prefix of the layer
	//////////////////////////////////
	CString csLayerNameWOSuffix;

	/*
	if (csLayerName.Find(_T("_EXIST")) != -1)
		 csLayerNameWOSuffix = csLayerName.Mid(0, csLayerName.Find(_T("_EXIST")));
	else if (csLayerName.Find(_T("_PROP")) != -1)
		csLayerNameWOSuffix = csLayerName.Mid(0, csLayerName.Find(_T("_PROP")));
	*/

	if (csLayerName.Find(csLayerSuffix) != -1) csLayerNameWOSuffix = csLayerName.Mid(0, csLayerName.Find(csLayerSuffix));
		
	// Make the selection set to select entities from intended layers
	// struct resbuf *rbpFilt = acutBuildList(-4, _T("<OR"), 8, csLayerNameProp, 8, csLayerNameExist, -4, _T("OR>"), 67, 0, NULL);
	// CString csLayerNameProp;  csLayerNameProp  = csLayerNameWOSuffix + _T("_PROP*");
	// CString csLayerNameExist; csLayerNameExist = csLayerNameWOSuffix + _T("_EXIST*");

	CString csLayerNameFilt;
	struct resbuf *rbpFilt = acutBuildList(67, 0, -4, _T("<OR"), NULL);
	struct resbuf *rbpTemp;
	for (int iLay = 0; iLay < csaApplicableLayerSuffix.GetSize(); iLay++)
	{
		for (rbpTemp = rbpFilt; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);
		csLayerNameFilt.Format(L"%s%s", csLayerNameWOSuffix, csaApplicableLayerSuffix.GetAt(iLay));

		rbpTemp->rbnext = acutBuildList(8, csLayerNameFilt, NULL);
	}

	for (rbpTemp = rbpFilt; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);
	rbpTemp->rbnext = acutBuildList(-4, _T("OR>"), NULL);
			
	// Get a selection set of the users choice
	ads_name mSS;
	ads_name mEX;
	if (acedSSGet (_T("X"), NULL, NULL, rbpFilt, mEX) == RTNORM)
	{
		// Check for length
		//long lLength = 0L;
		int lLength = 0L;
		acedSSLength(mEX, &lLength);
		if (lLength == 0L)
		{
			// Free the selection set 
			acedSSFree (mEX);
			acedSSFree (mSS);

			acutPrintf(L"\nThere are no valid entities to join to the selected object.");
			return false;
		}
		
		// Create a vector array which will hold all of the selected entities
		VAsdkBasicEntityInfo mArrEntInfo;

		// Delete this entity in the selection set and add it to the last and reverse the selection set
		acedSSDel(enJoinSrc, mSS);

		// Reverse the selection set 
		ads_name enSeg;
		// long lLength = 0L; acedSSLength(mEX, &lLength);
		acedSSAdd(NULL, NULL, mSS);
		acedSSAdd(enJoinSrc, mSS, mSS);	
		for (long lCtr = 0L; lCtr < lLength; lCtr++) { acedSSName(mEX, lCtr, enSeg); acedSSAdd(enSeg, mSS, mSS); }
				
		// Get the start and end point into an array so we can order it all correctly
		if (GatherSelectionSetDetails (mArrEntInfo, mSS) == Adesk::kFalse)	acutPrintf(_T("\nERROR - Failed to get selection set details."));

		// Remove the entities from the selection set that are not even considered to be fit for making pline
		DeleteEntsInSS (mSS, mArrEntInfo);	

		// Create the pline
		CreatePline(mArrEntInfo, csLayerName);

		// Remove the old entities from which we have made our pline
		DeleteEntsInSS (mSS, mArrEntInfo);		
	}
	else
	{
		// Free the selection set 
		acedSSFree (mEX);
		acedSSFree (mSS);

		acutPrintf(L"\nThere are no valid entities to join to the selected object.\n");
		return false;
	}

	// Free the selection set 
	acedSSFree (mEX);
	acedSSFree (mSS);

	acutPrintf(L"\nDone.");
	return true;
}

///////////////////////////////////////////////////////////////////////////
// Function name : Command_Join()
// Description   : Function that is called when "NJ" command is issued.
//////////////////////////////////////////////////////////////////////////
void Command_Join()
{
	switchOff();

	acutPrintf(L"\nVersion: V2.0");

	// Get the search distance from the XRecord (if any)
	double dOffset;
	struct resbuf *rbpJoin = getXRecordFromDictionary(_T("eCapture"), _T("Join Search Distance"));
	if (rbpJoin) { dOffset = rbpJoin->resval.rreal; acutRelRb(rbpJoin); } else dOffset = _tstof(g_csMaxConnGap);
	if (!dOffset) dOffset = gFuzz;
		
	/*
	// Get the fuzz distance from the user
	acedInitGet(RSG_NONEG | RSG_NOZERO, _T(""));
	
	CString csPrompt; csPrompt.Format(_T("\nSpecify search distance <%.1f>: "), dOffset);
	double dFuzz; int iRet = acedGetReal(csPrompt, &dOffset);

	if (iRet == RTCAN)  return;
	else if (iRet == RTNORM) gFuzz = dOffset;
	*/

	// Get the applicable layer names for which join has to work
	CQueryTbl tblStatus;
	CString csSQL;
	csSQL.Format(L"SELECT [fldLayerName], [fldStatus], [Default] FROM tblStatus ORDER BY [fldSequence]");
	
	if (!tblStatus.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;
	if (tblStatus.GetRows() <= 0) { appMessage(L"Applicable layer names for join not found in standard tables."); return; }
	
	CStringArray csaApplicableLayerSuffix; tblStatus.GetColumnAt(0, csaApplicableLayerSuffix);
	CStringArray csaApplicableLayerName;   tblStatus.GetColumnAt(1, csaApplicableLayerName);
	CStringArray csaDefaults; tblStatus.GetColumnAt(2, csaDefaults);

	// Coin the error message for applicable status selection
	CString csStatus; 
	CStringArray csaLayerNamesForJoin;
	for (int iLay = 0; iLay < csaApplicableLayerName.GetSize(); iLay++)
	{
		csStatus += (csStatus.IsEmpty() ? csaApplicableLayerName.GetAt(iLay) : (L"/" + csaApplicableLayerName.GetAt(iLay)));
		// if (csaDefaults.GetAt(iLay) == L"1") csaLayerNamesForJoin.Add(csaApplicableLayerSuffix.GetAt(iLay));
		csaLayerNamesForJoin.Add(csaApplicableLayerSuffix.GetAt(iLay));
	}

	CString csErrMsg; csErrMsg.Format(L"\nThe object must have %s status.", csStatus);
	
	// Allow the user to select a LINE or POLYLINE
	ads_name enJoinSrc;
	ads_point ptDummy;
	Acad::ErrorStatus es;
	AcDbObjectId objId;
	CString csLayerName;
	int iRet;
	CString csPrompt;
	
	while (T)
	{
		acedInitGet(RSG_NONEG | RSG_NOZERO, L"Distance");
		csPrompt.Format(L"\nSelect a line/polyline object or [Distance <%.2f>]: ", dOffset);
		iRet = acedEntSel(csPrompt, enJoinSrc, ptDummy);
		/**/ if ((iRet == RTCAN) || (iRet == RTERROR)) return;

		// Check for valid selection
		if (iRet == RTKWORD)
		{
			csPrompt.Format(_T("\nSpecify search distance <%.1f>: "), dOffset);
			iRet = acedGetReal(csPrompt, &dOffset);

			/**/ if (iRet == RTCAN)  return;
			else if (iRet == RTNORM) 
			{
				gFuzz = dOffset;

				// Add the new search distance to the dictionary
				rbpJoin = acutBuildList(AcDb::kDxfReal, gFuzz, NULL);
				acdbRegApp(_T("Join Search Distance"));
				addXRecordToDictionary(_T("eCapture"), _T("Join Search Distance"), rbpJoin);
				acutRelRb(rbpJoin);
			}
			continue;
		}
		else if (iRet == RTNORM)
		{
			acdbGetObjectId(objId, enJoinSrc);

			////////////////////////////////////////////////////////////////////
			// Check if the entity selected is a LINE from MODEL SPACE
			////////////////////////////////////////////////////////////////////
			struct resbuf *rbpGet = acdbEntGet(enJoinSrc);
			int iSpace = Assoc(rbpGet, 67)->resval.rint;
			acutRelRb(rbpGet);
			if (iSpace != 0) { acutPrintf(L"\nInvalid selection! The object must be in model space."); continue; }
			
			AcDbEntity *pEntity;
			es = acdbOpenObject(pEntity, objId, AcDb::kForRead);
			if (es != Acad::eOk) continue;

			// Check if the ENTITY selected is a VALID Entity type
			if (!pEntity->isKindOf(AcDbLine::desc()) && !pEntity->isKindOf(AcDbPolyline::desc()) && !pEntity->isKindOf(AcDb2dPolyline::desc())) 
			{
				acutPrintf(L"\nInvalid selection. Select a line or polyline object.");
				pEntity->close(); continue;	
			}
			
			// Get the layer of which this entity is placed...
			csLayerName.Format(_T("%s"), pEntity->layer()); 
			csLayerName = csLayerName.MakeUpper();
			pEntity->close();

			//////////////////////////////////////////////////////////////////////////
			// The status of the layer must be PROP or EXIST
			//////////////////////////////////////////////////////////////////////////
			// Mail 14.07.08: Can we extend the current “EA_JOIN” function to allow any lines to be connected; ie cables on cadastre, abandoned lines etc
			// if ((csLayerName.Find(_T("_ABAND")) != -1) || ((csLayerName.Find(_T("_EXIST")) == -1) && (csLayerName.Find(_T("_PROP")) == -1)))

			// Check if the layer retrieved from the entity is on applicable layer types
			bool bValidLayer = false;
			CString csLayerSuffix = L"";
			for (int iLay = 0; iLay < csaApplicableLayerSuffix.GetSize(); iLay++)
			{
				if (csLayerName.Find(csaApplicableLayerSuffix.GetAt(iLay)) != -1) 
				{
					bValidLayer   = true; 
					csLayerSuffix = csaApplicableLayerSuffix.GetAt(iLay); 
					break; 
				}
			}
			if (!bValidLayer) { acutPrintf(csErrMsg); continue; }

			/*
			if ((csLayerName.Find(_T("_ABAND")) == -1) && ((csLayerName.Find(_T("_EXIST")) == -1) && (csLayerName.Find(_T("_PROP")) == -1)))
			{
				acutPrintf(_T("\nInvalid selection! The object must have PROPOSED/EXISTING/ABANDONED status.\n"));
				continue;
			}
			*/

			// Command is to request selection of a line or polyline then look for nearby lines with color and line type set “By Level” and with the 
			// USE, VOLTAGE & INSTALLATION components of the Layer Name the same and with the STATUS component of EXIST or PROP. 
			// e.g A HV_OH_EXIST can connect to a HV_OH_PROP line but can not connect to a HV_OH_ABAND line
			ConvertToPline(csLayerName, enJoinSrc, csLayerSuffix, csaLayerNamesForJoin);
			acutPrintf(_T("Select another or ENTER to exit...\n"));
		}
	}
}

