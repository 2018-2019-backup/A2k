#pragma once

class CConductorInfo
{
public:
	CConductorInfo(void);
	~CConductorInfo(void);

	int m_iIndex;
	CString m_csLayer;
	AcDbObjectId m_objId;
	AcDbObjectId m_objIdNext;
	double m_dOffset;
};


