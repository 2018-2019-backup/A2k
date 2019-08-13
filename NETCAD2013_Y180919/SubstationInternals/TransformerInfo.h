#pragma once

class CTransformerInfo
{
private:
	CString m_strRating;
	CString m_strVoltage;
	int m_iPhases;
	CString m_strTapRatio;
	int m_iTapSetting;
	CString m_strCTRatio;
	CString m_strPoleLength;
	CString m_strPoleStrength;
	CString m_strKFactor;
public:
	CTransformerInfo(void);
	~CTransformerInfo(void);
	void getRating(CString &strRating);
	bool setRating(const CString &strRating);
	void getVoltage(CString &strVoltage);
	bool setVoltage(const CString &strVoltage);
	int getPhases();
	bool setPhases(const int iPhases);
	void getTapRatio(CString &strTapRatio);
	bool setTapRatio(const CString &strTapRatio);
	int getTapSetting();
	bool setTapSetting(const int iTapSetting);
	void getCTRatio(CString &strCTRatio);
	bool setCTRatio(const CString &strCTRatio);
	void getPoleLength(CString &strPoleLength);
	bool setPoleLength(const CString &strPoleLength);
	void getPoleStrength(CString &strPoleStrength);
	bool setPoleStrength(const CString &strPoleStrength);
	void getKFactor(CString &strKFactor);
	bool setKFactor(const CString &strKFactor);
	void setInfoToRegistry(HKEY hKey);
	void loadDataFromRegistry(HKEY hKey);
};
 