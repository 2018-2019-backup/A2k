#include "StdAfx.h"
#include "LayoutInfo.h"

CLayoutInfo::CLayoutInfo(void)
{
	m_iTabOrder = 0;
	m_csLayoutName = _T("");
}

CLayoutInfo::~CLayoutInfo(void)
{
}

///////////////////////////////////////////////////////////////
// Function name: sortLayoutInfo()
// Description  : Sorts the layouts based on its index.
///////////////////////////////////////////////////////////////
void sortLayoutInfo (std:: vector <CLayoutInfo> &layoutInfo)
{
	CLayoutInfo tmpLayoutInfo;
	for (int i = 0; i < layoutInfo.size() - 1; i++)
	{
		for (int j = i + 1; j < layoutInfo.size(); j++)
		{
			if (layoutInfo[i].m_iTabOrder > layoutInfo[j].m_iTabOrder)
			{
				tmpLayoutInfo = layoutInfo[i]; // Swapping entire class
				layoutInfo[i] = layoutInfo[j];
				layoutInfo[j] = tmpLayoutInfo;
			}
		}
	}

	return;
}

