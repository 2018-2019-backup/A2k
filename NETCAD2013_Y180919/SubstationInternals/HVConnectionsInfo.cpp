#include "StdAfx.h"
#include "HVConnectionsInfo.h"
#include "atlbase.h"

CHVConnectionsInfo::CHVConnectionsInfo(void)
{
}

CHVConnectionsInfo::~CHVConnectionsInfo(void)
{
}

void CHVConnectionsInfo::getType(CString &strType)
{
	strType = m_strType;
}
bool CHVConnectionsInfo::setType(const CString &strType)
{
	if(m_strType == strType) return false;
	m_strType = strType;
	return true;
}
void CHVConnectionsInfo::getEquipment(CString &strEquipment)
{
	strEquipment = m_strEquipment;
}
bool CHVConnectionsInfo::setEquipment(const CString &strEquipment)
{
	if(m_strEquipment == strEquipment) return false;
	m_strEquipment = strEquipment;
	return true;
}
void CHVConnectionsInfo::getRating(CString &strRating)
{
	strRating = m_strRating;
}
bool CHVConnectionsInfo::setRating(const CString &strRating)
{
	if(m_strRating == strRating) return false;
	m_strRating = strRating;
	return true;
}

int CHVConnectionsInfo::getEFIA()
{
	return m_iEFIA;
}
bool CHVConnectionsInfo::setEFIA(const int &iEFIA)
{
	if(m_iEFIA == iEFIA) return false;
	m_iEFIA = iEFIA;
	return true;
}
int CHVConnectionsInfo::getEFIB()
{
	return m_iEFIB;
}
bool CHVConnectionsInfo::setEFIB(const int &iEFIB)
{
	if(m_iEFIB == iEFIB) return false;
	m_iEFIB = iEFIB;
	return true;
}

void CHVConnectionsInfo::getSideA(CString &strSideA)
{
	strSideA = m_strSideA;
}
bool CHVConnectionsInfo::setSideA(const CString &strSideA)
{
	if(m_strSideA == strSideA) return false;
	m_strSideA = strSideA;
	return true;
}
void CHVConnectionsInfo::getSideB(CString &strSideB)
{
	strSideB = m_strSideB;
}
bool CHVConnectionsInfo::setSideB(const CString &strSideB)
{
	if(m_strSideB == strSideB) return false;
	m_strSideB = strSideB;
	return true;
}
void CHVConnectionsInfo::getNumber(CString &strNumber)
{
	strNumber = m_strNumber;
}
bool CHVConnectionsInfo::setNumber(const CString &strNumber)
{
	if(m_strNumber == strNumber) return false;
	m_strNumber = strNumber;
	return true;
}
void CHVConnectionsInfo::setInfoToRegistry(HKEY hKey)
{
	CRegKey rkeyHVC;
	CString strVal;
	rkeyHVC.Attach(hKey);
	rkeyHVC.SetStringValue( _T("HVCEquip"), m_strEquipment );
	rkeyHVC.SetStringValue( _T("HVCType"), m_strType);
	rkeyHVC.SetStringValue( _T("HVCRating"), m_strRating);
	rkeyHVC.SetStringValue( _T("HVCNumber"), m_strNumber);
	rkeyHVC.SetStringValue( _T("HVCSideA"), m_strSideA);
	rkeyHVC.SetStringValue( _T("HVCSideB"), m_strSideB);
	strVal.FormatMessage(L"%1!d!",m_iEFIA);
	rkeyHVC.SetStringValue( _T("HVCEfiA"), strVal);
	strVal.FormatMessage(L"%1!d!",m_iEFIB);
	rkeyHVC.SetStringValue( _T("HVCEfiB"), strVal);
	rkeyHVC.Detach();
}
void CHVConnectionsInfo::loadDataFromRegistry(HKEY hKey)
{
	CRegKey rkeyHVC;
	TCHAR szBuffer[256];
	ULONG nChars = 256;
	rkeyHVC.Attach(hKey);
	rkeyHVC.QueryStringValue( _T("HVCEquip"), szBuffer,&nChars);
	m_strEquipment = szBuffer;
	nChars = 256;
	rkeyHVC.QueryStringValue( _T("HVCType"),szBuffer,&nChars);
	m_strType = szBuffer;
	nChars = 256;
	rkeyHVC.QueryStringValue( _T("HVCRating"), szBuffer,&nChars);
	m_strRating = szBuffer;
	nChars = 256;
	rkeyHVC.QueryStringValue( _T("HVCNumber"), szBuffer,&nChars);
	m_strNumber = szBuffer;
	nChars = 256;
	rkeyHVC.QueryStringValue( _T("HVCSideA"), szBuffer,&nChars);
	m_strSideA = szBuffer;
	nChars = 256;
	rkeyHVC.QueryStringValue( _T("HVCSideB"), szBuffer,&nChars);
	m_strSideB = szBuffer;
	nChars = 256;
	rkeyHVC.QueryStringValue( _T("HVCEfiA"), szBuffer,&nChars);
	m_iEFIA = _wtoi(szBuffer);
	nChars = 256;
	rkeyHVC.QueryStringValue( _T("HVCEfiB"), szBuffer,&nChars);
	m_iEFIB = _wtoi(szBuffer);
	rkeyHVC.Detach();
}