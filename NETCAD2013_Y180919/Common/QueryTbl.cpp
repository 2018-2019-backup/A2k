/*------------------------------------------------------------------------------------------------------
|  Created   :  3-9-2002   8:31
|  Filename  :  \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  File path :  \\DCSSERVER\PROJECTS\SPACE2K\Input
|  File base :  QueryTbl
|  File ext  :  cpp
|  Author    :  Rakesh S. S
|	
|  Purpose   :  To reduce the codes to be written for queries
|------------------------------------------------------------------------------------------------------*/

// QueryTbl.cpp: implementation of the CQueryTbl class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "QueryTbl.h"
//#import <msxml6.dll>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CString ResetSpaces(CString csSQL)
{
  // Make a copy of the string
  CString csSQLCopy = csSQL;

  // Repeatedly replace all double spaces with a single space
  csSQLCopy.Replace(_T("  "), _T(" "));
  csSQLCopy.Replace(_T("  "), _T(" "));
  csSQLCopy.Replace(_T("  "), _T(" "));
  csSQLCopy.Replace(_T("  "), _T(" "));
  csSQLCopy.Replace(_T("  "), _T(" "));
  csSQLCopy.Replace(_T("  "), _T(" "));

  // Return the copied and un-double-spaced string
  return csSQLCopy;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CQueryTbl::LogSqlErr(CString csDsn, CString csSql, CString csFileName, CString csFuncName, int iLineNumb, CString csErr)
{
}

CQueryTbl::CQueryTbl()
{
  m_iColums = -1;
  m_pcsaRow = NULL;
  m_iCurLine = 0;
  m_csCurFile = _T("");
  m_csDatabasePassword = _T("");
}

CQueryTbl::~CQueryTbl()
{
  Close();
}


/*-------------------------------------------------------------------------------------------------------
|  Function    : ClearPointersMemory
|  Created     : 3-9-2002   8:37
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : 
|  Returns     : Nothing
|  Description : To Clear memory alocated by the pointers stored in this array
|-------------------------------------------------------------------------------------------------------*/
void CQueryTbl::Close()
{
  for (int iCount = GetUpperBound(); iCount > -1; iCount--)
  {
    m_pcsaRow = (CStringArray *)GetAt(iCount);
    m_pcsaRow->RemoveAll();
    RemoveAt(iCount);
    delete m_pcsaRow;
  }
}


/*-------------------------------------------------------------------------------------------------------
|  Function    : SqlExecute
|  Created     : 3-9-2002   8:43
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : 
|  Returns     : Nothing
|  Description : Executes an Sql without returning anything
|-------------------------------------------------------------------------------------------------------*/
BOOL CQueryTbl::SqlExecute(CString csDsn, CString csSql, int iLineNumb, CString csFileName, CString csFunctionName)
{
  // Close any existing connection
  Close();

  // Reset the number of columns
  m_iColums = -1;    

  // Open the database
  CDatabase cdDSN;
  try { cdDSN.Open(csDsn, FALSE, FALSE, _T("ODBC;UID=;PWD=" + m_csDatabasePassword)); } 
  catch (CDBException *e) 
  { 
    LogSqlErr(csDsn, csSql, csFileName, csFunctionName, iLineNumb, e->m_strError); 
    //appError(csFileName, csFunctionName, iLineNumb, e->m_strError + "\n\n" + ResetSpaces(csSql)); 
    appError(csFileName, csFunctionName, iLineNumb, e->m_strError); 
    return FALSE;
  }
  
  // Execute the sql
  try { cdDSN.ExecuteSQL(csSql); } 
  catch (CDBException *e) 
  { 
    cdDSN.Close();
    LogSqlErr(csDsn, csSql, csFileName, csFunctionName, iLineNumb, e->m_strError); 
    //appError(csFileName, csFunctionName, iLineNumb, e->m_strError + "\n\n" + ResetSpaces(csSql)); 
    appError(csFileName, csFunctionName, iLineNumb, e->m_strError); 
    return FALSE;
  }

  // Close that connection
  cdDSN.Close();

  // Return success
  return TRUE;
}


/*-------------------------------------------------------------------------------------------------------
|  Function    : SqlRead
|  Created     : 3-9-2002   8:43
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : 
|  Returns     : Nothing
|  Description : Executes and returns records read by it
|-------------------------------------------------------------------------------------------------------*/
BOOL CQueryTbl::SqlRead(CString csDsn, CString csSql, int iLineNumb, CString csFileName, CString csFunctionName)
{
	// Close any existing connection
  Close();

  // Reset the number of columns
  m_iColums = -1;    

  // Open the database
  CDatabase cdDSN;
  try { cdDSN.Open(csDsn, FALSE, TRUE, _T( "ODBC;UID=;PWD=" + m_csDatabasePassword), FALSE); } 
  catch (CDBException *e) 
  {
    LogSqlErr(csDsn, csSql, csFileName, csFunctionName, iLineNumb, e->m_strError); 
    appError(csFileName, csFunctionName, iLineNumb, e->m_strError + _T("\n\n") + ResetSpaces(csSql)); 
    return FALSE;
  }

	// Open the table with the given SQL statement
  CRecordset crRecords(&cdDSN);
  try { crRecords.Open(CRecordset::snapshot, csSql); } 
  catch (CDBException *e) 
  {
    cdDSN.Close();
    LogSqlErr(csDsn, csSql, csFileName, csFunctionName, iLineNumb, e->m_strError); 
    appError(csFileName, csFunctionName, iLineNumb, e->m_strError + _T("\n\n") + ResetSpaces(csSql)); 
    return FALSE;
  }

  // If no records resulted, close the table and return now
  if (crRecords.IsBOF())
  {
    crRecords.Close();
    cdDSN.Close();
    return TRUE;
  }

  // Get the field count
  m_iColums = crRecords.GetODBCFieldCount();
  
  // Retrieve the field values into the row arrays
  for (crRecords.MoveFirst(); !crRecords.IsEOF(); crRecords.MoveNext())
  {
    m_pcsaRow = new CStringArray;
    for (short iC = 0; iC < m_iColums; iC++)
    {
      crRecords.GetFieldValue(iC, m_csTemp);
      m_pcsaRow->Add(m_csTemp);
    }
    Add(m_pcsaRow);
  }

  // Store the file name and the line number
  m_csCurFile = csFileName;
  m_iCurLine  = iLineNumb;

  // Close the recordset
  crRecords.Close();

  // Close the database
  cdDSN.Close();

  // Return success
  return TRUE;
}

#if defined _TRANSLATOR
using namespace XPathNS;
BOOL CQueryTbl::SqlRead(CString csDsn, CString csSql, int iLineNumb, CString csFileName, CString csFunctionName, bool xml)
{
	return xmlRead(csDsn,  csSql,  iLineNumb,  csFileName,  csFunctionName);
}

BOOL CQueryTbl::xmlRead(CString csDsn, CString csSql, int iLineNumb, CString csFileName, CString csFunctionName)
{
	// Reset the number of columns
	 RemoveAll();
  m_iColums = -1;    
    //csSql.Format(_T("SELECT [Depot], [Address1], [Address2], [Phone], [Fax], [Mobile] FROM tblAddresses ORDER BY [Depot]"));
  int nTokenPos = 0;
	CString strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
	CString strStatus="SELECT";
	CStringArray fieldNames;
	CStringArray orderByNames;
	CStringArray whereByExpr;
	CString tableName;
	while (!strToken.IsEmpty())
	{
			/*strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
			if(strToken=="FROM")
			{
				strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
				tableName=strToken;
				break;
			}
			else
			{
				fieldNames.Add(strToken);
			}*/
		strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
			if(strToken.IsEmpty())
				break;
			if(strToken=="FROM")
			{
				strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
				strStatus="FROM";
			}
						
			if(strToken=="ORDER")
			{
				strToken = csSql.Tokenize(L" " L"BY" L"[" L"]" L",", nTokenPos);
				strStatus="ORDER";
			}

			if(strToken=="WHERE")
			{
				strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
				strStatus="WHERE";
			}

			
			if(strStatus=="SELECT")
			{
				if(strToken!="DISTINCT")
					fieldNames.Add(strToken);
			}
			if(strStatus=="FROM")
			{
				tableName=strToken;
			}
				
			if(strStatus=="ORDER")
			{
				orderByNames.Add(strToken);
			}

			if(strStatus=="WHERE")
			{
				if(strToken=="AND" || strToken=="OR")
					strToken.MakeLower();
				if(strToken=="<>")
					strToken="!=";
				whereByExpr.Add(strToken);
			}
	}
	
	//CString g_csNetCadDBPath="C:\\Program Files\\NET CAD STANDARD\\NET CAD\\NetCADDB";
	CString fileName;
	fileName = g_csNetCadDBPath + "\\" + tableName +".xml";
	std::string fName= CW2A(fileName);

	std::vector<XPathNS::XMLNode> nodeList;
	try
	{
		XPathNS::XPathParser xPath(fName,true);// "c:\\tblAddresses.xml",true   );//
		// // Get the field count
		m_iColums = fieldNames.GetCount();
		
		//SELECT
		CString exprA= "//";
		exprA += tableName;
		//WHERE
		if(whereByExpr.GetCount()>0)
		{
			exprA +="[";
			for(int i=0;i<whereByExpr.GetCount();i++)
			{
				exprA += whereByExpr.GetAt(i);
				exprA +=" ";
			}
			exprA +="]";
		}
		std::string expr1=CW2A(exprA);

		//std::vector<XPathNS::XMLNode> *tempNodeList = static_cast<std::vector<XPathNS::XMLNode>> (xPath.selectNodes(expr1 )); // drill down and select all author elements
		nodeList = xPath.selectNodes(expr1 ); // drill down and select all author elements
		
		if(nodeList.empty())
			return TRUE;

		//nodeList=tempNodeList;
		//ORDER BY
		if(orderByNames.GetCount()>0)
		{
			StringListComparer.strFields.clear();
			for(int i=0;i<orderByNames.GetCount();i++)
			{
				/*StringListComparer.strField = "//";
				StringListComparer.strField += CW2A(orderByNames[i]);
				std::sort(nodeList.begin(), nodeList.end(), StringListComparer);*/
				std::string str=CW2A(orderByNames[i]);
				StringListComparer.strFields.push_back(str);
			}
			std::sort(nodeList.begin(), nodeList.end(), StringListComparer);
		}
	}
  catch(...)
  {
	  return FALSE;
  }
  int m_iRows=nodeList.size();
  std::vector<XPathNS::XMLNode>::iterator i;
 
	for(i=nodeList.begin();i!=nodeList.end();i++)
	{
	XMLNode node=*i;
	m_pcsaRow = new CStringArray;

		for(int j=0;j<m_iColums;j++)
		{
		  CString exprB= "//";
		  exprB += fieldNames.GetAt(j);
		  std::string expr2=CW2A(exprB);
		  XPathNS::XPathParser xPath2(node.xml_,false);
		  try
		  {
			XMLNode node2 = xPath2.selectSingleNode(expr2 ); 
			m_pcsaRow->Add((LPCSTR)node2.value_.c_str());
		  }
		  catch(...)
		  {
			  m_pcsaRow->Add("");
		  }
		  
		}
	Add(m_pcsaRow);
	}
  //
  //// Retrieve the field values into the row arrays
  //for (crRecords.MoveFirst(); !crRecords.IsEOF(); crRecords.MoveNext())
  //{
  //  m_pcsaRow = new CStringArray;
  //  for (short iC = 0; iC < m_iColums; iC++)
  //  {
  //    crRecords.GetFieldValue(iC, m_csTemp);
  //    m_pcsaRow->Add(m_csTemp);
  //  }
  //  Add(m_pcsaRow);
  //}

  // Store the file name and the line number
  m_csCurFile = csFileName;
  m_iCurLine  = iLineNumb;



  // Return success
	return TRUE;
}
bool CQueryTbl::StringListCompare::operator()(const XPathNS::XMLNode& lhs, const XPathNS::XMLNode& rhs)
{
	/*XPathNS::XPathParser xPath1(lhs.xml_,false);
	XPathNS::XPathParser xPath2(rhs.xml_,false);
		XPathNS::XMLNode nodeList1 = xPath1.selectSingleNode(strField); // select all author elements
		XPathNS::XMLNode nodeList2 = xPath2.selectSingleNode( strField ); // select all author elements
		return nodeList1.value_< nodeList2.value_;*/
	try

	{
	XPathNS::XPathParser xPath1(lhs.xml_,false);
	XPathNS::XPathParser xPath2(rhs.xml_,false);
	std::vector<std::string>::iterator i;
	    for(i=strFields.begin();i!=strFields.end();i++)
		{
			std::string str="//" + *i;
			XPathNS::XMLNode lhsNode = xPath1.selectSingleNode(str); // select all author elements
			XPathNS::XMLNode rhsNode = xPath2.selectSingleNode(str); // select all author elements
			if(lhsNode.value_==rhsNode.value_)
			{
			}
			else
			{
				return lhsNode.value_< rhsNode.value_;
			}
		}
	}
	catch(...)
	{
	}

	return false;
}

#endif
//--------------------------------------------------------------------------------------------------------
//-Utility functions


/*-------------------------------------------------------------------------------------------------------
|  Function    : GetRows
|  Created     : 3-9-2002   9:40
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : Caller
|  Returns     : Nothing
|  Description : To get the number of rows fetched
|-------------------------------------------------------------------------------------------------------*/
int CQueryTbl::GetRows()
{
  return GetSize();
}

/*-------------------------------------------------------------------------------------------------------
|  Function    : GetColumns
|  Created     : 3-9-2002   9:41
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : Caller
|  Returns     : Nothing
|  Description : To get the number of columns in the result
|-------------------------------------------------------------------------------------------------------*/
int CQueryTbl::GetColumns()
{
  return m_iColums;
}

/*-------------------------------------------------------------------------------------------------------
|  Function    : GetRowAt
|  Created     : 3-9-2002   9:43
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : 
|  Returns     : Nothing
|  Description : To get the data in the column and index
|-------------------------------------------------------------------------------------------------------*/
CStringArray* CQueryTbl::GetRowAt(int iRow)
{
  if ( (iRow >= GetSize() ) || (iRow < 0))
  {
    CString csLine;
    csLine.Format(_T("%d"), iRow);
    appError(m_csCurFile, _T("GetRowAt"), m_iCurLine, _T("Invalid row index [") + csLine + _T("] provided for function"));
    return NULL;
  }

  return (CStringArray*)GetAt(iRow);
}


/*-------------------------------------------------------------------------------------------------------
|  Function    : GetColumnAt
|  Created     : 3-9-2002   9:50
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : 
|  Returns     : Nothing
|  Description : To get the entire column in a string array
|-------------------------------------------------------------------------------------------------------*/
void CQueryTbl::GetColumnAt(int iColumn, CStringArray &csaColumn)
{
  if ( (iColumn >= m_iColums) || (iColumn < 0))
  {
    appError(__FILE__, _T("GetColumnAt"), __LINE__, _T("Invalid column index provided for function"));
    return;
  }
  
  // First Clear all the data
  csaColumn.RemoveAll();

  for (int iCount = 0; iCount < GetSize(); iCount++)
  {
    m_pcsaRow = (CStringArray *) GetAt(iCount);
    csaColumn.Add(m_pcsaRow->GetAt(iColumn));
  }
  
}

/*-------------------------------------------------------------------------------------------------------
|  Function    : Open
|  Created     : 13-9-2002   11:55
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : 
|  Returns     : Nothing
|  Description : Opens a database in the member database object
|-------------------------------------------------------------------------------------------------------*/
BOOL CQueryTbl::Open(CString csDsn, int iLineNumb, CString csFileName, CString csFunctionName)
{ 
  if (m_cdDsn.IsOpen())
  {
    appError(csFileName, csFunctionName, iLineNumb, _T("Database already opened")); 
    return FALSE;
  }

  // OPEN THE DSN
  try {m_cdDsn.Open(csDsn, FALSE, FALSE, _T("ODBC;UID=;PWD=" + m_csDatabasePassword) );} catch (CDBException *e) {LogSqlErr(csDsn, _T("OpenErr"), csFileName, csFunctionName, iLineNumb, e->m_strError); appError(csFileName, csFunctionName, iLineNumb, e->m_strError); return FALSE ;}
  return TRUE;
}

/*-------------------------------------------------------------------------------------------------------
|  Function    : ExecuteSql
|  Created     : 13-9-2002   11:55
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : 
|  Returns     : Nothing
|  Description : Excecute the sql in member database object
|-------------------------------------------------------------------------------------------------------*/
BOOL CQueryTbl::ExecuteSql(CString csSql, int iLineNumb, CString csFileName, CString csFunctionName)
{
  if (!m_cdDsn.IsOpen())
  {
    appError(csFileName, csFunctionName, iLineNumb, _T("Data base not opened")); 
    return FALSE;
  }

  // OPEN THE DSN
  try {m_cdDsn.ExecuteSQL(csSql);} catch (CDBException *e) {LogSqlErr(_T("Already opened"), csSql, csFileName, csFunctionName, iLineNumb, e->m_strError); appError(csFileName, csFunctionName, iLineNumb, e->m_strError); return FALSE ;}
  return TRUE;
}

/*-------------------------------------------------------------------------------------------------------
|  Function    : CloseDatabase
|  Created     : 13-9-2002   11:54
|  Filename    : \\DCSSERVER\PROJECTS\SPACE2K\Input\QueryTbl.cpp
|  Author      : Rakesh S. S
|  Arguments   : Nill
|  Called from : 
|  Returns     : Nothing
|  Description : Close the member database object
|-------------------------------------------------------------------------------------------------------*/
BOOL CQueryTbl::CloseDatabase()
{
  if (!m_cdDsn.IsOpen())
  {
    AfxMessageBox(_T("Data base not opened")); 
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------------------------------------------------
//public functions
void CQueryTbl::RemoveAt(int nIndex, int nCount)
{
  CPtrArray::RemoveAt(nIndex, nCount);
}

void* CQueryTbl::GetAt(int nIndex) const
{
  return CPtrArray::GetAt(nIndex);
}

int CQueryTbl::Add(void* newElement)
{
  return CPtrArray::Add(newElement);
}

void CQueryTbl::RemoveAll()
{
  CPtrArray::RemoveAll();
}
