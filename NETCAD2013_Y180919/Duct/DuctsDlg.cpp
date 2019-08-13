// DuctsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DuctsDlg.h"
#include "ConduitDlg.h"

extern CString g_csProposedPattern;
extern double g_dProposedScale;

#define PROPOSED 0;
#define EXISTING 1;

//////////////////////////////////////////////////////////////////////////
// Externally defined functions
//////////////////////////////////////////////////////////////////////////
extern bool getConduitAndCableItem(std:: vector <CConduitAndCableInfo> &conduitAndCableInfoVector, int iRow, int iCol, CConduitAndCableInfo &conduitAndCableInfo);
extern double GetMinimumConduitDia(std:: vector <CConduitAndCableInfo> &conduitAndCableInfoVector);

//////////////////////////////////////////////////////////////////////////
// Global variables
//////////////////////////////////////////////////////////////////////////
AcCmColor g_ByLayerColor; 
CStringArray g_csaCableDia;
CStringArray g_csaCableColor;
CStringArray g_csaDuctScale;

//////////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: ChangeCustomParameters()
// Description  : Sets the value for the given dynamic parameter in a dynamic block.
//////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : ChangeAttributeValueAndRelocate
// Arguments        : 1. The parent entity, as object id
//                    2. The new value, as string
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ChangeAttributeValueAndRelocate(AcDbObjectId objInsertId, CString csTag, CString csAttValue, double dHeight, bool bRelocate)
{
	// Get the attributes attached to this insert
	AcDbBlockReference *pInsert;
	acdbOpenObject(pInsert, objInsertId, AcDb::kForRead);
	AcDbObjectIterator *pIter = pInsert->attributeIterator();
	AcGeScale3d acgeScale			= pInsert->scaleFactors();
	double dAngle							= pInsert->rotation();
	ads_point ptInsert				= { pInsert->position().x, pInsert->position().y, 0.0 };
	pInsert->close();

	// Get the attribute tag specified and change the value specified
	AcDbAttribute *pAtt;
	AcDbObjectId objAttId;
	for (pIter->start(); !pIter->done(); pIter->step())
	{
		objAttId = pIter->objectId();
		acdbOpenObject(pAtt, objAttId, AcDb::kForWrite);

		if (!csTag.CompareNoCase(pAtt->tag())) 
		{
			// Set the new text
			if (pAtt->isMTextAttribute())
			{
				AcDbMText *pMtext = new AcDbMText();
				pMtext->setContents(csAttValue);
				pAtt->setMTextAttribute(pMtext);
				delete pMtext;
			}
			else
				pAtt->setTextString(csAttValue);	

			if (dHeight) pAtt->setHeight(dHeight);
			if (bRelocate)
			{
				// Change the location to the head of the arrow
				ads_point ptNew;
				acutPolar(ptInsert, dAngle, acgeScale.sx * 0.75, ptNew);
				pAtt->setPosition(AcGePoint3d(ptNew[X], ptNew[Y], 0.0));
				pAtt->setAlignmentPoint(AcGePoint3d(ptInsert[X], ptInsert[Y], 0.0));
			}
		}

		pAtt->close();
	}

	delete pIter;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Function name: SetNotes
// Description  : Sets each line of notes as attribute values in the SECTION blocks.
/////////////////////////////////////////////////////////////////////////////////////////
void SetNotes(AcDbObjectId objTrench, CString csNotesInput, double dHeight)
{
	int iCnt = 0;
	CString csNotes = csNotesInput;
	CString csValue;
	CString csTag;

	if (csNotes.Find(L"\n") != -1)
	{
		while (csNotes.Find(L"\n") != -1)
		{
			csValue = csNotes.Mid(0, csNotes.Find(L"\n")); 
			csValue = csValue.Trim();
			iCnt++;
			csTag.Format(L"Data%d", iCnt);

			ChangeAttributeValueAndRelocate(objTrench, csTag, csValue, dHeight, false);
			csNotes = csNotes.Mid(csNotes.Find(L"\n") + 1);

			// Last lines must be written too
			if (csNotes.Find(L"\n") == -1)
			{
				csTag.Format(L"Data%d", iCnt + 1);
				ChangeAttributeValueAndRelocate(objTrench, csTag, csNotes, dHeight, false);
				break;
			}
		}
	}
	else
	{
		ChangeAttributeValueAndRelocate(objTrench, L"DATA1", csNotes, dHeight, false);
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: GetSectionLabel() 
// Description  : Retrieves the next section label to be used.
// Arguments    : CString &, Place holder for the section label retrieved.
//////////////////////////////////////////////////////////////////////////
bool GetSectionLabel(CString &csLabel)
{
	csLabel = "";

	// Collect all texts placed with this XDATA
	struct resbuf *rbpFilt = acutBuildList(RTDXF0, _T("TEXT"), AcDb::kDxfRegAppName, _T("EA_SECTIONLABEL"), 67, 0, NULL); 
	ads_name ssGet;
	if (acedSSGet(_T("X"), NULL, NULL, rbpFilt, ssGet) != RTNORM)
	{
		acutRelRb(rbpFilt);
		acedSSFree(ssGet);
		return false;
	}

	acutRelRb(rbpFilt);

	// Get the length of the selection
	//long lLength = 0L; acedSSLength(ssGet, &lLength);
	int lLength = 0L; acedSSLength(ssGet, &lLength);
	if (lLength != 0L) csLabel.Format(_T("%c%c"), char(65 + (int) lLength / 2), char(65 + (int) lLength / 2));
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: ChangeVisibility 
// Description  : Sets the VISIBILITY state in the block reference to a given value.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ChangeVisibility(AcDbObjectId eId, CString csProperty, CString csVisibilityState)
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
					if (wcscmp(blkProp.propertyName().kACharPtr(), csProperty) != 0) continue;

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetConduitParamsToDraw
// Description  : For the given conduit size the function retrieves the block name and the layer name from standard tables.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetConduitParamsToDraw(CString csStatus, CString csConduitSize, CString &csBlkName, CString &csVisibility, double &dCondDia, CString &csLayer)
{
	CQueryTbl tblDuctConduit;
	CString csSQL;
	csSQL.Format(L"SELECT fldBlock, fldVisibility, fldOuterSize, fldLayer FROM tblDuctConduit WHERE fldStatus = '%s' AND fldConduitSize = '%s'", csStatus, csConduitSize);
	if (!tblDuctConduit.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return false;
	if (!tblDuctConduit.GetRows()) return false;

	csBlkName		 = tblDuctConduit.GetRowAt(0)->GetAt(0);
	csVisibility = tblDuctConduit.GetRowAt(0)->GetAt(1);
	dCondDia     = _tstof(tblDuctConduit.GetRowAt(0)->GetAt(2));
	csLayer	     = tblDuctConduit.GetRowAt(0)->GetAt(3);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name: GetCableParamsToDraw
// Description  : For the given cable size the function retrieves the block name and the layer name from standard tables.
//////////////////////////////////////////////////////////////////////////
bool GetCableParamsToDraw(CString csStatus, CString csCableType, CString &csBlkName, double &dCableDia, CString &csLayer)
{
	CQueryTbl tblDuctConduit;
	CString csSQL;
	csSQL.Format(L"SELECT fldBlock, fldOuterSize, fldLayer FROM tblDuctCable WHERE fldStatus = '%s' AND fldCableType = '%s'", csStatus, csCableType);
	if (!tblDuctConduit.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return false;
	if (!tblDuctConduit.GetRows()) return false;

	csBlkName = tblDuctConduit.GetRowAt(0)->GetAt(0);
	dCableDia  = _tstof(tblDuctConduit.GetRowAt(0)->GetAt(1));
	csLayer	  = tblDuctConduit.GetRowAt(0)->GetAt(2);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: GetCableDetails() 
// Description  : Retrieves the records from tblCableDucts into global arrays. Used by duct tool to draw the duct cross section.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL GetCableDetails()
{
	// Reset the cable array's
	g_csaCableDia.RemoveAll(); g_csaCableColor.RemoveAll();

	// Read the cable sizes and colors
	CString csSQL;
	CQueryTbl tblCables;
	csSQL.Format(_T("SELECT [fldCableType], [fldCableDiameter], [fldFillColourNo] FROM tblCableDucts"));
	if (!tblCables.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, _T("GetCableDetails"),true)) return FALSE;
	if (tblCables.GetRows() <= 0) 
	{
		acutPrintf(_T("\nCable sizes and colors have not been identified in tblCableDucts.\nPlease contact your System Administrator.\n")); 
		return FALSE;
	}

	CString csCableDia;
	CString csCableCol;
	for (int iCtr = 0; iCtr < tblCables.GetRows(); iCtr++)
	{
		csCableDia.Format(L"%s#%s", tblCables.GetRowAt(iCtr)->GetAt(0), tblCables.GetRowAt(iCtr)->GetAt(1)); g_csaCableDia.Add(csCableDia);
		csCableCol.Format(L"%s#%s", tblCables.GetRowAt(iCtr)->GetAt(0), tblCables.GetRowAt(iCtr)->GetAt(2)); g_csaCableColor.Add(csCableCol);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name: GetDuctScales() 
// Description  : Retrieves the records from tblDuctCrossSection into global array. 
//                Used by duct tool to draw the duct cross section.
//////////////////////////////////////////////////////////////////////////
BOOL GetDuctScales()
{
	// Reset the cable array's
	g_csaDuctScale.RemoveAll();

	// Read the cable sizes and colors
	CString csSQL;
	CQueryTbl tblDucts;
	csSQL.Format(_T("SELECT [fldCrossSectionPlacement], [fldScaleFactor] FROM tblDuctCrossSection"));
	if (!tblDucts.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, _T("GetDuctScales"),true)) return FALSE;
	if (tblDucts.GetRows() <= 0) 
	{
		acutPrintf(_T("\nScales for duct cross sections have not been identified in tblDuctCrossSection.\nPlease contact your System Administrator.\n")); 
		return FALSE;
	}

	CString csScale;
	for (int iCtr = 0; iCtr < tblDucts.GetRows(); iCtr++) { csScale.Format(L"%s#%s", tblDucts.GetRowAt(iCtr)->GetAt(0), tblDucts.GetRowAt(iCtr)->GetAt(1)); g_csaDuctScale.Add(csScale); }
	return true;
}

//////////////////////////////////////////////////////////////////////////
// CDuctsDlg dialog
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CDuctsDlg, CDialog)

CDuctsDlg::CDuctsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDuctsDlg::IDD, pParent)
{
	m_bDrawFill= false;

	m_ptPrev[X] = -99999.9;
	m_ptPrev[Y] = -99999.9;
	m_ptPrev[Z] = 0.0;

	m_csCoverStrip = _T("");
	m_csCoverStripDepth = _T("");
	m_csNotes  = _T("");

	m_csTrenchWidth = _T("");
	m_csTrenchDepth = _T("");
	m_csLabel = _T("");

	m_csCols = _T("3");
	m_csRows = _T("3");

	m_iRows = 3;
	m_iCols = 3;

	m_bReference = true;
	m_bDrawTrench = true;

	m_dMaxUnderBDepth = 0.0;
	m_dMaxUnderBWidth = 0.0;
}

CDuctsDlg::~CDuctsDlg()
{
}

void CDuctsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_lcConduits);
	DDX_Control(pDX, IDC_DUCTS_WIDTH, m_cbTrenchWidth);
	DDX_Text(pDX, IDC_DUCTS_WIDTH, m_csTrenchWidth);
	DDX_Text(pDX, IDC_DUCTS_DEPTH, m_csTrenchDepth);
	DDX_Control(pDX, IDC_DUCT_STATUS, m_cbStatus);
	DDX_CBString(pDX, IDC_DUCT_STATUS, m_csStatus);
	DDX_Control(pDX, IDC_DUCT_COVERSTRIP, m_cbCoverStrip);
	DDX_CBString(pDX, IDC_DUCT_COVERSTRIP, m_csCoverStrip);
	DDX_Control(pDX, IDC_DUCT_COVERSDEPTH, m_cbCoverStripDepth);
	DDX_CBString(pDX, IDC_DUCT_COVERSDEPTH, m_csCoverStripDepth);
	DDX_Control(pDX, IDC_DUCTS_REFERENCE, m_btnReference);
	DDX_Text(pDX, IDC_DUCTS_NOTES,  m_csNotes);
	DDX_Control(pDX, IDC_DUCT_DRAWTRENCH, m_btnDrawTrench);
	DDX_Control(pDX, IDC_COLS, m_cbCol);
	DDX_CBString(pDX, IDC_COLS, m_csCols);
	DDX_Control(pDX, IDC_ROWS, m_cbRow);
	DDX_CBString(pDX, IDC_ROWS, m_csRows);
	DDX_Text(pDX, IDC_DUCTS_SECTION, m_csLabel);
	DDX_Control(pDX, IDC_DUCTS_NOTES, m_edNotes);
	DDX_CBString(pDX, IDC_DUCT_STATUS, m_csStatus);
	DDX_Control(pDX, IDC_STATIC_TRENCH, m_stTrench);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_stPreview);
	DDX_Control(pDX, IDC_DUCTS_DEPTH, m_edTrenchDepth);
	DDX_Control(pDX, IDC_STATIC_WIDTH, m_stWidth);
	DDX_Control(pDX, IDC_DUCT_DRAWFILL, m_btnDrawFill);
	DDX_Check(pDX, IDC_DUCT_DRAWFILL, m_bDrawFill);
}

BEGIN_MESSAGE_MAP(CDuctsDlg, CDialog)
	ON_BN_CLICKED(IDC_DUCT_REFRESH, &CDuctsDlg::OnBnClickedDuctRefresh)
	ON_BN_CLICKED(IDC_DUCT_COVERAGE, &CDuctsDlg::OnBnClickedDuctCoverage)
	ON_BN_CLICKED(IDC_DUCT_DRAWCROSSSECTION, &CDuctsDlg::OnBnClickedDuctDrawcrosssection)
	ON_CBN_EDITCHANGE(IDC_DUCT_COVERSDEPTH, &CDuctsDlg::OnCbnEditchangeDuctCoversdepth)
	ON_CBN_SELCHANGE(IDC_DUCT_COVERSDEPTH, &CDuctsDlg::OnCbnSelchangeDuctCoversdepth)
	ON_BN_CLICKED(IDCANCEL, &CDuctsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CLEAR, &CDuctsDlg::OnBnClickedClear)
	ON_CBN_SELCHANGE(IDC_COLS, &CDuctsDlg::OnCbnSelchangeCols)
	ON_CBN_SELCHANGE(IDC_ROWS, &CDuctsDlg::OnCbnSelchangeRows)
	ON_BN_CLICKED(IDC_CLOSE, &CDuctsDlg::OnBnClickedClose)
	ON_CBN_SELCHANGE(IDC_DUCT_STATUS, &CDuctsDlg::OnCbnSelchangeDuctStatus)
	ON_BN_CLICKED(IDCHELP, &CDuctsDlg::OnBnClickedChelp)
	ON_BN_CLICKED(IDC_DUCT_DRAWFILL, &CDuctsDlg::OnBnClickedDuctDrawFill)
END_MESSAGE_MAP()

// CDuctsDlg message handlers
//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnInitDialog()
// Description  :
//////////////////////////////////////////////////////////////////////////
BOOL CDuctsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// X Section types
	CQueryTbl tblDuctStatus;
	CString csSQL;
	csSQL.Format(L"SELECT [fldDuctStatus] FROM tblDuctStatus ORDER BY [fldSequence]");
	if (!tblDuctStatus.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return true;
	for (int iRow = 0; iRow < tblDuctStatus.GetRows(); iRow++) m_cbStatus.AddString(tblDuctStatus.GetRowAt(iRow)->GetAt(0));
			
	// Create the list control for the conduits and cables
	m_lcConduits.m_bCustomDraw = TRUE;
	m_lcConduits.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	// Get the columns on the grid
	m_lcConduits.InsertColumn(0, _T("Deep"), LVCFMT_CENTER, 37);
	for (int iCol = 1; iCol <= 8; iCol++) { m_lcConduits.InsertColumn(iCol, _T(""), LVCFMT_CENTER, 80); }
	
	// Build the conduit list control
	m_cbRow.SelectString(0, m_csRows);
	m_cbCol.SelectString(0, m_csCols); 
	BuildConduitGrid();

	// Set defaults
	if (m_csStatus.IsEmpty())			     
	{
		m_cbStatus.SetCurSel(0);
		m_btnDrawTrench.SetCheck(TRUE);
	}
	else 
		m_cbStatus.SelectString(-1, m_csStatus);

	if (m_csCoverStrip.IsEmpty())			 SetDlgItemText(IDC_DUCT_COVERSTRIP, L"None");	else SetDlgItemText(IDC_DUCT_COVERSTRIP, m_csCoverStrip);
	if (m_csCoverStripDepth.IsEmpty()) m_cbCoverStripDepth.SetCurSel(1);			        else SetDlgItemText(IDC_DUCT_COVERSDEPTH, m_csCoverStripDepth);
	
	// Get the next section label
	if (!GetSectionLabel(m_csLabel)) m_csLabel = _T("AA");
	SetDlgItemText(IDC_DUCTS_SECTION, m_csLabel);

	m_btnDrawTrench.SetCheck(m_bDrawTrench);
	m_btnReference.SetCheck(m_bReference);

	// Call the function to populate the trench width, cover strip depths and strip names based on the selected status
	OnCbnSelchangeDuctStatus();

	// Set the width and height if already present
	if (!m_csTrenchWidth.IsEmpty()) SetDlgItemText(IDC_DUCTS_WIDTH, m_csTrenchWidth);
	if (!m_csTrenchDepth.IsEmpty()) m_edTrenchDepth.SetWindowText(m_csTrenchDepth);
				
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::ShowImage()
// Description  : Sets the TRENCH section block based on the current status.
//////////////////////////////////////////////////////////////////////////////////////
void CDuctsDlg::ShowImage()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ARX's are installed here: C:\Program Files\Autodesk\AutoCAD 2011\NET CAD STANDARD\NET CAD
	// Images need to be stored here: C:\Program Files\Autodesk\AutoCAD 2011\NET CAD STANDARD\support\Images
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Form the bitmap path and check for its presence
	CString csBMPPath; 
	/**/ if (!m_csStatus.CompareNoCase(L"Underbore"))
		csBMPPath.Format(L"%s\\..\\Support\\Images\\SECT_BORE_PROP.bmp", g_csHome); 
	else if (!m_csStatus.CompareNoCase(L"Proposed"))
	{
		if (!IsDlgButtonChecked(IDC_DUCT_DRAWFILL)) 
			csBMPPath.Format(L"%s\\..\\Support\\Images\\SECT_UNFILL_PROP.bmp", g_csHome);
		else 
			csBMPPath.Format(L"%s\\..\\Support\\Images\\SECT_FILL_PROP.bmp", g_csHome);
	}
	else if (!m_csStatus.CompareNoCase(L"Existing"))
	{
		if (!IsDlgButtonChecked(IDC_DUCT_DRAWFILL))
			csBMPPath.Format(L"%s\\..\\Support\\Images\\SECT_UNFILL.bmp", g_csHome);
		else 
			csBMPPath.Format(L"%s\\..\\Support\\Images\\SECT_FILL.bmp", g_csHome);
	}

	if (_taccess(csBMPPath, 00) == -1) csBMPPath.Format(_T("%s\\..\\Support\\Images\\NoPreview.bmp"), g_csHome);
	HANDLE hImage = LoadImage(NULL, csBMPPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (hImage == NULL) appMessage(_T("Unable to load preview images of trench.\n") + csBMPPath);
	m_stPreview.SetBitmap((HBITMAP)hImage);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnCbnSelchangeDuctStatus()
// Description  : Set defaults for other controls based on the change here
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnCbnSelchangeDuctStatus()
{
	// Get the trench status
	if (m_cbStatus.GetCurSel() == CB_ERR) return;
	m_cbStatus.GetLBText(m_cbStatus.GetCurSel(), m_csStatus);

	// Based on this, set other controls and its values
	m_stWidth.SetWindowText(L"Width: ");
	if (!m_csStatus.CompareNoCase(L"Proposed"))	
	{
		m_stTrench.SetWindowText(L"Trench");
		
		m_btnDrawTrench.SetCheck(true); 
		m_btnDrawTrench.EnableWindow(true);
		m_btnDrawFill.EnableWindow(true);

		m_edTrenchDepth.EnableWindow(TRUE);
	}
	else
	{
		m_btnDrawTrench.SetCheck(false);

		if (!m_csStatus.CompareNoCase(L"Underbore"))
		{
			m_btnDrawFill.SetCheck(false);
			m_btnDrawFill.EnableWindow(false);
			m_stWidth.SetWindowText(L"Bore Dia: ");
			m_btnDrawTrench.EnableWindow(false);
		}
		else 
			m_btnDrawTrench.EnableWindow(true);

		m_stTrench.SetWindowText(L"Installation");

		// Trench depth should be disabled out for "Existing" and "Underbore"
		m_edTrenchDepth.EnableWindow(FALSE);
	}

	// Place the appropriate image to aid user input
	ShowImage();

	// Populate the Trench width's for the given status
	m_cbTrenchWidth.ResetContent();
		
	CQueryTbl tblDucts;
	CString csSQL;

	csSQL.Format(L"SELECT fldDuctWidth, Default FROM tblDuctWidth WHERE fldDuctStatus = '%s' ORDER BY fldSequence", m_csStatus);
	if (!tblDucts.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;

	for (int i = 0; i < tblDucts.GetRows(); i++)
	{
		m_cbTrenchWidth.AddString(tblDucts.GetRowAt(i)->GetAt(0));
		if (_tstoi(tblDucts.GetRowAt(i)->GetAt(1))) m_cbTrenchWidth.SelectString(-1, tblDucts.GetRowAt(i)->GetAt(0));

		// If the trench width is already specified in the previous instance of the command, that value is shown
		if (!m_csTrenchWidth.IsEmpty())
		{
			if (m_cbTrenchWidth.SelectString(-1, m_csTrenchWidth) == CB_ERR) m_cbTrenchWidth.SetWindowText(m_csTrenchWidth);
		}
	}

	// Populate the Trench cover depth's for the given status
	m_cbCoverStripDepth.ResetContent();
	csSQL.Format(L"SELECT fldDuctCoverDepth, Default FROM tblDuctCoverDepth WHERE fldDuctStatus = '%s' ORDER BY fldSequence", m_csStatus);
	if (!tblDucts.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;

	for (int i = 0; i < tblDucts.GetRows(); i++)
	{
		m_cbCoverStripDepth.AddString(tblDucts.GetRowAt(i)->GetAt(0));
		if (_tstoi(tblDucts.GetRowAt(i)->GetAt(1))) m_cbCoverStripDepth.SelectString(-1, tblDucts.GetRowAt(i)->GetAt(0));

		// If the cover depth is already specified in the previous instance of the command, that value is shown
		if (!m_csCoverStripDepth.IsEmpty())
		{
			if (m_cbCoverStripDepth.SelectString(-1, m_csCoverStripDepth) == CB_ERR) m_cbCoverStripDepth.SetWindowText(m_csCoverStripDepth);
		}
	}

	// Populate the Trench Cover Strip names
	m_cbCoverStrip.ResetContent();
	
	tblDucts.RemoveAll();
	csSQL.Format(L"SELECT fldDuctCoverStrip, Default FROM tblDuctCoverStrip WHERE fldDuctStatus = '%s' ORDER BY fldSequence", m_csStatus);
	if (!tblDucts.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;

	for (int i = 0; i < tblDucts.GetRows(); i++)
	{
		m_cbCoverStrip.AddString(tblDucts.GetRowAt(i)->GetAt(0));
		if (_tstoi(tblDucts.GetRowAt(i)->GetAt(1))) m_cbCoverStrip.SelectString(-1, tblDucts.GetRowAt(i)->GetAt(0));
	}

	// Call the refresh function
	OnBnClickedDuctRefresh();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::SpecifyConduit()
// Description  : For the given cell, displays the CONDUIT dialog to get the user input.
//////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctsDlg::SpecifyConduit(int iRow, int iCol)
{
	CString csConduit, csCable, csStatus;

	// Get the status of the trench
	int iSel = m_cbStatus.GetCurSel();
	if (iSel == CB_ERR) return;
	m_cbStatus.GetLBText(iSel, m_csStatus);
		
	// Get the cell data and from it the Conduit, Cable and its Status
	CString csCell = m_lcConduits.GetItemText(iRow, iCol);
	if (csCell != _T("N.A."))
	{
		csConduit = csCell.Mid(0, csCell.Find(_T(" | ")));
		csCable   = csCell.Mid(csCell.Find(_T(" | ")) + 3);
		
		// If the text has "(P)", it implies the conduit has proposed cable details. However, this additional information will only be there when the complete trench is existing!
		if (!m_csStatus.CompareNoCase(L"Existing"))
		{
			csStatus = L"Existing";
			if (csCable.Find(L" (P)") != -1) 
			{
				csCable = csCable.Mid(0, csCable.Find(L" (P)") + 1); csCable.Trim();
				csStatus = L"Proposed";
			}
		}
		else csStatus = m_csStatus;
	}
	else csStatus = m_csStatus;
		
	// Show the conduit dialog
	CConduitDlg dlgConduit;
	dlgConduit.m_csConduit			= csConduit;
	dlgConduit.m_csCable				= csCable;
	dlgConduit.m_csCableStatus	= csStatus;
	dlgConduit.m_csTrenchStatus = m_csStatus;

	if (dlgConduit.DoModal() == IDCANCEL) return;
	
	// Update the cell
	if (!dlgConduit.m_csConduit.CompareNoCase(_T("None"))) dlgConduit.m_csConduit.Empty();
	if (!dlgConduit.m_csCable.CompareNoCase(_T("None")))   dlgConduit.m_csCable.Empty();

	// If the status combo is "Enabled" get the status. This control is enabled only if the trench status is "Existing".
	if (!m_csStatus.CompareNoCase(L"Existing"))
	{
		// The cable text must be prefixed with "(P)" text if the cable status is "Proposed"
		if (!dlgConduit.m_csCableStatus.CompareNoCase(L"Proposed"))	dlgConduit.m_csCable += L" (P)";
	}

	csCell.Format(_T("%s | %s"), dlgConduit.m_csConduit, dlgConduit.m_csCable);

	// This is for updating the status ICON
	m_csConduitStatus = (dlgConduit.m_csCableStatus.CompareNoCase(m_csStatus) ? dlgConduit.m_csCableStatus : L"");
	m_lcConduits.SetItemText(iRow, iCol, csCell);
		
	// Call the refresh function
	OnBnClickedDuctRefresh();
}

//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::SetHeaderText()
// Description  : Sets the column labels for the list control.
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::SetHeaderText(int iCol, CString csText)
{
	CHeaderCtrl *pHeaderCtrl = m_lcConduits.GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	
	HDITEM hdi;
	enum   { sizeOfBuffer = 256 };
	TCHAR  lpBuffer[sizeOfBuffer];

	hdi.mask = HDI_TEXT;
	hdi.pszText = lpBuffer;
	hdi.cchTextMax = sizeOfBuffer;

	pHeaderCtrl->GetItem(iCol, &hdi);
	_tcscpy(hdi.pszText, csText);
	pHeaderCtrl->SetItem(iCol, &hdi);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::BuildConduitGrid()
// Description  : Constructs the grid control with appropriate values for Rows/Columns.
////////////////////////////////////////////////////////////////////////////////////////////
void CDuctsDlg::BuildConduitGrid()
{
	if (m_hWnd == NULL) return;

	// Get the number of columns selected
	int iSel = m_cbCol.GetCurSel();
	if (iSel == CB_ERR) return;

	m_cbCol.GetLBText(iSel, m_csCols);
	m_iCols = _tstof(m_csCols);

	// Get the number of rows selected
	iSel = m_cbRow.GetCurSel();
	if (iSel == CB_ERR) return;

	m_cbRow.GetLBText(iSel, m_csRows);
	m_iRows = _tstof(m_csRows);
		
	// Column headers
	CString csColumn;
	for (int iCol = 1; iCol < 8 + 1; iCol++)
	{
		if (iCol < m_iCols + 1) csColumn.Format(_T("Across %d"), iCol); else csColumn = _T("");
		SetHeaderText(iCol, csColumn);
	}

	// Delete all items
	m_lcConduits.DeleteAllItems();
	CString csRow;
	for (int iRow = 0; iRow < m_iRows; iRow++)
	{
		csRow.Format(_T("%d"), iRow + 1);
		m_lcConduits.InsertItem(iRow, csRow);

		for (int iCol = 1; iCol <= m_iCols; iCol++)
		{
			CConduitAndCableInfo conductorAndConduitInfo;
			if (getConduitAndCableItem(m_conduitConductorVectorInfo, iRow, iCol, conductorAndConduitInfo)) 
			{
				if (!conductorAndConduitInfo.m_csSelected.IsEmpty())
				{
					m_lcConduits.SetItemText(iRow, iCol, conductorAndConduitInfo.m_csSelected);
				}
				else
					m_lcConduits.SetItemText(iRow, iCol, _T(" | "));
			}
			else
			{
				// Change Request : A.2.2.3 (A.2.1.2) Clarifications relating to Ducts 
				// Old:
				// m_lcConduits.SetItemText(iRow, iCol, _T(" | "));
				m_lcConduits.SetItemText(iRow, iCol, _T("150 | "));
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnCbnSelchangeCols()/CDuctsDlg::OnCbnSelchangeRows
// Description  : Saves the current conduit/cable configuration. This is later used to build the grid based on the new row/column numbers.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnCbnSelchangeCols() { SaveConfig(TRUE); BuildConduitGrid(); OnBnClickedDuctRefresh(); }
void CDuctsDlg::OnCbnSelchangeRows() { SaveConfig(TRUE); BuildConduitGrid(); OnBnClickedDuctRefresh(); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnBnClickedDuctRefresh()
// Description  : Recalculates the trench dimensions based on the current grid values.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnBnClickedDuctRefresh()
{
	UpdateData();

	// Get the number of columns selected
	int iSel = m_cbCol.GetCurSel();
	m_cbCol.GetLBText(iSel, m_csCols);
	m_iCols = _tstof(m_csCols);

	// Get the number of rows selected
	iSel = m_cbRow.GetCurSel();
	m_cbRow.GetLBText(iSel, m_csRows);
	m_iRows = _tstof(m_csRows);

	// Calculate the maximum trench depth and maximum width
	CString csCellValue;
	CStringArray csaLabels;
	CUIntArray uiaQty;

	// NETCAD: Depth calculation can now be based on table tblDuctConduit field fldOuterSize. The outer size measurement includes the conduit size 
	// and the spacing requirement. 
	CQueryTbl tblDuctConduit;
	CString csSQL, csConduitSize;
	CStringArray csaConduitSizes;
	csSQL.Format(L"SELECT fldConduitSize, fldOuterSize FROM tblDuctConduit WHERE fldStatus = '%s'", m_csStatus);
	if (!tblDuctConduit.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;
	for (int iR = 0; iR < tblDuctConduit.GetRows(); iR++)
	{
		csConduitSize.Format(L"%s#%s", tblDuctConduit.GetRowAt(iR)->GetAt(0), tblDuctConduit.GetRowAt(iR)->GetAt(1));
		csaConduitSizes.Add(csConduitSize);
	}

	CQueryTbl tblDuctCable;
	CString csCableSize;
	CStringArray csaCableSizes;
	csSQL.Format(L"SELECT fldCableType, fldOuterSize FROM tblDuctCable WHERE fldStatus = '%s'", m_csStatus);
	if (!tblDuctConduit.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;
	for (int iR = 0; iR < tblDuctConduit.GetRows(); iR++)
	{
		csCableSize.Format(L"%s#%s", tblDuctConduit.GetRowAt(iR)->GetAt(0), tblDuctConduit.GetRowAt(iR)->GetAt(1));
		csaCableSizes.Add(csCableSize);
	}

	double dMaxTrenchWidth = 0.0;
	double dMaxTrenchDepth = 0.0;

	CString csCableType;
	CString csSize;

	double dTrenchDepth = 0.0;
	double dTrenchWidth = 70.0;
	double dUnderBDepth = 0.0;
	double dUnderBWidth = 0.0;
	m_dMaxUnderBDepth = 0.0;
	m_dMaxUnderBWidth = 0.0;

	// Calculate the Trench Depth. 
	for (int iCol = 1; iCol <= m_iCols; iCol++)
	{
		dTrenchDepth = 0.0;
		dUnderBDepth = 0.0;
						
		for (int iRow = 0; iRow <= m_iRows; iRow++)
		{
			// Get the text in the cell i.e it should be in the format <Conduit size>|<Cable>
			csCellValue = m_lcConduits.GetItemText(iRow, iCol); 

			// If the text doesn't have the delimiter ignore the cell
			if (csCellValue.Find(_T("|")) == -1) continue;

			// Get the text before "|" from the cell value
			csCableType   = csCellValue.Mid(csCellValue.Find(L"| ") + 2);
			csConduitSize = csCellValue.Mid(0, csCellValue.Find(_T(" | "))); 
			
			if (csConduitSize.IsEmpty()) 
			{
				// Where cables are direct buried, the tblDuctCable field fldOuterSize should be used in the Depth/Width calculation. 
				// Note that the value of outer size differs for Under bore situations to allow for different spacing. 
				GetParameterValue(csaCableSizes, csCableType, csSize, 0);
			}
			else
			{
				// Get the value from the tables)
				GetParameterValue(csaConduitSizes, csConduitSize, csSize, 0);
			}
						
			// Add 70mm spacing vertically between each conduit or direct buried cable 
			if (m_csStatus.CompareNoCase(L"Underbore"))
			{
				if (iRow > 0)	{	if (_tstof(csSize) > 0)	dTrenchDepth += 70.0;	}
			}
						
			dTrenchDepth += _tstof(csSize);  
			dUnderBDepth += _tstof(csSize);  

			// The last conduit will have 80mm space to the bottom
			if (m_csStatus.CompareNoCase(L"Underbore"))
			{
				if (iRow == m_iRows - 1) { dTrenchDepth += 80.0; }
			}
		}

		// Get the maximum depth
		dMaxTrenchDepth = ((dTrenchDepth > dMaxTrenchDepth) ? dTrenchDepth : dMaxTrenchDepth);
		if (dUnderBDepth > m_dMaxUnderBDepth)	{ m_dMaxUnderBDepth = dUnderBDepth; m_iUBCol = iCol; }
	}
	
	// Calculate the Trench Width 
	// for (int iRow = 0; iRow < m_iRows; iRow++) // CHNAGED ON 25.07.11 to correct the UB DIA calculation
	for (int iRow = 0; iRow < m_iRows; iRow++)
	{
		dTrenchWidth = 70;
		dUnderBWidth = 0.0;
		
		for (int iCol = 1; iCol <= m_iCols; iCol++)
		{
			// Get the text in the cell i.e it should be in the format <Conduit size>|<Cable>
			csCellValue = m_lcConduits.GetItemText(iRow, iCol); 

			// If the text doesn't have the delimiter ignore the cell
			if (csCellValue.Find(_T("|")) == -1) continue;

			// Get the text before "|" from the cell value
			csCableType   = csCellValue.Mid(csCellValue.Find(L"| ") + 2);
			csConduitSize = csCellValue.Mid(0, csCellValue.Find(_T(" | "))); 

			if (csConduitSize.IsEmpty()) 
			{
				// Where cables are direct buried, the tblDuctCable field fldOuterSize should be used in the Depth/Width calculation. 
				// Note that the value of outer size differs for Under bore situations to allow for different spacing. 
				GetParameterValue(csaCableSizes, csCableType, csSize, 0);
			}
			else
			{
				// Get the value from the tables)
				GetParameterValue(csaConduitSizes, csConduitSize, csSize, 0);
			}

			// Add 70mm spacing horizontally between each conduit or direct buried cable 
			if (iCol > 1)
			{
				// If there a cable or conduit add the 70 mm spacing b/w it and the cable above
				if (_tstof(csSize) > 0)	{ dTrenchWidth += 70.0; }
			}

			dTrenchWidth += _tstof(csSize);  
			dUnderBWidth += _tstof(csSize);  

			// The last conduit will have 80mm space to the bottom
			if (iCol == m_iCols) { dTrenchWidth += 70.0; }
		}

		// Get the maximum depth
		dMaxTrenchWidth = ((dTrenchWidth > dMaxTrenchWidth) ? dTrenchWidth : dMaxTrenchWidth);

		if ((dUnderBWidth > m_dMaxUnderBWidth)) { m_dMaxUnderBWidth = dUnderBWidth; m_iUBRow = iRow; }
	}
		
	// Depth calculation for Existing and Proposed is the greatest of the calculated depth for each vertical line of conduits / cables plus the cover depth.
	if (m_cbCoverStripDepth.GetCurSel() == CB_ERR)
		GetDlgItemText(IDC_DUCT_COVERSDEPTH, m_csCoverStripDepth); 
	else
		m_cbCoverStripDepth.GetLBText(m_cbCoverStripDepth.GetCurSel(), m_csCoverStripDepth);

	CString csDepth; 
	if (m_csStatus.CompareNoCase(L"Underbore")) 
		csDepth.Format(L"%d", int (dMaxTrenchDepth + _tstof(m_csCoverStripDepth) + 0.5));
	else
		csDepth.Format(L"%d", int (max(m_dMaxUnderBWidth, m_dMaxUnderBDepth) + 0.5));
	SetDlgItemText(IDC_DUCTS_DEPTH, csDepth);

	// Set the trench width with the calculated value
	CString csWidth; 
	if (!m_csStatus.CompareNoCase(L"Proposed"))
	{
		// For Proposed status, a width calculation should add 70mm
		csWidth.Format(_T("%.2f"), dMaxTrenchWidth);
	}
	else if (!m_csStatus.CompareNoCase(L"Underbore"))
		csWidth = csDepth;
	else
		csWidth.Format(_T("%.2f"), dMaxTrenchWidth);
	SetDlgItemText(IDC_DUCTS_WIDTH, csWidth);

	// Set the cover strip back to "None"
	SetDlgItemText(IDC_DUCT_COVERSTRIP, L"None");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnCbnEditchangeDuctCoversdepth()/OnCbnSelchangeDuctCoversdepth()
// Description  : Calls the trench calculations for the given inputs.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnCbnEditchangeDuctCoversdepth() { OnBnClickedDuctRefresh(); }
void CDuctsDlg::OnCbnSelchangeDuctCoversdepth()  { OnBnClickedDuctRefresh(); }


//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::SaveConfig()
// Description  : Saves the current grid configuration.
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::SaveConfig(bool bClearArray)
{
	// Clear the present values in the vector
	if (bClearArray) m_conduitConductorVectorInfo.clear();
	
	// Get the values specified in the conduit to an array 
	CString csCellValue, csDiameter;
	for (int iRow = 0; iRow < m_iRows; iRow++)
	{
		for (int iCol = 1; iCol <= m_iCols; iCol++)
		{
			// Create an instance of the class
			CConduitAndCableInfo conduitCableInfo;
			conduitCableInfo.m_iRow = iRow;
			conduitCableInfo.m_iCol = iCol;
			
			// Get the conduit and cable selected
			csCellValue = m_lcConduits.GetItemText(iRow, iCol); 
			conduitCableInfo.m_csSelected = csCellValue;

			if (csCellValue.Find(_T("|")) != -1)
			{
				// Get the cable type and from it the block name, cable diameter from the standard tables
				conduitCableInfo.m_csCableType = csCellValue.Mid(csCellValue.Find(_T(" | ")) + 3);
				conduitCableInfo.m_csCableType = conduitCableInfo.m_csCableType.Trim();
				conduitCableInfo.m_iColorIndex = 0;

				// If the cable has " (P)" suffix, then we will have to remove it
				CString csCable  = conduitCableInfo.m_csCableType;
				CString csStatus = m_csStatus;
				if (csCable.Find(L" (P)") != -1) 
				{
					csCable = csCable.Mid(0, csCable.Find(L" (P)") + 1);
					csCable.Trim();
					csStatus = L"Proposed";
				}

				/**/ if (!csCable.IsEmpty() && csCable.CompareNoCase(_T("None")))
				{
					if (!GetCableParamsToDraw(csStatus, csCable, conduitCableInfo.m_csBlkNameForCable, conduitCableInfo.m_dCableDia, conduitCableInfo.m_csLayerForCable)) return; 
				}
 
				// Get the conduit diameter, layer and block name for the specified conduit size
				csCellValue = csCellValue.Mid(0, csCellValue.Find(_T(" | "))); 
				if (csCellValue.IsEmpty()) 
				{
					if (!csCable.IsEmpty())	conduitCableInfo.m_dCoduitDia	= 80.0;
				}
				else
				{
					if (!GetConduitParamsToDraw(m_csStatus, csCellValue, conduitCableInfo.m_csBlkNameForConduit, conduitCableInfo.m_csVisibilityForConduit, conduitCableInfo.m_dCoduitDia, conduitCableInfo.m_csLayerForConduit)) continue;
				}
			}

			// Add the instance to the array
			if (bClearArray)
				m_conduitConductorVectorInfo.push_back(conduitCableInfo);
			else if (iRow + (iCol - 1) < m_conduitConductorVectorInfo.size())
				m_conduitConductorVectorInfo[iRow + (iCol - 1)] = conduitCableInfo;
			else
				m_conduitConductorVectorInfo.push_back(conduitCableInfo);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::ValidateInputs()
// Description  : Before exiting the dialog to draw the trench and conduit/cable representation, the given inputs are validated and confirmed.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDuctsDlg::ValidateInputs() 
{
	UpdateData(TRUE);

	// Status
	m_cbStatus.GetLBText(m_cbStatus.GetCurSel(), m_csStatus);

	// Get the number of columns selected
	int iSel = m_cbCol.GetCurSel();
	m_cbCol.GetLBText(iSel, m_csCols);
	m_iCols = _tstof(m_csCols);

	// Get the number of rows selected
	iSel = m_cbRow.GetCurSel();
	m_cbRow.GetLBText(iSel, m_csRows);
	m_iRows = _tstof(m_csRows);

	// Get the value of trench width
	if (m_csTrenchWidth.IsEmpty())      { ShowBalloon(_T("\nSpecify trench width."), this, IDC_DUCTS_WIDTH, _T("")); return false; }
	if (_tstof(m_csTrenchWidth) <= 0.0) { ShowBalloon(_T("\nSpecify valid trench width."), this, IDC_DUCTS_WIDTH, _T("")); return false; }

	if (m_csTrenchDepth.IsEmpty())      { ShowBalloon(_T("\nSpecify trench depth."), this, IDC_DUCTS_DEPTH, _T("")); return false; }
	if (_tstof(m_csTrenchDepth) <= 0.0) { ShowBalloon(_T("\nSpecify valid trench depth."), this, IDC_DUCTS_DEPTH, _T("")); return false; }

	// Cover depth && Trench depth should not be less than cover depth
	if (_tstof(m_csCoverStripDepth) <= 0.0) { ShowBalloon(_T("\nSpecify valid cover depth."), this, IDC_DUCT_COVERSDEPTH, _T("")); return false; }
	if (m_csStatus.CompareNoCase(L"Underbore") && (_tstof(m_csTrenchDepth) <= _tstof(m_csCoverStripDepth))) { ShowBalloon(_T("\nTrench depth must be greater than cover depth."), this, IDC_DUCTS_DEPTH, _T("")); return false; }

	// Get the values for the check boxes
	m_bReference  = m_btnReference.GetCheck();
	m_bDrawTrench = m_btnDrawTrench.GetCheck();
	m_bDrawFill   = m_btnDrawFill.GetCheck();

	// Cover strip
	GetDlgItemText(IDC_DUCT_COVERSTRIP, m_csCoverStrip); m_csCoverStrip = m_csCoverStrip.Trim();
	if (m_csCoverStrip.IsEmpty()) { ShowBalloon(_T("\nSpecify valid cover strip."), this, IDC_DUCT_COVERSTRIP, _T("")); return false; }

	SaveConfig(true);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name: OnBnClickedDuctCoverage
// Description  :
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnBnClickedDuctCoverage() 
{
	// Used to call the appropriate function when the dialog is dismissed
	m_iDlgExitIndex = 1;

	// Validate the specified inputs
	if (!ValidateInputs()) return;

	// Exit the dialog
	OnOK(); 
}

///////////////////////////////////////////////////////////////////////////////////////////
// Function name: DrawConduitsAndCables()
// Description   : Draws the conduits and cables for Proposed and Existing status
///////////////////////////////////////////////////////////////////////////////////////////
void CDuctsDlg::DrawConduitsAndCables(ads_point ptSection, double dScale, bool bMSPace, AcDbObjectIdArray &aryXSectionObjIds)
{
	double dSpacing = 0.0;
	double dMaxDia;
	double dTotalDia;
	double dConduitDia;
	int iNoOfConduits;
	CString csCellValue;
	ads_point ptConduit, ptFirst, ptSecond;
	AcDbObjectIdArray objHatchBndy;
	
	// Location of the first conduit from the corner of the trench
	acutPolar(ptSection, 0.0,   70.0 * dScale, ptConduit);
	acutPolar(ptConduit, PIby2, 80.0 * dScale, ptConduit);

	for (int iRow = m_iRows - 1; iRow >= 0; iRow--)
	{
		dMaxDia = 0.0;	   // Maximum diameter of the conduit in this row
		dTotalDia = 0.0;
		iNoOfConduits = 0;
		AcGeVector3d normal(0, 0, 1);

		// Get the spacing to place the conduits in this ROW, when there is more than one column
		if (m_iCols > 1)
		{
			for (int iCol = 1; iCol <= m_iCols; iCol++)
			{
				// Get this conduit and cable item
				CConduitAndCableInfo conduitAndCableInfo;
				if (!getConduitAndCableItem(m_conduitConductorVectorInfo, iRow, iCol, conduitAndCableInfo)) continue; 
				if (!conduitAndCableInfo.m_dCoduitDia) continue;

				dTotalDia += (conduitAndCableInfo.m_dCoduitDia);
				iNoOfConduits++;
			}

			dSpacing = (((_tstof(m_csTrenchWidth) - 140.0) - dTotalDia) / (iNoOfConduits - 1)) * dScale;
		}
		
		for (int iCol = 1; iCol <= m_iCols; iCol++)
		{
			// Get this conduit and cable item
			CConduitAndCableInfo conduitAndCableInfo;
			if (!getConduitAndCableItem(m_conduitConductorVectorInfo, iRow, iCol, conduitAndCableInfo)) continue; 
			
			if (!(dConduitDia = conduitAndCableInfo.m_dCoduitDia * dScale)) continue;
			dMaxDia = (dConduitDia > dMaxDia) ? dConduitDia : dMaxDia;
						
			// Insert the block representing the CONDUIT
			acutPolar(ptConduit, 0.0, (dConduitDia / 2.0), ptConduit);
			if (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForConduit, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForConduit); }

				AcDbObjectId objConduit;
				if (!insertBlock(conduitAndCableInfo.m_csBlkNameForConduit, m_csLayer, ptConduit, 1.0, 1.0, 0.0, 0.0, _T(""), objConduit, bMSPace)) return;
				aryXSectionObjIds.append(objConduit);
				objHatchBndy.append(objConduit);

				// Change the visibility status of the conduit 
				ChangeVisibility(objConduit, L"Conduit Size", conduitAndCableInfo.m_csVisibilityForConduit);
			}

			// Insert the block representing the CABLE
			if (!conduitAndCableInfo.m_csBlkNameForCable.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				// acutPrintf(L"\nHere [%d,%d] [%s]", iRow, iCol, conduitAndCableInfo.m_csBlkNameForCable);
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForCable, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForCable); }

				AcDbObjectId objCable;
				if (insertBlock(conduitAndCableInfo.m_csBlkNameForCable, m_csLayer, ptConduit, 1.0, 1.0, 0.0, 0.0, _T(""), objCable, bMSPace))
				{
					aryXSectionObjIds.append(objCable);
				}
			}

			// Calculate the location of the next conduit
			acutPolar(ptConduit, 0.0, (dConduitDia / 2.0) + dSpacing, ptConduit);
		}

		// Shift the section to the next row
		acutPolar(ptConduit, PIby2, dMaxDia + (70 * dScale), ptConduit); 
		ptConduit[X] = ptSection[X] + (70.0 * dScale);
	}

	// Draw the polygon representing the portion of the trench representing the conduits
	if (m_bDrawFill)
	{
		// Calculate the corners to draw the fill boundary
		ads_point ptFill1    = { ptSection[X], ptSection[Y], 0.0 };
		ads_point ptFill2    = { ptSection[X] + (_tstof(m_csTrenchWidth) * dScale), ptSection[Y] + (_tstof(m_csTrenchDepth) * dScale) - (_tstof(m_csCoverStripDepth) - 80.0) * dScale, 0.0 };

		// Draw the fill boundary
		acedCommandS(RTSTR, L".RECTANG", RTPOINT, ptFill1, RTPOINT, ptFill2, NULL);
		ads_name enBoundary; acdbEntLast(enBoundary); ChangeProperties(FALSE, enBoundary, 0, m_csLayer);
		AcDbObjectId objBndy; acdbGetObjectId(objBndy, enBoundary);
		aryXSectionObjIds.append(objBndy);
		objHatchBndy.append(objBndy);

		// Hatch now
		if (objHatchBndy.logicalLength() > 0)
		{
			ads_name enResult;
			AcDbObjectId objId;

			acedCommandS(RTSTR, L".HATCH", RTSTR, g_csProposedPattern, RTREAL, 1.0, RTSTR, L"0", NULL);
			
			for (int iBdy = 0; iBdy < objHatchBndy.logicalLength(); iBdy++)
			{
				acdbGetAdsName(enResult, objHatchBndy.at(iBdy));
				acedCommandS(RTENAME, enResult, NULL);
			}
			acedCommandS(RTSTR, L"", NULL);

			// Get the hatch drawn and append it to section array
			acdbEntLast(enResult); 
			ChangeProperties(FALSE, enResult, 0, m_csLayer);
			acdbGetObjectId(objId, enResult);
			aryXSectionObjIds.append(objId);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Function name: DrawConduitsAndCablesForUBColumn()
// Description   : Draws the conduits and cables for Under bore status
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::DrawConduitsAndCablesForUBColumn(ads_point ptSection, double dScale, bool bMSPace, AcDbObjectIdArray &aryXSectionObjIds)
{
	double dMaxDia;
	double dTotalDia;
	double dConduitDia;
	int iNoOfConduits;
	CString csCellValue;
	ads_point ptConduit, ptFirst;

	acutPolar(ptSection, 0.0, 0.0, ptConduit);

	// Determine if a row or column has the maximum diameter
	double dOffsetX, dOffsetY;
	AcGeVector3d normal(0, 0, 1);
	CConduitAndCableInfo conduitAndCableInfo;
	double dLHSDistFromAxis = 0.0;
	double dRHSDistFromAxis = 0.0;

	for (int iRow = m_iRows - 1; iRow >= 0; iRow--)
	{
		// Draw the column with the biggest diameter in this row
		if (!getConduitAndCableItem(m_conduitConductorVectorInfo, iRow, m_iUBCol, conduitAndCableInfo)) { continue; } 
		if (conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty() || !(dConduitDia = conduitAndCableInfo.m_dCoduitDia * dScale))
		{
			if (!(dConduitDia = conduitAndCableInfo.m_dCableDia * dScale)) continue;
		}

		if (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty())
		{
			// Get the block definition from the template (if not available locally)
			if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForConduit, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForConduit); }

			AcDbObjectId objConduit;
			if (!insertBlock(conduitAndCableInfo.m_csBlkNameForConduit, m_csLayer, ptConduit, 1.0, 1.0, 0.0, 0.0, _T(""), objConduit, bMSPace)) return;
			aryXSectionObjIds.append(objConduit);

			// Change the visibility status of the conduit 
			ChangeVisibility(objConduit, L"Conduit Size", conduitAndCableInfo.m_csVisibilityForConduit);
		}

		// Insert the block representing the CABLE
		if (!conduitAndCableInfo.m_csBlkNameForCable.IsEmpty())
		{
			// Get the block definition from the template (if not available locally)
			if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForCable, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForCable); }

			AcDbObjectId objCable;
			if (insertBlock(conduitAndCableInfo.m_csBlkNameForCable, m_csLayer, ptConduit, 1.0, 1.0, 0.0, 0.0, _T(""), objCable, bMSPace)) aryXSectionObjIds.append(objCable);
		}

		double dMaxConduitDia = dConduitDia;

		///////////////////////////////////////////////////////////////
		// Draw all the conduits to the LEFT of the bigger conduit
		///////////////////////////////////////////////////////////////
		ads_point ptOther; 
		for (int iCol = m_iUBCol; iCol >= 1; iCol--)
		{
			// Get the offset of this conduit from the location of the biggest conduit on this row
			if (!getConduitAndCableItem(m_conduitConductorVectorInfo, iRow, iCol, conduitAndCableInfo)) { continue; } 
			if (conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty() || !(dConduitDia = conduitAndCableInfo.m_dCoduitDia * dScale))
			{
				if (!(dConduitDia = conduitAndCableInfo.m_dCableDia * dScale)) continue;
			}

			// The biggest column is already drawn, so let us skip it
			if (iCol == m_iUBCol) 
			{
				acutPolar(ptConduit, PIby2, dConduitDia / 2, ptOther);
				acutPolar(ptOther,   PI,    dConduitDia / 2, ptOther);

				if (iRow == m_iUBRow)	dLHSDistFromAxis = dConduitDia / 2;
				continue;
			}

			if ((iRow == m_iUBRow) && (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty() || !conduitAndCableInfo.m_csBlkNameForCable.IsEmpty()))	dLHSDistFromAxis += dConduitDia;
						
			acutPolar(ptOther, PI,				 dConduitDia / 2, ptOther);
			acutPolar(ptOther, MinusPIby2, dConduitDia / 2, ptOther);
						
			if (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForConduit, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForConduit); }

				AcDbObjectId objConduit;
				if (!insertBlock(conduitAndCableInfo.m_csBlkNameForConduit, m_csLayer, ptOther, 1.0, 1.0, 0.0, 0.0, _T(""), objConduit, bMSPace)) return;
				aryXSectionObjIds.append(objConduit);

				// Change the visibility status of the conduit 
				ChangeVisibility(objConduit, L"Conduit Size", conduitAndCableInfo.m_csVisibilityForConduit);
			}

			// Insert the block representing the CABLE
			if (!conduitAndCableInfo.m_csBlkNameForCable.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForCable, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForCable); }

				AcDbObjectId objCable;
				if (insertBlock(conduitAndCableInfo.m_csBlkNameForCable, m_csLayer, ptOther, 1.0, 1.0, 0.0, 0.0, _T(""), objCable, bMSPace)) aryXSectionObjIds.append(objCable);
			}
		}

		///////////////////////////////////////////////////////////////
		// Draw all the conduits to the RIGHT of the bigger conduit
		///////////////////////////////////////////////////////////////
		for (int iCol = m_iUBCol; iCol <= m_iCols; iCol++)
		{
			// Get the offset of this conduit from the location of the biggest conduit on this row
			if (!getConduitAndCableItem(m_conduitConductorVectorInfo, iRow, iCol, conduitAndCableInfo)) { continue; } 
			if (conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty() || !(dConduitDia = conduitAndCableInfo.m_dCoduitDia * dScale))
			{
				if (!(dConduitDia = conduitAndCableInfo.m_dCableDia * dScale)) continue;
			}

			// The biggest column is already drawn, so let us skip it
			if (iCol == m_iUBCol) 
			{
				// Relocate to RHS quadrant of the biggest conduit
				acutPolar(ptConduit, PIby2, dConduitDia / 2, ptOther);
				acutPolar(ptOther,   0.0,   dConduitDia / 2, ptOther);

				if (iRow == m_iUBRow)	dRHSDistFromAxis = dConduitDia / 2;
				continue;
			}
			
			if ((iRow == m_iUBRow) && (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty() || !conduitAndCableInfo.m_csBlkNameForCable.IsEmpty())) dRHSDistFromAxis += dConduitDia;
			
			acutPolar(ptOther, 0.0,				 dConduitDia / 2, ptOther);
			acutPolar(ptOther, MinusPIby2, dConduitDia / 2, ptOther);

			if (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForConduit, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForConduit); }

				AcDbObjectId objConduit;
				if (!insertBlock(conduitAndCableInfo.m_csBlkNameForConduit, m_csLayer, ptOther, 1.0, 1.0, 0.0, 0.0, _T(""), objConduit, bMSPace)) return;
				aryXSectionObjIds.append(objConduit);

				// Change the visibility status of the conduit 
				ChangeVisibility(objConduit, L"Conduit Size", conduitAndCableInfo.m_csVisibilityForConduit);
			}

			// Insert the block representing the CABLE
			if (!conduitAndCableInfo.m_csBlkNameForCable.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForCable, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForCable); }

				AcDbObjectId objCable;
				if (insertBlock(conduitAndCableInfo.m_csBlkNameForCable, m_csLayer, ptOther, 1.0, 1.0, 0.0, 0.0, _T(""), objCable, bMSPace)) aryXSectionObjIds.append(objCable);
			}

			acutPolar(ptOther, PIby2, dConduitDia / 2, ptOther);
			acutPolar(ptOther, 0.0,		dConduitDia / 2, ptOther);
		}

		// Relocate to draw the next row
		acutPolar(ptConduit, PIby2, dMaxConduitDia, ptConduit);
	}

	/////////////////////////////////////////////
	// Draw the UNDERBORE and dimension it
	/////////////////////////////////////////////
	ads_point ptUB;
	
	acutPolar(ptSection, 0.0,  (dRHSDistFromAxis - dLHSDistFromAxis) / 2, ptUB);
	acutPolar(ptUB,      PIby2, m_dMaxUnderBDepth * dScale / 2,      ptUB);
 
	// Place the SECT_BORE_PROP block
	if (!acdbTblSearch(L"BLOCK", L"SECT_BORE_PROP", FALSE)) {	UpdateBlockFromStandards(L"SECT_BORE_PROP"); }

	AcDbObjectId objTrench;
	if (!insertBlock(L"SECT_BORE_PROP", m_csLayer, ptUB, 1.0, 1.0, 0.0, 0.0, _T(""), objTrench, bMSPace)) return;
	aryXSectionObjIds.append(objTrench);

	// Change the reference value
	ChangeAttributeValueAndRelocate(objTrench, L"REFERENCE", m_csLabel, 75 * dScale, false);

	// Set the NOTES 
	SetNotes(objTrench, m_csNotes, 75 * dScale);
	
	// The custom parameters or the inserted block are to be modified
	CString csValue; 
	csValue.Format(L"%s", suppressZero(_tstof(m_csCoverStripDepth) * dScale));  ChangeCustomParameters(objTrench, L"COVER_DIM", csValue);
	csValue.Format(L"%s", suppressZero(max(m_dMaxUnderBWidth, m_dMaxUnderBDepth) * dScale / 2));  ChangeCustomParameters(objTrench, L"BORE_SIZE", csValue);
}

//////////////////////////////////////////////////////////////////////////
// Function name: DrawConduitsAndCablesForUBRow()
// Description   : Draws the conduits and cables for Under bore status
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::DrawConduitsAndCablesForUBRow(ads_point ptSection, double dScale, bool bMSPace, AcDbObjectIdArray &aryXSectionObjIds)
{
	double dMaxDia;
	double dTotalDia;
	double dConduitDia;
	int iNoOfConduits;
	CString csCellValue;
	ads_point ptConduit, ptFirst;
	double dOffsetX, dOffsetY;
	AcGeVector3d normal(0, 0, 1);
	CConduitAndCableInfo conduitAndCableInfo;
	double dHORDistFromSect = 0.0;
	double dMaxYCoord = -999.999;
	double dMinYCoord = -999.999;
	double dMaxXCoord = -999.999;
	double dMinXCoord = -999.999;
	double dMaxConduitDia;
	ads_point ptOther; 
	AcDbObjectId objConduit;
	AcDbObjectId objCable;
	double dYCoord = 0.0;

	acutPolar(ptSection, 0.0, 0.0, ptConduit);
	for (int iCol = 1; iCol <= m_iCols; iCol++)
	{
		// Draw the row which defines the BORE DIAMETER at this column
		if (!getConduitAndCableItem(m_conduitConductorVectorInfo, m_iUBRow, iCol, conduitAndCableInfo)) { continue; } 
		if (conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty() || !(dConduitDia = conduitAndCableInfo.m_dCoduitDia * dScale))
		{
			if (conduitAndCableInfo.m_csBlkNameForCable.IsEmpty()) continue;
			if (!(dConduitDia = conduitAndCableInfo.m_dCableDia * dScale)) continue;
		}
		
		if (iCol != 1)
		{
			// Calculate the location of the first conduit/cable for second column onwards. The first column is placed at PICK point, this will not be required
			acutPolar(ptConduit, 0.0, dConduitDia / 2, ptConduit);
			acutPolar(ptConduit, MinusPIby2, dConduitDia / 2, ptConduit);
		}
		else
		{
			// Radius of the first conduit/cable that is LHS of PICK point
			dHORDistFromSect = dConduitDia / 2;
		}

		// Used to place the BORE DIA block
		/**/ if ((dMinYCoord == -999.999) || (dMinYCoord > (ptConduit[Y]))) dMinYCoord = ptConduit[Y];
		else if ((dMaxYCoord == -999.999) || (dMaxYCoord < (ptConduit[Y] + dConduitDia))) {	dMaxYCoord = ptConduit[Y] + dConduitDia; }

		/**/ if ((dMinXCoord == -999.999) || (dMinXCoord > (ptConduit[X] - dConduitDia / 2))) dMinXCoord = ptConduit[X] - dConduitDia / 2;
		else if ((dMaxXCoord == -999.999) || (dMaxXCoord < (ptConduit[X] + dConduitDia / 2))) dMaxXCoord = ptConduit[X] + dConduitDia / 2;

		if (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty())
		{
			// Get the block definition from the template (if not available locally)
			if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForConduit, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForConduit); }
			if (!insertBlock(conduitAndCableInfo.m_csBlkNameForConduit, m_csLayer, ptConduit, 1.0, 1.0, 0.0, 0.0, _T(""), objConduit, bMSPace)) return;

			aryXSectionObjIds.append(objConduit);

			// Change the visibility status of the conduit 
			ChangeVisibility(objConduit, L"Conduit Size", conduitAndCableInfo.m_csVisibilityForConduit);
		}

		// Insert the block representing the CABLE
		if (!conduitAndCableInfo.m_csBlkNameForCable.IsEmpty())
		{
			// Get the block definition from the template (if not available locally)
			if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForCable, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForCable); }
			if (insertBlock(conduitAndCableInfo.m_csBlkNameForCable, m_csLayer, ptConduit, 1.0, 1.0, 0.0, 0.0, _T(""), objCable, bMSPace)) aryXSectionObjIds.append(objCable);
		}

		// Draw all the conduits/cables to the TOP of the ROW defining the BORE DIA
		dMaxConduitDia = dConduitDia;

		for (int iRow = m_iUBRow; iRow >= 0; iRow--)
		{
		  // Get the conduit and cable diameter
			if (!getConduitAndCableItem(m_conduitConductorVectorInfo, iRow, iCol, conduitAndCableInfo)) { continue; } 
			if (conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty() || !(dConduitDia = conduitAndCableInfo.m_dCoduitDia * dScale))
			{
				if (conduitAndCableInfo.m_csBlkNameForCable.IsEmpty()) continue;
				if (!(dConduitDia = conduitAndCableInfo.m_dCableDia * dScale)) continue;
			}
			
			// The conduit/cable at the BORE_DIA is already drawn, so let us skip it. But we will have to relocate the insertion point to place the next conduit/cable.
			if (iRow == m_iUBRow) 
			{
				acutPolar(ptConduit, PIby2, dConduitDia, ptOther);
				continue;
			}
			
			/**/ if ((dMinYCoord == -999.999) || (dMinYCoord > (ptOther[Y]))) dMinYCoord = ptOther[Y];
			else if ((dMaxYCoord == -999.999) || (dMaxYCoord < (ptOther[Y] + dConduitDia))) {	dMaxYCoord = ptOther[Y] + dConduitDia; }

			/**/ if ((dMinXCoord == -999.999) || (dMinXCoord > (ptOther[X] - dConduitDia / 2))) dMinXCoord = ptOther[X] - dConduitDia / 2;
			else if ((dMaxXCoord == -999.999) || (dMaxXCoord < (ptOther[X] + dConduitDia / 2))) dMaxXCoord = ptOther[X] + dConduitDia / 2;
									
			if (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForConduit, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForConduit); }

				AcDbObjectId objConduit;
				if (!insertBlock(conduitAndCableInfo.m_csBlkNameForConduit, m_csLayer, ptOther, 1.0, 1.0, 0.0, 0.0, _T(""), objConduit, bMSPace)) return;
				aryXSectionObjIds.append(objConduit);

				// Change the visibility status of the conduit 
				ChangeVisibility(objConduit, L"Conduit Size", conduitAndCableInfo.m_csVisibilityForConduit);
			}

			// Insert the block representing the CABLE
			if (!conduitAndCableInfo.m_csBlkNameForCable.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForCable, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForCable); }

				AcDbObjectId objCable;
				if (insertBlock(conduitAndCableInfo.m_csBlkNameForCable, m_csLayer, ptOther, 1.0, 1.0, 0.0, 0.0, _T(""), objCable, bMSPace)) aryXSectionObjIds.append(objCable);
			}

			acutPolar(ptOther, PIby2, dConduitDia, ptOther);
		}

		// Draw all the conduits to the BOTTOM of the bigger conduit
		for (int iRow = m_iUBRow; iRow < m_iRows; iRow++)
		{
			// Get the offset of this conduit from the location of the biggest conduit on this row
			if (!getConduitAndCableItem(m_conduitConductorVectorInfo, iRow, iCol, conduitAndCableInfo)) { continue; } 
			if (conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty() || !(dConduitDia = conduitAndCableInfo.m_dCoduitDia * dScale))
			{
				if (conduitAndCableInfo.m_csBlkNameForCable.IsEmpty()) continue;
				if (!(dConduitDia = conduitAndCableInfo.m_dCableDia * dScale)) continue;
			}

			// The biggest conduit/cable is already drawn, so let us skip it
			if (iRow == m_iUBRow) 
			{
				acutPolar(ptConduit, 0.0, 0.0, ptOther); 
				continue; 
			}
			
			// Calculate the location to place this conduit/cable
			acutPolar(ptOther, MinusPIby2, dConduitDia, ptOther);
			/**/ if ((dMinYCoord == -999.999) || (dMinYCoord > (ptOther[Y]))) dMinYCoord = ptOther[Y];
			else if ((dMaxYCoord == -999.999) || (dMaxYCoord < (ptOther[Y] + dConduitDia)))	{	dMaxYCoord = ptOther[Y] + dConduitDia; }

			/**/ if ((dMinXCoord == -999.999) || (dMinXCoord > (ptOther[X] - dConduitDia / 2))) dMinXCoord = ptOther[X] - dConduitDia / 2;
			else if ((dMaxXCoord == -999.999) || (dMaxXCoord < (ptOther[X] + dConduitDia / 2))) dMaxXCoord = ptOther[X] + dConduitDia / 2;

			if (!conduitAndCableInfo.m_csBlkNameForConduit.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForConduit, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForConduit); }

				AcDbObjectId objConduit;
				if (!insertBlock(conduitAndCableInfo.m_csBlkNameForConduit, m_csLayer, ptOther, 1.0, 1.0, 0.0, 0.0, _T(""), objConduit, bMSPace)) return;
				aryXSectionObjIds.append(objConduit);

				// Change the visibility status of the conduit 
				ChangeVisibility(objConduit, L"Conduit Size", conduitAndCableInfo.m_csVisibilityForConduit);
			}

			// Insert the block representing the CABLE
			if (!conduitAndCableInfo.m_csBlkNameForCable.IsEmpty())
			{
				// Get the block definition from the template (if not available locally)
				if (!acdbTblSearch(L"BLOCK", conduitAndCableInfo.m_csBlkNameForCable, FALSE)) {	UpdateBlockFromStandards(conduitAndCableInfo.m_csBlkNameForCable); }

				AcDbObjectId objCable;
				if (insertBlock(conduitAndCableInfo.m_csBlkNameForCable, m_csLayer, ptOther, 1.0, 1.0, 0.0, 0.0, _T(""), objCable, bMSPace)) aryXSectionObjIds.append(objCable);
			}
		}

		// Relocate to draw the next column
		if (iCol != m_iCols)
		{
			acutPolar(ptConduit, PIby2, dMaxConduitDia / 2, ptConduit);
			acutPolar(ptConduit, 0.0,   dMaxConduitDia / 2, ptConduit);
		}
	}

	// Draw the UNDERBORE and dimension it
	ads_point ptUB;
	double dDistX    = (dMinXCoord + dMaxXCoord) / 2.0;
	double dDistY    = (dMinYCoord + dMaxYCoord) / 2.0;
	ptUB[X] = dDistX;
	ptUB[Y] = dDistY;

	// Place the SECT_BORE_PROP block
	if (!acdbTblSearch(L"BLOCK", L"SECT_BORE_PROP", FALSE)) {	UpdateBlockFromStandards(L"SECT_BORE_PROP"); }

	AcDbObjectId objTrench;
	if (!insertBlock(L"SECT_BORE_PROP", m_csLayer, ptUB, 1.0, 1.0, 0.0, 0.0, _T(""), objTrench, bMSPace)) return;
	aryXSectionObjIds.append(objTrench);

	// Change the reference value
	ChangeAttributeValueAndRelocate(objTrench, L"REFERENCE", m_csLabel, 75 * dScale, false);
	
	// Set the NOTES
	SetNotes(objTrench, m_csNotes, 75 * dScale);

	// The custom parameters or the inserted block are to be modified
	CString csValue; 
	csValue.Format(L"%s", suppressZero(_tstof(m_csCoverStripDepth) * dScale));  ChangeCustomParameters(objTrench, L"COVER_DIM", csValue);
	csValue.Format(L"%s", suppressZero(max(m_dMaxUnderBWidth, m_dMaxUnderBDepth) * dScale / 2));  ChangeCustomParameters(objTrench, L"BORE_SIZE", csValue);
}

//////////////////////////////////////////////////////////////////////////
// Function name: DrawCrosssectionPreview
// Description  : Draws the cross section at the given point.
// Arguments    : Point on the bottom mid of the cross section.
//////////////////////////////////////////////////////////////////////////
int CDuctsDlg::DrawCrosssectionPreview(ads_point ptMid) 
{  
	// Determine where the cross section information has to be appended
	bool bMSPace = true;
	if (!acdbHostApplicationServices()->workingDatabase()->tilemode()) bMSPace = false;
		
	// Depending on the DXF Scale used, the cross section dimensions are to be scaled for better visibility
	double dScale = 0.001;
			
	Acad::ErrorStatus es;
	AcDbObjectIdArray aryXSectionObjIds; // Object Id array of all elements in the the cross section
	while (T)
	{
		// Ask the insertion point for the cross section. If the reference mark is placed, show an elastic line from the mid of the reference
		// else elastic line is not required.
		ads_point ptSection;
		acedInitGet(RSG_NONULL, _T(""));
		int iRet = acedGetPoint((m_bReference ? ptMid : NULL), _T("\nSpecify insertion point for cross section: "), ptSection);

		/**/ if (iRet == RTCAN)	 return iRet;
		else if (iRet != RTNORM) continue;

		double dWidth = (_tstof(m_csTrenchWidth) * dScale);
		double dDepth = (_tstof(m_csTrenchDepth) * dScale);

		
		CString csValue; 
		if (!m_csStatus.CompareNoCase(L"Proposed"))
		{
			// Insert the SECT_PROP block
			if (!acdbTblSearch(L"BLOCK", L"SECT_PROP", FALSE)) {	UpdateBlockFromStandards(L"SECT_PROP"); }

			AcDbObjectId objTrench;
			if (!insertBlock(L"SECT_PROP", m_csLayer, ptSection, 1.0, 1.0, 0.0, 0.0, _T(""), objTrench, bMSPace)) return false;
			aryXSectionObjIds.append(objTrench);

			// Change the reference value
			ChangeAttributeValueAndRelocate(objTrench, L"REFERENCE", m_csLabel, 75 * dScale, false);

			// Set the NOTES
			SetNotes(objTrench, m_csNotes, 75 * dScale);
			
			// The custom parameters or the inserted block are to be modified
			csValue.Format(L"%s", suppressZero(_tstof(m_csCoverStripDepth) * dScale)); ChangeCustomParameters(objTrench, L"COVER_DIM", csValue);
			csValue.Format(L"%s", suppressZero(dWidth));  ChangeCustomParameters(objTrench, L"X_DIM", csValue);
			csValue.Format(L"%s", suppressZero(dDepth )); ChangeCustomParameters(objTrench, L"Y_DIM", csValue);

			// Change the cover strip visibility state
			ChangeVisibility(objTrench, L"COVER", m_csCoverStrip + (!m_bDrawTrench ? L" - W/O Trench" : L""));
		}
		else if (!m_csStatus.CompareNoCase(L"Existing"))
		{
			// Insert the SECT_PROP block
			if (!acdbTblSearch(L"BLOCK", L"SECT_EXIST", FALSE)) {	UpdateBlockFromStandards(L"SECT_EXIST"); }

			AcDbObjectId objTrench;
			if (!insertBlock(L"SECT_EXIST", m_csLayer, ptSection, 1.0, 1.0, 0.0, 0.0, _T(""), objTrench, bMSPace)) return false;
			aryXSectionObjIds.append(objTrench);

			// Change the reference value
			ChangeAttributeValueAndRelocate(objTrench, L"REFERENCE", m_csLabel, 75 * dScale, false);

			// Set the NOTES
			SetNotes(objTrench, m_csNotes, 75 * dScale);

			// The custom parameters or the inserted block are to be modified
			csValue.Format(L"%s", suppressZero(_tstof(m_csCoverStripDepth) * dScale)); ChangeCustomParameters(objTrench, L"COVER_DIM", csValue);
			csValue.Format(L"%s", suppressZero(dWidth));  ChangeCustomParameters(objTrench, L"X_DIM", csValue);
			csValue.Format(L"%s", suppressZero(dDepth )); ChangeCustomParameters(objTrench, L"Y_DIM", csValue);

			// Change the cover strip visibility state
			ChangeVisibility(objTrench, L"COVER", m_csCoverStrip + (!m_bDrawTrench ? L" - W/O Trench" : L""));
		}
			
		// Draw the conduits with cables
		if (m_csStatus.CompareNoCase(L"Underbore"))
			DrawConduitsAndCables(ptSection, dScale, bMSPace, aryXSectionObjIds);
		else if (m_dMaxUnderBDepth >= m_dMaxUnderBWidth)
			DrawConduitsAndCablesForUBColumn(ptSection, dScale, bMSPace, aryXSectionObjIds);
		else
			DrawConduitsAndCablesForUBRow(ptSection, dScale, bMSPace, aryXSectionObjIds);

		// Scale the drawn trench and cable details as per the DXF Scale for the drawing
		double dScaleFactor = 1.0;
		struct resbuf *rbpScale = getXRecordFromDictionary(_T("eCapture"), _T("DXF Scale"));
		if (rbpScale)
		{
			CQueryTbl tblDuctCrossSection;
			CString csSQL;
			csSQL.Format(L"SELECT fldScaleFactor FROM tblDuctCrossSection WHERE fldCrossSectionPlacement = '%s'", suppressZero(rbpScale->rbnext->resval.rreal));

			if (!tblDuctCrossSection.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return false;
			if (tblDuctCrossSection.GetRows()) dScaleFactor = _tstof(tblDuctCrossSection.GetRowAt(0)->GetAt(0));
		}
					
		while (T)
		{
			// Ask the user the rotation angle
			double dRotation = 0.0;
			ads_point ptRotation;
			TCHAR pszInput[8]; 

			CString csPrompt; csPrompt.Format(_T("\nRotate/Undo/Next/Cancel/<Accept>: "), dRotation);
			acedInitGet(NULL, _T("Rotate Accept Next Undo Cancel"));
			iRet = acedGetKword(csPrompt, pszInput);
			/**/ if (iRet == RTCAN) 
			{
				// Erase all the entities drawn
				deleteArray(aryXSectionObjIds);
				aryXSectionObjIds.removeAll();

				return RTERROR;
			}
			else if (iRet == RTNONE) return RTNORM;
			else 
			{
				/**/ if (!_tcsicmp(pszInput, _T("Rotate")))
				{
					// Ask the rotation angle
					double dRefAngle = 0.0;
					while (T)
					{
						double dNewAngle;
						iRet = acedGetAngle(ptSection, _T("\nSpecify rotation angle or ESC to continue: "), &dNewAngle);
						dNewAngle = RTD(dNewAngle);
						if (iRet == RTCAN) break;
						else if (iRet == RTNORM)
						{
							ads_name enName;
							Acad::ErrorStatus es;
							for (int iCtr = 0; iCtr < aryXSectionObjIds.length(); iCtr++)
							{
								es = acdbGetAdsName(enName, aryXSectionObjIds.at(iCtr));
								if (es != Acad::eOk) continue;
		
								saveOSMode();
								acedCommandS(RTSTR, _T(".ROTATE"), RTENAME, enName, RTSTR, _T(""), RTPOINT, ptSection, RTSTR, _T("R"), RTREAL, dRefAngle, RTREAL, dNewAngle, NULL);
								restoreOSMode();
							}

							// Useful when a new rotation angle is specified (we will know how the base is aligned now)
							dRefAngle = dNewAngle;
						}
					}
				}
				else if (!_tcsicmp(pszInput, _T("Undo")))
				{
					// Erase all the entities drawn
					deleteArray(aryXSectionObjIds);
					aryXSectionObjIds.removeAll();
					break;
				}
				else if (!_tcsicmp(pszInput, _T("Cancel")))
				{
					// Erase all the entities drawn
					deleteArray(aryXSectionObjIds);
					aryXSectionObjIds.removeAll();

					return RTCAN;
				}
				else if (!_tcsicmp(pszInput, _T("Accept"))) return RTNORM; 
				else if (!_tcsicmp(pszInput, _T("Next")))   return RTERROR; // Actually RTNORM but we will have to bring the dialog back, so.
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::DrawCrosssection()
// Description  : Calls the functions to draw the reference marks and cross sections.
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDuctsDlg::DrawCrosssection()
{
	// The reference mark need not be placed, so go ahead and place only the cross section
	while (!m_bReference)
	{
		ads_point ptMid = { -99999.99, -99999.99, 0.0 };
		int iRet = DrawCrosssectionPreview(ptMid);
		if ((iRet == RTCAN) || (iRet == RTNORM)) m_iDlgExitIndex = 3; return;
	}

	// Determine where the cross section information has to be appended
	bool bMSPace = true;
	if (!acdbHostApplicationServices()->workingDatabase()->tilemode()) bMSPace = false;

	// Depending on the DXF Scale used, the cross section dimensions are to be scaled for better visibility
	double dScale = 0.001;

	/*
	struct resbuf *rbpScale = getXRecordFromDictionary(_T("eCapture"), _T("DXF Scale"));
	
	// Check if the view is made in paper space or model space. If in paper space, the cross section need not be scaled.
	if (acdbHostApplicationServices()->workingDatabase()->tilemode() && rbpScale) 
	{
		if (rbpScale->rbnext) 
		{
			dScale = rbpScale->rbnext->resval.rreal;
			CString csParam; csParam.Format(L"%d", (int) dScale);
			CString csValue; GetParameterValue(g_csaDuctScale, csParam, csValue, 0);
			if (!csValue.IsEmpty()) dScale = _tstof(csValue);
		}
	}
	else if (rbpScale)
	{
		CString csValue; GetParameterValue(g_csaDuctScale, L"Paper Space", csValue, 0); 
		if (!csValue.IsEmpty()) dScale = _tstof(csValue);
	}
	*/
	
	// The following code is to place the reference mark and them draw the cross section
	while (T)
	{
		// Allow user to pick the start point
		ads_point ptFirst;
		acedInitGet(RSG_NONULL, _T(""));
		if (acedGetPoint(NULL, _T("\nSpecify first point of cross section reference line: "), ptFirst) == RTCAN) return;

		// Allow user to pick the end point
		ads_point ptSecond;
		acedInitGet(RSG_NONULL, _T(""));
		if (acedGetPoint(ptFirst, _T("\nSpecify second point of cross section reference line: "), ptSecond) == RTCAN) return;

		// Angle to rotate the direction block
		double dRotation = acutAngle(ptFirst, ptSecond) + PIby2;

		AcDbObjectId objLine;
		AcDbObjectId objFirst;
		AcDbObjectId objFirstLabel;
		AcDbObjectId objSecond;
		AcDbObjectId objSecondLabel;
		ads_point ptSection;
		ads_point ptText;
		TCHAR result[512];
		int iRet;
		AcDbObjectIdArray ary_ObjIds;
		acdbRegApp(_T("EA_SECTIONLABEL"));
		struct resbuf *rbpXData = acutBuildList(AcDb::kDxfRegAppName, _T("EA_SECTIONLABEL"), AcDb::kDxfXdInteger16, 1, NULL);

		while (T)
		{
			// Clear the drawn objects
			ary_ObjIds.removeAll();

			// Draw the cross section line
			AcDbLine *pLine = new AcDbLine(AcGePoint3d(ptFirst[X], ptFirst[Y], 0.0), AcGePoint3d(ptSecond[X], ptSecond[Y], 0.0));
			
			pLine->setLayer(m_csLayer);
			pLine->setLinetype(_T("BYLAYER"));
			pLine->setColor(g_ByLayerColor);

			appendEntityToDatabase(pLine, bMSPace);
			objLine = pLine->objectId();
			
			ary_ObjIds.append(pLine->objectId());
			pLine->close();

			//////////////////////////////////////////////////////////////////////////
			// Show the direction as PLINE at the START
			//////////////////////////////////////////////////////////////////////////
			// double dSectionSize = 0.15 * acutDistance(ptFirst, ptSecond);
			// double dSectionSize = dScale * 2 * 0.025;
			double dSectionSize = 75 * dScale;
			
			AcDbPolyline *pSectionFirst = new AcDbPolyline(2);
			pSectionFirst->addVertexAt(0, AcGePoint2d(ptFirst[X], ptFirst[Y]));   
			acutPolar(ptFirst, dRotation, dSectionSize, ptSection);
			pSectionFirst->addVertexAt(1, AcGePoint2d(ptSection[X], ptSection[Y])); 

			pSectionFirst->setLayer(m_csLayer);
			pSectionFirst->setLinetype(_T("BYLAYER"));
			pSectionFirst->setColor(g_ByLayerColor);

			appendEntityToDatabase(pSectionFirst, bMSPace);

			pSectionFirst->setWidthsAt(0, dSectionSize, 0.0);

			objFirst = pSectionFirst->objectId();
			pSectionFirst->close();
			ary_ObjIds.append(objFirst);
			
			//////////////////////////////////////////////////////////////////////////
			// Show the section label at the START
			//////////////////////////////////////////////////////////////////////////
			acutPolar(ptSection, dRotation,  dSectionSize * 0.20, ptText);
			AcDbText *pTextFirst = new AcDbText(AcGePoint3d(ptText[X], ptText[Y], 0.0), m_csLabel.Mid(0, 1), AcDbObjectId::kNull, dSectionSize, dRotation - PIby2);
			pTextFirst->setHorizontalMode(AcDb::kTextCenter);
			pTextFirst->setAlignmentPoint(AcGePoint3d(ptText[X], ptText[Y], 0.0));

			pTextFirst->setLayer(m_csLayer);
			pTextFirst->setColor(g_ByLayerColor);
			pTextFirst->setHeight(dSectionSize);

			pTextFirst->setXData(rbpXData);
			
			appendEntityToDatabase(pTextFirst, bMSPace);

			objFirstLabel = pTextFirst->objectId();
			pTextFirst->close();
			ary_ObjIds.append(objFirstLabel);

			//////////////////////////////////////////////////////////////////////////
			// Show the direction as PLINE at the END
			//////////////////////////////////////////////////////////////////////////

			AcDbPolyline *pSectionSecond = new AcDbPolyline(2);
			pSectionSecond->setWidthsAt(0, dSectionSize, 0.0);
			pSectionSecond->addVertexAt(0, AcGePoint2d(ptSecond[X],  ptSecond[Y]));   

			acutPolar(ptSecond, dRotation, dSectionSize, ptSection);
			pSectionSecond->addVertexAt(1, AcGePoint2d(ptSection[X], ptSection[Y])); 

			pSectionSecond->setLayer(m_csLayer);
			pSectionSecond->setColor(g_ByLayerColor);

			appendEntityToDatabase(pSectionSecond, bMSPace);
			pSectionSecond->setWidthsAt(0, dSectionSize, 0.0);

			objSecond = pSectionSecond->objectId();
			pSectionSecond->close();
			ary_ObjIds.append(objSecond);

			//////////////////////////////////////////////////////////////////////////
			// Show the section label at the END
			//////////////////////////////////////////////////////////////////////////

			acutPolar(ptSection, dRotation, dSectionSize * 0.20, ptText);
			AcDbText *pTextSecond = new AcDbText(AcGePoint3d(ptText[X], ptText[Y], 0.0), m_csLabel.Mid(0, 1), AcDbObjectId::kNull, dSectionSize, dRotation - PIby2);
			pTextSecond->setHorizontalMode(AcDb::kTextCenter);
			pTextSecond->setAlignmentPoint(AcGePoint3d(ptText[X], ptText[Y], 0.0));

			pTextSecond->setLayer(m_csLayer);
			pTextSecond->setColor(g_ByLayerColor);

			pTextSecond->setXData(rbpXData);
			pTextSecond->setHeight(dSectionSize);

			appendEntityToDatabase(pTextSecond, bMSPace);

			objSecondLabel = pTextSecond->objectId();
			pTextSecond->close();
			ary_ObjIds.append(objSecondLabel);

			//////////////////////////////////////////////////////////////////////////
			// Define a group (Section)
			//////////////////////////////////////////////////////////////////////////
			AcDbDictionary  *pGroupDict = NULL;
			acdbCurDwg()->getGroupDictionary(pGroupDict, AcDb::kForWrite);

			AcDbGroup *pGroup = new AcDbGroup; 
			AcDbObjectId  groupObjectId;

			CTime tm = CTime::GetCurrentTime();
			CString sTm = tm.Format("%H%M%S%d%m%Y");
			pGroupDict->setAt(m_csLabel + sTm, pGroup, groupObjectId);
			pGroupDict->close();

			pGroup->append(objLine);
			pGroup->append(objFirst);
			pGroup->append(objFirstLabel);
			pGroup->append(objSecond);
			pGroup->append(objSecondLabel);

			pGroup->close();

			//////////////////////////////////////////////////////////////////////////
			// Now with the options
			//////////////////////////////////////////////////////////////////////////

			acedInitGet(NULL, _T("Flip Accept aLign Undo"));
			iRet = acedGetKword(_T("\nFlip side/Undo/aLign/<Accept>: "), result);
			/**/ if (iRet == RTNONE) 
			{
				break;
			}
			else if (iRet == RTNORM)
			{ 
				if (result[0] == 'F') 
				{
					dRotation += PI;	
					if (dRotation >= (2 * PI))	dRotation = dRotation - (2 * PI);

					// Remove the appended entities, so that the new set of entities on the flip side can be created
					deleteArray(ary_ObjIds);
					ary_ObjIds.removeAll();
				}
				else if (result[1] == 'L') 
				{
					// Allow the user to select a line to align
					ads_point ptDummy;
					ads_name enSelect;
					int iRetAlign = acedEntSel(L"\nSelect a line to align the mark: ", enSelect, ptDummy);
					if (iRetAlign == RTNORM)
					{
						// Validate the line
						AcDbObjectId objSelect;
						acdbGetObjectId(objSelect, enSelect);

						AcDbEntity *pEntity;
						Acad::ErrorStatus es = acdbOpenObject(pEntity, objSelect, AcDb::kForRead);
						if (es == Acad::eOk) 
						{
							ads_point ptInters; 
							if (pEntity->isKindOf(AcDbLine::desc()) || pEntity->isKindOf(AcDbArc::desc()) || pEntity->isKindOf(AcDbPolyline::desc()) || pEntity->isKindOf(AcDb2dPolyline::desc()))
							{
								pEntity->close();

								//////////////////////////////////////////////////////////////////////////
								// Get the distance b/w them
								//////////////////////////////////////////////////////////////////////////
								struct resbuf rbOsnap; rbOsnap.restype = RTPOINT;  
								acutPolar(ptFirst, 0.0, 0.0, rbOsnap.resval.rpoint); acedSetVar(L"LASTPOINT", &rbOsnap);
								
								if (acedOsnap(ptDummy, L"PER", ptInters) == RTNORM)
								{
									acutPolar(ptFirst, acutAngle(ptFirst, ptInters), acutDistance(ptFirst, ptSecond), ptSecond);
									dRotation = acutAngle(ptFirst, ptSecond) + PIby2;
									
									// Remove the appended entities, so that the new set of entities on the flip side can be created
									deleteArray(ary_ObjIds);
									ary_ObjIds.removeAll();
								}
							}
							else pEntity->close();
						}
					}
					else break;
				}
				else if (result[0] == 'U')
				{
					// Remove the appended entities, so that the new set of entities on the flip side can be created
					deleteArray(ary_ObjIds);
					ary_ObjIds.removeAll();
					iRet = RTERROR;
					break;
				}
				else break;
			}
			else if (iRet == RTCAN) 
			{
				deleteArray(ary_ObjIds);
				ary_ObjIds.removeAll();

				m_iDlgExitIndex = 3;
				return;
			}
		}

		// Call the function to draw the cross section
		if (iRet == RTERROR) continue;

		ads_point ptMid = { (ptFirst[X] + ptSecond[X]) / 2.0, (ptFirst[Y] + ptSecond[Y]) / 2.0, 0.0 };
		iRet = DrawCrosssectionPreview(ptMid);
		if (iRet == RTCAN)
		{
			deleteArray(ary_ObjIds);
			ary_ObjIds.removeAll();
			m_iDlgExitIndex = 3;
			return;
		}
		else if (iRet == RTNORM)
		{
			m_iDlgExitIndex = 3;
			return;
		}

		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnBnClickedDuctDrawcrosssection()
// Description  : 
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnBnClickedDuctDrawcrosssection()
{
	// Used to call the appropriate function when the dialog is dismissed
	m_iDlgExitIndex = 2;

	// Validate the specified inputs
	if (!ValidateInputs()) return;

	// Exit the dialog
	OnOK(); 
}

//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnBnClickedClear()
// Description  : Resets all values in controls to its defaults
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnBnClickedClear()
{
	// Confirm that the "Clear" button clears the cable leaving 3 Across, 3 deep and with no cable/conduit details. 
	m_iRows = 3;
	m_iCols = 3;

	m_csRows.Format(_T("%d"), m_iRows);
	m_csCols.Format(_T("%d"), m_iCols); 

	m_cbCol.SelectString(0, m_csCols);
	m_cbRow.SelectString(0, m_csRows);

	m_conduitConductorVectorInfo.clear();
	BuildConduitGrid();

	// Confirm Trench is reset to "Proposed" Cover depth reset to 450, both check boxes reset to checked and reference notes cleared
	m_cbStatus.SetCurSel(0);
	m_btnDrawTrench.SetCheck(TRUE);
	m_cbCoverStripDepth.SetCurSel(1);

	m_bDrawTrench = TRUE; m_btnDrawTrench.SetCheck(m_bDrawTrench);
	m_bReference = TRUE;	m_btnReference.SetCheck(m_bReference);

	// Clear the reference notes
	SetDlgItemText(IDC_DUCTS_NOTES, L"");
	
	// Set section
	SetDlgItemText(IDC_DUCTS_SECTION, L"AA");

	// Reset the trench dimensions calculated
	OnBnClickedDuctRefresh();

	// Remove the highlight
	for (int iR = 0; iR < m_lcConduits.GetItemCount(); iR++) m_lcConduits.SetItemState(iR, 0, LVIS_SELECTED);
}

//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnBnClickedDuctDrawFill()
//
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnBnClickedDuctDrawFill() { ShowImage(); }

//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnBnClickedCancel()
// Description  : 
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnBnClickedCancel() { OnCancel(); }

//////////////////////////////////////////////////////////////////////////
// Function name: CDuctsDlg::OnBnClickedHelp()
// Description  : Calls the help window with the context displayed
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnBnClickedChelp() { displayHelp((DWORD)_T("Duct.htm")); }

//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
void CDuctsDlg::OnBnClickedClose()
{
	UpdateData(TRUE);

	// Get the number of columns selected
	int iSel = m_cbCol.GetCurSel();
	m_cbCol.GetLBText(iSel, m_csCols);
	m_iCols = _tstof(m_csCols);

	// Get the number of rows selected
	iSel = m_cbRow.GetCurSel();
	m_cbRow.GetLBText(iSel, m_csRows);
	m_iRows = _tstof(m_csRows);

	// Get the values for the check boxes
	m_bReference  = m_btnReference.GetCheck();
	m_bDrawTrench = m_btnDrawTrench.GetCheck();

	SaveConfig(true);

	m_iDlgExitIndex = 3;
	OnOK();
}

//////////////////////////////////////////////////////////////////////////
// Function name: Command_Duct()
// Description  : Function that defines the command NCS.
//////////////////////////////////////////////////////////////////////////
void Command_Duct()
{
	// Display the Duct dialog
	CDuctsDlg dlgDuct;
	g_ByLayerColor.setColor(AcCmEntityColor::kByLayer);

	// GetCableDetails();

	int iRet = RTERROR;
	while (T)
	{
		// Get the previously specified values (if any)
		struct resbuf *rbpDucts = getXRecordFromDictionary(_T("eCapture"), _T("Ducts"));
		if (rbpDucts)
		{
			dlgDuct.m_csStatus = rbpDucts->resval.rstring; rbpDucts = rbpDucts->rbnext; 
			dlgDuct.m_iRows		 = rbpDucts->resval.rint;    rbpDucts = rbpDucts->rbnext; dlgDuct.m_csRows.Format(L"%d", dlgDuct.m_iRows);
			dlgDuct.m_iCols    = rbpDucts->resval.rint;    rbpDucts = rbpDucts->rbnext; dlgDuct.m_csCols.Format(L"%d", dlgDuct.m_iCols);

			for (int iRow = 0; iRow < dlgDuct.m_iRows; iRow++)
			{
				for (int iCol = 0; iCol < dlgDuct.m_iCols; iCol++)
				{
					CConduitAndCableInfo conduitCableInfo;
					conduitCableInfo.m_iRow       = rbpDucts->resval.rint;    rbpDucts = rbpDucts->rbnext;
					conduitCableInfo.m_iCol       = rbpDucts->resval.rint;    rbpDucts = rbpDucts->rbnext;
					conduitCableInfo.m_csSelected = rbpDucts->resval.rstring; rbpDucts = rbpDucts->rbnext;

					dlgDuct.m_conduitConductorVectorInfo.push_back(conduitCableInfo);
				}
			}

			dlgDuct.m_csTrenchDepth     = rbpDucts->resval.rstring; rbpDucts = rbpDucts->rbnext; 
			dlgDuct.m_csTrenchWidth     = rbpDucts->resval.rstring; rbpDucts = rbpDucts->rbnext;
			dlgDuct.m_csCoverStripDepth = rbpDucts->resval.rstring; rbpDucts = rbpDucts->rbnext;
			dlgDuct.m_csCoverStrip      = rbpDucts->resval.rstring; rbpDucts = rbpDucts->rbnext;
			dlgDuct.m_csNotes           = rbpDucts->resval.rstring; rbpDucts = rbpDucts->rbnext;
			dlgDuct.m_csLabel           = rbpDucts->resval.rstring; rbpDucts = rbpDucts->rbnext;
			dlgDuct.m_bReference        = rbpDucts->resval.rint;    rbpDucts = rbpDucts->rbnext;
			dlgDuct.m_bDrawTrench       = rbpDucts->resval.rint;    rbpDucts = rbpDucts->rbnext; 
		}

		// Call the duct dialog
		if (dlgDuct.DoModal() == IDCANCEL) return;

		// Set the dimension style
		// Command_Dimensions();

		// Create the layers "BASE_SECTION_EXIST" and "BASE_SECTION_PROP". Depending on the trench status, the details of the cross section are placed on relevant
		// layers. However, a proposed cable of an existing trench will be placed on BASE_SECTION_PROP though the other details are drawn on BASE_SECTION_EXIST
		if (!dlgDuct.m_csStatus.CompareNoCase(_T("Existing"))) dlgDuct.m_csLayer = _T("BASE_SECTION_EXIST"); else dlgDuct.m_csLayer = _T("BASE_SECTION_PROP");
		createLayer(_T("BASE_SECTION_EXIST"), Adesk::kFalse, Adesk::kFalse);
		createLayer(_T("BASE_SECTION_PROP"),  Adesk::kFalse, Adesk::kFalse);

		// Call the appropriate function depending on the button selected
		/**/ if (dlgDuct.m_iDlgExitIndex == 1)	{ /*dlgDuct.DrawDuctCoverage();	acedCommand( RTSTR, _T(".REGEN"), NULL); */ }
		else if (dlgDuct.m_iDlgExitIndex == 2)	{	dlgDuct.DrawCrosssection();	acedCommandS( RTSTR, _T(".REGEN"), NULL); }

		// Store the DUCTS information for future use
		rbpDucts = acutBuildList(AcDb::kDxfText, dlgDuct.m_csStatus, AcDb::kDxfInt16, dlgDuct.m_iRows, AcDb::kDxfInt16, dlgDuct.m_iCols, NULL);

		int iIndex = 0;
		struct resbuf *rbpTemp = NULL;
		CString csSelected;
		int iRowInLst = 0;
		int iColInLst = 0;
		for (int iRow = 0; iRow < dlgDuct.m_iRows; iRow++)
		{
			for (int iCol = 0; iCol < dlgDuct.m_iCols; iCol++)
			{
				CConduitAndCableInfo conduitCableInfo;
				csSelected = " | ";
				if (iIndex < dlgDuct.m_conduitConductorVectorInfo.size()) 
				{
					conduitCableInfo = dlgDuct.m_conduitConductorVectorInfo[iIndex];

					csSelected = conduitCableInfo.m_csSelected;
					iRowInLst  = conduitCableInfo.m_iRow;
					iColInLst  = conduitCableInfo.m_iCol;
				}

				iIndex++;

				for (rbpTemp = rbpDucts; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);
				rbpTemp->rbnext = acutBuildList(AcDb::kDxfInt16, iRowInLst, AcDb::kDxfInt16, iColInLst, AcDb::kDxfText, csSelected, NULL);
			}
		}

		for (rbpTemp = rbpDucts; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);
		rbpTemp->rbnext = acutBuildList(AcDb::kDxfText,  dlgDuct.m_csTrenchDepth, 
			AcDb::kDxfText,  dlgDuct.m_csTrenchWidth, 
			AcDb::kDxfText,  dlgDuct.m_csCoverStripDepth, 
			AcDb::kDxfText,  dlgDuct.m_csCoverStrip, 
			AcDb::kDxfText,  dlgDuct.m_csNotes, 
			AcDb::kDxfText,	 dlgDuct.m_csLabel,
			AcDb::kDxfInt16, ((dlgDuct.m_bReference == true) ? 1 : 0), 
			AcDb::kDxfInt16, ((dlgDuct.m_bDrawTrench == true) ? 1 : 0),
			NULL);

		addXRecordToDictionary(_T("eCapture"), _T("Ducts"), rbpDucts);
		acutRelRb(rbpDucts);

		// Exit the command if the user cancelled the dialog
		if (dlgDuct.m_iDlgExitIndex == 3) return;
	}
}


/*
//////////////////////////////////////////////////////////////////////////
// Utility functions (*** PRESENTLY NOT USED ***)
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

//////////////////////////////////////////////////////////////////////////
// Function name: CreateEATextStyle
// Description  :
//////////////////////////////////////////////////////////////////////////
void CreateEATextStyle(double &dHeight)
{
	// Get the text values from tblText
	CQueryTbl tblText;
	if (!tblText.SqlRead(DSN_DWGTOOLS, _T("SELECT [Style], [Font], [Height], [WidthFactor], [ObliqueAngle], [TTForSHX] FROM tblText ORDER BY [ID]"), __LINE__, _T(__FILE__), __FUNCTION__))) return;

	CString csStyle;
	for (int iRow = 0; iRow < tblText.GetRows(); iRow++)
	{
		CStringArray *pszRow = tblText.GetRowAt(iRow);

		// Create the text style
	dHeight = _tstof(pszRow->GetAt(2));
	csStyle.Format(L"eCapture_%s", (TCHAR *)(LPCTSTR) pszRow->GetAt(0));
	CreateTextStyle(csStyle, (TCHAR *)(LPCTSTR) pszRow->GetAt(1), 0.0, _tstof(pszRow->GetAt(3)), _tstof(pszRow->GetAt(4)), pszRow->GetAt(5));
}

//////////////////////////////////////////////////////////////////////////
// Function name: Command_Dimensions
// Description  : Function that 
//////////////////////////////////////////////////////////////////////////
void Command_Dimensions()
{
  switchOff();

  // Call the function to set the TEXT style
  double dDummy;
  CreateEATextStyle(dDummy);

  // Get the dimension variables and its proposed values from the table and set them all
  CQueryTbl tblDimensions;
  if (!tblDimensions.SqlRead(DSN_DWGTOOLS, _T("SELECT [VarType], [DimVar], [DimValue]  FROM tblDimensions"), __LINE__, __FILE__, __FUNCTION__)) return;

  struct resbuf rbStrVar; rbStrVar.restype = RTSTR;
  struct resbuf rbIntVar; rbStrVar.restype = RTSHORT;
  struct resbuf rbFltVar; rbStrVar.restype = RTREAL;

  Acad::ErrorStatus es;
  LPCTSTR csDimVar, csDimVal, csVarTyp;
  for (int iRow = 0; iRow < tblDimensions.GetRows(); iRow++)
  {
    csVarTyp = tblDimensions.GetRowAt(iRow)->GetAt(0);
    csDimVar = tblDimensions.GetRowAt(iRow)->GetAt(1);
    csDimVal = tblDimensions.GetRowAt(iRow)->GetAt(2);

				 if (!_tcsicmp(csVarTyp, _T("INT")))		es = setSysVar(csDimVar, _ttoi(csDimVal));
    else if (!_tcsicmp(csVarTyp, _T("FLOAT")))  es = setSysVar(csDimVar, _tstof(csDimVal));
    else if (!_tcsicmp(csVarTyp, _T("STRING"))) es = setSysVar(csDimVar, csDimVal);
  }

  if (!acdbTblSearch(_T("DIMSTYLE"), _T("DIMENSION"), FALSE))
  {
    struct resbuf rbSetvar;
    rbSetvar.restype = RTSTR;   rbSetvar.resval.rstring = _T("eCapture_Data"); acedSetVar(_T("DIMTXSTY"), &rbSetvar);
    rbSetvar.restype = RTSHORT; rbSetvar.resval.rint    = 5;                   acedSetVar(_T("EXPERT"),   &rbSetvar);

    acedCommand(RTSTR, _T(".DIM1"), RTSTR, _T("SAVE"), RTSTR, _T("STANDARD"), NULL);
  }
}
*/


