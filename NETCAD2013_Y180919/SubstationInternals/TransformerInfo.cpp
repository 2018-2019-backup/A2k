#include "StdAfx.h"
#include "TransformerInfo.h"
#include "atlbase.h"

CTransformerInfo::CTransformerInfo(void)
{
	m_iPhases = -1;
	m_iTapSetting = -1;
}

CTransformerInfo::~CTransformerInfo(void)
{
}

void CTransformerInfo::getRating(CString &strRating)
{
	strRating = m_strRating;
}
bool CTransformerInfo::setRating(const CString &strRating)
{
	if(m_strRating == strRating) return false;
	m_strRating = strRating;
	return true;
}
void CTransformerInfo::getVoltage(CString &strVoltage)
{
	strVoltage = m_strVoltage;
}
bool CTransformerInfo::setVoltage(const CString &strVoltage)
{
	if(m_strVoltage== strVoltage) return false;
	m_strVoltage = strVoltage;
	return true;
}
int CTransformerInfo::getPhases()
{
	return m_iPhases;
}
bool CTransformerInfo::setPhases(const int iPhases)
{
	if(m_iPhases== iPhases) return false;
	m_iPhases = iPhases;
	return true;
}
void CTransformerInfo::getTapRatio(CString &strTapRatio)
{
	strTapRatio = m_strTapRatio;
}
bool CTransformerInfo::setTapRatio(const CString &strTapRatio)
{
	if(m_strTapRatio== strTapRatio) return false;
	m_strTapRatio = strTapRatio;
	return true;
}
int CTransformerInfo::getTapSetting()
{
	return m_iTapSetting;
}
bool CTransformerInfo::setTapSetting(const int iTapSetting)
{
	if(m_iTapSetting== iTapSetting) return false;
	m_iTapSetting = iTapSetting;
	return true;
}
void CTransformerInfo::getCTRatio(CString &strCTRatio)
{
	strCTRatio = m_strCTRatio;
}
bool CTransformerInfo::setCTRatio(const CString &strCTRatio)
{
	if(m_strCTRatio== strCTRatio) return false;
	m_strCTRatio = strCTRatio;
	return true;
}
void CTransformerInfo::getPoleLength(CString &strPoleLength)
{
	strPoleLength = m_strPoleLength;
}
bool CTransformerInfo::setPoleLength(const CString &strPoleLength)
{
	if(m_strPoleLength == strPoleLength) return false;
	m_strPoleLength = strPoleLength;
	return true;
}
void CTransformerInfo::getPoleStrength(CString &strPoleStrength)
{
	strPoleStrength = m_strPoleStrength;
}
bool CTransformerInfo::setPoleStrength(const CString &strPoleStrength)
{
	if(m_strPoleStrength== strPoleStrength) return false;
	m_strPoleStrength = strPoleStrength;
	return true;
}
void CTransformerInfo::getKFactor(CString &strKFactor)
{
	strKFactor = m_strKFactor;
}
bool CTransformerInfo::setKFactor(const CString &strKFactor)
{
	if(m_strKFactor== strKFactor) return false;
	m_strKFactor = strKFactor;
	return true;
}
void CTransformerInfo::setInfoToRegistry(HKEY hKey)
{
	CString strVal;
	CRegKey rkeyTX;
	rkeyTX.Attach(hKey);
	rkeyTX.SetStringValue( _T("TXRating"), m_strRating );
	rkeyTX.SetStringValue( _T("TXVoltage"), m_strVoltage);
	rkeyTX.SetStringValue( _T("TXTapRatio"), m_strTapRatio);
	rkeyTX.SetStringValue( _T("TXCTRatio"), m_strCTRatio);
	rkeyTX.SetStringValue( _T("TXPoleLength"), m_strPoleLength);
	rkeyTX.SetStringValue( _T("TXPoleStrength"), m_strPoleStrength);
	rkeyTX.SetStringValue( _T("TXKFactor"), m_strKFactor);
	strVal.FormatMessage(L"%1!d!",m_iPhases);
	rkeyTX.SetStringValue( _T("TXPhases"), strVal);
	strVal.FormatMessage(L"%1!d!",m_iTapSetting);
	rkeyTX.SetStringValue( _T("TXTapSetting"), strVal);
	rkeyTX.Detach();
}
void CTransformerInfo::loadDataFromRegistry(HKEY hKey)
{
	CRegKey rkeyTX;
	TCHAR szBuffer[256];
	ULONG nChars = 256;
	rkeyTX.Attach(hKey);
	rkeyTX.QueryStringValue( _T("TXRating"), szBuffer,&nChars);
	m_strRating = szBuffer;
	nChars = 256;
	rkeyTX.QueryStringValue( _T("TXVoltage"),szBuffer,&nChars);
	m_strVoltage = szBuffer;
	nChars = 256;
	rkeyTX.QueryStringValue( _T("TXTapRatio"), szBuffer,&nChars);
	m_strTapRatio = szBuffer;
	nChars = 256;
	rkeyTX.QueryStringValue( _T("TXCTRatio"), szBuffer,&nChars);
	m_strCTRatio = szBuffer;
	nChars = 256;
	rkeyTX.QueryStringValue( _T("TXPoleLength"), szBuffer,&nChars);
	m_strPoleLength = szBuffer;
	nChars = 256;
	rkeyTX.QueryStringValue( _T("TXPoleStrength"), szBuffer,&nChars);
	m_strPoleStrength = szBuffer;
	nChars = 256;
	rkeyTX.QueryStringValue( _T("TXKFactor"), szBuffer,&nChars);
	m_strKFactor = szBuffer;
	nChars = 256;
	rkeyTX.QueryStringValue( _T("TXPhases"), szBuffer,&nChars);
	m_iPhases = _wtoi(szBuffer);
	nChars = 256;
	rkeyTX.QueryStringValue( _T("TXTapSetting"), szBuffer,&nChars);
	m_iTapSetting = _wtoi(szBuffer);
	rkeyTX.Detach();
}