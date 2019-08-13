#pragma once

class CSubstationInfo
{
private:
	CString m_strType;
	CString m_strSize;
	CString m_strDescription;
	CString m_strStockType;
	CString m_strLabel;
	CString m_strFunction;
	CString m_strPrefix;
	CString m_strName;
	CString m_strNumber;
	CString m_strOptions;

public:
	CSubstationInfo(void);
	virtual ~CSubstationInfo(void);
	void getType(CString &strType);
	bool setType(const CString &strType);
	void getSize(CString &strSize);
	bool setSize(const CString &strSize);
	void getDescription(CString &strDesc);
	bool setDescription(const CString &strDesc);
	void getStockType(CString &strStockType);
	bool setStockType(const CString &strStockType);	
	void getLabel(CString &strLabel);
	bool setLabel(const CString &strLabel);
	void getFunction(CString &strFunction);
	bool setFunction(const CString &strFunction);
	void getPrefix(CString &strPrefix);
	bool setPrefix(const CString &strPrefix);
	void getName(CString &strName);
	bool setName(const CString &strName);
	void getNumber(CString &strNumber);
	bool setNumber(const CString &strNumber);
	void getOptions(CString &strOptions);
	bool setOptions(const CString &strOptions);
	void setInfoToRegistry(HKEY hKey);
	void loadDataFromRegistry(HKEY hKey);
};
