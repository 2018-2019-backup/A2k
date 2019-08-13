#pragma once

class CConduitAndCableInfo
{
public:
	CConduitAndCableInfo(void);
	~CConduitAndCableInfo(void);

	double  m_dCoduitDia;
	double  m_dCableDia;
	double  m_dCableColorIndex;

	CString m_csConduitSize;
	CString m_csCableType;
	CString m_csSelected;

	CString m_csBlkNameForConduit;
	CString m_csVisibilityForConduit;
	CString m_csLayerForConduit;

	CString m_csBlkNameForCable;
	CString m_csLayerForCable;

	CString m_csConduitCable;
	
	int m_iRow;
	int m_iCol;
	double m_dMaxRowDia;
	double m_dMaxColDia;

	int m_iColorIndex;
};
