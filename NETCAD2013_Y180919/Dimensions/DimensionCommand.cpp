#include "StdAfx.h"

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
    acedCommandS(RTSTR, _T(".STYLE"), RTSTR, csStyle, RTSTR, pszFont, RTREAL, dHeight, RTREAL, dWidthFactor, RTREAL, 0.0, RTSTR, _T("N"), RTSTR, _T("N"), NULL);
  }
  else if (!_tcsicmp(pszType, _T("SHX")))
  {
    // Shape based
    acedCommandS(RTSTR, _T(".STYLE"), RTSTR, csStyle, RTSTR, pszFont, RTREAL, dHeight, RTREAL, dWidthFactor, RTREAL, 0.0, RTSTR, _T("N"), RTSTR, _T("N"), RTSTR, _T("N"), NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CreateEATextStyles
// Description  : Creates all the TEXT style definitions identified in the standard tables.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateEATextStyles()
{
	// Get the text values from tblText
	CQueryTbl tblText;
	CString csSQL;
	csSQL.Format(L"SELECT [Style], [Font], [Height], [WidthFactor], [ObliqueAngle], [TTForSHX] FROM tblText");
	if (!tblText.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, _T(__FILE__), __FUNCTION__,true)) return;
	
	CString csStyle;
	for (int iRow = 0; iRow < tblText.GetRows(); iRow++)
	{
		CStringArray *pszRow = tblText.GetRowAt(iRow);

		// Create the text style
		csStyle.Format(L"%s", (TCHAR *)(LPCTSTR) pszRow->GetAt(0));

		// Call the common function to create the TEXT Style
		CreateTextStyle(csStyle, (TCHAR *)(LPCTSTR) pszRow->GetAt(1), 0.0, _tstof(pszRow->GetAt(3)), _tstof(pszRow->GetAt(4)), pszRow->GetAt(5));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name: CreateDimensionStyle()
// Description  : For a given dimension style name, retrieves values for dimension variables specified in the standard tables.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateDimensionStyle(CString csID, CString csTxtStyle)
{
	CQueryTbl tblDimensions;
	CString csSQL;
	CString csDimStyleName;

	csSQL.Format(L"SELECT [VarType], [DimVar], [DimValue], [DimStyleName] FROM tblDimensions WHERE [ID] = %d", _tstoi(csID));
	if (!tblDimensions.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;

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
		csDimStyleName = tblDimensions.GetRowAt(iRow)->GetAt(3);

		// Skip the DIMVAR variable as the text style is already known by a variable
		if (!_tcsicmp(csVarTyp, _T("Dimtxsty"))) continue;
		
		/**/ if (!_tcsicmp(csVarTyp, _T("INT")))		es = setSysVar(csDimVar, _ttoi(csDimVal));
		else if (!_tcsicmp(csVarTyp, _T("FLOAT")))  es = setSysVar(csDimVar, _tstof(csDimVal));
		else if (!_tcsicmp(csVarTyp, _T("STRING"))) es = setSysVar(csDimVar, csDimVal);
	}

	// Set the text style and the variable to redefine the style if it already exists
	struct resbuf rbSetvar;
	//rbSetvar.restype = RTSTR; _tcscpy(rbSetvar.resval.rstring, csTxtStyle); acedSetVar(_T("DIMTXSTY"), &rbSetvar);
	rbSetvar.restype = RTSHORT; rbSetvar.resval.rint    = 5;                acedSetVar(_T("EXPERT"),   &rbSetvar);
	acedCommandS(RTSTR, L".SETVAR", RTSTR, L"TEXTSTYLE", RTSTR, csTxtStyle, NULL);
	acedCommandS(RTSTR, _T(".DIM1"), RTSTR, _T("SAVE"), RTSTR, csDimStyleName, NULL);
}

//////////////////////////////////////////////////////////////////////////
// Function name: ApplyDimstyle()
// Description  : Sets the given dimension style to a given dimension.
//////////////////////////////////////////////////////////////////////////
void ApplyDimstyle(ads_name enDim, CString csDimStyle)
{
	AcDbObjectId objDim; 
	AcDbDimension *pDim;

	// Get the STANDARD dimension style ID
	ads_name ssDim;
	AcDbDimStyleTable *pDimStyleTbl;
	if (acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pDimStyleTbl, AcDb::kForRead) != Acad::eOk) { acedSSFree(ssDim); return; }
	AcDbDimStyleTableRecord *pDimStyleTblRcd;
	if (pDimStyleTbl->getAt(csDimStyle, pDimStyleTblRcd, AcDb::kForRead) != Acad::eOk) { pDimStyleTbl->close(); return; }
	pDimStyleTbl->close();
	AcDbObjectId objDimStyleID = pDimStyleTblRcd->objectId();
	pDimStyleTblRcd->close();

	// Apply the Dimension Style to the selected entity
	acdbGetObjectId(objDim, enDim);
	if (acdbOpenObject(pDim, objDim, AcDb::kForWrite) == Acad::eOk)
	{
		pDim->setDimensionStyle(objDimStyleID);
		pDim->close();
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name: Command_Dimensions
// Description  : Called when "ND" command is issued at command prompt.
//////////////////////////////////////////////////////////////////////////
void Command_Dimensions()
{
	switchOff();
	acutPrintf(L"\nVersion: V2.0");

	// Create all the text styles defined in the tables
	CreateEATextStyles();

	// Get the applicable dimension styles from the tables
	CQueryTbl tblDimensions;
	CString csSQL;
	
	csSQL.Format(L"SELECT DISTINCT [DimStyleName], [DimValue], [ID] FROM tblDimensions WHERE DimVar = 'Dimtxsty' ORDER BY [ID]");
	if (!tblDimensions.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;
	if (tblDimensions.GetRows() <= 0) { acutPrintf(L"Dimension style information in standard tables not found!"); return;	}

	CString csDimStyleOption;
	CString csDimStyleOptions = L"";

	CString csInitGet = L"";
	CString csPrompt;
	struct resbuf rbDimStyleVar;
	struct resbuf rbTxtStyleVar;

	CString csTmpStr;
	CStringArray csaTxtStyleNames;

	for (int iRow = 0; iRow < tblDimensions.GetRows(); iRow++)
	{
		// String formatted to retrieve keywords
		csInitGet += (csInitGet.IsEmpty() ? suppressZero(iRow + 1) : (L" " + suppressZero(iRow + 1)));

		// String formatted to display Move  for text style
		csDimStyleOption.Format(L"%d = %s", iRow + 1, tblDimensions.GetRowAt(iRow)->GetAt(0));
		csDimStyleOptions += (csDimStyleOptions.IsEmpty() ? csDimStyleOption : (L"/" + csDimStyleOption));

		// Format the string as PARAMETER and VALUE, so that we can retrieve the TXT style's applicable, given the DIM STYLE
		if (!acdbTblSearch(L"STYLE", tblDimensions.GetRowAt(iRow)->GetAt(1), 0))
		{
			acutPrintf(L"\nText style \"%s\" assigned to \"%s\" not defined in the drawing. Set to STANDARD instead.", tblDimensions.GetRowAt(iRow)->GetAt(1), tblDimensions.GetRowAt(iRow)->GetAt(0));
			csTmpStr.Format(L"%d#STANDARD", iRow + 1);
		}
		else
			csTmpStr.Format(L"%d#%s", iRow + 1, tblDimensions.GetRowAt(iRow)->GetAt(1));
		csaTxtStyleNames.Add(csTmpStr);
	}
		
	csPrompt.Format(L"\nEnter Dimension style [%s]: ", csDimStyleOptions);

	// Allow the user to select texts to modify
	int iRet;
	ads_name enDim;
	TCHAR result[7];
	TCHAR szInput[7];
	ads_point ptDummy;
	CString csTxtStyle;
	int iValue;

	while (T)
	{
		// Display the current DIMENSION Style at command line
	  acedGetVar(L"DIMSTYLE", &rbDimStyleVar);
		acedGetVar(L"DIMTXSTY", &rbTxtStyleVar);
		acutPrintf(L"\nCurrent settings: Style = %s", rbDimStyleVar.resval.rstring);
		
		// Allow selection of entity
		acedInitGet(NULL, L"Style");
		iRet = acedEntSel(L"\nSelect dimension object or [Style]: ", enDim, ptDummy);
		
		/**/ if (iRet == RTCAN) return;
		else if (iRet == RTKWORD)
		{
			// Prompt the user to select DIMENSION style to set
			acedInitGet(RSG_NONULL, csInitGet);
			iRet = acedGetKword(csPrompt, result);
			/**/ if (iRet == RTCAN) return;
			else if (iRet == RTNORM) 
			{
				// Get the TXT style for the current DIM style. If the DIM style is NOT a valid one defined in the database we can skip the TEXT style option
				iValue = GetParameterValue(csaTxtStyleNames, result, csTxtStyle, 0);

				// Create the dimension style. This is automatically set as current style
				CreateDimensionStyle(result, csTxtStyle);
			}
		}
		else if (iRet == RTNORM)
		{
			// Check if the selected entity is a DIMENSION
			AcDbObjectId objTxt; acdbGetObjectId(objTxt, enDim);
			AcDbEntity *pEntity;
			if (acdbOpenObject(pEntity, objTxt, AcDb::kForRead) != Acad::eOk) continue;
			if (!pEntity->isKindOf(AcDbDimension::desc())) 
			{
				acutPrintf(L"\nInvalid object! Select DIMENSION entity type.");
				pEntity->close();
				continue;
			}
			pEntity->close();

			// Apply the style to the selected dimension
			ApplyDimstyle(enDim, rbDimStyleVar.resval.rstring);
		}
	}
}
