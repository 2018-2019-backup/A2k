#include "StdAfx.h"
#include "LayoutInfo.h"

///////////////////////////////////////
// Externally defined functions
///////////////////////////////////////

extern void sortLayoutInfo(std::vector <CLayoutInfo> &layoutInfo);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetLayoutNamesInPaperSpace()
// Description  : Retrieves the names of paper space layouts in the drawing.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : GetTotalNumberOfSheets
// Description      : Get the total number of valid TITLE BLOCKS in the drawing.
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
	//long lLength = 0L; acedSSLength(ssTBlocks, &lLength); 
	int lLength = 0L; acedSSLength(ssTBlocks, &lLength);
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
// Function name: Command_PageNumber()
// Description  : Called when "NPN" command is invoked at command prompt.
//////////////////////////////////////////////////////////////////////////
void Command_PageNumber()
{
	switchOff();
	acutPrintf(L"\nVersion: V2.0");

	// Ask whether automatic numbering has to be done
	TCHAR result[5];
	acedInitGet(NULL, _T("Yes No"));
	int iRet = acedGetKword(_T("\nDo you want to automatically number the pages? [Yes/No] <Yes>: "), result);
	/**/ if (iRet == RTCAN) return;
	else if (iRet == RTNONE) result[0] = _T('Y');

	// Do not perform any action if "N" is selected
	if (result[0] == 'N') return;
		
	CStringArray csaApplicableBlockNames;
	int iLength = GetTotalNumberOfSheets(csaApplicableBlockNames); // ")) || !csTag.CompareNoCase(_T(""))
	if (iLength == 0) { acutPrintf(_T("\nThere are no valid title blocks in the drawing that has \"NO_OF_SHEETS\" & \"SHEET\" attribute tags.")); return; } 
	
	// Get the list of paper space layouts and sort them based on their tab order
	std::vector <CLayoutInfo> layoutInfo_Vector;
	GetLayoutNamesInPaperSpace(layoutInfo_Vector);
	sortLayoutInfo(layoutInfo_Vector);

	// For each layout in the drawing get the inserts on it and renumber the sheets attribute
	int iNumAttribs = 0;
	AcDbEntity *pEnt;
	AcDbBlockReference *pInsert = NULL;
	AcDbObjectId objTblRcd;
	AcDbSymbolTableRecord *pBlkTblRcd = NULL;
	AcString acsName;
	CString csName;
	AcDbAttribute *pAtt;
	AcDbObjectId objAttId;
	CString csValue, csTag;
	int iPageNumber = 1;
	
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
	for (int iCtr = 0; iCtr < layoutInfo_Vector.size(); iCtr++)
	{
		// Get the layout name from the vector and get its block table record object id
		//Commented for ACAD 2018
		//AcDbLayout *pLayout = pLayoutMngr->findLayoutNamed(layoutInfo_Vector[iCtr].m_csLayoutName, true);
		AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(layoutInfo_Vector[iCtr].m_csLayoutName);
		AcDbLayout *pLayout = NULL;
		acdbOpenObject(pLayout, objLayoutId, AcDb::kForWrite);
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
		for (pBlkTblRcdItr->start(); !pBlkTblRcdItr->done(); pBlkTblRcdItr->step())
		{
			// Get the next entity in the drawing database. The check is in case the database is corrupted
			if (pBlkTblRcdItr->getEntity(pEnt, AcDb::kForRead) != Acad::eOk) { delete pBlkTblRcdItr; return; }

			// Cast the entity pointer as an AcDbBlockReference pointer. 
			pInsert = AcDbBlockReference::cast(pEnt); pEnt->close();
			if (pInsert == NULL) continue;

			//////////////////////////////////////////////////////////////////////////
			// Get the block name of this insert
			//////////////////////////////////////////////////////////////////////////
			// Get the object id of the block definition from the insert
			objTblRcd = pInsert->blockTableRecord();
			
			// Open the symbol table record for this id
			if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk) { delete pBlkTblRcdItr; pInsert->close(); return; }

			pBlkTblRcd->getName(acsName); pBlkTblRcd->close();
			csName.Format(_T("%s"), acsName.kTCharPtr()); csName.MakeUpper();
			
			// If the block name has _TITLE in it implies this is a title block in th drawing
			// if (csName.Find(_T("TITLE")) == -1) { pInsert->close(); continue; }

			// Check if this block name is applicable
			if (!CheckForDuplication(csaApplicableBlockNames, csName)) { pInsert->close(); continue; }
			
			// Get the attribute iterator for this insert
			AcDbObjectIterator *pIter = pInsert->attributeIterator();
			pInsert->close();

			// Iterate through and change the necessary values
			for (pIter->start(); !pIter->done(); pIter->step())
			{
				objAttId = pIter->objectId();
				acdbOpenObject(pAtt, objAttId, AcDb::kForWrite);

				// Change total sheets value
				csTag.Format(_T("%s"), pAtt->tag());

				// /**/ if (!csTag.CompareNoCase(_T("No. of Sheets")))                   { csValue.Format(_T("%d"), iLength);  }
				// else if (!csTag.CompareNoCase(_T("Sheet No.")) && (result[0] == 'Y')) { csValue.Format(_T("%d"), iPageNumber++); }
				// else { pAtt->close(); continue; }
				
				/**/ if (!csTag.CompareNoCase(_T("NO_OF_SHEETS")))                { csValue.Format(_T("%d"), iLength);  }
				else if (!csTag.CompareNoCase(_T("SHEET")) && (result[0] == 'Y')) {	csValue.Format(_T("%d"), iPageNumber++); }
				else { pAtt->close(); continue; }


				pAtt->setTextString(csValue); 
				pAtt->close();
			}

			// Delete the attribute iterator
			delete pIter;
		}

		// Delete the block table record iterator
		delete pBlkTblRcdItr;
	}
}
