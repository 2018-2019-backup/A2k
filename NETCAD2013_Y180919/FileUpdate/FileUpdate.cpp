////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : DXFTrans.cpp
// Created          : 18th January 2008
// Created by       : S. Jaisimha
// Description      : Main application loader file for eCapture
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include <afxdllx.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Define the sole extension module object.
AC_IMPLEMENT_EXTENSION_MODULE(FileUpdateDLL)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Please do not remove the 3 following lines. These are here to make .NET MFC Wizards
// running properly. The object will not compile but is require by .NET to recognize
// this project as being an MFC project
#ifdef NEVER
AFX_EXTENSION_MODULE FileUpdateExtDLL = { NULL, NULL };
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Now you can use the CAcModuleResourceOverride class in your application to switch to the correct resource instance.
// Please see the ObjectARX Documentation for more details

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DLL Entry Point
extern "C" BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) 
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

  // If the DLL is being loaded
	if (dwReason == DLL_PROCESS_ATTACH)
  {
    _hdllInstance = hInstance;
		FileUpdateDLL.AttachInstance(hInstance);
		InitAcUiDLL();
	} 
  // If the DLL is being unloaded
  else if (dwReason == DLL_PROCESS_DETACH) 
  {
		FileUpdateDLL.DetachInstance();
	}

  // Success
	return (TRUE);
}
