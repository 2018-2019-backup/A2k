// CertificateNumberDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CertificateNumberDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include <vector>

// CCertificateNumberDlg dialog

IMPLEMENT_DYNAMIC(CCertificateNumberDlg, CDialogEx)

CCertificateNumberDlg::CCertificateNumberDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CERTIFICATENUMBER, pParent)
{
	m_csNewNumber = _T("");
}

CCertificateNumberDlg::~CCertificateNumberDlg()
{
}

void CCertificateNumberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCertificateNumberDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_UPDATE, &CCertificateNumberDlg::OnBnClickedBtnUpdate)
END_MESSAGE_MAP()


// CCertificateNumberDlg message handlers


BOOL CCertificateNumberDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GetDlgItem(IDC_CERTIFICATE_NUMBER)->SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CCertificateNumberDlg::OnBnClickedBtnUpdate()
{
	UpdateData();
	GetDlgItemText(IDC_CERTIFICATE_NUMBER, m_csNewNumber);
	
	if (m_csNewNumber.IsEmpty()) 
	{ 
		::MessageBox(::GetActiveWindow(), _T("A New Certificate Number must be given."), _T("NET CAD message"), MB_OK | MB_ICONINFORMATION);
		return; 
	}

	// To Close the dialog
	OnOK();
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
	int lLength = 0L;
	acedSSLength(ssTBlocks, &lLength);
	if (lLength == 0L) { acedSSFree(ssTBlocks); return iLength; }
	ads_name enTBlock;
	AcDbObjectId objInsertId;
	AcDbObjectId objTblRcd;
	AcDbSymbolTableRecord *pBlkTblRcd = NULL;
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
		AcString acsName;
		pBlkTblRcd->getName(acsName);
		pBlkTblRcd->close();
		CString csName;
		csName.Format(_T("%s"), acsName.kTCharPtr());
		csName.MakeUpper();

		// Get the attribute tag specified and change the value specified
		AcDbObjectIterator *pIter = pInsert->attributeIterator();
		pInsert->close();
		for (pIter->start(); !pIter->done(); pIter->step())
		{
			AcDbAttribute *pAtt;
			acdbOpenObject(pAtt, pIter->objectId(), AcDb::kForRead);
			CString csTag;
			csTag.Format(_T("%s"), pAtt->tag());
			// Check if the necessary attribute is there. If YES, then this block counts
			if (!csTag.CompareNoCase(_T("CERTIFICATION_NUMBER")))
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetLayoutNamesInPaperSpace()
// Description  : Retrieves the names of paper space layouts in the drawing.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void GetLayoutNamesInPaperSpace(std::vector <CString> &layoutInfoVector)
{
	AcDbBlockTable *pBT;
	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBT, AcDb::kForRead);
	AcDbBlockTableIterator* pIter;
	pBT->newIterator(pIter);

	// Loop through the iterator
	for (; !pIter->done(); pIter->step())
	{
		// Get the block table record
		AcDbBlockTableRecord* pBTR;
		pIter->getRecord(pBTR, AcDb::kForRead);
		// If this is a layout
		if (pBTR->isLayout())
		{
			// Get the layout's object ID and from there its name
			AcDbObjectId layoutId = pBTR->getLayoutId();
			AcDbLayout *pLayout;
			acdbOpenAcDbObject((AcDbObject*&)pLayout, layoutId, AcDb::kForRead);
			ACHAR *pLayoutName;
			pLayout->getLayoutName(pLayoutName);
			CString csLayoutName = pLayoutName;
			pLayout->close();
			// If this is not the "Model" layout
			if (csLayoutName != _T("Model"))
			{
				int iEnt = 0;
				// Create a new iterator for this record
				AcDbBlockTableRecordIterator* pBtblrIter;
				pBTR->newIterator(pBtblrIter);
				// Just loop through it and count the number of steps
				for (; !pBtblrIter->done(); pBtblrIter->step()) iEnt++;

				// Delete the record iterator
				delete pBtblrIter;
				// If the count is 0, then the layout is not initialized
				if (iEnt != 0)
				{
					layoutInfoVector.push_back(csLayoutName);
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
// Function name    : CheckForDuplication
// Arguments        : 1. Array to check values in, as reference to CStringArray
//                    2. Value to check for in array, as CString
//
// Returns          : TRUE if found, FALSE if not
// Called from      : Anywhere
// Description      : Checks for presence of string in the array
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckForDuplication(CStringArray& csaArray, CString csCheck)
{
	for (int iCtr = 0; iCtr < csaArray.GetSize(); iCtr++)
	{
		if (!csaArray.GetAt(iCtr).CompareNoCase(csCheck))
		{
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Function name: Command_CertificateNumber()
// Description  : Called when "NCN" command is invoked at command prompt.
//////////////////////////////////////////////////////////////////////////
void Command_CertificateNumber()
{
	// First to find the drawing contains valid title blocks with CERTIFICATION_NUMBER attribute tag
	CStringArray csaApplicableBlockNames;
	int iLength = GetTotalNumberOfSheets(csaApplicableBlockNames);
	if (iLength == 0)
	{
		acutPrintf(_T("\nThere are no valid title blocks in the drawing that has \"CERTIFICATION_NUMBER\" attribute tag."));
		return;
	}
	// Display the Certificate Number dialog
	CCertificateNumberDlg dlgCFNumber;
	if (dlgCFNumber.DoModal() == IDCANCEL) return;
	// If New Certificate number was not given
	if (dlgCFNumber.m_csNewNumber.IsEmpty())
	{
		::MessageBox(::GetActiveWindow(), _T("A New Certificate Number must be given."), _T("NET CAD message"), MB_OK | MB_ICONINFORMATION);
		return;
	}
	
	// Get the list of paper space layouts and sort them based on their tab order
	std::vector <CString> layoutInfo_Vector;
	GetLayoutNamesInPaperSpace(layoutInfo_Vector);

	// For each layout in the drawing get the inserts on it and renumber the certificate number attribute
	AcDbLayoutManager *pLayoutMngr = acdbHostApplicationServices()->layoutManager();
	for (int iCtr = 0; iCtr < layoutInfo_Vector.size(); iCtr++)
	{
		// Get the layout name from the vector and get its block table record object id
		AcDbObjectId objLayoutId = pLayoutMngr->findLayoutNamed(layoutInfo_Vector[iCtr]);
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
			AcDbEntity *pEnt;
			// Get the next entity in the drawing database. The check is in case the database is corrupted
			if (pBlkTblRcdItr->getEntity(pEnt, AcDb::kForRead) != Acad::eOk) { delete pBlkTblRcdItr; return; }

			// Cast the entity pointer as an AcDbBlockReference pointer. 
			AcDbBlockReference *pInsert = NULL;
			pInsert = AcDbBlockReference::cast(pEnt); pEnt->close();
			if (pInsert == NULL) continue;

			//////////////////////////////////////////////////////////////////////////
			// Get the block name of this insert
			//////////////////////////////////////////////////////////////////////////
			// Get the object id of the block definition from the insert
			AcDbObjectId objTblRcd = pInsert->blockTableRecord();

			// Open the symbol table record for this id
			AcDbSymbolTableRecord *pBlkTblRcd = NULL;
			if (acdbOpenObject(pBlkTblRcd, objTblRcd, AcDb::kForRead) != Acad::eOk)
			{
				delete pBlkTblRcdItr;
				pInsert->close();
				return;
			}
			AcString acsName;
			pBlkTblRcd->getName(acsName);
			pBlkTblRcd->close();
			CString csName;
			csName.Format(_T("%s"), acsName.kTCharPtr());
			csName.MakeUpper();

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
				AcDbAttribute *pAtt;
				acdbOpenObject(pAtt, pIter->objectId(), AcDb::kForWrite);
				// Change certificate number value
				CString csTag;
				csTag.Format(_T("%s"), pAtt->tag());
				if (!csTag.CompareNoCase(_T("CERTIFICATION_NUMBER")))
				{
					pAtt->setTextString(dlgCFNumber.m_csNewNumber);
				}
				pAtt->close();
			}
			// Delete the attribute iterator
			delete pIter;
		}
		// Delete the block table record iterator
		delete pBlkTblRcdItr;
	}
}



