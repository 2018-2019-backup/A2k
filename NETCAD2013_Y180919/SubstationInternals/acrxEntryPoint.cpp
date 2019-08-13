// (C) Copyright 2002-2007 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//

//-----------------------------------------------------------------------------
//----- acrxEntryPoint.cpp
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "resource.h"

#include "NSMainSubstaionDlg.h"
#include "NSDWGMgr.h"
#include "NSDatabaseMgr.h"
#include <ODBCINST.H>

//-----------------------------------------------------------------------------
#define szRDS _RXST("NS")



//-----------------------------------------------------------------------------
//----- ObjectARX EntryPoint
class CSubstationInternalsApp : public AcRxArxApp {

public:
	CSubstationInternalsApp () : AcRxArxApp () {}

	virtual AcRx::AppRetCode On_kInitAppMsg (void *pkt) {
		// TODO: Load dependencies here

		// You *must* call On_kInitAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kInitAppMsg (pkt) ;
		
		// TODO: Add your initialization code here

		return (retCode) ;
	}

	virtual AcRx::AppRetCode On_kUnloadAppMsg (void *pkt) {
		// TODO: Add your code here

		// You *must* call On_kUnloadAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kUnloadAppMsg (pkt) ;

		// TODO: Unload dependencies here

		return (retCode) ;
	}

	virtual void RegisterServerComponents () {
	}

	static bool IsSettingsDotINIFilePresent()
	{
		/*ConfigFile *m_pConfigFile;

		m_pConfigFile = new ConfigFile;*/

		TCHAR szAppPath[MAX_PATH+1];
		GetModuleFileName(_hdllInstance,szAppPath,MAX_PATH+1);
		CString strAppPath = szAppPath;
		int nIndex = strAppPath.ReverseFind('\\');
		if(nIndex != -1)
		{
			strAppPath = strAppPath.Left(nIndex);
		}
		strAppPath = strAppPath + "\\settings.ini";
		NSSTRING szSettingsFile = strAppPath;
		ConfigFile *pConfigFile = NULL;
		try
		{
			pConfigFile = new ConfigFile((TCHAR*)szSettingsFile.c_str());
		}
		catch(...)
		{
			MessageBox(acedGetAcadFrame()->m_hWnd, _T("Missing settings.ini file"), _T("Substation Internals"), MB_ICONEXCLAMATION|MB_OK);
			return false;
		}

		TCHAR m_szMDBLocation[_MAX_PATH + 1];
		TCHAR m_szXMLDBLocation[_MAX_PATH + 1];
		if(pConfigFile->readString(m_szXMLDBLocation,_T("NET_CAD_DB_PATH")))
		{
			CNSDatabaseMgr::getInstance()->g_csNetCadDBPath=m_szXMLDBLocation;
		}
		if(pConfigFile->readString(m_szMDBLocation,_T("LOCAL_MDB")))
		{
			return createDSN(m_szMDBLocation);
		}

		delete pConfigFile;
		pConfigFile = NULL;

		
	}

	static bool createDSN(TCHAR* szMDBLoc)
	{
		HKEY hKey;
		bool bCreateDSN = false;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\ODBC\\ODBC.INI\\SUBSTN"), 0L, KEY_QUERY_VALUE, &hKey))
		{ 
			DWORD dwLength;
			TCHAR strValue[_MAX_PATH + 1] ;
			LONG ret = RegQueryValueEx(hKey,_T("DBQ"), NULL, NULL, (LPBYTE)strValue,
				&dwLength);

			if(ret != 0 && (NSACCESS(szMDBLoc, 0) != -1))	
				bCreateDSN = true;
			else
			{
				int nAccess = NSACCESS(szMDBLoc, 0);
				if((NSSTRCMP(szMDBLoc, strValue) != 0) && (nAccess != -1))
				{
					bCreateDSN = true;
				}
			}
		} 
		else
		{
			if(NSACCESS(szMDBLoc, 0) != -1)
			{
				bCreateDSN = true;
			}
			else
			{
				acutPrintf(_T("Can't Access %s"),szMDBLoc);
				bCreateDSN = false;
				//return false
			}
		}

		if(bCreateDSN)
		{
			TCHAR szDesc[_MAX_PATH + 1];
			TCHAR szAttributes[_MAX_PATH + 1];
			//Use Hexadecimal 'FF' (=255) as temporary place holder
			wsprintf(szDesc, _T("DSN=SUBSTN \xFF DESCRIPTION=TEST DES \xFF DBQ=%s\xFF \xFF "), szMDBLoc);
			int mlen = (int)NSSTRLEN(szDesc);
			int i = 0;
			int j = 0;
			//Loop to replace "FF" by "\0"(so as to store multiple strings into one):
			while(i<mlen-1)
			{
				//#ifdef _VS_2002
				//	if ((szDesc[i] == '\xFF') && (szDesc[i+1] == ' '))
				//#else
					if ((szDesc[i] == L'\xFF') && (szDesc[i+1] == L' '))
				//#endif
			//	if ((szDesc[i] == L'\xFF') && (szDesc[i+1] == L' '))
				{
					szAttributes[j] = '\0';
					i++;
				}
				else
				{
					szAttributes[j] = szDesc[i];
				}
				i++;
				j++;
			}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Code added by SJ, DCS, 24.06.2013
      TCHAR sDSNStr[512];
      TCHAR sMDBDriver[512];

      _stprintf(sDSNStr, _T("DSN=SUBSTN;UID=;PWD=;DBQ=%s;"), szMDBLoc);

#ifdef WIN64
      _stprintf(sMDBDriver, _T("Microsoft Access Driver (*.mdb, *.accdb)"));
#else
      _stprintf(sMDBDriver, _T("Microsoft Access Driver (*.mdb)"));
#endif

	  //Commented by XML _TRANSLATOR
      //if (SQLConfigDataSource(NULL, ODBC_ADD_DSN, sMDBDriver, sDSNStr) == FALSE)
      //{ 
      //  CString csError;
      //  csError.Format(_T("Unable to setup DSN \"SUBSTN\"\nto %s"), szMDBLoc);
      //  AfxMessageBox(csError);
      //  return FALSE; 
      //}
      return TRUE;

// End code added by SJ, DCS, 24.06.2013
/////////////////////////////////////////////////////////////////////////////////////////////////////////

      //return  SQLConfigDataSource(NULL, ODBC_ADD_SYS_DSN, _T("Microsoft Access Driver (*.mdb)\0"), (LPCWSTR)szAttributes);
			//return false;
		}
		else
		{
			return true;
		}
	}


	static bool CorrectDrawingTemplateIsOpened()
	{
			CNSDWGMgr ThisDrawing;
			if( ThisDrawing.CheckDWTFileStatus() == false )
			{
				MessageBox(acedGetAcadFrame()->m_hWnd,_T("Please Open DWT Template"), _T("Error"), MB_ICONERROR | MB_OK);

				return false;
			}
			else
			{
				return true;
			}
	}
	static bool SettingsDotINIFileIsPresent()
	{
		if (IsSettingsDotINIFilePresent() == false)
		{
				return false;
		}
		else
		{
				return true;
		}
	}
	static bool DatabaseConnectedSuccessfully()
	{
	/*	if(NS_SUCCESS != CNSDatabaseMgr::getInstance()->openDatabase(_T(""), _T(""), _T("DSN=SUBSTN")))
		{
					MessageBox(acedGetAcadFrame()->m_hWnd,_T("Database connection Failed"), _T("Error"), MB_ICONERROR | MB_OK);
					return false;
		}
		else*/
		{
					return true;
		}
	}
	// ----- NSSubstationInternals.NET_Substation command
	static void NSSubstationInternalsNET_Substation(void)
	{
		if(CorrectDrawingTemplateIsOpened())
		{
			if(SettingsDotINIFileIsPresent())
			{
				if(DatabaseConnectedSuccessfully())
				{
						InvokeMainUI();
				}
			}
		}
		
	}

	static void InvokeMainUI(void)
	{
				AcDbObjectId objID;

				switch (IsSubstationPreSelected(objID))
				{
					case -1: 						
						break;
					case 0: // nothing is selected, so start with out editing mode
									{										
										MYSTRINGSTRINGMAP mapOfAttributesFromRegistry;
										getAttributeInfoFromWithInBlock(mapOfAttributesFromRegistry);

										CAcModuleResourceOverride myResources;
										CNSMainSubstaionDlg* objDlg = new CNSMainSubstaionDlg(mapOfAttributesFromRegistry,acedGetAcadFrame());
										if(objDlg!=NULL)
										{
											objDlg->DoModal();
											delete objDlg;
										}

									}
						break;
					case 1: //Valid Substation is selected, so start in edit mode
							{
								AcDbEntity *pEnt;        
									acdbOpenAcDbEntity(pEnt,objID,AcDb::kForRead);

									if (pEnt->isKindOf(AcDbBlockReference::desc()))
									{		
										CNSDWGMgr ThisDrawing;
										ThisDrawing.SubStnBlokEdit(pEnt);
									}						

								pEnt->close();
							}

						break;					
				}			

	}	

	static void getAttributeInfoFromWithInBlock(MYSTRINGSTRINGMAP &mapOfAttributesFromBlock)
	{
		// registry read directly in UI
	}


	
	static int IsSubstationPreSelected(AcDbObjectId &entId)
	{
		ads_name sset;
		const ACHAR *str = _T("_I");
		int err = acedSSGet(str, NULL, NULL, NULL, sset);
		if (err != RTNORM) 
		{
			return 0;
		}

		long i;
		int length;
		ads_name ename;
		acedSSLength(sset, &length);

		if (length > 1 )
		{
			acutPrintf(_T("\nFor Editing Substation Edit, select only one Substation and fire command\n"));
			return -1;
		}

		for (i = 0; i < 1; i++) // only 1st one is considered as per SRS
		{
			acedSSName(sset, i, ename);
			acdbGetObjectId(entId, ename);

					CNSDWGMgr ThisDrawing;
					std::map<NSSTRING, NSSTRING> mapXData;
					ThisDrawing.fillXDATAMAP(entId,mapXData);

					MYSTRINGSTRINGMAP::iterator m_XDATAMapIterator;
					m_XDATAMapIterator = mapXData.find(_T("IsSubstation"));
					if(m_XDATAMapIterator != mapXData.end())
					{
					   NSSTRING name =  (*m_XDATAMapIterator).first;
					   NSSTRING Val =  (*m_XDATAMapIterator).second;

					    if((NSSTRCMP(name.c_str(), _T("NS::Okey")) != 0))
						{
							return 1;
						}
						else
						{
							acutPrintf(_T("\nSelected entity is not a valid schematic\n"));
							return -1;
						}
					}
					else
					{
					   acutPrintf(_T("\nSelected entity is not a valid schematic\n"));
					   return -1;
					}


		}		

		acedSSFree(sset);

		return false;
	}

} ;

//-----------------------------------------------------------------------------
IMPLEMENT_ARX_ENTRYPOINT(CSubstationInternalsApp)

ACED_ARXCOMMAND_ENTRY_AUTO(CSubstationInternalsApp, NSSubstationInternals, NET_Substation, NS, ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET | ACRX_CMD_REDRAW, NULL)


