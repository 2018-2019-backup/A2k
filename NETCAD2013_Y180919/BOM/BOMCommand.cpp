#include "StdAfx.h"
#include "LayoutInfo.h"
#include "LegendInfo.h"
#include "BOMNotifyDlg.h"

//////////////////////
// Globals
//////////////////////

extern double g_LegendLinearLen;
extern CString g_csBOMLayer;

int f_iProposedColIndex;
int f_iQuantityColIndex;
int f_iDescriptColIndex;

///////////////////////////////////
// Externally defined functions
///////////////////////////////////

extern void sortLegendInfo			(std:: vector <CLegendInfo> &legendInfo);
extern void sortLayoutInfo      (std::vector <CLayoutInfo> &layoutInfo);
void GetLayoutNamesInPaperSpace	(std::vector <CLayoutInfo> &layoutInfoVector);

//////////////////////////////////////////////////////////////////////////
// Function name: GetDescriptionForLayer()
// Description  :
//////////////////////////////////////////////////////////////////////////
bool GetDescriptionForLayer(CString csLayerName, CString &csLayerDesc)
{
  // csLayerDesc = csLayerName;
	// FRS: Where the linear object layer does not have a layer description or where the block object does not have a LEGEND attribute,
	// that object is not included in the BOM.  
	csLayerDesc = L"";

  AcDbLayerTable *pLayerTbl;
  Acad::ErrorStatus es = acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead);
  if (es != Acad::eOk) { return false; }

  // If the layer doesn't exist return quietly
  if (!(pLayerTbl->has(csLayerName))) { pLayerTbl->close(); return false; }

  // Get the object id and the layer table record
  AcDbLayerTableRecord *pLayerTblRecord = ::new AcDbLayerTableRecord;
  es = pLayerTbl->getAt(csLayerName, pLayerTblRecord, AcDb::kForRead);
  if (es != Acad::eOk) { pLayerTbl->close(); return false; }
  pLayerTbl->close();

  // Description
  csLayerDesc.Format(_T("%s"), pLayerTblRecord->description()); 
  csLayerDesc = csLayerDesc.Trim();
	if (csLayerDesc == _T("(null)")) csLayerDesc = L"";
  // if (csLayerDesc.IsEmpty() || csLayerDesc == _T("(null)")) csLayerDesc = csLayerName;

  pLayerTblRecord->close();
  return true;
}

/////////////////////////////////////////////////////
// Function name: GetConstantAttributeValue()
// Description  :
/////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetObjectsForBOM()
// Description  : Get the details of LINES/INSERTS that can be placed in the BOM.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetObjectsForBOM(std::vector <CLegendInfo> &legendInfo_Vector, ads_point ptMin, ads_point ptMax, AcDbObjectIdArray &aryObjIds)
{
	// Select all inserts and lines placed in the drawing
	ads_name ssBOM;
	struct resbuf *rbpFilt = acutBuildList(-4, _T("<OR"), RTDXF0, L"SPLINE", RTDXF0, L"ARC", RTDXF0, L"INSERT", RTDXF0, L"LWPOLYLINE", RTDXF0, L"LINE", RTDXF0, L"POLYLINE", -4, _T("OR>"), 67, 0, NULL);

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

	if (acedSSGet(_T("C"), ptMin, ptMax, rbpFilt, ssBOM) != RTNORM) 
	{
		acedSSFree(ssBOM); 
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
		
	// Process the entities in the selection set for BOM
	ads_name enEntity;
	AcDbObjectId objEntity;
	AcDbEntity *pEntity = NULL;

	CStringArray csaSearched;
	CString csDescription; 
	Acad::ErrorStatus es;
	//long lLength = 0L; acedSSLength(ssBOM, &lLength); 
	int lLength = 0L; acedSSLength(ssBOM, &lLength);
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
			acedMspace();
			ads_name enSel; acdbGetAdsName(enSel, objEntity);
			acedCommandS(RTSTR, L".LENGTHEN", RTLB, RTENAME, enSel, RTPOINT, ptDummy, RTLE, RTSTR, L"", NULL);
			struct resbuf rbSetVar; acedGetVar(L"PERIMETER", &rbSetVar);
			double dLength = rbSetVar.resval.rreal;
			acedPspace();

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
			
			if (!GetDescriptionForLayer(csLayerName, csDescription)) { return FALSE; }
			if (csDescription.IsEmpty()) continue;

			// Status
			bool bIsProposed = false;
			bool bIsExisting = false;

			/**/ if ((csLayerName.Find(L"_PROP_") != -1) || (csLayerName.Find(L"_PROP") != -1))   bIsProposed = true;
			else if ((csLayerName.Find(L"_EXIST_") != -1) || (csLayerName.Find(L"_EXIST") != -1)) bIsExisting = true;
			
			// Add other details 
			CLegendInfo legendInfo;
			legendInfo.m_csPropLayer = csLayerName;
			legendInfo.m_dLength = dLength;
			legendInfo.m_csType  = L"LINE";
			legendInfo.m_iIndex  = legendInfo_Vector.size();
			legendInfo.m_csDescription = csDescription;
			legendInfo_Vector.push_back(legendInfo);
		}
		else if (pEntity->isKindOf(AcDbBlockReference::desc()))
		{
			// Get the block reference pointer for the entity
			AcDbBlockReference *pInsert = AcDbBlockReference::cast(pEntity);
			csDescription = GetConstantAttributeValue(pInsert, L"LEGEND");
			if (csDescription.IsEmpty()) { pInsert->close(); continue; }

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
			pBlkTblRcd->getName(pValue);  CString csName; csName.Format(_T("%s"), pValue); csName.MakeUpper();
			pBlkTblRcd->comments(pValue); CString csDesc; csDesc.Format(_T("%s"), pValue); csDesc.MakeUpper(); 
			delete pValue;
			pBlkTblRcd->close();

			// If the description in EMPTY, ignore the block
		  // Where the linear object layer does not have a layer description or where the block object does not have a LEGEND attribute, that object is not included in the BOM
			if (csDescription.IsEmpty()) continue;

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
			legendInfo.m_csDescription = csDescription;

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : setTextString
// Description  : Places the given text in the given cell of a table.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setTextString(AcDbTable *pTable, int iRow, int iCol, AcDbObjectId objIDTxtStyle, AcDb::RotationAngle rotAngle, CString csText)
{
	if (objIDTxtStyle) pTable->setTextStyle(iRow, iCol, objIDTxtStyle);

  pTable->setTextString(iRow, iCol, L" " + csText); 
  pTable->setTextRotation(iRow, iCol, rotAngle);
  //pTable->setTextHeight(iRow, iCol, 1.5);

  // Set the row alignment
  pTable->setAlignment(iRow, iCol, AcDb::kMiddleLeft);
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

//////////////////////////////////////////////////////////////////////////
// Function name: PlaceThisBOMInTable()
// Description  : Writes the TEXTS for a given ROW in the BOM table.
//////////////////////////////////////////////////////////////////////////
void PlaceThisBOMInTable(AcDbTable *pTable, int iRow, CLegendInfo legendInfo, AcDbObjectId objIDTxtStyle)
{
	// If the information placed is for a LTYPE, get the line type assigned to the layer on which the line is.
	Adesk::Int16 iColor = 254;
	CString csName; // For a LINE, the LTYPE and for an INSERT its BLOCK NAME are used for BOM

	// Block for LINE/INSERT
	if (!legendInfo.m_csType.CompareNoCase(_T("LINE")))
	{
		// Get the appropriate layer on which the LINE is placed
		CString csLType;
		double dLineWt;

		// Call the function to determine the LYPE used
		if (GetLTypeAndColorFromLayer(legendInfo.m_csPropLayer, csLType, iColor, dLineWt)) 
		{
			csName = csLType; 
			createBlock(csName, iColor);
			csName.Format(L"%s-%d", csName, iColor);
		}
	}
	else if (!legendInfo.m_csType.CompareNoCase(L"INSERT")) 
	{
		csName = legendInfo.m_csObject;
	}
	
	// If the name is not EMPTY
	CString csValue;
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
				pTable->setBlockTableRecordId(iRow + 2, f_iProposedColIndex, objId, false);
			else
				pTable->setBlockTableRecordId(iRow + 2, f_iProposedColIndex, objId, true);

			pTable->setCellType(iRow + 2, f_iProposedColIndex, AcDb::kBlockCell);
			pTable->setAlignment(AcDb::kMiddleCenter);

			// Apply respective scale to the cell
			if (!legendInfo.m_csType.CompareNoCase(_T("LINE")))
			{
				pTable->setScale(iRow + 2, f_iProposedColIndex, 0, (g_LegendLinearLen - 2) / 10);
			}
		}
		else
		{
			pTable->setCellType(iRow + 2, f_iProposedColIndex, AcDb::kTextCell);
			pTable->setTextString(iRow + 2, f_iProposedColIndex, L"X");
			// setTextString(pTable, iRow + 2, iCol, objIDTxtStyle, AcDb::kDegrees000, L"X");
		}
	}

	// Place the Quantity
	pTable->setTextString(iRow + 2, f_iQuantityColIndex, suppressZero((int)(legendInfo.m_dLength + 0.5)));

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

/////////////////////////////////////////////////////
// Function name: PlaceBOM
// Description  :
/////////////////////////////////////////////////////
bool PlaceBOM(std::vector <CLegendInfo> &legendInfo_Vector, AcDbObjectId &objBOM)
{
	// Predefined column locations for NEW BOM generation
	f_iProposedColIndex = 0;
	f_iQuantityColIndex = 1;
	f_iDescriptColIndex = 2;

	CString csLayer;
	CUIntArray uiaIndex; 
	int iCategory = 0;
	int iNoMatched = 0;
	CString csThisObject;

	// Line types are required to be ordered like this
	CStringArray csaCableSortOrder; 
	CString csCableType;
	csaCableSortOrder.Add(_T("SV"));   csaCableSortOrder.Add(_T("SL"));   csaCableSortOrder.Add(_T("LV"));  csaCableSortOrder.Add(_T("HV"));  csaCableSortOrder.Add(_T("AUX")); 
	csaCableSortOrder.Add(_T("TR33")); csaCableSortOrder.Add(_T("TR66")); csaCableSortOrder.Add(_T("TR132")); csaCableSortOrder.Add(_T("BASE"));
	
	sortLegendInfo(legendInfo_Vector);
	
	//////////////////////////////////////////////////////////////////////////
	// Placing BOM as table
	//////////////////////////////////////////////////////////////////////////
	int iCtr = 0;
	AcDbTable *pTableLegend = new AcDbTable();
	pTableLegend->setLayer(g_csBOMLayer);
	pTableLegend->setSize(legendInfo_Vector.size() + 2, 3); // The methods setNumColumns() and setNumRows() have been deprecated beyond AutoCAD 2007.
	pTableLegend->setColumnWidth(g_LegendLinearLen);
	pTableLegend->setRowHeight(4.0);
	pTableLegend->setFlowDirection(AcDb::kTtoB);
	pTableLegend->setLinetype(L"CONTINUOUS");

	// Before setting the "Standard" text style for the table, set the text size in it to 0.0. Just in case someone sets it to nonzero value
	double dTxtSizeData;
	double dTxtSizeHead;
	double dTxtSizeTitl;
	AcDbObjectId objIDTxtStyleData;	if (setTextSizeInStyle(L"Data",   0.0, dTxtSizeData, objIDTxtStyleData)) { pTableLegend->setTextStyle(objIDTxtStyleData); }
	AcDbObjectId objIDTxtStyleHead; if (setTextSizeInStyle(L"Header", 0.0, dTxtSizeHead, objIDTxtStyleHead)) { pTableLegend->setTextStyle(objIDTxtStyleHead); }
	AcDbObjectId objIDTxtStyleTitl; if (setTextSizeInStyle(L"Title",  0.0, dTxtSizeTitl, objIDTxtStyleTitl)) { pTableLegend->setTextStyle(objIDTxtStyleTitl); }

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
	pTableLegend->setTextString(0, 0, _T("BILL OF MATERIALS"));
	pTableLegend->mergeCells(0, 0, 0, 2);

	pTableLegend->setTextString(1, f_iProposedColIndex, _T("PROPOSED"));  
	pTableLegend->setTextString(1, f_iQuantityColIndex, _T("QUANTITY/ LENGTH (M)"));
	pTableLegend->setTextString(1, f_iDescriptColIndex, _T("DESCRIPTION"));
	pTableLegend->setColumnWidth(1, 22.0);
	pTableLegend->setRowHeight(1, 6.75);
	pTableLegend->setColumnWidth(2, 40.0);

	// Add the table to the drawing
	AcDbBlockTable *pBlkTbl = NULL;
	if (acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlkTbl, AcDb::kForRead) != Acad::eOk) { pTableLegend->close(); return false; }

	AcDbBlockTableRecord *pBlkTblRcd = NULL;
	if (pBlkTbl->getAt(ACDB_PAPER_SPACE, pBlkTblRcd, AcDb::kForWrite) != Acad::eOk) { pBlkTbl->close(); pTableLegend->close(); return false; }
	pBlkTbl->close();

	double dTableWidth  = pTableLegend->width();
	double dTableHeight = pTableLegend->height();
	pBlkTblRcd->appendAcDbEntity(pTableLegend);

	// Used to place the the second table
	AcDbExtents extents; pTableLegend->getGeomExtents(extents);
	pBlkTblRcd->close();


	// Get on with the placement of items
	for (; iCtr < legendInfo_Vector.size(); iCtr++)
	{
		// The blocks/lines are to be separated
		// if ((legendInfo_Vector.at(iCtr).m_iIndex > 1000)) { break; }
		PlaceThisBOMInTable(pTableLegend, iCtr, legendInfo_Vector[iCtr], NULL);
		legendInfo_Vector[iCtr].m_bIsDescPlcd = true;
	}

	// Get the geometric extents of the table already placed
	int iRet;
	ads_point ptBOM;
	acedInitGet(RSG_NONULL, _T(""));
	while (T)
	{
		iRet = acedGetPoint(NULL, _T("\nSpecify insertion point: "), ptBOM);
		if (iRet == RTCAN) { pTableLegend->erase(); pTableLegend->close(); return false; }
		else if (iRet == RTNORM) break;
	}

	pTableLegend->setPosition(AcGePoint3d(ptBOM[X], ptBOM[Y], 0.0));
	pTableLegend->setAlignment(AcDb::kMiddleCenter, AcDb::kAllRows);
	pTableLegend->deleteRows(iCtr + 2, 100);

	// Get the BOM object ID
	objBOM = pTableLegend->objectId();
	es = pTableLegend->close(); 

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetLayoutNamesInPaperSpace
// Description  : Retrieves the names of all PAPER SPACE View ports and sorts them based on their tab order.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GetLayoutNamesInPaperSpace(std::vector <CLayoutInfo> &layoutInfoVector)
{
	// Get the block table of the drawing and create a new iterator
	AcDbBlockTable *pBT; 
	AcDbBlockTableIterator* pIter;

	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBT, AcDb::kForRead);
	pBT->newIterator(pIter);

	// Loop through the iterator
	int iTabOrder;
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
			iTabOrder = pLayout->getTabOrder();
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
				if (iEnt != 0) 
				{
					CLayoutInfo layoutInfo;
					layoutInfo.m_csLayoutName = csLayoutName;
					layoutInfo.m_iTabOrder    = iTabOrder;

					layoutInfoVector.push_back(layoutInfo);
				}
			}
		}

		// Close the block table record
		pBTR->close();
	}

	// Close the block table
	pBT->close();

	// Delete the iterator
	delete pIter;
}

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: EditBOM()
// Description  : Called when a BOM output is selected instead of VPORTS. 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool EditBOM(std::vector <CLegendInfo> &legendInfo_Vector, AcDbObjectId &objBOM)
{
	// Open the table to determine the locations of columns we are interested
	AcDbTable *pTable;
	if (acdbOpenObject(pTable, objBOM, AcDb::kForWrite) != Acad::eOk) return false;

	f_iProposedColIndex = -1;
	f_iQuantityColIndex = -1;
	f_iDescriptColIndex = -1;
	
	for (int iCol = 0; iCol < pTable->numColumns(); iCol++)
	{
		/**/ if (!wcscmp(pTable->textString(1, iCol), L"PROPOSED")) f_iProposedColIndex = iCol;
		else if (!wcscmp(pTable->textString(1, iCol), L"QUANTITY/ LENGTH (M)")) f_iQuantityColIndex = iCol;
		else if (!wcscmp(pTable->textString(1, iCol), L"DESCRIPTION")) f_iDescriptColIndex = iCol;
	}

	if ((f_iProposedColIndex == -1) || (f_iQuantityColIndex == -1) || (f_iDescriptColIndex == -1))
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
			if ((iCol == f_iProposedColIndex) || (iCol == f_iQuantityColIndex) || (iCol == f_iDescriptColIndex)) continue;

			// <Column Name>-<Description>#<Value>
			csColumn.Format(L"%s-%s#%s", pTable->textString(1, iCol), pTable->textString(iRow, f_iDescriptColIndex), pTable->textString(iRow, iCol)); 
			csaOtherColumns.Add(csColumn);
		}
	}

	// Set the number of rows for the table
	pTable->deleteRows(2, pTable->numRows() - 2);
	pTable->setSize(legendInfo_Vector.size() + 2, pTable->numColumns());
			
	for (int iRow = 0; iRow < legendInfo_Vector.size(); iRow++)
	{
		PlaceThisBOMInTable(pTable, iRow, legendInfo_Vector.at(iRow), NULL);
		legendInfo_Vector[iRow].m_bIsDescPlcd = true;
	}

	// Update the other columns with its previous values
	CString csValue;
	for (int iRow = 2; iRow < pTable->numRows(); iRow++)
	{
		for (int iCol = 0; iCol < pTable->numColumns(); iCol++)
		{
			// Not interested to modify the values in interested columns
			if ((iCol == f_iProposedColIndex) || (iCol == f_iQuantityColIndex) || (iCol == f_iDescriptColIndex)) continue;

			csColumn.Format(L"%s-%s", pTable->textString(1, iCol), pTable->textString(iRow, f_iDescriptColIndex));
			GetParameterValue(csaOtherColumns, csColumn, csValue, -1);
			pTable->setTextString(iRow, iCol, csValue);
		}
	}


	pTable->close();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Function name: Command_BOM()
// Description  : Called when "NBOM" command is issued at command prompt.
/////////////////////////////////////////////////////////////////////////////
void Command_BOM()
{
	// Switch off certain system variables
	switchOff();

	// Check if there are objects already selected
	ads_name ssPickFirst;
	//long lPickFirstLen = 0L;
	int lPickFirstLen = 0L;
	struct resbuf rbCTab; acedGetVar(L"CTAB", &rbCTab);
	struct resbuf *rbpFilt = acutBuildList(RTDXF0, L"VIEWPORT", 410, rbCTab.resval.rstring, NULL);
	if (acedSSGet(_T("I"), NULL, NULL, NULL, ssPickFirst) == RTNORM)
	{
		if (acedSSLength(ssPickFirst, &lPickFirstLen) != RTNORM) { acutRelRb(rbpFilt);return; }
	}
	acutRelRb(rbpFilt);

	// Ensure that the layer for this block is created and current
	createLayer(g_csBOMLayer, Adesk::kFalse); 

	// If in Model space, switch to Paper Space
	struct resbuf rbTileMode; rbTileMode.restype = RTSHORT; rbTileMode.resval.rint = 0; acedSetVar(_T("TILEMODE"), &rbTileMode);

	// Get the list of paper space layouts and sort them based on their tab order
	std::vector <CLayoutInfo> layoutInfo_Vector;
	CStringArray csaLayouts, csaVPorts, csaVPortsCnt;
	GetLayoutNamesInPaperSpace(layoutInfo_Vector);
	sortLayoutInfo(layoutInfo_Vector);

	ads_name ssGet; 
	CString csValue;
	//long lLength;
	int lLength;
	ads_name enVP;
	AcDbObjectId objVPort;
	AcDbViewport *pVPort;
	ACHAR sHandle[18];
	AcDbHandle handle;
	CString csVPort = L"";

	// Display the BOM dialog to enable selection of view ports
	CBOMNotifyDlg dlgBOM;
	dlgBOM.m_iCalledFor = 1;
	
	for (int iCtr = 0; iCtr < layoutInfo_Vector.size(); iCtr++) 
	{
		csaLayouts.Add(layoutInfo_Vector.at(iCtr).m_csLayoutName); 
		if (!layoutInfo_Vector.at(iCtr).m_csLayoutName.CompareNoCase(rbCTab.resval.rstring))
		{
			// Add the PRE-SELECTED VPORTS to the array after validating it
			for (long lCtr = 0L; lCtr < lPickFirstLen; lCtr++)
			{
				// Get the entity details
				acedSSName(ssPickFirst, lCtr, enVP);
				acdbGetObjectId(objVPort, enVP);
				handle = objVPort.handle();
				handle.getIntoAsciiBuffer(sHandle);

				// Validate if the VPORT is on/off
				if (acdbOpenObject(pVPort, objVPort, AcDb::kForWrite) != Acad::eOk) continue;
				if (!pVPort->isOn()) { pVPort->close(); pVPort->close(); continue; }
				pVPort->close();
				
				// Suffix the handle to the other valid viewport handles saved
				csValue.Format(L";%s", sHandle);
				csVPort += csValue;
			}

			// If there are valid view port in the selection append the VPORT handle string to the array
			if (!csVPort.IsEmpty())	csaVPorts.Add(csVPort); else csaVPorts.Add(L"");

			// Free the selection set
			acedSSFree(ssPickFirst);
		}
		else csaVPorts.Add(L"");

		// Get the number of VPORTS in the layout
		rbpFilt = acutBuildList(RTDXF0, L"VIEWPORT", 410, layoutInfo_Vector.at(iCtr).m_csLayoutName, NULL);
		if (acedSSGet(L"X", NULL, NULL, rbpFilt, ssGet) == RTNORM)
		{
			lLength = 0L; 
			acedSSLength(ssGet, &lLength); 
			csValue.Format(L"%ld", lLength - 1L);
			csaVPortsCnt.Add(csValue);
		}
		else
			csaVPortsCnt.Add("0");

		acutRelRb(rbpFilt);
		acedSSFree(ssGet);
	}
	
	// Allow selection of an existing BOM, provided the user has not selected VIEWPORTS
	ads_name enBOM;
	int iRet;
	struct resbuf *rbpBOM = NULL;
	ads_point ptDummy;

	if (lPickFirstLen == 0L)
	{
		// If VPORTS in the drawing are not initially selected, the following prompts will enable the user to select a BOM to edit or can proceed to
		// have the dialog displayed
		while (T)
		{
			iRet = acedEntSel(L"\nENTER to create or select BOM to EDIT: ", enBOM, ptDummy);
			/**/ if (iRet == RTCAN) return;
			else if (iRet == RTNORM)
			{
				// Check if BOM XDATA is attached
				rbpBOM = getXDataFromEntity(enBOM, L"BOM");
				if (rbpBOM == NULL) { appMessage(L"Select a valid BOM."); continue; }
				break;
			}
			else if (iRet == RTERROR)
			{
				// Fresh BOM to be created
				break;
			}
		}
		
		if (rbpBOM)
		{
			// Called for edit
			dlgBOM.m_iCalledFor = 2;

			// Get the XDATA in BOM to an array
			CStringArray csaBOM;
			while (rbpBOM->rbnext) { rbpBOM = rbpBOM->rbnext; csaBOM.Add(rbpBOM->resval.rstring);	}

			// Set the layout values
			CString csValue;
			for (int iLay = 0; iLay < csaLayouts.GetCount(); iLay++)
			{
				GetParameterValue(csaBOM, csaLayouts.GetAt(iLay), csValue, -1);
				if (!csValue.IsEmpty())
				{
					csaVPorts.SetAt(iLay, csValue);	
					int iCnt = 1;
					while (csValue.Find(L";") != -1) 
					{
						iCnt++; 
						csValue = csValue.Mid(csValue.Find(L";") + 1); 
					}
	
					csaVPortsCnt.SetAt(iLay, suppressZero(iCnt));
				}
			}
		}
	}
	
	while (T)
	{
		// Display the selection dialog
		dlgBOM.m_csaLayouts.Copy(csaLayouts);
		dlgBOM.m_csaVports.Copy(csaVPorts);
		dlgBOM.m_csaVportsCnt.Copy(csaVPortsCnt);
				
		if (dlgBOM.DoModal() == IDCANCEL) {	acedCommandS(RTSTR, L".REGEN", NULL); 	return; }

		csaLayouts.Copy(dlgBOM.m_csaLayouts);
		csaVPorts.Copy(dlgBOM.m_csaVports);
		csaVPortsCnt.Copy(dlgBOM.m_csaVportsCnt);

		if (dlgBOM.m_iCurrentLayout != -1)
		{
			AcDbObjectId objId; 
			AcDbViewport *pVPort;
			CStringArray csaHandles;
			CStringArray csaVPortsInLayout;
			ads_point ptDummy;
			ads_name enVPort;
			int iRet;
			CString csHandle;

			// Get the viewport handles in the current layout. Required to check if a non-rectangular viewport is selected
			AcDbObjectIdArray aryClipObjIds;
			AcDbObjectIdArray aryVPObjIds;
			int iLen = getVPortClipHandles(dlgBOM.m_csLayout, aryClipObjIds, aryVPObjIds);

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
					csaVPorts.SetAt(dlgBOM.m_iCurrentLayout, csHandle);
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
						// AcDbCircle, AcDbPolyline, AcDb2dPolyline, AcDb3dPolyline, AcDbEllipse,	AcDbRegion, AcDbSpline, AcDbFace. As anyone these types can be 
						// used to clip the viewport view
						AcDbEntity *pEntChk;
						acdbOpenObject(pEntChk, objId, AcDb::kForRead);
						if (pEntChk->isKindOf(AcDbCircle::desc())  || pEntChk->isKindOf(AcDbPolyline::desc()) || pEntChk->isKindOf(AcDb2dPolyline::desc()) ||
							  pEntChk->isKindOf(AcDbEllipse::desc()) || pEntChk->isKindOf(AcDbRegion::desc())   || pEntChk->isKindOf(AcDbSpline::desc())
							)
						{
							// Check if the object selected is used as a clip entity for a viewport
							for (iCtr = 0; iCtr < aryClipObjIds.length(); iCtr++)
							{
								if (aryClipObjIds.at(iCtr) == pEntChk->objectId()) { bValidEntity = true; break; }
							}
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
					if (!bIsOn) { appMessage(L"\nThe selected viewport information is OFF and will not be included for BOM.", 00); continue; }

					// Get the viewport handle and add it to the array
					struct resbuf *rbpGet = acdbEntGet(enVPort);
					if (rbpGet)
					{
						if (!CheckForDuplication(csaHandles, Assoc(rbpGet,5)->resval.rstring)) csaHandles.Add(Assoc(rbpGet,5)->resval.rstring);	
						acutRelRb(rbpGet);
					}
				}
			}
		}
		else break;
	}
	
	// Get the details of the objects for BOM
	CStringArray csaObjects;			// Block name or Layer name
	CStringArray csaLayers;				// Layer names
	CStringArray csaTypes;				// Entity type
	CStringArray csaDescriptions; // Description from LEGEND attribute and Layer description
	CStringArray csaBlocksSearched; // Blocks already processed

	ads_point ptMin;
	ads_point ptMax;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// For each layout, get the view port handles selected for legend, get the window extents, get the frozen layer list in the viewport
	// call the function to collect all objects eligible for legend.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
	CString csLayout; 

	std::vector <CLegendInfo> legendInfo_Vector;
	for (int iCtr = 0; iCtr < dlgBOM.m_csaLayouts.GetSize(); iCtr++)
	{
		// Get the VPORT handles selected
		csVPort = dlgBOM.m_csaVports.GetAt(iCtr);

		// Get the viewport object id's selected
		int iCnt = 0;
		while (csVPort.Find(L";") != -1) { csVPort = csVPort.Mid(csVPort.Find(L";") + 1); iCnt++; }

		// If nothing selected, go to the next one
		if (!iCnt) { continue; } 
		
		// If this layout is different from layout iterated, set it as current
		csLayout = dlgBOM.m_csaLayouts.GetAt(iCtr);
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
		csVPort = dlgBOM.m_csaVports.GetAt(iCtr);

		while (csVPort.Find(L";") != -1) 
		{
			csVPort  = csVPort.Mid(csVPort.Find(L";") + 1); 
			csHandle = csVPort;
			if (csHandle.Find(L";") != -1) { csHandle = csHandle.Mid(0, csHandle.Find(L";")); }
			csaHandles.Add(csHandle);
		}

		// Get the entity names from view port handles
		ads_name enHandle;
		AcDbObjectId objHandle;
		AcDbViewport *pVport;
		AcDbObjectIdArray aryObjIds;
		for (int i = 0; i < csaHandles.GetSize(); i++)
		{
			// Get the view port entity name for this handle
			acdbHandEnt(csaHandles.GetAt(i), enHandle);

			// Get the view port object id for this entity name
			acdbGetObjectId(objHandle, enHandle);

			// Open the view port and get the layers frozen in it
			int iCVNumber;
			if (acdbOpenObject(pVport, objHandle, AcDb::kForRead) != Acad::eOk) continue;
			
			// Switch to paper space
			acedMspace();

			// Set CVPORT to the number of your viewport to ensure that your viewport is active
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

			// Go get all the objects to place in the BOM
			GetObjectsForBOM(legendInfo_Vector, ptMin, ptMax, aryObjIds);
		}
	}

	// Check if there any information qualifying a legend birth
	if (legendInfo_Vector.size() <= 0) { acutPrintf(_T("\nThere are no information for placing in BOM. Exiting command!")); return; }

	// Sort the information in the legend
	sortLegendInfo(legendInfo_Vector);

	// Start the transaction
	acTransactionManagerPtr()->startTransaction();

	// If in Model space, switch to Paper Space
	rbTileMode.restype = RTSHORT; rbTileMode.resval.rint = 0; acedSetVar(_T("TILEMODE"), &rbTileMode);

	// Set the current VPORT to 1
	struct resbuf rbCVPort; acedGetVar(_T("CVPORT"), &rbCVPort); if (rbCVPort.resval.rint != 1) { acedCommandS(RTSTR, _T(".PSPACE"), NULL); }

	AcDbObjectId objBOM;
	if (dlgBOM.m_iCalledFor == 1)
	{
		if (!PlaceBOM(legendInfo_Vector, objBOM)) { acTransactionManagerPtr()->abortTransaction(); return; }
	}
	else
	{
		acdbGetObjectId(objBOM, enBOM);
		if (!EditBOM(legendInfo_Vector, objBOM)) { acTransactionManagerPtr()->abortTransaction(); return; }
	}

	acTransactionManagerPtr()->endTransaction();

	// Add the XDATA to the BOM generated. THis XDATA will have  details of all the VPORTS selected for BOM generated. This information is used when the user
	// selects the BOM for editing.
	CString csParameter;
	struct resbuf *rbpXD = acutBuildList(AcDb::kDxfRegAppName, L"BOM", NULL);
	struct resbuf *rbpTemp;

	for (int iCtr = 0; iCtr < dlgBOM.m_csaLayouts.GetCount(); iCtr++)
	{
		csParameter.Format(L"%s#%s", dlgBOM.m_csaLayouts.GetAt(iCtr), dlgBOM.m_csaVports.GetAt(iCtr));
		for (rbpTemp = rbpXD; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);
		rbpTemp->rbnext = acutBuildList(AcDb::kDxfXdAsciiString, csParameter, NULL);
	}

	acdbGetAdsName(enBOM, objBOM);
	addXDataToEntity(enBOM, rbpXD);
	acutRelRb(rbpXD);
		
	// Regenerate the details
	acedCommandS(RTSTR, L".REGEN", NULL);
}