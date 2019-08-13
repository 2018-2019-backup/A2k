////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : NETCAD
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : CheckUpdates.cpp
// Created          : 11th February 2011
// Created by       : S. Jaisimha
// Description      : Checks for updates to file specified in the database
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "ConfirmUpdateDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CopyServerFile
// Description  : Simply copies the file from the server to the local system
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CopyServerFile(CString csServerFile, CString csLocalFile)
{
  // Ensure that the folder of the local file exists
  CString csLocalPath = csLocalFile.Mid(0, csLocalFile.ReverseFind(_T('\\')));
  CreateDirectory(csLocalPath, NULL);

  // Overwrite the local database with the server database
  if (CopyFile(csServerFile, csLocalFile, FALSE) == FALSE)
  {
    LPVOID lpMsgBuf;
    TCHAR szBuf[201]; 
    DWORD dw = GetLastError(); 
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
    wsprintf(szBuf, _T("%s\n%s\n\nCould not copy server file to the local file.\n\nError %d: %s"), csServerFile, csLocalFile, dw, lpMsgBuf); 
    appMessage(szBuf, MB_ICONSTOP);
    LocalFree(lpMsgBuf);
    return -1;
  }
  else
  {
    TCHAR szBuf[201]; 
    wsprintf(szBuf, _T("New file have been downloaded. Please restart AutoCAD for changes to take effect")); 
    appMessage(szBuf, MB_ICONSTOP);
  }
  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : UpdateFileFromServer
// Description  : Compares the last modified date/time values for the local and server files and updates the local if necessary
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int f_iDownloadStatus = 0;
int UpdateFileFromServer(CString csServerFile, CString csLocalFile)
{
  // If the user has elected to postpone the update, return now
  if (f_iDownloadStatus == 2) return TRUE;

  // Ensure file on server is present
  if (CheckFileAccess(csServerFile, 00) == FALSE) return -1;

  // Check for presence of local file
  if (_taccess(csLocalFile, 00) == -1) 
  {
    // If it is not found, simply copy the file and return the status
    return CopyServerFile(csServerFile, csLocalFile);
  }

  // Get the last modified date/time of the server and local files
  CFileStatus ftServer; CFile::GetStatus(csServerFile, ftServer);
  CFileStatus ftLocal;  CFile::GetStatus(csLocalFile,  ftLocal);

  // If the server file modification date/time is later than the local date/time
  if (ftServer.m_mtime > ftLocal.m_mtime)
  {
    // If this is the first time we are coming here
    if (f_iDownloadStatus == 0)
    {
      // Tell the user that there is an update
      CAcModuleResourceOverride useThis;
      CConfirmUpdateDlg dlgConfirm;
      if (dlgConfirm.DoModal() == IDCANCEL) 
      {
        // User has elected to postpone the update
        f_iDownloadStatus = 2;
        return FALSE;
      }

      // User has elected to download the update
      f_iDownloadStatus = 1;
    }

    // Copy the file and return the status
    return CopyServerFile(csServerFile, csLocalFile);
  }

  // Nothing to do
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CheckForUpdates
// Description  : Clones the layer from the standard drawing into the current drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckForUpdates()
{
  ///////////////////////////////////////////////////////////////////////////////////////////
  // There is no need to have a local copy of the "Updates.mdb" file.
  // Change requested via mail dated 02.06.2011.
  //
  // Check and update the local database from the server
  // int iStatus = UpdateFileFromServer(g_csServerUpdatesMDB, g_csLocalUpdatesMDB, true);
  // if (iStatus == -1)    return FALSE; // Error comparing files
  // if (iStatus == FALSE) return TRUE;  // Files are up-to-date
  //
  ///////////////////////////////////////////////////////////////////////////////////////////

  // Setup a data source to the server database (changed from local database due to above)
  if (SetupDataSource(DSN_UPDATE, g_csServerUpdatesMDB) == FALSE) return FALSE;

  // Retrieve the list of files in the table
  CQueryTbl tblUpd;
  CString csSQL = _T("SELECT [fldSourceFile], [fldSourceLocation], [fldTargetFile], [fldTargetLocation] FROM [tblNET_Updates]");
  if (!tblUpd.SqlRead(DSN_UPDATE, csSQL, __LINE__, __FILE__, __FUNCTIONW__,true)) return FALSE;
  if (tblUpd.GetRows() <= 0) { appMessage(_T("NET CAD Update: No files have been specified for updating."), MB_ICONSTOP); return TRUE; }

  CString csSourceFile, csTargetFile;
  CStringArray *pcsaData = NULL;

  // For each file in the result
  for (int iCtr = 0; iCtr < tblUpd.GetRows(); iCtr++)
  {
    // Get the row data
    pcsaData = tblUpd.GetRowAt(iCtr);

    // Concatenate the source and target location and file
    csSourceFile.Format(_T("%s\\%s"), pcsaData->GetAt(1), pcsaData->GetAt(0));
    csTargetFile.Format(_T("%s\\%s"), pcsaData->GetAt(3), pcsaData->GetAt(2));

    // If the source file specification contains a wild card character
    if (csSourceFile.FindOneOf(_T("*?")) != -1)
    {
      // Run a file find on the server
      CFileFind ffServer;
      CString csWildCard = csSourceFile;
      BOOL bFound = ffServer.FindFile(csWildCard);
      while (bFound)
      {
        // Get the next file (required before next step)
        bFound = ffServer.FindNextFile();

        // Set the path to the server file
        csSourceFile = ffServer.GetFilePath();

        // Format the path to the local file
        csTargetFile.Format(_T("%s\\%s"), pcsaData->GetAt(3), ffServer.GetFileName());

        // Check and update the file
        UpdateFileFromServer(csSourceFile, csTargetFile);
      }
    }
    // Otherwise, check and update the file
    else UpdateFileFromServer(csSourceFile, csTargetFile);
  }

  // Success
  return TRUE;
}
