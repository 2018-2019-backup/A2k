#include "StdAfx.h"
#include "OffsetInfo.h"

COffsetInfo::COffsetInfo(void)
{
	m_dOffset = 0.0;
}

COffsetInfo::~COffsetInfo(void)
{
}

/////////////////////////////////////////////////////
// Function name: sortOffsetInfo()
// Description  : Sorts the...
/////////////////////////////////////////////////////
void sortOffsetInfo (std:: vector <COffsetInfo> &offsetInfo)
{
	COffsetInfo tmpOffsetInfo;
	for (int i = 0; i < offsetInfo.size() - 1; i++)
	{
		for (int j = i + 1; j < offsetInfo.size(); j++)
		{
			if (offsetInfo[i].m_dOffset > offsetInfo[j].m_dOffset)
			{
				tmpOffsetInfo = offsetInfo[i]; // Swapping entire class
				offsetInfo[i] = offsetInfo[j];
				offsetInfo[j] = tmpOffsetInfo;
			}
		}
	}

	return;
}

