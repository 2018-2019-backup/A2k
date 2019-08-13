////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : NETCAD
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : acrxEntryPoint.cpp
// Description      : Main application loader file for eCapture
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General defines
#define szRDS _RXST("")
#define ARX_COMMAND L"New Pole and Conductors"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
extern BOOL ReadINIFile();                  // ReadINI.cpp
extern void Command_NewPoleAndConductors(); // InsertPolesAndConductorsDlg.cpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class name       : CDXFTransApp
// Description      : The one and only application class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDwgTools_NET_NewPoleAndConductors : public AcRxArxApp
{
  // Public section
public:

  // Default constructor
  CDwgTools_NET_NewPoleAndConductors() : AcRxArxApp() { }

  // Called when the ARX is first loaded into AutoCAD
  virtual AcRx::AppRetCode On_kInitAppMsg(void *pkt) 
  {
    // You *must* call On_kInitAppMsg here
    AcRx::AppRetCode retCode = AcRxArxApp::On_kInitAppMsg(pkt);

    // If the version is 2011, read the INI file
    if (isAcad2011(ARX_COMMAND)) ReadINIFile();
		// ReadINIFile();

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
  static void DwgTools_NET_NEWPANDC(void) 
	{
		if (isAcad2011(ARX_COMMAND) && ReadINIFile()) Command_NewPoleAndConductors(); 
		// if (ReadINIFile()) Command_NewPoleAndConductors(); 
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This will implement the class as the Entry Point
IMPLEMENT_ARX_ENTRYPOINT(CDwgTools_NET_NewPoleAndConductors)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command registrations
ACED_ARXCOMMAND_ENTRY_AUTO(CDwgTools_NET_NewPoleAndConductors, DwgTools, _NET_NEWPANDC, NNPC,	ACRX_CMD_MODAL, NULL)
