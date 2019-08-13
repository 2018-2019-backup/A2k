////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : NETCAD
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : FileVersionsDlg.cpp
// Created          : 15th February 2011
// Created by       : S. Jaisimha
// Description      : Displays date/time stamps for various files
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "FileVersionsDlg.h"


// CFileVersionsDlg dialog
IMPLEMENT_DYNAMIC(CFileVersionsDlg, CDialog)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the static variables used for sorting
int  CFileVersionsDlg::m_iSortColumn = 0;
bool CFileVersionsDlg::m_bAscendingSort = true;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : GetFileType
// Description  : Retrieves the file type string from the system, given a file name. This file need not exist, it just uses the extension to get the information.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString GetFileType(CString strFPath)
{
  SHFILEINFO sfi;
  memset(&sfi, 0, sizeof(sfi));
  SHGetFileInfo(strFPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);

  static TCHAR lpBuff[MAX_PATH];
  lpBuff[0] = TCHAR ('\0');

  lstrcpy(lpBuff, sfi.szTypeName);
  if (lpBuff[0] == TCHAR('\0'))
  {
    int nDotIdx = strFPath.ReverseFind(TCHAR('.'));
    int nBSIdx = strFPath.ReverseFind(TCHAR('\\'));
    if (nDotIdx > nBSIdx)
    {
      strFPath = strFPath.Mid(nDotIdx + 1);
      strFPath.MakeUpper();
      lstrcpy (lpBuff, strFPath + TCHAR (' '));
    }

    lstrcat (lpBuff, _T("File"));
  }

  CString csFileType = lpBuff;
  return csFileType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : GetFileIconIndex
// Description  : Retrieves the file icon index from the system, given a file name. This file need not exist, it just uses the extension to get the information.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetFileIconIndex(CString csFileName)
{
  SHFILEINFO sfi;
  SHGetFileInfo(csFileName, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
  return sfi.iIcon;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CFileVersionsDlg::CFileVersionsDlg
// Description  : Default constructor for the class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFileVersionsDlg::CFileVersionsDlg(CWnd* pParent /*=NULL*/)	: CDialog(CFileVersionsDlg::IDD, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CFileVersionsDlg::~CFileVersionsDlg
// Description  : Default destructor for the class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFileVersionsDlg::~CFileVersionsDlg()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CFileVersionsDlg::DoDataExchange
// Description  : Control to variable data exchange
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileVersionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_FILES_LIST, m_lcFiles);
  DDX_Control(pDX, IDC_STATIC_UPDATE, m_stUpdate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : BEGIN_MESSAGE_MAP
// Description  : Dialog message handlers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CFileVersionsDlg, CDialog)
  ON_NOTIFY(HDN_ITEMCLICKA, 0, &CFileVersionsDlg::OnSortList) 
  ON_NOTIFY(HDN_ITEMCLICKW, 0, &CFileVersionsDlg::OnSortList)
  ON_BN_CLICKED(IDC_SAVE,      &CFileVersionsDlg::OnBnClickedSave)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CFileVersionsDlg::OnInitDialog
// Description  : Called by the MFC framework before the dialog is displayed on screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFileVersionsDlg::OnInitDialog()
{
  // call the parent class implementation first
  CDialog::OnInitDialog();

  // Initialize the list control
  m_lcFiles.InsertColumn(0,  _T("File name"), LVCFMT_LEFT,   240);
  m_lcFiles.InsertColumn(1,  _T("Type"),      LVCFMT_LEFT,   195);
  m_lcFiles.InsertColumn(2,  _T("Date"),      LVCFMT_CENTER, 120);
  m_lcFiles.InsertColumn(3,  _T("Location"),  LVCFMT_LEFT,   295);
  m_lcFiles.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  // Get a handle to the system small icon list and assign it to the list control
  m_lcFiles.SetImageList(m_silFiles.GetImageList(), LVSIL_SMALL);

  // Setup a data source to the local database
  if (SetupDataSource(DSN_UPDATE, g_csServerUpdatesMDB) == FALSE) { OnCancel(); return TRUE; }

  // Retrieve the list of files in the table
  CQueryTbl tblUpd;
  CString csSQL = _T("SELECT [fldTargetLocation], [fldTargetFile] FROM [tblNET_Updates]");
  if (!tblUpd.SqlRead(DSN_UPDATE, csSQL, __LINE__, __FILE__, __FUNCTIONW__),true) return FALSE;
  if (tblUpd.GetRows() <= 0) { OnCancel(); return TRUE; }

  CString csTargetFile;
  CStringArray *pcsaData = NULL;

  // For each file in the result
  for (int iCtr = 0; iCtr < tblUpd.GetRows(); iCtr++)
  {
    // Get the row data
    pcsaData = tblUpd.GetRowAt(iCtr);

    // Concatenate the target location and file
    csTargetFile.Format(_T("%s\\%s"), pcsaData->GetAt(0), pcsaData->GetAt(1));

    // If the file specification contains a wild card character
    if (csTargetFile.FindOneOf(_T("*?")) != -1)
    {
      // Run a file find
      CFileFind ffSearch;
      CString csWildCard = csTargetFile;
      BOOL bFound = ffSearch.FindFile(csWildCard);
      while (bFound)
      {
        // Get the next file (required before next step)
        bFound = ffSearch.FindNextFile();

        // Set the path to the server file
        csTargetFile = ffSearch.GetFilePath();

        // Get the date time stamp of the file
        AddFileDetails(csTargetFile);
      }
    }
    // Otherwise, get the date time stamp of the file
    else AddFileDetails(csTargetFile);
  }

  /*
  // Get the date stamp of the local and server "Updates.mdb"
  CFileStatus fsLocal;  if (CFile::GetStatus(g_csLocalUpdatesMDB,  fsLocal)  == FALSE) { OnCancel(); return TRUE; }
  CFileStatus fsServer; if (CFile::GetStatus(g_csServerUpdatesMDB, fsServer) == FALSE) { OnCancel(); return TRUE; }

  // Display the status
  CString csUpdate;
  csUpdate.Format(_T("Last successful download of updates was on %s.  "), fsLocal.m_mtime.Format(_T("%d-%m-%Y %H:%M")));

  // If the server version is later
  if (fsServer.m_mtime > fsLocal.m_mtime) csUpdate += _T("New updates are available!");
  else csUpdate += _T("There are no new updates.");

  // Show the status
  m_stUpdate.SetWindowText(csUpdate);
  */

  // Everything is OK
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : AddFileDetails
// Description  : Adds the details of the file to the list control
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileVersionsDlg::AddFileDetails(CString csFilePath)
{
  // Get the date stamp
  CFileStatus fsFile;
  if (CFile::GetStatus(csFilePath, fsFile) == FALSE) return;
  CString csFileDate = fsFile.m_mtime.Format(_T("%d-%m-%Y %H:%M"));

  // Get the file type
  CString csFileType = GetFileType(csFilePath);

  // Get the file icon
  int iFileIcon = GetFileIconIndex(csFilePath);

  // Separate the file name and path
  CString csPath = csFilePath.Mid(0, csFilePath.ReverseFind(_T('\\')));
  CString csName = csFilePath.Mid(csFilePath.ReverseFind(_T('\\')) + 1);

  // Add the file to the list
  int iIndex = m_lcFiles.GetItemCount();
  m_lcFiles.InsertItem(iIndex, csName, iFileIcon);
  m_lcFiles.SetItemText(iIndex, 1, csFileType);
  m_lcFiles.SetItemText(iIndex, 2, csFileDate);
  m_lcFiles.SetItemText(iIndex, 3, csPath);
  m_lcFiles.SetItemData(iIndex, iIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CFileVersionsDlg::CompareItemsProc
// Description  : Helper function to sort the list control
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*static*/ int CALLBACK CFileVersionsDlg::CompareItemsProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  CListCtrl* pListCtrl = (CListCtrl*) lParamSort;
  CString strItem1 = pListCtrl->GetItemText(int(lParam1), m_iSortColumn);
  CString strItem2 = pListCtrl->GetItemText(int(lParam2), m_iSortColumn);

  // If the values are same, return 0
  if (strItem1 == strItem2) return 0;

  // If we are sorting on the second column, perform a date based sorting
  if (m_iSortColumn == 2)
  {
    // Convert the strings from "DD-MM-YYYY HH:MM" format to date
    CTime tmItem1(_tstoi(strItem1.Mid(6, 4)), _tstoi(strItem1.Mid(3, 2)), _tstoi(strItem1.Mid(0, 2)), _tstoi(strItem1.Mid(11, 2)), _tstoi(strItem1.Mid(14, 2)), 0);
    CTime tmItem2(_tstoi(strItem2.Mid(6, 4)), _tstoi(strItem2.Mid(3, 2)), _tstoi(strItem2.Mid(0, 2)), _tstoi(strItem2.Mid(11, 2)), _tstoi(strItem2.Mid(14, 2)), 0);

    // Calculate the difference in dates, as per the sort order required
    CTimeSpan tmSort;
    if (m_bAscendingSort) tmSort = tmItem2 - tmItem1;
    else tmSort = tmItem1 - tmItem2;

    // Return the difference (positive or negative)
    return int(tmSort.GetTotalSeconds());
  }

  // Otherwise, return the string comparison (we wont come here for column 1 anyway)
  return (m_bAscendingSort ? _tcscmp(strItem1, strItem2) : _tcscmp(strItem2, strItem1));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CFileVersionsDlg::OnSortList
// Description  : Called when the user clicks on one of the column headers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileVersionsDlg::OnSortList(NMHDR* pNMHDR, LRESULT* pResult)
{
  // Convert the first parameter
  HD_NOTIFY *pHdn = (HD_NOTIFY *)pNMHDR;

  // If the user clicked the left mouse button on the header control
  if (pHdn->iButton == 0)
  {
    // Copy the column number
    m_iSortColumn = pHdn->iItem;

    // Sort the column in ascending or descending order
    m_lcFiles.SortItems(CompareItemsProc, (LPARAM)&m_lcFiles);

    // Reset the item data for the rows
    for (int iCtr = 0; iCtr < m_lcFiles.GetItemCount(); iCtr++) m_lcFiles.SetItemData(iCtr, iCtr);

    // Invert the sort order
    m_bAscendingSort = !m_bAscendingSort;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CFileVersionsDlg::OnBnClickedSave
// Description  : Called when the user clicks on the "Save" button
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileVersionsDlg::OnBnClickedSave()
{
  CTime tmNow = CTime::GetCurrentTime();

  // Form the save file name
  CString csSaveFile; csSaveFile.Format(_T("%s NET Versions.csv"), tmNow.Format(_T("%Y-%m-%d-%H-%M-%S")));

  // Get the file save location
  //CAcModuleResourceOverride useMe;
  //CFileDialog dlgSave(FALSE, _T("csv"), csSaveFile, OFN_HIDEREADONLY | OFN_EXPLORER, _T("Comma Separated Values (*.csv)|*.csv||"));
  //if (dlgSave.DoModal() == IDCANCEL) return;

  // Prefix the selected path to the file name
  //csSaveFile = dlgSave.GetFolderPath() + _T("\\") + csSaveFile;

  // Set the location of the save file
  CString csSavePath = g_csLocalMDB;                              // C:\NETCAD\Database\NETCAD.mdb
  csSavePath = csSavePath.Mid(0, csSavePath.ReverseFind('\\'));   // C:\NETCAD\Database
  csSavePath = csSavePath.Mid(0, csSavePath.ReverseFind('\\'));   // C:\NETCAD
  csSavePath = csSavePath + _T("\\Version");                      // C:\NETCAD\Version
  csSaveFile = csSavePath + _T("\\") + csSaveFile;                // C:\NETCAD\Version\Something.csv

  // Ensure that the "Version" folder exists
  CreateDirectory(csSavePath, NULL);

  // Open a file in the same location as the ARX
  FILE *fpOut = _wfopen(csSaveFile, _T("w"));
  if (!fpOut) { appMessage(csSaveFile + _T("\n\nUnable to create or write to file."), MB_ICONSTOP); return; }

  // For each entry in the list
  for (int iCtr = 0; iCtr < m_lcFiles.GetItemCount(); iCtr++)
  {
    // Write the details
    fwprintf(fpOut, _T("%s,%s,%s\n"), m_lcFiles.GetItemText(iCtr, 0), m_lcFiles.GetItemText(iCtr, 2), m_lcFiles.GetItemText(iCtr, 3));
  }

  // Close the file
  fclose(fpOut);

  // Success
  appMessage(csSaveFile + _T("\n\nFile written successfully."));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_FileVersions
// Description  : Displays the file versions dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_FileVersions()
{
  // Display the dialog
  CAcModuleResourceOverride useMe;
  CFileVersionsDlg dlgFV;
  dlgFV.DoModal();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_NEXL
// Description  : UNUSED. Prints out the pattern definition for a line type.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_NEXL()
{
  AcDbLinetypeTable *pLTT;
  if (acdbHostApplicationServices()->workingDatabase()->getLinetypeTable(pLTT, AcDb::kForRead) != Acad::eOk) return;

  CString csLineType = _T("HV_OH");
  if (pLTT->has(csLineType))
  {
    AcDbLinetypeTableRecord *pLTTR;
    if (pLTT->getAt(csLineType, pLTTR, AcDb::kForRead) != Acad::eOk) { pLTT->close(); return; }

    pLTT->close();

    CString csLineTypePattern, csTemp, csName;
    double dPLen = pLTTR->patternLength();
    int iNDashes = pLTTR->numDashes();
    ACHAR *pszText;
    for (int iCtr = 0; iCtr < iNDashes; iCtr++)
    {
      double dDashLen = pLTTR->dashLengthAt(iCtr);
      csTemp.Format(_T("%.2f"), dDashLen);
      
      if (csLineTypePattern.IsEmpty()) csLineTypePattern = csTemp;
      else csLineTypePattern = csLineTypePattern + _T(",") + csTemp;

      pszText = NULL;
      Acad::ErrorStatus es = pLTTR->textAt(iCtr, pszText);
      if (pszText)
      {
        csTemp.Format(_T("%s"), pszText);
        csLineTypePattern = csLineTypePattern + _T(",") + csTemp;

        AcGeVector2d geOff = pLTTR->shapeOffsetAt(iCtr);

        AcDbObjectId objStyle = pLTTR->shapeStyleAt(iCtr);
        AcDbTextStyleTableRecord* pTSTR;
        if (acdbOpenObject(pTSTR, objStyle, AcDb::kForRead) == Acad::eOk)
        {
          pTSTR->getName(pszText);
          csName.Format(_T("%s"), pszText);
          pTSTR->close();
        }

        double dScale = pLTTR->shapeScaleAt(iCtr);

        csTemp.Format(_T(" (%.2f,%.2f, %s, %.2f)"), geOff.x, geOff.y, csName, dScale);
        csLineTypePattern = csLineTypePattern + csTemp;
      }
    }

    pLTTR->close();

    acutPrintf(_T("Definition for %s:\n%s\n"), csLineType, csLineTypePattern);
  }
}
