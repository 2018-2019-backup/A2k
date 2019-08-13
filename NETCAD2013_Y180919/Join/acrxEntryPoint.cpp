////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
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
#define ARX_COMMAND _T("Join")


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
extern BOOL ReadINIFile();  // ReadINI.cpp
extern void Command_Join(); // DimensionCommand.cpp


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class name       : CDXFTransApp
// Description      : The one and only application class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDwgTools_NET_Join : public AcRxArxApp
{
  // Public section
public:

  // Default constructor
  CDwgTools_NET_Join() : AcRxArxApp() { }

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
  static void DwgTools_NET_Join(void) { if (isAcad2011(ARX_COMMAND) && ReadINIFile()) Command_Join(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This will implement the class as the Entry Point
IMPLEMENT_ARX_ENTRYPOINT(CDwgTools_NET_Join)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command registrations
ACED_ARXCOMMAND_ENTRY_AUTO(CDwgTools_NET_Join, DwgTools, _NET_Join, NJ,	ACRX_CMD_MODAL, NULL)
