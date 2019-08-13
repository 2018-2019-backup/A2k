#include "StdAfx.h"

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name : ApplyTextStyle
// DEscription   : Modifies the "Standard" text style based on the inputs given
////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ApplyTextStyle(ads_name enText, CString csTxtStyle, double dTextHeight)
{
	// Get the object id of given style from symbol table
	AcDbTextStyleTable *pTxtStyleTbl;
	AcDbObjectId objStdTxtId;
	if (acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pTxtStyleTbl, AcDb::kForRead) != Acad::eOk) return false;
	if (pTxtStyleTbl->getAt(csTxtStyle, objStdTxtId) != Acad::eOk) { pTxtStyleTbl->close(); return false; }
	pTxtStyleTbl->close();
		
	// The above would have changed the text selected, if it was defined with the given style. But if it was written in other styles, they wouldn't be 
	// affected. Hence we will have to modify their data.
	AcDbObjectId objId; 
	AcDbText *pText;
	AcDbMText *pMText;

	acdbGetObjectId(objId, enText);
	if (acdbOpenObject(pMText, objId, AcDb::kForWrite) == Acad::eOk)
	{
		//////////////////////////////////////////////////////////////////////////
		// MText
		//////////////////////////////////////////////////////////////////////////
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
		pText->setTextStyle(objStdTxtId);
		pText->setHeight(dTextHeight);
		pText->close();
	}

	// Return a success flag
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Function name: CreateEATextStyle
// Description  :
//////////////////////////////////////////////////////////////////////////
void CreateEATextStyle(CString csID, double &dHeight)
{
	// Get the text values from tblText
	CQueryTbl tblText;
	CString csSQL;
	csSQL.Format(L"SELECT [Style], [Font], [Height], [WidthFactor], [ObliqueAngle], [TTForSHX] FROM tblText WHERE [ID] = %d", _tstoi(csID));
	if (!tblText.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, _T(__FILE__), __FUNCTION__,true)) return;

	CString csStyle;
	for (int iRow = 0; iRow < tblText.GetRows(); iRow++)
	{
		CStringArray *pszRow = tblText.GetRowAt(iRow);

		// Create the text style
		dHeight = _tstof(pszRow->GetAt(2));
		// csStyle.Format(L"eCapture_%s", (TCHAR *)(LPCTSTR) pszRow->GetAt(0));
		csStyle.Format(L"%s", (TCHAR *)(LPCTSTR) pszRow->GetAt(0));

		// Call the common function to create the TEXT Style
		CreateTextStyle(csStyle, (TCHAR *)(LPCTSTR) pszRow->GetAt(1), 0.0, _tstof(pszRow->GetAt(3)), _tstof(pszRow->GetAt(4)), pszRow->GetAt(5));
	}
}

//////////////////////////////////////////////////////////////////////////////// 
// Function   : SSGetKwordCallBack
// Description: Callback for the SSGET 
//////////////////////////////////////////////////////////////////////////////// 
struct resbuf *SSGetKwordCallBack (const ACHAR *kword)
{
	acutPrintf (L"\nkword = %s", kword);
	if (!wcscmp (kword, L"Cancel"))  return (acutBuildList (RTSHORT, -1, RTNONE));
	return (NULL);
}

///////////////////////////////////////////////////////////////////////////////////
// Function name: Command_Text
// Description  : Called when "NT" command is invoked at the command prompt.
///////////////////////////////////////////////////////////////////////////////////
void Command_Text()
{
	switchOff();

	acutPrintf(L"\nVersion: V2.0");
	
	// Get the applicable text styles from the tables
	CQueryTbl tblText;
	CString csSQL;
	double dHeight = 0.0;

	csSQL.Format(L"SELECT [Style], [ID], [Height] FROM tblText ORDER BY [ID]");
	if (!tblText.SqlRead(DSN_DWGTOOLS, csSQL, __LINE__, __FILE__, __FUNCTION__,true)) return;
	if (tblText.GetRows() <= 0) { acutPrintf(L"Text style information in standard tables not found!"); return;	}

	CString csOptions = L"";
	CString csInitGet = L"";
	CString csOption;
	CString csPrompt;

	// Get the current text style
	struct resbuf rbGetVar; acedGetVar(L"TEXTSTYLE", &rbGetVar);

	for (int iRow = 0; iRow < tblText.GetRows(); iRow++)
	{
		// Get the height of the text for the current style
		if (!tblText.GetRowAt(iRow)->GetAt(0).CompareNoCase(rbGetVar.resval.rstring)) dHeight = _tstof(tblText.GetRowAt(iRow)->GetAt(2));

		// String formatted to retrieve keywords
		csInitGet += (csInitGet.IsEmpty() ? tblText.GetRowAt(iRow)->GetAt(1) : (L" " + tblText.GetRowAt(iRow)->GetAt(1)));

		// String formatted to display prompt for text style
		csOption.Format(L"%s = %s", tblText.GetRowAt(iRow)->GetAt(0), tblText.GetRowAt(iRow)->GetAt(1));
		csOptions += (csOptions.IsEmpty() ? csOption : (L"/" + csOption));
	}
	csPrompt.Format(L"\nEnter Style [%s]: ", csOptions);

	// Allow the user to select texts to modify
	int iRet;
	ads_name enText;
	TCHAR result[7];
	ads_point ptDummy;
	struct resbuf *rbpFilt = acutBuildList(-4, L"<OR", RTDXF0, L"TEXT", RTDXF0, L"MTEXT", -4, L"OR>", NULL);
	
	int iFirstAttempt = true;
	while (T)
	{
		// Check if there are objects already selected
		ads_name ssTxt;
		iRet = RTERROR;
		if (iFirstAttempt) { iRet = acedSSGet(_T("I"), NULL, NULL, rbpFilt, ssTxt); iFirstAttempt = false; }
		if (iRet != RTNORM)
		{
			iRet = acedSSGet(NULL, NULL, NULL, rbpFilt, ssTxt);
		}
		
		/**/ if (iRet == RTCAN) { acutRelRb(rbpFilt); return; }
		else if (iRet != RTNORM) continue;
		
		//long lLength = 0L;
		int lLength = 0L;
		acedSSLength(ssTxt, &lLength);
		if (lLength <= 0L) continue;
				
		// Display the current TEXT Style at command line
		acedInitGet(NULL, L"Style");
		acedGetVar(L"TEXTSTYLE", &rbGetVar);
		acutPrintf(L"\nCurrent settings: Style = %s", rbGetVar.resval.rstring);
		iRet = acedGetKword(L"\nENTER to continue or [Style]: ", result);
		
		/**/ if (iRet == RTCAN) { acedSSFree(ssTxt); acutRelRb(rbpFilt); return; }
		else if (iRet == RTNORM)
		{
			// Prompt the user to select TEXT style to set
			acedInitGet(RSG_NONULL, csInitGet);
			iRet = acedGetKword(csPrompt, result);
			/**/ if (iRet == RTCAN) { acedSSFree(ssTxt); acutRelRb(rbpFilt); return; }
			else 
			{
				// Create the text style opted for
				CreateEATextStyle(result, dHeight);
			}
		}

		// Apply the current text style to all the objects in the selection set
		ads_name enText;
		AcDbObjectId objTxt;
		AcDbEntity *pEntity;

		for (long lCtr = 0L; lCtr < lLength; lCtr++)
		{
			// Apply the current text style on texts
			acedSSName(ssTxt, lCtr, enText);
			ApplyTextStyle(enText, rbGetVar.resval.rstring, dHeight);
		}

		acedSSFree(ssTxt);
	}

	acutRelRb(rbpFilt);
}
