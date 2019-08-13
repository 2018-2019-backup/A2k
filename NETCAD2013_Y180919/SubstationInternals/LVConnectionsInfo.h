#pragma once

class CLVConnectionsInfo
{
private:
	int m_iNoOfDistributors;
	int m_iDistributorNo;
	int m_iPanelRating[5];
	int m_iFuseA[5];
	CString m_strFuse[5];
	CString m_strDistributorName[5];
	int m_iWDNO[5];
	int m_iIDLINK[5];
	CString m_strDistCaption[5];
	
public:
	CLVConnectionsInfo(void);
	virtual ~CLVConnectionsInfo(void);
	int getNoOfDistributors();
	bool setNoOfDistributors(const int iNoOfDistributors);
	int getDistributorNo();
	bool setDistributorNo(const int iDistributorNO);
	int getPanelRating(const int index);
	bool setPanelRating(const int index, const int iPanelRating);
	int getFuseA(const int index);
	bool setFuseA(const int index, const int iFuseA);
	int getWDNO(const int index);
	bool setWDNO(const int index, const int iWDNO);
	int getIDLINK(const int index);
	bool setIDLINK(const int index, const int iIDLINK);
	void getDistributorName(const int index, CString &strDistributorName);
	bool setDistributorName(const int index, const CString &strDistributorName);
	void getFuse(const int index, CString &strFuse);
	bool setFuse(const int index, const CString &strFuse);
	void getDistCaption(const int index, CString &strDistCaption);
	bool setDistCaption(const int index, const CString &strDistCaption);
	void clearInfo(int index = -1);
	void setInfoToRegistry(HKEY hKey);
	void loadDataFromRegistry(HKEY hKey);
};
