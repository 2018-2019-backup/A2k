#include "StdAfx.h"
#include "LVConnectionsInfo.h"
#include "atlbase.h"

CLVConnectionsInfo::CLVConnectionsInfo(void)
{
	m_iNoOfDistributors = 0;
	clearInfo();
}

CLVConnectionsInfo::~CLVConnectionsInfo(void)
{
}

int CLVConnectionsInfo::getNoOfDistributors()
{
	return m_iNoOfDistributors;
}
bool CLVConnectionsInfo::setNoOfDistributors(const int iNoOfDistributors)
{
	if(m_iNoOfDistributors== iNoOfDistributors) return false;
	m_iNoOfDistributors = iNoOfDistributors;
	return true;
}
int CLVConnectionsInfo::getDistributorNo()
{
	return m_iDistributorNo;
}
bool CLVConnectionsInfo::setDistributorNo(const int iDistributorNO)
{
	if(m_iDistributorNo== iDistributorNO) return false;
	m_iDistributorNo = iDistributorNO;
	return true;
}
int CLVConnectionsInfo::getPanelRating(const int index)
{
	return m_iPanelRating[index];
}
bool  CLVConnectionsInfo::setPanelRating(const int index, const int iPanelRating)
{
	if(m_iPanelRating[index]== iPanelRating) return false;
	m_iPanelRating[index] = iPanelRating;
	return true;
}
int  CLVConnectionsInfo::getFuseA(const int index)
{
	return m_iFuseA[index];
}
bool  CLVConnectionsInfo::setFuseA(const int index, const int iFuseA)
{
	if(m_iFuseA[index]== iFuseA) return false;
	m_iFuseA[index] = iFuseA;
	return true;
}
int  CLVConnectionsInfo::getWDNO(const int index)
{
	return m_iWDNO[index];
}
bool  CLVConnectionsInfo::setWDNO(const int index, const int iWDNO)
{
	if(m_iWDNO[index]== iWDNO) return false;
	m_iWDNO[index] = iWDNO;
	return true;
}
int  CLVConnectionsInfo::getIDLINK(const int index)
{
	return m_iIDLINK[index];
}
bool  CLVConnectionsInfo::setIDLINK(const int index, const int iIDLINK)
{
	if(m_iIDLINK[index]== iIDLINK) return false;
	m_iIDLINK[index] = iIDLINK;
	return true;
}
void  CLVConnectionsInfo::getDistributorName(const int index, CString &strDistributorName)
{
	strDistributorName = m_strDistributorName[index];
}
bool  CLVConnectionsInfo::setDistributorName(const int index, const CString &strDistributorName)
{
	if(m_strDistributorName[index]== strDistributorName) return false;
	m_strDistributorName[index] = strDistributorName;
	return true;
}
void  CLVConnectionsInfo::getFuse(const int index, CString &strFuse)
{
	strFuse = m_strFuse[index];
}
bool  CLVConnectionsInfo::setFuse(const int index, const CString &strFuse)
{
	if(m_strFuse[index]== strFuse) return false;
	m_strFuse[index] = strFuse;
	return true;
}
void CLVConnectionsInfo::getDistCaption(const int index, CString &strDistCaption)
{
	strDistCaption = m_strDistCaption[index];
}
bool CLVConnectionsInfo::setDistCaption(const int index, const CString &strDistCaption)
{
	if(m_strDistCaption[index]== strDistCaption) return false;
	m_strDistCaption[index] = strDistCaption;
	return true;
}
void CLVConnectionsInfo::clearInfo(int index)
{
	m_iDistributorNo= 0;
	if(index == -1)
	{
		for(int i=0; i<5; i++)
		{
			m_iPanelRating[i] = 0;
			m_iFuseA[i] = 0;
			m_iWDNO[i] = 0;
			m_iIDLINK[i] = 0;
			m_strFuse[i] = "";
			m_strDistributorName[i] = "";
			m_strDistCaption[i] = "";
		}
	}
	else
	{
			m_iPanelRating[index] = 0;
			m_iFuseA[index] = 0;
			m_iWDNO[index] = 0;
			m_iIDLINK[index] = 0;
			m_strFuse[index] = "";
			m_strDistributorName[index] = "";
			m_strDistCaption[index] = "";
	}
}
void CLVConnectionsInfo::setInfoToRegistry(HKEY hKey)
{
	CRegKey rkeyLVC;
	CString strVal;
	rkeyLVC.Attach(hKey);
	strVal.FormatMessage(L"%1!d!",m_iNoOfDistributors);
	rkeyLVC.SetStringValue( _T("LVCNoOfDist"), strVal);
	strVal.FormatMessage(L"%1!d!",m_iDistributorNo);
	rkeyLVC.SetStringValue( _T("LVCDistNo"), strVal);
	CString strName;
	for(int i=0; i<5; i++)
	{
		strName.FormatMessage(L"LVCPanelRating%1!d!",i+1);
		strVal.FormatMessage(L"%1!d!",m_iPanelRating[i]);
		rkeyLVC.SetStringValue( strName, strVal);
		
		strName.FormatMessage(L"LVCFuseA%1!d!",i+1);
		strVal.FormatMessage(L"%1!d!",m_iFuseA[i]);
		rkeyLVC.SetStringValue( strName, strVal);
		
		strName.FormatMessage(L"LVCWDNO%1!d!",i+1);
		strVal.FormatMessage(L"%1!d!",m_iWDNO[i]);
		rkeyLVC.SetStringValue( strName, strVal);
		
		strName.FormatMessage(L"LVCIDLINK%1!d!",i+1);
		strVal.FormatMessage(L"%1!d!",m_iIDLINK[i]);
		rkeyLVC.SetStringValue( strName, strVal);
		
		strName.FormatMessage(L"LVCFuse%1!d!",i+1);
		rkeyLVC.SetStringValue( strName, m_strFuse[i]);
		
		strName.FormatMessage(L"LVCDistributorName%1!d!",i+1);
		rkeyLVC.SetStringValue( strName, m_strDistributorName[i]);
		
		strName.FormatMessage(L"LVCDistCaption%1!d!",i+1);
		rkeyLVC.SetStringValue( strName, m_strDistCaption[i]);
	}
	rkeyLVC.Detach();
}


void CLVConnectionsInfo::loadDataFromRegistry(HKEY hKey)
{
	CRegKey rkeyLVC;
	TCHAR szBuffer[256];
	ULONG nChars = 256;
	rkeyLVC.Attach(hKey);
	
	rkeyLVC.QueryStringValue( _T("LVCNoOfDist"), szBuffer,&nChars);
	m_iNoOfDistributors = _wtoi(szBuffer);
	nChars = 256;
	rkeyLVC.QueryStringValue( _T("LVCDistNo"), szBuffer,&nChars);
	m_iDistributorNo = _wtoi(szBuffer);
	CString strName;

	for(int i=0; i<5; i++)
	{
		
		nChars = 256;
		strName.FormatMessage(L"LVCPanelRating%1!d!",i+1);
		rkeyLVC.QueryStringValue(strName,szBuffer,&nChars);
		m_iPanelRating[i] = _wtoi(szBuffer);
		
		nChars = 256;
		strName.FormatMessage(L"LVCFuseA%1!d!",i+1);
		rkeyLVC.QueryStringValue(strName,szBuffer,&nChars);
		m_iFuseA[i] = _wtoi(szBuffer);

		nChars = 256;
		strName.FormatMessage(L"LVCWDNO%1!d!",i+1);
		rkeyLVC.QueryStringValue(strName,szBuffer,&nChars);
		m_iWDNO[i] = _wtoi(szBuffer);

		nChars = 256;
		strName.FormatMessage(L"LVCIDLINK%1!d!",i+1);
		rkeyLVC.QueryStringValue(strName,szBuffer,&nChars);
		m_iIDLINK[i] = _wtoi(szBuffer);		
		
		nChars = 256;
		strName.FormatMessage(L"LVCFuse%1!d!",i+1);
		rkeyLVC.QueryStringValue(strName,szBuffer,&nChars);
		m_strFuse[i] = szBuffer;	
		
		nChars = 256;
		strName.FormatMessage(L"LVCDistributorName%1!d!",i+1);
		rkeyLVC.QueryStringValue(strName,szBuffer,&nChars);
		m_strDistributorName[i] = szBuffer;
		
		nChars = 256;
		strName.FormatMessage(L"LVCDistCaption%1!d!",i+1);
		rkeyLVC.QueryStringValue(strName,szBuffer,&nChars);
		m_strDistCaption[i] = szBuffer;
	}
	rkeyLVC.Detach();
}