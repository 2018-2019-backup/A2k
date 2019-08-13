#include "StdAfx.h"
#include "PoleInfo.h"

CPoleInfo::CPoleInfo(void)
{
	m_objId = AcDbObjectId::kNull;
}

CPoleInfo::~CPoleInfo(void)
{
}

void appendPoleInfo(AcDbObjectId objId, ads_point ptPole, std:: vector <CConductorInfo> conductorInfo, std:: vector <CPoleInfo> &poleIInfoVector)
{
	CPoleInfo poleInfo;
	poleInfo.m_objId = objId;
	acutPolar(ptPole, 0.0, 0.0, poleInfo.m_ptInsert);
	poleInfo.m_conductorInfo_Vector = conductorInfo;
	poleIInfoVector.push_back(poleInfo);
}

