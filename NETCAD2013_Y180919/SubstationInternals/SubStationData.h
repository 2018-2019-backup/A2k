#pragma once

#include "SubStationInfo.h"
#include "NSDatabaseMgr.h"
#include "Substation.h"

class CSSData
{
private:
	CString m_szUserName;
	CString m_szPwd;
	CString m_cmdStr;
	CNSDatabaseMgr* m_DbMgr;
	CSubstation m_Substation;

	CString m_strGeographic;	
	CString m_strGeoVisibility;	
	CString m_strGeoLookupProp;

	CString m_strSchematic;
	CString m_strSchemVisibility;
	CString m_strSchemLookupProp;

	CString m_strDistSchem[5];
	CString m_strDistSchemVisibility[5];
	CString m_strDistXPos[5];
	CString m_strDistYPos[5];
	int		m_iDistRotation[5];
	bool m_bManualTapSetting;
	bool m_bManualFuseA[5];
	bool m_bManualFuse[5];
	

public:
	CSSData( const CString &Database,const CString &UserName = "", const CString &Password = "");
	~CSSData(void);
	void CSSData::getGeographic(CString &strGeographic);
	void CSSData::getGeoVisibility(CString &strGeoVisibility);
	void CSSData::getGeoLookupProp(CString &strGeoLookupProp);
	void getSchematic(CString &strSchematic);
	void getSchemVisibility(CString &strSchemVisibility);
	void getSchemLookupProp(CString &strSchemLookupProp);
	void getDistributorBlockInfo(int index,CString &strdistSchem, CString &strDistSchemVisibility
		, CString &strDistXPos, CString &strDistYPos, int iDistRotation);
	void getSSStockTypes(std::vector<variant_t> &vStockTypes, const CString &strType = "", const CString &strSize = "" );
	void getSSDescriptions(std::vector<variant_t> &vDescriptions, const CString &strType = "", const CString &strSize = "" );
	void getSSSizes(std::vector<variant_t> &vSizes,const CString &strType = "");
	void getSSTypes(std::vector<variant_t> &vTypes);
	void getSSSize(CString &strSize);
	bool setSSSize(const CString &strSize);
	void getSSType(CString &strType);
	bool setSSType(const CString &strType);
	void getSSDescription(CString &strDesc);
	bool setSSDescription(const CString &strDesc);
	void getSSStockType(CString &strStockType);
	bool setSSStockType(const CString &strStockType);
	bool setSSFunctionAndLabel(const CString &strType);
	void getSSFunction(CString &strFunction);
	void getSSLabel(CString &strLabel);
	void getSSPrefix(CString &strPrefix);
	bool setSSPrefix(const CString &strSSPrefix);
	void getSSName(CString &strName);
	bool setSSName(const CString &strName);
	void getSSNumber(CString &strNumber);
	bool setSSNumber(const CString &iNumber);
	void getSSOptions(CString &strOptions);
	bool setSSOptions(const CString &strOptions);


	void getHVCEquipments(std::vector<variant_t> &vEquipments);
	void getHVCTypes(std::vector<variant_t> &vTypes, const CString &strHVCEquip);
	void getHVCRatings(std::vector<variant_t> &vTypes, const CString &strHVCEquip, const CString &strHVCType);
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

	void getTxRatings(std::vector<variant_t> &vTypes, const CString &strSSSize);
	void getTXRating(CString &strRating);
	bool setTXRating(const CString &strRating);
	void getTXVoltage(CString &strVoltage);
	void getTXVoltage(variant_t &vtVoltage,const CString &strSSSize,const CString &strRating);
	bool setTXVoltage(const CString &strVoltage);
	int getTXPhases();
	int getTXPhases(const CString &strSSSize,const CString &strRating);
	bool setTXPhases(const int strPhases);
	void getTXTapRatios(std::vector<variant_t> &vTapRatios, const CString &strSSSize);
	void getTXTapRatio(CString &strTapRatio);
	bool setTXTapRatio(const CString &strTapRatio);
	int getTXTapSetting();
	int getTXTapSettingFromDB(const CString &strSSSize,const CString &strRating);
	bool setTXTapSetting(const int iTapSetting);
	void getTXMDIVals(CString &strCTRatio, CString &strKFactor, const CString &strDesc);
	void getTXCTRatio(CString &strCTRatio);
	bool setTXCTRatio(const CString &strCTRatio);
	void getTXPoleLength(CString &strPoleLength);
	bool setTXPoleLength(const CString &strPoleLength);
	void getTXPoleStrength(CString &strPoleStrength);
	bool setTXPoleStrength(const CString &strPoleStrength);
	void getTXPoleStrengths(std::vector<variant_t> &vPoleStrentghs, const CString &strSSSize, const CString &strTXRating);
	void getTXPoleLengths(std::vector<variant_t> &vPoleLengths, const CString &strSSSize, const CString &strTXRating);
	void getTXKFactor(CString &strKFactor);
	bool setTXKFactor(const CString &strKFactor);

	
	int getLVCNoOfDistributors();
	int getLVCNoOfDistributors(const CString &strSSDesc);
	bool setLVCNoOfDistributors(const int iNoOfDistributors);
	int getLVCPanelRating(const int index);
	bool setLVCPanelRating(const int index, const int iPanelRating);
	int getLVCFuseA(const int index);	
	int getLVCPanelRatingFromDB(const CString &strDesc, const int iDistNo);
	int getLVCFuseAFromDB(const CString &strDesc, const int iDistNo);
	void getLVCFuseFromDB(const CString &strDesc, const int iDistNo, CString &strFuse);
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
	void getLVCFuseAList(std::vector<variant_t> &vFuseAList, const int iDistNo);
	void getLVCFuseList(std::vector<variant_t> &vFuseList, const int FuseA, const int iDistNo);
	void getLVCDistCaption(const int index, CString &strDistCaption);
	bool setLVCDistCaption(const int index, const CString &strDistCaption);
	void clearLVCInfo(int index = -1);

	void loadGeographicInfo(const CString &strOPtion);
	void loadSchematicInfo();
	void loadDistributorsBlockInfo();
	void getSSOptionsFromDB(std::vector<variant_t> &vOptions, const CString &strSSType, const CString &strSSSize, int item);
	void getSSDetails(CString &strDetails, const CString &strOption, const CString &strSSType, const CString &strSSSize);
	void setDataToRegistry();
	void loadDataFromRegistry();
	bool isTxTapSettingManual()
	{
		return m_bManualTapSetting;
	}
	void setTxTapSettingManual(bool bIsManual)
	{
		m_bManualTapSetting = bIsManual;
	}
	bool isLVCFuseAManual(int index){return m_bManualFuseA[index];}
	void setLVCFuseAManual(int index, bool bIsManual)
		{m_bManualFuseA[index] = bIsManual;}
	bool isLVCFuseManual(int index){return m_bManualFuse[index];}
	void setLVCFuseManual(int index, bool bIsManual)
		{m_bManualFuse[index] = bIsManual;}

};

