#include "StdAfx.h"
#include "LayoutInfo.h"
#include "LegendInfo.h"
#include "LegendNotificationDlg.h"


extern AcGeVector3d ucsToWcs(AcGeVector3d& vec);
const double gPI   = 4 * atan(1.0);  // best way to get accurate value of pi

extern CString g_csBOMLayer;


double g_dPoleDia = 0.0;		
CStringArray g_csaInvalidLegendBlock;

extern void deleteArray					(AcDbObjectIdArray objectIds);
extern void deleteArray					(AcDbVoidPtrArray entities);

extern void sortLayoutInfo      (std::vector <CLayoutInfo> &layoutInfo);
void GetLayoutNamesInPaperSpace	(std::vector <CLayoutInfo> &layoutInfoVector);
void CreateEATextStyle					(double &dHeight);
extern int getVPortClipHandles	(CString csLayout, AcDbObjectIdArray &aryClipObjIds, AcDbObjectIdArray &aryVPObjIds);
//////////////////////////////////////////////////////////////////////////
// Function name: CheckComponentInLayer
// Description  : Says if the layer name has the specified component in it.
//////////////////////////////////////////////////////////////////////////
bool CheckComponentInLayer(CString csLayerName, CString csCmp)
{
	if (csLayerName.Find(_T("_")) == -1) return false;

	// Check if the layer name has more than the number of characters in specified component.
	if (csLayerName.GetLength() < csCmp.GetLength() + 1) return false;

	// Check if the last few characters are specified component
	if (csLayerName.Find(csCmp) == -1) 
	{
		// Check if it has specified component in b/w the layer name
		if (csLayerName.Find(csCmp + _T("_")) == -1) { acutPrintf(_T("\nFailed [%s][%s]"), csCmp, csLayerName); return false; }
	}

	return true;
}







//////////////////////////////////////////////////////////////////////////
// Functions specific to EA_LEGEND
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PlaceThisLegendInfo(int bStatus, AcDbObjectIdArray &objIdArray, ads_point ptLegend, double dXDelta, CLegendInfo legendInfo)
{
	ads_point ptDescription; acutPolar(ptLegend, 0.0, 30.0, ptDescription);
	ptLegend[X] += dXDelta;

	// Determine the type of the object
	if (!legendInfo.m_csType.CompareNoCase(_T("LINE")))
	{
		// Draw a line and set its linetype
		AcDbLine *pLine = new AcDbLine(AcGePoint3d(ptLegend[X] - 2.5, ptLegend[Y], 0.0), AcGePoint3d(ptLegend[X] + 2.5, ptLegend[Y], 0.0));

		// Get the layer on which the original is placed as this depends on the status
		CString csLayer;
		/**/ if (bStatus == PROPOSED) 
			csLayer = legendInfo.m_csPropLayer;
		else if (bStatus == EXISTING)
			csLayer = legendInfo.m_csExistLayer;
		else
			csLayer = legendInfo.m_csOtherLayer;

		// Get the linetype & color from this layer.
		CString csLType;
		Adesk::Int16 iColor;
		double dLineWt;

		if (GetLTypeAndColorFromLayer(csLayer, csLType, iColor, dLineWt))
		{
			pLine->setLinetype(csLType);
			pLine->setColorIndex(iColor);
			pLine->setLineWeight(AcDb::LineWeight(int (dLineWt)));
			pLine->setLinetypeScale(0.3);
		}

		appendEntityToDatabase(pLine, false);
		objIdArray.append(pLine->objectId());
		pLine->close();
	}
	else if (!legendInfo.m_csType.CompareNoCase(_T("INSERT")))
	{
		// Insert the block
		AcDbObjectId objInsert;
		CString csName = legendInfo.m_csObject;
		/**/ if ((bStatus == PROPOSED) && (csName.Find(L"_PROP") == -1)) csName = csName + L"_PROP";
		else if ((bStatus == EXISTING) && (csName.Find(L"_PROP") != -1)) csName = csName.Mid(0, csName.Find(L"_PROP")); 

		// Get the geometry extents of the INSERT
		insertBlock(csName, _T("XT_BASE_SHEET_LEGEND_EXIST"), ptLegend, 0.0, 0.0, 0.0, 0.0, "", objInsert, false); 
				
		// Open the block reference and get its attribute value, insertion point, geom extents and rotation angle
		AcDbBlockReference *pInsert;
		if (acdbOpenObject(pInsert, objInsert, AcDb::kForWrite) == Acad::eOk)
		{
			// Get the width and height of the insert
			AcGeScale3d acgeScale = pInsert->scaleFactors();
			AcDbExtents extents; 		pInsert->getGeomExtents(extents);
			double dHeight = (extents.maxPoint().y - extents.minPoint().y);

			if (dHeight > 4.0)
			{
				acgeScale.set(ceil(4.0 / dHeight), ceil(4.0/dHeight), 1);
				pInsert->setScaleFactors(acgeScale);
			}

			pInsert->close();
		}

		objIdArray.append(objInsert);
	}

	// Place the description of the object if unmatched
	if (!legendInfo.m_bIsDescPlcd)
	{
		// If the description has "PROP" at the end, remove it before placement
		AcDbText *pText = new AcDbText(AcGePoint3d(ptDescription[X], ptDescription[Y], 0.0), legendInfo.m_csDescription, AcDbObjectId::kNull, 1.5, 0.0);
		appendEntityToDatabase(pText, false);
		objIdArray.append(pText->objectId());
		pText->close();
	}
}







//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
int GetLayouts(CStringArray &csaLayouts, CStringArray &csaVports)
{
	// Get the block table of the drawing and create a new iterator
	AcDbBlockTable *pBT; acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBT, AcDb::kForRead);
	AcDbBlockTableIterator* pIter; pBT->newIterator(pIter);

	// Loop through the iterator
	int iRow = 0;
	for( ; !pIter->done(); pIter->step())
	{
		// Get the block table record
		AcDbBlockTableRecord* pBTR; pIter->getRecord(pBTR, AcDb::kForRead);

		// If this is a layout
		if (pBTR->isLayout())
		{
			// Get the layout's object ID and from there its name
			AcDbObjectId layoutId = pBTR->getLayoutId();
			AcDbLayout *pLayout; acdbOpenAcDbObject((AcDbObject*&)pLayout, layoutId, AcDb::kForRead);
			ACHAR *pLayoutName; pLayout->getLayoutName(pLayoutName);
			CString csLayoutName = pLayoutName;
			pLayout->close();

			// If this is not the "Model" layout
			if (csLayoutName != _T("Model"))
			{
				int iEnt = 0;

				// Create a new iterator for this record
				AcDbBlockTableRecordIterator* pBtblrIter; pBTR->newIterator(pBtblrIter);

				// Just loop through it and count the number of steps
				for( ; !pBtblrIter->done(); pBtblrIter->step()) iEnt++;

				// Delete the record iterator
				delete pBtblrIter;

				// If the count is 0, then the layout is not initialized
				if (iEnt != 0) { csaLayouts.Add(csLayoutName); csaVports.Add(L""); }
			}
		}

		// Close the block table record
		pBTR->close();
	}

	// Close the block table
	pBT->close();

	// Delete the iterator
	delete pIter;

	return csaLayouts.GetSize();
} // csaVports


//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
double GetOffsetFromPoleCenter(AcDbEntity *pEntity, ads_point ptPole, double &dRefAngle /*, AcGeVector3d *pVector*/)
{
	double dOffset = -99999.9;
	// pVector->set(0.0, 0.0, 0.0);

	// Cast the entity to a curve
	AcDbCurve *pCurve = (AcDbCurve *) pEntity;
	if (!pCurve) return dOffset;

	// If the entity is a LWPolyLine, temporarily create a LINE entity. What to do
	// the getClosestPoint() is not working with extend flag for LWPOLYLINE.
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
// Functions specific to EA_EXTENDPOLEANDCONDCUTORS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Function name: selectPole()
// Description  : Enables selection of valid pole block.
// Arguments    : 1. AcDbObjectId &, Object ID of the selected pole.
//  				    : 2. ads_point,			 Insertion point of the pole.
//////////////////////////////////////////////////////////////////////////
bool selectPole(AcDbObjectId &objPole, ads_point ptPole)
{
	int iRet;
	ads_name enPole;
	ads_point ptDummy;
	Acad::ErrorStatus es;
	AcDbBlockReference *pInsert = NULL;
	while (TRUE)
	{
		iRet = acedEntSel(_T("\n\rSelect a pole: "), enPole, ptDummy);
		/**/ if (iRet == RTCAN) return false;
		else if (iRet == RTNORM) 
		{
			// Check if the entity selected is a valid pole
			acdbGetObjectId(objPole, enPole);

			es = acdbOpenObject(pInsert, objPole, AcDb::kForRead);
			if ((es != Acad::eOk) && (es != Acad::eNotThatKindOfClass)) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); return false; }

			// Check if the selection is an insert
			if (pInsert == NULL) { acutPrintf(_T("\nNot a valid pole.\n")); continue; }

			// Get the insertion point
			ptPole[X] = pInsert->position().x; ptPole[Y] = pInsert->position().y;	ptPole[Z] = 0.0;

			// Close the insert as it found an insert
			pInsert->close();
			return true;
		}
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
//////////////////////////////////////////////////////////////////////////

Acad::ErrorStatus setVar(LPCTSTR varName, const resbuf* newVal)
{
	CString str;
	if (acedSetVar(varName, newVal) != RTNORM) 
	{
		str.Format(_T("Could not set system variable \"%s\"."), varName);
		return Acad::eInvalidInput;
	}
	else
		return Acad::eOk;
}

Acad::ErrorStatus setSysVar(LPCTSTR varName, int val)
{
	ASSERT(varName != NULL);

	resbuf rb;
	rb.restype = RTSHORT;
	rb.resval.rint = val;

	return(setVar(varName, &rb));
}

Acad::ErrorStatus setSysVar(LPCTSTR varName, double val)
{
	ASSERT(varName != NULL);

	resbuf rb;
	rb.restype = RTREAL;
	rb.resval.rreal = val;

	return(setVar(varName, &rb));
}

Acad::ErrorStatus setSysVar(LPCTSTR varName, const TCHAR* val)
{
	ASSERT(varName != NULL);

	resbuf rb;
	rb.restype = RTSTR;
	rb.resval.rstring = const_cast<TCHAR*>(val);

	return(setVar(varName, &rb));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name : ApplyTextStyle
// DEscription   : Modifies the "Standard" text style based on the inputs given
////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ApplyTextStyle(double dTextHeight)
{
	// Get the object id of "Standard" style from symbol table
	AcDbTextStyleTable *pTxtStyleTbl;
	AcDbObjectId objStdTxtId;
	if (acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pTxtStyleTbl, AcDb::kForRead) != Acad::eOk) return false;
	if (pTxtStyleTbl->getAt(L"eCapture_Data", objStdTxtId) != Acad::eOk) { pTxtStyleTbl->close(); return false; }
	pTxtStyleTbl->close();
		
	// The above would have changed all the texts written using "eCapture" style. But if there are TEXTS/MTEXTS written in our styles, they wouldn't be affected. 
	// Hence to modify them, we will have to modify their data individually.
	ads_name ssGet;
	ads_name enGet;
	AcDbObjectId objId;
	struct resbuf *rbpFilt = acutBuildList(-4, L"<OR", RTDXF0, L"TEXT", RTDXF0, L"MTEXT", -4, L"OR>", NULL);
	if (acedSSGet(L"P", NULL, NULL, rbpFilt, ssGet) == RTNORM)
	{
		long lLength = 0L; acedSSLength(ssGet, &lLength);
		for (long lCtr = 0L; lCtr < lLength; lCtr++)
		{
			// Get the object id of the entity
			acedSSName(ssGet, lCtr, enGet);
			acdbGetObjectId(objId, enGet);

			// Open the entity
			AcDbText *pText;
			AcDbMText *pMText;
			if (acdbOpenObject(pMText, objId, AcDb::kForWrite) == Acad::eOk)
			{
				//////////////////////////////////////////////////////////////////////////
				// MText
				//////////////////////////////////////////////////////////////////////////
				// Modify the style to "Standard"
				pMText->setTextStyle(objStdTxtId);
				pMText->setTextHeight(dTextHeight); 

				// Modify the text value to remove any font based data like "{\\fColonna MT|b0|i0|c0|p82;sdsd}" to "sdsd"
				CString csTxtValue = pMText->contents();
				if ((csTxtValue.Find(L"{\\") != -1) && (csTxtValue.Find(L";") != -1))
				{
					csTxtValue = csTxtValue.Mid(csTxtValue.Find(L";") + 1);     // Gives me "sdsd}"
					csTxtValue = csTxtValue.Mid(0, csTxtValue.GetLength() - 1); // Gives me "sdsd"
					pMText->setContents(csTxtValue);
				}

				pMText->close();
			}
			else if (acdbOpenObject(pText, objId, AcDb::kForWrite) == Acad::eOk)
			{
				//////////////////////////////////////////////////////////////////////////
				// Text
				//////////////////////////////////////////////////////////////////////////
				// Modify the style to "Standard"
				pText->setTextStyle(objStdTxtId);
				pText->setHeight(dTextHeight);
				pText->close();
			}
		}
	}

	acedSSFree(ssGet);
	acutRelRb(rbpFilt);

	// Return a success flag
	return TRUE;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : GetBlockNamesForPageNumbering
// Description      : 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetTotalNumberOfSheets(CStringArray &csaApplicableBlockNames)
{
	int iLength = 0;

	// Make a selection set of all INSERTS in paper space.
	ads_name ssTBlocks;
	// struct resbuf *rbpFilt = acutBuildList(67, 1, RTDXF0, _T("INSERT"), 2, _T("*TITLE*"), NULL);
	struct resbuf *rbpFilt = acutBuildList(67, 1, RTDXF0, _T("INSERT"), NULL);
	if (acedSSGet(_T("X"), NULL, NULL, rbpFilt, ssTBlocks) != RTNORM)
	{
		acedSSFree(ssTBlocks);
		acutRelRb(rbpFilt);
		return iLength;
	}

	// Release the memory
	acutRelRb(rbpFilt);

	// Length
	long lLength = 0L; acedSSLength(ssTBlocks, &lLength); 
	if (lLength == 0L) { acedSSFree(ssTBlocks); return iLength; }

	ads_name enTBlock;
	AcDbObjectId objInsertId;
	AcDbObjectId objTblRcd;
	AcDbSymbolTableRecord *pBlkTblRcd = NULL;
	AcString acsName;
	CString csName;
	CString csTag;
	AcDbAttribute *pAtt;
	AcDbObjectId objAttId;

	for (long lCtr = 0L; lCtr < lLength; lCtr++)
	{
		// Get the entity name and its object Id
		acedSSName(ssTBlocks, lCtr, enTBlock);
		acdbGetObjectId(objInsertId, enTBlock);
	
		// Get the block reference pointer and from it its name
		AcDbBlockReference *pInsert;
		acdbOpenObject(pInsert, objInsertId, AcDb::kForRead);
		
		// Get the object id of the block definition from the insert
		objTblRcd = pInsert->blockTableRecord();

		// Open the symbol table record for this id
		if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk) { pInsert->close(); return iLength; }

		pBlkTblRcd->getName(acsName); 
		pBlkTblRcd->close();

		csName.Format(_T("%s"), acsName.kTCharPtr()); csName.MakeUpper(); 

		// Check if this block name is already accounted
		AcDbHandle handle;  pInsert->getAcDbHandle(handle);
		TCHAR handleStr[256]; handle.getIntoAsciiBuffer(handleStr);
		
		/*
		if (CheckForDuplication(csaApplicableBlockNames, csName)) 
		{
			iLength++; 
			pInsert->close(); 
			acutPrintf(L".......Duplicate");
			continue; 
		}*/
						
		AcDbObjectIterator *pIter = pInsert->attributeIterator();
		pInsert->close();

		// Get the attribute tag specified and change the value specified
		int iFoundAll = 0;
		for (pIter->start(); !pIter->done(); pIter->step())
		{
			objAttId = pIter->objectId();
			acdbOpenObject(pAtt, objAttId, AcDb::kForRead);
			csTag.Format(_T("%s"), pAtt->tag());

			/**/ if (!csTag.CompareNoCase(_T("NO_OF_SHEETS")) || !csTag.CompareNoCase(_T("SHEET"))) 
				iFoundAll++;
			else { pAtt->close(); continue; }

			// Check if the necessary attributes are all there. If YES, then this block counts
			if (iFoundAll == 2) 
			{
				iLength++; 
				pAtt->close(); 
				csaApplicableBlockNames.Add(csName); 
				acutPrintf(L".......Done");
				pAtt->close();
				break; 
			}
			pAtt->close();
		}

		delete pIter;
	}

	acedSSFree(ssTBlocks);
	return iLength;
}

//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
LPCTSTR ptToStr(const AcGePoint3d& pt, CString& str, int unit, int prec)
{
	TCHAR xstr[100], ystr[100], zstr[100];

	acdbRToS(pt.x, unit, prec, xstr);
	acdbRToS(pt.y, unit, prec, ystr);
	acdbRToS(pt.z, unit, prec, zstr);

	str.Format(_T("(%s, %s, %s)"), xstr, ystr, zstr);
	return str;
}

LPCTSTR vectorToStr(const AcGeVector3d& vec, CString& str, int unit, int prec)
{
	TCHAR xstr[100], ystr[100], zstr[100];

	acdbRToS(vec.x, unit, prec, xstr);
	acdbRToS(vec.y, unit, prec, ystr);
	acdbRToS(vec.z, unit, prec, zstr);

	str.Format(_T("(%s, %s, %s)"), xstr, ystr, zstr);
	return str;
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
void EA_SetLegendTagConstantToNo()
{
	// Get the block table and its iterator
	AcDbBlockTable *pBlkTbl; acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlkTbl, AcDb::kForRead);
	AcDbBlockTableIterator *pBlkIter; pBlkTbl->newIterator(pBlkIter); 
	pBlkTbl->close();

	ACHAR *pszName;
	CString csName, csHeight;
	CStringArray csaBlocks, csaStyles, csaHeights;
	AcDbBlockTableRecord *pBlkRcd;
	AcDbTextStyleTableRecord *pTxtRcd;

	// Get all the blocks defined in the drawing
	for (pBlkIter->start(); !pBlkIter->done(); pBlkIter->step())
	{
		// Get the block name
		pBlkIter->getRecord(pBlkRcd, AcDb::kForRead);
		pBlkRcd->getName(pszName);
		pBlkRcd->close();

		// We need not add special blocks
		csName = pszName;
		if ((csName[0] == _T('*')) || 
			(csName[0] == _T('$')) || 
			(csName.Mid(0, 3) == _T("A$C")) || 
			(csName.Mid(0, 6) == _T("SHEET_")) || 
			(csName.Mid(0, 5).CompareNoCase(_T("Scale")) == 0) || 
			(csName.Right(5).CompareNoCase(_T("_LOGO")) == 0) || 
			(csName.CompareNoCase(_T("North Point")) == 0) ||
			(csName.CompareNoCase(_T("VALIDATION_ERROR")) == 0)
			) continue;

		// Add the block name to the array
		csaBlocks.Add(pszName);
	}

	// Delete the iterator
	delete pBlkIter;

	// For each selected block
	Acad::ErrorStatus es;
	AcDbAttributeDefinition* pAttDef;
	AcGePoint3d geAttLoc(0.0, 0.0, 0.0);

	for (int iCtr = 0; iCtr < csaBlocks.GetSize(); iCtr++)
	{
		// Status
		acutPrintf(_T("[%d of %d] Updating %s..."), iCtr + 1, csaBlocks.GetSize(), csaBlocks.GetAt(iCtr));

		// Get this block
		es = pBlkTbl->getAt(csaBlocks.GetAt(iCtr), pBlkRcd, AcDb::kForWrite);
		if (es != Acad::eOk) { acutPrintf(_T("Error %s getting block %s\n"), acadErrorStatusText(es), csaBlocks.GetAt(iCtr)); pBlkTbl->close(); return; }

		// Remove the tag if it already exists in the selected blocks
		AcDbEntity *pEntity;
		AcDbBlockTableRecordIterator *pIter;
		if (Acad::eOk == pBlkRcd->newIterator(pIter))
		{
			for (; !pIter->done(); pIter->step())
			{
				if (Acad::eOk != pIter->getEntity(pEntity, AcDb::kForWrite)) continue;
				pAttDef = AcDbAttributeDefinition::cast(pEntity);

				if (pAttDef && !_tcsicmp(pAttDef->tag(), L"LEGEND"))
				{
					pAttDef->setConstant(Adesk::kFalse);
					pAttDef->close();
				}
				else pEntity->close();
			}

			delete pIter;
		}

		pBlkRcd->close();
	}

	// Close the block table
	pBlkTbl->close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetObjectsForBOM()
// Description  : Get the details of LINES/INSERTS that can be placed in the BOM.
// Arguments    : 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetObjectsForBOM(std::vector <CLegendInfo> &legendInfo_Vector)
{
	//////////////////////////////////////////////////////////////////////////
	// Select all inserts and lines placed in the drawing
	//////////////////////////////////////////////////////////////////////////
	ads_name ssBOM;
	struct resbuf *rbpFilt = acutBuildList(-4, _T("<OR"), RTDXF0, L"SPLINE", RTDXF0, L"ARC", RTDXF0, L"INSERT", RTDXF0, L"LWPOLYLINE", RTDXF0, L"LINE", RTDXF0, L"POLYLINE", -4, _T("OR>"), 67, 0, NULL);
		
	if (acedSSGet(_T("X"), NULL, NULL, rbpFilt, ssBOM) != RTNORM) { acedSSFree(ssBOM); acutRelRb(rbpFilt); return false; }
	acutRelRb(rbpFilt);
	
	// Process the entities in the selection set for BOM
	ads_name enEntity;
	AcDbObjectId objEntity;
	AcDbEntity *pEntity = NULL;

	Acad::ErrorStatus es;
	long lLength = 0L; acedSSLength(ssBOM, &lLength); 

	for (long lCtr = 0L; lCtr < lLength; lCtr++)
	{
		// Get the object Id
		acedSSName(ssBOM, lCtr, enEntity);
		acdbGetObjectId(objEntity, enEntity);

		// Get the entity pointer
		es = acdbOpenObject(pEntity, objEntity, AcDb::kForRead);
		if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); acedSSFree(ssBOM); return false; }

		// Call the appropriate functions to process INSERTS and LINES for legends
		if (pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbSpline::desc()) || pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc())) 
		{
			// Add the linetype to the object array. If the linetype is "BYLAYER", get its linetype name from the layer
			CString csLayerName = pEntity->layer();
			CString csLType			= pEntity->linetype(); 
			csLType = csLType.MakeUpper();

			ads_point ptDummy;
			/**/ if (pEntity->isKindOf(AcDbLine::desc()))
			{
				AcDbLine *pLine = AcDbLine::cast(pEntity);
				ptDummy[X] = pLine->startPoint().x; ptDummy[Y] = pLine->startPoint().y;	ptDummy[Z] = 0.0;
				pLine->close();
			}
			else if (pEntity->isKindOf(AcDbPolyline::desc()))
			{
				AcDbPolyline *pPline = AcDbPolyline::cast(pEntity);
				AcGePoint3d geStartPt;
				pPline->getPointAt(0, geStartPt);
				pPline->close();

				ptDummy[X] = geStartPt.x; ptDummy[Y] = geStartPt.y; ptDummy[Z] = 0.0;
			}
			else if (pEntity->isKindOf(AcDb2dPolyline::desc()))
			{
				AcDb2dPolyline *pPline = AcDb2dPolyline::cast(pEntity);
				AcGePoint3d geStartPt;
				pPline->getStartPoint(geStartPt);
				pPline->close();

				ptDummy[X] = geStartPt.x; ptDummy[Y] = geStartPt.y; ptDummy[Z] = 0.0;
			}
			else if (pEntity->isKindOf(AcDbArc::desc()))
			{
				AcDbArc *pArc = AcDbArc::cast(pEntity);
				acutPolar(asDblArray(pArc->center()), DTR(45), pArc->radius(), ptDummy);
				pArc->close();
			}
			else if (pEntity->isKindOf(AcDbSpline::desc()))
			{
				AcDbSpline *pSpline = AcDbSpline::cast(pEntity);
				AcGePoint3d geStartPt;
				pSpline->getControlPointAt(0, geStartPt);
				pSpline->close();

				ptDummy[X] = geStartPt.x; ptDummy[Y] = geStartPt.y; ptDummy[Z] = 0.0;
			}

			// If the layer name is not valid skip it
			if (csLayerName.Find(L"_") == -1) continue;

			// If the layer has "ANNO" skip it
			if (csLayerName.Find(L"ANNO") != - 1) continue;
									
			// Use and Voltage
			bool bValidLayer = false;
			
			/**/ if ((csLayerName.Find(L"HV_OH")          != -1) || (csLayerName.Find(L"HV_UG")  != -1)  || (csLayerName.Find(L"HV_5") != -1) || (csLayerName.Find(L"HV_6") != -1) || (csLayerName.Find(L"HV_22") != -1) || (csLayerName.Find(L"HV_S12") != -1) || (csLayerName.Find(L"HV_S19") != -1)) bValidLayer = true;
			else if ((csLayerName.Find(L"LV_OH")          != -1) || (csLayerName.Find(L"LV_UG")  != -1)) bValidLayer = true;
			else if ((csLayerName.Find(L"SL_OH")          != -1) || (csLayerName.Find(L"SL_UG")  != -1)) bValidLayer = true;
			else if ((csLayerName.Find(L"SV_OH")          != -1) || (csLayerName.Find(L"SV_UG")  != -1)) bValidLayer = true;
			else if ((csLayerName.Find(L"AUX_OH")         != -1) || (csLayerName.Find(L"AUX_UG") != -1)) bValidLayer = true;
			else if ((csLayerName.Find(L"TR_33_")         != -1) || (csLayerName.Find(L"TR_66_") != -1) || (csLayerName.Find(L"TR_132_") != -1)) bValidLayer = true;
			else if ((csLayerName.Find(L"BASE_CADASTRE_") != -1)) bValidLayer = true;

			if (!bValidLayer) continue;

			// Installation
			if ((csLayerName.Find(L"_OH_") == -1) && (csLayerName.Find(L"_UG_") == -1)) continue;

			// Status
			if ((csLayerName.Find(L"_PROP_") == -1) && (csLayerName.Find(L"_PROP") == -1)) continue;

			// Get the length of this entity
			ads_name enSel; acdbGetAdsName(enSel, objEntity);
			acedCommand(RTSTR, L".LENGTHEN", RTLB, RTENAME, enSel, RTPOINT, ptDummy, RTLE, RTSTR, L"", NULL);
			struct resbuf rbSetVar; acedGetVar(L"PERIMETER", &rbSetVar);
			double dLength = rbSetVar.resval.rreal;

			// If the line is already considered for BOM we must increment the length
			bool bMatched = false;
			for (int iCtr = 0; iCtr < legendInfo_Vector.size(); iCtr++)
			{
				// If the information in objects array is not for a LINE, skip it
				if (legendInfo_Vector.at(iCtr).m_csType != _T("LINE") || legendInfo_Vector.at(iCtr).m_csPropLayer != csLayerName) continue;

				// If the layer name matched, add the length
				legendInfo_Vector.at(iCtr).m_dLength = legendInfo_Vector.at(iCtr).m_dLength + dLength;
				bMatched = true;
				break;
			}

			if (bMatched) continue;
			
			// Add other details 
			CLegendInfo legendInfo;
			legendInfo.m_csPropLayer = csLayerName;
			legendInfo.m_dLength = dLength;
			legendInfo.m_csType  = L"LINE";
			legendInfo.m_iIndex  = legendInfo_Vector.size();
			
			CString csDescription; 
			if (!GetDescriptionForLayer(csLayerName, csDescription)) { return FALSE; }
			legendInfo.m_csDescription = csDescription;

			legendInfo_Vector.push_back(legendInfo);
		}
		else if (pEntity->isKindOf(AcDbBlockReference::desc()))
		{
			// Get the block reference pointer for the entity
			AcDbBlockReference *pInsert = AcDbBlockReference::cast(pEntity);

			// Get the block table record for the insert
			AcDbObjectId objTblRcd;
			objTblRcd = pInsert->blockTableRecord();
			pInsert->close();

			// Get the layer name of the insert
			CString csLayerName = pInsert->layer(); 
			csLayerName = csLayerName.MakeUpper();

			// Status
			if ((csLayerName.Find(L"_PROP_") == -1) && (csLayerName.Find(L"_PROP") == -1)) continue;

			// Get the block name and the description assigned to its LEGEND tag
			AcDbBlockTableRecord *pBlkTblRcd = NULL;
			if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk) { return false; }

			ACHAR *pValue; 
			pBlkTblRcd->getName(pValue); 
			CString csName; csName.Format(_T("%s"), pValue); csName.MakeUpper();
			pBlkTblRcd->comments(pValue); CString csDesc; csDesc.Format(_T("%s"), pValue); csDesc.MakeUpper(); 
			delete pValue;
			pBlkTblRcd->close();

			// Check if the block name is already searched for BOM
			bool bMatched = false;
			for (int iCtr = 0; iCtr < legendInfo_Vector.size(); iCtr++) 
			{
				if (!legendInfo_Vector.at(iCtr).m_csObject.CompareNoCase(csName)) 
				{
					legendInfo_Vector.at(iCtr).m_dLength = legendInfo_Vector.at(iCtr).m_dLength + 1;
					bMatched = true;
					break;
				}
			}

			if (bMatched) continue;
						
			CLegendInfo legendInfo;
			legendInfo.m_csObject = csName;
			legendInfo.m_csPropLayer = csLayerName;
			legendInfo.m_csType = L"INSERT";
			legendInfo.m_dLength = 1.0;
			legendInfo.m_csDescription = csName;

			legendInfo.m_iIndex = legendInfo_Vector.size() + 1000;
			legendInfo_Vector.push_back(legendInfo);
		}
		else
		{
			// Close the entity processed
			pEntity->close();
		}
	}

	// Free the selection set
	acedSSFree(ssBOM);
	return true;
}
