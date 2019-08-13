////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : NETCAD
// Client           : Energy Australia, via AEC Systems, Sydney
// File name        : acrxEntryPoint.cpp
// Created          : 18th January 2008
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General defines
#define szRDS _RXST("")
#define ARX_COMMAND L"MOVE to ON/OFF LAYER"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
extern BOOL ReadINIFile();        // ReadINI.cpp
extern void Command_Visibility(); // Visibility.cpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class name       : CDwgTools_NET_MoveOnOff
// Description      : The one and only application class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDwgTools_NET_MoveOnOff : public AcRxArxApp
{
  // Public section
public:

  // Default constructor
  CDwgTools_NET_MoveOnOff() : AcRxArxApp() { }

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
  static void DwgTools_NET_MoveOnOff(void) { if (isAcad2011(ARX_COMMAND) && ReadINIFile()) Command_Visibility(); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This will implement the class as the Entry Point
IMPLEMENT_ARX_ENTRYPOINT(CDwgTools_NET_MoveOnOff)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Command registrations
ACED_ARXCOMMAND_ENTRY_AUTO(CDwgTools_NET_MoveOnOff, DwgTools, _NET_MoveOnOff, NMOF,	ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET | ACRX_CMD_REDRAW, NULL)
