#include "StdAfx.h"
#include "ConductorInfo.h"
#include <vector>

CConductorInfo::CConductorInfo(void)
{
	m_iIndex = -1;
	m_csLayer = _T("");
	m_objIdNext = AcDbObjectId::kNull;
	m_objId			= AcDbObjectId::kNull;
}

CConductorInfo::~CConductorInfo(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// Function name: sortConductorInfo()
// Description  : Sorts the conductors based on the index assigned to it.
///////////////////////////////////////////////////////////////////////////////
void sortConductorInfo (std:: vector <CConductorInfo> &conductorInfo)
{
	CConductorInfo tmpConductorInfo;
	for (int i = 0; i < conductorInfo.size() - 1; i++)
	{
		for (int j = i + 1; j < conductorInfo.size(); j++)
		{
			if (conductorInfo[i].m_iIndex > conductorInfo[j].m_iIndex)
			{
				tmpConductorInfo = conductorInfo[i]; // Swapping entire class
				conductorInfo[i] = conductorInfo[j];
				conductorInfo[j] = tmpConductorInfo;
			}
		}
	}

	return;
}

