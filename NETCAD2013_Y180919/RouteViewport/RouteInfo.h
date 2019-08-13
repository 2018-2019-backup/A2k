#pragma once

class CRouteInfo
{
public:
	CRouteInfo(void);
	~CRouteInfo(void);

	int m_iSheetNo;
	int m_iAsciiNo;

	ads_point m_ptInsert;
	
	double m_dRotation;
	AcDbObjectId m_objId;
	AcDbObjectId m_objIdVPort;
	AcDbObjectId m_objIdUcs;
	AcDbObjectId m_objLayoutID;
	AcDbExtents m_extents;

	CString m_csUcsName;
	CString m_csSheetName;

	int m_iVPN;	
	int m_iNumber;
	AcDbObjectId m_objViewId;

	double m_dHeight;
	double m_dWidth;

	bool m_bIsNewSheet;
};
