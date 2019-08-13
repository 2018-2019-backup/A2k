#include "StdAfx.h"
#include "SubStationData.h"

CSSData::CSSData(const CString &Database, const CString &UserName, const CString &Password)
		:m_bManualTapSetting(false)
{	
	m_DbMgr = CNSDatabaseMgr::getInstance();
	/*HRESULT hr = m_DbMgr->openDatabase(UserName,Password,Database);
	if(hr != NS_SUCCESS) 
	{
		m_DbMgr->closeDatabase();
	}	*/
	for(int i=0; i<5; i++)
	{
		m_bManualFuse[i] = false;
		m_bManualFuseA[i] = false;
	}
}
CSSData::~CSSData(void)
{	
	//m_DbMgr->closeDatabase();
}
void  CSSData::loadSchematicInfo()
{
	CString strType, strSize, strHVCEquip, strEFIA, strEFIB, strNoOfDist,strDynConditions;
	getSSType(strType);
	getSSSize(strSize);
	getHVCEquipment(strHVCEquip);
	strEFIA.FormatMessage(L"%1!d!",getHVCEFIA());
	strEFIB.FormatMessage(L"%1!d!",getHVCEFIB());
	strNoOfDist.FormatMessage(L"%1!d!",getLVCNoOfDistributors());
	variant_t vtVal;
	vtVal.SetString("");
	CString strQrySchematic("Select schematic,schem_visibility,lookupproperty from tblSubstation_Schematic ");
	strQrySchematic = strQrySchematic + "where TYPE = '" + strType + "' ";
	strQrySchematic = strQrySchematic + "and SIZE = '" + strSize + "' ";

	strQrySchematic = strQrySchematic + "and EFA = " + strEFIA + " ";
	strQrySchematic = strQrySchematic + "and EFB = " + strEFIB + " ";
	strQrySchematic = strQrySchematic + "and LV_OUT = " + strNoOfDist + " ";
	strDynConditions =CString("and HV_EQUIPMENT = '") + strHVCEquip + "' ";
	m_DbMgr->executeQuery(strQrySchematic + strDynConditions);
	m_DbMgr->getValue(L"schematic",vtVal);
	m_strSchematic = vtVal.bstrVal;
	if(m_strSchematic.IsEmpty())
	{
		strDynConditions =CString("and hv_equipment is null");
	}
	m_DbMgr->executeQuery(strQrySchematic + strDynConditions);
	m_DbMgr->getValue(L"schematic",vtVal);
	m_strSchematic = vtVal.bstrVal;
	vtVal.SetString("");
	m_DbMgr->getValue(L"schem_visibility",vtVal);
	m_strSchemVisibility = vtVal.bstrVal;
	vtVal.SetString("");
	m_DbMgr->getValue(L"lookupproperty",vtVal);
	m_strSchemLookupProp = vtVal.bstrVal;
}

void  CSSData::loadGeographicInfo(const CString &strOPtion)
{
	CString strType, strSize, strHVCEquip, strNoOfDist,strDynConditions;
	getSSType(strType);
	getSSSize(strSize);
	getHVCEquipment(strHVCEquip);
	strNoOfDist.FormatMessage(L"%1!d!",getLVCNoOfDistributors());
	variant_t vtVal;
	vtVal.SetString("");
	CString strQrySchematic("Select schematic,schem_visibility,lookupproperty from tblSubstation_Geographic ");
	strQrySchematic = strQrySchematic + "where TYPE = '" + strType + "' ";
	strQrySchematic = strQrySchematic + "and SIZE = '" + strSize + "' ";
	strQrySchematic = strQrySchematic + "and LV_OUT = " + strNoOfDist + " ";
	strDynConditions =CString("and HV_EQUIPMENT = '") + strHVCEquip + "' ";
	strDynConditions =strDynConditions + "and OPTION = '" + strOPtion + "' ";
	m_DbMgr->executeQuery(strQrySchematic + strDynConditions);
	m_DbMgr->getValue(L"schematic",vtVal);
	m_strGeographic = vtVal.bstrVal;
	if( m_strGeographic.IsEmpty())
	{
		strDynConditions =CString("and hv_equipment is null ");
		strDynConditions =strDynConditions + "and option = '" + strOPtion + "' ";
		m_DbMgr->executeQuery(strQrySchematic + strDynConditions);
		m_DbMgr->getValue(L"schematic",vtVal);
		m_strGeographic = vtVal.bstrVal;
		if( m_strGeographic.IsEmpty())
		{		
			strDynConditions =CString("and option is null ");
			strDynConditions =strDynConditions + "and hv_equipment = '" + strHVCEquip + "' ";
			m_DbMgr->executeQuery(strQrySchematic + strDynConditions);
			m_DbMgr->getValue(L"schematic",vtVal);
			m_strGeographic = vtVal.bstrVal;
			if( m_strGeographic.IsEmpty())
			{		
				strDynConditions =CString("and hv_equipment is null ");
				strDynConditions =strDynConditions + "and option is null ";
				m_DbMgr->executeQuery(strQrySchematic + strDynConditions);
				m_DbMgr->getValue(L"schematic",vtVal);
			}	
		}	
	}
	m_strGeographic = vtVal.bstrVal;
	vtVal.SetString("");
	m_DbMgr->getValue(L"schem_visibility",vtVal);
	m_strGeoVisibility = vtVal.bstrVal;
	vtVal.SetString("");
	m_DbMgr->getValue(L"lookupproperty",vtVal);
	m_strGeoLookupProp = vtVal.bstrVal;
}

void CSSData::loadDistributorsBlockInfo()
{	
	CString strType, strSize, strNoOfDist, strWDNO, strIDLINK, strLVSeq, strFuseA, strFuseType;	
	int iNoOfDistributors = getLVCNoOfDistributors();
	strNoOfDist.FormatMessage(L"%1!d!",iNoOfDistributors);
	variant_t vtVal;
	for(int index = 0; index < iNoOfDistributors; index++)
	{
		strLVSeq.FormatMessage(L"%1!d!",index + 1);
		getSSType(strType);
		getSSSize(strSize);
		int iLVCWDNO = getLVCWDNO(index);
		int iIDLINK = getLVCIDLINK(index);
		strFuseA.FormatMessage(L"%1!d!",getLVCFuseA(index));
		getLVCFuse(index,strFuseType);
		vtVal.intVal = 0;	
		CString strQryFuse("Select fuse from tblSubstation_LV_Fuse");
		strQryFuse = strQryFuse + " where lv_type = '" + strFuseType + "' ";
		strQryFuse = strQryFuse + " and size = '" + strSize + "' ";
		strQryFuse = strQryFuse + " and lv_amps = '" + strFuseA + "' ";
		m_DbMgr->executeQuery(strQryFuse);
		m_DbMgr->getValue(L"fuse",vtVal);
		CString strFuse;
		if(vtVal.intVal == 1)
		{
			strFuse.FormatMessage(L"%1!d!",vtVal.intVal);
		}

		vtVal.SetString("");
		CString strQrySchematic("Select schematic,schem_visibility,x_position, y_position, rotation");
		strQrySchematic = strQrySchematic + " from tblSubstation_LV_Output ";
		strQrySchematic = strQrySchematic + " where type = '" + strType + "' ";
		strQrySchematic = strQrySchematic + " and size = '" + strSize + "' ";		
		if(iLVCWDNO == 1)
		{
			strWDNO.FormatMessage(L"%1!d!",iLVCWDNO);
			strQrySchematic = strQrySchematic + " and wdno = '" + strWDNO + "' ";
		}
		else
		{
			strQrySchematic = strQrySchematic + " and wdno is null ";
		}
		if(iIDLINK == 1)
		{
			strIDLINK.FormatMessage(L"%1!d!",iIDLINK);
			strQrySchematic = strQrySchematic + " and id_link = '" + strIDLINK + "' ";
		}
		else
		{
			strQrySchematic = strQrySchematic + " and id_link is null ";
		}
		strQrySchematic = strQrySchematic + " and lv_out = " + strNoOfDist;
		strQrySchematic = strQrySchematic + " and lv_sequence = " + strLVSeq ;
		if(strFuse.IsEmpty())
		{
			strQrySchematic = strQrySchematic + " and fuse is null ";
		}
		else
		{
			strQrySchematic = strQrySchematic + " and fuse = " + strFuse ;
		}

		m_DbMgr->executeQuery(strQrySchematic);
		m_DbMgr->getValue(L"schematic",vtVal);
		m_strDistSchem[index] = vtVal.bstrVal;
		vtVal.SetString("");
		m_DbMgr->getValue(L"schem_visibility",vtVal);
		m_strDistSchemVisibility[index]= vtVal.bstrVal;
		vtVal.SetString("");
		m_DbMgr->getValue(L"x_position",vtVal);
		m_strDistXPos[index]= vtVal.bstrVal;
		vtVal.SetString("");
		m_DbMgr->getValue(L"y_position",vtVal);
		m_strDistYPos[index]= vtVal.bstrVal;
		m_DbMgr->getValue(L"rotation",vtVal);
		m_iDistRotation[index]= vtVal.intVal;
	}
}
void CSSData::getSSOptionsFromDB(std::vector<variant_t> &vOptions, const CString &strSSType, const CString &strSSSize, int item)
{
	vOptions.clear();
	CString strItem;
	strItem.FormatMessage(L"%1!d!",item);
	CString strQrySSOptions = "Select distinct option,sequence from tblSubstation_Details ";
	strQrySSOptions = strQrySSOptions + "where TYPE = '" + strSSType + "'";
	strQrySSOptions = strQrySSOptions + " and SIZE = '" + strSSSize + "'";
	strQrySSOptions = strQrySSOptions + " and ITEM = " + strItem ;
	strQrySSOptions = strQrySSOptions + " order by sequence";
	m_DbMgr->executeQuery(strQrySSOptions);
	m_DbMgr->getValue(L"option",vOptions);
}

void CSSData::getSSDetails(CString &strDetails, const CString &strOption, const CString &strSSType, const CString &strSSSize)
{
	variant_t vtVal;
	vtVal.SetString("");
	CString strQrySSDetails = "Select details from tblSubstation_Details ";
	strQrySSDetails = strQrySSDetails + " where type = '" + strSSType + "'";
	strQrySSDetails = strQrySSDetails + " and size = '" + strSSSize + "'";
	strQrySSDetails = strQrySSDetails + " and option = '" + strOption + "'";
	m_DbMgr->executeQuery(strQrySSDetails);
	m_DbMgr->getValue(L"details",vtVal);
	strDetails = vtVal.bstrVal;

}

void CSSData::getGeographic(CString &strGeographic)
{
	strGeographic = m_strGeographic;	
}
void CSSData::getGeoVisibility(CString &strGeoVisibility)
{
	strGeoVisibility = m_strGeoVisibility;	
}
void CSSData::getGeoLookupProp(CString &strGeoLookupProp)
{
	strGeoLookupProp = m_strGeoLookupProp;
}

void CSSData::getSchematic(CString &strSchematic)
{
	strSchematic = m_strSchematic;	
}
void CSSData::getSchemVisibility(CString &strSchemVisibility)
{
	strSchemVisibility = m_strSchemVisibility;	
}
void CSSData::getSchemLookupProp(CString &strSchemLookupProp)
{
	strSchemLookupProp = m_strSchemLookupProp;
}
void CSSData::getDistributorBlockInfo(int index, CString &strdistSchem, CString &strDistSchemVisibility
							   , CString &strDistXPos, CString &strDistYPos
							   , int iDistRotation)
{
	strdistSchem = m_strDistSchem[index-1];
	strDistSchemVisibility = m_strDistSchemVisibility[index-1];
	strDistXPos = m_strDistXPos[index-1];
	strDistYPos = m_strDistYPos[index-1];
	iDistRotation = m_iDistRotation[index-1];
}
void CSSData::getSSTypes(std::vector<variant_t> &vTypes)
{
	vTypes.clear();
	CString strQrySSType("Select distinct type from tblSubstation_Distribution order by type");
	m_DbMgr->executeQuery(strQrySSType);
	m_DbMgr->getValue(L"Type",vTypes);
}
void CSSData::getSSSizes(std::vector<variant_t> &vSizes, const CString &strType)
{
	vSizes.clear();
	CString strQrySSType = "Select distinct size from tblSubstation_Distribution ";
	if(strType != "")
		strQrySSType = strQrySSType + "where TYPE = '" + strType + "'";
	strQrySSType = strQrySSType + " order by size";
	m_DbMgr->executeQuery(strQrySSType);
	m_DbMgr->getValue(L"Size",vSizes);
}
void CSSData::getSSDescriptions(std::vector<variant_t> &vDescriptions, const CString &strType, const CString &strSize )
{
	vDescriptions.clear();
	CString strQrySSDescriptions = "Select distinct description from tblSubstation_Distribution ";
	if(!strType.IsEmpty())
	{
		strQrySSDescriptions = strQrySSDescriptions + "where TYPE = '" + strType + "' ";
		if(!strSize.IsEmpty())
		{
			strQrySSDescriptions = strQrySSDescriptions + "and SIZE = '" + strSize + "'";
		}
	}
	strQrySSDescriptions = strQrySSDescriptions + " order by description";
	m_DbMgr->executeQuery(strQrySSDescriptions);
	m_DbMgr->getValue(L"Description",vDescriptions);
}

void CSSData::getSSStockTypes(std::vector<variant_t> &vStockTypes, const CString &strType, const CString &strSize )
{
	vStockTypes.clear();
	CString strQrySSStockTypes = "Select distinct stock_type from tblSubstation_Distribution ";
	if(!strType.IsEmpty())
	{
		strQrySSStockTypes = strQrySSStockTypes + "where TYPE = '" + strType + "' ";
		if(!strSize.IsEmpty())
		{
			strQrySSStockTypes = strQrySSStockTypes + "and SIZE = '" + strSize + "'";
		}
	}
	strQrySSStockTypes = strQrySSStockTypes + " order by stock_type";
	m_DbMgr->executeQuery(strQrySSStockTypes);
	m_DbMgr->getValue(L"stock_type",vStockTypes);
}
void CSSData::getHVCEquipments(std::vector<variant_t> &vEquipments)
{	
	vEquipments.clear();
	CString strQryHVCEquips = "Select distinct hv_equipment from tblSubstation_HV_Protection order by hv_equipment";
	
	m_DbMgr->executeQuery(strQryHVCEquips);
	m_DbMgr->getValue(L"hv_equipment",vEquipments);
	return ; 

}

void CSSData::getHVCTypes(std::vector<variant_t> &vTypes, const CString &strHVCEquip)
{	
	vTypes.clear();
	if(strHVCEquip.IsEmpty()) return;
	CString strQryHVCTypes = "Select distinct hv_type from tblSubstation_HV_Protection ";
	strQryHVCTypes = strQryHVCTypes + "where HV_EQUIPMENT = '" + strHVCEquip + "' ";
	strQryHVCTypes = strQryHVCTypes + " order by HV_TYPE";
	
	m_DbMgr->executeQuery(strQryHVCTypes);
	m_DbMgr->getValue(L"hv_type",vTypes);
	return ; 

}
void CSSData::getHVCRatings(std::vector<variant_t> &vRatings, const CString &strHVCEquip, const CString &strHVCType)
{	
	vRatings.clear();
	if(strHVCType.IsEmpty()) return;
	CString strQryHVCRatings = "Select distinct hv_rating from tblSubstation_HV_Protection ";
	strQryHVCRatings = strQryHVCRatings + "where HV_EQUIPMENT = '" + strHVCEquip + "' ";
	strQryHVCRatings = strQryHVCRatings + "and HV_TYPE = '" + strHVCType + "' ";
	strQryHVCRatings = strQryHVCRatings + " order by hv_rating";
	
	m_DbMgr->executeQuery(strQryHVCRatings);
	m_DbMgr->getValue(L"hv_rating",vRatings);
	return ; 

}


void CSSData::getSSSize(CString &strSize)
{
	m_Substation.getSSSize(strSize);
}
bool CSSData::setSSSize(const CString &strSize)
{
	bool bRet = m_Substation.setSSSize(strSize);
	if(bRet == false)
	{
		CString strType;
		getSSType(strType);
		if(strType.IsEmpty() && !strSize.IsEmpty())
		{
			CString strQrySS = "Select type from tblSubstation_Distribution ";
			strQrySS = strQrySS + "where SIZE = '" + strSize + "'";
			m_DbMgr->executeQuery(strQrySS);
			variant_t vtVal;
			vtVal.SetString("");
			m_DbMgr->getValue(L"type",vtVal);
			strType = vtVal.bstrVal;
			m_Substation.setSSType(strType);
			//setSSFunctionAndLabel resets m_DbMgr recordSet
			setSSFunctionAndLabel(strType);
			bRet = m_Substation.setSSSize(strSize);
		}
	}
	if(bRet)
	{
		 setSSDescription("");
		 setSSStockType("");
	}
	return bRet;
}

void CSSData::getSSType(CString &strType)
{
	m_Substation.getSSType(strType);
}
bool CSSData::setSSType(const CString &strType)
{
	bool bRet = m_Substation.setSSType(strType);
	if(bRet)
	{
		setSSOptions("");
		if(setSSSize("") == false)
		{
			clearLVCInfo();
			setLVCNoOfDistributors(-1);
			setHVCEquipment("");
			setHVCType("");
			setTXRating("");
			setTXVoltage("");
			setTXPhases(0);
			setTXTapRatio("");
			setTXTapSetting(-1);
			m_bManualTapSetting = false;
		}
		setSSFunctionAndLabel(strType);
	}
	return bRet;
}
void CSSData::getSSDescription(CString &strDesc)
{
	m_Substation.getSSDescription(strDesc);
}
bool CSSData::setSSDescription(const CString &strDesc)
{	
	bool bRet = m_Substation.setSSDescription(strDesc);
	if(bRet && strDesc.IsEmpty() == false)
	{
		CString strQrySS = "Select type, size, stock_type, hv_equipment, hv_type, hv_rating,";
		strQrySS = strQrySS + " tx_rating, voltage, phases from tblSubstation_Distribution ";
		strQrySS = strQrySS + "where DESCRIPTION = '" + strDesc + "'";
		m_DbMgr->executeQuery(strQrySS);
		variant_t vtVal;
		CString strType;
		vtVal.SetString("");
		m_DbMgr->getValue(L"stock_type",vtVal);	
		m_Substation.setSSStockType(CString(vtVal.bstrVal));
		vtVal.SetString("");
		m_DbMgr->getValue(L"Type",vtVal);
		strType = vtVal.bstrVal;
		vtVal.SetString("");
		m_Substation.setSSType(strType);
		//setsize call should be always after setting type	
		m_DbMgr->getValue(L"Size",vtVal);
		m_Substation.setSSSize(CString(vtVal.bstrVal));
		vtVal.SetString("");
		m_DbMgr->getValue(L"hv_equipment",vtVal);	
		setHVCEquipment(CString(vtVal.bstrVal));
		vtVal.SetString("");
		m_DbMgr->getValue(L"hv_type",vtVal);	
		setHVCType(CString(vtVal.bstrVal));
		vtVal.SetString("");
		m_DbMgr->getValue(L"hv_rating",vtVal);	
		setHVCRating(CString(vtVal.bstrVal));
		vtVal.SetString("");
		m_DbMgr->getValue(L"tx_rating",vtVal);	
		setTXRating(CString(vtVal.bstrVal));
		vtVal.SetString("");
		m_DbMgr->getValue(L"voltage",vtVal);	
		setTXVoltage(CString(vtVal.bstrVal));

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Code modified by SJ, DCS, 28.06.2013
    //
    // Re-using the same vtVal for both strings and double seems to throw an un-handled exception.
    // Therefore, a new variant value is used for this setting. Works fine now, 32-bit and 64-bit.
    //
    //vtVal.dblVal=-1;
    //m_DbMgr->getValue(L"phases",vtVal);	
    //setTXPhases(vtVal.dblVal);
    //
    variant_t vtNewVal;
		vtNewVal.dblVal=-1;
		m_DbMgr->getValue(L"phases",vtNewVal);	
		if(vtNewVal.vt!=VT_EMPTY)
		{
		  vtNewVal.vt=VT_R8;
		  vtNewVal.dblVal=wcstod(vtNewVal.bstrVal,NULL);
		}
    setTXPhases(vtNewVal.dblVal);
    // End code modified by SJ, DCS, 28.06.2013
    //////////////////////////////////////////////////////////////////////////////////////////////////
		
    //setSSFunctionAndLabel resets m_DbMgr recordSet
		setSSFunctionAndLabel(strType);
		
    //int iNoOfLVCDistributors = getLVCNoOfDistributors(strDesc);
		
    setTXTapRatio("");
		setTXTapSetting(-1);
		m_bManualTapSetting = false;
		clearLVCInfo();
		setLVCNoOfDistributors(-1);
	}
	else if(bRet == true)
	{
		clearLVCInfo();
		setLVCNoOfDistributors(-1);
	}
	return bRet;
}

void CSSData::getSSStockType(CString &strStockType)
{
	m_Substation.getSSStockType(strStockType);
}
bool CSSData::setSSStockType(const CString &strStockType)
{	bool bRet = m_Substation.setSSStockType(strStockType);
	if(bRet && strStockType.IsEmpty() == false)
	{
		CString strQrySS = "Select type ,size, description from tblSubstation_Distribution ";
		strQrySS = strQrySS + "where STOCK_TYPE = '" + strStockType + "'";
		m_DbMgr->executeQuery(strQrySS);
		variant_t vtVal;
		CString strType;
		vtVal.SetString("");
		m_DbMgr->getValue(L"description",vtVal);	
		setSSDescription(CString(vtVal.bstrVal));
		//int iNoOfLVCDistributors = getLVCNoOfDistributors(CString(vtVal.bstrVal));
		//setLVCNoOfDistributors(iNoOfLVCDistributors);
		//vtVal.SetString("");
		//m_DbMgr->getValue(L"Type",vtVal);
		//strType = vtVal.bstrVal;
		//vtVal.SetString("");
		//m_Substation.setSSType(strType);	
		//m_DbMgr->getValue(L"Size",vtVal);
		////setsize call should be always after setting type
		//m_Substation.setSSSize(CString(vtVal.bstrVal));
		////setSSFunctionAndLabel resets m_DbMgr recordSet
		//setSSFunctionAndLabel(strType);		
	}
	return bRet;
}
bool CSSData::setSSFunctionAndLabel(const CString &strType)
{
	if(strType.IsEmpty() == false)
	{
		CString strQrySS = "Select distinct label, function from tblSubstation_Details ";
		strQrySS = strQrySS + "where TYPE = '" + strType + "'";		
		m_DbMgr->executeQuery(strQrySS);
		variant_t vtVal;
		vtVal.SetString("");
		m_DbMgr->getValue(L"label",vtVal);	
		m_Substation.setSSLabel(CString(vtVal.bstrVal));
		vtVal.SetString("");
		m_DbMgr->getValue(L"function",vtVal);
		m_Substation.setSSFunction(CString(vtVal.bstrVal));
	}
	else
	{
		m_Substation.setSSLabel("Substation Details");
		m_Substation.setSSFunction("");
	}
	return true;
}

void CSSData::getSSFunction(CString &strFunction)
{
	m_Substation.getSSFunction(strFunction);
}
void CSSData::getSSLabel(CString &strLabel)
{
	m_Substation.getSSLabel(strLabel);
}
void CSSData::getSSPrefix(CString &strPrefix)
{
	m_Substation.getSSPrefix(strPrefix);
}
bool CSSData::setSSPrefix(const CString &strPrefix)
{
	return m_Substation.setSSPrefix(strPrefix);
}	
void CSSData::getSSName(CString &strName)
{
	m_Substation.getSSName(strName);
}
bool CSSData::setSSName(const CString &strName)
{
	return m_Substation.setSSName(strName);
}	
void CSSData::getSSNumber(CString &strNumber)
{
	m_Substation.getSSNumber(strNumber);
}
bool CSSData::setSSNumber(const CString &strNumber)
{
	return m_Substation.setSSNumber(strNumber);
}
void CSSData::getSSOptions(CString &strOptions)
{
	m_Substation.getSSOptions(strOptions);
}
bool CSSData::setSSOptions(const CString &strOptions)
{
	return m_Substation.setSSOptions(strOptions);
}

void CSSData::getHVCType(CString &strType)
{
	m_Substation.getHVCType(strType);	
}
bool CSSData::setHVCType(const CString &strType)
{
	bool bRet = m_Substation.setHVCType(strType);	
	if(bRet)
	{
		setHVCRating("");
	}
	return bRet;
}
void CSSData::getHVCEquipment(CString &strEquipment)
{
	m_Substation.getHVCEquipment(strEquipment);	
}
bool CSSData::setHVCEquipment(const CString &strEquipment)
{	
	bool bRet = m_Substation.setHVCEquipment(strEquipment);	
	if(bRet)
	{
		setHVCType("");
	}
	return bRet;
}
void CSSData::getHVCRating(CString &strRating)
{
	m_Substation.getHVCRating(strRating);	
}
bool CSSData::setHVCRating(const CString &strRating)
{
	return m_Substation.setHVCRating(strRating);	
}
int CSSData::getHVCEFIA()
{
	return m_Substation.getHVCEFIA();
}
bool CSSData::setHVCEFIA(const int &iEFIA)
{
	return m_Substation.setHVCEFIA(iEFIA);
}
int CSSData::getHVCEFIB()
{
	return m_Substation.getHVCEFIB();
}
bool CSSData::setHVCEFIB(const int &iEFIB)
{
	return m_Substation.setHVCEFIB(iEFIB);
}
void CSSData::getHVCSideA(CString &strSideA)
{
	m_Substation.getHVCSideA(strSideA);	
}
bool CSSData::setHVCSideA(const CString &strSideA)
{
	return m_Substation.setHVCSideA(strSideA);
}
void CSSData::getHVCSideB(CString &strSideB)
{
	m_Substation.getHVCSideB(strSideB);	
}
bool CSSData::setHVCSideB(const CString &strSideB)
{
	return m_Substation.setHVCSideB(strSideB);
}
void CSSData::getHVCNumber(CString &strNumber)
{
	m_Substation.getHVCNumber(strNumber);	
}
bool CSSData::setHVCNumber(const CString &strNumber)
{
	return m_Substation.setHVCNumber(strNumber);
}
void CSSData::getTxRatings(std::vector<variant_t> &vRatings, const CString &strSSSize)
{	
	vRatings.clear();
	if(strSSSize.IsEmpty()) return;
	CString strQryTXRatings = "Select distinct tx_rating from tblSubstation_Transformer ";
	strQryTXRatings = strQryTXRatings + "where SIZE = '" + strSSSize + "' ";
	strQryTXRatings = strQryTXRatings + " order by tx_rating";
	
	m_DbMgr->executeQuery(strQryTXRatings);
	m_DbMgr->getValue(L"tx_rating",vRatings);
	return ; 

}

void CSSData::getTXVoltage(variant_t &vtVoltage,const CString &strSSSize,const CString &strRating)
{
	vtVoltage.SetString("");
	if(strSSSize.IsEmpty() || strRating.IsEmpty()) return;
	CString strQryTXVoltage = "Select voltage from tblSubstation_Transformer ";
	strQryTXVoltage = strQryTXVoltage + "where SIZE = '" + strSSSize + "' ";
	strQryTXVoltage = strQryTXVoltage + "and tx_rating = '" + strRating + "' ";
	
	m_DbMgr->executeQuery(strQryTXVoltage);
	m_DbMgr->getValue(L"voltage",vtVoltage);
	return ; 
}	
int CSSData::getTXPhases(const CString &strSSSize,const CString &strRating)
{
	variant_t vtPhase;
	vtPhase.intVal = -1;
	if(strSSSize.IsEmpty() || strRating.IsEmpty()) return vtPhase.intVal;
	CString strQryTXPhases = "Select phases from tblSubstation_Transformer ";
	strQryTXPhases = strQryTXPhases + "where SIZE = '" + strSSSize + "' ";
	strQryTXPhases = strQryTXPhases + "and tx_rating = '" + strRating + "' ";
	
	m_DbMgr->executeQuery(strQryTXPhases);
	m_DbMgr->getValue(L"phases",vtPhase);
	return vtPhase.intVal; 
}
void CSSData::getTXTapRatios(std::vector<variant_t> &vTapRatios, const CString &strSSSize)
{	
	vTapRatios.clear();
	if(strSSSize.IsEmpty()) return;
	CString strQryTXTapRatios = "Select distinct ratio from tblSubstation_Tap_Settings ";
	strQryTXTapRatios = strQryTXTapRatios + "where SIZE = '" + strSSSize + "' ";
	strQryTXTapRatios = strQryTXTapRatios + " order by ratio";
	
	m_DbMgr->executeQuery(strQryTXTapRatios);
	m_DbMgr->getValue(L"ratio",vTapRatios);
	return ; 

}
int CSSData::getTXTapSettingFromDB(const CString &strSSSize,const CString &strTXTapRatio)
{
	variant_t vtTapSetting;
	vtTapSetting.intVal = -1;
	if(strSSSize.IsEmpty() || strTXTapRatio.IsEmpty()) return vtTapSetting.intVal;
	CString strQryTXTapSetting = "Select position from tblSubstation_Tap_Settings ";
	strQryTXTapSetting = strQryTXTapSetting + "where SIZE = '" + strSSSize + "' ";
	strQryTXTapSetting = strQryTXTapSetting + "and RATIO = '" + strTXTapRatio + "' ";
	
	m_DbMgr->executeQuery(strQryTXTapSetting);
	m_DbMgr->getValue(L"position",vtTapSetting);
	return vtTapSetting.intVal; 
}
void CSSData::getTXMDIVals(CString &strCTRatio, CString &strKFactor, const CString &strDesc)
{	
	if(strDesc.IsEmpty()) return;
	
	CString strQryTXMDI = "Select mdi_ct,mdi_ratio from tblSubstation_Distribution ";
	strQryTXMDI = strQryTXMDI + "where DESCRIPTION = '" + strDesc + "' ";
	variant_t vtVal;
	vtVal.SetString("");
	m_DbMgr->executeQuery(strQryTXMDI);
	m_DbMgr->getValue(L"mdi_ct",vtVal);
	strCTRatio = vtVal.bstrVal;
	vtVal.SetString("");
	m_DbMgr->getValue(L"mdi_ratio",vtVal);
	strKFactor = vtVal.bstrVal;
	return ; 
}

void CSSData::getTXPoleStrengths(std::vector<variant_t> &vPoleStrentghs
								 , const CString &strSSSize
								 , const CString &strTXRating)
{	
	vPoleStrentghs.clear();
	CString strQryTXPoleStrengths = "Select distinct pole_strength from tblSubstation_Transformer ";
	strQryTXPoleStrengths = strQryTXPoleStrengths + "where SIZE = '" + strSSSize + "' ";
	strQryTXPoleStrengths = strQryTXPoleStrengths + "and TX_RATING = '" + strTXRating + "' ";
	strQryTXPoleStrengths = strQryTXPoleStrengths + " order by pole_strength ";
	m_DbMgr->executeQuery(strQryTXPoleStrengths);
	m_DbMgr->getValue(L"pole_strength",vPoleStrentghs);

}
void CSSData::getTXPoleLengths(std::vector<variant_t> &vPoleLengths
							   , const CString &strSSSize
							   , const CString &strTXRating)
{
	vPoleLengths.clear();
	CString strQryTXPoleLengths = "Select distinct pole_length from tblSubstation_Transformer ";
	strQryTXPoleLengths = strQryTXPoleLengths + "where SIZE = '" + strSSSize + "' ";
	strQryTXPoleLengths = strQryTXPoleLengths + "and TX_RATING = '" + strTXRating + "' ";
	strQryTXPoleLengths = strQryTXPoleLengths + " order by pole_length ";
	m_DbMgr->executeQuery(strQryTXPoleLengths);
	m_DbMgr->getValue(L"pole_length",vPoleLengths);
}

void  CSSData::getLVCFuseAList(std::vector<variant_t> &vFuseAList, const int iDistNo)
{
	vFuseAList.clear();
	CString strDesc,strSSSize;
	getSSDescription(strDesc);
	if(strDesc.IsEmpty()) return;
	getSSSize(strSSSize);


	CString strQryLVCFuseAList = "Select distinct lv_amps from tblSubstation_LV_Fuse ";
	strQryLVCFuseAList = strQryLVCFuseAList + "where SIZE = '" + strSSSize + "' ";
	strQryLVCFuseAList = strQryLVCFuseAList + " order by lv_amps";
	m_DbMgr->executeQuery(strQryLVCFuseAList);
	m_DbMgr->getValue(L"lv_amps",vFuseAList);
}


void CSSData::getLVCFuseList(std::vector<variant_t> &vFuseList, const int FuseA, const int iDistNo)
{
	vFuseList.clear();
	CString strDesc, strSSSize;
	getSSDescription(strDesc);
	if(strDesc.IsEmpty()) return;
	getSSSize(strSSSize);
	CString strFuseA;
	strFuseA.FormatMessage(L"%1!d!",FuseA);


	CString strQryLVCFuseList = "Select distinct lv_type from tblSubstation_LV_Fuse ";
	strQryLVCFuseList = strQryLVCFuseList + "where SIZE = '" + strSSSize + "' ";
	strQryLVCFuseList = strQryLVCFuseList + "and LV_AMPS = '" + strFuseA + "' ";
	strQryLVCFuseList = strQryLVCFuseList + " order by lv_type";
	m_DbMgr->executeQuery(strQryLVCFuseList);
	m_DbMgr->getValue(L"lv_type",vFuseList);
}

int CSSData::getLVCFuseAFromDB(const CString &strDesc, const int iDistNo)
{
	if(strDesc.IsEmpty()) return 0;

	CString strColumnName;
	strColumnName.FormatMessage(L"LV_FUSE%1!d!_AMPS",iDistNo+1);
	CString strQryLVCFuseA = L"Select " + strColumnName + " from tblSubstation_Distribution ";
	strQryLVCFuseA = strQryLVCFuseA + "where DESCRIPTION = '" + strDesc + "' ";
	variant_t vtVal;
	vtVal.dblVal = -1;
	m_DbMgr->executeQuery(strQryLVCFuseA);
	m_DbMgr->getValue(strColumnName,vtVal);
		if(vtVal.vt!=VT_EMPTY)
		{
			vtVal.vt=VT_R8;
			vtVal.dblVal=wcstod(vtVal.bstrVal,NULL);
		}
	return vtVal.dblVal;
}int CSSData::getLVCPanelRatingFromDB(const CString &strDesc, const int iDistNo)
{
	if(strDesc.IsEmpty()) return 0;

	CString strColumnName;
	strColumnName.FormatMessage(L"LV_P%1!d!_AMPS",iDistNo+1);
	CString strQryLVCFuseA = L"Select " + strColumnName + " from tblSubstation_Distribution ";
	strQryLVCFuseA = strQryLVCFuseA + "where DESCRIPTION = '" + strDesc + "' ";
	variant_t vtVal;
	vtVal.dblVal = -1;
	m_DbMgr->executeQuery(strQryLVCFuseA);
	m_DbMgr->getValue(strColumnName,vtVal);
		if(vtVal.vt!=VT_EMPTY)
	{
		vtVal.vt=VT_R8;
		vtVal.dblVal=wcstod(vtVal.bstrVal,NULL);
		}
	return vtVal.dblVal;
}
void CSSData::getLVCFuseFromDB(const CString &strDesc, const int iDistNo, CString &strFuse)
{
	if(strDesc.IsEmpty()) return ;

	CString strColumnName;
	strColumnName.FormatMessage(L"LV_FUSE%1!d!_TYPE",iDistNo+1);
	CString strQryLVCFuse = L"Select " + strColumnName + " from tblSubstation_Distribution ";
	strQryLVCFuse = strQryLVCFuse + "where DESCRIPTION = '" + strDesc + "' ";
	variant_t vtVal;
	vtVal.SetString("");
	m_DbMgr->executeQuery(strQryLVCFuse);
	m_DbMgr->getValue(strColumnName,vtVal);
	strFuse = vtVal.bstrVal;
}
int  CSSData::getLVCNoOfDistributors(const CString &strSSDesc)
{
	variant_t vtNoOfDistributors;
	vtNoOfDistributors.intVal = 0;
	if(strSSDesc.IsEmpty()) return vtNoOfDistributors.intVal;
	CString strQryLVNoOfDistributors = "Select lv_out from tblSubstation_Distribution ";
	strQryLVNoOfDistributors = strQryLVNoOfDistributors + "where DESCRIPTION = '" + strSSDesc + "' ";
	
	m_DbMgr->executeQuery(strQryLVNoOfDistributors);
	m_DbMgr->getValue(L"lv_out",vtNoOfDistributors);

		if(vtNoOfDistributors.vt!=VT_EMPTY)
		{
			vtNoOfDistributors.vt=VT_R8;
			vtNoOfDistributors.dblVal=wcstod(vtNoOfDistributors.bstrVal,NULL);
		}
	return vtNoOfDistributors.dblVal; 
}



void CSSData::getTXRating(CString &strRating)
{
	m_Substation.getTXRating(strRating);
}
bool CSSData::setTXRating(const CString &strRating)
{
	return m_Substation.setTXRating(strRating);
}

void CSSData::getTXVoltage(CString &strVoltage)
{
	m_Substation.getTXVoltage(strVoltage);
}
bool CSSData::setTXVoltage(const CString &strVoltage)
{
	return m_Substation.setTXVoltage(strVoltage);
}
int CSSData::getTXPhases()
{
	return m_Substation.getTXPhases();
}
bool CSSData::setTXPhases(const int strPhases)
{
	return m_Substation.setTXPhases(strPhases);
}
void CSSData::getTXTapRatio(CString &strTapRatio)
{
	m_Substation.getTXTapRatio(strTapRatio);
}
bool CSSData::setTXTapRatio(const CString &strTapRatio)
{
	return m_Substation.setTXTapRatio(strTapRatio);
}
int CSSData::getTXTapSetting()
{
	return m_Substation.getTXTapSetting();
}
bool CSSData::setTXTapSetting(const int iTapSetting)
{
	return m_Substation.setTXTapSetting(iTapSetting);
}
void CSSData::getTXCTRatio(CString &strCTRatio)
{
	m_Substation.getTXCTRatio(strCTRatio);
}
bool CSSData::setTXCTRatio(const CString &strCTRatio)
{
	return m_Substation.setTXCTRatio(strCTRatio);
}
void CSSData::getTXPoleLength(CString &strPoleLength)
{
	m_Substation.getTXPoleLength(strPoleLength);
}
bool CSSData::setTXPoleLength(const CString &strPoleLength)
{
	return m_Substation.setTXPoleLength(strPoleLength);
}
void CSSData::getTXPoleStrength(CString &strPoleStrength)
{
	m_Substation.getTXPoleStrength(strPoleStrength);
}
bool CSSData::setTXPoleStrength(const CString &strPoleStrength)
{
	return m_Substation.setTXPoleStrength(strPoleStrength);
}
void CSSData::getTXKFactor(CString &strKFactor)
{
	m_Substation.getTXKFactor(strKFactor);
}
bool  CSSData::setTXKFactor(const CString &strKFactor)
{
	return m_Substation.setTXKFactor(strKFactor);
}


int CSSData::getLVCNoOfDistributors()
{
	return m_Substation.getLVCNoOfDistributors();
	//if(iRET == 0)
	//{
	//	CString strDesc;
	//	getSSDescription(strDesc);
	//	iRET = getLVCNoOfDistributors(strDesc);
	//}
	//return iRET;
}
bool CSSData::setLVCNoOfDistributors(const int iNoOfDistributors)
{
	return m_Substation.setLVCNoOfDistributors(iNoOfDistributors);
}
int CSSData::getLVCPanelRating(const int index)
{
	return m_Substation.getLVCPanelRating(index);
}
bool CSSData::setLVCPanelRating(const int index, const int iPanelRating)
{
	return m_Substation.setLVCPanelRating(index,iPanelRating);
}
int CSSData::getLVCFuseA(const int index)
{
	return m_Substation.getLVCFuseA(index);
}
bool CSSData::setLVCFuseA(const int index, const int iFuseA)
{
	bool bRET = m_Substation.setLVCFuseA(index,iFuseA);
	if(bRET)
	{
		setLVCFuse(index,"");
	}
	return bRET;
}
int CSSData::getLVCWDNO(const int index)
{
	return m_Substation.getLVCWDNO(index);
}
bool CSSData::setLVCWDNO(const int index, const int iWDNO)
{
	return m_Substation.setLVCWDNO(index,iWDNO);
}
int CSSData::getLVCIDLINK(const int index)
{
	return m_Substation.getLVCIDLINK(index);
}
bool CSSData::setLVCIDLINK(const int index, const int iIDLINK)
{
	return m_Substation.setLVCIDLINK(index,iIDLINK);
}
void CSSData::getLVCDistributorName(const int index, CString &strDistributorName)
{
	m_Substation.getLVCDistributorName(index,strDistributorName);
}
bool CSSData::setLVCDistributorName(const int index, const CString &strDistributorName)
{
	return m_Substation.setLVCDistributorName(index,strDistributorName);
}
void CSSData::getLVCFuse(const int index, CString &strFuse)
{
	m_Substation.getLVCFuse(index,strFuse);
}
bool CSSData::setLVCFuse(const int index, const CString &strFuse)
{
	return m_Substation.setLVCFuse(index,strFuse);
}
int CSSData::getLVCDistributorNo()
{
	return m_Substation.getLVCDistributorNo();
}
bool CSSData::setLVCDistributorNo(const int iDistributorNO)
{
	return m_Substation.setLVCDistributorNo(iDistributorNO);
}
void CSSData::getLVCDistCaption(const int index, CString &strDistCaption)
{
	m_Substation.getLVCDistCaption(index, strDistCaption);
}
bool CSSData::setLVCDistCaption(const int index, const CString &strDistCaption)
{
	return m_Substation.setLVCDistCaption(index, strDistCaption);
}
void CSSData::clearLVCInfo(int index)
{
	m_Substation.clearLVCInfo(index);
	for(int i=0; i<5; i++)
	{
		m_bManualFuse[i] = false;
		m_bManualFuseA[i] = false;
	}
}
void CSSData::setDataToRegistry()
{
	HKEY hKey;
	DWORD dwDisposition; 
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\SUBSTN"), 0, NULL,REG_OPTION_VOLATILE,
		KEY_SET_VALUE, NULL, &hKey, &dwDisposition)!= ERROR_SUCCESS)
	{     
		::MessageBox(NULL,L"Registry Not Created! Please check if you have Admin Privilege",_T("NET CAD"),MB_OK|MB_ICONERROR);
		return;
	}
	m_Substation.setInfoToRegistry(hKey);
	RegCloseKey(hKey);
}
void CSSData::loadDataFromRegistry()
{
	HKEY hKey;
	//DWORD dwDisposition; 
	//if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\SUBSTN"), 0, NULL,REG_OPTION_VOLATILE,
	//	KEY_SET_VALUE, NULL, &hKey, &dwDisposition)!= ERROR_SUCCESS)
	//{     
	//	::MessageBox(NULL,L"Registry Not Created! Please check if you have Admin Privilege",MB_ICONERROR);
	//	return;
	//}
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\SUBSTN"), 0, KEY_ALL_ACCESS, &hKey)!=ERROR_SUCCESS)
	{     
		//::MessageBox(NULL,L"No Previous Data Found",MB_ICONERROR);
		return;
	}
	m_Substation.loadDataFromRegistry(hKey);
	RegCloseKey(hKey);
}
