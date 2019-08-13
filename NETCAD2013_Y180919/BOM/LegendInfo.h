#pragma once

class CLegendInfo
{
public:
	CLegendInfo(void);

	CString m_csType;
	CString m_csDescription;
	CString m_csObject;
	bool	  m_bIsOthers;
	bool    m_bIsProposed;
	bool    m_bIsExisting;
	CString m_csPropLayer;
	CString m_csExistLayer;
	CString m_csOtherLayer;
	bool    m_bIsDescPlcd;
	int m_iIndex;
	double m_dLength;
public:
	~CLegendInfo(void);
};
