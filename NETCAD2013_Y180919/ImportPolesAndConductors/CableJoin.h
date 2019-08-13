#pragma once

class CCableJoin
{
	public:
		CCableJoin(void);
		~CCableJoin(void);

		ads_point ptTrim;
		ads_point ptOther;
		CString csLayer;

		bool bTrimmed;
		int iVertexAt;
		AcDbObjectId objId;
};
