/****************************************************************************
*                                                                           *
* Hherror.h --- HTML Help API errors                                        *
*                                                                           *
* Copyright (c) 1996-1997, Microsoft Corp. All rights reserved.             
*
*                                                                           *
***************************************************************************
*/ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __HHERROR_H__
#define __HHERROR_H__

#ifdef __cplusplus
extern "C" 
{
#endif  // __cplusplus


	// HH_LAST_ERROR Command Related structures and constants

	typedef struct tagHH_LAST_ERROR
	{
		int cbStruct ;
		HRESULT hr ;            // The last error code.
		BSTR description ;      // Unicode description string.
	} HH_LAST_ERROR ;

	// Error codes

  #define HH_E_FILENOTFOUND              MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0201L ) // %1 could not be found.
  #define HH_E_TOPICDOESNOTEXIST         MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0202L ) // The requested topic does not exist.
  #define HH_E_INVALIDHELPFILE           MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0203L ) // %1 is not a valid help file.
  #define HH_E_NOCONTEXTIDS              MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x020AL ) // Help file does not contain context ids.
  #define HH_E_CONTEXTIDDOESNTEXIT       MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x020BL ) // The context id does not exist.

	// 0x0300 - 0x03FF reserved for keywords
  #define HH_E_KEYWORD_NOT_FOUND         MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0300L ) // no hits found.
  #define HH_E_KEYWORD_IS_PLACEHOLDER    MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0301L ) // keyword is a placeholder or a "runaway" see also.
  #define HH_E_KEYWORD_NOT_IN_SUBSET     MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0302L ) // no hits found because of subset exclusion.
  #define HH_E_KEYWORD_NOT_IN_INFOTYPE   MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0303L ) // no hits found because of infotype exclusion.
  #define HH_E_KEYWORD_EXCLUDED          MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0304L ) // no hits found because of infotype and subset exclusion.
  #define HH_E_KEYWORD_NOT_SUPPORTED     MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, 0x0305L ) // no hits found because of keywords not being supported in this mode.

#ifdef __cplusplus
}
#endif  // __cplusplus


#endif