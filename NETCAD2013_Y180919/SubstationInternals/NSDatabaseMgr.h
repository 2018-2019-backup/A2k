/*!
 @Class		:	CNSDatabaseMgr
 @Brief		:	This class provides the functionality for opening and closing database
				connection.
 @Author	:	Neilsoft Ltd
 @Date		:	25-06-2007
 @History 	:	Change history
 */

#pragma once
#include "map"
#include "vector"
#include "tchar.h"
#include "..\common\XPathParser.h"

enum TABLE_TYPE
	{
		TABLE_INVALID = -1,
		TABLE_FEATURE_LIST = 1,
		TABLE_COMP_LIST = 2,
		TABLE_POLE_LIST = 3,
		TABLE_SL_LIST = 4,
		TABLE_SWITCH_LIST = 5,
		TABLE_COLUMN_LIST = 6,
		TABLE_LINE_LIST = 7,
		TABLE_CHILD_LIST = 8,
		TABLE_PARENT_LIST = 9
	};
//typedef std::map<TABLE_TYPE, CNSTable*> TABLE_MAP;

class __declspec(dllexport)CNSDatabaseMgr
{
private:
	//_ConnectionPtr m_pConn; // Connection pointer
	//_RecordsetPtr m_pRecSet; // Recordset pointer
	//TABLE_MAP m_mapOfTable; // map to store all table objects

//public:
//	CNSDatabaseMgr(void);
public:
	CString g_csNetCadDBPath;
	std::vector<std::vector<variant_t>> recordSet;
	std::vector<XPathNS::XMLNode> nodeList;
	struct StringListCompare
{
	 bool operator()(const XPathNS::XMLNode& lhs, const XPathNS::XMLNode& rhs);
	 XPathNS::XPathParser *xPath;
	 std::string strField;
	 std::vector<std::string> strFields;
} StringListComparer;

	struct StringListCompareDist
{
	 bool operator()(const XPathNS::XMLNode& lhs, const XPathNS::XMLNode& rhs);
	 XPathNS::XPathParser *xPath;
	 std::string strField;
	 std::vector<std::string> strFields;
} StringListComparerDist;

	~CNSDatabaseMgr(void);
	static CNSDatabaseMgr* getInstance();
	//int openDatabase(const TCHAR* szUserName, const TCHAR* szPwd, const TCHAR* cmdStr);
	//void closeDatabase();
	/*CNSRecord* getRecord(TABLE_TYPE tableType, int nFNO);*/
	//void setDatabaseFileLoc(const TCHAR *szFileLoc);
	int executeQuery(const TCHAR* szQuery);
	bool getValue(const TCHAR* szValue, variant_t & Holder);
	void getValue(const TCHAR* szValue, std::vector<variant_t> & vValues);
private:
	static CNSDatabaseMgr* pDatabaseMgr;
	CNSDatabaseMgr(void);
};
