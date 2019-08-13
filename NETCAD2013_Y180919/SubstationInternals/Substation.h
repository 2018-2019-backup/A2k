#pragma once
#include "SubstationInfo.h"
#include "HVConnectionsInfo.h"
#include "TransformerInfo.h"
#include "LVConnectionsInfo.h"


class CSubstation
{
private:

	CSubstationInfo m_SSubstationInfo;
	CHVConnectionsInfo m_HVConnectionsInfo;
	CTransformerInfo m_TransformerInfo;
	CLVConnectionsInfo m_LVConnectionsInfo;	
public:
	CSubstation(void);
	virtual ~CSubstation(void);
	void getSSType(CString &strType);
	bool setSSType(const CString &strType);
	void getSSSize(CString &strSize);
	bool setSSSize(const CString &strSize);
	void getSSDescription(CString &strDesc);
	bool setSSDescription(const CString &strDesc);
	void getSSStockType(CString &strStockType);
	bool setSSStockType(const CString &strStockType);
	void getSSLabel(CString &strLabel);
	bool setSSLabel(const CString &strLabel);
	void getSSFunction(CString &strFunction);
	bool setSSFunction(const CString &strFunction);
	void getSSPrefix(CString &strPrefix);
	bool setSSPrefix(const CString &strPrefix);
	void getSSName(CString &strName);
	bool setSSName(const CString &strName);
	void getSSNumber(CString &strNumber);
	bool setSSNumber(const CString &strNumber);
	void getSSOptions(CString &strOptions);
	bool setSSOptions(const CString &strOptions);

	
	void getHVCType(CString &strType);
	bool setHVCType(const CString &strType);
	void getHVCEquipment(CString &strEquipment);
	bool setHVCEquipment(const CString &strEquipment);
	void getHVCRating(CString &strRating);
	bool setHVCRating(const CString &strRating);
	int getHVCEFIA();
	bool setHVCEFIA(const int &iEFIA);
	int getHVCEFIB();
	bool setHVCEFIB(const int &iEFIB);
	void getHVCSideA(CString &strSideA);
	bool setHVCSideA(const CString &strSideA);
	void getHVCSideB(CString &strSideB);
	bool setHVCSideB(const CString &strSideB);
	void getHVCNumber(CString &strNumber);
	bool setHVCNumber(const CString &strNumber);

	void getTXRating(CString &strRating);
	bool setTXRating(const CString &strRating);
	void getTXVoltage(CString &strVoltage);
	bool setTXVoltage(const CString &strVoltage);
	int getTXPhases();
	bool setTXPhases(const int iPhases);
	void getTXTapRatio(CString &strTapRatio);
	bool setTXTapRatio(const CString &strTapRatio);
	int getTXTapSetting();
	bool setTXTapSetting(const int iTapSetting);
	void getTXCTRatio(CString &strCTRatio);
	bool setTXCTRatio(const CString &strCTRatio);
	void getTXPoleLength(CString &strPoleLength);
	bool setTXPoleLength(const CString &strPoleLength);
	void getTXPoleStrength(CString &strPoleStrength);
	bool setTXPoleStrength(const CString &strPoleStrength);
	void getTXKFactor(CString &strKFactor);
	bool setTXKFactor(const CString &strKFactor);
	
	int getLVCNoOfDistributors();
	bool setLVCNoOfDistributors(const int iNoOfDistributors);
	int getLVCPanelRating(const int index);
	bool setLVCPanelRating(const int index, const int iPanelRating);
	int getLVCFuseA(const int index);
	bool setLVCFuseA(const int index, const int iFuseA);
	int getLVCWDNO(const int index);
	bool setLVCWDNO(const int index, const int iWDNO);
	int getLVCIDLINK(const int index);
	bool setLVCIDLINK(const int index, const int iIDLINK);
	void getLVCDistributorName(const int index, CString &strDistributorName);
	bool setLVCDistributorName(const int index, const CString &strDistributorName);
	void getLVCFuse(const int index, CString &strFuse);
	bool setLVCFuse(const int index, const CString &strFuse);
	int getLVCDistributorNo();
	bool setLVCDistributorNo(const int iDistributorNO);
	void getLVCDistCaption(const int index, CString &strDistCaption);
	bool setLVCDistCaption(const int index, const CString &strDistCaption);
	void clearLVCInfo(int index);
	void setInfoToRegistry(HKEY hKey);
	void loadDataFromRegistry(HKEY hKey);

};
