// QueryTbl.h: interface for the CQueryTbl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUERYTBL_H__52B15337_20A4_4EB6_AB33_7CD60E7D4A33__INCLUDED_)
#define AFX_QUERYTBL_H__52B15337_20A4_4EB6_AB33_7CD60E7D4A33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined _TRANSLATOR
#include "XPathParser.h"
#endif

class CQueryTbl : public CPtrArray  
{
public: 
  CString m_csDatabasePassword;
  //--------------------------------------------------------------------------------------------------------
  // Addons
  BOOL SqlExecute(CString csDsn, CString csSql, int iLineNumb, CString csFileName, CString csFunctionName); // Executes an Sql without returning anything
  BOOL SqlRead    (CString csDsn, CString csSql, int iLineNumb, CString csFileName, CString csFunctionName); // Executes and returns records read by it
#if defined _TRANSLATOR
  BOOL SqlRead    (CString csDsn, CString csSql, int iLineNumb, CString csFileName, CString csFunctionName, bool xml); // Executes and returns records read by it
  BOOL xmlRead    (CString csDsn, CString csSql, int iLineNumb, CString csFileName, CString csFunctionName); // Executes and returns records read by it
  struct StringListCompare
{
	 bool operator()(const XPathNS::XMLNode& lhs, const XPathNS::XMLNode& rhs);
	 XPathNS::XPathParser *xPath;
	 std::string strField;
	 std::vector<std::string> strFields;
} StringListComparer;
#endif
  int GetRows();                        // Get the number of rows fetched
  int GetColumns();                     // Get the number of columns fetched
  CStringArray *GetRowAt(int iRow);     // Get the row at the given index
  void GetColumnAt(int iColumn, CStringArray &csaColumn);  // returns the column data in the array
  void Close();                         // Clear the pointers memory used
  //--------------------------------------------------------------------------------------------------------
  void LogSqlErr(CString csDsn, CString csSql, CString csFileName, CString csFuncName, int iLineNumb, CString csErr);

  BOOL Open(CString csDsn, int iLineNumb, CString csFileName, CString csFunctionName);
  BOOL ExecuteSql(CString csSql, int iLineNumb, CString csFileName, CString csFunctionName);
  BOOL CloseDatabase();
private:

  int m_iColums;
  CDatabase m_cdDsn;  
  CStringArray *m_pcsaRow;
  CString m_csTemp;
//--------------------------------------------------------------------------------------------------------
  void SetSize(int nNewSize, int nGrowBy = -1);

  // Operations
	// Clean up
	void FreeExtra();
	

	// Accessing elements
	void* GetAt(int nIndex) const;
	void SetAt(int nIndex, void* newElement);

	void*& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const void** GetData() const;
	void** GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, void* newElement);

	int Add(void* newElement);

	int Append(const CPtrArray& src);
	void Copy(const CPtrArray& src);

	// overloaded operator helpers
	void* operator[](int nIndex) const;
	void*& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, void* newElement, int nCount = 1);

	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CPtrArray* pNewArray);
public:
	void RemoveAll();
	
//--------------------------------------------------------------------------------------------------------
protected:
  CString m_csCurFile;
  int m_iCurLine;

public:
	CQueryTbl();
	virtual ~CQueryTbl();
};

#endif // !defined(AFX_QUERYTBL_H__52B15337_20A4_4EB6_AB33_7CD60E7D4A33__INCLUDED_)
