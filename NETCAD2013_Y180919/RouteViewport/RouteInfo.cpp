#include "StdAfx.h"
#include "RouteInfo.h"
#include <vector>

CRouteInfo::CRouteInfo(void)
{
	m_iSheetNo   = 0;
	m_iAsciiNo   = 64;

	m_dRotation  = 0.0;

	m_objId			 = AcDbObjectId::kNull;
	m_objIdUcs	 = AcDbObjectId::kNull;
	m_objIdVPort = AcDbObjectId::kNull;

	m_csUcsName   = _T("");
	m_csSheetName = _T("");

	m_objLayoutID = AcDbObjectId::kNull;

	m_iVPN			 = 0;
	m_iNumber    = -1;

	m_objViewId	 = AcDbObjectId::kNull;
	m_dHeight    = 0.0;
	m_dWidth     = 0.0;

	m_bIsNewSheet = false;
}

CRouteInfo::~CRouteInfo(void)
{
}

void assignVPN(std:: vector <CRouteInfo> &routeInfo)
{
	int iVPN;
	int iPrevSheetNo = -1;
	for (int i = 0; i < routeInfo.size(); i++) 
	{ 
		if (iPrevSheetNo != routeInfo[i].m_iSheetNo) iVPN = 1; else iVPN++;
		routeInfo[i].m_iVPN =  iVPN;
		iPrevSheetNo = routeInfo[i].m_iSheetNo;
	}
}


int getRouteCount(std:: vector <CRouteInfo> &routeInfo, int iSheetNo)
{
	int iReturn = 0;
	for (int i = 0; i < routeInfo.size(); i++) 
	{ 
		if (routeInfo[i].m_iSheetNo == iSheetNo) iReturn++;
	}

	return iReturn;
}

double getRouteHeight(std:: vector <CRouteInfo> &routeInfo, int iSheetNo)
{
	int dReturn = 0;
	for (int i = 0; i < routeInfo.size(); i++) 
	{ 
		if (routeInfo[i].m_iSheetNo == iSheetNo) dReturn += routeInfo[i].m_dHeight;
	}

	return dReturn;
}

void sortRouteInfo (std:: vector <CRouteInfo> &routeInfo)
{
	CRouteInfo tmpRouteInfo;
	for (int i = 0; i < routeInfo.size() - 1; i++)
	{
		for (int j = i + 1; j < routeInfo.size(); j++)
		{
			if (routeInfo[i].m_iSheetNo > routeInfo[j].m_iSheetNo)
			{
				tmpRouteInfo = routeInfo[i]; // Swapping entire class
				routeInfo[i] = routeInfo[j];
				routeInfo[j] = tmpRouteInfo;
			}
		}
	}

	// Get the viewport index (Useful when more than one viewport on the same sheet)
	assignVPN(routeInfo);
	
	return;
}

