#pragma once

#include "ConductorInfo.h"

class CPoleInfo
{
public:
	CPoleInfo(void);

	AcDbObjectId m_objId;
	ads_point m_ptInsert;
	std::vector <CConductorInfo> m_conductorInfo_Vector;
public:
	~CPoleInfo(void);
};
