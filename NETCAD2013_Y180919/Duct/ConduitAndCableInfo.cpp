#include "StdAfx.h"
#include "ConduitAndCableInfo.h"

CConduitAndCableInfo::CConduitAndCableInfo(void)
{
	m_dCoduitDia = 0.0;
	m_dCableColorIndex = -1;
	m_csConduitSize  = _T("");
	m_csCableType = _T("");
	m_iColorIndex = 0;

	m_csBlkNameForConduit = L"";
	m_csLayerForConduit = L"";
	m_csVisibilityForConduit = L"";
	
	m_csBlkNameForCable = L"";
	m_csLayerForCable = L"";
	
	m_iRow = -1;
	m_iCol = -1;

	m_dMaxRowDia = 0.0;
	m_dMaxColDia = 0.0;
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
CConduitAndCableInfo::~CConduitAndCableInfo(void)
{
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
double GetMinimumConduitDia(std:: vector <CConduitAndCableInfo> &conduitAndCableInfoVector)
{
	CConduitAndCableInfo conduitAndCableInfo;
	double dMinDia = 999999.99;
	for (int i = 0; i < conduitAndCableInfoVector.size(); i++) 
	{
		conduitAndCableInfo = conduitAndCableInfoVector[i];
		if (conduitAndCableInfo.m_dCoduitDia == 0.0) continue;
		if (conduitAndCableInfo.m_dCoduitDia < dMinDia) { dMinDia = conduitAndCableInfo.m_dCoduitDia; }
	}

	if (dMinDia == 999999.99) dMinDia = 0.0;
	return dMinDia;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool getConduitAndCableItem(std:: vector <CConduitAndCableInfo> &conduitAndCableInfoVector, int iRow, int iCol, CConduitAndCableInfo &conduitAndCableInfo)
{
	for (int i = 0; i < conduitAndCableInfoVector.size(); i++) 
	{
		conduitAndCableInfo = conduitAndCableInfoVector[i];
		if ((conduitAndCableInfo.m_iRow == iRow) && (conduitAndCableInfo.m_iCol == iCol)) return true; 
	}

	return false;
}