#include "StdAfx.h"
#include "OPFileInfo.h"

COPFileInfo::COPFileInfo(void)
{
	
}

COPFileInfo::~COPFileInfo(void)
{
}

/////////////////////////////////////////////////////
// Function name: 
// Description  :
// Arguments    :
// Called by    :
/////////////////////////////////////////////////////
void sortOPFileInfo (std:: vector <COPFileInfo> &OPFileInfo_Vector)
{
	COPFileInfo tmpInfo;
	for (int i = 0; i < OPFileInfo_Vector.size() - 1; i++)
	{
		for (int j = i + 1; j < OPFileInfo_Vector.size(); j++)
		{
			if (OPFileInfo_Vector[i].m_iIndex > OPFileInfo_Vector[j].m_iIndex)
			{
				tmpInfo = OPFileInfo_Vector[i]; // Swapping entire class
				OPFileInfo_Vector[i] = OPFileInfo_Vector[j];
				OPFileInfo_Vector[j] = tmpInfo;
			}
		}
	}

	return;
}
