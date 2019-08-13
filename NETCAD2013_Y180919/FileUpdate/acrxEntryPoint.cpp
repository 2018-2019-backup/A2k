////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : acrxEntryPoint.cpp
// Created          : 18th January 2008
// Created by       : S. Jaisimha
// Description      : Main application loader file for eCapture
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General defines
#define szRDS _RXST("")
#define ARX_COMMAND _T("File Update")

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
extern BOOL ReadINIFile();          // ReadINI.cpp
extern BOOL CheckForUpdates();      // CheckUpdates.cpp
extern void Command_FileVersions(); // FileVersionsDlg.cpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class name       : CDXFTransApp
// Description      : The one and only application class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDXFTransApp : public AcRxArxApp
{
  // Public section
public:

  // Default constructor
	CDXFTransApp() : AcRxArxApp() { }

  // Called when the ARX is first loaded into AutoCAD
	virtual AcRx::AppRetCode On_kInitAppMsg(void *pkt) 
  {
		// You *must* call On_kInitAppMsg here
		AcRx::AppRetCode retCode = AcRxArxApp::On_kInitAppMsg(pkt);

    // If the version is 2011
    if (isAcad2011(ARX_COMMAND)) 
    {
      // Read the INI file
      if (ReadINIFile())
      {
        // Check for updates
        CheckForUpdates();
      }
    }

    // Return the result
		return (retCode);
	}

  // Called when the ARX is unloaded from AutoCAD
	virtual AcRx::AppRetCode On_kUnloadAppMsg(void *pkt)
  {
		// You *must* call On_kUnloadAppMsg here
		AcRx::AppRetCode retCode = AcRxArxApp::On_kUnloadAppMsg(pkt);

    // Return the result
    return (retCode);
	}

  // Called to register COM components
	virtual void RegisterServerComponents() 
  {
	}

  // The following functions are used as wrapper functions for registered commands
  static void DXFTrans_NET_Versions(void) { if (isAcad2011(ARX_COMMAND) && ReadINIFile()) Command_FileVersions(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This will implement the class as the Entry Point
IMPLEMENT_ARX_ENTRYPOINT(CDXFTransApp)

ACED_ARXCOMMAND_ENTRY_AUTO(CDXFTransApp, DXFTrans, _NET_Versions, NETVERSION, ACRX_CMD_MODAL, NULL)
