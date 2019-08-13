////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : JoinSegments.cpp
// Created          : Unknown
// Created by       : Autodesk
// Description      : Joins lines whose endpoints are within a specified distance into a single polyline
//
// Modification     : S. Jaisimha, 13-02-2008, to make this layer specific
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "JoinSegments.h"

//////////////////////////////////////////////////////////////////////////
// Function name: 
// Description  : Gets the start and end point into a vector array
//////////////////////////////////////////////////////////////////////////
Adesk::Boolean GatherSelectionSetDetails (VAsdkBasicEntityInfo &mArrEntInfo, ads_name ss)
{
	//long mLength = 0l;
	int mLength = 0l;
	double mZValue = gFuzz;

	// Get the number of entities in the selection set
	if (acedSSLength (ss, &mLength) != RTNORM) return Adesk::kFalse;

	// Get the current UCS MATRIX
	AcGeVector3d mNorm;
	AcGeMatrix3d mCurUCSMat;	
	AcGeMatrix3d mWCS2UCSMat;
	acdbUcsMatrix(mCurUCSMat);
	mWCS2UCSMat = mCurUCSMat.inverse(); // This required to get the coordinates in current UCS
	
	// Loop the selection set extracting the entities out of the selection set and processing them
	for (long i= 0l; i< mLength; ++i)
	{
		// Create a temporary object that we can add to the entityInfo array
		CAsdkBasicEntityInfo mTempEntInfo;
		
		// Extract the ith ename from the selection set
		ads_name ename;	if (acedSSName (ss, i, ename) != RTNORM) continue;

		// Get an object id from the ename supplied from the selection set
		acdbGetObjectId (mTempEntInfo.m_objectId, ename);
		
		// Open the object because we want to extract some info out of it like the start and end points etc
		AcDbObject *pObj = NULL;
		Acad::ErrorStatus es = acdbOpenObject (pObj, mTempEntInfo.m_objectId, AcDb::kForRead);

		// If it opened ok
		if (es != Acad::eOk) continue;
		
		// Take extra care and make sure we have a valid pointer
		if (pObj == NULL) continue;
			
		// AcDbLine and AcDbArc are both derived from AcDbCurve so we'll get the common data first
		if (!pObj->isKindOf(AcDbLine::desc())) { pObj->close(); continue; }
		AcDbCurve *pCurve = AcDbCurve::cast (pObj);

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
			continue;
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
				continue;
			}
		}
		else
		{
			// acutPrintf(_T("\nEntity is not parallel to UCS.\tObject ID : %u"), pObj->objectId().asOldId());
			pObj->close();
			continue;
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
		pObj->close ();
	}	

	return Adesk::kTrue;
}

//////////////////////////////////////////////////////////////////////////
// Function name:
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

/////////////////////////////////////////////////////////////////////////////////////////
// Function name:
// Description  : Checks the end points of all entities and aligns the selections set
//                so they all run start to end to start to end etc
//////////////////////////////////////////////////////////////////////////
Adesk::Boolean CreatePline (VAsdkBasicEntityInfo &mArrEntInfo, CString csLayer)
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
	for (mCtr = 0; mCtr < mArrEntInfo.size();mCtr++)
	{
		mEntInfo = mArrEntInfo.at(mCtr);

		// Test with start point first
		mSearchPt = mEntInfo.m_from;
		if (-1 == fGetNextPoint(mSearchPt,mArrEntInfo,mBulge,mCtr))
		{
			mBulge =  mEntInfo.m_bulge;
			mTempPt = mEntInfo.m_to;//this is the next point
			break;
		}

		// If not found start with end point
		mSearchPt = mEntInfo.m_to;
		if (-1 == fGetNextPoint(mSearchPt,mArrEntInfo,mBulge,mCtr))
		{
			mBulge =  -1 * mEntInfo.m_bulge;//change the bulge direction as we are going against the arc direction
			mTempPt = mEntInfo.m_from;//this is the next point
			break;
		}
	}

	// If the loop is closed then there will no start point. Pick the last entity and start processing
	if (mCtr == mArrEntInfo.size())
	{
		// You will reach here if the entities form a loop and are not open ended 
		--mCtr; //pick the last entity from the list
		mEntInfo = mArrEntInfo.at(mCtr);
		mSearchPt = mEntInfo.m_from;
		mTempPt = mEntInfo.m_to;
		mBulge = mEntInfo.m_bulge;
	}

	// Add the first vertex
	pPoly->addVertexAt(mVertexCtr++,AcGePoint2d(mSearchPt.x,mSearchPt.y),mBulge);

	mArrEntInfo.erase(mArrEntInfo.begin() + mCtr);
	mSearchPt = mTempPt;

	// Add the vertices to the pline. Search for the continuous points
	do
	{
		mFoundAt = fGetNextPoint(mSearchPt,mArrEntInfo,mBulge);

		//this means we have hit the last vertex
		if (-1 == mFoundAt )break;

		mEntInfo = mArrEntInfo.at(mFoundAt);
		pPoly->addVertexAt(mVertexCtr++,AcGePoint2d(mTempPt.x,mTempPt.y),mBulge);		

		// Temp point. Required to add correct bulge
		mTempPt = mSearchPt; 

		// Delete the entity info from array once the vertex is added
		mArrEntInfo.erase(mArrEntInfo.begin() + mFoundAt); 
	} while(true);

	// Add the last vertex and always has zero bulge
	pPoly->addVertexAt(mVertexCtr++,AcGePoint2d(mTempPt.x,mTempPt.y),mZeroBulg);

	// Set the elevation
	pPoly->setElevation(mTempPt.z);

	// Add to the database
	mRetVal = fPostToMs(pPoly,acdbHostApplicationServices()->workingDatabase());	

	// Transform it to the current UCS
  pPoly->transformBy(mUCS2WCSMat);

	// Change the color
	// pPoly->setColorIndex(1);

  // Change the layer
  pPoly->setLayer(csLayer);

	// Close the pline
	pPoly->close();

	return mRetVal;
}

//////////////////////////////////////////////////////////////////////////
// Function name: 
// Description  : Function to get next point in VAsdkBasicEntityInfo given
//                a search point.
//////////////////////////////////////////////////////////////////////////
long fGetNextPoint(AcGePoint3d &mSearchPt, const VAsdkBasicEntityInfo &mArrEntInfo, double &mBulge, long mSkip)
{
	CAsdkBasicEntityInfo mEntInfo;
	double mDist = 0.0;
	for(long mCtr =0;mCtr < mArrEntInfo.size(); mCtr++ )
	{
		if (mCtr == mSkip) continue;
		mEntInfo = mArrEntInfo.at(mCtr);

		//check against the start point
		mDist = mSearchPt.distanceTo(mEntInfo.m_from);
		if (mDist <= gFuzz )
		{
			mSearchPt = mEntInfo.m_to;
			mBulge = mEntInfo.m_bulge;
			return mCtr;
		}

		//if it doesn't match check against the end point
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
// Function name: 
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

///////////////////////////////////////////////////////////////////////////
// Function name :
// Description   :
//////////////////////////////////////////////////////////////////////////
void SnapLineEndPoints()
{
  // Set the possible prefixes
  CStringArray csaPattern;
  csaPattern.Add(_T("AUX*"));
  csaPattern.Add(_T("HV*"));
  csaPattern.Add(_T("LV*"));
  csaPattern.Add(_T("SV*"));
  csaPattern.Add(_T("SL*"));
  csaPattern.Add(_T("TR*"));

  // Set the fuzz factor (gFuzz is declared in the .h file)
  gFuzz = _tstof(g_csMaxConnGap);

  // Build the list of layers in the drawing
  ACHAR *pLayerName;
  CStringArray csaLayers;
  AcDbLayerTableRecord *pLayerTblRecord;
  AcDbLayerTable *pLayerTbl; acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForWrite);
  AcDbLayerTableIterator *pLayerIterator; pLayerTbl->newIterator(pLayerIterator); pLayerTbl->close();
  for (pLayerIterator->start(); !pLayerIterator->done(); pLayerIterator->step())
  {  
    // Get the layer name, color, line type and line weight settings
    pLayerIterator->getRecord(pLayerTblRecord, AcDb::kForRead);
    pLayerTblRecord->getName(pLayerName);
    pLayerTblRecord->close();
    csaLayers.Add(pLayerName);
  }
  delete pLayerIterator;

  ads_name mSS;
  resbuf *rFilter = NULL;

  // For each pattern defined in the array
  for (int iCtr = 0; iCtr < csaPattern.GetSize(); iCtr++)
  {
    // For each layer in the drawing
    for (int iLayer = 0; iLayer < csaLayers.GetSize(); iLayer++)
    {
      // If the layer matches the pattern
      if (WildMatch(csaPattern.GetAt(iCtr), csaLayers.GetAt(iLayer)) == true)
      {
        // Build the selection set of lines on this layer
        rFilter = acutBuildList(RTDXF0, _T("LINE"), 8, csaLayers.GetAt(iLayer), NULL);
	      if (acedSSGet(_T("X"), NULL, NULL, rFilter, mSS) == RTNORM)
	      {
		      while (T)
		      {
			      // Create a vector array which will hold all of the selected entities
			      VAsdkBasicEntityInfo mArrEntInfo;

			      // Get the start and end point into an array so we can order it all correctly
			      if (GatherSelectionSetDetails (mArrEntInfo, mSS) == Adesk::kFalse)	acutPrintf(_T("\nERROR - Failed to get selection set details."));
      			
			      // Remove the entities from the selection set that are not even considered to be fit for making pline
			      DeleteEntsInSS (mSS, mArrEntInfo);	

			      // Create the pline
			      CreatePline(mArrEntInfo, csaLayers.GetAt(iLayer));

			      // Remove the old entities from which we have made our pline
			      DeleteEntsInSS (mSS, mArrEntInfo);		

			      // Exit the while if no more entities are to be processed
			      if (mArrEntInfo.size() <= 0) break;
		      }

		      // Free the selection set 
		      acedSSFree (mSS);
          acutRelRb(rFilter);
	      }
      }
    }
  }
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
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
// Function name: 
// Description  :
// Arguments    :
// Called by    :
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
	if (pObj->isKindOf(AcDbPolyline::desc()) || pObj->isKindOf(AcDb2dPolyline::desc()) || pObj->isKindOf(AcDbSpline::desc())) 
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
// Function name: 
// Description  : Gets the start and end point into a vector array
//////////////////////////////////////////////////////////////////////////
Adesk::Boolean GatherSelectionSetDetailsKMK(VAsdkBasicEntityInfo &mArrEntInfo, ads_name ss)
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

///////////////////////////////////////////////////////////////////////////
// Function name :
// Description   :
//////////////////////////////////////////////////////////////////////////
int CreateHatchBoundary(AcDbObjectIdArray &aryObjIds)
{
	// If there are no entities in the array, exit the function 
	if (aryObjIds.logicalLength() <= 1) return 0;

	// Set the fuzz factor (gFuzz is declared in the .h file)
	gFuzz = 0.0;

	// Create a selection set of these entities
	ads_name enName;
	ads_name mSS; acedSSAdd(NULL, NULL, mSS);

	for (int iObj = 0; iObj < aryObjIds.logicalLength(); iObj++)
	{
		acdbGetAdsName(enName, aryObjIds.at(iObj));
		acedSSAdd(enName, mSS, mSS);
	}
	
	ads_name enLast;
	while (T)
	{
		// Create a vector array which will hold all of the selected entities
		VAsdkBasicEntityInfo mArrEntInfo;

		// Get the start and end point into an array so we can order it all correctly
		if (GatherSelectionSetDetailsKMK(mArrEntInfo, mSS) == Adesk::kFalse) { acutPrintf(_T("\nERROR - Failed to get selection set details.")); return FALSE; }

		// Remove the entities from the selection set that are not even considered to be fit for making pline
		DeleteEntsInSS (mSS, mArrEntInfo);	

		// Create the pline
		CreatePline(mArrEntInfo, L"0");

		// Remove the old entities from which we have made our pline
		DeleteEntsInSS (mSS, mArrEntInfo);		

		// Exit the while if no more entities are to be processed
		if (mArrEntInfo.size() <= 0) 
		{
			// Get the last drawn entity
			aryObjIds.removeAll();
			
			ads_name enPline; acdbEntLast(enPline);
			AcDbObjectId objPline; acdbGetObjectId(objPline, enPline);
			aryObjIds.append(objPline);

			return 1;
		}
		else return -1;
	}
}

