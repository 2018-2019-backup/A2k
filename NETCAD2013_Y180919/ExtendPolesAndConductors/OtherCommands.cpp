#include "StdAfx.h"
#include "PoleInfo.h"
#include "OffsetInfo.h"
#include "ConductorInfo.h"

#define FILENAME _T("OC") 
#define EXISTING 1
#define PROPOSED 2
#define OTHERS	 3

extern AcGeVector3d ucsToWcs(AcGeVector3d& vec);
const double gPI   = 4 * atan(1.0);  // best way to get accurate value of pi
extern double g_LegendLinearLen;
extern CString g_csBOMLayer;

CString g_csCmpTxtForProposed; // This will store the component name for all proposed objects
CString g_csCmpTxtForExisting; // This will store the component name for all existing objects
double g_dPoleDia = 0.0;		
CStringArray g_csaInvalidLegendBlock;

extern void sortConductorInfo   (std:: vector <CConductorInfo> &conductorInfo);
extern void reverseConductorInfo(std:: vector <CConductorInfo> &conductorInfo);
extern void appendPoleInfo			(AcDbObjectId objId, ads_point ptPole, std:: vector <CConductorInfo> conductorInfo, std:: vector <CPoleInfo> &poleIInfoVector);
extern void deleteArray					(AcDbObjectIdArray objectIds);
extern void deleteArray					(AcDbVoidPtrArray entities);
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
// Function name: GetDescriptionForLayer()
// Description  :
//////////////////////////////////////////////////////////////////////////
bool GetDescriptionForLayer(CString csLayerName, CString &csLayerDesc)
{
	CString csFuncName = _T("GetDescriptionForLayer()");
	csLayerDesc = csLayerName;

	AcDbLayerTable *pLayerTbl;
	Acad::ErrorStatus es = acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead);
	if (es != Acad::eOk) { return false; }

	// If the layer doesn't exist return quietly
	if (!(pLayerTbl->has(csLayerName))) { pLayerTbl->close(); return false; }

	// Get the object id and the layer table record
	AcDbLayerTableRecord *pLayerTblRecord = ::new AcDbLayerTableRecord;
	es = pLayerTbl->getAt(csLayerName, pLayerTblRecord, AcDb::kForRead);
	if (es != Acad::eOk) { pLayerTbl->close(); acutPrintf(_T("Return 3")); return false; }
	pLayerTbl->close();

	// Description
	csLayerDesc.Format(_T("%s"), pLayerTblRecord->description()); 
	csLayerDesc = csLayerDesc.Trim();
	if (csLayerDesc.IsEmpty() || csLayerDesc == _T("(null)")) csLayerDesc = csLayerName;
	
	pLayerTblRecord->close();
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name: GetLTypeAndColorFromLayer()
// Description  : 
///////////////////////////////////////////////////////////////////////////
bool GetLTypeAndColorFromLayer(CString csLayerName, CString &csLType, Adesk::Int16 &iColor, double &dLineWt)
{
	// Get the layer table pointer 
	AcDbLayerTable *pLayerTbl = NULL;
	Acad::ErrorStatus es = acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead);
	if (es != Acad::eOk) { return false;}

	// Get the layer table record pointer
	AcDbLayerTableRecord *pLayerTblRcd = NULL;
	es = pLayerTbl->getAt(csLayerName, pLayerTblRcd, AcDb::kForRead);
	pLayerTbl->close();
	if (es != Acad::eOk) { return false;}

	// Get the line type name from the linetype ID
	AcDbObjectId objLType = pLayerTblRcd->linetypeObjectId();
	iColor  = pLayerTblRcd->color().colorIndex();
	dLineWt = pLayerTblRcd->lineWeight();
	pLayerTblRcd->close();

	// Get the symbol table record
	AcDbLinetypeTableRecord *pLTypeRcd;
	es = acdbOpenObject(pLTypeRcd, objLType, AcDb::kForRead);
	if (es != Acad::eOk) { return false; }

	// Get the linetype name
	AcString acsName; 
	pLTypeRcd->getName(acsName); pLTypeRcd->close();
	csLType.Format(_T("%s"), acsName.kTCharPtr());
	csLType = csLType.MakeUpper();

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name: ProcessLineForLegend
// Description  :
//////////////////////////////////////////////////////////////////////////
int ProcessLineForLegend(AcDbEntity *pLine, CStringArray &csaObjects, CStringArray &csaLayers, CStringArray &csaTypes, CStringArray &csaDescriptions)
{
	// Add the linetype to the object array. If the linetype is "BYLAYER", get its linetype name from the layer
	CString csLayerName = pLine->layer();

	// If the layer name is not valid skip it
	if (csLayerName.Find(L"_") == -1) 
	{
		pLine->close();
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// Check if the layer name has to be considered for legend
	//////////////////////////////////////////////////////////////////////////
	// If the layer has "ANNO" skip it
	if (csLayerName.Find(L"ANNO") != - 1) return 0;

	// Use and Voltage
	bool bValidLayer = false;
	/**/ if ((csLayerName.Find(L"HV_OH")          != -1) || (csLayerName.Find(L"HV_UG")  != -1)  || (csLayerName.Find(L"HV_5") != -1) || (csLayerName.Find(L"HV_6") != -1) || (csLayerName.Find(L"HV_22") != -1) || (csLayerName.Find(L"HV_S12") != -1) || (csLayerName.Find(L"HV_S19") != -1)) bValidLayer = true;
	else if ((csLayerName.Find(L"LV_OH")          != -1) || (csLayerName.Find(L"LV_UG")  != -1)) bValidLayer = true;
	else if ((csLayerName.Find(L"SL_OH")          != -1) || (csLayerName.Find(L"SL_UG")  != -1)) bValidLayer = true;
	else if ((csLayerName.Find(L"SV_OH")          != -1) || (csLayerName.Find(L"SV_UG")  != -1)) bValidLayer = true;
	else if ((csLayerName.Find(L"AUX_OH")         != -1) || (csLayerName.Find(L"AUX_UG") != -1)) bValidLayer = true;
	else if ((csLayerName.Find(L"TR_33_")         != -1) || (csLayerName.Find(L"TR_66_") != -1) || (csLayerName.Find(L"TR_132_") != -1)) bValidLayer = true;
	else if ((csLayerName.Find(L"BASE_CADASTRE_") != -1)) bValidLayer = true;
	
	if (!bValidLayer) { pLine->close(); return 0; }
	
	// Installation
	if ((csLayerName.Find(L"_OH_") == -1) && (csLayerName.Find(L"_UG_") == -1)) { pLine->close(); return 0; }
	
	// Status
	// if ((csLayerName.Find(L"_EXIST_") == -1) && (csLayerName.Find(L"_PROP_") == -1) && (csLayerName.Find(L"_EXIST") == -1) && (csLayerName.Find(L"_PROP") == -1)) { pLine->close(); return 0; }
	bool bProcessThis = FALSE;
	if ((csLayerName.Find(L"_PROP_") != -1) || (csLayerName.Find(L"_PROP") != -1)) bProcessThis = true;
	if ((csLayerName.Find(L"_EXIST_") != -1) || (csLayerName.Find(L"_EXIST") != -1)) bProcessThis = true;
	if (!bProcessThis){ pLine->close(); return 0; }
		
	CString csLType = pLine->linetype(); csLType = csLType.MakeUpper();
		
	// Check if the line is already considered for legend
	for (int iCtr = 0; iCtr < csaObjects.GetSize(); iCtr++)
	{
		// If the information in objects array is not for a LINE, skip it.
		if (csaTypes.GetAt(iCtr) != _T("LINE")) continue;

		// If the layer name matched, exit the function
		if (!csaObjects.GetAt(iCtr).CompareNoCase(csLayerName)) return 0;
	}

	// Add other details 
	csaObjects.Add(csLayerName);
	csaLayers.Add(csLayerName);
	csaTypes.Add(_T("LINE"));

	CString csDescription; 
	if (!GetDescriptionForLayer(csLayerName, csDescription)) { return -1; }
	csaDescriptions.Add(csDescription);
	pLine->close();

	return 1;
}

//////////////////////////////////////////////////////////////////////////
// Function name: ProcessInsertForLegend
// Description  : 
//////////////////////////////////////////////////////////////////////////
bool ProcessInsertForLegend(AcDbBlockReference *pInsert, CStringArray &csaObjects, CStringArray &csaLayers, CStringArray &csaTypes, CStringArray &csaDescriptions, CStringArray &csaSearched)
{
	// Get the block table record for the insert
	AcDbObjectId objTblRcd;
	objTblRcd = pInsert->blockTableRecord();
	pInsert->close();

	// Get the layer name of the insert
	CString csLayerName = pInsert->layer(); 
	csLayerName = csLayerName.MakeUpper();
	
	// Get the block name and the description assigned to its LEGEND tag
	AcDbBlockTableRecord *pBlkTblRcd = NULL;
	if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk) { return false; }
	
	ACHAR *pValue; 
	pBlkTblRcd->getName(pValue); 
	CString csName; csName.Format(_T("%s"), pValue); csName.MakeUpper();
	pBlkTblRcd->comments(pValue); CString csDesc; csDesc.Format(_T("%s"), pValue); csDesc.MakeUpper(); 
	delete pValue;
	pBlkTblRcd->close();
	
	// Check if the block name is already searched for LEGEND tag
	for (int iCtr = 0; iCtr < csaSearched.GetSize(); iCtr++) { if (!csaSearched.GetAt(iCtr).CompareNoCase(csName)) return true; }

	// Add this to the array so that they need not be processed again
	csaSearched.Add(csName);

	// Get the list of attribute tags, default values for the block
	CStringArray csaPrompts;
	CStringArray csaTags;
	CStringArray csaDefaults;
	int iAttrib = BuildAttribsList(csName, csaPrompts, csaTags, csaDefaults, FALSE);
	int iCtr;
	if (iAttrib)
	{
		// Check if the LEGEND tag is present
		for (iCtr = 0; iCtr < csaTags.GetSize(); iCtr++) { if (!csaTags.GetAt(iCtr).CompareNoCase(L"LEGEND")) break; }

		// If LEGEND tag not present, exit the function
		if (iCtr == csaTags.GetSize()) { return true; }
	}
	else return true; 
		
	csaObjects.Add(csName);
	csaLayers.Add(csLayerName);
	csaTypes.Add(_T("INSERT"));
	// csaDescriptions.Add(csDesc); 
	csaDescriptions.Add(csaDefaults.GetAt(iCtr)); 
		
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Functions specific to EA_LEGEND
//////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetObjectsForLegend()
// Description  : Get the details of LINES/INSERTS that can be placed in the Legend.
// Arguments    : 1. CStringArray &, Holds the block name or layer name of the object found suitable for LEGEND o/p.
//                2. CStringArray &, Holds the layer names of these objects.
//                3. CStringArray &, Holds the entity type of the objects.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetObjectsForLegend(ads_point ptMin, ads_point ptMax, AcDbObjectIdArray &aryObjIds, CStringArray &csaObjects,  CStringArray &csaLayers, CStringArray &csaTypes, CStringArray &csaDescriptions, CStringArray &csaBlocksSearched)
{
	//////////////////////////////////////////////////////////////////////////
	// Select all inserts and lines placed in the drawing
	//////////////////////////////////////////////////////////////////////////
	ads_name ssLegend;
	struct resbuf *rbpFilt = acutBuildList(-4, _T("<OR"), RTDXF0, _T("INSERT"), RTDXF0, _T("LWPOLYLINE"), RTDXF0, _T("LINE"), RTDXF0, _T("POLYLINE"), -4, _T("OR>"), 67, 0, NULL);

	// Add the layers that are to be ignored
	if (aryObjIds.length())
	{
		// Move to the last of the selection set chain
		struct resbuf *rbpTemp;
		for (rbpTemp = rbpFilt; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);
		rbpTemp->rbnext = acutBuildList(-4, _T("<NOT"), -4, _T("<OR"), NULL);

		AcDbLayerTableRecord *pLayerTblRcd;
		AcString acsName;
		CString csName;
		for (int iCtr = 0; iCtr < aryObjIds.length(); iCtr++)
		{
			for (rbpTemp = rbpFilt; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);

			// Get the layer name that is frozen
			acdbOpenObject(pLayerTblRcd, aryObjIds.at(iCtr), AcDb::kForRead);
			pLayerTblRcd->getName(acsName);
			pLayerTblRcd->close();

			csName.Format(_T("%s"), acsName.kTCharPtr()); csName.MakeUpper();
			rbpTemp->rbnext = acutBuildList(8, csName, NULL);
		}

		for (rbpTemp = rbpFilt; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);
		rbpTemp->rbnext = acutBuildList(-4, _T("OR>"), -4, _T("NOT>"), NULL);
	}

	// Set the UCSFollow mode to 1 so that the UCS set to the viewport is automatically set when it is activated
	saveUCSMode();
	
	//////////////////////////////////////////////////////////////////////////
	// Go the model space of the viewport
	//////////////////////////////////////////////////////////////////////////
	acedMspace();

	if (acedSSGet(_T("C"), ptMin, ptMax, rbpFilt, ssLegend) != RTNORM) 
	{
		acedSSFree(ssLegend); 
		acutRelRb(rbpFilt); 

		// Restore the UCSFollow to original
		restoreUCSMode();

		// Get back to paper space
		acedPspace();
		return false; 
	}

	acutRelRb(rbpFilt);
	restoreUCSMode();
	acedPspace();

	// Process the entities in the selection set for legend
	ads_name enEntity;
	AcDbObjectId objEntity;
	AcDbEntity *pEntity = NULL;

	Acad::ErrorStatus es;
	long lLength = 0L; acedSSLength(ssLegend, &lLength); 
	for (long lCtr = 0L; lCtr < lLength; lCtr++)
	{
		// Get the object Id
		acedSSName(ssLegend, lCtr, enEntity);
		acdbGetObjectId(objEntity, enEntity);

		// Get the entity pointer
		es = acdbOpenObject(pEntity, objEntity, AcDb::kForRead);
		if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); acedSSFree(ssLegend); return false; }
		
		// Call the appropriate functions to process INSERTS and LINES for legends
		if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc())) 
		{
			// Call the function to process an insert for legend
			if (ProcessLineForLegend(pEntity, csaObjects, csaLayers, csaTypes, csaDescriptions) == -1) { pEntity->close(); acedSSFree(ssLegend); return false; }
		}
		else if (pEntity->isKindOf(AcDbBlockReference::desc()))
		{
			// Get the block reference pointer for the entity
			AcDbBlockReference *pInsert = AcDbBlockReference::cast(pEntity);

			// Call the function to process an insert for legend
			if (!ProcessInsertForLegend(pInsert, csaObjects, csaLayers, csaTypes, csaDescriptions, csaBlocksSearched)) { pEntity->close(); acedSSFree(ssLegend); return false; }
		}

		// Close the entity processed
		pEntity->close();
	}

	// Free the selection set
	acedSSFree(ssLegend);
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : setTextString
// Description  : 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setTextString(AcDbTable *pTable, int iRow, int iCol, AcDbObjectId objIDTxtStyle, AcDb::RotationAngle rotAngle, CString csText)
{
	pTable->setTextStyle(iRow, iCol, objIDTxtStyle);
	pTable->setTextString(iRow, iCol, L" " + csText); 
	pTable->setTextRotation(iRow, iCol, rotAngle);
	//pTable->setTextHeight(iRow, iCol, 1.5);

	// Set the row alignment
	pTable->setAlignment(iRow, iCol, AcDb::kMiddleLeft);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL createBlock(CString csLType, int iColor)
{
	// Check if the block is already present
	CString csBlkName; csBlkName.Format(L"%s-%d", csLType, iColor);
	if (acdbTblSearch(L"BLOCK", csBlkName, false) != NULL) return true;

	//////////////////////////////////////////////////////////////////////////
	// Create a block definition
	//////////////////////////////////////////////////////////////////////////
	// Draw a line to place in BLOCK
	AcDbLine *pLine = new AcDbLine(AcGePoint3d(0.0, 0.0, 0.0), AcGePoint3d(10.0, 0.0, 0.0));
	pLine->setLinetype(csLType);
	pLine->setLinetypeScale(1);
	pLine->setColorIndex(iColor);

	AcDbBlockTable *pBlkTbl;
	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlkTbl, AcDb::kForWrite);
	AcDbBlockTableRecord *pTblRcd = new AcDbBlockTableRecord();
	pTblRcd->setName(csBlkName);
	pTblRcd->setOrigin(AcGePoint3d(0.0, 0.0, 0.0));
	Acad::ErrorStatus es = pTblRcd->appendAcDbEntity(pLine);

	es = pBlkTbl->add(pTblRcd);
	es = pTblRcd->close(); 
	es = pLine->close();
	es = pBlkTbl->close(); 
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
bool setTextSizeInStyle(CString csStyle, double dNewTxtSize, double &dOldTxtSize, AcDbObjectId &objIDTxtStyle)
{
	AcDbTextStyleTable *pTxtStyle = NULL;
	if (acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pTxtStyle, AcDb::kForRead) != Acad::eOk) return false;

	// Get the object id of the given text style. Set its text height as 0.0, in case they are fixed at a non-zero value.
	AcDbTextStyleTableRecord *pTxtStyleRcd = new AcDbTextStyleTableRecord();
	if (!pTxtStyle->has(csStyle))
	{
		// Create the style as specified in the tables
		double dHeight;
		pTxtStyle->close();
		CreateEATextStyle(dHeight);
	}

	if (pTxtStyle->getAt(csStyle, pTxtStyleRcd, AcDb::kForWrite) != Acad::eOk) 
	{
		pTxtStyle->close(); 
		return false; 
	}

	dOldTxtSize = pTxtStyleRcd->textSize();
	objIDTxtStyle = pTxtStyleRcd->objectId();
	pTxtStyleRcd->setTextSize(0.0);
	pTxtStyleRcd->close();
	pTxtStyle->close();

	return true;
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
// Function name: selectConductors()
// Description  : Enables selection of valid conductors.
// Arguments    : 1. ads_point,			 Insertion point of the pole.
//                2. std:: vector <CConductorInfo> &, Conductor vector
//  				    : 3. std:: vector <COffsetInfo> &, Offset vector
//////////////////////////////////////////////////////////////////////////
bool selectConductors(ads_point ptPole, std:: vector <CConductorInfo> &conductorInfo_Vector, std:: vector <COffsetInfo> &offsetInfo_Vector)
{
	Acad::ErrorStatus es;
	AcDbObjectId objId;
	ads_name enEntity;
	AcDbEntity *pEntity;
	long lLength = 0L;
	CString csLayerName;

	// Array that defines the order of drawing the conductors
	CStringArray csaCableSortOrder; 
	csaCableSortOrder.Add(_T("SV")); 
	csaCableSortOrder.Add(_T("LV")); 
	csaCableSortOrder.Add(_T("SL")); 
	csaCableSortOrder.Add(_T("HV")); 
	csaCableSortOrder.Add(_T("TR"));

	while (T)
	{
		ads_name ssGet;

		acutPrintf(_T("\r\nSelect conductors connected to this pole...\n"));
		int iRet = acedSSGet(NULL, NULL, NULL, NULL, ssGet);

		/**/ if (iRet == RTCAN)  return false;
		else if (iRet != RTNORM) continue;
		
		// Validate the entities selected
		acedSSLength(ssGet, &lLength);

		conductorInfo_Vector.clear();
		double dRefAngle = -999.99;
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
			// pEntity->close();

			// The first two characters must have SV, LV, SL & TR and the layer name must have "_" in it.
			if ((csLayerName.GetLength() < 2) || (csLayerName.Find(_T("_")) == -1)) {	pEntity->close(); continue; }

			// Check if this layer has to be processed
			int iIndex = -1;
			if (!CheckForDuplication(csaCableSortOrder, csLayerName.Mid(0, 2), iIndex)) { pEntity->close(); continue; }

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

			// Remove the "_EXIST" in the layer name and suffix it with _PROP 
			//			if (csLayerName.Find(_T("_EXIST_")) != -1) csLayerName.Replace(_T("_EXIST_"), _T(""));
			// else if (csLayerName.Find(_T("_EXIST"))  != -1) csLayerName.Replace(_T("_EXIST"), _T(""));
			// else if (csLayerName.Find(_T("_PROP_"))  != -1) csLayerName.Replace(_T("_PROP_"), _T(""));
			// else if (csLayerName.Find(_T("_PROP"))  != -1)  csLayerName.Replace(_T("_PROP"), _T(""));
			// csLayerName = csLayerName + _T("_PROP");

			/**/ if (csLayerName.Find(_T("EXIST")) != -1)	
				csLayerName.Replace(_T("EXIST"), _T("PROP"));
			else if ((csLayerName.Find(_T("EXIST")) == -1) && (csLayerName.Find(_T("PROP")) == -1)) 
				csLayerName = csLayerName + _T("_PROP");
						
			conductorInfo.m_csLayer = csLayerName;
			conductorInfo.m_dOffset = offsetInfo.m_dOffset;
			createLayer(csLayerName, Adesk::kFalse, Adesk::kFalse);

			// Add this to the vector for sort
			conductorInfo_Vector.push_back(conductorInfo);
		}

		// Free the selection set
		acedSSFree(ssGet);

		// Check if there are any valid conductors selected
		if (conductorInfo_Vector.size() <= 0) { acutPrintf(_T("\nNo valid conductors selected.\nConductor must be a LINE/POLYLINE on layers with prefix \"SV/LV/SL/HV/TR\".\n"));	} else return true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: insertNewPole()
// Description  : Inserts the new pole
// Arguments    : 1. ads_point, Insertion point of the new pole
//  				      2. AcDbObjectId, Object Id of the selected pole that should be duplicated
//                3. AcDbObjectId &, Object Id of the new pole
////////////////////////////////////////////////////////////////////////////////////////////////
bool insertNewPole(ads_point ptNewPole, AcDbObjectId objSelPole, AcDbObjectId &objNewPole)
{
	// Open the block reference
	AcDbBlockReference *pInsert;
	Acad::ErrorStatus es = acdbOpenObject(pInsert, objSelPole, AcDb::kForRead);
	if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); return false; }

	// Get the layer name of the insert
	// CString csLayerName; csLayerName.Format(_T("%s"), pInsert->layer());
	
	// Get the block table record object id f the existing pole
	AcDbObjectId objBlkTblRcd  = pInsert->blockTableRecord();

	// Close the reference
	pInsert->close();

	// Get the block name from the symbol table
	AcDbSymbolTableRecord *pBlkTblRcd = NULL;
	es = acdbOpenObject(pBlkTblRcd, objBlkTblRcd, AcDb::kForRead);
	if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); return false; }

	AcString acsName; pBlkTblRcd->getName(acsName); pBlkTblRcd->close();
	CString csName;   csName.Format(_T("%s"), acsName.kTCharPtr());
	pBlkTblRcd->close();

	// Insert the new pole
	if (!insertBlock(_T("POLE_PROP"), _T(""), ptNewPole, 0.0, 0.0, 0.0, 0.0, _T(""), objNewPole, true)) return false;
	
	return true;
}

// bool DrawConductors(AcDbObjectId objNewPole, ads_point ptSelPole, ads_point ptNewPole, std:: vector <CConductorInfo> &conductorInfo_Vector, std:: vector <COffsetInfo> offsetInfo_Vector, AcDbObjectIdArray &arObjIds)
bool DrawConductors(bool bModifySourceConductor, AcDbObjectId objNewPole, std::vector <CPoleInfo> &poleInfo_Vector, ads_point ptNewPole, std:: vector <CConductorInfo> &conductorInfo_Vector, std:: vector <COffsetInfo> offsetInfo_Vector, AcDbObjectIdArray &arObjIds)
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
		// Point at offset from center of the pole perpendicular to the conductor
		// acutPrintf(_T("\nOffset [%.2f]\n"), offsetInfo_Vector[iRow].m_dOffset);
		// acutPolar(ptSelPole, dAngle + PIby2, offsetInfo_Vector[iRow].m_dOffset, ptStart);
		dOffset = conductorInfo_Vector[iRow].m_dOffset;
		acutPolar(ptSelPole, dAngle + PIby2, dOffset, ptStart);
		
		// Get the offset along the conductor
		if (fabs(dOffset) < g_dPoleDia / 2)
		{
			double dOppSide = sqrt((g_dPoleDia * g_dPoleDia / 4) - (dOffset * dOffset));

			// Start point of the conductor at selected pole
			acutPolar(ptStart, dAngle, dOppSide, ptStart);
		}

		// End point at offset from center of the new pole
		acutPolar(ptNewPole, dAngle + PIby2, dOffset, ptEnd);

		// Get the entity type of the conductor to duplicate
		AcDbEntity *pEntity;
		es = acdbOpenObject(pEntity, conductorInfo_Vector[iRow].m_objId, AcDb::kForRead);
		if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); acTransactionManagerPtr()->abortTransaction(); return false; }

		bool bIsLine = true;
		if (pEntity->isKindOf(AcDbLine::desc())) bIsLine = true; else bIsLine = false;

		if (!bIsLine)
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

			//////////////////////////////////////////////////////////////////////////
			// Extend the lines to meet inside the pole
			//////////////////////////////////////////////////////////////////////////
			AcGePoint3dArray geIntersectPts;
			es = pPline->intersectWith(pEntity, AcDb::kExtendBoth, geIntersectPts);
			if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); }
			else if (geIntersectPts.length() > 0)
			{
				// Modify the start point of the new conductors
				pPline->setPointAt(0, AcGePoint2d(geIntersectPts.at(0).x, geIntersectPts.at(0).y));
				pEntity->close();

				//////////////////////////////////////////////////////////////////////////
				// Modify the start point of the source conductor
				//////////////////////////////////////////////////////////////////////////
				if (bModifySourceConductor || (poleInfo_Vector.size() > 1))
				{
					AcDbPolyline *pExtPline;
					es = acdbOpenObject(pExtPline, conductorInfo_Vector[iRow].m_objId, AcDb::kForWrite);
					if (es == Acad::eOk)
					{
						AcGePoint3d geStartPt; pExtPline->getStartPoint(geStartPt);
						AcGePoint3d geEndPt;   pExtPline->getEndPoint(geEndPt);
	
						CString csDist1; csDist1.Format(_T("%.2f"), acutDistance(asDblArray(geStartPt), ptSelPole));
						CString csDist2; csDist2.Format(_T("%.2f"), acutDistance(asDblArray(geEndPt),   ptSelPole));

						if (_tstof(csDist1) <= _tstof(csDist2))
						{
							pExtPline->setPointAt(0, AcGePoint2d(geIntersectPts.at(0).x, geIntersectPts.at(0).y));
						}
						else
						{
							pExtPline->setPointAt(pExtPline->numVerts() - 1, AcGePoint2d(geIntersectPts.at(0).x, geIntersectPts.at(0).y));
						}

						pExtPline->close();
					}
				}
			}
			
			// Get the entity name of this cable to use in extend command
			pPline->close();
		}
		else
		{
			// Create a new line
			AcDbLine *pLine = new AcDbLine(AcGePoint3d(ptStart[X], ptStart[Y], 0.0), AcGePoint3d(ptEnd[X], ptEnd[Y], 0.0));

			// Set the layer
			pLine->setLayer(conductorInfo_Vector[iRow].m_csLayer);
			pLine->setLinetype(_T("BYLAYER"));
			AcCmColor color; color.setColor(AcCmEntityColor::kByLayer);
			pLine->setColor(color);

			// Append the entity for deleting just in case if a flip is required
			appendEntityToDatabase(pLine);

			// The new conductor ID is required for the continuity of the command
			conductorInfo_Vector[iRow].m_objIdNext = pLine->objectId();

			// This is done to erase them during a flip
			arObjIds.append(pLine->objectId());

			//////////////////////////////////////////////////////////////////////////
			// Extend the lines to meet inside the pole
			//////////////////////////////////////////////////////////////////////////
			AcGePoint3dArray geIntersectPts;
			es = pLine->intersectWith(pEntity, AcDb::kExtendBoth, geIntersectPts);
			/**/ if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); }
			else if (geIntersectPts.length() > 0)
			{
				// Modify the start point
				pLine->setStartPoint(AcGePoint3d(geIntersectPts.at(0).x, geIntersectPts.at(0).y, 0.0));
			}
			
			pEntity->close();

			//////////////////////////////////////////////////////////////////////////
			// Modify the start point of the source conductor
			//////////////////////////////////////////////////////////////////////////
			if (bModifySourceConductor || (poleInfo_Vector.size() > 1))
			{
				AcDbLine *pExtLine;
				acdbOpenObject(pExtLine, conductorInfo_Vector[iRow].m_objId, AcDb::kForWrite);
				if (es == Acad::eOk)
				{
					// if ((acutDistance(asDblArray(pExtLine->startPoint()), ptSelPole)) < acutDistance(asDblArray(pExtLine->endPoint()), ptSelPole))
					CString csDist1; csDist1.Format(_T("%.2f"), acutDistance(asDblArray(pExtLine->startPoint()), ptSelPole));
					CString csDist2; csDist2.Format(_T("%.2f"), acutDistance(asDblArray(pExtLine->endPoint()),   ptSelPole));

					if (_tstof(csDist1) <= _tstof(csDist2))
						pExtLine->setStartPoint(geIntersectPts.at(0));
					else
						pExtLine->setEndPoint(geIntersectPts.at(0));
					pExtLine->close();
				}
			}
			
			// Close the entity
			pLine->close();
		}

		// Replace the new object id with the new one
		conductorInfo_Vector[iRow].m_objId		 = conductorInfo_Vector[iRow].m_objIdNext;
		conductorInfo_Vector[iRow].m_objIdNext = AcDbObjectId::kNull;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////	
// Function name: Command_ExtendPoleAndConductors()
// Description  : Defines the function that is called by "EA_EXTENDPOLEANDCONDCUTORS".
////////////////////////////////////////////////////////////////////////////////////////////
void Command_ExtendPoleAndConductors()
{
	Acad::ErrorStatus es;

	// Allow user to an select existing pole and get insertion point too.
	AcDbObjectId objSelPole;
	ads_point ptSelPole;
	AcGePoint3dArray gePrvPoints;
	if (!selectPole(objSelPole, ptSelPole)) return;

	// Allow the user to select valid conductors
	std::vector <CConductorInfo> conductorInfo_Vector;
	std::vector <COffsetInfo>    offsetInfo_Vector;
	std::vector <CPoleInfo>			 poleInfo_Vector;	

	if (!selectConductors(ptSelPole, conductorInfo_Vector, offsetInfo_Vector)) return; 

	// Sort vector array's
	sortConductorInfo(conductorInfo_Vector);
	
	// sortOffsetInfo(offsetInfo_Vector);

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
		if (acedGetPoint(ptSelPole, _T("\nInsertion point for new pole: "), ptNewPole) == RTCAN) return;

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

			// Later in this while the code the possibility of there returns are possible. However, RTNORM will be the value if a new point is 
			// selected for pole location.
			if (iRet == RTNORM)
			{
				//////////////////////////////////////////////////////////////////////////
				// Insert the new pole
				//////////////////////////////////////////////////////////////////////////
				// Mail: 16.07.08: The client has updated a the POLE_PROP block in the template, which is now called POLE_NUMBERED_PROP. This resides on the tool palette, and when placed it is exploded 
				//                 to POLE_PROP etc... to be placed on the correct layer. 
				//                 With this said, the NEW POLE command when used via the tool bar places the block on screen; however the 2 attributes contained within the block are not shown. 
				//                 Can you accommodate this?, ie, use the POLE_NUMBER_PROP, so that when it is placed it is exploded as above and displays the two attributes.
				// if (!insertBlock(_T("POLE_PROP"), _T(""), ptNewPole, 0.0, 0.0, 0.0, 0.0, _T(""), objNewPole, true)) { acTransactionManagerPtr()->abortTransaction(); return; }
				if (!insertBlock(_T("POLE_NUMBERED_PROP"), L"POLE_PROP", ptNewPole, 0.0, 0.0, 0.0, 0.0, _T(""), objNewPole, true)) { acTransactionManagerPtr()->abortTransaction(); return; }

				// Get the geomExtents of the block so that the diameter of the block can be determined. The diameter of the block is taken to calculate the end points
				// of the conductors on the pole.
				if (!g_dPoleDia)
				{
					AcDbExtents exBounds; if (!GetPoleExtents(objNewPole, exBounds)) { acTransactionManagerPtr()->abortTransaction(); return; }
					g_dPoleDia = fabs(exBounds.maxPoint().x - exBounds.minPoint().x);
				}

				// Explode the newly inserted pole block
				if (objNewPole.isValid())
				{
					ads_name enNewPole; acdbGetAdsName(enNewPole, objNewPole);
					acedCommand(RTSTR, L".EXPLODE", RTENAME, enNewPole, NULL);

					ads_name ssExplode; 
					if (acedSSGet(L"P", NULL,	NULL, NULL, ssExplode) == RTNORM)
					{
						// Get the object id of the new pole "POLE_PROP" block that results from EXPLODE
						ads_name enEntity; 
						AcDbObjectId objExplode;

						long lLength = 0L; acedSSLength(ssExplode, &lLength);
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
							pInsert->setLayer(L"POLE_PROP");
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

			//////////////////////////////////////////////////////////////////////////
			// Draw the new conductors
			//////////////////////////////////////////////////////////////////////////
			AcDbObjectIdArray arObjIds;
			if (result[0] != 'U') 
			{
				// To draw new conductors from previous pole to the new pole, the information of conductors connected to the pole is passed as reference.
				// The draw function will draw the new set of conductors to the new pole and return the new set of conductor information that is appended
				// to the new pole.
				conductorInfo_Vector = poleInfo_Vector[poleInfo_Vector.size() - 1].m_conductorInfo_Vector;
				// DrawConductors(objNewPole, poleInfo_Vector[poleInfo_Vector.size() - 1].m_ptInsert, ptNewPole, conductorInfo_Vector, offsetInfo_Vector, arObjIds);
				DrawConductors(bModifySourceConductor, objNewPole, poleInfo_Vector, ptNewPole, conductorInfo_Vector, offsetInfo_Vector, arObjIds);
				appendPoleInfo(objNewPole, ptNewPole, conductorInfo_Vector, poleInfo_Vector);
			}

			//////////////////////////////////////////////////////////////////////////
			// All conductors drawn, check if the user wants to flip the side or Undo 
			// what is done or Specify a new point or Exit the command.
			//////////////////////////////////////////////////////////////////////////
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
						return;
					}
				}
				else if (iRet == RTNONE)
				{
					// End the command
					acTransactionManagerPtr()->endTransaction();
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
// Function name : CreateTextStyle
// DEscription   : Modifies the "Standard" text style based on the inputs given
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateTextStyle(CString csStyle, const TCHAR *pszFont, double dHeight, double dWidthFactor, double dObliqueAngle, const TCHAR *pszType)
{
	// We have different arguments for STYLE command while settings fonts. 
	if (!_tcsicmp(pszType, _T("TTF")))
	{
		// For True type
		acedCommand(RTSTR, _T(".STYLE"), RTSTR, csStyle, RTSTR, pszFont, RTREAL, dHeight, RTREAL, dWidthFactor, RTREAL, 0.0, RTSTR, _T("N"), RTSTR, _T("N"), NULL);
	}
	else if (!_tcsicmp(pszType, _T("SHX")))
	{
		// Shape based
		acedCommand(RTSTR, _T(".STYLE"), RTSTR, csStyle, RTSTR, pszFont, RTREAL, dHeight, RTREAL, dWidthFactor, RTREAL, 0.0, RTSTR, _T("N"), RTSTR, _T("N"), RTSTR, _T("N"), NULL);
	}
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

//////////////////////////////////////////////////////////////////////////
// Function name: CreateEATextStyle
// Description  :
//////////////////////////////////////////////////////////////////////////
void CreateEATextStyle(double &dHeight)
{
	// Get the text values from tblText
	CQueryTbl tblText;
	if (!tblText.SqlRead(DSN_DWGTOOLS, _T("SELECT [Style], [Font], [Height], [WidthFactor], [ObliqueAngle], [TTForSHX] FROM tblText ORDER BY [ID]"), __LINE__, _T(__FILE__), _T("CreateEATextStyle()"),true)) return;

	CString csStyle;
	for (int iRow = 0; iRow < tblText.GetRows(); iRow++)
	{
		CStringArray *pszRow = tblText.GetRowAt(iRow);

		// Create the text style
		dHeight = _tstof(pszRow->GetAt(2));
		csStyle.Format(L"eCapture_%s", (TCHAR *)(LPCTSTR) pszRow->GetAt(0));
		CreateTextStyle(csStyle, (TCHAR *)(LPCTSTR) pszRow->GetAt(1), 0.0, _tstof(pszRow->GetAt(3)), _tstof(pszRow->GetAt(4)), pszRow->GetAt(5));
	}
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
