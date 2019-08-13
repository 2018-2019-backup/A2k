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
#include "Resource.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General defines
#define szRDS _RXST("")
#define ARX_COMMAND _T("Validate")

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
extern BOOL ReadINIFile();                // ReadINI.cpp
extern void Command_Validate();           // ValidateInt.cpp
extern void Command_ExtValidate();        // ExtValidateDlg.cpp

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

    // If the version is 2011, read the INI file
    if (isAcad2011(ARX_COMMAND)) ReadINIFile();

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
  static void DXFTrans_NET_Validate(void)    { if (isAcad2011(ARX_COMMAND) && ReadINIFile()) Command_Validate();    }
//static void DXFTrans_NET_ExtValidate(void) { if (isAcad2011(ARX_COMMAND) && ReadINIFile()) Command_ExtValidate(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This will implement the class as the Entry Point
IMPLEMENT_ARX_ENTRYPOINT(CDXFTransApp)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command registrations
ACED_ARXCOMMAND_ENTRY_AUTO(CDXFTransApp, DXFTrans, _NET_Validate,    NV,  ACRX_CMD_MODAL, NULL)

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Command removed on request, SJ, 26.05.2011
//ACED_ARXCOMMAND_ENTRY_AUTO(CDXFTransApp, DXFTrans, _NET_ExtValidate, NEV, ACRX_CMD_MODAL, NULL)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
