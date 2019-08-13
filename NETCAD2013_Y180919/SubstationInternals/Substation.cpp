#include "StdAfx.h"
#include "Substation.h"

CSubstation::CSubstation(void)
{
}

CSubstation::~CSubstation(void)
{
}

void CSubstation::getSSType(CString &strType)
{
	m_SSubstationInfo.getType(strType);
}
bool CSubstation::setSSType(const CString &strType)
{
	return m_SSubstationInfo.setType(strType);
}
void CSubstation::getSSSize(CString &strSize)
{
	m_SSubstationInfo.getSize(strSize);
}
bool CSubstation::setSSSize(const CString &strSize)
{
	return m_SSubstationInfo.setSize(strSize);
}
void CSubstation::getSSDescription(CString &strDesc)
{
	m_SSubstationInfo.getDescription(strDesc);
}
bool CSubstation::setSSDescription(const CString &strDesc)
{
	return m_SSubstationInfo.setDescription(strDesc);
}
void CSubstation::getSSStockType(CString &strStockType)
{
	m_SSubstationInfo.getStockType(strStockType);
}
bool CSubstation::setSSStockType(const CString &strStockType)
{
	return m_SSubstationInfo.setStockType(strStockType);
}

void CSubstation::getSSLabel(CString &strLabel)
{
	m_SSubstationInfo.getLabel(strLabel);
}
bool CSubstation::setSSLabel(const CString &strLabel)
{
	return m_SSubstationInfo.setLabel(strLabel);
}
void CSubstation::getSSFunction(CString &strFunction)
{
	m_SSubstationInfo.getFunction(strFunction);
}
bool CSubstation::setSSFunction(const CString &strFunction)
{
	return m_SSubstationInfo.setFunction(strFunction);
}	
void CSubstation::getSSPrefix(CString &strPrefix)
{
	m_SSubstationInfo.getPrefix(strPrefix);
}
bool CSubstation::setSSPrefix(const CString &strPrefix)
{
	return m_SSubstationInfo.setPrefix(strPrefix);
}	
void CSubstation::getSSName(CString &strName)
{
	m_SSubstationInfo.getName(strName);
}
bool CSubstation::setSSName(const CString &strName)
{
	return m_SSubstationInfo.setName(strName);
}	
void CSubstation::getSSNumber(CString &strNumber)
{
	m_SSubstationInfo.getNumber(strNumber);
}
bool CSubstation::setSSNumber(const CString &strNumber)
{
	return m_SSubstationInfo.setNumber(strNumber);
}
void CSubstation::getSSOptions(CString &strOptions)
{
	m_SSubstationInfo.getOptions(strOptions);
}
bool CSubstation::setSSOptions(const CString &strOptions)
{
	return m_SSubstationInfo.setOptions(strOptions);
}
void CSubstation::getHVCType(CString &strType)
{
	m_HVConnectionsInfo.getType(strType);	
}
bool CSubstation::setHVCType(const CString &strType)
{
	return m_HVConnectionsInfo.setType(strType);	
}
void CSubstation::getHVCEquipment(CString &strEquipment)
{
	m_HVConnectionsInfo.getEquipment(strEquipment);	
}
bool CSubstation::setHVCEquipment(const CString &strEquipment)
{
	return m_HVConnectionsInfo.setEquipment(strEquipment);	
}
void CSubstation::getHVCRating(CString &strRating)
{
	m_HVConnectionsInfo.getRating(strRating);	
}
bool CSubstation::setHVCRating(const CString &strRating)
{
	return m_HVConnectionsInfo.setRating(strRating);	
}
int CSubstation::getHVCEFIA()
{
	return m_HVConnectionsInfo.getEFIA();
}
bool CSubstation::setHVCEFIA(const int &iEFIA)
{
	return m_HVConnectionsInfo.setEFIA(iEFIA);
}
int CSubstation::getHVCEFIB()
{
	return m_HVConnectionsInfo.getEFIB();
}
bool CSubstation::setHVCEFIB(const int &iEFIB)
{
	return m_HVConnectionsInfo.setEFIB(iEFIB);
}
void CSubstation::getHVCSideA(CString &strSideA)
{
	m_HVConnectionsInfo.getSideA(strSideA);	
}
bool CSubstation::setHVCSideA(const CString &strSideA)
{
	return m_HVConnectionsInfo.setSideA(strSideA);
}
void CSubstation::getHVCSideB(CString &strSideB)
{
	m_HVConnectionsInfo.getSideB(strSideB);	
}
bool CSubstation::setHVCSideB(const CString &strSideB)
{
	return m_HVConnectionsInfo.setSideB(strSideB);
}
void CSubstation::getHVCNumber(CString &strNumber)
{
	m_HVConnectionsInfo.getNumber(strNumber);	
}
bool CSubstation::setHVCNumber(const CString &strNumber)
{
	return m_HVConnectionsInfo.setNumber(strNumber);
}
void CSubstation::getTXRating(CString &strRating)
{
	m_TransformerInfo.getRating(strRating);
}
bool CSubstation::setTXRating(const CString &strRating)
{
	return m_TransformerInfo.setRating(strRating);
}

void CSubstation::getTXVoltage(CString &strVoltage)
{
	m_TransformerInfo.getVoltage(strVoltage);
}
bool CSubstation::setTXVoltage(const CString &strVoltage)
{
	return m_TransformerInfo.setVoltage(strVoltage);
}
int CSubstation::getTXPhases()
{
	return m_TransformerInfo.getPhases();
}
bool CSubstation::setTXPhases(const int iPhases)
{
	return m_TransformerInfo.setPhases(iPhases);
}
void CSubstation::getTXTapRatio(CString &strTapRatio)
{
	m_TransformerInfo.getTapRatio(strTapRatio);
}
bool CSubstation::setTXTapRatio(const CString &strTapRatio)
{
	return m_TransformerInfo.setTapRatio(strTapRatio);
}
int CSubstation::getTXTapSetting()
{
	return m_TransformerInfo.getTapSetting();
}
bool CSubstation::setTXTapSetting(const int iTapSetting)
{
	return m_TransformerInfo.setTapSetting(iTapSetting);
}
void CSubstation::getTXCTRatio(CString &strCTRatio)
{
	m_TransformerInfo.getCTRatio(strCTRatio);
}
bool CSubstation::setTXCTRatio(const CString &strCTRatio)
{
	return m_TransformerInfo.setCTRatio(strCTRatio);
}
void CSubstation::getTXPoleLength(CString &strPoleLength)
{
	m_TransformerInfo.getPoleLength(strPoleLength);
}
bool CSubstation::setTXPoleLength(const CString &strPoleLength)
{
	return m_TransformerInfo.setPoleLength(strPoleLength);
}
void CSubstation::getTXPoleStrength(CString &strPoleStrength)
{
	m_TransformerInfo.getPoleStrength(strPoleStrength);
}
bool CSubstation::setTXPoleStrength(const CString &strPoleStrength)
{
	return m_TransformerInfo.setPoleStrength(strPoleStrength);
}
void CSubstation::getTXKFactor(CString &strKFactor)
{
	m_TransformerInfo.getKFactor(strKFactor);
}
bool CSubstation::setTXKFactor(const CString &strKFactor)
{
	return m_TransformerInfo.setKFactor(strKFactor);
}
int CSubstation::getLVCNoOfDistributors()
{
	return m_LVConnectionsInfo.getNoOfDistributors();
}
bool CSubstation::setLVCNoOfDistributors(const int iNoOfDistributors)
{
	return m_LVConnectionsInfo.setNoOfDistributors(iNoOfDistributors);
}
int CSubstation::getLVCPanelRating(const int index)
{
	return m_LVConnectionsInfo.getPanelRating(index);
}
bool CSubstation::setLVCPanelRating(const int index, const int iPanelRating)
{
	return m_LVConnectionsInfo.setPanelRating(index,iPanelRating);
}
int CSubstation::getLVCFuseA(const int index)
{
	return m_LVConnectionsInfo.getFuseA(index);
}
bool CSubstation::setLVCFuseA(const int index, const int iFuseA)
{
	return m_LVConnectionsInfo.setFuseA(index,iFuseA);
}
int CSubstation::getLVCWDNO(const int index)
{
	return m_LVConnectionsInfo.getWDNO(index);
}
bool CSubstation::setLVCWDNO(const int index, const int iWDNO)
{
	return m_LVConnectionsInfo.setWDNO(index,iWDNO);
}
int CSubstation::getLVCIDLINK(const int index)
{
	return m_LVConnectionsInfo.getIDLINK(index);
}
bool CSubstation::setLVCIDLINK(const int index, const int iIDLINK)
{
	return m_LVConnectionsInfo.setIDLINK(index,iIDLINK);
}
void CSubstation::getLVCDistributorName(const int index, CString &strDistributorName)
{
	m_LVConnectionsInfo.getDistributorName(index,strDistributorName);
}
bool CSubstation::setLVCDistributorName(const int index, const CString &strDistributorName)
{
	return m_LVConnectionsInfo.setDistributorName(index,strDistributorName);
}
void CSubstation::getLVCFuse(const int index, CString &strFuse)
{
	m_LVConnectionsInfo.getFuse(index,strFuse);
}
bool CSubstation::setLVCFuse(const int index, const CString &strFuse)
{
	return m_LVConnectionsInfo.setFuse(index,strFuse);
}
int CSubstation::getLVCDistributorNo()
{
	return m_LVConnectionsInfo.getDistributorNo();
}
bool CSubstation::setLVCDistributorNo(const int iDistributorNO)
{
	return m_LVConnectionsInfo.setDistributorNo(iDistributorNO);
}
void CSubstation::getLVCDistCaption(const int index, CString &strDistCaption)
{
	m_LVConnectionsInfo.getDistCaption(index, strDistCaption);
}
bool CSubstation::setLVCDistCaption(const int index, const CString &strDistCaption)
{
	return m_LVConnectionsInfo.setDistCaption(index, strDistCaption);
}
void CSubstation::clearLVCInfo(int index)
{
	m_LVConnectionsInfo.clearInfo(index);
}
void CSubstation::setInfoToRegistry(HKEY hKey)
{
	m_SSubstationInfo.setInfoToRegistry(hKey);
	m_TransformerInfo.setInfoToRegistry(hKey);
	m_HVConnectionsInfo.setInfoToRegistry(hKey);
	m_LVConnectionsInfo.setInfoToRegistry(hKey);
}
void CSubstation::loadDataFromRegistry(HKEY hKey)
{
	m_SSubstationInfo.loadDataFromRegistry(hKey);
	m_HVConnectionsInfo.loadDataFromRegistry(hKey);
	m_TransformerInfo.loadDataFromRegistry(hKey);
	m_LVConnectionsInfo.loadDataFromRegistry(hKey);
}