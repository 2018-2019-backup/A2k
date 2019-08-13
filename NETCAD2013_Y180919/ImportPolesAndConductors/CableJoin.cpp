#include "StdAfx.h"
#include "CableJoin.h"

CCableJoin::CCableJoin(void)
{
	bTrimmed = false;
	iVertexAt = 0;
}

CCableJoin::~CCableJoin(void)
{
}

/////////////////////////////////////////////////////
// Function name: TrimCable()
// Description  :
/////////////////////////////////////////////////////
void TrimCable(std::vector <CCableJoin> &cableJoinVec)
{
	int iColor = 1;
	for (int i = 0; i < cableJoinVec.size(); i++)
	{
		if (cableJoinVec.at(i).bTrimmed == true) { continue; }
		for (int j = i + 1; j < cableJoinVec.size(); j++) 
		{
			if (cableJoinVec.at(j).bTrimmed == true) { continue; }

			// If the layer names are the same
			if (!cableJoinVec.at(i).csLayer.CompareNoCase(cableJoinVec.at(j).csLayer))
			{
				// Get the intersection points for the cables
				ads_point ptInters;
				if (acdbInters(cableJoinVec.at(i).ptOther, cableJoinVec.at(i).ptTrim, cableJoinVec.at(j).ptOther, cableJoinVec.at(j).ptTrim, false, ptInters) != RTNORM) { continue; }
								
				// Open the entities and modify the points
				AcDbPolyline *pLine1 = NULL;
				if (acdbOpenObject(pLine1, cableJoinVec.at(i).objId, AcDb::kForWrite) == Acad::eOk)
				{
					pLine1->setPointAt(cableJoinVec.at(i).iVertexAt, AcGePoint2d(ptInters[X], ptInters[Y]));
					pLine1->close();
					cableJoinVec.at(i).bTrimmed = true;
				}

				AcDbPolyline *pLine2 = NULL;
				if (acdbOpenObject(pLine2, cableJoinVec.at(j).objId, AcDb::kForWrite) == Acad::eOk)
				{
					pLine2->setPointAt(cableJoinVec.at(j).iVertexAt, AcGePoint2d(ptInters[X], ptInters[Y]));
					pLine2->close();
					cableJoinVec.at(j).bTrimmed = true;
				}

				// Use PEDIYT to join them
				ads_name enPline1; acdbGetAdsName(enPline1, cableJoinVec.at(i).objId);
				ads_name enPline2; acdbGetAdsName(enPline2, cableJoinVec.at(j).objId);
				acedCommandS(RTSTR, L".PEDIT", RTENAME, enPline1, RTSTR, L"J", RTENAME, enPline2, RTSTR, L"", RTSTR, L"", NULL);

				break;
			}
		}
	}
}

