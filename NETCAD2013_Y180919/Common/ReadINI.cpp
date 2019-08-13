////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : NETCAD
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : ReadINI.cpp
// Created          : 18th January 2008
// Created by       : S. Jaisimha
// Description      : Reads the INI file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables
CString g_csHome, g_csStdTemplates, g_csLocalDWT, g_csLocalMDB, g_csMaxConnGap, g_csValidateMove, g_csExcludeOff, g_csPurgeLayers, g_csPurgeBlocks, g_csPurgeText, g_csPurgeLTypes, g_csTranslateScr, g_csMaxFileSize;
CString g_csDWGLocation, g_csMDBLocation, g_csBOMLayer;
CString g_csLocalUpdatesMDB, g_csServerUpdatesMDB,g_csNetCadDBPath;
double g_LegendLinearLen     = 0;
CString g_csProposedPattern  = L"ANSI31";
CString g_csUnderborePattern = L"ANGLE";
double g_dProposedScale      = 1;
double g_dUnderboreScale     = 1;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : UpdateLocalCopy
// Arguments    : 1. Path to the server file, as CString
//                2. Path to the local file, as CString
//
// Called from  : ReadINIFile
// Description  : Checks for presence of both files and updates the local file if necessary
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UpdateLocalCopy(CString csServerFile, CString csLocalFile)
{
  BOOL bLocalExists = TRUE, bUpdateLocal = FALSE;
  CTime tmLocal, tmServer;

  // First, check if the local file is present
  if (_taccess(csLocalFile, 0) == -1) { bLocalExists = FALSE; bUpdateLocal = TRUE; }

  // Next, check if the server file is present
  if (_taccess(csServerFile, 0) == -1)
  {
    CString csNoServer;
    csNoServer.Format(_T("[%s]\n\nThe server configuration file is not accessible."), csServerFile);

    // If the local is also not present
    if (bLocalExists == FALSE)
    {
      // Cannot continue
      appMessage(csNoServer + _T(" Since the local configuration file is also not available, the program cannot continue.\n\nPlease contact your Systems Administrator."), MB_ICONSTOP);
      return FALSE;
    }
    // Otherwise
    else
    {
      // Ask if user wants to risk using the local copy
      if (getConfirmation(csNoServer + _T(" Do you want to proceed with the local copy?")) == IDNO) return FALSE;
    }
  }
  // Otherwise
  else
  {
    // Get the time stamp of the server file
    CFileStatus fsServer;
    CFile::GetStatus(csServerFile, fsServer);
    tmServer = fsServer.m_mtime;
  }

  // If the local file is present
  if (bLocalExists == TRUE)
  {
    // Get the time stamp of the local file
    CFileStatus fsLocal;
    CFile::GetStatus(csLocalFile, fsLocal);
    tmLocal = fsLocal.m_mtime;

    // If the local file is older than the server file
    if (tmLocal < tmServer) bUpdateLocal = TRUE;
  }

  // If we must update the local copy
  if (bUpdateLocal == TRUE)
  {
    // Copy the file from date folder to previous folder
    acutPrintf(_T("\nUpdating local configuration data...\n"));
    if (CopyFile(csServerFile, csLocalFile, FALSE) == FALSE)
    {
      LPVOID lpMsgBuf;
      TCHAR szBuf[201]; 
      DWORD dw = GetLastError(); 
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
      wsprintf(szBuf, _T("%s\n%s\n\nCould not copy server file to the local file.\n\nError %d: %s"), csServerFile, csLocalFile, dw, lpMsgBuf); 
      appMessage(szBuf, MB_ICONSTOP);
      LocalFree(lpMsgBuf);
      return FALSE;
    }
  }

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ReadINIFile
// Called from  : CDXFTransApp::On_kInitAppMsg(), in acrxEntryPoint.cpp
// Description  : Reads the "Settings.ini" file for default settings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ReadINIFile()
{
  CString csError, csErrorTitle = _T("NETCAD: Error in INI");

  // Get the location from which this ARX is loaded
  TCHAR sDllPath[_MAX_PATH];
  GetModuleFileName(_hdllInstance, sDllPath, _MAX_PATH);
  CString csBaseFolder = sDllPath;
  if (csBaseFolder.ReverseFind(_T('\\')) > 0) csBaseFolder = csBaseFolder.Mid(0, csBaseFolder.ReverseFind('\\'));

  // Copy the home folder to the global variable
  g_csHome = csBaseFolder;

  // Check if the "Settings.ini" file is present in this location
  CString csINIFile; 
  csINIFile.Format(_T("%s\\Settings.ini"), g_csHome);
  if (_taccess(csINIFile, 00) == -1) 
  { 
    csError.Format(_T("Unable to read default settings from\n%s\n\nThe file does not exist."), csINIFile);
    appMessage(csError, MB_ICONSTOP);
    return FALSE;    
  }

  // Open the file for reading
  CString csLine;
  CStdioFile cfIni;
  CFileException cfe;
  if (cfIni.Open(csINIFile, CFile::modeRead, &cfe) == FALSE)
  {
    TCHAR szError[1024];
    cfe.GetErrorMessage(szError, 1024);
    csError.Format(_T("Error opening file for reading\n%s\n\n%s"), csINIFile, szError);
    appMessage(csError, MB_ICONSTOP);
    return FALSE;
  }

  // Clear all default variables
  CString csServerDWT, csServerMDB, csLocalMDB, csLocalExtMDB;

  // Read the INI file for all the default values
  while (cfIni.ReadString(csLine) == TRUE)
  {
    // Remove leading and trailing spaces
    csLine.TrimLeft(); csLine.TrimRight();

    // If this first character of the line is ';' or if this is an empty line, continue
    if ((csLine.GetAt(0) == _T(';')) || (csLine.IsEmpty() == TRUE)) continue;

    // Depending on what is the line read, set the appropriate variable
    /**/ if (csLine.Mid(0, 17) == _T("LOCAL_UPDATES_MDB"))       { g_csLocalUpdatesMDB  = csLine.Mid(csLine.Find(_T('=')) + 1); g_csLocalUpdatesMDB.TrimLeft();  }
    else if (csLine.Mid(0, 18) == _T("SERVER_UPDATES_MDB"))      { g_csServerUpdatesMDB = csLine.Mid(csLine.Find(_T('=')) + 1); g_csServerUpdatesMDB.TrimLeft(); }
    else if (csLine.Mid(0, 20) == _T("GLOBAL_TEMPLATE_PATH"))    { g_csStdTemplates     = csLine.Mid(csLine.Find(_T('=')) + 1); g_csStdTemplates.TrimLeft();     }
    else if (csLine.Mid(0, 23) == _T("SERVER_STANDARD_DRAWING")) { csServerDWT          = csLine.Mid(csLine.Find(_T('=')) + 1); csServerDWT.TrimLeft();          }
    else if (csLine.Mid(0, 22) == _T("LOCAL_STANDARD_DRAWING"))  { g_csLocalDWT         = csLine.Mid(csLine.Find(_T('=')) + 1); g_csLocalDWT.TrimLeft();         }
    else if (csLine.Mid(0, 10) == _T("SERVER_MDB"))              { csServerMDB          = csLine.Mid(csLine.Find(_T('=')) + 1); csServerMDB.TrimLeft();          }
    else if (csLine.Mid(0,  9) == _T("LOCAL_MDB"))               { g_csLocalMDB         = csLine.Mid(csLine.Find(_T('=')) + 1); g_csLocalMDB.TrimLeft();         }
    else if (csLine.Mid(0, 12) == _T("EXTERNAL_MDB"))            { csLocalExtMDB        = csLine.Mid(csLine.Find(_T('=')) + 1); csLocalExtMDB.TrimLeft();        }
	
	else if (csLine.Mid(0, 15) == _T("NET_CAD_DB_PATH"))            { g_csNetCadDBPath        = csLine.Mid(csLine.Find(_T('=')) + 1); g_csNetCadDBPath.TrimLeft();  }
    else if (csLine.Mid(0, 17) == _T("CONNECTIVITY_ZONE"))       { g_csMaxConnGap       = csLine.Mid(csLine.Find(_T('=')) + 1); g_csMaxConnGap.TrimLeft();       }
    else if (csLine.Mid(0, 17) == _T("VALIDATE_MOVEMENT"))       { g_csValidateMove     = csLine.Mid(csLine.Find(_T('=')) + 1); g_csValidateMove.TrimLeft();     }
    else if (csLine.Mid(0, 18) == _T("EXCLUDE_OFF_LAYERS"))      { g_csExcludeOff       = csLine.Mid(csLine.Find(_T('=')) + 1); g_csExcludeOff.TrimLeft();       }
    else if (csLine.Mid(0, 12) == _T("PURGE_LAYERS"))            { g_csPurgeLayers      = csLine.Mid(csLine.Find(_T('=')) + 1); g_csPurgeLayers.TrimLeft();      }
    else if (csLine.Mid(0, 12) == _T("PURGE_BLOCKS"))            { g_csPurgeBlocks      = csLine.Mid(csLine.Find(_T('=')) + 1); g_csPurgeBlocks.TrimLeft();      }
    else if (csLine.Mid(0, 17) == _T("PURGE_TEXT_STYLES"))       { g_csPurgeText        = csLine.Mid(csLine.Find(_T('=')) + 1); g_csPurgeText.TrimLeft();        }
    else if (csLine.Mid(0, 16) == _T("PURGE_LINE_TYPES"))        { g_csPurgeLTypes      = csLine.Mid(csLine.Find(_T('=')) + 1); g_csPurgeLTypes.TrimLeft();      }
    else if (csLine.Mid(0, 21) == _T("POST_TRANSLATE_SCRIPT"))   { g_csTranslateScr     = csLine.Mid(csLine.Find(_T('=')) + 1); g_csTranslateScr.TrimLeft();     }
    else if (csLine.Mid(0, 22) == _T("LOCAL_STANDARD_DRAWING"))  { g_csDWGLocation      = csLine.Mid(csLine.Find(_T('=')) + 1); g_csDWGLocation.Trim();          } 
    else if (csLine.Mid(0,  9) == _T("LOCAL_MDB"))						   { g_csMDBLocation      = csLine.Mid(csLine.Find(_T('=')) + 1); g_csMDBLocation.Trim();          }
    else if (csLine.Mid(0,  9) == _T("BOM_LAYER"))						   { g_csBOMLayer         = csLine.Mid(csLine.Find(_T('=')) + 1); g_csBOMLayer.Trim();             }
    else if (csLine.Mid(0, 19) == _T("LEGEND_LINEARLENGTH"))	   { g_LegendLinearLen    = _tstof(csLine.Mid(csLine.Find(_T('=')) + 1).Trim());                   }
		else if (csLine.Mid(0, 21) == _T("PROPOSED_HATCHPATTERN"))	 { g_csProposedPattern  = csLine.Mid(csLine.Find(_T('=')) + 1).Trim();                           }
		else if (csLine.Mid(0, 19) == _T("PROPOSED_HATCHSCALE"))	   { g_dProposedScale     = _tstof(csLine.Mid(csLine.Find(_T('=')) + 1).Trim());                   }
		else if (csLine.Mid(0, 22) == _T("UNDERBORE_HATCHPATTERN"))	 { g_csUnderborePattern = csLine.Mid(csLine.Find(_T('=')) + 1).Trim();                           }
		else if (csLine.Mid(0, 20) == _T("UNDERBORE_HATCHSCALE"))	   { g_dUnderboreScale    = _tstof(csLine.Mid(csLine.Find(_T('=')) + 1).Trim());                   }
    else if (csLine.Mid(0, 16) == _T("MAX_FILE_SETTING"))				 { g_csMaxFileSize      = csLine.Mid(csLine.Find(_T('=')) + 1); g_csMaxFileSize.Trim();          }
  }

  // Close the file
  cfIni.Close();

  // Copy variable for DwgTools compatibility
  g_csMDBLocation = g_csLocalMDB;
	g_csDWGLocation = g_csLocalDWT;
	
  ///////////////////////////////////////////////////////////////////////////////////////////
  // This part is no longer necessary, since there is now a separate UPDATE utility
  // ~SJ, 26.05.2011
  //
  // Update local copies from the server, if necessary
  // if (UpdateLocalCopy(csServerDWT, g_csLocalDWT) == FALSE) return FALSE;
  // if (UpdateLocalCopy(csServerMDB, g_csLocalMDB) == FALSE) return FALSE;
  //
  ///////////////////////////////////////////////////////////////////////////////////////////

  // Setup the data sources for the local databases
  if (SetupDataSource(DSN_ECapture,     g_csLocalMDB)  == FALSE) return FALSE;
//if (SetupDataSource(DSN_ECapture_Ext, csLocalExtMDB) == FALSE) return FALSE; // Not necessary, since External MDB is no longer used
  if (SetupDataSource(DSN_DWGTOOLS,     g_csLocalMDB)  == FALSE) return FALSE;

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : UpdateINIFile
// Called from  : CSelectDXFDlg::OnFileNameOK(), in SelectDXFDlg.cpp
// Description  : Updates the INI file for the given setting and value
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UpdateINIFile(CString csSetting, CString csValue)
{
  CString csError, csErrorTitle = _T("NETCAD: Error in INI");

  // Check if the "Settings.ini" file is present in this location
  CString csINIFile; 
  csINIFile.Format(_T("%s\\Settings.ini"), g_csHome);
  if (_taccess(csINIFile, 00) == -1) 
  { 
    csError.Format(_T("Unable to read default settings from\n%s\n\nThe file does not exist."), csINIFile);
    appMessage(csError, MB_ICONSTOP);
    return FALSE;    
  }

  // Open the file for reading
  CString csLine;
  CStdioFile cfIni;
  CFileException cfe;
  if (cfIni.Open(csINIFile, CFile::modeRead, &cfe) == FALSE)
  {
    TCHAR szError[1024];
    cfe.GetErrorMessage(szError, 1024);
    csError.Format(_T("Error opening file for reading\n%s\n\n%s"), csINIFile, szError);
    appMessage(csError, MB_ICONSTOP);
    return FALSE;
  }

  // Read the INI file into the array
  CStringArray csaLines;
  while (cfIni.ReadString(csLine) == TRUE)
  {
    // If the line contains the given setting, change it to the given value
    if (csLine.Left(csSetting.GetLength()) == csSetting) 
    {
      csLine.Format(_T("%s = %s"), csSetting, csValue);
    }

    // Add the line to the array
    csaLines.Add(csLine);
  }

  // Close the file
  cfIni.Close();

  // Re-open the file for reading
  if (cfIni.Open(csINIFile, CFile::modeWrite, &cfe) == FALSE)
  {
    TCHAR szError[1024];
    cfe.GetErrorMessage(szError, 1024);
    csError.Format(_T("Error opening file for writing\n%s\n\n%s"), csINIFile, szError);
    appMessage(csError, MB_ICONSTOP);
    return FALSE;
  }

  // Write out the contents of the array
  for (int iCtr = 0; iCtr < csaLines.GetSize(); iCtr++) 
  { 
    csLine.Format(_T("%s\n"), csaLines.GetAt(iCtr));
    cfIni.WriteString(csLine);
  }

  // Close the file
  cfIni.Close();

  // Success
  return TRUE;
}
