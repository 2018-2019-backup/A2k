////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File name        : ProjectUtils.cpp
// Created          : 15th May 2000
// Created by       : S.Jaisimha
// Description      : Project specific utilities.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "BalloonHelp.h"
#include <io.h>
#include "acedCmdNf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name : appMessage
// Arguments     : Message, as string
// Returns       : Nothing
// Called from   : Anywhere
// Description   : Displays the message by calling the MessageBox function with the information icon.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void appMessage(CString sMessage, int iIcon)
{
  if (!iIcon) iIcon = MB_ICONINFORMATION;
  ::MessageBox(::GetActiveWindow(), sMessage, _T("NET CAD message"), MB_OK | iIcon);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name : appError
// Arguments     : 1. File name, as string
//                 2. Function name, as string
//                 3. Line number, as int
//                 4. Error message, as string
//
// Returns       : Nothing
// Called from   : Anywhere
// Description   : Displays an error message by calling the MessageBox function with the appropriate icon.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void appError(CString sFileName, CString sFuncName, int iLine, CString sErrorMsg)
{
  CString sShowMessage; 
  sShowMessage.Format(_T("%s (Line %d)\n\n%s\n\n\n%s"), sFileName.Mid(sFileName.ReverseFind(_T('\\')) + 1), iLine, sFuncName, sErrorMsg);
  ::MessageBox(::GetActiveWindow(), sShowMessage, _T("NET CAD Error"), MB_OK | MB_ICONSTOP);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name : displayMessage
// Arguments     : Message, as string
// Returns       : Nothing
// Called from   : Anywhere
// Description   : Displays the message by calling the MessageBox function with the appropriate icon. This 
//                 new version accepts variable arguments, exactly like a normal printf function
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayMessage(LPTSTR fmt, ...)
{
  CString csDispl;
  
  // format and write the data we were given
  va_list args;
  va_start(args, fmt);
  csDispl.FormatV(fmt, args);
  appMessage(csDispl, MB_ICONINFORMATION);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : CheckFileAccess
// Arguments        : 1. File name to check for, as const char *
//                    2. Access mode, as integer
// Called from      : Anywhere
// Returns          : TRUE if file exists, FALSE if not
// Description      : Checks if the given file exists, displays an error message if not.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckFileAccess(CString szFilePath, int iMode)
{
  if (_taccess(szFilePath, iMode))
  {
    CString csError;
    switch (iMode)
    {
      case 00   : csError.Format(_T("File does not exist\n%s"),               szFilePath); break;
      case 02   : csError.Format(_T("No write permission for file\n%s"),      szFilePath); break;
      case 04   : csError.Format(_T("No read permission for file\n%s"),       szFilePath); break;
      case 06   : csError.Format(_T("No read/write permission for file\n%s"), szFilePath); break;
      default   : csError.Format(_T("Unknown mode for file\n%s"),             szFilePath); break;
    }
    appMessage(csError, MB_ICONSTOP);
    return FALSE;
  }
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : CheckFileAccess
// Arguments        : 1. File name to check for, as const char *
//                    2. Access mode, as integer
// Called from      : Anywhere
// Returns          : TRUE if file exists, FALSE if not
// Description      : Checks if the given file exists, displays an error message if not.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckFileAccess(CString sFileName, CString sFuncName, int iLine, CString szFilePath, int iMode)
{
  if (_taccess(szFilePath, iMode))
  {
    CString csError;
    switch (iMode)
    {
      case 00   : csError.Format(_T("File does not exist\n%s"),               szFilePath); break;
      case 02   : csError.Format(_T("No write permission for file\n%s"),      szFilePath); break;
      case 04   : csError.Format(_T("No read permission for file\n%s"),       szFilePath); break;
      case 06   : csError.Format(_T("No read/write permission for file\n%s"), szFilePath); break;
      default   : csError.Format(_T("Unknown mode for file\n%s"),             szFilePath); break;

    }
    appError(sFileName, sFuncName, iLine, csError);
    return FALSE;
  }

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name : getConfirmation
// Arguments     : Question to display, as string
// Returns       : IDYES if user answered yes, IDNO if not
// Called from   : Anywhere
// Description   : Displays a message box with the given question and two buttons, YES and NO. 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int getConfirmation(CString csQuestion)
{
  return (::MessageBox(::GetActiveWindow(), csQuestion, _T("Please confirm!"), MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : suppressZero
// Arguments        : Decimal value, as double
// Returns          : String equivalent of decimal, as string
// Called from      : Anywhere
// Description      : Converts the decimal value to string and removes all trailing zeros.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString suppressZero(double dValue)
{
	CString csReturn; csReturn.Format(_T("%.2f"), dValue);

	if (dValue = 0.0) 
	{ 
		csReturn = _T("0");
	}
	else
	{
		int iCount = (csReturn.GetLength() -1);
		
		if (csReturn.Find(_T('.')) > 0)
		{
			//.. First remove the zeroes at last
			while (true)
			{
				if (csReturn.GetAt(iCount) != _T('0')) break;
				if (iCount <= 0 ) break;
				csReturn.SetAt(iCount, _T(' '));
				iCount--;
			}

			//.. If the last one is . then also remove it
			if (iCount >= 0) 
			{
				if (csReturn.GetAt(iCount) == _T('.')) csReturn.SetAt(iCount, _T(' '));
			}
		}
	}

	csReturn.TrimLeft();
	csReturn.TrimRight();
  return csReturn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function         : isAcad2011
// Description      : Shows the standard version error message, when a command is run and the m_bAcadVerOK flag is FALSE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL isAcad2011(CString csCommand)
{
  // Check if AutoCAD version is 2013/14/15
	return TRUE;
  struct resbuf rbAcadVer;
  acedGetVar(_T("ACADVER"), &rbAcadVer);
  CString csAcadVer = rbAcadVer.resval.rstring;
  if (csAcadVer.Mid(0, 2) != "19") 
  {
    CString csMessage;
    csMessage.Format(_T("The %s function is not compatible with this version of AutoCAD."), csCommand);
    appMessage(csMessage, MB_ICONSTOP);
    return FALSE;
  }

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : addXDataToEntity
// Arguments        : 1. Entity name to which xdata has to be attached, as ads_name
//                    2. XData, as struct resbuf
// Returns          : Nothing
// Called from      : Anywhere
// Description      : Adds the given xdata to the entity.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void addXDataToEntity(ads_name enEnt, struct resbuf *rbpXData)
{
  // Register the XData name
  ads_regapp(rbpXData->resval.rstring);

  // Open the object and set the xdata
  AcDbObjectId objId;
  AcDbObject *pObject;
  acdbGetObjectId(objId, enEnt);
  if (acdbOpenObject(pObject, objId, AcDb::kForWrite) != Acad::eOk) return;
  pObject->setXData(rbpXData);
  pObject->close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : getXDataFromEntity
// Arguments        : 1. Entity name from which xdata has to be extracted, as ads_name
//                    2. Application name as string 
// Returns          : Pointer to XData as struct resbuf *
// Called from      : Anywhere
// Description      : Retrieves the xdata from an entity
////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct resbuf *getXDataFromEntity(ads_name enEnt, const TCHAR *sAppName)
{
  // Open the object and set the xdata
  AcDbObjectId objId;
  AcDbObject *pObject;
  acdbGetObjectId(objId, enEnt);
  if (acdbOpenObject(pObject, objId, AcDb::kForRead) != Acad::eOk) return (struct resbuf *)NULL;
  struct resbuf *rbpXData = pObject->xData(sAppName);
  pObject->close();

  return rbpXData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : switchOff
// Arguments        : None
// Returns          : Nothing
// Called from      : Anywhere
// Description      : Switches off a few AutoCAD setvars.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void switchOff()
{
  struct resbuf rbSetvar;

  rbSetvar.restype = RTSHORT;
  rbSetvar.resval.rint = 0;

  acedSetVar(_T("CMDECHO"),   &rbSetvar);
  acedSetVar(_T("BLIPMODE"),  &rbSetvar);
  // acedSetVar(_T("OSMODE"),    &rbSetvar);

  rbSetvar.resval.rint = 5;
  acedSetVar(_T("EXPERT"),    &rbSetvar);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : switchOn
// Arguments        : None
// Returns          : Nothing
// Called from      : Anywhere
// Description      : Switches on a few AutoCAD setvars.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void switchOn()
{
  struct resbuf rbSetvar;

  rbSetvar.restype = RTSHORT;
  rbSetvar.resval.rint = 1;

  acedSetVar(_T("CMDECHO"),   &rbSetvar);
  acedSetVar(_T("HIGHLIGHT"), &rbSetvar);
  acedSetVar(_T("BLIPMODE"),  &rbSetvar);
  acedSetVar(_T("UCSICON"),   &rbSetvar);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : getSystemDate
// Arguments        : None
// Returns          : System date in dd-mm-yyyy format, as string (Avoid Y2K, right here!!)
// Called from      : Anywhere
// Description      : Looks at the "CDATE" AutoCAD variable, strips it of its decimal stuff and converts the
//                    number to "dd-mm-yyyy" format
////////////////////////////////////////////////////////////////////////////////////////////////////////////
TCHAR *getSystemDate()
{
  // First, get the value of CDATE and convert it to a string
  // which will be in the form yyyymmdd
  TCHAR sDate[21];
  struct resbuf rbCDate;
  acedGetVar(_T("CDATE"), &rbCDate);
  _stprintf(sDate, _T("%.0f"), rbCDate.resval.rreal);
  
  // Form the dd-mm-yyyy string from the date
  TCHAR sYear[11];  _tcscpy(sYear,  sDate);      sYear[4] = _T('\0');
  TCHAR sMonth[11]; _tcscpy(sMonth, sDate + 4); sMonth[2] = _T('\0');
  TCHAR sDay[11];   _tcscpy(sDay,   sDate + 6);   
  static TCHAR sReturn[21];
  _stprintf(sReturn, _T("%s.%s.%s"), sDay, sMonth, sYear);

  // Return this
  return sReturn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : Assoc
// Arguments        : 1. the ENTGET list to be parased
//                    2. the association number
// Returns          : The stuct resbuf of the association no. asked for.
// Called from      : Anywhere
// Description      : This is an quivalent of the ASSOC function of AutoLISP.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct resbuf *Assoc(struct resbuf *rbpList, int iAssocNum)
{
	struct resbuf *rbpTemp;

	for (rbpTemp = rbpList; rbpTemp; rbpTemp = rbpTemp->rbnext)
		if (rbpTemp->restype == iAssocNum) return rbpTemp;
	    return (struct resbuf *)NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : getXRecordFromDictionary
// Arguments        : 1. Dictionary name, as string
//                    2. Xrecord name, as string
// Returns          : Xrecord attached to the dictionary, as struct resbuf *
// Called from      : Anywhere
// Description      : Retrieves the Xrecord of given name in the dictionary of the specified name.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct resbuf *getXRecordFromDictionary(const TCHAR *sDictName, const TCHAR *sXrecName)
{
  Acad::ErrorStatus eStatus;

  // Get the named objects dictionary
  AcDbDictionary *pNamedObj;
  acdbHostApplicationServices()->workingDatabase()->getNamedObjectsDictionary(pNamedObj, AcDb::kForRead);

  // see if the dictionary name specified is present
  AcDbDictionary *pDict;
  eStatus = pNamedObj->getAt(sDictName, (AcDbObject *&)pDict, AcDb::kForRead);
  pNamedObj->close();
  if (eStatus == Acad::eKeyNotFound) return (struct resbuf *) NULL;

  // See if the xrecord name specified is present
  AcDbXrecord *pXrec;
  eStatus = pDict->getAt(sXrecName, (AcDbObject *&)pXrec, AcDb::kForRead);
  pDict->close();
  if (eStatus == Acad::eKeyNotFound) return (struct resbuf *) NULL;

  // Get the xrecord value and return it
  struct resbuf *rbpXED;
  pXrec->rbChain(&rbpXED);
  pXrec->close();

  return rbpXED;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : getXRecordFromFile
// Arguments        : 1. AutoCAD drawing file name, as string
//                    2. Dictionary name, as string
//                    3. Xrecord name, as string
// Returns          : Xrecord attached to the dictionary, as struct resbuf *
// Called from      : Anywhere
// Description      : Retrieves the Xrecord of given name in the dictionary of the specified name in the
//                    drawing file specified.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct resbuf *getXRecordFromFile(const TCHAR *sFile, const TCHAR *sDict, const TCHAR *sXrec)
{
  Acad::ErrorStatus eStatus;

  AcDbDatabase *pDb = new AcDbDatabase;
  if (pDb->readDwgFile(sFile) != Acad::eOk) { delete pDb; return (struct resbuf *)NULL; }

  // Get the named objects dictionary
  AcDbDictionary *pNamedObj;
  pDb->getNamedObjectsDictionary(pNamedObj, AcDb::kForRead);

  // see if the dictionary name specified is present
  AcDbDictionary *pDict;
  eStatus = pNamedObj->getAt(sDict, (AcDbObject *&)pDict, AcDb::kForRead);
  pNamedObj->close();
  if (eStatus == Acad::eKeyNotFound)
	{
		delete pDb;
		return (struct resbuf *) NULL;
	}

  // See if the xrecord name specified is present
  AcDbXrecord *pXrec;
  eStatus = pDict->getAt(sXrec, (AcDbObject *&)pXrec, AcDb::kForRead);
  pDict->close();
  if (eStatus == Acad::eKeyNotFound)
	{
		delete pDb;
		return (struct resbuf *) NULL;
	}

  // Get the xrecord value and return it
  struct resbuf *rbpXrec;
  pXrec->rbChain(&rbpXrec);
  pXrec->close();

  delete pDb;
  return rbpXrec;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : addXRecordToDictionary
// Arguments        : 1. Dictionary name, as string
//                    2. Xrecord name, as string
//                    3. XED to be attached, as struct resbuf *
// Returns          : None
// Called from      : Anywhere
// Description      : Creates a dictionary not attached to any entity.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void addXRecordToDictionary(const TCHAR *sDictName, const TCHAR *sXrecName, struct resbuf *rbpXED)
{
  if (!rbpXED) return;

  AcDbDictionary *pNamedObj = NULL, *pDict = NULL;
  acdbHostApplicationServices()->workingDatabase()->getNamedObjectsDictionary(pNamedObj, AcDb::kForWrite);
  if (!pNamedObj) { acedAlert(_T("Unable to retrieve Named Objects Dictionary from drawing.")); return; }

  // Check if the specified dictionary name is already present in this named object
  if (pNamedObj->getAt(sDictName, (AcDbObject *&) pDict, AcDb::kForWrite) == Acad::eKeyNotFound)
  {
    pDict = ::new AcDbDictionary;
    if (!pDict) { acedAlert(_T("Unable to create new AcDbDictionary object.")); return; }
    AcDbObjectId DictId;
    pNamedObj->setAt(sDictName, pDict, DictId);
  }
  pNamedObj->close();

  // Add a new XRecord to the dictionary
  AcDbXrecord *pXrec = ::new AcDbXrecord;
  if (!pXrec) { acedAlert(_T("Unable to create new AcDbXrecord object.")); return; }
  AcDbObjectId xrecObjId;
  pDict->setAt(sXrecName, pXrec, xrecObjId);
  pDict->close();

  pXrec->setFromRbChain(*rbpXED);
  pXrec->close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : appendXRecordToDictionary
// Arguments        : 1. Dictionary name, as string
//                    2. Xrecord name, as string
//                    3. Data to be attached, as struct resbuf *
// Returns          : None
// Called from      : Anywhere
// Description      : Gets the existing xrecord. If found, append the new data to it. Otherwise, creates one
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void appendXRecordToDictionary(const TCHAR *sDictName, const TCHAR *sXrecName, struct resbuf *rbpAppend)
{
  // Get the xrecord from the dictionary
  struct resbuf *rbpExisting = getXRecordFromDictionary(sDictName, sXrecName);

  // If it exists
  if (rbpExisting)
  {
    // Move to the end of the data
    struct resbuf *rbpTemp = NULL;
    for (rbpTemp = rbpExisting; rbpTemp->rbnext; rbpTemp = rbpTemp->rbnext);

    // Append the given data to the temp pointer
    rbpTemp->rbnext = rbpAppend;

    // Add the existing list as the xrecord
    addXRecordToDictionary(sDictName, sXrecName, rbpExisting);
  }
  // Otherwise, create a new dictionary
  else addXRecordToDictionary(sDictName, sXrecName, rbpAppend);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : GetDictionaryName
// Arguments        : 1. Dictionary name, as string
// Returns          : TRUE if dictionary name is found.
// Called from      : Anywhere
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL GetDictionaryName(const TCHAR *sDictName)
{
  Acad::ErrorStatus eStatus;

  // Get the named objects dictionary
  AcDbDictionary *pNamedObj;
  acdbHostApplicationServices()->workingDatabase()->getNamedObjectsDictionary(pNamedObj, AcDb::kForRead);

  // see if the dictionary name specified is present
  AcDbDictionary *pDict;
  eStatus = pNamedObj->getAt(sDictName, (AcDbObject *&)pDict, AcDb::kForRead);
  pNamedObj->close();
  if(eStatus == Acad::eKeyNotFound) return FALSE;

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : deleteXRecordFromDictInDwg
// Arguments        : 1. Pointer to AutoCAD drawing, as AcDbDatabase
//                    2. Dictionary name, as string
//                    3. Xrecord name, as string
//
// Returns          : BOOL - false if any mistake
// Called from      : Anywhere
// Description      : deletes the given xrecord data to the specified drawing, even if it is not open.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL deleteXRecordFromDictInDwg(AcDbDatabase *pCurDwg, const TCHAR *sDictName, const TCHAR *sXrecName)
{
  Acad::ErrorStatus eStatus;
  AcDbDictionary *pNamedObj, *pDict;
  if (pCurDwg->getNamedObjectsDictionary(pNamedObj, AcDb::kForWrite) != Acad::eOk)
    return FALSE;

  eStatus = pNamedObj->getAt(sDictName, (AcDbObject *&)pDict, AcDb::kForRead);
  pNamedObj->close();
  if (eStatus == Acad::eKeyNotFound)
    return FALSE;
  
  // See if the xrecord name specified is present
  AcDbXrecord *pXrec;
  eStatus = pDict->getAt(sXrecName, (AcDbObject *&)pXrec, AcDb::kForWrite);
  pDict->close();
  if (eStatus == Acad::eKeyNotFound) 
    return FALSE;
 
  // Delete the XRecord
  pXrec->erase();
  pXrec->close();
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : addXRecordToDictInDwg
// Arguments        : 1. Pointer to AutoCAD drawing, as AcDbDatabase
//                    2. Dictionary name, as string
//                    3. Xrecord name, as string
//                    4. Xrecord to be attached, as pointer to struct resbuf
//
// Returns          : Nothing
// Called from      : Anywhere
// Description      : Adds the given xrecord data to the specified drawing, even if it is not open.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void addXRecordToDictInDwg(AcDbDatabase *pCurDwg, const TCHAR *sDictName, const TCHAR *sXrecName, struct resbuf *rbpXED)
{
  AcDbDictionary *pNamedObj, *pDict;
  pCurDwg->getNamedObjectsDictionary(pNamedObj, AcDb::kForWrite);

  // Check if the specified dictionary name is already present in this named object
  if (pNamedObj->getAt(sDictName, (AcDbObject *&) pDict, AcDb::kForWrite) == Acad::eKeyNotFound)
  {
    pDict = ::new AcDbDictionary;
    AcDbObjectId DictId;
    pNamedObj->setAt(sDictName, pDict, DictId);
  }
  pNamedObj->close();

  // Add a new XRecord to the dictionary
  AcDbXrecord *pXrec = ::new AcDbXrecord;
  AcDbObjectId xrecObjId;
  pDict->setAt(sXrecName, pXrec, xrecObjId);
  pDict->close();

  pXrec->setFromRbChain(*rbpXED);
  pXrec->close();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : DeleteXRecord
// Arguments        : Dictionary name and XRecord as char
// Called from      : DeleteXRecord()
// Returns          : Nothing
// Description      : Deletes the XRecord passed as argument in the dictionary name passed on
// Created on       : 5/12/99
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteXRecord(const TCHAR *sDictName, const TCHAR *sXrecName)
{
  Acad::ErrorStatus eStatus;

  // Get the named objects dictionary
  AcDbDictionary *pNamedObj;
  acdbHostApplicationServices()->workingDatabase()->getNamedObjectsDictionary(pNamedObj, AcDb::kForRead);

  // see if the dictionary name specified is present
  AcDbDictionary *pDict;
  eStatus = pNamedObj->getAt(sDictName, (AcDbObject *&)pDict, AcDb::kForRead);
  pNamedObj->close();
  if (eStatus == Acad::eKeyNotFound)
  {
    //ads_printf("\nCould not delete the XRecord!");
    return;
  }

  // See if the xrecord name specified is present
  AcDbXrecord *pXrec;
  eStatus = pDict->getAt(sXrecName, (AcDbObject *&)pXrec, AcDb::kForWrite);
  pDict->close();
  if (eStatus == Acad::eKeyNotFound) 
  {
    //ads_printf("\nCould not delete the XRecord!");
    return;
  }

  // Delete the XRecord
  pXrec->erase();
  pXrec->close();
  //ads_printf("\nXRecord deleted successfully\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : GetAcDbDatabasePointer
// Arguments    : Drawing name, as CString
// Returns      : Pointer to AcDbDatabase
// Called from  : Anywhere
// Description  : Retrieves the database pointer to the specified drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////
AcDbDatabase* GetAcDbDatabasePointer(CString csDwg)
{
  AcDbDatabase *pDb;
  if (csDwg.IsEmpty()) pDb = acdbHostApplicationServices()->workingDatabase();
  else 
  {
    pDb = new AcDbDatabase;
    //if (pDb->readDwgFile(csDwg, _SH_DENYNO) != Acad::eOk)  //Commented for ACAD 2018
	if (pDb->readDwgFile(csDwg, AcDbDatabase::OpenMode::kForReadAndAllShare) != Acad::eOk)
    { 
      delete pDb; 
      appError(__FILE__, _T("GetAcDbDatabasePointer"), __LINE__, _T("Unable to read ") + csDwg);      
      return NULL; 
    }
  }
  return pDb;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : DeleteDictionary
// Arguments        : 1. Dictionary name, as string
//                    2. Drawing file name, as string
//
// Called from      : Anywhere
// Returns          : Nothing
// Description      : Deletes the given dictionary in the specified file
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void DeleteDictionary(CString csDictName, CString csDwg)
{
  Acad::ErrorStatus eStatus;

  // Get the database pointer
  AcDbDatabase *pDb = GetAcDbDatabasePointer(csDwg); if (!pDb) return;

  // Get the named objects dictionary
  AcDbDictionary *pNamedObj;
  if (pDb->getNamedObjectsDictionary(pNamedObj, AcDb::kForRead) != Acad::eOk) { if (!csDwg.IsEmpty()) delete pDb; return; }

  // See if the dictionary name specified is present
  AcDbDictionary *pDict;
  eStatus = pNamedObj->getAt(csDictName, (AcDbObject *&)pDict, AcDb::kForWrite);
  pNamedObj->close();
  if (eStatus == Acad::eKeyNotFound) { if (!csDwg.IsEmpty()) delete pDb; return; }

  // Erase the dictionary and close it
  pDict->erase();
  pDict->close();

  // Delete the database if it is not the current one
  if (!csDwg.IsEmpty()) delete pDb;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Function    : setLayer
// Arguments   : const char * for layer name
// Return      : None
// Called from : Anywhere
// Comments    : Sets the layer name specified as current and freezes the rest
/////////////////////////////////////////////////////////////////////////////////////////////////////
void setLayer(const TCHAR *sLayerName, Adesk::Boolean iFreezeRest = Adesk::kTrue)
{
  AcDbObjectId objLayer;
  AcDbLayerTable *pLayerTbl;
  AcDbLayerTableIterator *pLayerIterator;
  AcDbLayerTableRecord *pLayerTblRecord = ::new AcDbLayerTableRecord;

  // Set the current space to MODEL
  acdbHostApplicationServices()->workingDatabase()->setTilemode(1);
  
  // Get the layer table pointer
  acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForWrite);

  // If the layer doesn't exist return quitely
  if (!(pLayerTbl->has(sLayerName))) { pLayerTbl->close(); return; }

  // Get the object id and the layer table record
  pLayerTbl->getAt(sLayerName, objLayer);
  pLayerTbl->getAt(sLayerName, pLayerTblRecord, AcDb::kForWrite);
  pLayerTblRecord->setIsFrozen(0);  // Thaw the layer specified
  pLayerTblRecord->close();

  // A close to the pointer to the layer table is forced to successfully execute the statement following it
  pLayerTbl->close();

  // Set the layer specified as current
  acdbHostApplicationServices()->workingDatabase()->setClayer(objLayer);

  // If the programmer has opted not to freeze the other layers, return
  if (iFreezeRest == Adesk::kFalse) return;

  // Freeze the rest of the layers
  const TCHAR *pLayerName;
  acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForWrite);
  pLayerTbl->newIterator(pLayerIterator);
  for (; !pLayerIterator->done(); pLayerIterator->step())
  {  
    pLayerIterator->getRecord(pLayerTblRecord, AcDb::kForWrite);
    pLayerTblRecord->getName(pLayerName);
    if (_tcsicmp(pLayerName, sLayerName)) pLayerTblRecord->setIsFrozen(1);
    pLayerTblRecord->close();
    //free(pLayerName);
  }

  // Delete the layer iterator and close the Layer Table pointer
  delete pLayerIterator;
  pLayerTbl->close();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Function    : createLayer
// Arguments   : char * giving the name of the new layer
//               Freeze other layers flag (defaults to true)
//
// Return      : None
// Called from : Anywhere
// Comments    : Creates a new layer given its name. Sets this layer as current and freezes the rest
/////////////////////////////////////////////////////////////////////////////////////////////////////
void createLayer(const TCHAR* sLayerName, Adesk::Boolean iFreezeRest, Adesk::Boolean doSetLayer, int iColor, CString csLType, int iLTypeWt)
{
  AcDbLayerTable *pLayerTbl;
  AcDbObjectId objLayer;

  // Get this drawing's layer table pointer
  acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForWrite);

  // Create a new layer table record
  AcDbLayerTableRecord *pLayerTblRecord = ::new AcDbLayerTableRecord;
  if (!(pLayerTbl->has(sLayerName)))
  {
    // Fill in the appropriate information. We also have to specify the color and linetype for this layer. AutoCAD assumes the rest.
    pLayerTblRecord->setName(sLayerName);
    pLayerTblRecord->setIsFrozen(0);		

    AcCmColor cmColor; cmColor.setColorIndex(iColor);
    pLayerTblRecord->setColor(cmColor);

    AcDbObjectId objLType = GetLineTypeObjectId(csLType);
    pLayerTblRecord->setLinetypeObjectId(objLType);
    if (iLTypeWt != -1) 
    {
      // If the line weight is not BYLAYER
      pLayerTblRecord->setLineWeight(AcDb::LineWeight(int(iLTypeWt * 100.0))); // Since line weights are defined as 1 for 0.01 and 200 for 2.00
    }

    // Add this record to the layer table
    pLayerTbl->add(pLayerTblRecord); 
    pLayerTbl->getAt(sLayerName, objLayer);

    // Close the record
    pLayerTblRecord->close();
  }

  // Close the pointer of the layer table
  pLayerTbl->close();

  // Set the layer as the current layer and modify the status of other layers accordingly
  if (doSetLayer == Adesk::kFalse) return;
  setLayer(sLayerName, iFreezeRest);

  /*
  AcDbLayerTable *pLayerTbl;
  AcDbObjectId objLayer;

  // Get this drawing's layer table pointer
	acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForWrite);

	// Create a new layer table record
  AcDbLayerTableRecord *pLayerTblRecord = ::new AcDbLayerTableRecord;
	if (!(pLayerTbl->has(sLayerName)))
	{
		// Fill in the appropriate information. We also have to specify the color 
		// and linetype for this layer. AutoCAD assumes the rest.
		pLayerTblRecord->setName(sLayerName);
		pLayerTblRecord->setIsFrozen(0);		
    
		AcCmColor color;
		color.setColorIndex(7);
		pLayerTblRecord->setColor(color);

		AcDbLinetypeTable *pLinetypeTbl;
		AcDbObjectId ltId;
		acdbHostApplicationServices()->workingDatabase()->getLinetypeTable(pLinetypeTbl, AcDb::kForRead);
		pLinetypeTbl->getAt(_T("CONTINUOUS"), ltId);
		pLinetypeTbl->close();
		pLayerTblRecord->setLinetypeObjectId(ltId);

		// Add this record to the layer table
		pLayerTbl->add(pLayerTblRecord); 
		pLayerTbl->getAt(sLayerName, objLayer);
  
		// Close the record
		pLayerTblRecord->close();
	}

  // Close the pointer of the layer table
  pLayerTbl->close();
  
  // Set the layer as the current layer ans modify the status of other layers accordingly
  setLayer(sLayerName, iFreezeRest);
  */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function    : purgeLayer
// Arguments   : const char * giving the name of the layer to be purged
// Return      : TRUE if successful, FALSE if not
// Called from : Anywhere
// Comments    : Checks if this layer has any hard references and purges it, if not.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL purgeLayer(const TCHAR *sLayerName)
{
  CString csError;
  AcDbObjectId objLayer;
  AcDbObjectIdArray objArray;
  AcDbLayerTable *pLayerTbl;

  // Get the layer table pointer for reading
  acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead);

  // If the layer doesn't exist return with a message not to repeat such stupidities in future
  if (!(pLayerTbl->has(sLayerName))) 
  { 
    pLayerTbl->close(); 
    csError.Format(_T("Layer %s does not exist."), sLayerName);
    appMessage(csError, MB_ICONSTOP);
    return FALSE; 
  }

  // Get the object id of this layer and add it to the object id array
  pLayerTbl->getAt(sLayerName, objLayer);
  pLayerTbl->close();
  objArray.append(objLayer);

  // Now, call the purge() function on the current database with this array
  acdbHostApplicationServices()->workingDatabase()->purge(objArray);

  // If the array does not contains the object id, there are some hard references to it
  if (!objArray.contains(objLayer))
  { 
    acutPrintf(_T("Layer '%s' is hard referenced or it is the current layer.\n"), sLayerName);
    return FALSE;
  }

  // Open the object and call its erase() function
  AcDbLayerTableRecord *pLayerTblRecord;
  if (acdbOpenObject(pLayerTblRecord, objLayer, AcDb::kForWrite) != Acad::eOk) 
  { appMessage(_T("Unable to get layer table record pointer."), MB_ICONSTOP); return FALSE; } 
  pLayerTblRecord->erase();
  pLayerTblRecord->close();

  // All's well that ends well
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : eraseAllEntities
// Arguments        : Layer name on which to erase entities, as CString (null string signifies current layer)
// Called from      : Anywhere
// Returns          : TRUE if all specified operations are successful, FALSE if not
// Description      : Depending on whether this is a selection set or a single entity, this will call the
//                    base function once or as many times as there are entities in the selection set.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void eraseAllEntities(CString csLayerName)
{
  CString csLayer;
  ads_name ssErase;
  struct resbuf rbClayer, *rbpFilter;

  // If there is no layer name
  if (csLayerName.IsEmpty())
  {
    // Determine the name of the current layer
    ads_getvar(_T("CLAYER"), &rbClayer);
    csLayer = rbClayer.resval.rstring;
  }
  else csLayer = csLayerName;
  
  // Define the filter list for selection set
  rbpFilter = acutBuildList(8, csLayer, NULL);

  // Make the selection set
  if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssErase) != RTERROR)
  {
    acedCommandS(RTSTR, _T(".ERASE"), RTPICKS, ssErase, RTSTR, _T(""), NULL);
  }

  // Free the selection set
  acedSSFree(ssErase);
}

//----------------------------------------------------------------------------------------------------------
// Function name    : ChangeAttribsLayer
// Arguments        : 1. The parent entity, as object id
//                    2. The destination layer, as string
// Returns          : Nothing
// Called from      : Anywhere
// Description      : Loops through the parent entity and for each attribute found, changes its layer to
//                    the destination layer. This function is necessiated because if a user creates a block
//                    on a layer other that 0, the attrib entities tend to remain on that layer even if the 
//                    block is inserted on a different layer. Strange bug, but it is there.
//----------------------------------------------------------------------------------------------------------
void ChangeAttribsLayer(AcDbObjectId objInsertId, const TCHAR *pDestLayer)
{
  // Get the attributes attached to this insert
  AcDbBlockReference *pInsert;
  acdbOpenObject(pInsert, objInsertId, AcDb::kForRead);
  AcDbObjectIterator *pIter = pInsert->attributeIterator();
  pInsert->close();
   
  // For all attributes in this block, change the layer to the destination layer
  AcDbAttribute *pAtt;
  AcDbObjectId objAttId;
  for (pIter->start(); !pIter->done(); pIter->step())
  {
    objAttId = pIter->objectId();
    acdbOpenObject(pAtt, objAttId, AcDb::kForWrite);
    pAtt->setLayer(pDestLayer);
    pAtt->close();
  }

  delete pIter;
}

//----------------------------------------------------------------------------------------------------------
// Function name    : RotateAttribs
// Arguments        : 1. The parent entity, as object id
//                    2. Rotation angle.
// Returns          : Nothing
// Called from      : Anywhere
// Description      : Rotates the attributes within the given block to the given angle.
//----------------------------------------------------------------------------------------------------------
void RotateAttribs(ads_name enBlock, double dRotation)
{
  AcDbObjectId objInsertId;
  if (acdbGetObjectId(objInsertId, enBlock) != Acad::eOk) return;

  // Get the attributes attached to this insert
  AcDbBlockReference *pInsert;
  acdbOpenObject(pInsert, objInsertId, AcDb::kForRead);
  AcDbObjectIterator *pIter = pInsert->attributeIterator();
  pInsert->close();
   
  // Rotate all attributes of this block back to zero.
  AcDbAttribute *pAtt;
  AcDbObjectId objAttId;

  for (pIter->start(); !pIter->done(); pIter->step())
  {
    objAttId = pIter->objectId();
    acdbOpenObject(pAtt, objAttId, AcDb::kForWrite);
    pAtt->setRotation(0.0);
    AcGePoint3d ptIns = pAtt->alignmentPoint();
    if (pAtt->horizontalMode() != AcDb::kTextLeft) pAtt->setPosition(ptIns);
    pAtt->close();
  }

  delete pIter;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : BuildAttribsList
// Arguments        : 1. Symbol name, as string
//                    2. Resultant array of prompts, as reference to CStringArray
//                    3. Resultant array of tags, as reference to CStringArray
//                    4. Resultant array of defaults, as reference to CStringArray
//                    5. Flag to specify inclusion of the designation tag, as BOOL
//
// Called from      : Anywhere
// Returns          : CStringArray which contains the attribute prompts.
// Description      : Reads the specified drawing and fills the array with attribute prompts.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int BuildAttribsList(CString csSymName, CStringArray & csPrompts, CStringArray & csTags, CStringArray & csDefault, BOOL bIncludeDesn)
{
  TCHAR sSymName[256];
  _tcscpy(sSymName, LPCTSTR(csSymName));

  // Check for the existence of the given file
  if (_taccess(sSymName, 00)) return 0;

  // Open the drawing for reading, without opening it in the AutoCAD Drawing Editor
  AcDbDatabase *pDb = new AcDbDatabase(Adesk::kFalse);
  if (pDb->readDwgFile(sSymName) != Acad::eOk) { delete pDb; return 0; }

  // Open the model space block table record
  AcDbBlockTable *pBlkTbl;
  AcDbBlockTableRecord *pBlkTblRcd;
  if (pDb->getBlockTable(pBlkTbl, AcDb::kForRead) != Acad::eOk) { delete pDb; return 0; }
  if (pBlkTbl->getAt(ACDB_MODEL_SPACE, pBlkTblRcd, AcDb::kForRead) != Acad::eOk) { delete pDb; return 0; }
  pBlkTbl->close(); 

  // Initialise a new block table record iterator
  AcDbBlockTableRecordIterator *pBlkTblRcdItr;
  pBlkTblRcd->newIterator(pBlkTblRcdItr);

  // For each entity in the database, check if it is an ATTDEF object
  int iNumAttribs = 0;
  AcDbEntity *pEnt;
  AcDbAttributeDefinition *pAttdef;
  for (pBlkTblRcdItr->start(); !pBlkTblRcdItr->done(); pBlkTblRcdItr->step())
  {
    // Get the next entity in the drawing database. The check is in case the database is corrupted
    if (pBlkTblRcdItr->getEntity(pEnt, AcDb::kForRead) != Acad::eOk) { delete pBlkTblRcdItr; delete pDb; return 0; }

    // Cast the entity pointer as an AcDbAttributeDefinition pointer. 
    // If it returns NULL, then this entity is not an ATTDEF object
    if (pAttdef = AcDbAttributeDefinition::cast(pEnt))
    {
      // Add this attribute to the array
      csPrompts.Add(pAttdef->prompt());
			csTags.Add(pAttdef->tag());
			csDefault.Add(pAttdef->textString());
      iNumAttribs++;
    }

    // Close the entity
    pEnt->close();
  }

  // Close the block table record and delete the pointers
  pBlkTblRcd->close();  
  delete pBlkTblRcdItr; 
  delete pDb;           

  return iNumAttribs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : SetupDataSource
// Arguments        : 1. DSN, as CString
//                    2. MS-Access database name, as CString (assumed to be in "Database" folder)
//
// Called from      : DefineDSNs
// Returns          : TRUE if successful, FALSE if not
// Description      : Sets up a data source name.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SetupDataSource(CString csDSN, CString csMDB)
{
  TCHAR sDSNStr[512];
  TCHAR sError[_MAX_PATH];
  TCHAR sMDBDriver[512];

  _stprintf(sDSNStr, _T("DSN=%s;UID=;PWD=;DBQ=%s;"), csDSN, csMDB);

#ifdef WIN64
  _stprintf(sMDBDriver, _T("Microsoft Access Driver (*.mdb, *.accdb)"));
#else
  _stprintf(sMDBDriver, _T("Microsoft Access Driver (*.mdb)"));
#endif
  //Commented by XML _TRANSLATOR
  ////if (SQLConfigDataSource(NULL, ODBC_ADD_DSN, sMDBDriver, sDSNStr) == FALSE)
  ////{ 
  ////  CString csError;

  ////  csError.Format(_T("Unable to setup DSN \"%s\"\nto %s"), csDSN, csMDB);
  ////  _tcscpy(sError, csError);
  ////  appError(_T(__FILE__), _T("SetupDataSource"), __LINE__, sError); 
  ////  return FALSE; 
  ////}
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : InsertBlock
// Arguments        : 1. Drawing name with full path, as string
//                    2. Insertion point, as ads_point
//                    3. Scale factor, as a double(assumes same scale factor for X,Y,Z axes).
//                    4. Rotation angle in degrees, as a double.
//                    5. Array of atributes as a CStringArray
// Called from      : Anywhere
// Returns          : TRUE/FALSE.
// Description      : Reads the specified drawing and inserts it into the current drawing.
//                    Assumes the block name as filename of the drawing without its path and extension
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL InsertBlock(const TCHAR* szDwg, ads_point ptIns, double dScale, double dRotation, CStringArray & csAttribs)
{
  // Check if the drawing file can be read.
  if (!CheckFileAccess(szDwg, 04)) return FALSE;

  // Initialise a new database.
  TCHAR sFunc[] = _T("InsertBlock");
  CString csError;
  AcDbDatabase *pDb = new AcDbDatabase(Adesk::kFalse);
  if (!pDb) { appError(__FILE__, sFunc, __LINE__, _T("Could not create AcDbDataBase.")); return FALSE; }

  // read the drawing file.
  AcDbObjectId objId;
  if (pDb->readDwgFile(szDwg) != Acad::eOk)
    { appError(__FILE__, sFunc, __LINE__, _T("Unable to read drawing file.")); return FALSE; }
  
  // Get the block name.
  TCHAR sPath[_MAX_PATH], sName[101], sExt[21];
  acedFNSplit(szDwg, sPath, sName, sExt);

  // create a block entry into the blocktable record and delete the source database.
  Acad::ErrorStatus es = acdbHostApplicationServices()->workingDatabase()->insert(objId, sName, pDb);
  delete pDb;
  if (es != Acad::eOk) 
  { 
    csError.Format(_T("Could not insert %s"), sName);
    appError(__FILE__, sFunc, __LINE__, (TCHAR *)LPCTSTR(csError));
    return FALSE;
  }

  // Get a block ref. from the objID.
  AcGePoint3d geptIns(ptIns[X], ptIns[Y], 0.0);
  AcDbBlockReference *pInsert = new AcDbBlockReference(geptIns, objId);
  if (!pInsert)
  {
    appError(__FILE__, sFunc, __LINE__, _T("Unable to get block reference."));
    return FALSE;
  }

  // set the rotation angle.
  es = pInsert->setRotation(dRotation);
  if (es != Acad::eOk)
  {
    appError(__FILE__, sFunc, __LINE__, _T("Unable to set rotation angle."));
    delete pInsert; 
    return FALSE;
  }

  // set the scale.
  AcGeScale3d geScale(dScale, dScale, dScale);
  es = pInsert->setScaleFactors(geScale);
  if (es != Acad::eOk)
  {
    appError(__FILE__, sFunc, __LINE__, _T("Unable to set scale factors."));
    delete pInsert; 
    return FALSE;
  }

  // Get the block table pointer.
  AcDbBlockTable *pBlockTable;
  es = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlockTable, AcDb::kForRead);
  if (es != Acad::eOk)
  {
    appError(__FILE__, sFunc, __LINE__, _T("Unable to get block table pointer."));
    delete pInsert; 
    return FALSE;
  }

  // Get the bock table record.
  AcDbBlockTableRecord *pBlockTableRecord;
  es = pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
  if (es != Acad::eOk)
  {
    appError(__FILE__, sFunc, __LINE__, _T("Unable to get block table record."));
    delete pInsert; 
    return FALSE;
  }

  // Append the block to the database(insert it).
  AcDbObjectId objIns;
  es = pBlockTableRecord->appendAcDbEntity(objIns, pInsert);
  if (es != Acad::eOk)
  {
    delete pInsert; pBlockTableRecord->close(); pBlockTable->close();
    appError(__FILE__, sFunc, __LINE__, _T("Unable to get append block to drawing."));
    return FALSE;
  }

  pBlockTableRecord->close();
  pBlockTable->close();
  pInsert->close();
  
  // if no attributes, return.
  if (csAttribs.GetSize() == 0) return TRUE;

  // Open the block ref.
  AcDbBlockReference *pNewIns;
  es = acdbOpenObject(pNewIns, objIns, AcDb::kForWrite);
  if (es != Acad::eOk) return FALSE;

  // Create the attribute iterator.
  int iCtr;
  AcDbObjectId objAtt;
  AcDbAttribute *pAttrib;
  AcDbObjectIterator *pIter = pNewIns->attributeIterator();

  for (iCtr = 0, pIter->start(); !pIter->done(); pIter->step(), iCtr++)
  {
    // Get the attribute.
    objAtt = pIter->objectId();
    es = pNewIns->openAttribute(pAttrib, objAtt, AcDb::kForWrite);
    if (es != Acad::eOk) { delete pNewIns; delete pIter; return FALSE; }

    // set the string.
    if (iCtr < csAttribs.GetSize()) pAttrib->setTextString(csAttribs.GetAt(iCtr));
    pAttrib->close();
  }
  delete pIter;

  pNewIns->close();

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : InsertBlock
// Arguments        : 1. Drawing name with full path, as string
//                    2. Insertion point, as ads_point
//                    3. Scale factor, as a double(assumes same scale factor for X,Y,Z axes).
//                    4. Rotation angle in degrees, as a double.
// Called from      : Anywhere
// Returns          : TRUE/FALSE.
// Description      : Reads the specified drawing and inserts it into the current drawing.
//                    Assumes the block name as filename of the drawing without its path and extension
//                    Calls the overload of this function defined above.
//                    This function is to help insertion of blocks without attibutes.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL InsertBlock(const TCHAR* szDwg, ads_point ptIns, double dScale, double dRotation)
{
  CStringArray csTemp;
  return InsertBlock(szDwg, ptIns, dScale, dRotation, csTemp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////  These functions are for drawing management  ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : getDocFromFilename
// Arguments        : 1. File name, as CString
//                    2. Resultant document pointer, as AcApDocument*
//
// Called from      : Mostly from one of the functions defined below
// Returns          : TRUE if successful, FALSE if not
// Description      : Returns the document pointer of the given drawing. Code taken from ObjectARX 2000
//                    sample "docman". (NOT EXPORTED)
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL getDocFromFilename(CString csFileName, AcApDocument* &pNewDocument)
{
  CString csFilePath;
  CString csFileOnly;

  // Instantiate a document iterator
  AcApDocumentIterator* iter = acDocManager->newAcApDocumentIterator();
  AcApDocument* pThisDocument = NULL;

  // and loop until there are no more document in the editor
  while(!iter->done()) 
  {   
    // Tiptoe through the tulips
    pThisDocument = iter->document();
    csFilePath = pThisDocument->fileName();
    csFileOnly = csFilePath.Right(csFilePath.GetLength() - csFilePath.ReverseFind(_T('\\')) - 1);

    // Compare the full file name (with path) or just the file name (without the path)
    if (!csFileName.CompareNoCase(csFilePath) || !csFileName.CompareNoCase(csFileOnly))
    {
      // Assign the pointer, delete the iterator and return
      pNewDocument = pThisDocument;
      if (iter) delete iter;
      return TRUE;
    }

    // Move to the next document in line
    iter->step();
  }

  // Well, there is no such document open
  pNewDocument = NULL;
  if (iter) delete iter;

  // No match found 
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : OpenDrawingHelper
// Arguments        : Void pointer containing the drawing name
// Called from      : OpenDrawing
// Returns          : Nothing
// Description      : Opens the drawing in the application context (NOT EXPORTED)
////////////////////////////////////////////////////////////////////////////////////////////////////////////
TCHAR sCommand[297];
void OpenDrawingHelper(void *pData)
{
  // Check if we are in the application context (we better be...)
  if (acDocManager->isApplicationContext()) 
  {
    // Open the given drawing in a synchronous fashion
    acDocManager->appContextOpenDocument((const TCHAR *)pData);

    // Get the document pointer to the opened drawing and set it as the current document
    AcApDocument *pDoc = NULL;
    getDocFromFilename((const TCHAR *)pData, pDoc);
    acDocManager->setCurDocument(pDoc, AcAp::kNone, true);
    if (_tcslen(sCommand)) acDocManager->sendStringToExecute(pDoc, sCommand, false, true, false);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : OpenDrawing
// Arguments        : Drawing file to open, as const char pointer
// Called from      : Anywhere
// Returns          : Nothing
// Description      : Activates the drawing if it is already loaded or opens in in the editor.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL OpenDrawing(const TCHAR *pDwgName, const TCHAR *pCommand)
{
  // Check if the drawing exists
  AcApDocument *pOldDoc = curDoc();
  if (_taccess(pDwgName, 00)) { displayMessage(_T("Drawing does not exist:\n%s"), pDwgName); return FALSE; }

  // Check if this drawing is already open in the editor and, if so, activate it
  AcApDocument *pDoc = NULL;
  if (getDocFromFilename(pDwgName, pDoc)) acDocManager->activateDocument(pDoc);
  else
  {
    // Open the given drawing in the application context
    _wcsset(sCommand, _T('\0'));
    if (pCommand) _tcscpy(sCommand, pCommand);
    CAcModuleResourceOverride use(acedGetAcadResourceInstance());
    acDocManager->executeInApplicationContext(OpenDrawingHelper, (void *)pDwgName);
  }

  // Set the command string back to NULL.
  _wcsset(sCommand, _T('\0'));

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name : ShowBalloon
// Arguments     : 1. Message string, as CString
//                 2. Parent window pointer, as CWnd* (defaults to NULL, i.e., the desktop window)
//                 3. Control ID in the current window, as UINT (defaults to 0)
//                 4. Message title, as CString (defaults to "Invalid parameter")
//
// Returns       : Nothing
// Called from   : Anywhere
// Description   : Displays the balloon help dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowBalloon(CString csMessage, CWnd *pParent, UINT nCtrlID, CString csTitle)
{
  // If we have a parent window and a control ID
  if (pParent && nCtrlID)
  {
    // Decide the icon
    CRect rect;
    pParent->GetDlgItem(nCtrlID)->GetWindowRect(&rect);
    CBalloonHelp::LaunchBalloon(nCtrlID, L"NET CAD message", csMessage, rect.CenterPoint(), ((csTitle == _T("Invalid!")) ? IDI_ERROR : IDI_INFORMATION), CBalloonHelp::unCLOSE_ON_KEYPRESS | CBalloonHelp::unCLOSE_ON_LBUTTON_UP, pParent, _T(""), 0);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function Name           : CheckForDuplication
// Arguments               : 1.csCheck as CString
//                           2.csCheckArray as CStiringArray.						     
// Returns                 : BOOL
// Description             : Checks whether the passed string is there in array or not.
//                           Returns TRUE if it matches and FALSE if it not matches. 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckForDuplication(CString csCheck, CStringArray & csaCheckArray)
{
  // Check the string in the array
  for (int iCtr = 0; iCtr < csaCheckArray.GetSize(); iCtr++)
  {
    if (!csaCheckArray.GetAt(iCtr).CompareNoCase(csCheck))
      return TRUE;
  }
  return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function Name           : CheckForDuplication
// Arguments               : 1.csCheck as CString
//                           2.csCheckArray as CStiringArray.						     
// Returns                 : BOOL
// Description             : Checks whether the passed string is there in array or not.
//                           Returns TRUE if it matches and FALSE if it not matches. 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckForDuplication(CString csCheck, CStringArray &csaCheckArray, int &iIndex)
{
  // Check the string in the array
  iIndex = -1;
  for (int iCtr = 0; iCtr < csaCheckArray.GetSize(); iCtr++)
  {
    if (!csaCheckArray.GetAt(iCtr).CompareNoCase(csCheck))
    {
      iIndex = iCtr;
      return TRUE;
    }
  }
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : ChangeEntityProperties
// Arguments        : 1. Object ID of the entity whose properties are to be modified, as AcDbObjectId
//                    2. New color, as ACI integer     (defaults to 0,    i.e., no color change)
//                    3. New layer, as string          (defaults to NULL, i.e., no layer change)
//                    4. New linetype, as string       (defaults to NULL, i.e., no linetype change)
//                    5. New linetype scale, as double (defaults to 1.0,  i.e., no linetype scale change)
//
// Called from      : Anywhere
// Returns          : TRUE if all specified operations are successful, FALSE if not
// Description      : Changes the specified properties of the entity.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ChangeEntityProperties(AcDbObjectId objId, int iNewColor, const TCHAR *pszNewLayer, const TCHAR *pszNewLType, double dNewLTScale)
{
  // Get a pointer to the current database
  AcDbDatabase *pCurDb = acdbHostApplicationServices()->workingDatabase();

  // Open the entity for writing
  AcDbEntity *pEnt; if (acdbOpenObject(pEnt, objId, AcDb::kForWrite) != Acad::eOk) return FALSE;

  // If the color specified is not 0, change the entity's color
  if (iNewColor > 0) pEnt->setColorIndex(iNewColor);

  // If the layer specified is not NULL, change the entity's layer
  if (pszNewLayer) 
  {
    // Open the layer table for reading
    AcDbLayerTable *pLayerTbl;
    if (pCurDb->getLayerTable(pLayerTbl, AcDb::kForRead) != Acad::eOk) { pEnt->close(); return FALSE; }

    // Check if the specified layer exists
    if (!pLayerTbl->has(pszNewLayer)) { pLayerTbl->close(); pEnt->close(); return FALSE; }

    // Get the object id of the specified layer from the layer table
    AcDbObjectId objLayer; if (pLayerTbl->getAt(pszNewLayer, objLayer) != Acad::eOk) { pLayerTbl->close(); pEnt->close(); return FALSE; }

    // Close the layer table
    pLayerTbl->close();

    // Change the entity's layer
    pEnt->setLayer(objLayer);
  }

  // If the linetype specified is not NULL, change the entity's linetype
  if (pszNewLType)
  {
    // Open the linetype table for reading
    AcDbLinetypeTable *pLTypeTbl;
    if (pCurDb->getLinetypeTable(pLTypeTbl, AcDb::kForRead) != Acad::eOk) { pEnt->close(); return FALSE; }

    // Check if the specified linetype exists
    if (!pLTypeTbl->has(pszNewLType)) 
    { 
      // Close the linetype table, otherwise we cannot use the "loadLineTypeFile" function
      pLTypeTbl->close();

      // Try to load the linetype from the "ACAD.LIN" standard file
      if (pCurDb->loadLineTypeFile(pszNewLType, _T("ACAD.LIN")) != Acad::eOk) { pEnt->close(); return FALSE; }

      // Re-open the linetype table
      if (pCurDb->getLinetypeTable(pLTypeTbl, AcDb::kForRead) != Acad::eOk) { pEnt->close(); return FALSE; }
    }

    // Get the object id of the specified linetype from the linetype table
    AcDbObjectId objLType; if (pLTypeTbl->getAt(pszNewLType, objLType) != Acad::eOk) { pLTypeTbl->close(); pEnt->close(); return FALSE; }

    // Close the linetype table
    pLTypeTbl->close();

    // Change the entity's linetype
    pEnt->setLinetype(objLType);
  }

  // If the linetype scale specified is not 1.0, change the entity's linetype scale
  if (dNewLTScale != 1.0) pEnt->setLinetypeScale(dNewLTScale);

  // Close the entity pointer
  pEnt->close();

  // Return a success flag
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : ChangeProperties
// Arguments        : 1. Flag signifying whether a selection set or an entity is specified, as BOOL
//                    2. Entity name of the selection set or entity, as ads_name
//                    3. New color, as ACI integer     (defaults to 0,    i.e., no color change)
//                    4. New layer, as string          (defaults to NULL, i.e., no layer change)
//                    5. New linetype, as string       (defaults to NULL, i.e., no linetype change)
//                    6. New linetype scale, as double (defaults to 1.0,  i.e., no linetype scale change)
//
// Called from      : Anywhere
// Returns          : TRUE if all specified operations are successful, FALSE if not
// Description      : Depending on whether this is a selection set or a single entity, this will call the
//                    base function once or as many times as there are entities in the selection set.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ChangeProperties(BOOL bIsSSet, ads_name anEnOrSS, int iNewColor,const TCHAR *pszNewLayer, const TCHAR *pszNewLType, double dNewLTScale)
{
  // If this is a single entity...
  if (bIsSSet == FALSE)
  {
    // Get the object ID of the specified entity and call the base function on it
    AcDbObjectId objId; if (acdbGetObjectId(objId, anEnOrSS) != Acad::eOk) return FALSE;
    return ChangeEntityProperties(objId, iNewColor, pszNewLayer, pszNewLType, dNewLTScale);
  }
  // If this is a selection set
  else
  {
    // Get the length of the selection set
    //long lLength; if (acedSSLength(anEnOrSS, &lLength) != RTNORM) return FALSE;
	int lLength; if (acedSSLength(anEnOrSS, &lLength) != RTNORM) return FALSE;

    // Get the name and object id of each entity in the selection set and call the base function on it
    ads_name enSSEnt;
    AcDbObjectId objId;
    for (long lCtr = 0L; lCtr < lLength; lCtr++)
    {
      if (acedSSName(anEnOrSS, lCtr, enSSEnt) != RTNORM) return FALSE;
      if (acdbGetObjectId(objId, enSSEnt) != Acad::eOk)  return FALSE;
      if (!ChangeEntityProperties(objId, iNewColor, pszNewLayer, pszNewLType, dNewLTScale)) return FALSE;
    }
  }

  // Return a success flag
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : SetVisibility
// Arguments        : 1. Flag signifying whether a selection set or an entity is specified, as BOOL
//                    2. Entity name of the selection set or entity, as ads_name
//                    3. Visibility flag, as AcDb::Visibility
//
// Called from      : Anywhere
// Returns          : TRUE if all specified operations are successful, FALSE if not
// Description      : Depending on whether this is a selection set or a single entity, this will show or
//                    hide the entity or entities, as the case may be.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SetVisibility(BOOL bIsSSet, ads_name anEnOrSS, AcDb::Visibility vShow)
{
  ads_name enSSEnt;
  AcDbEntity *pEnt;   
  AcDbObjectId objId; 

  // If this is a single entity...
  if (bIsSSet == FALSE)
  {
    // Get the object ID of the specified entity, open it and set its visibility
    if (acdbGetObjectId(objId, anEnOrSS) != Acad::eOk) return FALSE;
    if (acdbOpenObject(pEnt, objId, AcDb::kForWrite) != Acad::eOk) return FALSE;
    pEnt->setVisibility(vShow);
    pEnt->close();
  }
  // If this is a selection set
  else
  {
    // Get the length of the selection set
    //long lLength; if (acedSSLength(anEnOrSS, &lLength) != RTNORM) return FALSE;
	int lLength; if (acedSSLength(anEnOrSS, &lLength) != RTNORM) return FALSE;

    // Get the name and object id of each entity in the selection set and call the base function on it
    for (long lCtr = 0L; lCtr < lLength; lCtr++)
    {
      if (acedSSName(anEnOrSS, lCtr, enSSEnt) != RTNORM) return FALSE;
      if (acdbGetObjectId(objId, enSSEnt) != Acad::eOk)  return FALSE;
      if (acdbOpenObject(pEnt, objId, AcDb::kForWrite) != Acad::eOk) return FALSE;
      pEnt->setVisibility(vShow);
      pEnt->close();
    }
  }

  // Return a success flag
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : GetSSExtents
// Arguments        : 1. Selection set of entities whose extents have to be determined, as ads_name
//					          2. Resultant lower left extent, as ads_point
//                    3. Resultant upper right extent, as ads_point
//                    4. Resultant center point, as ads_point
//
// Called from      : Anywhere
// Returns          : TRUE if successful, FALSE if not
// Description      : Calculates the co-ordinate extents of the selection set
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL GetSSExtents(ads_name ssSet, ads_point ptLL, ads_point ptUR, ads_point ptCenter)
{
  // Reset the positions of the extent points
  ptLL[X] = ptLL[Y] = +999999.00;
  ptUR[X] = ptUR[Y] = -999999.00;
  ptLL[Z] = ptUR[Z] = 0.0;

  // Get the length of the selection set
  //long lLength; if (acedSSLength(ssSet, &lLength) != RTNORM || (lLength < 1L)) return FALSE;
  int lLength; if (acedSSLength(ssSet, &lLength) != RTNORM || (lLength < 1L)) return FALSE;

  // Calculate the geometric extents of each entity in the selection set
  ads_name enSet;
  AcDbObjectId objId;
  AcDbEntity *pEnt = NULL;
  AcDbExtents exLimits;
  AcGePoint3d geMax, geMin;
  for (long lCtr = 0L; lCtr < lLength; lCtr++)
  {
    // Get the next entity name in the selection set and get its object id
    acedSSName(ssSet, lCtr, enSet);
    if (acdbGetObjectId(objId, enSet) != Acad::eOk) return FALSE;

    // Open it for reading and get is geometric extents
    if (acdbOpenObject(pEnt, objId, AcDb::kForWrite) != Acad::eOk) return FALSE;
    if (pEnt->getGeomExtents(exLimits) != Acad::eOk) { pEnt->close(); continue; }
    pEnt->close();
    
    // Check if either the lower left limit or the upper right limit lie outside the current values
    geMin = exLimits.minPoint(); geMax = exLimits.maxPoint();
    if (geMax[X] > ptUR[X]) ptUR[X] = geMax[X]; if (geMax[Y] > ptUR[Y]) ptUR[Y] = geMax[Y];
    if (geMin[X] < ptLL[X]) ptLL[X] = geMin[X]; if (geMin[Y] < ptLL[Y]) ptLL[Y] = geMin[Y];
  }

  // Calculate the center point of the extents box
  ptCenter[X] = (ptLL[X] + ptUR[X]) / 2.0;
  ptCenter[Y] = (ptLL[Y] + ptUR[Y]) / 2.0;
  ptCenter[Z] = 0.0;

  // Everything is fine
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : displayHelp
// Arguments        : Topic ID, as CString
// Called from      : Anywhere
// Returns          : Nothing
// Description      : Displays the help file with the given topic
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayHelp(DWORD dwTopic)
{
  // Display the HTML help
  CString csCHM; csCHM.Format(_T("%s\\..\\Help\\NETCAD.chm"), g_csHome);
  //::HtmlHelp(::GetDesktopWindow(), csCHM, HH_DISPLAY_TOPIC, dwTopic);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : WildMatch
// Arguments    : 1. Pattern string, as const TCHAR *
//                2. String to be matched, as const TCHAR *
//
// Called from  : MatchLayerName
// Returns      : TRUE if the string matched the pattern, FALSE if not
// Description  : Performs a wild card match for the string and returns the status
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool WildMatch(const TCHAR *pattern, const TCHAR *str) 
{
  enum State { Exact, Any, AnyRepeat }; // Exact, ?, *

  const TCHAR *s = str;
  const TCHAR *p = pattern;
  const TCHAR *q = 0;
  int state = 0;

  bool match = true;
  while (match && *p) 
  {
    if (*p == '*') 
    {
      state = AnyRepeat;
      q = p + 1;
    } 
    else if (*p == '?') state = Any;
    else state = Exact;

    if (*s == 0) break;

    switch (state) 
    {
    case Exact      : match = *s == *p; s++; p++; break;
    case Any        : match = true; s++; p++; break;
    case AnyRepeat  : match = true; s++; if (*s == *q) p++; break;
    }
  }

  if (state == AnyRepeat) return (*s == *q);
  else if (state == Any) return (*s == *p);
  else return match && (*s == *p);
} 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : MatchLayerName
// Arguments    : 1. Layer name to be matched, as CString
//                2. Prefix list of layer names, as reference to CStringArray
//                3. First mid list of layer names, as reference to CStringArray
//                4. Second mid list of layer names, as reference to CStringArray
//                5. Suffix list of layer names, as reference to CStringArray
//
// Called from  : ChangeLayerSettings
// Returns      : Index of the matching entry, or -1
// Description  : Performs a wild card match for the layer name against the entries in the list and returns the status
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MatchLayerName(CString csLayerName, CStringArray& csaPrefix, CStringArray& csaMid1, CStringArray& csaMid2, CStringArray& csaSuffix)
{
  int iIndex = -1;
  CString csPreMatch, csMid1Match, csMid2Match, csSufMatch;

  // If the layer name is "0", return now
  if (csLayerName == _T("0")) return iIndex;

  // Loop through the array
  for (int iCtr = 0; iCtr < csaPrefix.GetSize(); iCtr++)
  {
    // Form the matching strings
    csPreMatch.Format(_T("%s*"),   csaPrefix.GetAt(iCtr));
    csMid1Match.Format(_T("*%s*"), csaMid1.GetAt(iCtr)); 
    csMid2Match.Format(_T("*%s*"), csaMid2.GetAt(iCtr)); 
    csSufMatch.Format(_T("*%s"),   csaSuffix.GetAt(iCtr));

    // Adjust for empty string
    if (csMid1Match == _T("**")) csMid1Match = _T("*");
    if (csMid2Match == _T("**")) csMid2Match = _T("*");

    // If the prefix, mid and suffix matches the given layer name
    if ((WildMatch(csPreMatch, csLayerName) == true)  && 
      (WildMatch(csMid1Match, csLayerName) == true) && 
      (WildMatch(csMid2Match, csLayerName) == true) && 
      (WildMatch(csSufMatch, csLayerName) == true)
      )
    {
      // Copy the index
      iIndex = iCtr;

      // No more processing required for "_ABAND" and "_PROP-WORK_" layers only
      if ((csSufMatch == _T("*ABAND")) || (csMid1Match == _T("*PROP-WORK*")) || (csMid2Match == _T("*PROP-WORK*"))) return iIndex;
    }
  }

  // Return the index
  return iIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : GetLineTypeObjectId
// Arguments    : Linetype for which object id is required, as CString
// Called from  : ChangeLayerSettings
// Returns      : Object id of the given linetype, as AcDbObjectId
// Description  : Loads the linetype from acad.lin and returns its object id.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AcDbObjectId GetLineTypeObjectId(CString csLineType)
{
  // Force load it from "acad.lin" (not for "Continuous")
  if (csLineType != _T("Continuous")) acdbLoadLineTypeFile(csLineType, _T("acad.lin"), acdbHostApplicationServices()->workingDatabase());

  // Get its object id
  AcDbLinetypeTable *pLTypeTable;
  acdbHostApplicationServices()->workingDatabase()->getLinetypeTable(pLTypeTable, AcDb::kForRead);
  AcDbObjectId objLType; pLTypeTable->getAt(csLineType, objLType);
  pLTypeTable->close();

  // Return the object id
  return objLType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : UpdateBlockFromStandards
// Description  : Checks if the block is available and if not, clones it from the standard drawing into the current drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int UpdateBlockFromStandards(CString csSymbol)
{
  Acad::ErrorStatus es;

  // Get a pointer to the current drawing and its block table's object ID
  AcDbBlockTable *pCurBT;
  AcDbDatabase *pCurDb = acdbHostApplicationServices()->workingDatabase();
  if ((es = pCurDb->getSymbolTable(pCurBT, AcDb::kForRead)) != Acad::eOk) { acutPrintf(_T("\nError %s getting block table from current drawing.\n"), acadErrorStatusText(es)); return 0; }
  AcDbObjectId curBTObjectId = pCurBT->objectId();
  pCurBT->close();

  // Open the standards drawing for reading and its block table
  AcDbBlockTable *pStdBT;
  AcDbDatabase *pStdDb = new AcDbDatabase(false);
  if (!pStdDb) { acutPrintf(_T("\nError allocating memory to read standards drawing.\n")); return 0; }

	acutPrintf(L"\nGetting %s for %s", csSymbol, g_csLocalDWT);
	//Commented for ACAD 2018
  //if ((es = pStdDb->readDwgFile(g_csLocalDWT, _SH_DENYWR))  != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return 0; }
	if ((es = pStdDb->readDwgFile(g_csLocalDWT, AcDbDatabase::OpenMode::kForReadAndReadShare)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return 0; }
  if ((es = pStdDb->getSymbolTable(pStdBT, AcDb::kForRead)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s getting block table from standards drawing.\n"), acadErrorStatusText(es)); return 0; }

  // Get the required block from the standards drawing
  AcDbObjectId objStdBlock;
  if ((es = pStdBT->getAt(csSymbol, objStdBlock)) != Acad::eOk) { acutPrintf(_T(" not found.\n")); pStdBT->close(); delete pStdDb; return 0; }
  pStdBT->close();

  // Create an object ID array
  AcDbObjectIdArray objectIds;
  objectIds.append(objStdBlock);

  // Clone the block from the standard drawing into the current drawing and replace the current definition
  AcDbIdMapping idMap;
  if (((es = pCurDb->wblockCloneObjects(objectIds, curBTObjectId, idMap, AcDb::kDrcReplace)) != Acad::eOk) && (es != Acad::ePermanentlyErased))
  { 
    delete pStdDb; 

    // Adjustment for a strange error
    if (es == eOutOfRange) return 1; else { acutPrintf(_T(" %s.\n"), acadErrorStatusText(es)); return 0; } // In any other case
  }

  // Delete the standards drawing pointer
  delete pStdDb;

  // Status
  // acutPrintf(_T(" done.\n"));

  // Success
  return 1;
}

//////////////////////// Added for DwgTools compatibility /////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : appendEntityToDatabase
// Description  : Adds an entity to the current drawing's database
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AcDbObjectId appendEntityToDatabase(AcDbEntity *pEntity, BOOL bIsMSpace)
{
  // Get the current drawing's block table
  Acad::ErrorStatus es;
  AcDbObjectId objId;
  AcDbBlockTable *pBlockTable;
  es = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlockTable, AcDb::kForRead);
  if (es != Acad::eOk) { acutPrintf(_T("\n%s opening block table for reading."), acadErrorStatusText(es)); return objId; }

  // Get the model space block table record from the block table
  AcDbBlockTableRecord *pSpaceRecord;
  es = pBlockTable->getAt((bIsMSpace ? ACDB_MODEL_SPACE : ACDB_PAPER_SPACE), pSpaceRecord, AcDb::kForWrite);
  if (es != Acad::eOk) { acutPrintf(_T("\n%s retrieving model space for writing."), acadErrorStatusText(es)); pBlockTable->close(); return objId; }

  // Close the block table
  pBlockTable->close();

  // Append the equipment to the model space block table record
  es = pSpaceRecord->appendAcDbEntity(pEntity);
  objId = pEntity->objectId();
  if (es != Acad::eOk) { acutPrintf(_T("\n%s appending entity to model space."), acadErrorStatusText(es)); pSpaceRecord->close(); return objId; }  

  // Close the model space block table record
  pSpaceRecord->close();

  // Success
  return objId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function    : saveOSMode
// Arguments   : None
// Returns	   : void
// Description : 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void saveOSMode()
{
  // Get the OSMODE and add it to XREC	
  struct resbuf rbOSMode; acedGetVar(_T("OSMODE"), &rbOSMode);
  struct resbuf *rbpOSMode = acutBuildList(AcDb::kDxfInt16, rbOSMode.resval.rint, NULL);
  addXRecordToDictionary(_T("eCapture"), _T("EA_OSMODE"), rbpOSMode);
  acutRelRb(rbpOSMode);

  // Switch off OSMODE
  switchOff();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function    : restoreOSMode
// Arguments   : None
// Returns	   : void
// Description : 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void restoreOSMode()
{
  // Set the OSMODE saved in the XREC
  struct resbuf *rbpOSMode = getXRecordFromDictionary(_T("eCapture"), _T("EA_OSMODE"));
  if (rbpOSMode) 
  {
    struct resbuf rbSetvar; rbSetvar.restype = RTSHORT; 
    rbSetvar.resval.rint = rbpOSMode->resval.rint; 
    acedSetVar(_T("OSMODE"), &rbSetvar);

    acutRelRb(rbpOSMode);
  }
}

void saveUCSMode()
{
  struct resbuf rbUCSFollow; acedGetVar(_T("UCSFOLLOW"), &rbUCSFollow); 
  struct resbuf *rbpUCSFollow = acutBuildList(AcDb::kDxfInt16, rbUCSFollow.resval.rint, NULL);
  addXRecordToDictionary(_T("eCapture"), _T("EA_UCSFOLLOW"), rbpUCSFollow);
  acutRelRb(rbpUCSFollow);

  rbUCSFollow.resval.rint = 1; 
  acedSetVar(_T("UCSFOLLOW"), &rbUCSFollow);
}

void restoreUCSMode()
{
  struct resbuf *rbpUCSFollow = getXRecordFromDictionary(_T("eCapture"), _T("EA_UCSFOLLOW"));
  if (rbpUCSFollow) 
  {
    struct resbuf rbUCSFollow; rbUCSFollow.restype = RTSHORT; 
    rbUCSFollow.resval.rint = rbpUCSFollow->resval.rint; acedSetVar(_T("UCSFOLLOW"), &rbUCSFollow);

    acutRelRb(rbpUCSFollow);
  }
}

void appErrorTxt(CString csFile, int iLine, CString csMsg) 
{ 
  acutPrintf(_T("Error @%s-%d: %s"), csFile, iLine, csMsg); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ensureSymbolAvailability
// Description  : Checks if the block is available and if not, clones it from the standard drawing into the current drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ensureSymbolAvailability(CString csSymbol, AcDbObjectId& objBlock)
{
  Acad::ErrorStatus es;

  // Get a pointer to the current drawing
  AcDbDatabase *pCurDb = acdbHostApplicationServices()->workingDatabase();

  // If this symbol is not already present
  if (acdbTblSearch(_T("BLOCK"), csSymbol, 0) == NULL)
  {
    // Check if the standard drawing file is available
    if (g_csDWGLocation.IsEmpty()) return FALSE;

    // Set the "PROXYNOTICE" system variable to 0
    CString csProxyNotice = _T("PROXYNOTICE");
    struct resbuf rbProxyNotice;
    acedGetVar(csProxyNotice, &rbProxyNotice);
    int iOldProxyNotice = rbProxyNotice.resval.rint;
    rbProxyNotice.resval.rint = 0;
    acedSetVar(csProxyNotice, &rbProxyNotice);

    // Open the standards drawing for reading
    AcDbDatabase *pStdDb = new AcDbDatabase;
	//Commented for ACAD 2018
    //if ((es = pStdDb->readDwgFile(g_csDWGLocation, _SH_DENYNO)) != Acad::eOk) { delete pStdDb ; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }
	if ((es = pStdDb->readDwgFile(g_csDWGLocation, AcDbDatabase::OpenMode::kForReadAndAllShare)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }

    // Get the Block Table from standards drawing
    AcDbBlockTable *pStdBT;
    if ((es = pStdDb->getSymbolTable(pStdBT, AcDb::kForRead)) != Acad::eOk) { delete pStdDb ; acutPrintf(_T("\nError %s getting block table from standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }

    // Get the Block Table from the current drawing
    AcDbBlockTable *pCurBT;
    if ((es = pCurDb->getSymbolTable(pCurBT, AcDb::kForRead)) != Acad::eOk) { delete pStdDb ; acutPrintf(_T("\nError %s getting block table from current drawing.\n"), acadErrorStatusText(es)); return FALSE; }

    // Get the object ID of the current block table
    AcDbObjectId curBTObjectId = pCurBT->objectId();
    pCurBT->close();

    // Get the required block from the standards drawing
    AcDbObjectId objStdBlock;
    if ((es = pStdBT->getAt(csSymbol, objStdBlock)) != Acad::eOk) { pStdBT->close(); delete pStdDb; acutPrintf(_T("\nBlock '%s' not found in standards drawing.\n"), csSymbol); return FALSE; }
    pStdBT->close();

    // Create an object ID array
    AcDbObjectIdArray objectIds;
    objectIds.append(objStdBlock);

    // Clone the block from the standard drawing into the current drawing
    AcDbIdMapping idMap;
    // if ((es = pCurDb->wblockCloneObjects(objectIds, curBTObjectId, idMap, AcDb::kDrcIgnore)) != Acad::eOk) { delete pStdDb ; acutPrintf(_T("\nError %s inserting block into current drawing.\n"), acadErrorStatusText(es)); return FALSE; }
    es = pCurDb->wblockCloneObjects(objectIds, curBTObjectId, idMap, AcDb::kDrcReplace);
    if ((es != Acad::ePermanentlyErased) && (es != Acad::eOk)) 
		{
			delete pStdDb ; acutPrintf(_T("\nError %s inserting block into current drawing.\n"), acadErrorStatusText(es)); return FALSE; 
		}
		
    // Delete the standards drawing pointer
    delete pStdDb;

    // Reset the "PROXYNOTICE" system variable
    rbProxyNotice.resval.rint = iOldProxyNotice;
    acedSetVar(csProxyNotice, &rbProxyNotice);
  }
	
  // Get the Block Table from the current drawing
  AcDbBlockTable *pCurBT;
  if ((es = pCurDb->getSymbolTable(pCurBT, AcDb::kForRead)) != Acad::eOk) { acutPrintf(_T("\nError %s getting block table from current drawing.\n"), acadErrorStatusText(es)); return FALSE; }

  // Get the object id of the required block
  // if  ((es = pCurBT->getAt(csSymbol, objBlock)) != Acad::eOk) { pCurBT->close(); acutPrintf(_T("\nError %s getting block from current drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pCurBT->getAt(csSymbol, objBlock)) != Acad::eOk) { pCurBT->close(); acutPrintf(_T("\nError %s getting block %s from current drawing.\n"), acadErrorStatusText(es), csSymbol); return FALSE; }
  pCurBT->close();

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
// Function name:
// Description  : Gets the layer name assigned to "SymbolLayer" tag of block.
////////////////////////////////////////////////////////////////////////////////////
BOOL GetSymbolLayer(CString csBlockName, CString &csLayerName)
{
  csLayerName.Empty();

  // Ensure that the symbol is available in the current drawing
  AcDbObjectId objBlock;
  if (ensureSymbolAvailability(csBlockName, objBlock) == FALSE) return FALSE;

  // Open the block definition for read
  AcDbBlockTableRecord *pBlockDef; acdbOpenObject(pBlockDef, objBlock, AcDb::kForRead);

  // Create a new record iterator from this block definition
  AcDbBlockTableRecordIterator *pIterator; pBlockDef->newIterator(pIterator);

  AcDbEntity *pEnt;
  AcDbAttributeDefinition *pAttdef;

  // For each entity in the block
  for (pIterator->start(); !pIterator->done(); pIterator->step()) 
  {
    // Get the next entity
    pIterator->getEntity(pEnt, AcDb::kForRead);

    // Make sure the entity is an attribute definition
    pAttdef = AcDbAttributeDefinition::cast(pEnt);
    if (pAttdef != NULL) 
    {
      if (!_tcsicmp((TCHAR *)pAttdef->tag(), _T("SymbolLayer")))
      {
        csLayerName = pAttdef->textString();

        pAttdef->close();
        delete pIterator;
        return TRUE;
      }
    }
  }

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : insertBlock
// Description  : Inserts the block and appends all attributes with null values
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL insertBlock(CString csBlockName, CString csLayerName, ads_point ptIns, double dScaleX, double dScaleY, double dHeight, double dRotation, CString csTextStyle, AcDbObjectId& objInsert, BOOL bIsMSpace)
{
  // Ensure that the symbol is available in the current drawing
  AcDbObjectId objBlock;
  if (ensureSymbolAvailability(csBlockName, objBlock) == FALSE) return FALSE;
	
  // Get the layer name for the symbol
  if (csLayerName.IsEmpty()) 
  {
    // The layer name is assigned to "SymbolLayer" tag in the block definition
    if (GetSymbolLayer(csBlockName, csLayerName)) 
    {
      // Create the layer to place the symbol
      createLayer(csLayerName, Adesk::kFalse, Adesk::kFalse);
    }
    else
    {
      // Else let it go to the "0" layer
      acutPrintf(_T("\nCould not find \"SymbolLayer\" attribute tag in the block. Default LAYER 0 will be assumed to place this symbol."));
      csLayerName = "0"; 
    }
  }
  else
  {
    // Create the layer to place the symbol
    createLayer(csLayerName, Adesk::kFalse, Adesk::kFalse);
  }

  // Get the scale for the symbol
  if (dScaleX <= 0)
  {
    struct resbuf *rbpScale = getXRecordFromDictionary(_T("eCapture"), _T("DXF Scale"));
    if (rbpScale) 
    {
      dScaleX = rbpScale->resval.rreal;
      dScaleY = rbpScale->resval.rreal;
    }
  }

  // Convert the insertion base point
  AcGePoint3d geBasePoint(ptIns[X], ptIns[Y], 0.0);

  // Create a new block reference object
  AcDbBlockReference *pInsert = new AcDbBlockReference;
  pInsert->setBlockTableRecord(objBlock);
  pInsert->setPosition(geBasePoint);
  pInsert->setNormal(AcGeVector3d(0, 0, 1));
  pInsert->setRotation(dRotation);
  pInsert->setScaleFactors(AcGeScale3d(dScaleX, dScaleY, dScaleX));
  pInsert->setLayer(csLayerName);

  // Add this entity to the current drawing
  // NOTE: This will create the block reference without any attributes in it
  objInsert = appendEntityToDatabase(pInsert, bIsMSpace);

  // If the INSERT is on layer "XT_BASE_SHEET_LEGEND_EXIST" attributes are not required
  if (!csLayerName.CompareNoCase(L"XT_BASE_SHEET_LEGEND_EXIST")) { pInsert->close(); return TRUE;	}

  // Open the block definition for read
  AcDbBlockTableRecord *pBlockDef; acdbOpenObject(pBlockDef, objBlock, AcDb::kForRead);

  // Create a new record iterator from this block definition
  AcDbBlockTableRecordIterator *pIterator; pBlockDef->newIterator(pIterator);

  AcDbEntity *pEnt;
  AcDbAttributeDefinition *pAttdef;

  // For each entity in the block
  CString csLayerForInsert = _T("");
  for (pIterator->start(); !pIterator->done(); pIterator->step()) 
  {
    // Get the next entity
    pIterator->getEntity(pEnt, AcDb::kForRead);

    // Make sure the entity is an attribute definition and not a constant
    pAttdef = AcDbAttributeDefinition::cast(pEnt);
    if (pAttdef != NULL && !pAttdef->isConstant()) 
    {
      // We have a non-constant attribute definition, so create an attribute entity
      AcDbAttribute *pAtt = new AcDbAttribute;
      pAtt->setPropertiesFrom(pAttdef);
      pAtt->setInvisible(pAttdef->isInvisible());

      // Translate attribute by block reference
      geBasePoint = pAttdef->position();
      geBasePoint += pInsert->position().asVector();
      pAtt->setPosition(geBasePoint);

      if (dHeight) pAtt->setHeight(dHeight); else pAtt->setHeight(pAttdef->height());

      pAtt->setRotation(pAttdef->rotation());
      pAtt->setTag(pAttdef->tag());

      pAtt->setFieldLength(pAttdef->fieldLength());
      pAtt->setHorizontalMode(pAttdef->horizontalMode());
      pAtt->setVerticalMode(pAttdef->verticalMode());
      pAtt->setAlignmentPoint(pAttdef->alignmentPoint() + pInsert->position().asVector());

      // Text style
      if (!csTextStyle.IsEmpty())
      {
        AcDbObjectId objStyle;
        AcDbTextStyleTable *pTxtStyleTbl;
        acdbHostApplicationServices()->workingDatabase()->getTextStyleTable(pTxtStyleTbl, AcDb::kForRead);
        if (pTxtStyleTbl->getAt(csTextStyle, objStyle) == Acad::eOk) pAtt->setTextStyle(objStyle);
        pTxtStyleTbl->close();
      }
      else pAtt->setTextStyle(pAttdef->textStyle());

      // By default, the attribute value will be empty
      pAtt->setTextString(pAttdef->textString());

      // Get the layer to place the block
      pAtt->setLayer(csLayerName);

      // Append the attribute to the block reference
      pInsert->appendAttribute(pAtt);

      // Close the attribute 
      pAtt->close();
    }     

    // Close the entity
    pEnt->close();
  }

  // Delete the iterator
  delete pIterator;

  // Close the block definition
  pBlockDef->close();

  // Close the block reference
  pInsert->close();

  // Success
  return TRUE;
} // objBlock

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : CheckForDuplication
// Arguments        : 1. Array to check values in, as reference to CStringArray
//                    2. Value to check for in array, as CString
//
// Returns          : TRUE if found, FALSE if not
// Called from      : Anywhere
// Description      : Checks for presence of string in the array
////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckForDuplication(CStringArray& csaArray, CString csCheck, int &iIndex)
{
  for (int iCtr = 0; iCtr < csaArray.GetSize(); iCtr++)
  {
    if (!csaArray.GetAt(iCtr).CompareNoCase(csCheck))
    {
      iIndex = iCtr;
      return TRUE;
    }
  }

  return FALSE;
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
  int iUnused = 0;
  return CheckForDuplication(csaArray, csCheck, iUnused);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name    : GetActualWorkstationExtents
// Description      : Retrieves the workstation extents considering only the FRAME objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetPoleExtents(AcDbObjectId objPole, AcDbExtents &exBounds)
{
  // Open the pole for reading
  AcDbBlockReference *pPole; if (acdbOpenObject(pPole, objPole, AcDb::kForRead) != Acad::eOk) return false;

  // Get the block definition pointer
  AcDbObjectId objId = pPole->blockTableRecord();

  // Get the insertion point of the Workstation
  AcGePoint3d geIns = pPole->position();

  // Explode the workstation to get the entities in it
  AcDbVoidPtrArray objArray;
  pPole->explode(objArray);

  // Close the workstation
  pPole->close();

  // For each entity in the array, get the geomExtents if it is not a CIRCLE
  AcDbEntity *pEntity = NULL;
  AcDbExtents exItem;
  AcGePoint3d geMin, geMax;
  CString csItemExtents, csMinX, csMinY, csMinZ, csMaxX, csMaxY, csMaxZ;
  double dMinX = 100000000.0, dMinY = 100000000.0, dMinZ = 100000000.0, dMaxX = -100000000.0, dMaxY = -100000000.0, dMaxZ = -100000000.0;
  AcDbObjectId objID;

  for (int iCtr = 0; iCtr < objArray.length(); iCtr++)
  {
    pEntity = (AcDbEntity *)objArray.at(iCtr);

    if (pEntity == NULL) continue;
    if (AcDbText::cast(pEntity))      { pEntity->close(); continue; }
    else if (AcDbAttribute::cast(pEntity)) { pEntity->close(); continue; }

    pEntity->getGeomExtents(exItem);
    pEntity->close();

    // Store the formatted extents in the array
    geMin = exItem.minPoint();
    geMax = exItem.maxPoint();

    // Update the minimum extents, if required
    if (geMin.x < dMinX) dMinX = geMin.x;
    if (geMin.y < dMinY) dMinY = geMin.y;
    if (geMin.z < dMinZ) dMinZ = geMin.z;

    // Update the maximum extents, if required
    if (geMax.x > dMaxX) dMaxX = geMax.x;
    if (geMax.y > dMaxY) dMaxY = geMax.y;
    if (geMax.z > dMaxZ) dMaxZ = geMax.z;
  }

  // Set the minimum and maximum extents
  geMin.x = dMinX; geMin.y = dMinY; geMin.z = dMinZ;
  geMax.x = dMaxX; geMax.y = dMaxY; geMax.z = dMaxZ;

  // Set the extents
  exBounds.set(geMin, geMax);

  // Success
  return true;
}

//////////////////////////////////////////////////////////////////////////
// Function name:
// Description  :
//////////////////////////////////////////////////////////////////////////
void deleteArray(AcDbVoidPtrArray entities)
{
  AcDbEntity* pEnt = NULL;
  int nEnts;
  nEnts = entities.length();

  for (int i = 0; i < nEnts; i++)
  {
    pEnt = (AcDbEntity*)(entities[i]);
    delete pEnt;
  }
}

//////////////////////////////////////////////////////////////////////////
// Function name: 
// Description  :
//////////////////////////////////////////////////////////////////////////
void deleteArray(AcDbObjectIdArray objectIds)
{
  // Remove the appended entities, so that the new set of entities on the flip side can be created
  Acad::ErrorStatus es;
  for (int iEnt = 0; iEnt < objectIds.length(); iEnt++)
  {
    AcDbEntity *pDelEntity = NULL;
    es = acdbOpenObject(pDelEntity, objectIds[iEnt], AcDb::kForWrite);
    if (es != Acad::eOk) 
    {
      acutPrintf(_T("\nERROR at %d: %s"), __LINE__, acadErrorStatusText(es)); // pDelEntity->close(); 
    }
    else
    {
      pDelEntity->erase(Adesk::kTrue); 
      pDelEntity->close();
    }
  }

  objectIds.removeAll();
}

//..//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//.. Function Name :  GetParameterValue
//.. Arugument     :  1. csaParameter as CStringArray
//..                  2. csParameter as CString
//..                  3. csValue as CString
//.. Called From   :  Anywhere 
//.. Return Type   :  int - Pos Of the parameter in the array
//.. Description   :  This function will give value for the parameter passed
//..//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetParameterValue(CStringArray &csaParameter, CString csParameter, CString &csValue, int iSearch)
{
  //.. Initialize the value to null
  csValue.Empty();

  //.. Add '#' at the end of the string.
  csParameter = csParameter + _T("#");

  //.. Check for the value in each single element of the array.
  for (int iCounter = ((iSearch < 0) ? 0 : iSearch); iCounter < csaParameter.GetSize(); iCounter++)
  {
    if (csaParameter.GetAt(iCounter).Find(csParameter) == 0)
    {
      csValue = csaParameter.GetAt(iCounter).Mid(csParameter.GetLength())  ;
      return iCounter;
    }
  }
  //.. Everything is OK
  return -1;
}

/////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : GetStdLineTypeName
// Arguments    : Object id of line type, as AcDbObjectId
// Called from  : ValidateLayerSettings
// Description  : Returns the name of the linetype as present in the standards drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString GetStdLineTypeName(AcDbDatabase *pStdDb, AcDbObjectId objStdLType)
{
	CString csStdLTName;

	// Get the linetype table from the standard drawing
	AcDbLinetypeTable *pStdLTTable; if (pStdDb->getLinetypeTable(pStdLTTable, AcDb::kForRead) != Acad::eOk) return csStdLTName;
	AcDbLinetypeTableIterator *pStdIter; pStdLTTable->newIterator(pStdIter);
	pStdLTTable->close();

	// Loop through the iterator
	ACHAR *pStdLTName;
	AcDbLinetypeTableRecord *pStdLTRec;
	Acad::ErrorStatus es;
	for ( ; !pStdIter->done(); pStdIter->step())
	{
		if (pStdIter->getRecord(pStdLTRec, AcDb::kForRead) != Acad::eOk) continue;
		if (pStdLTRec->objectId() == objStdLType)
		{
			pStdLTRec->getName(pStdLTName); csStdLTName = pStdLTName; 
			pStdLTRec->close();

			///////////////////////////////////////////////////////////////////////////////////////////////////
			// Clone this linetype and add it to the current drawing (If it doesn't exist in this drawing)
			///////////////////////////////////////////////////////////////////////////////////////////////////
			// Get the Block Table from the current drawing
			AcDbLinetypeTable *pCurLT;
			if (acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pCurLT, AcDb::kForRead) == Acad::eOk) 
			{
				if (!pCurLT->has(csStdLTName))
				{
					// Get the object ID of the current block table
					AcDbObjectId curLTObjectId = pCurLT->objectId();
					pCurLT->close();

					// Create an object ID array
					AcDbObjectIdArray objectIds;
					objectIds.append(objStdLType);

					// Clone the linetype from the standard drawing into the current drawing
					AcDbIdMapping idMap;
					es = acdbHostApplicationServices()->workingDatabase()->wblockCloneObjects(objectIds, curLTObjectId, idMap, AcDb::kDrcReplace);
					if ((es != Acad::ePermanentlyErased) && (es != Acad::eOk))
					{
						// Delete the iterator
						delete pStdIter;
						acutPrintf(_T("\nError %d: %s importing linetype into current drawing.\n"), __LINE__, acadErrorStatusText(es)); 
						return L"";
					}
				}
				else pCurLT->close();
			}
		} else pStdLTRec->close();

	}

	// Delete the iterator
	delete pStdIter;

	// Return the value
	return csStdLTName;
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
/////////////////////////////////////////////////////
bool UpdateLayerFromStandards(CString csLayerName)
{
	// Check if the given layer exists in the current drawing. If NO create it before mapping the properties from the standard DWT
	if (!acdbTblSearch(L"LAYER", csLayerName, false)) { createLayer(csLayerName, Adesk::kFalse); }

	// Open the standards drawing
	Acad::ErrorStatus es;
	AcDbDatabase *pStdDb = new AcDbDatabase;
	//Commented for ACAD 2018
	//if ((es = pStdDb->readDwgFile(g_csLocalDWT, _SH_DENYWR))  != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }
	if ((es = pStdDb->readDwgFile(g_csLocalDWT, AcDbDatabase::OpenMode::kForReadAndReadShare)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }

	// Get the layer table iterator from the standards drawing
	AcDbLayerTable *pStdLayerTbl; if ((es = pStdDb->getLayerTable(pStdLayerTbl, AcDb::kForRead)) != Acad::eOk) { delete pStdDb; return false; }
	AcDbLayerTableRecord *pStdLayerTblRecord;
	if (pStdLayerTbl->getAt(csLayerName, pStdLayerTblRecord, AcDb::kForRead) != Acad::eOk) return false;
	pStdLayerTbl->close();

	// Get the layer properties in the standards
	AcCmColor color  = pStdLayerTblRecord->color();
	AcDb::LineWeight lwStdWeight = pStdLayerTblRecord->lineWeight();
	CString csStdLType = GetStdLineTypeName(pStdDb, pStdLayerTblRecord->linetypeObjectId());
	pStdLayerTblRecord->close();
	delete pStdDb;

	// Check if the linetype is defined in this drawing. Else import the definition from DWT
	AcDbLinetypeTable *pLTypeTbl;
	if (acdbHostApplicationServices()->workingDatabase()->getLinetypeTable(pLTypeTbl, AcDb::kForRead) != Acad::eOk) return false;
	AcDbLinetypeTableRecord *pLTypeTblRcd;
	es = pLTypeTbl->getAt(csStdLType, pLTypeTblRcd, AcDb::kForRead);
	if (es != Acad::eOk) { acutPrintf(L"\nError %d: %s", __LINE__, acadErrorStatusText(es)); return false; }
	pLTypeTbl->close();

	AcDbObjectId objLTypeId = pLTypeTblRcd->objectId();
	pLTypeTblRcd->close();

	// Get the layer pointer in the current drawing and assign the properties retrieved from the standard to this
	AcDbLayerTable *pLayerTbl; if ((es = acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForWrite)) != Acad::eOk) { return false; }
	AcDbLayerTableRecord *pLayerTblRecord;
	if (pLayerTbl->getAt(csLayerName, pLayerTblRecord, AcDb::kForWrite) != Acad::eOk) return false;
	pLayerTbl->close();

	pLayerTblRecord->setLineWeight(lwStdWeight);
	pLayerTblRecord->setLinetypeObjectId(objLTypeId);
	pLayerTblRecord->setColor(color);
	pLayerTblRecord->close();

	return true;
}
