#pragma once
#include <string>
#include "tchar.h"
#include "vector"
#include "map"

// Error Codes for Success, Fail, SharingViolation
#define NS_SUCCESS					0
#define NS_FAIL						1
#define NS_SHARING_VIOLATION		-1

// Unicode and non unicode functions
#ifdef _UNICODE
	#define NSACCESS _waccess
	#define NSSTRING std::wstring
	#define NS_LP_STR LPCWSTR
	#define NSSTRCPY wcscpy_s
	#define NSPRINT	wprintf
	#define NSSPRINTF swprintf_s
	#define NSSTRCMP _wcsicmp
	#define NSDATE	_wstrdate_s
	#define NSTIME	_wstrtime_s
	#define NSFOPEN _wfopen_s
	#define NSFPRINTF fwprintf
	#define NS_LP_STR LPCWSTR
	#define NSISTREAM std::wistream
	#define NSOSTREAM std::wostream
	#define NSFSTREAM std::wfstream 
	#define NSISTRINGSTREAM std::wistringstream
	#define NSOSTRINGSTREAM std::wostringstream
	#define NSIFSTREAM std::wifstream
	#define	NSDQUOATE _T("")
	#define	NSATOI _wtoi
	#define	NSITOW _itow_s
	#define NSSTRLEN wcslen
	#define NSSSCANF swscanf_s
	#define NSFILEOPEN _wfopen_s
    #define NSSTRTOK wcstok_s
    #define NSATOF _wtof
	#define NSGMTIME _gmtime64_s
	#define NSTIME_T __time64_t
    #define NSSTATFUNC  _wstat64i32
    #define NSSTATSTRUCT  _stat64i32
	#define NSSTRUPR _wcsupr_s
	#define NSSTRCAT	wcscat_s
//#elif _VS_2002
//		//#define NSACCESS _access
//		//#define NSSTRING std::string
//		//#define NS_LP_STR LPCSTR
//		//#define NSSTRCPY strcpy
//		//#define NSPRINT	printf
//		//#define NSSPRINTF sprintf
//		//#define NSSTRCMP _stricmp
//		//#define NSDATE	_strdate
//		//#define NSTIME	_strtime
//		//#define NSFOPEN fopen
//		//#define NSFPRINTF fprintf
//		//#define NS_LP_STR LPCSTR
//		//#define NSISTREAM std::istream
//		//#define NSOSTREAM std::ostream
//		//#define NSFSTREAM std::fstream 
//		//#define NSISTRINGSTREAM std::istringstream
//		//#define NSOSTRINGSTREAM std::ostringstream
//		//#define NSIFSTREAM std::ifstream
//		//#define	NSDQUOATE ""
//		//#define	NSATOI atoi
//		//#define	NSITOW _itoa
//		//#define NSSTRLEN strlen
//		//#define NSSSCANF sscanf
//		//#define NSFILEOPEN fopen
//		//#define NSSTRTOK strtok
//		//#define NSATOF atof
//		//#define NSGMTIME gmtime
//		//#define NSTIME_T time_t
//  //      #define NSSTATFUNC  _stat
//  //      #define NSSTATSTRUCT  _stat
//		//#define NSSTRUPR _strupr
//#else
//		//#define NSACCESS _access
//		//#define NSSTRING std::string
//		//#define NS_LP_STR LPCSTR
//		//#define NSSTRCPY strcpy_s
//		//#define NSPRINT	printf
//		//#define NSSPRINTF sprintf_s
//		//#define NSSTRCMP _stricmp
//		//#define NSDATE	_strdate_s
//		//#define NSTIME	_strtime_s
//		//#define NSFOPEN fopen_s
//		//#define NSFPRINTF fprintf
//		//#define NS_LP_STR LPCSTR
//		//#define NSISTREAM std::istream
//		//#define NSOSTREAM std::ostream
//		//#define NSFSTREAM std::fstream 
//		//#define NSISTRINGSTREAM std::istringstream
//		//#define NSOSTRINGSTREAM std::ostringstream
//		//#define NSIFSTREAM std::ifstream
//		//#define	NSDQUOATE ""
//		//#define	NSATOI atoi
//		//#define	NSITOW _itoa_s
//		//#define NSSTRLEN strlen
//		//#define NSSSCANF sscanf_s
//		//#define NSFILEOPEN fopen_s
//		//#define NSSTRTOK strtok_s
//		//#define NSATOF atof
//		//#define NSGMTIME gmtime
//		//#define NSTIME_T time_t
//  //      #define NSSTATFUNC  _stat
//  //      #define NSSTATSTRUCT  _stat
//		//#define NSSTRUPR _strupr_s
#endif

typedef std::map<NSSTRING, NSSTRING> MYSTRINGSTRINGMAP;

//struct ErrRecord
//{
//public:
//	ErrRecord()
//	{
//		strErrorCategory = _T(" ");
//		nErrorNo = 0;
//		strErrorSeverity = _T("P1");
//		strErrorDescription = _T("Record not found in the database");
//		nLogType = 0; // 0 = Errror type, 1 = Header Type
//	}
//	int nErrorNo;
//	NSSTRING  strErrorSeverity;
//	NSSTRING  strErrorCategory;
//	NSSTRING  strErrorDescription;
//	short nLogType;
//	int  nErrorID;
//};





