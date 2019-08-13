#include "StdAfx.h"
#include "SubstationInfo.h"
#include "atlbase.h"

CSubstationInfo::CSubstationInfo(void)
{
	m_strLabel = "Substation Details";
}

CSubstationInfo::~CSubstationInfo(void)
{
}
void CSubstationInfo::getType(CString &strType)
{
	strType = m_strType;
}
bool CSubstationInfo::setType(const CString &strType)
{
	if(m_strType == strType) return false;
	m_strType = strType;
	return true;
}
void CSubstationInfo::getSize(CString &strSize)
{
	strSize = m_strSize;
}
bool CSubstationInfo::setSize(const CString &strSize)
{
	if(m_strType.IsEmpty()&& m_strSize.IsEmpty() || m_strSize == strSize) return false;
	m_strSize = strSize;
	return true;
}
void CSubstationInfo::getDescription(CString &strDesc)
{
	strDesc = m_strDescription;
}
bool CSubstationInfo::setDescription(const CString &strDesc)
{
	if(m_strDescription == strDesc) return false;
	m_strDescription = strDesc;
	return true;
}


void CSubstationInfo::getStockType(CString &strStockType)
{
	strStockType = m_strStockType;
}
bool CSubstationInfo::setStockType(const CString &strStockType)
{
	if(m_strStockType == strStockType) return false;
	m_strStockType = strStockType;
	return true;
}
void CSubstationInfo::getLabel(CString &strLabel)
{
	strLabel = m_strLabel;
}
bool CSubstationInfo::setLabel(const CString &strLabel)
{
	if(m_strLabel == strLabel) return false;
	m_strLabel = strLabel;
	return true;
}
void CSubstationInfo::getFunction(CString &strFunction)
{
	strFunction = m_strFunction;
}
bool CSubstationInfo::setFunction(const CString &strFunction)
{
	if(m_strFunction == strFunction) return false;
	m_strFunction = strFunction;
	return true;
}
void CSubstationInfo::getPrefix(CString &strPrefix)
{
	strPrefix = m_strPrefix;
}
bool CSubstationInfo::setPrefix(const CString &strPrefix)
{
	if(m_strPrefix == strPrefix) return false;
	m_strPrefix = strPrefix;
	return true;
}
void CSubstationInfo::getName(CString &strName)
{
	strName = m_strName;
}
bool CSubstationInfo::setName(const CString &strName)
{
	if(m_strName == strName) return false;
	m_strName = strName;
	return true;
}
void CSubstationInfo::getNumber(CString &strNumber)
{
	strNumber = m_strNumber;
}
bool CSubstationInfo::setNumber(const CString &strNumber)
{
	if(m_strNumber == strNumber) return false;
	m_strNumber = strNumber;
	return true;
}
void CSubstationInfo::getOptions(CString &strOptions)
{
	strOptions = m_strOptions;
}
bool CSubstationInfo::setOptions(const CString &strOptions)
{
	if(m_strOptions == strOptions) return false;
	m_strOptions = strOptions;
	return true;
}
void CSubstationInfo::setInfoToRegistry(HKEY hKey)
{
	CRegKey rkeySUBSTN;
	rkeySUBSTN.Attach(hKey);
	rkeySUBSTN.SetStringValue( _T("SSType"), m_strType );
	rkeySUBSTN.SetStringValue( _T("SSSize"), m_strSize);
	rkeySUBSTN.SetStringValue( _T("SSDescription"), m_strDescription);
	rkeySUBSTN.SetStringValue( _T("SSFunction"), m_strFunction);
	rkeySUBSTN.SetStringValue( _T("SSLabel"), m_strLabel);
	rkeySUBSTN.SetStringValue( _T("SSName"), m_strName);
	rkeySUBSTN.SetStringValue( _T("SSNumber"), m_strNumber);
	rkeySUBSTN.SetStringValue( _T("SSOptions"), m_strOptions);
	rkeySUBSTN.SetStringValue( _T("SSPrefix"), m_strPrefix);
	rkeySUBSTN.SetStringValue( _T("SSStockType"), m_strStockType);
	rkeySUBSTN.Detach();
}
void CSubstationInfo::loadDataFromRegistry(HKEY hKey)
{
	CRegKey rkeySUBSTN;
	TCHAR szBuffer[256];
	ULONG nChars = 256;
	rkeySUBSTN.Attach(hKey);
	rkeySUBSTN.QueryStringValue( _T("SSType"), szBuffer,&nChars);
	m_strType = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSSize"),szBuffer,&nChars);
	m_strSize = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSDescription"), szBuffer,&nChars);
	m_strDescription = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSFunction"), szBuffer,&nChars);
	m_strFunction = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSLabel"), szBuffer,&nChars);
	m_strLabel = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSName"), szBuffer,&nChars);
	m_strName = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSNumber"), szBuffer,&nChars);
	m_strNumber = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSOptions"), szBuffer,&nChars);
	m_strOptions = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSPrefix"), szBuffer,&nChars);
	m_strPrefix = szBuffer;
	nChars = 256;
	rkeySUBSTN.QueryStringValue( _T("SSStockType"), szBuffer,&nChars);
	m_strStockType = szBuffer;
	nChars = 256;
	rkeySUBSTN.Detach();
}

