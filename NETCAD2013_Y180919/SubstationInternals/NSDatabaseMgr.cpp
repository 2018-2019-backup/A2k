#include "StdAfx.h"
#include "NSDatabaseMgr.h"
#include "NSDirectives.h"

CNSDatabaseMgr* CNSDatabaseMgr::pDatabaseMgr = NULL;
// Constructor
CNSDatabaseMgr::CNSDatabaseMgr(void)
{
	//m_pConn = NULL;
}

// Destructor
CNSDatabaseMgr::~CNSDatabaseMgr(void)
{

}

/*!
 @Brief			:	This function provides the functionality to open
					database using username and password.
 @Param [in]	:	szUserName	-	DSN UserName
 @Param [in]	:	szPwd		-	DSN Password
 @Param [in]	:	szCmdStr	-	DSN command string.
 @Return		:	int	NS_FAIL - Connection not established successfully
						NS_SUCCESS - Connection established successfully.
 @Date			:	25-06-2007
*/

//int CNSDatabaseMgr::openDatabase(const TCHAR* szUserName, const TCHAR* szPwd, const TCHAR* szCmdStr)
//{
//	HRESULT hr = S_OK;
//	try
//	{
//		hr = m_pConn.CreateInstance( __uuidof(Connection));
//		if(hr != S_OK)
//		{
//			return NS_FAIL;
//		}
//		hr = m_pConn->Open(szCmdStr, szUserName, szPwd, -1);
//		if(hr != S_OK)
//		{
//			return NS_FAIL;
//		}
//	}
//	catch(...)
//	{
//		return NS_FAIL;
//	}
//	return NS_SUCCESS;
//}

/*!
 @Brief			:	This function provides the functionality to close
					database.
 @Return		:	 void
 @Date			:	25-06-2007
*/
//void CNSDatabaseMgr::closeDatabase()
//{
//	m_pConn->Close();
//}

/*!
 @Brief			:	This function provides the functionality get the record from
					database for given Feature number.
 @Param [in]	:	TABLE_TYPE	-	Table type enum value.
 @Param [in]	:	nFNO		-	Feature number
 @Return		:	Record which is fetched from the Table 
 @Date			:	25-06-2007
*/
//CNSRecord* CNSDatabaseMgr::getRecord(TABLE_TYPE tableType, int nFNO)
//{
//	CNSTable* pTable = NULL;
//	
//	TABLE_MAP::iterator it = m_mapOfTable.find(TABLE_TYPE(tableType));
//	if(it != m_mapOfTable.end())
//	{
//		pTable = (*it).second;
//	}
//	else
//	{
//		pTable = new CNSTable();
//		pTable->m_pConn = m_pConn;
//		m_mapOfTable.insert(TABLE_MAP::value_type(TABLE_TYPE(tableType), pTable));
//	}
//	return pTable->getRecord(TABLE_TYPE(tableType), nFNO);
//}

//int CNSDatabaseMgr::executeQuery(const TCHAR* szQuery)
//{
//	try
//	{
//		/*HRESULT hr = S_OK;
//		hr = m_pRecSet.CreateInstance(__uuidof(Recordset));
//		if(hr != S_OK)
//			return NS_FAIL;
//		hr = m_pRecSet->Open(szQuery, m_pConn.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText );
//		if(hr != S_OK)
//			return NS_FAIL;*/
//		return NS_SUCCESS;
//	}
//	catch(...)
//	{
//		return -1;
//	}
//}

bool CNSDatabaseMgr::getValue(const TCHAR* szValue, variant_t & Holder)
{
	/*try
	{
		if(!m_pRecSet->adoEOF)
		{
			Holder = m_pRecSet->GetCollect(szValue);
			if(Holder.vt!=VT_NULL)
				return true;
			else
				return false;
		}
		return false;
	}
	catch(...)
	{
	}*/
	try
	{
	if(nodeList.empty())
		return false;
	XPathNS::XMLNode node=nodeList[0];
		  CString exprB= "//";
		  exprB += szValue;
		  std::string expr2=CW2A(exprB.MakeUpper());
		  XPathNS::XPathParser xPath2(node.xml_,false);
		  try
		  {
			XPathNS::XMLNode node2 = xPath2.selectSingleNode(expr2 ); 
			Holder.SetString(node2.value_.c_str());
			return true;
			//row.push_back(node2.value_.c_str());
			//m_pcsaRow->Add((LPCSTR)node2.value_.c_str());
		  }
		  catch(...)
		  {
			  //row.push_back("");
			  //m_pcsaRow->Add("");
		  }
	}
	catch(...)
	{
	}

	return false;
}

void CNSDatabaseMgr::getValue(const TCHAR* szValue, std::vector<variant_t> & vValues)
{
	try
	{
		/*while(!m_pRecSet->adoEOF)
		{
			variant_t Holder = m_pRecSet->GetCollect(szValue);
			if(Holder.vt!=VT_NULL)
			{
				vValues.push_back(Holder);
			}
			m_pRecSet->MoveNext();
		}*/
		 std::vector<XPathNS::XMLNode>::iterator i;
	for(i=nodeList.begin();i!=nodeList.end();i++)
	{
		XPathNS::XMLNode node=*i;
		  CString exprB= "//";
		  exprB += szValue;
		  std::string expr2=CW2A(exprB.MakeUpper());
		  XPathNS::XPathParser xPath2(node.xml_,false);
		  try
		  {
			XPathNS::XMLNode node2 = xPath2.selectSingleNode(expr2 ); 
			variant_t Holder;
			Holder.SetString(node2.value_.c_str());
			vValues.push_back(Holder);
			//row.push_back(node2.value_.c_str());
			//m_pcsaRow->Add((LPCSTR)node2.value_.c_str());
		  }
		  catch(...)
		  {
			  //row.push_back("");
			  //m_pcsaRow->Add("");
		  }
		}
	}
	catch(...)
	{

	}
	
	
}

CNSDatabaseMgr* CNSDatabaseMgr::getInstance()
{
    if(pDatabaseMgr == NULL)
    {
        pDatabaseMgr = new CNSDatabaseMgr();
    }
    return pDatabaseMgr;
}

int CNSDatabaseMgr::executeQuery(const TCHAR* szQuery)
{
	// Reset the number of columns
	CString csSql=szQuery;
	std::string sSelect=CW2A(csSql);
	int nTokenPos=0;
	CString csSelect = csSql;
	CString csWhere,csOrder;

	csSelect.MakeUpper();
	int oPos=csSelect.Find(L"ORDER");
	int wPos=csSelect.Find(L"WHERE");
	
	if(wPos>0 && oPos>wPos)
	{
		csWhere=csSql.Mid(wPos,oPos-wPos);
		csSelect=csSql.Left(wPos);
		csOrder=csSql.Right(csSql.GetLength()-oPos);
	}
	if(wPos<0 && oPos>0)
	{
		csSelect=csSql.Left(oPos);
		csOrder=csSql.Right(csSql.GetLength()-oPos);
	}
	if(wPos>0 && oPos<0)
	{
		csWhere=csSql.Right(csSql.GetLength()-wPos);
		csSelect=csSql.Left(wPos);
	}
	if(wPos<0 && oPos<0)
		csSelect=csSql;

	csSql = csSelect+csOrder;

  int m_iColums = -1;    
    //csSql.Format(_T("SELECT [Depot], [Address1], [Address2], [Phone], [Fax], [Mobile] FROM tblAddresses ORDER BY [Depot]"));
   nTokenPos = 0;
	CString strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
	CString strStatus="SELECT";
	CStringArray fieldNames;
	CStringArray orderByNames;
	CStringArray whereByExpr;
	CString tableName;
	CString str;
	bool distFlag=false;
	while (!strToken.IsEmpty())
	{
		strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
			if(strToken.IsEmpty())
				break;
			if((str=strToken).MakeUpper()=="FROM")
			{
				strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
				strStatus="FROM";
			}
						
			if((str=strToken).MakeUpper()=="ORDER")
			{
				strToken = csSql.Tokenize(L" " L"BY" L"by" L"[" L"]" L",", nTokenPos);
				strStatus="ORDER";
			}

			if((str=strToken).MakeUpper()=="WHERE")
			{
				strToken = csSql.Tokenize(L" " L"[" L"]" L",", nTokenPos);
				strStatus="WHERE";
			}
			
			if(strStatus.MakeUpper()=="SELECT")
			{
				if((str=strToken).MakeUpper()!="DISTINCT")
					fieldNames.Add(strToken.MakeUpper());
				else
					distFlag=true;
			}
			if(strStatus.MakeUpper()=="FROM")
			{
				tableName=strToken;
			}
				
			if(strStatus.MakeUpper()=="ORDER")
			{
				orderByNames.Add(strToken.MakeUpper());
			}

			if(strStatus.MakeUpper()=="WHERE")
			{
				if((str=strToken).MakeUpper()=="AND" || (str=strToken).MakeUpper()=="OR")
					strToken.MakeLower();
				if(strToken=="<>")
					strToken="!=";
				whereByExpr.Add(strToken);
			}
	}
	
	//CString g_csNetCadDBPath="C:\\Program Files\\NET CAD STANDARD\\NET CAD\\NetCADDB";
	CString fileName=g_csNetCadDBPath;
	fileName = fileName + "\\" + tableName +".xml";
	std::string fName= CW2A(fileName);

	nodeList.clear();
	try
	{
		XPathNS::XPathParser xPath(fName,true);// "c:\\tblAddresses.xml",true   );//
		// // Get the field count
		m_iColums = fieldNames.GetCount();
		
		//SELECT
		CString exprA= "//";
		exprA += tableName;
		CString whereExpr="";
		if(wPos>0)
		{
			whereExpr = whereExpr+"[" + csWhere.Right(csWhere.GetLength()-6)+"]"; 
			whereExpr.Replace(L"<>",L"!=");
			exprA+=whereExpr;
		}
		//WHERE
		/*if(whereByExpr.GetCount()>0)
		{
			exprA +="[";
			for(int i=0;i<whereByExpr.GetCount();i++)
			{
				exprA += whereByExpr.GetAt(i);
				exprA +=" ";
			}
			exprA +="]";
		}*/
		std::string expr1=CW2A(exprA);
		nodeList = xPath.selectNodes(expr1 ); // drill down and select all author elements

		//DISTINCT
		if(distFlag)
		{
			//Order by feilds
			StringListComparer.strFields.clear();
			for(int i=0;i<fieldNames.GetCount();i++)
			{
				std::string str=CW2A(fieldNames[i]);
				StringListComparer.strFields.push_back(str);
			}
			std::sort(nodeList.begin(), nodeList.end(), StringListComparer);
			//Erase distinct records
			StringListComparerDist.strFields.clear();
			for(int i=0;i<fieldNames.GetCount();i++)
			{
				std::string str=CW2A(fieldNames[i]);
				StringListComparerDist.strFields.push_back(str);
			}
			nodeList.erase(std::unique(nodeList.begin(),nodeList.end(),StringListComparerDist),nodeList.end());
		}
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
  /*int m_iRows=nodeList.size();
  std::vector<XPathNS::XMLNode>::iterator i;
  if(!recordSet.empty())
  {
		//remove all - Clear
		recordSet.clear();
  }
  //RemoveAll();
	for(i=nodeList.begin();i!=nodeList.end();i++)
	{
	XPathNS::XMLNode node=*i;
	std::vector<variant_t> row;
	//m_pcsaRow = new CStringArray;
		for(int j=0;j<m_iColums;j++)
		{
		  CString exprB= "//";
		  exprB += fieldNames.GetAt(j);
		  std::string expr2=CW2A(exprB.MakeUpper());
		  XPathNS::XPathParser xPath2(node.xml_,false);
		  try
		  {
			XPathNS::XMLNode node2 = xPath2.selectSingleNode(expr2 ); 
			row.push_back(node2.value_.c_str());
			//m_pcsaRow->Add((LPCSTR)node2.value_.c_str());
		  }
		  catch(...)
		  {
			  row.push_back("");
			  //m_pcsaRow->Add("");
		  }
		  
		}
recordSet.push_back(row);
	//Add(m_pcsaRow);
	}*/

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
 // m_csCurFile = csFileName;
  //m_iCurLine  = iLineNumb;



  // Return success
	return NS_SUCCESS;
}
bool CNSDatabaseMgr::StringListCompare::operator()(const XPathNS::XMLNode& lhs, const XPathNS::XMLNode& rhs)
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

bool CNSDatabaseMgr::StringListCompareDist::operator()(const XPathNS::XMLNode& lhs, const XPathNS::XMLNode& rhs)
{
	bool flag=true;
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
				flag = flag &  (lhsNode.value_==rhsNode.value_);
		}
	}
	catch(...)
	{
	}

	return flag;
}