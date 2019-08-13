#include "StdAfx.h"
#include "LayoutInfo.h"
#include "LegendInfo.h"
#include "LegendNotificationDlg.h"

#define FILENAME _T("OC") 
#define EXISTING 1
#define PROPOSED 2
#define OTHERS	 3

/////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////

int f_iProposedColIndex;
int f_iExistingColIndex;
int f_iDescriptColIndex;

extern double g_LegendLinearLen;
CString g_csCmpTxtForProposed; // This will store the component name for all proposed objects
CString g_csCmpTxtForExisting; // This will store the component name for all existing objects
extern CStringArray g_csaInvalidLegendBlock;

//////////////////////////////////////////
// Externally defines functions
//////////////////////////////////////////

void sortLayoutInfo     (std::vector <CLayoutInfo> &layoutInfo);
void sortLegendInfo			(std:: vector <CLegendInfo> &legendInfo);
int getNumberOfRows			(std:: vector <CLegendInfo> &legendInfo, int iLegendType);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: getVPortClipHandles()
// Description  : Collect valid VPORT handles for the given paper space layout.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
int getVPortClipHandles(CString csLayout, AcDbObjectIdArray &aryClipObjIds, AcDbObjectIdArray &aryVPObjIds)
{
	// Get the layout manager pointer
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();

	// Get the layout pointer queried
	//Commented for ACAD 2018
	//AcDbLayout *pLayout = pLayoutMngr->findLayoutNamed(csLayout, false);
	AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(csLayout);
	AcDbLayout *pLayout = NULL;
	acdbOpenObject(pLayout, objLayoutId, AcDb::kForRead);

	if (!pLayout) return 0;

	AcDbObjectIdArray geObjIdArray = pLayout->getViewportArray();
	pLayout->close();

	AcDbViewport *pViewPort;
	ads_name enVPort;
	struct resbuf *rbpGet;
	Acad::ErrorStatus es;

	for (int iCtr = geObjIdArray.length() - 1; iCtr >= 0; iCtr--)
	{
		// Get the viewport object id
		AcDbObjectId objIdVPort = geObjIdArray.at(iCtr);

		// Get the location and the size of the viewport
		es = acdbOpenObject(pViewPort, objIdVPort, AcDb::kForRead);
		if (es != Acad::eOk) continue;

		bool bIsOn  = pViewPort->isOn();
		int iNumber = pViewPort->number();
		pViewPort->close();

		// If the viewport is not ON or when PAPERSPACE default skip it
		if (!bIsOn || (iNumber == 1)) continue;

		// Check if this viewport is non-rectangular
		if (pViewPort->isNonRectClipOn())
		{
			aryClipObjIds.append(pViewPort->nonRectClipEntityId());
			aryVPObjIds.append(pViewPort->objectId());
		}

		pViewPort->close();
	}

	return aryClipObjIds.length();
}

//////////////////////////////////////////////////////////////////////////
// Function name: GetLayoutNamesInPaperSpace
// Description  : Retrieves the names of all PAPER SPACE View ports and
//                sorts them based on their tab order.
//////////////////////////////////////////////////////////////////////////
bool GetLayoutNamesInPaperSpace(std::vector <CLayoutInfo> &layoutInfoVector)
{
	// Get the block table of the drawing and create a new iterator
	Acad::ErrorStatus es;
	AcDbBlockTable *pBT; 
	AcDbBlockTableIterator* pIter;

	es = acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBT, AcDb::kForRead); 
	if (es != Acad::eOk) { acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es)); return false; }

	es = pBT->newIterator(pIter);
	pBT->close();
	if (es != Acad::eOk) { acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es)); return false; }

	// Loop through the iterator
	int iTabOrder;
	AcDbBlockTableRecord* pBTR; 
	AcDbObjectId layoutId;
	AcDbLayout *pLayout;
	ACHAR *pLayoutName;
	CString csLayoutName;

	for( ; !pIter->done(); pIter->step())
	{
		// Get the block table record
		es = pIter->getRecord(pBTR, AcDb::kForRead);
		if (es != Acad::eOk) { acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es)); delete pIter; return false; }

		// If this is a layout
		if (!pBTR->isLayout()) { pBTR->close(); continue; }

		// Get the layout's object ID and from there its name
		layoutId = pBTR->getLayoutId();
		acdbOpenAcDbObject((AcDbObject*&)pLayout, layoutId, AcDb::kForRead);
		iTabOrder = pLayout->getTabOrder();
		pLayout->getLayoutName(pLayoutName);
		csLayoutName = pLayoutName;
		pLayout->close();

		// If this is "Model" layout, we are NOT interested
		if (csLayoutName == _T("Model")) { pBTR->close(); continue; }

		// Create a new iterator for this record
		int iEnt = 0;
		AcDbBlockTableRecordIterator* pBtblrIter; 
		es = pBTR->newIterator(pBtblrIter);
		if (es != Acad::eOk) { acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es)); pBTR->close(); return false; }

		// Just loop through it and count the number of steps
		for( ; !pBtblrIter->done(); pBtblrIter->step()) iEnt++;

		// Delete the record iterator
		delete pBtblrIter;

		// If the count is 0, then the layout is not initialized
		if (iEnt != 0) 
		{
			CLayoutInfo layoutInfo;
			layoutInfo.m_csLayoutName = csLayoutName;
			layoutInfo.m_iTabOrder    = iTabOrder;

			layoutInfoVector.push_back(layoutInfo);
		}

		// Close the block table record
		pBTR->close();
	}

	// Delete the iterator
	delete pIter;

	// Sort the layout information in the vector
	sortLayoutInfo(layoutInfoVector);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetConstantAttributeValue()
// Description  : Retrieves the value assigned to CONSTANT attribute value of an INSERT
//////////////////////////////////////////////////////////////////////////////////////////////////////////
CString GetConstantAttributeValue(AcDbBlockReference *pInsert, CString csTag)
{
	CString csLegend = L"";

	// Get the block table record
	Acad::ErrorStatus es;
	AcDbObjectId objBlkRcdId = pInsert->blockTableRecord();
	AcDbBlockTableRecord *pBlkTblRcd;
	es = acdbOpenObject(pBlkTblRcd, objBlkRcdId, AcDb::kForRead); 
	if (es != Acad::eOk) { return csLegend; }

	// Get block table record iterator
	AcDbBlockTableRecordIterator *pIter;
	es = pBlkTblRcd->newIterator(pIter);
	if (es != Acad::eOk) { return csLegend; }
	pBlkTblRcd->close();

	AcDbEntity *pEnt;
	AcDbAttributeDefinition *pAttDef;
	AcDbObjectId objAttId;

	for (pIter->start(); !pIter->done(); pIter->step())
	{
		// Get the next entity
		es = pIter->getEntity(pEnt, AcDb::kForRead);
		if (es != Acad::eOk) { return csLegend; }

		// Make sure the entity is an attribute definition
		pAttDef = AcDbAttributeDefinition::cast(pEnt);
		if (pAttDef == NULL) { pEnt->close(); continue; }

		if (!wcscmp(pAttDef->tag(), csTag)) 
		{
			csLegend = pAttDef->textString();
			pAttDef->close(); 

			delete pIter; 
			return csLegend; 
		}

		pAttDef->close();
	}

	delete pIter;
	return csLegend;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: ProcessInsertForLegend
// Description  : Check if the given INSERT qualifies for a place in the LEGEND. 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ProcessInsertForLegend(AcDbBlockReference *pInsert, std::vector <CLegendInfo> &legendInfo_Vector, CStringArray &csaSearched)
{
	// Get the block name  and layer name of this INSERT
	// Layer name
	CString csLayerName = pInsert->layer(); 
	csLayerName = csLayerName.MakeUpper();

	// Block name
	AcDbObjectId objTblRcd;
	objTblRcd = pInsert->blockTableRecord();
	pInsert->close();

	AcDbBlockTableRecord *pBlkTblRcd = NULL;
	if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk) { return false; }

	ACHAR *pValue; 
	pBlkTblRcd->getName(pValue); 
	CString csName; csName.Format(_T("%s"), pValue); csName.MakeUpper();
	delete pValue;
	pBlkTblRcd->close();

	// Check if the block name is already searched for LEGEND tag
	for (int iCtr = 0; iCtr < csaSearched.GetSize(); iCtr++) { if (!csaSearched.GetAt(iCtr).CompareNoCase(csName)) return true; }

	// Add this to the array so that they need not be processed again
	csaSearched.Add(csName);

	// Call the function to get the value of LEGEND attribute within this INSERT
	CString csDescription = GetConstantAttributeValue(pInsert, L"LEGEND");
	if (csDescription.IsEmpty()) return true;

	// Status
	CString csCounterName;
	bool bIsProposed = false;
	bool bIsExisting = false;

	/**/ if (csName.Find(L"_PROP") != -1) 
	{
		bIsProposed = true; 

		// All proposed block names will have _PROP as its suffix and Existing block names are without this as suffix
		csCounterName = csName;
		if (csCounterName.Find(L"_PROP") != -1) csCounterName = csCounterName.Mid(0, csCounterName.Find(L"_PROP"));
	}
	else
	{
		bIsExisting = true;

		// All proposed block names will have _PROP as its suffix and Existing block names are without this as suffix
		csCounterName = csName + L"_PROP";
	}
	
	if (!bIsProposed && !bIsExisting) { return 0; }

	// Check if this INSERT is already processed
	bool bMatched = false;
	for (int iCnt = 0; iCnt < legendInfo_Vector.size(); iCnt++)
	{
		// Skip if the type does not match
		if (legendInfo_Vector.at(iCnt).m_csType.CompareNoCase(L"INSERT")) continue;

		// Already processed
		if (!legendInfo_Vector.at(iCnt).m_csObject.CompareNoCase(csName)) 
		{
			//acutPrintf(L"...Matched 1");
			bMatched = true;
			break;
		}

		// Already processed
		if (!legendInfo_Vector.at(iCnt).m_csObject.CompareNoCase(csCounterName)) 
		{
			//acutPrintf(L"...Matched 2");
			bMatched = true;

			if (bIsExisting)
			{
				legendInfo_Vector[iCnt].m_bIsExisting = true;
				legendInfo_Vector[iCnt].m_csExistLayer    = csLayerName;
				legendInfo_Vector[iCnt].m_csDescription   = csDescription;
			}
			else
			{
				legendInfo_Vector[iCnt].m_bIsProposed = true;
				legendInfo_Vector[iCnt].m_csPropLayer = csLayerName;
			}

			break;
		}
	}

	if (bMatched == true) return true;
	
	// Add this INSERT to the LEGEND array as it qualifies for it.
	CLegendInfo legendInfo;
	/**/ if (bIsProposed) { legendInfo.m_bIsProposed = true; legendInfo.m_csPropLayer  = csLayerName; }
	else if (bIsExisting) { legendInfo.m_bIsExisting = true; legendInfo.m_csExistLayer = csLayerName; }

	legendInfo.m_csType				 = L"INSERT";
	legendInfo.m_csDescription = csDescription;
	legendInfo.m_csObject      = csName;

	// Get the ASCII value of the first character and add 1000 to it. This will put all the blocks behind the LTYPES and SORT the block names based 
	// on the first character. But we will still have to sort the block names further to handle sort on all characters though.
	legendInfo.m_iIndex = (int) legendInfo.m_csDescription.GetAt(0) + 1000;
	legendInfo_Vector.push_back(legendInfo);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Function name: createBlock()
// Description  : Creates a BLOCK definition of a line with the given linetype.
/////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : setTextString
// Description  : Places the given text in the given cell of a table.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setTextString(AcDbTable *pTable, int iRow, int iCol, AcDbObjectId objIDTxtStyle, AcDb::RotationAngle rotAngle, CString csText)
{
	pTable->setTextStyle(iRow, iCol, objIDTxtStyle);
	pTable->setTextString(iRow, iCol, L" " + csText); 
	pTable->setTextRotation(iRow, iCol, rotAngle);
	
	// Set the row alignment
	pTable->setAlignment(iRow, iCol, AcDb::kMiddleLeft);
}

//////////////////////////////////////////////////////////////////////////
// Function name: GetLTypeAndColorFromLayer()
// Description  : Retrieves the layer properties for a given layer.
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
// Function name: GetDescriptionForLayer()
// Description  :
//////////////////////////////////////////////////////////////////////////
bool GetDescriptionForLayer(CString csLayerName, CString &csLayerDesc)
{
	// FRS: Where the linear object layer does not have a layer description or where the block object does not have a LEGEND attribute,
	// that object is not included in the LEGEND.  
	csLayerDesc = L"";

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
	if (csLayerDesc == _T("(null)")) csLayerDesc = L"";
	// if (csLayerDesc.IsEmpty() || csLayerDesc == _T("(null)")) csLayerDesc = csLayerName;

	pLayerTblRecord->close();
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: ProcessLineForLegend
// Description  : Check if the given entity qualifies for a place in the legend. If YES, add it to the
//                array of qualified objects.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ProcessLineForLegend(AcDbEntity *pLine, std::vector <CLegendInfo> &legendInfo_Vector)
{
	// Add the linetype to the object array. If the linetype is "BYLAYER", get its linetype name from the layer
	CString csLayerName = pLine->layer();

	// If the layer name is not valid skip it
	if (csLayerName.Find(L"_") == -1) { pLine->close(); return 0; }

	// Check if the layer name has to be considered for legend
	if (csLayerName.Find(L"ANNO") != - 1) { pLine->close(); return 0; }

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

	// Get the description of the LAYER on which the entity resides. If the description is NOT specified, then this entity does not qualify for LEGEND
	CString csDescription; 
	if (!GetDescriptionForLayer(csLayerName, csDescription)) { pLine->close(); return -1; }
	if (csDescription.IsEmpty()) { pLine->close(); return 0; }

	// Status
	bool bIsProposed = false;
	bool bIsExisting = false;

	// Check if layer name has "_PROP" in its layer name. If YES, we assume that the object is a proposed item or else it is assumed to be an
	// existing item.
	CString csCounterLayerName = csLayerName;

	/**/ if ((csLayerName.Find(L"_PROP_") != -1) || (csLayerName.Find(L"_PROP") != -1))   
	{
		bIsProposed = true;
		csCounterLayerName.Replace(L"PROP", L"EXIST");
	}
	else if ((csLayerName.Find(L"_EXIST_") != -1) || (csLayerName.Find(L"_EXIST") != -1)) 
	{
		bIsExisting = true;
		csCounterLayerName.Replace(L"EXIST", L"PROP");
	}

	if (!bIsProposed && !bIsExisting) { pLine->close(); return 0; }

	CString csLType = pLine->linetype(); 
	pLine->close();
	csLType = csLType.MakeUpper();

	// Check if the LINE is already considered for legend
	bool bMatched = false;
	for (int iCtr = 0; iCtr < legendInfo_Vector.size(); iCtr++)
	{
		// If the information in objects array is not for a LINE, skip it
		if (legendInfo_Vector.at(iCtr).m_csType != _T("LINE")) continue;

		// Already present
		if (!legendInfo_Vector.at(iCtr).m_csObject.CompareNoCase(csLayerName)) { bMatched = true; break; }

		// Check if its counter information is present
		if (!legendInfo_Vector.at(iCtr).m_csObject.CompareNoCase(csCounterLayerName))
		{
			bMatched = true; 

			if (bIsExisting)
			{
				legendInfo_Vector[iCtr].m_bIsExisting   = true;
				legendInfo_Vector[iCtr].m_csExistLayer  = csLayerName;
				legendInfo_Vector[iCtr].m_csDescription = csDescription;
			}
			else
			{
				legendInfo_Vector[iCtr].m_bIsProposed = true;
				legendInfo_Vector[iCtr].m_csPropLayer = csLayerName;
			}
		
			break; 
		}

		// If this entity is EXISTING, check if this is already present
		if (bIsProposed && !legendInfo_Vector.at(iCtr).m_csPropLayer.CompareNoCase(csLayerName)) { bMatched = true; break; }

		// If this entity is EXISTING, we must check whether its PROPOSED layer is present
		if (bIsProposed && !legendInfo_Vector.at(iCtr).m_csExistLayer.CompareNoCase(csCounterLayerName)) 
		{
			bMatched = true; 

			legendInfo_Vector[iCtr].m_bIsProposed = true;
			legendInfo_Vector[iCtr].m_csPropLayer = csLayerName;

			break; 
		}
	}

	if (bMatched) return 0;

	// Add the details to the LEGEND Information array
	CLegendInfo legendInfo;
	legendInfo.m_bIsProposed = bIsProposed;
	legendInfo.m_bIsExisting = bIsExisting;

	if (bIsProposed) legendInfo.m_csPropLayer  = csLayerName;
	if (bIsExisting) legendInfo.m_csExistLayer = csLayerName;

	legendInfo.m_csObject      = csLayerName;
	legendInfo.m_csType			   = L"LINE";
	legendInfo.m_csDescription = csDescription;

	// Line types are required to be ordered like this
	CStringArray csaCableSortOrder; 
	csaCableSortOrder.Add(_T("SV"));   csaCableSortOrder.Add(_T("SL"));   csaCableSortOrder.Add(_T("LV"));  csaCableSortOrder.Add(_T("HV"));  csaCableSortOrder.Add(_T("AUX")); 
	csaCableSortOrder.Add(_T("TR33")); csaCableSortOrder.Add(_T("TR66")); csaCableSortOrder.Add(_T("TR132")); csaCableSortOrder.Add(_T("BASE"));

	CString csCableType = csLayerName;
	if (csCableType.Find(L"TR_") != -1)
		csCableType = L"TR" + csCableType.Mid(3, csCableType.Mid(3).Find(L"_"));
	else
		csCableType = csCableType.Mid(0, csCableType.Find(L"_"));

	CheckForDuplication(csaCableSortOrder, csCableType, legendInfo.m_iIndex); 

	legendInfo_Vector.push_back(legendInfo);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetObjectsForLegend()
// Description  : Get the details of LINES/INSERTS that qualifies for a place in the LEGEND output
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetObjectsForLegend(std::vector <CLegendInfo> &legendInfo_Vector, ads_point ptMin, ads_point ptMax, AcDbObjectIdArray &aryObjIds)
{
	// Select all inserts and lines placed within the boundary of this VPORT
	ads_name ssLegend;
	struct resbuf *rbpFilt = acutBuildList(-4, _T("<OR"), RTDXF0, _T("INSERT"), RTDXF0, L"SPLINE", RTDXF0, L"ARC", RTDXF0, _T("LWPOLYLINE"), RTDXF0, _T("LINE"), RTDXF0, _T("POLYLINE"), -4, _T("OR>"), 67, 0, NULL);

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

	// Go the model space of the viewport
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
	CStringArray csaBlocksSearched;
	//long lLength = 0L; acedSSLength(ssLegend, &lLength); 
	int lLength = 0L; acedSSLength(ssLegend, &lLength);
	for (long lCtr = 0L; lCtr < lLength; lCtr++)
	{
		// Get the object Id
		acedSSName(ssLegend, lCtr, enEntity);
		acdbGetObjectId(objEntity, enEntity);

		// Get the entity pointer
		es = acdbOpenObject(pEntity, objEntity, AcDb::kForRead);
		if (es != Acad::eOk) { appErrorTxt(__FILE__, __LINE__, acadErrorStatusText(es)); acedSSFree(ssLegend); return false; }

		// Call the appropriate functions to process INSERTS and LINES for legends
		if (pEntity->isKindOf(AcDbSpline::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc())) 
		{
			// Call the function to determine if this entity qualifies for LEGEND output
			if (ProcessLineForLegend(pEntity, legendInfo_Vector) == -1) { acedSSFree(ssLegend); return false; }
		}
		else if (pEntity->isKindOf(AcDbBlockReference::desc()))
		{
			// Get the block reference pointer for the entity
			AcDbBlockReference *pInsert = AcDbBlockReference::cast(pEntity);

			// Call the function to determine if this entity qualifies for LEGEND output
			if (!ProcessInsertForLegend(pInsert, legendInfo_Vector, csaBlocksSearched)) { acedSSFree(ssLegend); return false; }
		}
		else
		{
			// Close the entity processed
			pEntity->close();
		}
	}

	// Free the selection set
	acedSSFree(ssLegend);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name: PlaceThisLegendInTable()
// Description  : Writes the TEXTS for a given ROW in the LEGEND table.
//////////////////////////////////////////////////////////////////////////
void PlaceThisLegendInTable(AcDbTable *pTable, int iRow, CLegendInfo legendInfo, AcDbObjectId objIDTxtStyle)
{
	// If the information placed is for a LTYPE, get the line type assigned to the layer on which the line is.
	Adesk::Int16 iColor = 254;
	CString csName; // For a LINE, the LTYPE and for an INSERT its BLOCK NAME are used for a LEGEND

	/////////////////////////
	// Proposed/Existing
	/////////////////////////
	if (!legendInfo.m_csType.CompareNoCase(_T("LINE")))
	{
		// Get the appropriate layer on which the LINE is placed
		CString csLType;
		double dLineWt;

		// Call the function to determine the LTYPE used
		CString csLayer;

		if (legendInfo.m_bIsProposed)	csLayer = legendInfo.m_csPropLayer; else if (legendInfo.m_bIsExisting) csLayer = legendInfo.m_csExistLayer;
		if (GetLTypeAndColorFromLayer(csLayer, csLType, iColor, dLineWt)) 
		{
			csName = csLType; 
			createBlock(csName, iColor);
			csName.Format(L"%s-%d", csName, iColor);
		}
	}
	else if (!legendInfo.m_csType.CompareNoCase(L"INSERT")) 
	{
		// For "Existing" it is the block name, For "Proposed" it is "<Block name>_PROP"
		csName = legendInfo.m_csObject;
		/**/if (legendInfo.m_bIsProposed && (csName.Find(L"_PROP") == -1)) 
			csName += "_PROP";
		else if (legendInfo.m_bIsExisting && (csName.Find(L"_PROP") != -1))
			csName = csName.Mid(0, csName.Find(L"_PROP"));
	}

	// If the name is not EMPTY
	if (!csName.IsEmpty())
	{
		// Open the symbol table record for this name and get the object of this blocks definition
		AcDbBlockTable *pBlkTbl;
		acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlkTbl, AcDb::kForRead);

		AcDbObjectId objId; 
		Acad::ErrorStatus es = pBlkTbl->getAt(csName, objId); 
		if (es != Acad::eOk) 
		{
			pBlkTbl->close();
			return; 
		}
		pBlkTbl->close();

		// Set the block table record id to the given row and column
		if (es == Acad::eOk) 
		{
			if (!legendInfo.m_csType.CompareNoCase(_T("LINE")))
				pTable->setBlockTableRecordId(iRow + 2, (legendInfo.m_bIsProposed ? f_iProposedColIndex : f_iExistingColIndex), objId, false);
			else
				pTable->setBlockTableRecordId(iRow + 2, (legendInfo.m_bIsProposed ? f_iProposedColIndex : f_iExistingColIndex), objId, true);

			pTable->setCellType(iRow + 2, (legendInfo.m_bIsProposed ? f_iProposedColIndex : f_iExistingColIndex), AcDb::kBlockCell);
			pTable->setAlignment(AcDb::kMiddleCenter);

			// Apply respective scale to the cell
			if (!legendInfo.m_csType.CompareNoCase(_T("LINE")))
			{
				pTable->setScale(iRow + 2, (legendInfo.m_bIsProposed ? f_iProposedColIndex : f_iExistingColIndex), 0, (g_LegendLinearLen - 2) / 10);
			}
		}
		else
		{
			pTable->setCellType(iRow + 2, (legendInfo.m_bIsProposed ? f_iProposedColIndex : f_iExistingColIndex), AcDb::kTextCell);
			pTable->setTextString(iRow + 2, (legendInfo.m_bIsProposed ? f_iProposedColIndex : f_iExistingColIndex), L"X");
		}
	}

	// Place the description of the object if unmatched
	if (!legendInfo.m_bIsDescPlcd)
	{
		// If the description has "PROP" at the end, remove it before placement
		pTable->setTextString(iRow + 2, f_iDescriptColIndex, legendInfo.m_csDescription);
		setTextString(pTable, iRow + 2, f_iDescriptColIndex, objIDTxtStyle, AcDb::kDegrees000, legendInfo.m_csDescription);
	}

	// Set the cells top margin
	pTable->setMargin(iRow + 2, -1, AcDb::kCellMarginTop, 0.5);
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
		acedCommandS(RTSTR, _T(".STYLE"), RTSTR, csStyle, RTSTR, pszFont, RTREAL, dHeight, RTREAL, dWidthFactor, RTREAL, 0.0, RTSTR, _T("N"), RTSTR, _T("N"), NULL);
	}
	else if (!_tcsicmp(pszType, _T("SHX")))
	{
		// Shape based
		acedCommandS(RTSTR, _T(".STYLE"), RTSTR, csStyle, RTSTR, pszFont, RTREAL, dHeight, RTREAL, dWidthFactor, RTREAL, 0.0, RTSTR, _T("N"), RTSTR, _T("N"), RTSTR, _T("N"), NULL);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CreateEATextStyle
// Description  : From standard tables, reads the values for various parameters that defines a TEXT style.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateEATextStyle(double &dHeight)
{
	// Get the text values from tblText
	CQueryTbl tblText;
	if (!tblText.SqlRead(DSN_DWGTOOLS, _T("SELECT [Style], [Font], [Height], [WidthFactor], [ObliqueAngle], [TTForSHX] FROM tblText ORDER BY [ID]"), __LINE__, _T(__FILE__), __FUNCTION__,true)) return;

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

//////////////////////////////////////////////////////////////////////////
// Function name: setTextSizeInStyle()
// Description  : Sets the values for style parameters for a TEXT.
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: EditLegend
// Description  : Called when a LEGEND output is selected instead of VPORTS. 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool EditLegend(std::vector <CLegendInfo> &legendInfo_Vector, struct resbuf *rbpXD, AcDbObjectId objLegend)
{
	f_iProposedColIndex = -1;
	f_iExistingColIndex = -1;
	f_iDescriptColIndex = -1;

	// Get the XDATA "LEGENDTYPE" from the LEGEND to determine the LEGEND type and its Legend ID
	ads_name enLegend; acdbGetAdsName(enLegend, objLegend);
	struct resbuf *rbpLegendType = getXDataFromEntity(enLegend, L"LEGENDTYPE");
	if (rbpLegendType == NULL) return false;

	// Get the Legend ID
	int iLegendType 	 = rbpLegendType->rbnext->resval.rint;
	CString csLegendID = rbpLegendType->rbnext->resval.rstring;

	// Sort the information in the legend
	sortLegendInfo(legendInfo_Vector);

	// Open the table to determine the locations of columns we are interested
	AcDbTable *pTable;
	if (acdbOpenObject(pTable, objLegend, AcDb::kForWrite) != Acad::eOk) return false;

	for (int iCol = 0; iCol < pTable->numColumns(); iCol++)
	{
		/**/ if (!wcscmp(pTable->textString(1, iCol), L"PROPOSED")) f_iProposedColIndex = iCol;
		else if (!wcscmp(pTable->textString(1, iCol), L"EXISTING")) f_iExistingColIndex = iCol;
		else if (!wcscmp(pTable->textString(1, iCol), L"DESCRIPTION")) f_iDescriptColIndex = iCol;
	}

	if ((f_iProposedColIndex == -1) || (f_iExistingColIndex == -1) || (f_iDescriptColIndex == -1))
	{
		appMessage(L"Could not find all the valid column headers in the table selected.");
		pTable->close();
		return false;
	}

	// Get the values in other columns (added by the user)
	CStringArray csaOtherColumns;
	CString csColumn;
	for (int iRow = 2; iRow < pTable->numRows(); iRow++)
	{
		for (int iCol = 0; iCol < pTable->numColumns(); iCol++)
		{
			// Not interested to save the values in interested columns
			if ((iCol == f_iProposedColIndex) || (iCol == f_iExistingColIndex) || (iCol == f_iDescriptColIndex)) continue;

			// <Column Name>-<Description>#<Value>
			csColumn.Format(L"%s-%s#%s", pTable->textString(1, iCol), pTable->textString(iRow, f_iDescriptColIndex), pTable->textString(iRow, iCol)); 
			csaOtherColumns.Add(csColumn);
		}
	}

	// Set the number of rows for the table
	int iNoOfRows = getNumberOfRows(legendInfo_Vector, iLegendType);
	pTable->deleteRows(2, pTable->numRows() - 2);
	pTable->setSize(iNoOfRows + 2, pTable->numColumns());
		
	int iCtr = 0;
	for (int iRow = 0; iRow < legendInfo_Vector.size(); iRow++)
	{
		if (legendInfo_Vector[iRow].m_bIsDescPlcd) continue;

		/**/ if ((iLegendType == 1) && (legendInfo_Vector.at(iRow).m_iIndex > 1000)) { break; }  // Only LINES
		else if ((iLegendType == 2) && (legendInfo_Vector.at(iRow).m_iIndex < 1000)) { continue; } // Only INSERTS

		PlaceThisLegendInTable(pTable, iCtr, legendInfo_Vector.at(iRow), NULL);
		legendInfo_Vector[iRow].m_bIsDescPlcd = true;

		iCtr++;
	}

	// Check if the table has appropriate records in it. If NO, do not place it and delete it
	if (pTable->numRows() <= 2)	{ pTable->erase(); pTable->close();	return true; }

	// Update the other columns with its previous values
	CString csValue;
	for (int iRow = 2; iRow < pTable->numRows(); iRow++)
	{
		for (int iCol = 0; iCol < pTable->numColumns(); iCol++)
		{
			// Not interested to modify the values in interested columns
			if ((iCol == f_iProposedColIndex) || (iCol == f_iExistingColIndex) || (iCol == f_iDescriptColIndex)) continue;

			csColumn.Format(L"%s-%s", pTable->textString(1, iCol), pTable->textString(iRow, f_iDescriptColIndex));
			GetParameterValue(csaOtherColumns, csColumn, csValue, -1);
			pTable->setTextString(iRow, iCol, csValue);
		}
	}

	// Add XDATA
	pTable->setXData(rbpXD);
	pTable->close();

	return true;
}


//////////////////////////////////////////////////////////////////
// Function name: SetTableStyle
// Description  : Sets the TABLE style for the LEGEND output.
//////////////////////////////////////////////////////////////////
void SetTableStyle(AcDbTable *pTableLegend, int iRows)
{
	pTableLegend->setLayer(_T("XT_BASE_SHEET_LEGEND_EXIST"));
	pTableLegend->setSize(iRows, 3); // The methods setNumColumns() and setNumRows() have been deprecated beyond AutoCAD 2007.
	pTableLegend->setColumnWidth(g_LegendLinearLen);
	pTableLegend->setRowHeight(4.0);
	pTableLegend->setFlowDirection(AcDb::kTtoB);
	pTableLegend->setLinetype(L"CONTINUOUS");

	// Before setting the "Standard" text style for the table, set the text size in it to 0.0. Just in case someone sets it to nonzero value
	double dTxtSizeData;
	double dTxtSizeHead;
	double dTxtSizeTitl;
	AcDbObjectId objIDTxtStyleData;	if (setTextSizeInStyle(L"eCapture_Data",   0.0, dTxtSizeData, objIDTxtStyleData)) { pTableLegend->setTextStyle(objIDTxtStyleData); }
	AcDbObjectId objIDTxtStyleHead; if (setTextSizeInStyle(L"eCapture_Header", 0.0, dTxtSizeHead, objIDTxtStyleHead)) { pTableLegend->setTextStyle(objIDTxtStyleHead); }
	AcDbObjectId objIDTxtStyleTitl; if (setTextSizeInStyle(L"eCapture_Title",  0.0, dTxtSizeTitl, objIDTxtStyleTitl)) { pTableLegend->setTextStyle(objIDTxtStyleTitl); }

	//////////////////////////////////////////////////////////////////////////
	// Modify the "Standard" table style
	//////////////////////////////////////////////////////////////////////////

	AcDbDictionary *pDict = NULL; 
	AcDbObjectId styleId;      
	acdbHostApplicationServices()->workingDatabase()->getTableStyleDictionary(pDict, AcDb::kForRead);  
	Acad::ErrorStatus es;
	pDict->getAt(_T("Standard"), styleId); 
	pDict->close();

	// Modify the table details for our requirements
	AcDbTableStyle *pTableStyleTbl;
	if (acdbOpenObject(pTableStyleTbl, styleId, AcDb::kForWrite) == Acad::eOk)
	{
		pTableStyleTbl->setTextStyle(objIDTxtStyleData, AcDb::kDataRow);
		pTableStyleTbl->setTextStyle(objIDTxtStyleHead, AcDb::kHeaderRow);
		pTableStyleTbl->setTextStyle(objIDTxtStyleTitl, AcDb::kTitleRow);

		pTableStyleTbl->setTextHeight(3.5, AcDb::kTitleRow);
		pTableStyleTbl->setTextHeight(2.5, AcDb::kHeaderRow);
		pTableStyleTbl->setTextHeight(2.0, AcDb::kDataRow);

		pTableStyleTbl->setFlowDirection(AcDb::kTtoB);
		pTableStyleTbl->setAlignment(AcDb::kMiddleCenter);

		// *** NOT WORKING THOUGH ***
		// pTableStyleTbl->setMargin(AcDb::kCellMarginHorzSpacing, 2.0, L"Data");

		// Setting INVISIBLE to the grid to hide it
		pTableStyleTbl->setGridVisibility(AcDb::kInvisible);
		pTableStyleTbl->close();
	}

	pTableLegend->setTableStyle(styleId);
	pTableLegend->setAlignment(AcDb::kMiddleCenter, AcDb::kAllRows);
	pTableLegend->generateLayout(); // Very very important, else expect crashes later on

	// Headers
	pTableLegend->setTextString(0, 0, _T("LEGEND"));
	pTableLegend->mergeCells(0, 0, 0, 2);

	pTableLegend->setTextString(1, 0, _T("PROPOSED")); pTableLegend->setColumnWidth(0, 22.0); 
	pTableLegend->setTextString(1, 1, _T("EXISTING")); pTableLegend->setColumnWidth(1, 22.0);
	pTableLegend->setTextString(1, 2, _T("DESCRIPTION"));	pTableLegend->setColumnWidth(2, 30.0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: PlaceLegend()
// Description  : 
// Arguments    : 1. CStringArray &, Holds the block name or layer name of the object found suitable for LEGEND o/p.
//                2. CStringArray &, Holds the layer names of these objects.
//                3. CStringArray &, Holds the entity type of the objects.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PlaceLegend(std::vector <CLegendInfo> &legendInfo_Vector, struct resbuf *rbpXD, int iLegendType)
{
	// Predefined column locations for NEW BOM generation
	f_iProposedColIndex = 0;
	f_iExistingColIndex = 1;
	f_iDescriptColIndex = 2;

	// Generate the UNIQUE ID for this LEGEND output
	CTime tmStart(CTime::GetCurrentTime());
	CString csLegendID; csLegendID.Format(L"LEGEND-%s", tmStart.Format(L"%H%M%S%d%m%Y"));
	acdbRegApp(csLegendID);
	struct resbuf *rbpLegendID = acutBuildList(AcDb::kDxfRegAppName, csLegendID, AcDb::kDxfXdInteger16, 1, NULL); 

	// Keep the LEGENDTYPE XDATA ready. This will be used to set the XDATA used for Editing Legend option
	acdbRegApp(L"LEGENDTYPE");
	struct resbuf *rbpLegendType = acutBuildList(AcDb::kDxfRegAppName, L"LEGENDTYPE", AcDb::kDxfXdInteger16, iLegendType, AcDb::kDxfXdAsciiString, csLegendID, NULL); 

	// Sort the information in the legend
	sortLegendInfo(legendInfo_Vector);
	
	//////////////////////////////////////////////////////////////////////////
	// Placing LEGEND as table
	//////////////////////////////////////////////////////////////////////////
	AcDbTable *pTableLegend = new AcDbTable();

	pTableLegend->setXData(rbpLegendType);
	pTableLegend->setXData(rbpLegendID);
	acutRelRb(rbpLegendType);

	// Set the tables format
	int iNoOfRows = getNumberOfRows(legendInfo_Vector, iLegendType);
	
	SetTableStyle(pTableLegend, iNoOfRows + 2);

	// Add the table to the drawing
	AcDbBlockTable *pBlkTbl = NULL;
	if (acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlkTbl, AcDb::kForRead) != Acad::eOk) { pTableLegend->close(); return false; }

	AcDbBlockTableRecord *pBlkTblRcd = NULL;
	if (pBlkTbl->getAt( ACDB_PAPER_SPACE, pBlkTblRcd, AcDb::kForWrite) != Acad::eOk) { pBlkTbl->close(); pTableLegend->close(); return false; }
	pBlkTbl->close();

	double dTableWidth  = pTableLegend->width();
	double dTableHeight = pTableLegend->height();
	pBlkTblRcd->appendAcDbEntity(pTableLegend);

	AcDbObjectId objLegend = pTableLegend->objectId();
	pBlkTblRcd->close();

	// Get on with the placement of items
	int iRow = 0;
	int iCtr = 0;
	for (; iCtr < legendInfo_Vector.size(); iCtr++)
	{
		if (legendInfo_Vector[iCtr].m_bIsDescPlcd) continue;

		/**/ if ((iLegendType == 1) && (legendInfo_Vector.at(iCtr).m_iIndex > 1000)) { break; }  // Only LINES
		else if ((iLegendType == 2) && (legendInfo_Vector.at(iCtr).m_iIndex < 1000)) { continue; } // Only INSERTS

		PlaceThisLegendInTable(pTableLegend, iRow, legendInfo_Vector[iCtr], NULL);
		legendInfo_Vector[iCtr].m_bIsDescPlcd = true;
		
		iRow++;
	}

	// Check if the table has appropriate records in it. If NO, do not place it and delete it
	if (pTableLegend->numRows() <= 2)	{ pTableLegend->erase(); pTableLegend->close();	return true; }

	// Get the geometric extents of the table already placed
	int iRet;
	ads_point ptLegend;
	acedInitGet(RSG_NONULL, _T(""));
	while (T)
	{
		iRet = acedGetPoint(NULL, _T("\nSpecify insertion point: "), ptLegend);
		if (iRet == RTCAN) { pTableLegend->erase(); pTableLegend->close(); return false; }
		else if (iRet == RTNORM) break;
	}

	pTableLegend->setPosition(AcGePoint3d(ptLegend[X], ptLegend[Y], 0.0));
	pTableLegend->setAlignment(AcDb::kMiddleCenter, AcDb::kAllRows);

	// Add the LEEGEND XDATA
	acdbRegApp(L"LEGEND");
	pTableLegend->setXData(rbpXD);
	
	// Add the LEGEND type XDATA to identify whether the LEGEND is for INSERT or LINES
	pTableLegend->close(); 
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CollectPreselectedVPortHandles()
// Description  : Returns the list of VPORT	handles those are PRE-SELECTED by user.
//////////////////////////////////////////////////////////////////////////////////////////////////
void CollectPreselectedVPortHandles(CStringArray &csaPickedHandles)
{
	// Check if there are objects already selected
	//long lPicked = 0L; 
	int lPicked = 0L;
	ads_name ssPickFirst;

	if (acedSSGet(_T("I"), NULL, NULL, NULL, ssPickFirst) != RTNORM) return;

	// Get the number of objects selected
	if ((acedSSLength(ssPickFirst, &lPicked) != RTNORM) || (lPicked == 0L)) return;

	// Get the VPORTS handles and add it to the handles array
	AcDbObjectId objPicked;
	ads_name enPicked;
	CString csHandle;
	struct resbuf *rbpPicked = NULL;
	AcDbViewport *pVport;

	for (long lCtr = 0L; lCtr < lPicked; lCtr++)
	{
		acedSSName(ssPickFirst, lCtr, enPicked);
		acdbGetObjectId(objPicked, enPicked);

		if (acdbOpenObject(pVport, objPicked, AcDb::kForRead) != Acad::eOk) continue;
		if (pVport)
		{
			AcDbHandle handle; pVport->getAcDbHandle(handle); 
			TCHAR handleStr[256]; handle.getIntoAsciiBuffer(handleStr);
			csaPickedHandles.Add(handleStr);

			pVport->close();
		}
	}

	// Free the selection set
	acedSSFree(ssPickFirst);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GenerateLegendForPreSelections
// Description  : Checks for any pre-selected VPORTS and generates LEGEND for these.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GenerateLegendForPreSelections(CStringArray &csaHandles, std::vector <CLayoutInfo> &layoutInfoVector)
{
	// Get the current layout name
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
	CString csLayout; csLayout.Format(L"%s", pLayoutMngr->findActiveLayout(false));

	AcDbObjectId objHandle;
	ads_name enHandle;
	AcDbViewport *pVport;
	AcDbObjectIdArray aryObjIds;
	CStringArray csaBlocksSearched;
	std::vector <CLegendInfo> legendInfo_Vector;

	for (int i = 0; i < csaHandles.GetSize(); i++)
	{
		// Get the view port entity name and its object ID for its handle
		acdbHandEnt(csaHandles.GetAt(i), enHandle);
		acdbGetObjectId(objHandle, enHandle);

		// Open the view port and get the layers frozen in it
		int iCVNumber;
		if (acdbOpenObject(pVport, objHandle, AcDb::kForRead) != Acad::eOk) continue;

		// Switch to paper space
		acedMspace();

		// Set CVPORT variable to the number of your viewport to ensure that your viewport is active
		struct resbuf rbSetVar; rbSetVar.restype = RTSHORT;
		iCVNumber = rbSetVar.resval.rint = pVport->number();
		acedSetVar(_T("CVPORT"), &rbSetVar);

		// Get all the frozen layers in this view port
		pVport->getFrozenLayerList(aryObjIds);

		// Get the lower and upper left corners of the viewport
		AcGePoint2d geView = pVport->viewCenter(); 
		double dHeight		 = pVport->viewHeight();
		double dWidthPS    = pVport->width();
		double dHeightPS   = pVport->height();
		double dWidth      = dHeight * dWidthPS / dHeightPS;
		pVport->close();

		// Get the objects for Legend
		ads_point ptCen = { asDblArray(geView)[X], asDblArray(geView)[Y], 0.0 }; // This center point will work only if WCS is used 

		// Get the view center and view twist angle for this viewport
		acedMspace();
		acedSetVar(_T("CVPORT"), &rbSetVar);
		struct resbuf rbVCtr; acedGetVar(_T("VIEWCTR"),   &rbVCtr); acutPolar(rbVCtr.resval.rpoint, 0.0, 0.0, ptCen);
		struct resbuf rbVAng; acedGetVar(_T("VIEWTWIST"), &rbVAng);
		acedPspace();

		saveUCSMode();

		ads_point ptMin = { ptCen[X] - (dWidth / 2), ptCen[Y] - (dHeight / 2), 0.0 };
		ads_point ptMax = { ptCen[X] + (dWidth / 2), ptCen[Y] + (dHeight / 2), 0.0 };

		restoreUCSMode();

		// If the coordinate system is not WCS, then we may have to rotate these points 
		AcGeVector3d geMin(ptMin[X], ptMin[Y], 0.0); geMin.rotateBy(DTR(rbVAng.resval.rreal), AcGeVector3d(ptCen[X], ptCen[Y], 0.0)); acutPolar(asDblArray(geMin), 0.0, 0.0, ptMin);
		AcGeVector3d geMax(ptMax[X], ptMax[Y], 0.0); geMax.rotateBy(DTR(rbVAng.resval.rreal), AcGeVector3d(ptCen[X], ptCen[Y], 0.0)); acutPolar(asDblArray(geMax), 0.0, 0.0, ptMax);

		// Go get all the objects to place in the legend
		GetObjectsForLegend(legendInfo_Vector, ptMin, ptMax, aryObjIds);
	}

	// Check if there are any information qualifying a legend birth
	if (legendInfo_Vector.size() <= 0) { acutPrintf(_T("\nThere are no information for placing in LEGEND. Exiting command!")); return true; }

	acedPspace();

	// Start the transaction
	acTransactionManagerPtr()->startTransaction();

	// Add the XDATA to the LEGEND generated. THis XDATA will have details of all the VPORTS selected for LEGEND generated. This information is used when the user
	// selects the LEGEND for editing.
	CString csParameter;
	CString csVPort;
	for (int iH = 0; iH < csaHandles.GetSize(); iH++) { csVPort += (L";" + csaHandles.GetAt(iH)); }
	csParameter.Format(L"%s#%s", csLayout, csVPort);
	struct resbuf *rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"LEGEND", AcDb::kDxfXdAsciiString, csParameter, NULL);

	// Place the legend
	if (!PlaceLegend(legendInfo_Vector, rbpXD, 0)) { acTransactionManagerPtr()->abortTransaction();  acutRelRb(rbpXD); return false; }

	// End the transaction
	acTransactionManagerPtr()->endTransaction();

	acutRelRb(rbpXD); 

	// Regenerate the details
	acedCommandS(RTSTR, L".REGEN", NULL);
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Function name: Command_Legend()
// Description  : Defines the function that is called by "".
///////////////////////////////////////////////////////////////////////////////////////////////
void Command_Legend()
{
	// Switch off certain system variables
	switchOff();

	// Get the handles if VPORTS already selected (Provided the layout is in paper space)
	CStringArray csaPickedHandles; 
	struct resbuf rbTileMode;
	acedGetVar(L"TILEMODE", &rbTileMode);
	if (!rbTileMode.resval.rint) CollectPreselectedVPortHandles(csaPickedHandles);

	// Assign the component names for proposed and existing objects
	g_csCmpTxtForExisting = _T("_EXIST");
	g_csCmpTxtForProposed = _T("_PROP");

	// Ensure that the layer for this block is created and current
	createLayer(_T("XT_BASE_SHEET_LEGEND_EXIST"), Adesk::kFalse);

	// If in Model space, switch to Paper Space
	rbTileMode.restype = RTSHORT; rbTileMode.resval.rint = 0; acedSetVar(_T("TILEMODE"), &rbTileMode);

	// Set the current VPORT to 1
	struct resbuf rbCVPort; acedGetVar(_T("CVPORT"), &rbCVPort); if (rbCVPort.resval.rint != 1) { acedCommandS(RTSTR, _T(".PSPACE"), NULL); }

	// Get the list of paper space layouts and sort them based on their tab order
	std::vector <CLayoutInfo> layoutInfo_Vector;
	CStringArray csaLayouts, csaVPorts, csaVPortsCnt;

	// Retrieves the names of all PAPER SPACE layouts and sort it based on tab order
	if (!GetLayoutNamesInPaperSpace(layoutInfo_Vector)) return;

	// Process the pre-selected VPORTS exit this function
	if (csaPickedHandles.GetCount() > 0) 
	{
		if (GenerateLegendForPreSelections(csaPickedHandles, layoutInfo_Vector)) return;
	}

	// Allow selection of an existing LEGEND
	ads_name enLegend;
	int iRet;
	struct resbuf *rbpLegend = NULL;
	ads_point ptDummy;
	CLegendNotificationDlg dlgLegend;

	while (T)
	{
		iRet = acedEntSel(L"\nENTER to create or select Legend to EDIT: ", enLegend, ptDummy);
		/**/ if (iRet == RTCAN) return;
		else if (iRet == RTNORM)
		{
			// Check if LEGEND XDATA is attached
			rbpLegend = getXDataFromEntity(enLegend, L"LEGEND");
			if (rbpLegend == NULL) { appMessage(L"Select a valid LEGEND"); continue; }

			// Get the XDATA "LEGENDTYPE" from the LEGEND to determine the LEGEND type and its Legend ID
			struct resbuf *rbpLegendType = getXDataFromEntity(enLegend, L"LEGENDTYPE");
			if (rbpLegendType == NULL) return;

			dlgLegend.m_iLegendType = rbpLegendType->rbnext->resval.rint;
			acutRelRb(rbpLegendType);
			break;
		}
		else if (iRet == RTERROR)
		{
			// Fresh LEGEND to be created
			break;
		}
	}
				
	// For each layout in the layout info vector, retrieve the number of view ports in it
	ads_name ssGet; 
	CString csValue;
	//long lLength;
	int lLength;
	struct resbuf *rbpFilt;

	for (int iCtr = 0; iCtr < layoutInfo_Vector.size(); iCtr++) 
	{
		csaLayouts.Add(layoutInfo_Vector.at(iCtr).m_csLayoutName); 
		csaVPorts.Add(L"");

		// Get the number of VPORTS in the layout
		rbpFilt = acutBuildList(RTDXF0, L"VIEWPORT", 410, layoutInfo_Vector.at(iCtr).m_csLayoutName, NULL);
		if (acedSSGet(L"X", NULL, NULL, rbpFilt, ssGet) == RTNORM)
		{
			lLength = 0L; acedSSLength(ssGet, &lLength); 

			csValue.Format(L"%ld", lLength - 1L);
			csaVPortsCnt.Add(csValue);
		}
		else
			csaVPortsCnt.Add("0");

		acedSSFree(ssGet);
		acutRelRb(rbpFilt);
	}

	if (rbpLegend)
	{
		// If the user has selected a valid LEGEND object
		dlgLegend.m_iCalledFor = 2;

		// Get the list of VPORTS previously selected
		CStringArray csaLegend;
		while (rbpLegend->rbnext)	{	rbpLegend = rbpLegend->rbnext; csaLegend.Add(rbpLegend->resval.rstring); }

		// Set the layout values
		CString csValue;
		for (int iLay = 0; iLay < csaLayouts.GetCount(); iLay++)
		{
			GetParameterValue(csaLegend, csaLayouts.GetAt(iLay), csValue, -1);

			if (!csValue.IsEmpty())
			{
				csaVPorts.SetAt(iLay, csValue);	
				int iCnt = 1;
				while (csValue.Find(L";") != -1) { iCnt++; csValue = csValue.Mid(csValue.Find(L";") + 1); }
			}
		}
	}
	else dlgLegend.m_iCalledFor = 1;

	while (T)
	{
		// Pass on the information of layouts and its view ports handles to the dialog
		dlgLegend.m_csaLayouts.Copy(csaLayouts);
		dlgLegend.m_csaVports.Copy(csaVPorts);
		dlgLegend.m_csaVportsCnt.Copy(csaVPortsCnt);

		if (dlgLegend.DoModal() == IDCANCEL) { acedCommandS(RTSTR, L".REGEN", NULL); return; }

		csaLayouts.Copy(dlgLegend.m_csaLayouts);
		csaVPorts.Copy(dlgLegend.m_csaVports);
		
		if (dlgLegend.m_iCurrentLayout >= 0)
		{
			AcDbObjectId objId; 
			AcDbViewport *pVPort;
			CStringArray csaVPortsInLayout;
			ads_point ptDummy;
			ads_name enVPort;
			int iRet;
			CString csHandle;
			CStringArray csaHandles;
			CString csVPort;

			// Retrieve the already selected VPORT handles to the handles array
			csVPort = dlgLegend.m_csaVports.GetAt(dlgLegend.m_iCurrentLayout);

			while (csVPort.Find(L";") != -1) 
			{
				// Eliminate the delimiter in front of the string
				if (csVPort[0] == L';') { csVPort = csVPort.Mid(1); continue; }

				csHandle = csVPort.Mid(0, csVPort.Find(L";")); 
				csVPort  = csVPort.Mid(csVPort.Find(L";") + 1);

				csaHandles.Add(csHandle);
			}

			if (!csVPort.IsEmpty()) csaHandles.Add(csVPort);

			// Get the viewport handles in the current layout. Required to check if a non-rectangular viewport is selected
			AcDbObjectIdArray aryClipObjIds;
			AcDbObjectIdArray aryVPObjIds;
			int iLen = getVPortClipHandles(dlgLegend.m_csLayout, aryClipObjIds, aryVPObjIds);

			while (T)
			{
				iRet = acedEntSel(_T("\nSelect viewport: "), enVPort, ptDummy);

				/**/ if (iRet == RTCAN) return;
				else if (iRet == RTERROR)
				{
					// Add all the handles collected into one string
					csHandle.Empty();
					for (int iVPort = 0; iVPort < csaHandles.GetSize(); iVPort++) csHandle.Format(L"%s;%s", csHandle, csaHandles.GetAt(iVPort));

					// Set the resultant handle to the array
					csaVPorts.SetAt(dlgLegend.m_iCurrentLayout, csHandle);
					break;
				}
				else if (iRet == RTNORM)
				{
					// Open the selected entity assuming that it is a viewport that is selected
					acdbGetObjectId(objId, enVPort);
					acdbOpenObject(pVPort, objId, AcDb::kForRead);

					// If the selected entity is a non-rectangular viewport we need to check whether the entity selected is used as a clip entity for a viewport
					if (pVPort == NULL)	{	appMessage(L"\nThe selected entity is not a valid viewport.", 00); continue; }

					bool bValidEntity = false;
					if (aryClipObjIds.length())
					{
						int iCtr;

						// If the selected entity is a polygonal viewport this could happen. Check whether the selected entity belongs to any one of the following 
						// AcDbCircle, AcDbPolyline, AcDb2dPolyline, AcDb3dPolyline, AcDbEllipse,	AcDbRegion, AcDbSpline, AcDbFace. As anyone these types can be used to clip the viewport view
						AcDbEntity *pEntChk;
						acdbOpenObject(pEntChk, objId, AcDb::kForRead);

						if (pEntChk->isKindOf(AcDbCircle::desc())  || pEntChk->isKindOf(AcDbPolyline::desc()) || pEntChk->isKindOf(AcDb2dPolyline::desc()) ||
							  pEntChk->isKindOf(AcDbEllipse::desc()) || pEntChk->isKindOf(AcDbRegion::desc())   || pEntChk->isKindOf(AcDbSpline::desc())
							 )
						{
							// Check if the object selected is used as a clip entity for a viewport
							for (iCtr = 0; iCtr < aryClipObjIds.length(); iCtr++)	{ if (aryClipObjIds.at(iCtr) == pEntChk->objectId()) { bValidEntity = true; break; }}
						}
						pEntChk->close();

						if (!bValidEntity)
						{
							appMessage(L"\nThe selected entity is not a valid viewport.", 00);	
							continue; 
						}
						else
						{
							objId = aryVPObjIds.at(iCtr);
							acdbOpenObject(pVPort, objId, AcDb::kForRead);
							acdbGetAdsName(enVPort, objId);
						}
					} 

					bool bIsOn = pVPort->isOn();
					pVPort->close();

					// If the VPORT is not on, there is no point including it as no entities in it are visible
					if (!bIsOn) { appMessage(L"\nThe selected viewport information is OFF and will not be included for legend.", 00);	continue; }

					// Get the viewport handle and add it to the array
					struct resbuf *rbpGet = acdbEntGet(enVPort);
					if (rbpGet)
					{
						int iAtIndex;
						if (!CheckForDuplication(csaHandles, Assoc(rbpGet,5)->resval.rstring, iAtIndex)) 
						{
							csaHandles.Add(Assoc(rbpGet,5)->resval.rstring);

							// Highlight the viewport selected
							acedRedraw(enVPort, 3);
						}
						else 
						{
							// This VIEWPORT was selected earlier. So remove it from the selection.
							csaHandles.RemoveAt(iAtIndex);
							acedRedraw(enVPort, 4);
						}
						acutRelRb(rbpGet);
					}
				}
			}
		}
		else break;
	}

	// Get the details of the objects for LEGEND
	CStringArray csaSearched;
	std::vector <CLegendInfo> legendInfo_Vector;

	ads_point ptMin;
	ads_point ptMax;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// For each layout, get the view port handles selected for legend, get the window extents, get the frozen layer list in the viewport
	// call the function to collect all objects eligible for legend.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
		
	CString csLayout;
	CString csVPort;

	for (int iCtr = 0; iCtr < dlgLegend.m_csaLayouts.GetSize(); iCtr++)
	{
		// Get the VPORT handles selected
		csVPort = dlgLegend.m_csaVports.GetAt(iCtr);
		
		// Get the viewport object id's selected
		int iCnt = 0;
		while (csVPort.Find(L";") != -1) { csVPort = csVPort.Mid(csVPort.Find(L";") + 1); iCnt++; }

		// If nothing selected, go to the next one
		if (!iCnt) { continue; } 

		// If this layout is different from layout iterated, set it as current
		csLayout = dlgLegend.m_csaLayouts.GetAt(iCtr);
		//Commented for ACAD 2018
		//AcDbLayout *pLayout = pLayoutMngr->findLayoutNamed(csLayout, false);
		AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(csLayout);
		AcDbLayout *pLayout = NULL;
		acdbOpenObject(pLayout, objLayoutId, AcDb::kForRead);
		if (pLayout) { Acad::ErrorStatus es = pLayoutMngr->setCurrentLayout(csLayout); pLayout->close(); }

		// Zoom big
		acedCommandS(RTSTR, L".ZOOM", RTSTR, L"E", NULL);

		// Get the object ID of frozen layers in the VPORT selected
		CStringArray csaHandles;
		CString csHandle;
		csVPort = dlgLegend.m_csaVports.GetAt(iCtr);
		while (csVPort.Find(L";") != -1) 
		{
			csVPort  = csVPort.Mid(csVPort.Find(L";") + 1); 
			csHandle = csVPort;
			if (csHandle.Find(L";") != -1) { csHandle = csHandle.Mid(0, csHandle.Find(L";")); }
			csaHandles.Add(csHandle);
		}
	
		// For all the VPORTs selected, collect the entities within it that qualifies for a place in the LEGEND
		ads_name enHandle;
		AcDbObjectId objHandle;
		AcDbViewport *pVport;
		AcDbObjectIdArray aryObjIds;

		for (int i = 0; i < csaHandles.GetSize(); i++)
		{
			// Get the view port entity name and its object ID for its handle
			acdbHandEnt(csaHandles.GetAt(i), enHandle);
			acdbGetObjectId(objHandle, enHandle);

			// Open the view port and get the layers frozen in it
			int iCVNumber;
			if (acdbOpenObject(pVport, objHandle, AcDb::kForRead) != Acad::eOk) continue;

			// Switch to paper space
			acedMspace();

			// Set CVPORT variable to the number of your viewport to ensure that your viewport is active
			struct resbuf rbSetVar; rbSetVar.restype = RTSHORT;
			iCVNumber = rbSetVar.resval.rint = pVport->number();
			acedSetVar(_T("CVPORT"), &rbSetVar);

			// Get all the frozen layers in this view port
			pVport->getFrozenLayerList(aryObjIds);

			// Get the lower and upper left corners of the viewport
			AcGePoint2d geView = pVport->viewCenter(); 
			double dHeight		 = pVport->viewHeight();
			double dWidthPS    = pVport->width();
			double dHeightPS   = pVport->height();
			double dWidth      = dHeight * dWidthPS / dHeightPS;
			pVport->close();

			// Get the objects for Legend
			ads_point ptCen = { asDblArray(geView)[X], asDblArray(geView)[Y], 0.0 }; // This center point will work only if WCS is used 

			// Get the view center and view twist angle for this viewport
			acedMspace();
			acedSetVar(_T("CVPORT"), &rbSetVar);
			struct resbuf rbVCtr; acedGetVar(_T("VIEWCTR"),   &rbVCtr); acutPolar(rbVCtr.resval.rpoint, 0.0, 0.0, ptCen);
			struct resbuf rbVAng; acedGetVar(_T("VIEWTWIST"), &rbVAng);
			acedPspace();

			saveUCSMode();

			ads_point ptMin = { ptCen[X] - (dWidth / 2), ptCen[Y] - (dHeight / 2), 0.0 };
			ads_point ptMax = { ptCen[X] + (dWidth / 2), ptCen[Y] + (dHeight / 2), 0.0 };

			restoreUCSMode();

			// If the coordinate system is not WCS, then we may have to rotate these points 
			AcGeVector3d geMin(ptMin[X], ptMin[Y], 0.0); geMin.rotateBy(DTR(rbVAng.resval.rreal), AcGeVector3d(ptCen[X], ptCen[Y], 0.0)); acutPolar(asDblArray(geMin), 0.0, 0.0, ptMin);
			AcGeVector3d geMax(ptMax[X], ptMax[Y], 0.0); geMax.rotateBy(DTR(rbVAng.resval.rreal), AcGeVector3d(ptCen[X], ptCen[Y], 0.0)); acutPolar(asDblArray(geMax), 0.0, 0.0, ptMax);

			// Go get all the objects to place in the legend
			GetObjectsForLegend(legendInfo_Vector, ptMin, ptMax, aryObjIds);
		}
	} 

	// Check if there are any information qualifying a legend birth
	if (legendInfo_Vector.size() <= 0) { acutPrintf(_T("\nThere are no information for placing in LEGEND. Exiting command!")); return; }

	acedPspace();

	// Start the transaction
	acTransactionManagerPtr()->startTransaction();

	// Format the XDATA to add to the LEGEND. This XDATA will have details of all the VPORTS selected for LEGEND generated. This information 
	// is used when the user selects the LEGEND for editing.
	CString csParameter;
	struct resbuf *rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"LEGEND", NULL);
	struct resbuf *rbpTemp;

	for (int iCtr = 0; iCtr < dlgLegend.m_csaLayouts.GetCount(); iCtr++)
	{
		csParameter.Format(L"%s#%s", dlgLegend.m_csaLayouts.GetAt(iCtr), dlgLegend.m_csaVports.GetAt(iCtr));
		for (rbpTemp = rbpXD; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);
		rbpTemp->rbnext = acutBuildList(AcDb::kDxfXdAsciiString, csParameter, NULL);
	}

	// Place/Edit the legend
	AcDbObjectId objLegend;
	if (dlgLegend.m_iCalledFor == 1)
	{
		if (!dlgLegend.m_bSeparateLineAndBlock)
		{
			// Lines and Inserts are together
			if (!PlaceLegend(legendInfo_Vector, rbpXD, 0)) { acTransactionManagerPtr()->abortTransaction(); acutRelRb(rbpXD); return; }
		}
		else
		{
			// Lines and Inserts are separate
			if (!PlaceLegend(legendInfo_Vector, rbpXD, 1)) { acTransactionManagerPtr()->abortTransaction(); acutRelRb(rbpXD); return; }

			// Lines and Inserts are separate
			if (!PlaceLegend(legendInfo_Vector, rbpXD, 2)) { acTransactionManagerPtr()->abortTransaction(); acutRelRb(rbpXD); return; }
		}
	}
	else
	{
		acdbGetObjectId(objLegend, enLegend);
		if (!EditLegend(legendInfo_Vector, rbpXD, objLegend)) { acTransactionManagerPtr()->abortTransaction(); acutRelRb(rbpXD); return; }
	}

	 acutRelRb(rbpXD); 

	// End the transaction
	acTransactionManagerPtr()->endTransaction();

	// Regenerate the details
	acedCommandS(RTSTR, L".REGEN", NULL);
}
