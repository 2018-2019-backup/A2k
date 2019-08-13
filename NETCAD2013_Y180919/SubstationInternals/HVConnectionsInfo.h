#pragma once

class CHVConnectionsInfo
{
private:
	CString m_strEquipment;
	CString m_strType;
	CString m_strRating;
	CString m_strNumber;
	CString m_strSideA;
	CString m_strSideB;
	int m_iEFIA;
	int m_iEFIB;
public:
	CHVConnectionsInfo(void);
	virtual ~CHVConnectionsInfo(void);
	void getType(CString &strType);
	bool setType(const CString &strType);
	void getEquipment(CString &strEquipment);
	bool setEquipment(const CString &strEquipment);
	void getRating(CString &strRating);
	bool setRating(const CString &strRating);
	int getEFIA();
	bool setEFIA(const int &iEFIA);
	int getEFIB();
	bool setEFIB(const int &iEFIB);
	void getSideA(CString &strSideA);
	bool setSideA(const CString &strSideA);
	void getSideB(CString &strSideB);
	bool setSideB(const CString &strSideB);
	void getNumber(CString &strNumber);
	bool setNumber(const CString &strNumber);
	void setInfoToRegistry(HKEY hKey);
	void loadDataFromRegistry(HKEY hKey);
};
