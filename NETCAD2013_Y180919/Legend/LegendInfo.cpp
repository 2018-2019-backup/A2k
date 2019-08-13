#include "StdAfx.h"
#include "LegendInfo.h"
#include <vector>

CLegendInfo::CLegendInfo(void)
{
	m_csType = _T("");
	m_csDescription = _T("");
	m_csObject = _T("");
	
	m_bIsOthers = false;
	m_bIsExisting = false;
	m_bIsProposed = false;
	m_bIsDescPlcd = false;
	m_iIndex = 1000;
	m_dLength = 0.0;
}

CLegendInfo::~CLegendInfo(void)
{
}

//////////////////////////////////////////////////////////////////////////
// Function name: sortLegendInfo()
// Description  : Sorts the legend information based on its index.
//////////////////////////////////////////////////////////////////////////
void sortLegendInfo (std:: vector <CLegendInfo> &legendInfo)
{
	CLegendInfo tmpLegendInfo;
	
	for (int i = 0; i < legendInfo.size() - 1; i++)
	{
		for (int j = i + 1; j < legendInfo.size(); j++)
		{
			if (legendInfo[i].m_iIndex > legendInfo[j].m_iIndex)
			{
				tmpLegendInfo = legendInfo[i]; // Swapping entire class
				legendInfo[i] = legendInfo[j];
				legendInfo[j] = tmpLegendInfo;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// This is to sort the strings for block descriptions
	//////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < legendInfo.size() - 1; i++)
	{
		// Skip the lines
		if (legendInfo[i].m_iIndex < 1000) continue;

		int iAsciiI, iAsciiJ;
		for (int j = i + 1; j < legendInfo.size(); j++)
		{
			// If the index is the same that means we must sort these guys. 
			// This can happen if the first character of the block names are the same.
			if (legendInfo[i].m_iIndex == legendInfo[j].m_iIndex)
			{
				for (int iCtr = 0; iCtr < legendInfo[i].m_csDescription.GetLength(); iCtr++)
				{
					if (iCtr >= legendInfo[j].m_csDescription.GetLength()) break;
					iAsciiI = (int) legendInfo[i].m_csDescription.GetAt(iCtr);
					iAsciiJ = (int) legendInfo[j].m_csDescription.GetAt(iCtr);

					if (iAsciiI > iAsciiJ)
					{
						tmpLegendInfo = legendInfo[i]; // Swapping entire class
						legendInfo[i] = legendInfo[j];
						legendInfo[j] = tmpLegendInfo;
						break;
					}
					else if (iAsciiI < iAsciiJ) break;
				}
			}
		}
	}

	return;
}

/////////////////////////////////////////////////////////////////////////////////////
// Function name: getNumberOfRows()
// Description  : Retrieves the number of rows to be written to the LEGEND table.
/////////////////////////////////////////////////////////////////////////////////////
int getNumberOfRows(std:: vector <CLegendInfo> &legendInfo, int iLegendType)
{
	int iNoOfRows = 0;
	if (iLegendType == 0) return legendInfo.size();

	for (int i = 0; i < legendInfo.size(); i++)
	{
		/**/ if ((iLegendType == 1) && (legendInfo.at(i).m_iIndex < 1000)) iNoOfRows++;
		else if ((iLegendType == 2) && (legendInfo.at(i).m_iIndex > 1000)) iNoOfRows++;
	}

	return iNoOfRows;
}
