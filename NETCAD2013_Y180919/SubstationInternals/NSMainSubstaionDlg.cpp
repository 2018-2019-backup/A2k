// NSMainSubstaionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NSMainSubstaionDlg.h"

#include "NSSubstationDet_Kiosk.h"
#include "NSSubstationDet_PT.h"
#include "LayOutSelDlg.h"


#include "NSDWGMgr.h"

// CNSMainSubstaionDlg dialog

IMPLEMENT_DYNAMIC(CNSMainSubstaionDlg, CAcUiDialog)

//CNSMainSubstaionDlg::CNSMainSubstaionDlg(CWnd* pParent /*=NULL*/)
//	: CAcUiDialog(CNSMainSubstaionDlg::IDD, pParent)
//	, m_iHVCEfiB(0)
//	, m_iHVCEfiA(0)
//	, m_strSSPrefix(_T(""))
//	, m_strSSNumber(_T(""))
//	, m_strSSName(_T(""))
//	, m_strHVCSideA(_T(""))
//	, m_strHVCSideB(_T(""))
//	, m_strHVCNumber(_T(""))
//	, m_strSSOptions(_T(""))
//{
//	m_labelFont.CreateFont(22, 0, 0, 0,
//		FW_BOLD, FALSE, FALSE, FALSE,
//        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
//		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,           // nQuality
//   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
//   L"Arial"); 
//	m_pSSData = new CSSData("SUBSTN");
//
//}

//CNSMainSubstaionDlg(,CWnd* pParent = NULL );  // edit mode

CNSMainSubstaionDlg::CNSMainSubstaionDlg(MYSTRINGSTRINGMAP mapOfAttributesFromBlock,  CWnd* pParent /*=NULL*/)
	: CAcUiDialog(CNSMainSubstaionDlg::IDD, pParent)
	, m_iHVCEfiB(0)
	, m_iHVCEfiA(0)
	, m_strSSPrefix(_T(""))
	, m_strSSNumber(_T(""))
	, m_strSSName(_T(""))
	, m_strHVCSideA(_T(""))
	, m_strHVCSideB(_T(""))
	, m_strHVCNumber(_T(""))
	, m_strSSOptions(_T(""))
	,m_bInitFromRegistry(true)
{
	m_labelFont.CreateFont(22, 0, 0, 0,
		FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,           // nQuality
   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
   L"Arial"); 

   m_pSSData = new CSSData("SUBSTN");	
   mapOfAttributes = mapOfAttributesFromBlock;   
   IsInEditMode = false;

}

CNSMainSubstaionDlg::CNSMainSubstaionDlg(MYSTRINGSTRINGMAP mapOfAttributesFromBlock, AcDbHandle handle, CWnd* pParent /*=NULL*/)
	: CAcUiDialog(CNSMainSubstaionDlg::IDD, pParent)
	, m_iHVCEfiB(0)
	, m_iHVCEfiA(0)
	,m_bInitFromRegistry(false)
{
	m_labelFont.CreateFont(22, 0, 0, 0,
		FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,           // nQuality
   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
   L"Arial"); 

   m_pSSData = new CSSData("SUBSTN");	
   mapOfAttributes = mapOfAttributesFromBlock;
   m_handle = handle;
   IsInEditMode = true;

}

void CNSMainSubstaionDlg::setUIControlFromAttributeValues()
{
	    NSSTRING st ;

		//// some values are not stored in attributes , but yet they are required to populate the main UI in eidt mode
		//// so those data is stored as XDATA and read here 

		ads_name entName;
		ACHAR *szhandle = new ACHAR[20];
		//m_handle.getIntoAsciiBuffer(szhandle);
		m_handle.getIntoAsciiBuffer(szhandle,sizeof(szhandle));
		acdbHandEnt(szhandle,entName);
		delete szhandle;//*subir

		AcDbObjectId objID;
		acdbGetObjectId(objID,entName );

		CNSDWGMgr ThisDrawing;
		std::map<NSSTRING, NSSTRING> mapXData;
		ThisDrawing.fillXDATAMAP(objID,mapXData);

		
		
		st = getXDATA(_T("HVCNumber"),mapXData);		m_pSSData->setHVCNumber(st.c_str());
		st = getXDATA(_T("HVCEquip"),mapXData);			m_pSSData->setHVCEquipment(st.c_str());
		st = getXDATA(_T("HVCType"),mapXData);			m_pSSData->setHVCType(st.c_str());
		st = getXDATA(_T("HVCRating"),mapXData);		m_pSSData->setHVCRating(st.c_str());
		st = getXDATA(_T("HVCSideA"),mapXData);			m_pSSData->setHVCSideA(st.c_str());
		st = getXDATA(_T("HVCSideB"),mapXData);			m_pSSData->setHVCSideB(st.c_str());
		st = getXDATA(_T("HVCEfiA"),mapXData);			m_pSSData->setHVCEFIA(_wtoi(st.c_str()));
		st = getXDATA(_T("HVCEfiB"),mapXData);			m_pSSData->setHVCEFIB(_wtoi(st.c_str()));

		st = getXDATA(_T("TXRating"),mapXData);			m_pSSData->setTXRating(st.c_str());
		st = getXDATA(_T("TXVoltage"),mapXData);		m_pSSData->setTXVoltage(st.c_str());
		st = getXDATA(_T("TXPhases"),mapXData);			m_pSSData->setTXPhases(_wtoi(st.c_str()));
		st = getXDATA(_T("TXTapRatio"),mapXData);		m_pSSData->setTXTapRatio(st.c_str());
		st = getXDATA(_T("TXTapSetting"),mapXData);		m_pSSData->setTXTapSetting(_wtoi(st.c_str()));
		st = getXDATA(_T("TXCTRatio"),mapXData);		m_pSSData->setTXCTRatio(st.c_str());
		st = getXDATA(_T("TXKFactor"),mapXData);		m_pSSData->setTXKFactor(st.c_str());
		st = getXDATA(_T("TXPoleLength"),mapXData);		m_pSSData->setTXPoleLength(st.c_str());
		st = getXDATA(_T("TXPoleStrength"),mapXData);	m_pSSData->setTXPoleStrength(st.c_str());

		st = getXDATA(_T("DISTRIBUTOR_NUMBER"),mapXData);	m_pSSData->setLVCNoOfDistributors(_wtoi(st.c_str()));
		int iLV_Out = _wtoi(st.c_str());
		////

		
		st = getAttributeValue(_T("SIZE"));					m_pSSData->setSSSize(st.c_str());
		st = getAttributeValue(_T("PREFIX"));				m_pSSData->setSSPrefix(st.c_str());
		st = getAttributeValue(_T("NUMBER"));				m_pSSData->setSSNumber(st.c_str());
		st = getAttributeValue(_T("NAME"));					m_pSSData->setSSName(st.c_str());
		st = getAttributeValue(_T("STOCK_TYPE"));			m_pSSData->setSSStockType(st.c_str());
		st = getAttributeValue(_T("HV_NUMBER"));			m_pSSData->setHVCNumber(st.c_str());
		st = getAttributeValue(_T("HV_TYPE"));				m_pSSData->setHVCType(st.c_str());
		st = getAttributeValue(_T("HV_RATING"));			m_pSSData->setHVCRating(st.c_str());
		st = getAttributeValue(_T("HVSIDE_1"));				m_pSSData->setHVCSideA(st.c_str());
		st = getAttributeValue(_T("HVSIDE_2"));				m_pSSData->setHVCSideB(st.c_str());
		st = getAttributeValue(_T("TXRATING"));				m_pSSData->setTXRating(st.c_str());
		st = getAttributeValue(_T("TXRATIO"));				m_pSSData->setTXTapRatio(st.c_str());
		st = getAttributeValue(_T("TXTAPSET"));				
		if(st==L"") st = L"-1";								m_pSSData->setTXTapSetting(_wtoi(st.c_str()));
		st = getAttributeValue(_T("MDICT"));				m_pSSData->setTXCTRatio(st.c_str());
		st = getAttributeValue(_T("MDICTRATIO"));			m_pSSData->setTXKFactor(st.c_str());
		//st = getAttributeValue(_T("DISTRIBUTOR_NUMBER"));	
		m_pSSData->setLVCNoOfDistributors(iLV_Out);


		st = getAttributeValue(_T("OPTIONS"));	
		/*if(st.empty())
		{
			st = getXDATA(_T("OPTIONS"),mapXData);	
		}*/	    		
		m_pSSData->setSSOptions(st.c_str());
		
		for(int index = 0; index < iLV_Out ; index++)
		{
			CString strDistNumber,strDistName,strPanelRating,strFuseRating,strArmType,strWdno,strIdlink;
			strDistNumber.FormatMessage(L"DISTRIBUTOR_NUMBER%1!d!",index+1);
			strDistName.FormatMessage(L"DISTRIBUTOR_NAME%1!d!",index+1);
			strPanelRating.FormatMessage(L"PANEL_RATING%1!d!",index+1);
			strFuseRating.FormatMessage(L"FUSE_RATING%1!d!",index+1);
			strArmType.FormatMessage(L"FUSE_ARMTYPE%1!d!",index+1);
			strWdno.FormatMessage(L"WDNO%1!d!",index+1);
			strIdlink.FormatMessage(L"IDLINK%1!d!",index+1);

			
			st = getAttributeValue(strDistNumber.GetBuffer());	m_pSSData->setLVCDistCaption(index, st.c_str());
			st = getAttributeValue(strDistName.GetBuffer());	m_pSSData->setLVCDistributorName(index, st.c_str());
			st = getAttributeValue(strPanelRating.GetBuffer());	
			if(st == L"") st = L"-1";								m_pSSData->setLVCPanelRating(index, _wtoi(st.c_str()));
			st = getAttributeValue(strFuseRating.GetBuffer());	
			if(st == L"") st = L"-1";								m_pSSData->setLVCFuseA(index, _wtoi(st.c_str()));
			st = getAttributeValue(strArmType.GetBuffer());		m_pSSData->setLVCFuse(index, st.c_str());
			st = getXDATA(strWdno.GetBuffer(),mapXData);		m_pSSData->setLVCWDNO(index, _wtoi(st.c_str()));
			st = getXDATA(strIdlink.GetBuffer(),mapXData);		m_pSSData->setLVCIDLINK(index,_wtoi(st.c_str()));

		}
}

NSSTRING CNSMainSubstaionDlg::getAttributeValue(NSSTRING attName)
{
	   //const TCHAR *pStr = NULL;
	   MYSTRINGSTRINGMAP::iterator m_AttMapIterator;
	   m_AttMapIterator = mapOfAttributes.find(attName);
	   if(m_AttMapIterator != mapOfAttributes.end())
	   {
		   NSSTRING tagname =  (*m_AttMapIterator).first;
		   NSSTRING attVal =  (*m_AttMapIterator).second;

		   return attVal;
	   }
	   else
	   {
		   return _T("");
	   }	
}


NSSTRING CNSMainSubstaionDlg::getXDATA(NSSTRING Name,std::map<NSSTRING, NSSTRING> mapXData)
{
	   //const TCHAR *pStr = NULL;
	   MYSTRINGSTRINGMAP::iterator m_MapIterator;
	   m_MapIterator = mapXData.find(Name);
	   if(m_MapIterator != mapXData.end())
	   {
		   NSSTRING name =  (*m_MapIterator).first;
		   NSSTRING Val =  (*m_MapIterator).second;

		   return Val;
	   }
	   else
	   {
		   return _T("");
	   }	
}
CNSMainSubstaionDlg::~CNSMainSubstaionDlg()
{
	m_labelFont.DeleteObject(); 
	delete m_pSSData;
}

void CNSMainSubstaionDlg::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_TabDistributor);
	DDX_Control(pDX, IDC_CBSSTYPE, m_CbSSType);
	DDX_Control(pDX, IDC_CBSSSIZE, m_CbSSSize);
	DDX_Control(pDX, IDC_CBSSDESCRIPTION, m_CbSSDescription);
	DDX_Control(pDX, IDC_CBSSSTOCKTYPE, m_CbSSStockType);
	DDX_Control(pDX, IDC_BTNSSDETAILS, m_BtnSSDetails);
	DDX_Control(pDX, IDC_CBHVCEQUIP, m_CbHVCEquipment);
	DDX_Control(pDX, IDC_COMBO6, m_CbHVCType);
	DDX_Control(pDX, IDC_COMBO7, m_CbHVCRating);
	DDX_Control(pDX, IDC_CBTXRATING, m_CbTXRating);
	DDX_Control(pDX, IDC_EDITTXVOLTAGE, m_EditTXVoltage);
	DDX_Control(pDX, IDC_EDITTXPHASES, m_EditTXPhases);
	DDX_Control(pDX, IDC_CBTXTAPRATIO, m_CbTXTapRatio);
	DDX_Control(pDX, IDC_EDITTXTAPSETTING, m_EditTXTapSetting);
	DDX_Check(pDX, IDC_CHKEFIA ,m_iHVCEfiA);
	DDX_Check(pDX, IDC_CHKEFIB ,m_iHVCEfiB);
	DDX_Text(pDX, IDC_EDITSSPREFIX ,m_strSSPrefix);
	DDX_Control(pDX, IDC_CBLVNOOFDIST, m_CbLVNoOfDist);
	DDX_Control(pDX, IDC_EDITTXCTRATIO, m_EditTXCTRatio);
	DDX_Control(pDX, IDC_EDITTXKFACTOR, m_EditTXKFactor);
	DDX_Control(pDX, IDC_CBLVCFUSEA, m_CbLVCFuseA);
	DDX_Control(pDX, IDC_CBLVCFUSE, m_CbLVCFuse);
	DDX_Control(pDX, IDC_EDITLVCPANELRATING, m_EditLVCPanelRating);
	DDX_Control(pDX, IDC_EDITLVCDISTNAME, m_EditLVCDistName);
	DDX_Control(pDX, IDC_CHKLVCWDNO, m_ChkLVCWDNO);
	DDX_Control(pDX, IDC_CHKLVCIDLINK, m_ChkLVCIDLINK);
	DDX_Text(pDX, IDC_EDITSSNUMBER, m_strSSNumber);
	DDX_Text(pDX, IDC_EDITSSNAME, m_strSSName);
	DDX_Text(pDX, IDC_EDITHVCNUMBER, m_strHVCNumber);
	DDX_Text(pDX, IDC_EDITSIDEA, m_strHVCSideA);
	DDX_Text(pDX, IDC_EDITSIDEB, m_strHVCSideB);
	DDX_Text(pDX, IDC_EDITSSOPTIONS, m_strSSOptions);
	DDX_Control(pDX, IDC_EDITTXLENGTH, m_EditTXLength);
	DDX_Control(pDX, IDC_EDITTXSTRENGT, m_EditTxStrength);
	DDX_Control(pDX, IDC_EDITSSOPTIONS, m_CbSSOptions);
}
void CNSMainSubstaionDlg::SetSubstationData()
{
	UpdateData();
	m_pSSData->setHVCEFIA(m_iHVCEfiA);
	m_pSSData->setHVCEFIB(m_iHVCEfiB);
	m_pSSData->setSSPrefix(m_strSSPrefix);
	m_pSSData->setSSNumber(m_strSSNumber);
	m_pSSData->setSSName(m_strSSName);
	m_pSSData->setHVCNumber(m_strHVCNumber);
	m_pSSData->setHVCSideA(m_strHVCSideA);
	m_pSSData->setHVCSideB(m_strHVCSideB);
	m_pSSData->setSSOptions(m_strSSOptions);
	m_pSSData->loadSchematicInfo();
	m_pSSData->loadGeographicInfo(m_strSSOptions);
	int iTabCount = m_TabDistributor.GetItemCount();
	for(int i=0; i<iTabCount; i++)
	{
		TCITEM tabDistributor;
		tabDistributor.mask = TCIF_TEXT;
		tabDistributor.cchTextMax = 63;
		tabDistributor.pszText = new TCHAR[64];
		m_TabDistributor.GetItem(i,&tabDistributor); 
		CString strCaption = tabDistributor.pszText;
		m_pSSData->setLVCDistCaption(i,strCaption);
		delete [] tabDistributor.pszText;
	}
}
void CNSMainSubstaionDlg::GetSubstationData()
{
	if(m_bInitFromRegistry)
	{
		m_pSSData->loadDataFromRegistry();
	}
	else
	{	
		setUIControlFromAttributeValues();
	}
	
	m_iHVCEfiA = m_pSSData->getHVCEFIA();
	m_iHVCEfiB = m_pSSData->getHVCEFIB();
	m_pSSData->getSSPrefix(m_strSSPrefix);
	m_pSSData->getSSNumber(m_strSSNumber);
	m_pSSData->getSSName(m_strSSName);
	m_pSSData->getHVCNumber(m_strHVCNumber);
	m_pSSData->getHVCSideA(m_strHVCSideA);
	m_pSSData->getHVCSideB(m_strHVCSideB);
	m_pSSData->getSSOptions(m_strSSOptions);
	UpdateData(FALSE);
}

BEGIN_MESSAGE_MAP(CNSMainSubstaionDlg, CAcUiDialog)
	ON_BN_CLICKED(IDC_Schematic, &CNSMainSubstaionDlg::OnBnClickedSchematic)
	ON_BN_CLICKED(IDC_Substation, &CNSMainSubstaionDlg::OnBnClickedSubstation)
	ON_BN_CLICKED(IDC_BTNSSDETAILS, &CNSMainSubstaionDlg::OnBnClickedSSdetails)
	ON_CBN_SELCHANGE(IDC_CBSSTYPE, &CNSMainSubstaionDlg::OnCbnSelchangeSStype)
	ON_CBN_SELCHANGE(IDC_CBSSSIZE, &CNSMainSubstaionDlg::OnCbnSelchangeSSsize)
	ON_CBN_SELCHANGE(IDC_CBSSDESCRIPTION, &CNSMainSubstaionDlg::OnCbnSelchangeSSdescription)
	ON_CBN_SELCHANGE(IDC_CBSSSTOCKTYPE, &CNSMainSubstaionDlg::OnCbnSelchangeSSStockType)	
	ON_CBN_SELCHANGE(IDC_CBHVCEQUIP, &CNSMainSubstaionDlg::OnCbnSelchangeHVCEquip)
	ON_CBN_SELCHANGE(IDC_CBHVCTYPE, &CNSMainSubstaionDlg::OnCbnSelchangeHVCType)
	ON_CBN_SELCHANGE(IDC_CBTXRATING, &CNSMainSubstaionDlg::OnCbnSelchangeTXRating)
	ON_CBN_SELCHANGE(IDC_CBTXTAPRATIO, &CNSMainSubstaionDlg::OnCbnSelchangeTXTapRatio)
	ON_CBN_SELCHANGE(IDC_CBHVCRATING, &CNSMainSubstaionDlg::OnCbnSelchangeHVCRating)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABLVC, &CNSMainSubstaionDlg::OnTcnSelchangeLVC)
	ON_CBN_SELCHANGE(IDC_CBLVCFUSEA, &CNSMainSubstaionDlg::OnCbnSelchangeLVCFuseA)
	ON_CBN_SELCHANGE(IDC_CBLVCFUSE, &CNSMainSubstaionDlg::OnCbnSelchangeLVCFuse)
	ON_EN_CHANGE(IDC_EDITLVCDISTNAME, &CNSMainSubstaionDlg::OnEnChangeLVCDistName)
	ON_BN_CLICKED(IDC_CHKLVCWDNO, &CNSMainSubstaionDlg::OnChkclickedLVCWDNO)
	ON_BN_CLICKED(IDC_CHKLVCIDLINK, &CNSMainSubstaionDlg::OnChkClickedLVCIDLINK)
	ON_CBN_SELCHANGE(IDC_CBLVNOOFDIST, &CNSMainSubstaionDlg::OnCbnSelchangeLVNoOfDist)
	ON_CBN_EDITCHANGE(IDC_CBHVCTYPE, &CNSMainSubstaionDlg::OnCbnEditchangeHVCType)
	ON_CBN_EDITCHANGE(IDC_CBHVCEQUIP, &CNSMainSubstaionDlg::OnCbnEditchangeHVCEquip)
	ON_CBN_EDITCHANGE(IDC_CBHVCRATING, &CNSMainSubstaionDlg::OnCbnEditchangeHVCRating)
	ON_CBN_EDITCHANGE(IDC_CBTXRATING, &CNSMainSubstaionDlg::OnCbnEditchangeTXRating)
	ON_EN_CHANGE(IDC_EDITTXVOLTAGE, &CNSMainSubstaionDlg::OnEnChangeTXVoltage)
	ON_EN_CHANGE(IDC_EDITTXPHASES, &CNSMainSubstaionDlg::OnEnChangeTXPhases)
	ON_EN_CHANGE(IDC_EDITTXTAPSETTING, &CNSMainSubstaionDlg::OnEnChangeTXTapsetting)
	ON_CBN_EDITCHANGE(IDC_CBLVCFUSEA, &CNSMainSubstaionDlg::OnCbnEditchangeLVCFuseA)
	ON_CBN_EDITCHANGE(IDC_CBLVCFUSE, &CNSMainSubstaionDlg::OnCbnEditchangeLVCFuse)
	ON_BN_CLICKED(IDCLEAR, &CNSMainSubstaionDlg::OnBnClickedClear)
END_MESSAGE_MAP()


// CNSMainSubstaionDlg message handlers

void CNSMainSubstaionDlg::OnBnClickedSchematic()
{
	AcDbObjectId objID;
	std::map<NSSTRING, NSSTRING> mapXData;

	SetSubstationData();
	m_pSSData->setDataToRegistry();

	CString strSSType,strSSSize,strSSDesc,strSSStockType;
	m_pSSData->getSSDescription(strSSType);
	m_pSSData->getSSSize(strSSSize);
	m_pSSData->getSSType(strSSDesc);
	m_pSSData->getSSStockType(strSSStockType);

	if(strSSDesc.IsEmpty() || strSSSize.IsEmpty() ||strSSDesc.IsEmpty() || strSSStockType.IsEmpty())
	{
		::MessageBox(NULL,_T("Substation Type or Size or Description or Stock Type should not be null"),_T("NET CAD"),MB_OK);
		return ;
	}

	int es;

	int LV_Out = PopulateAttributeMapfromUI();


			CString strSchematic;
			m_pSSData->getSchematic(strSchematic);

			if((NSSTRCMP(strSchematic, _T("")) == 0))
			{
				::MessageBox(NULL,_T("No Schematic found for Substation type"),_T("NET CAD"),MB_OK);
				return;
			}

			BeginEditorCommand();

			const ACHAR *ActiveLayOutName = ((AcApLayoutManager*)acdbHostApplicationServices()->layoutManager())->findActiveLayout(true);

			if((NSSTRCMP(ActiveLayOutName, _T("Model")) == 0))
			{
				CLayOutSelDlg* objDlg = new CLayOutSelDlg(acedGetAcadFrame());
				if(objDlg!=NULL)
				{
						INT_PTR nResponse = objDlg->DoModal();

						if (nResponse == IDOK)
						{
							// TODO: Place code here to handle when the dialog is
							//  dismissed with OK
							((AcApLayoutManager*)acdbHostApplicationServices()->layoutManager())->setCurrentLayout(objDlg->strAvailableLayOutNames);
						}
						else if (nResponse == IDCANCEL)
						{
							// TODO: Place code here to handle when the dialog is
							//  dismissed with Cancel

							 CompleteEditorCommand();
							 return;
						}

						delete objDlg;
				}
			}

			acedCommandS(RTSTR, _T("PSPACE"), RTNONE);
			acedRetVoid();

			ads_point pt;
			AcGePoint3d ptBase;

			if(!IsInEditMode)
			{
				if (acedGetPoint(NULL, _T("\nSpecify insertion point: "), pt) != RTNORM)
				{
					CancelEditorCommand();
				}

				ptBase = AcGePoint3d(pt[X], pt[Y], pt[Z]);
			}
			else
			{
				ptBase = AcGePoint3d(0,0, 0);

				ads_name entName;
				ACHAR *szhandle = new ACHAR[20];
				//m_handle.getIntoAsciiBuffer(szhandle);
				m_handle.getIntoAsciiBuffer(szhandle,sizeof(szhandle));
				acdbHandEnt(szhandle,entName);
				delete szhandle;//*subir

				
				acdbGetObjectId(objID,entName );

								CNSDWGMgr ThisDrawing;
								
								ThisDrawing.fillXDATAMAP(objID,mapXData);

				AcDbEntity *pEnt;        
				acdbOpenAcDbEntity(pEnt,objID,AcDb::kForWrite);

				AcDbBlockReference *pBlkRef;
				AcDbObjectId blockDefId;
				if (pEnt->isKindOf(AcDbBlockReference::desc()))
				{						
										pBlkRef = AcDbBlockReference::cast(pEnt);
										ptBase = pBlkRef->position();
				}

				pBlkRef->close();
				pEnt->close();
				es = acdbEntDel(entName);

				////////////////////////////////////////////////////////////////
				MYSTRINGSTRINGMAP::iterator m_XDATAMapIterator;
				m_XDATAMapIterator = mapXData.find(_T("ListOfDistributors"));
				if(m_XDATAMapIterator != mapXData.end())
				{
				   NSSTRING name =  (*m_XDATAMapIterator).first;
				   NSSTRING ListOfDistributors =  (*m_XDATAMapIterator).second;
					
				   CString szListOfDistributors(ListOfDistributors.c_str());

				   //szListOfDistributors.Tokenize(
				    int index = 0 ;
					CString disttributor;
					while (AfxExtractSubString(disttributor,szListOfDistributors,index,_T(',')))
					{
						acdbHandEnt(disttributor,entName);
						es = acdbEntDel(entName);
						++index;
					}

				}
				////////////////////////////////////////////////////////////////
				CString strDataTable ;

				//MYSTRINGSTRINGMAP::iterator m_XDATAMapIterator;
				m_XDATAMapIterator = mapXData.find(_T("HandleOfLVDataTable"));
				if(m_XDATAMapIterator != mapXData.end())
				{
				   NSSTRING name =  (*m_XDATAMapIterator).first;
				   NSSTRING szDataTable =  (*m_XDATAMapIterator).second;
					
				   CString strDataTable(szDataTable.c_str());

				    ads_name entTableName;
					acdbHandEnt(strDataTable,entTableName);
					es = acdbEntDel(entTableName);
				}

				
				////////////////////////////////////////////////////////////////



				acedUpdateDisplayPause(false);	
				acedUpdateDisplay();
				
			}
		    
				

				CNSDWGMgr ThisDrawing;
				AcDbObjectId NewSubStnBlkID = ThisDrawing.InsertSubstationBlock(strSchematic,ptBase,m_pSSData,mapOfAttributes,false);	
				ThisDrawing.PunchNSStampToLastEntityCreated();

				CString szListOfDistributors = _T("");
				
				m_pSSData->loadDistributorsBlockInfo();
				int distCnt = 0;
				for(int i = 1; i<= LV_Out; i++)
				{
					CString DistBlkNam;
					CString VisName;
					
					
					int x,y,r=0;
					CString strX, strY;
					m_pSSData->getDistributorBlockInfo(i,DistBlkNam,VisName,strX,strY,r);

					x = _wtoi(strX);
					y = _wtoi(strY);

					AcGePoint3d insPt = AcGePoint3d(ptBase[X]+x, ptBase[Y]+y, ptBase[Z]);

					TCHAR* p_szVisName(VisName.GetBuffer());

					CString strHandle =_T("");

					ThisDrawing.InsertDistributorBlock(DistBlkNam,p_szVisName,i,insPt,strHandle);
					//acutPrintf(_T("%s",tempStr.GetBuffer()));

					if(distCnt == 0)
					{
						if(!strHandle.IsEmpty())
						{
							szListOfDistributors = strHandle;
							distCnt++;
						}
					}
					else
					{
						if(!strHandle.IsEmpty())
						{
							szListOfDistributors = szListOfDistributors + _T(",") + strHandle;
							distCnt++;
						}
					}
				}

				ThisDrawing.addXDATA(NewSubStnBlkID,_T("ListOfDistributors"),szListOfDistributors.GetBuffer());
				//acutPrintf(_T("%s",szListOfDistributors.GetBuffer()));

				ThisDrawing.PostXDATAfromUI(NewSubStnBlkID,mapOfAttributes);

				AcGePoint3d insPt;
				CString strHandle =_T("");
				if(!IsInEditMode)
				{
					 if (acedGetPoint(NULL, _T("\nSpecify TABLE position: "), pt) == RTNORM)
					 {
						insPt = AcGePoint3d(pt[X], pt[Y], pt[Z]);						

						CString strX =_T("");
						strX.Format(_T("%f"),pt[X]);

						CString strY =_T("");
						strY.Format(_T("%f"),pt[Y]);

						ThisDrawing.addXDATA(NewSubStnBlkID,_T("PosXOfLVDataTable"),strX.GetBuffer());
						ThisDrawing.addXDATA(NewSubStnBlkID,_T("PosYOfLVDataTable"),strY.GetBuffer());
					}
					else
					{
						// to handle if required
					}
				 }
				else
				{
					//ads_name entName;
					//ACHAR *szhandle = new ACHAR[20];
					//m_handle.getIntoAsciiBuffer(szhandle);
					//acdbHandEnt(szhandle,entName);
					//delete szhandle;//*subir

					//
					//acdbGetObjectId(objID,entName);

					//CNSDWGMgr ThisDrawing;
					//std::map<NSSTRING, NSSTRING> mapXData;
					//ThisDrawing.fillXDATAMAP(objID,mapXData);

					MYSTRINGSTRINGMAP::iterator m_XDATAMapIterator;

					double xPos,yPos;

					CString strX;					
					m_XDATAMapIterator = mapXData.find(_T("PosXOfLVDataTable"));
					if(m_XDATAMapIterator != mapXData.end())
					{
					   NSSTRING name =  (*m_XDATAMapIterator).first;
					   NSSTRING szX =  (*m_XDATAMapIterator).second;
						
					   CString strX(szX.c_str());
					   xPos = _wtof(strX);
					}

					CString strY;					
					m_XDATAMapIterator = mapXData.find(_T("PosYOfLVDataTable"));
					if(m_XDATAMapIterator != mapXData.end())
					{
					   NSSTRING name =  (*m_XDATAMapIterator).first;
					   NSSTRING szY =  (*m_XDATAMapIterator).second;
						
					   CString strY(szY.c_str());	
					   yPos = _wtof(strY);
					}

					insPt = AcGePoint3d(xPos, yPos, 0);
				}

				AcDbObjectId newtableId = ThisDrawing.createSchematicTable(insPt,m_pSSData,strHandle);
				if(!strHandle.IsEmpty())
				{
					ThisDrawing.addXDATA(NewSubStnBlkID,_T("HandleOfLVDataTable"),strHandle.GetBuffer());							
				}

				ads_name table;
				acdbGetAdsName(table,newtableId);
				acdbEntUpd(table);

				acedRedraw(table,1);
				acedUpdateDisplay();

			CompleteEditorCommand();

			if(IsInEditMode)
			{
				OnCancel();
			}

}

void CNSMainSubstaionDlg::OnBnClickedSubstation()
{
	SetSubstationData();
	m_pSSData->setDataToRegistry();

	PopulateAttributeMapfromUI();

	CString strGeographic;
	m_pSSData->getGeographic(strGeographic);

	CString strSSType,strSSSize,strSSDesc,strSSStockType;
	m_pSSData->getSSDescription(strSSType);
	m_pSSData->getSSSize(strSSSize);
	m_pSSData->getSSType(strSSDesc);
	m_pSSData->getSSStockType(strSSStockType);

	if(strSSDesc.IsEmpty() || strSSSize.IsEmpty() ||strSSDesc.IsEmpty() || strSSStockType.IsEmpty())
	{
		::MessageBox(NULL,_T("Substation Type or Size or Description or Stock Type should not be null"),_T("NET CAD"),MB_OK);
		return ;
	}

	
	if((NSSTRCMP(strGeographic, _T("")) == 0))
	{
		::MessageBox(NULL,_T("No Schematic found for Substation type"),_T("NET CAD"),MB_OK);
		return;
	}

    const ACHAR *ActiveLayOutName = ((AcApLayoutManager*)acdbHostApplicationServices()->layoutManager())->findActiveLayout(true);

	if((NSSTRCMP(ActiveLayOutName, _T("Model")) != 0))
	{
			((AcApLayoutManager*)acdbHostApplicationServices()->layoutManager())->setCurrentLayout(_T("Model"));
	}

	BeginEditorCommand();

    ads_point pt;

    
    if (acedGetPoint(NULL, _T("\nSpecify insertion point: "), pt) == RTNORM)
	{
        //CompleteEditorCommand();
        
		AcGePoint3d ptBase = AcGePoint3d(pt[X], pt[Y], pt[Z]);

		CNSDWGMgr ThisDrawing;			   
		ThisDrawing.InsertSubstationBlock(strGeographic,ptBase,m_pSSData,mapOfAttributes,true);

		 if (acedGetPoint(NULL, _T("\nSpecify TABLE position: "), pt) == RTNORM)
		 {
			AcGePoint3d insPt = AcGePoint3d(pt[X], pt[Y], pt[Z]);
			AcDbObjectId newtableId = ThisDrawing.createGeoGraphicTable(insPt,m_pSSData);

			ads_name table;
			acdbGetAdsName(table,newtableId);
			acdbEntUpd(table);

			acedRedraw(table,1);
			acedUpdateDisplay();
		 }
		
		
		CompleteEditorCommand();
		
	} 
	else 
	{
                CancelEditorCommand();
    }
}

void CNSMainSubstaionDlg::OnBnClickedSSdetails()
{
	CString strFunctionName, strSize;
	m_pSSData->getSSSize(strSize);
	SetSubstationData();
	if(strSize.IsEmpty())
	{
		::MessageBox(NULL,L"Enter SubStation Size",_T("NET CAD"),MB_OK|MB_ICONERROR);

		
		return;
	}
	m_pSSData->getSSFunction(strFunctionName);

	if(strFunctionName =="Sub_Details_PT")
		Sub_Details_PT();
	else if(strFunctionName =="Sub_Details_Kiosk")
		Sub_Details_Kiosk();
	UpdateData(FALSE);
	UpdateSubstation();
}

void CNSMainSubstaionDlg::Sub_Details_PT()
{
		CNSSubstationDet_PT* objDlg = new CNSSubstationDet_PT(m_strSSOptions,m_pSSData);
		if(objDlg!=NULL)
		{
			objDlg->DoModal();
			delete objDlg;
		}
}

void CNSMainSubstaionDlg::Sub_Details_Kiosk()
{
	m_strSSOptions;

	CNSSubstationDet_Kiosk* objDlg = new CNSSubstationDet_Kiosk(m_strSSOptions,m_pSSData);
		if(objDlg!=NULL)
		{
			objDlg->DoModal();
			delete objDlg;
		}
}

int CNSMainSubstaionDlg::PopulateAttributeMapfromUI()
{
	 //m_pSSData->getHVCEFIA();

	mapOfAttributes.clear();

	CString strVal;

	//// these data is not stored as attributes but required in edit mode

	m_pSSData->getSSOptions (strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("OPTIONS"),strVal.GetBuffer()));

	m_pSSData->getHVCNumber (strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVCNumber"),strVal.GetBuffer()));

	m_pSSData->getHVCEquipment(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVCEquip"),strVal.GetBuffer()));

	m_pSSData->getHVCType(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVCType"),strVal.GetBuffer()));

	m_pSSData->getHVCRating(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVCRating"),strVal.GetBuffer()));

	m_pSSData->getHVCSideA (strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVCSideA"),strVal.GetBuffer()));

	m_pSSData->getHVCSideB(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVCSideB"),strVal.GetBuffer()));

	int efiA = m_pSSData->getHVCEFIA();
	strVal.FormatMessage(L"%1!d!",efiA);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVCEfiA"),strVal.GetBuffer()));

	int efiB =m_pSSData->getHVCEFIB();
	strVal.FormatMessage(L"%1!d!",efiB);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVCEfiB"),strVal.GetBuffer()));



	m_pSSData->getTXRating(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXRating"),strVal.GetBuffer()));

	m_pSSData->getTXVoltage(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXVoltage"),strVal.GetBuffer()));

	int phases = m_pSSData->getTXPhases();
	strVal.FormatMessage(L"%1!d!",phases);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXPhases"),strVal.GetBuffer()));

	m_pSSData->getTXTapRatio(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXTapRatio"),strVal.GetBuffer()));

	int tapsetting = m_pSSData->getTXTapSetting();
	strVal.FormatMessage(L"%1!d!",tapsetting);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXTapSetting"),strVal.GetBuffer()));

	m_pSSData->getTXCTRatio(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXCTRatio"),strVal.GetBuffer()));

	m_pSSData->getTXKFactor(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXKFactor"),strVal.GetBuffer()));

	m_pSSData->getTXPoleLength(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXPoleLength"),strVal.GetBuffer()));

	m_pSSData->getTXPoleStrength(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXPoleStrength"),strVal.GetBuffer()));
	////
	
	m_pSSData->getSSSize(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("SIZE"),strVal.GetBuffer()));

	m_pSSData->getSSPrefix(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("PREFIX"), strVal.GetBuffer()));

	m_pSSData->getSSNumber(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("NUMBER"), strVal.GetBuffer()));

	m_pSSData->getSSName(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("NAME"), strVal.GetBuffer()));

	m_pSSData->getSSStockType(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("STOCK_TYPE"), strVal.GetBuffer()));

	m_pSSData->getHVCNumber(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HV_NUMBER"), strVal.GetBuffer()));

	m_pSSData->getHVCType(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HV_TYPE"), strVal.GetBuffer()));

	m_pSSData->getHVCRating(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HV_RATING"), strVal.GetBuffer()));

	m_pSSData->getHVCSideA(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVSIDE_1"), strVal.GetBuffer()));

	m_pSSData->getHVCSideB(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("HVSIDE_2"), strVal.GetBuffer()));

	m_pSSData->getTXRating(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXRATING"), strVal.GetBuffer()));

	m_pSSData->getTXTapRatio(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXRATIO"), strVal.GetBuffer()));

	int iTapSetting = m_pSSData->getTXTapSetting();
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("TXTAPSET"), strVal.GetBuffer()));

	m_pSSData->getTXKFactor(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("MDICTRATIO"), strVal.GetBuffer()));

	m_pSSData->getTXCTRatio(strVal);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("MDICT"), strVal.GetBuffer()));


	int nLV_Out = m_pSSData->getLVCNoOfDistributors();
	strVal.FormatMessage(L"%1!d!",nLV_Out);
	mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(_T("DISTRIBUTOR_NUMBER"), strVal.GetBuffer()));

	for(int index = 0; index < nLV_Out ; index++)
	{
		CString strDistNumber,strDistName,strPanelRating,strFuseRating,strArmType,strWdno,strIdlink;
		strDistNumber.FormatMessage(L"DISTRIBUTOR_NUMBER%1!d!",index+1);
		strDistName.FormatMessage(L"DISTRIBUTOR_NAME%1!d!",index+1);
		strPanelRating.FormatMessage(L"PANEL_RATING%1!d!",index+1);
		strFuseRating.FormatMessage(L"FUSE_RATING%1!d!",index+1);
		strArmType.FormatMessage(L"FUSE_ARMTYPE%1!d!",index+1);
		strWdno.FormatMessage(L"WDNO%1!d!",index+1);
		strIdlink.FormatMessage(L"IDLINK%1!d!",index+1);

		
		m_pSSData->getLVCDistCaption(index, strVal);
		mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(strDistNumber.GetBuffer(), strVal.GetBuffer()));
		
		m_pSSData->getLVCDistributorName(index,strVal);
		mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(strDistName.GetBuffer(), strVal.GetBuffer()));
		
		int iLVCPanelRating = m_pSSData->getLVCPanelRating(index);
		strVal.FormatMessage(L"%1!d!",iLVCPanelRating);
		mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(strPanelRating.GetBuffer(), strVal.GetBuffer()));
		

		int iLVCFuseA = m_pSSData->getLVCFuseA(index);
		strVal.FormatMessage(L"%1!d!",iLVCFuseA);
		mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(strFuseRating.GetBuffer(), strVal.GetBuffer()));
		

		m_pSSData->getLVCFuse(index,strVal);
		mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(strArmType.GetBuffer(), strVal.GetBuffer()));
		
		//
		
		int iWdno = m_pSSData->getLVCWDNO(index);
		strVal.FormatMessage(L"%1!d!",iWdno);
		mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(strWdno.GetBuffer(),strVal.GetBuffer()));

		int iIdlink = m_pSSData->getLVCIDLINK(index);
		strVal.FormatMessage(L"%1!d!",iIdlink);
		mapOfAttributes.insert( MYSTRINGSTRINGMAP::value_type(strIdlink.GetBuffer(),strVal.GetBuffer()));
	}	

return nLV_Out;
}

BOOL CNSMainSubstaionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDC_LBLTRANSFORMER)->SetFont(&m_labelFont);
	GetDlgItem(IDC_LBLHVCONN)->SetFont(&m_labelFont);
	GetDlgItem(IDC_LBLSUBSTATION)->SetFont(&m_labelFont);
	GetDlgItem(IDC_STATIC6)->SetFont(&m_labelFont);
	InitSSControls();
	GetSubstationData();
	CString strLabel;
	m_pSSData->getSSLabel(strLabel);
	if(strLabel.IsEmpty() == false)
	{
		m_BtnSSDetails.SetWindowText(strLabel);
	}
	UpdateSubstation();
	m_CbSSType.SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CNSMainSubstaionDlg::InitSSControls(const CString &strSSType, const CString &strSSSize)
{
	std::vector<variant_t> vData;
	int index;
	CString strCurSel;

	//populate substation type combobox
	m_CbSSType.GetWindowText(strCurSel);
	if(strSSType.IsEmpty())
	{
		m_pSSData->setSSType("");
		m_pSSData->getSSTypes(vData);
		ResetCBcontrol(m_CbSSType,vData);
	}
	else if(strCurSel.IsEmpty())
	{
		m_CbSSType.SetCurSel(m_CbSSType.FindStringExact(-1,strSSType));		
		m_pSSData->getSSSizes(vData,strSSType);	
		ResetCBcontrol(m_CbSSSize,vData);
	}

	//populate substation size combobox	
	m_CbSSSize.GetWindowText(strCurSel);
	if(strSSSize.IsEmpty())
	{
		m_pSSData->getSSSizes(vData,strSSType);	
		ResetCBcontrol(m_CbSSSize,vData);
	}
	else if(strCurSel.IsEmpty())
	{
		m_CbSSSize.SetCurSel(m_CbSSSize.FindStringExact(-1,strSSSize));
	}

	//populate substation description combobox	
	m_pSSData->getSSDescriptions(vData,strSSType,strSSSize);	
	ResetCBcontrol(m_CbSSDescription,vData);

	//populate Stock Type combobox	
	m_pSSData->getSSStockTypes(vData,strSSType,strSSSize);	
	ResetCBcontrol(m_CbSSStockType,vData);
	
	CString strCurLabel,strNewLabel;
	m_pSSData->getSSLabel(strNewLabel);
	m_BtnSSDetails.GetWindowText(strCurLabel);
	if(strNewLabel != strCurLabel)
	{
		m_BtnSSDetails.SetWindowText(strNewLabel);
		m_strSSOptions = "";
		m_pSSData->setSSOptions(m_strSSOptions);
	}
	m_CbSSOptions.SetWindowText(m_strSSOptions);
}
void CNSMainSubstaionDlg::InitHVCControls(const CString &strHVCEquip, const CString &strHVCType)
{
	std::vector<variant_t> vData;
	int index;

	CString strSSDesc;
	m_pSSData->getSSDescription(strSSDesc);
	if(strSSDesc.IsEmpty()) 
	{
		ResetCBcontrol(m_CbHVCEquipment,vData);
		ResetCBcontrol(m_CbHVCType,vData);		
		ResetCBcontrol(m_CbHVCRating,vData);
		m_pSSData->setHVCEquipment("");
		m_pSSData->setHVCType("");
		m_pSSData->setHVCRating("");
		return;
	}
	
	//if(strHVCEquip.IsEmpty() || m_CbHVCEquipment.GetCount() < 1)
	//{
		if(strSSDesc.IsEmpty() == false)
		{
			m_pSSData->getHVCEquipments(vData);
		}
		ResetCBcontrol(m_CbHVCEquipment,vData);
		if(vData.size() > 0)
		{
			int nPos;
			if(strHVCEquip.IsEmpty())
			{
				m_CbHVCEquipment.SetCurSel(0);
				m_CbHVCEquipment.GetLBText(0,const_cast<CString&>(strHVCEquip));
			}
			else if((nPos=m_CbHVCEquipment.FindStringExact(-1,strHVCEquip))!=CB_ERR)
			{
				m_CbHVCEquipment.SetCurSel(nPos);
			}
			m_pSSData->setHVCEquipment(strHVCEquip);
		}
	//}
	
	//if(strHVCType.IsEmpty() || m_CbHVCType.GetCount() < 1)
	//{
		m_pSSData->getHVCTypes(vData,strHVCEquip);	
		ResetCBcontrol(m_CbHVCType,vData);		
		if(vData.size() > 0)
		{
			int nPos;
			if(strHVCType.IsEmpty())
			{
				m_CbHVCType.SetCurSel(0);
				m_CbHVCType.GetWindowText(const_cast<CString&>(strHVCType));
			}
			else if((nPos=m_CbHVCType.FindStringExact(-1,strHVCType))!=CB_ERR)
			{
				m_CbHVCType.SetCurSel(nPos);
			}
			m_pSSData->setHVCType(strHVCType);
		}
	//}
	
	m_pSSData->getHVCRatings(vData, strHVCEquip, strHVCType);	
	ResetCBcontrol(m_CbHVCRating,vData);
	CString strRating;
	m_pSSData->getHVCRating(strRating);
	if(strRating.IsEmpty() == false)
	{
		m_CbHVCRating.SetCurSel(m_CbHVCRating.FindStringExact(-1,strRating));
	}
	else if(vData.size() > 0)
	{
		CString strRating;
		m_CbHVCRating.SetCurSel(0);
		m_CbHVCRating.GetLBText(0,strRating);
		m_pSSData->setHVCRating(strRating);
	}	
}

void CNSMainSubstaionDlg::InitTXControls(const CString &strTXRating, const CString &strTXTapRatio)
{
	std::vector<variant_t> vData;
	CString strSSSize,strSSDesc;

	m_pSSData->getSSDescription(strSSDesc);
	if(strSSDesc.IsEmpty()) 
	{
		m_pSSData->setTXRating("");
		ResetCBcontrol(m_CbTXRating,vData);
		m_EditTXVoltage.SetWindowTextW(L"");
		m_pSSData->setTXVoltage("");
		m_EditTXPhases.SetWindowText(L"");
		m_pSSData->setTXPhases(-1);
		ResetCBcontrol(m_CbTXTapRatio,vData);
		m_pSSData->setTXTapRatio("");
		m_EditTXTapSetting.SetWindowText(L"");
		m_pSSData->setTXTapSetting(-1);
		m_EditTXCTRatio.SetWindowText(L"");
		m_pSSData->setTXCTRatio("");
		m_EditTXKFactor.SetWindowText(L"");
		m_pSSData->setTXKFactor("");
		m_EditTXLength.SetWindowText(L"");
		m_pSSData->setTXPoleLength("");
		m_EditTxStrength.SetWindowText(L"");
		m_pSSData->setTXPoleStrength("");
		return;
	}

	m_pSSData->getSSSize(strSSSize);
	//if(strTXRating.IsEmpty() || m_CbTXRating.GetCount() < 1)
	//{
		m_pSSData->getTxRatings(vData,strSSSize);
		ResetCBcontrol(m_CbTXRating,vData);
		if(vData.size() > 0)
		{
			int nPos;
			if(strTXRating.IsEmpty())
			{
				m_CbTXRating.SetCurSel(0);
				m_CbTXRating.GetWindowText(const_cast<CString&>(strTXRating));
			}
			else if((nPos=m_CbTXRating.FindStringExact(-1,strTXRating))!=CB_ERR)
			{
				m_CbTXRating.SetCurSel(nPos);
			}
			else
			{
				m_CbTXRating.SetWindowText(strTXRating);
			}
			m_pSSData->setTXRating(strTXRating);
		}
		else
		{
			m_CbTXRating.SetWindowText(strTXRating);
		}
	//}
	CString strTxVoltage;
	m_pSSData->getTXVoltage(strTxVoltage);
	if(strTxVoltage.IsEmpty())
	{
		variant_t vtVoltage;
		m_EditTXVoltage.GetWindowText(strTxVoltage);
		m_pSSData->getTXVoltage(vtVoltage,strSSSize,strTXRating);
		strTxVoltage = vtVoltage.bstrVal;
	}
	m_EditTXVoltage.SetWindowText(strTxVoltage);
	
	CString strTXPhase;
	int iPhases = m_pSSData->getTXPhases();	
	if(iPhases < 0)
	{
		iPhases = m_pSSData->getTXPhases(strSSSize,strTXRating);
		m_pSSData->setTXPhases(iPhases);	
	}
	//if(m_pSSData->setTXPhases(iPhases))
	//{
		if(iPhases < 0)
		{
			m_EditTXPhases.SetWindowText(L"");
			return;
		}
		strTXPhase.FormatMessage(L"%1!d!",iPhases);
	//}
	m_EditTXPhases.SetWindowText(strTXPhase);
	
	if(strTXTapRatio.IsEmpty() || m_CbTXTapRatio.GetCount() < 1)
	{
		m_pSSData->getTXTapRatios(vData,strSSSize);
		ResetCBcontrol(m_CbTXTapRatio,vData);
		if(vData.size() > 0)
		{
			int nPos;
			if(strTXTapRatio.IsEmpty())
			{
				m_CbTXTapRatio.SetCurSel(0);
				m_CbTXTapRatio.GetWindowText(const_cast<CString&>(strTXTapRatio));
			}
			else if((nPos=m_CbTXTapRatio.FindStringExact(-1,strTXTapRatio))!=CB_ERR)
			{
				m_CbTXTapRatio.SetCurSel(nPos);
			}
			m_pSSData->setTXTapRatio(strTXTapRatio);
		}
	}
	if(m_pSSData->isTxTapSettingManual() == false)
	{
		int iTapSetting = m_pSSData->getTXTapSettingFromDB(strSSSize,strTXTapRatio);
		CString strTXTapSetting;
		m_EditTXTapSetting.GetWindowText(strTXTapSetting);
		if(m_pSSData->setTXTapSetting(iTapSetting) || strTXTapSetting.IsEmpty())
		{
			if(iTapSetting < 0)
			{
				m_EditTXTapSetting.SetWindowText(L"");
				return;
			}
			strTXTapSetting.FormatMessage(L"%1!d!",iTapSetting);
			m_EditTXTapSetting.SetWindowText(strTXTapSetting);
		}
	}
	CString strTxCTRatio, strTXKFactor;
	m_pSSData->getTXMDIVals(strTxCTRatio, strTXKFactor, strSSDesc);
	m_pSSData->setTXCTRatio(strTxCTRatio);
	m_pSSData->setTXKFactor(strTXKFactor);
	m_EditTXCTRatio.SetWindowText(strTxCTRatio);
	m_EditTXKFactor.SetWindowText(strTXKFactor);	
	CString strTXPoleLength, strTXPoleStrength;
	m_pSSData->getTXPoleLength(strTXPoleLength);
	m_pSSData->getTXPoleStrength(strTXPoleStrength);
	m_EditTXLength.SetWindowText(strTXPoleLength);
	m_EditTxStrength.SetWindowText(strTXPoleStrength);
}

void  CNSMainSubstaionDlg::InitLVCControls(int iCurTabSelected)
{
	CString strSSDesc,strFuse;
	int iFuseA,iPanelRating;
	m_pSSData->getSSDescription(strSSDesc);
	
	
	int iNoOfDistributors = m_pSSData->getLVCNoOfDistributors();
	int iTabCount = m_TabDistributor.GetItemCount();
	if(iNoOfDistributors == -1)
	{
		iNoOfDistributors = m_pSSData->getLVCNoOfDistributors(strSSDesc);
		m_pSSData->setLVCNoOfDistributors(iNoOfDistributors);
		m_TabDistributor.DeleteAllItems();
		for(int index=0; index<iNoOfDistributors; index++)
		{
			CString strDistributor;
			strDistributor.FormatMessage(L"Distributor%1!d!",index+1);
			m_TabDistributor.InsertItem(index,strDistributor);	
			m_pSSData->setLVCDistCaption(index,strDistributor);
			iFuseA = m_pSSData->getLVCFuseAFromDB(strSSDesc,index);		
			m_pSSData->setLVCFuseA(index,iFuseA);		
			iPanelRating = m_pSSData->getLVCPanelRatingFromDB(strSSDesc,index);
			m_pSSData->setLVCPanelRating(index,iPanelRating);
			m_pSSData->getLVCFuseFromDB(strSSDesc,index,strFuse);		
			m_pSSData->setLVCFuse(index,strFuse);
		}
		iTabCount = iNoOfDistributors;
		m_TabDistributor.SetCurSel(iCurTabSelected);
	}
	m_CbLVNoOfDist.SetCurSel(iNoOfDistributors);
	if(iNoOfDistributors == 0)
	{

		GetDlgItem(IDC_TABLVC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CBLVCFUSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_CBLVCFUSEA)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITLVCDISTNAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDITLVCPANELRATING)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHKLVCEFI)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHKLVCWDNO)->EnableWindow(FALSE);
		m_pSSData->clearLVCInfo();
		m_TabDistributor.DeleteAllItems();
		return;
	}
	else if(GetDlgItem(IDC_TABLVC)->IsWindowVisible() == FALSE)
	{
		GetDlgItem(IDC_TABLVC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CBLVCFUSE)->EnableWindow(TRUE);
		GetDlgItem(IDC_CBLVCFUSEA)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITLVCDISTNAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDITLVCPANELRATING)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHKLVCEFI)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHKLVCWDNO)->EnableWindow(TRUE);
	}
	if(iTabCount != iNoOfDistributors)
	{
		while(iTabCount < iNoOfDistributors)
		{
			CString strDistributor;
			m_pSSData->getLVCDistCaption(iTabCount,strDistributor);
			if(strDistributor.IsEmpty())
			{
				strDistributor.FormatMessage(L"Distributor%1!d!",iTabCount+1);
				m_pSSData->setLVCDistCaption(iTabCount,strDistributor);
			}
			m_TabDistributor.InsertItem(iTabCount,strDistributor);	
			iTabCount++;
		}
	}
	while(iTabCount > iNoOfDistributors)
	{
		m_TabDistributor.DeleteItem(--iTabCount);	
		m_pSSData->clearLVCInfo(iTabCount);
	}
	CString strPanelRating;
	iPanelRating = m_pSSData->getLVCPanelRating(iCurTabSelected);
	
	strPanelRating.FormatMessage(L"%1!d!",iPanelRating);
	if(iPanelRating > 0)
	{
		m_EditLVCPanelRating.SetWindowText(strPanelRating);
	}
	else
	{
		m_EditLVCPanelRating.SetWindowText(L"");
	}
	// Populate FuseA combo and set value
	std::vector<variant_t> vData;
	int iStringFound;
	if(m_pSSData->isLVCFuseAManual(iCurTabSelected) == false)
	{
		m_pSSData->getLVCFuseAList(vData,iCurTabSelected);
		ResetCBcontrol(m_CbLVCFuseA,vData);
	}
		CString strFuseA;
		iFuseA = m_pSSData->getLVCFuseA(iCurTabSelected);	
		strFuseA.FormatMessage(L"%1!d!",iFuseA);
		iStringFound = m_CbLVCFuseA.FindStringExact(-1,strFuseA);
		if(iStringFound != CB_ERR)
		{
			m_CbLVCFuseA.SetCurSel(iStringFound);
		}
		else
		{
			if(iFuseA > 0)
			{
				m_CbLVCFuseA.SetWindowText(strFuseA);
			}
			else
			{
				m_CbLVCFuseA.SetWindowText(L"");
			}
		}
	//}

	// Populate Fuse or FuseArmType combo and set value
	if(m_pSSData->isLVCFuseManual(iCurTabSelected) == false)
	{
		m_pSSData->getLVCFuseList(vData,iFuseA,iCurTabSelected);
		ResetCBcontrol(m_CbLVCFuse,vData);
	}
		m_pSSData->getLVCFuse(iCurTabSelected,strFuse);
		
		iStringFound = m_CbLVCFuse.FindStringExact(-1,strFuse);
		if(iStringFound != CB_ERR)
		{
			m_CbLVCFuse.SetCurSel(iStringFound);
		}
		else
		{
			m_CbLVCFuse.SetWindowText(strFuse);
		}
	//}
	//populate Distributor Name Edit Box;
	CString strDistName;
	m_pSSData->getLVCDistributorName(iCurTabSelected,strDistName);
	m_EditLVCDistName.SetWindowText(strDistName);
	m_pSSData->setLVCDistributorNo(iCurTabSelected);

	//update wdno checkbox status
	m_ChkLVCWDNO.SetCheck(m_pSSData->getLVCWDNO(iCurTabSelected));
	//update idlink checkbox status
	m_ChkLVCIDLINK.SetCheck(m_pSSData->getLVCIDLINK(iCurTabSelected));
	m_TabDistributor.SetCurSel(iCurTabSelected);
	
}

void  CNSMainSubstaionDlg::ResetCBcontrol(CComboBox &CbControl,const std::vector<variant_t> &vData)
{
	std::vector<variant_t>::const_iterator iter;
	int index;	
	while(CbControl.GetCount() > 0)
	{
		CbControl.DeleteString(0);
	}
	CbControl.Invalidate();
	for( iter = vData.begin(),index=0; iter != vData.end(); iter++)
	{
		CString strData;
		strData = iter->bstrVal;
		CbControl.InsertString(index++,strData);
	}
	CbControl.SetWindowText(L"");
}

void CNSMainSubstaionDlg::OnCbnSelchangeSStype()
{
	CString strSSType;
	m_CbSSType.GetWindowTextW(strSSType);
	if(m_pSSData->setSSType(strSSType))
	{
		UpdateSubstation();	
	}
}

void CNSMainSubstaionDlg::OnCbnSelchangeSSsize()
{
	CString strSSSize;
	m_CbSSSize.GetWindowTextW(strSSSize);
	if(m_pSSData->setSSSize(strSSSize))
	{
		UpdateSubstation();
	}
}

void CNSMainSubstaionDlg::OnCbnSelchangeSSdescription()
{
	CString strSSDesc;	
	m_CbSSDescription.GetWindowText(strSSDesc);	
	if(m_pSSData->setSSDescription(strSSDesc))
	{		
		m_strSSOptions = "";
		UpdateSubstation();	
	}	
}
void CNSMainSubstaionDlg::OnCbnSelchangeSSStockType()
{
	CString strSSStockType;	
	m_CbSSStockType.GetWindowText(strSSStockType);	
	if(m_pSSData->setSSStockType(strSSStockType))
	{	
		m_strSSOptions = "";
		UpdateSubstation();	
	}	
}

void CNSMainSubstaionDlg::OnCbnSelchangeHVCEquip()
{
	CString strHVCEquip;
	m_CbHVCEquipment.GetLBText(m_CbHVCEquipment.GetCurSel(),strHVCEquip);
	if(m_pSSData->setHVCEquipment(strHVCEquip))
	{	
		UpdateSubstation();		
	}	
}

void CNSMainSubstaionDlg::OnCbnSelchangeHVCType()
{	
	CString strHVCType;
	m_CbHVCType.GetLBText(m_CbHVCType.GetCurSel(),strHVCType);
	if(m_pSSData->setHVCType(strHVCType))
	{	
		UpdateSubstation();			
	}		
}

void CNSMainSubstaionDlg::OnCbnSelchangeTXRating()
{
	CString strTXRating;
	m_CbTXRating.GetLBText(m_CbTXRating.GetCurSel(),strTXRating);
	m_pSSData->setTXPhases(-1);
	if(m_pSSData->setTXRating(strTXRating))
	{	
		UpdateSubstation();			
	}		
}

void CNSMainSubstaionDlg::OnCbnSelchangeTXTapRatio()
{
	CString strTXTapRatio;
	m_CbTXTapRatio.GetLBText(m_CbTXTapRatio.GetCurSel(),strTXTapRatio);
	if(m_pSSData->setTXTapRatio(strTXTapRatio))
	{	
		UpdateSubstation();			
	}	
}

void CNSMainSubstaionDlg::OnTcnSelchangeLVC(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(m_pSSData->setLVCDistributorNo(m_TabDistributor.GetCurSel()))
	{
		UpdateSubstation();
	}
	
	*pResult = 0;
}


void CNSMainSubstaionDlg::UpdateSubstation()
{	
	CString strType,strSize;
	m_pSSData->getSSType(strType);
	m_pSSData->getSSSize(strSize);
	InitSSControls(strType,strSize);

	//Added by Subir for Bug Fix on 27.06.2011
	m_strSSOptions = "";
	m_pSSData->setSSOptions(m_strSSOptions);
	///


	CString strSSDesc,strSSStockType;
	m_pSSData->getSSStockType(strSSStockType);
	m_pSSData->getSSDescription(strSSDesc);		


	m_CbSSDescription.SetCurSel(m_CbSSDescription.FindStringExact(-1,strSSDesc));
	m_CbSSStockType.SetCurSel(m_CbSSStockType.FindStringExact(-1,strSSStockType));

	CString strHVCEquip,strHVCType;
	m_pSSData->getHVCEquipment(strHVCEquip);
	m_pSSData->getHVCType(strHVCType);
	InitHVCControls(strHVCEquip,strHVCType);

	CString strTXRating,strTXTapRatio;
	m_pSSData->getTXRating(strTXRating);
	m_pSSData->getTXTapRatio(strTXTapRatio);
	InitTXControls(strTXRating,strTXTapRatio);
	int iDistributorNo = m_pSSData->getLVCDistributorNo();
	InitLVCControls(iDistributorNo);
}

// CEditDlg dialog

IMPLEMENT_DYNAMIC(CEditDlg, CDialog)

CEditDlg::CEditDlg(CString &strRename,CWnd* pParent /*=NULL*/)
: CDialog(CEditDlg::IDD, pParent),m_renameString(strRename)
{
	
}

CEditDlg::~CEditDlg()
{
}


void CEditDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_EDITRENAME,m_renameString);
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEditDlg, CDialog)
	ON_BN_CLICKED(IDC_OK, &CEditDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CANCEL, &CEditDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CEditDlg message handlers

void CEditDlg::OnBnClickedOk()
{
	UpdateData();	
	OnOK();
}
void CEditDlg::OnBnClickedCancel()
{
	OnCancel();
}

CSSTabCtrl::~CSSTabCtrl(void)
{
}
BEGIN_MESSAGE_MAP(CSSTabCtrl, CTabCtrl)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

void CSSTabCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{	
	TCITEM tabDistributor;
	tabDistributor.mask = TCIF_TEXT;
	tabDistributor.cchTextMax = 63;
	tabDistributor.pszText = new TCHAR[64];
	if(!GetItem(GetCurSel(),&tabDistributor)) return;
	CString strCaption = tabDistributor.pszText;
	CEditDlg RenameDialog(strCaption,this);
	RenameDialog.DoModal();
	if(wcscmp(tabDistributor.pszText,strCaption) == 0)
		return;
	strCaption.Trim(L' ');
	if(strCaption == L"")
	{
		::MessageBox(NULL,L"Empty Text",_T("NET CAD"),MB_OK|MB_ICONERROR);
		return;
	}
	for(int index = 0; index < GetItemCount(); index++)
	{
		GetItem(index,&tabDistributor);
		if(wcscmp(tabDistributor.pszText,strCaption) == 0)
		{
			::MessageBox(NULL,L"Duplicate Name",_T("NET CAD"),MB_OK|MB_ICONERROR);
			break;
		}
	}	
	if(wcscmp(tabDistributor.pszText,strCaption) != 0)
	{
		wcscpy(tabDistributor.pszText,strCaption);
		SetItem(GetCurSel(),&tabDistributor);
	}
	delete [] tabDistributor.pszText;
}


void CNSMainSubstaionDlg::OnCbnSelchangeHVCRating()
{
	CString strRating;
	m_CbHVCRating.GetLBText(m_CbHVCRating.GetCurSel(),strRating);
	m_pSSData->setHVCRating(strRating);
}
void CNSMainSubstaionDlg::OnCbnSelchangeLVCFuseA()
{
	CString strFuseA;
	m_CbLVCFuseA.GetLBText(m_CbLVCFuseA.GetCurSel(),strFuseA);
	m_pSSData->setLVCFuseAManual(m_pSSData->getLVCDistributorNo(),false);
	int iFuseA = _wtoi(strFuseA.GetString());
	if(m_pSSData->setLVCFuseA(m_pSSData->getLVCDistributorNo(),iFuseA))
	{
		UpdateSubstation();
	}
}

void CNSMainSubstaionDlg::OnCbnSelchangeLVCFuse()
{
	CString strFuse;
	m_pSSData->setLVCFuseManual(m_pSSData->getLVCDistributorNo(),false);
	m_CbLVCFuse.GetLBText(m_CbLVCFuse.GetCurSel(),strFuse);
	m_pSSData->setLVCFuse(m_pSSData->getLVCDistributorNo(), strFuse);	
}

void CNSMainSubstaionDlg::OnEnChangeLVCDistName()
{
	CString strLVCDistName;
	m_EditLVCDistName.GetWindowText(strLVCDistName);
	m_pSSData->setLVCDistributorName(m_pSSData->getLVCDistributorNo(),strLVCDistName);
}

void CNSMainSubstaionDlg::OnChkclickedLVCWDNO()
{
	if(m_ChkLVCWDNO.GetCheck() == BST_CHECKED)
	{
		m_pSSData->setLVCWDNO(m_pSSData->getLVCDistributorNo(),1);
	}
	else
	{
		m_pSSData->setLVCWDNO(m_pSSData->getLVCDistributorNo(),0);
	}	
}

void CNSMainSubstaionDlg::OnChkClickedLVCIDLINK()
{
	if(m_ChkLVCIDLINK.GetCheck() == BST_CHECKED)
	{
		m_pSSData->setLVCIDLINK(m_pSSData->getLVCDistributorNo(),1);
	}
	else
	{
		m_pSSData->setLVCIDLINK(m_pSSData->getLVCDistributorNo(),0);
	}	
}

void CNSMainSubstaionDlg::OnCbnSelchangeLVNoOfDist()
{
	CString strNoOfDistributors;
	m_CbLVNoOfDist.GetLBText(m_CbLVNoOfDist.GetCurSel(),strNoOfDistributors);
	
	CString strSSStockType;
	m_pSSData->getSSStockType(strSSStockType);
	if(strSSStockType.IsEmpty()==false)
	{
		resetDescnStockType();
		m_pSSData->clearLVCInfo();
	}
	int iNoOfDistributors = _wtoi(strNoOfDistributors);
	if(m_pSSData->setLVCNoOfDistributors(iNoOfDistributors))
	{
		if(iNoOfDistributors == 0)
		{
			m_pSSData->setLVCDistributorNo(iNoOfDistributors);
		}
		else
		{
			m_pSSData->setLVCDistributorNo(iNoOfDistributors-1);
		}
		UpdateSubstation();
		//m_TabDistributor.SetCurSel(iNoOfDistributors-1);
	}	
}

void CNSMainSubstaionDlg::OnCbnEditchangeHVCType()
{	
	CString strHVCType;
	m_CbHVCType.GetWindowText(strHVCType);
	if(m_CbHVCType.SelectString(-1,strHVCType)!=CB_ERR)
	{
		int icurTextLen = strHVCType.GetLength();
		m_CbHVCType.GetLBText(m_CbHVCType.GetCurSel(),strHVCType);
		if(m_pSSData->setHVCType(strHVCType))
		{
			UpdateSubstation();
		}
		m_CbHVCType.SetEditSel(icurTextLen, -1);
		return;
	}
	
	CString strSSStockType;
	m_pSSData->getSSStockType(strSSStockType);
	if(strSSStockType.IsEmpty()==false)
	{
		resetDescnStockType();
		m_CbHVCType.SetWindowText(strHVCType);
		m_CbHVCType.SetEditSel(strHVCType.GetLength(), -1);
	}
	m_CbHVCType.SetWindowText(strHVCType);
	m_CbHVCType.SetEditSel(strHVCType.GetLength(), -1);
	m_pSSData->setHVCType(strHVCType);

}

void CNSMainSubstaionDlg::OnCbnEditchangeHVCEquip()
{
	CString strHVCEquip;
	m_CbHVCEquipment.GetWindowText(strHVCEquip);
	if(m_CbHVCEquipment.SelectString(-1,strHVCEquip)!=CB_ERR)
	{
		int icurTextLen = strHVCEquip.GetLength();
		m_CbHVCEquipment.GetLBText(m_CbHVCEquipment.GetCurSel(),strHVCEquip);
		if(m_pSSData->setHVCEquipment(strHVCEquip))
		{
			UpdateSubstation();
		}
		m_CbHVCEquipment.SetEditSel(icurTextLen, -1);
		return;
	}
	
	CString strSSStockType;
	m_pSSData->getSSStockType(strSSStockType);
	if(strSSStockType.IsEmpty()==false)
	{
		resetDescnStockType();
		m_CbHVCEquipment.SetWindowText(strHVCEquip);
		m_CbHVCEquipment.SetEditSel(strHVCEquip.GetLength(), -1);
	}
	m_CbHVCEquipment.SetWindowText(strHVCEquip);
	m_CbHVCEquipment.SetEditSel(strHVCEquip.GetLength(), -1);
	m_pSSData->setHVCEquipment(strHVCEquip);
}

void CNSMainSubstaionDlg::OnCbnEditchangeHVCRating()
{
	CString strHVCRating;
	m_CbHVCRating.GetWindowText(strHVCRating);
	if(m_CbHVCRating.SelectString(-1,strHVCRating)!=CB_ERR)
	{
		int icurTextLen = strHVCRating.GetLength();
		m_CbHVCRating.GetLBText(m_CbHVCRating.GetCurSel(),strHVCRating);
		if(m_pSSData->setHVCRating(strHVCRating))
		{
			UpdateSubstation();
		}
		m_CbHVCRating.SetEditSel(icurTextLen, -1);
		return;
	}
	
	CString strSSStockType;
	m_pSSData->getSSStockType(strSSStockType);
	if(strSSStockType.IsEmpty()==false)
	{
		resetDescnStockType();
		m_CbHVCRating.SetWindowText(strHVCRating);
		m_CbHVCRating.SetEditSel(strHVCRating.GetLength(), -1);
	}
	m_CbHVCRating.SetWindowText(strHVCRating);
	m_CbHVCRating.SetEditSel(strHVCRating.GetLength(), -1);
	m_pSSData->setHVCRating(strHVCRating);
}

void CNSMainSubstaionDlg::OnCbnEditchangeTXRating()
{
	CString strTXRating;
	m_CbTXRating.GetWindowText(strTXRating);
	if(m_CbTXRating.SelectString(-1,strTXRating)!=CB_ERR)
	{
		int icurTextLen = strTXRating.GetLength();
		m_CbTXRating.GetLBText(m_CbTXRating.GetCurSel(),strTXRating);
		if(m_pSSData->setTXRating(strTXRating))
		{
			UpdateSubstation();
		}
		m_CbTXRating.SetEditSel(icurTextLen, -1);
		return;
	}
	
	CString strSSStockType;
	m_pSSData->getSSStockType(strSSStockType);
	if(strSSStockType.IsEmpty()==false)
	{
		resetDescnStockType();
		m_CbTXRating.SetWindowText(strTXRating);
		m_CbTXRating.SetEditSel(strTXRating.GetLength(), -1);
	}
	m_CbTXRating.SetWindowText(strTXRating);
	m_CbTXRating.SetEditSel(strTXRating.GetLength(), -1);
	m_pSSData->setTXRating(strTXRating);
	// TODO: Add your control notification handler code here
}

void CNSMainSubstaionDlg::OnEnChangeTXVoltage()
{	
	CWnd* curWindow = GetFocus();
	if(curWindow != (&m_EditTXVoltage))
		return;
	CString strEnteredText,strTxVoltage;
	static bool m_bCallRecursive = false;
	if(m_bCallRecursive)
	{
		m_bCallRecursive = false;
		return;
	}
	m_EditTXVoltage.GetWindowText(strEnteredText);
	m_pSSData->getTXVoltage(strTxVoltage);

	if(strEnteredText.IsEmpty()== false && strTxVoltage.Find(strEnteredText,0)==0)
	{
		m_bCallRecursive = true;
		m_EditTXVoltage.SetWindowText(strTxVoltage);
		m_EditTXVoltage.SetSel(strEnteredText.GetLength(),-1);
		return;
	}
	strTxVoltage = strEnteredText;
	CString strSSStockType;
	m_pSSData->getSSStockType(strSSStockType);
	if(strSSStockType.IsEmpty()==false)
	{
		resetDescnStockType();
		m_bCallRecursive = true;
		m_EditTXVoltage.SetWindowText(strTxVoltage);
	}
	m_pSSData->setTXVoltage(strTxVoltage);
}

void CNSMainSubstaionDlg::OnEnChangeTXPhases()
{
	CWnd* curWindow = GetFocus();
	if(curWindow != (&m_EditTXPhases))
		return;
	static bool m_bCallRecursive = false;
	if(m_bCallRecursive)
	{
		m_bCallRecursive = false;
		return;
	}
	CString strEnteredText,strTxPhase;
	int iPhases = m_pSSData->getTXPhases();
	strTxPhase.FormatMessage(L"%1!d!",iPhases);
	m_EditTXPhases.GetWindowText(strEnteredText);
	
	if(strEnteredText.IsEmpty()== false && strTxPhase.Find(strEnteredText,0)==0)
	{
		if(iPhases >0)
		{
			m_bCallRecursive = true;
			m_EditTXPhases.SetWindowText(strTxPhase);
			m_EditTXPhases.SetSel(strEnteredText.GetLength(),-1);
		}
		return;
	}
	strTxPhase = strEnteredText;
	CString strSSStockType;
	m_pSSData->getSSStockType(strSSStockType);
	if(strSSStockType.IsEmpty()==false)
	{
		resetDescnStockType();
		m_bCallRecursive = true;
		m_EditTXPhases.SetWindowText(strTxPhase);
	}
	m_pSSData->setTXPhases(_wtoi(strTxPhase));
}

void CNSMainSubstaionDlg::OnEnChangeTXTapsetting()
{
	CWnd* curWindow = GetFocus();
	if(curWindow != (&m_EditTXTapSetting))
		return;
	static bool m_bCallRecursive = false;
	if(m_bCallRecursive)
	{
		m_bCallRecursive = false;
		return;
	}
	CString strEnteredText,strTxTapSetting;
	int iTapSetting = m_pSSData->getTXTapSetting();
	strTxTapSetting.FormatMessage(L"%1!d!",iTapSetting);
	m_EditTXTapSetting.GetWindowText(strEnteredText);
	
	if(strEnteredText.IsEmpty()== false && strTxTapSetting.Find(strEnteredText,0)==0)
	{
		if(iTapSetting >0)
		{
			m_bCallRecursive = true;
			m_EditTXTapSetting.SetWindowText(strTxTapSetting);
			m_EditTXTapSetting.SetSel(strEnteredText.GetLength(),-1);
		}
		return;
	}
	strTxTapSetting = strEnteredText;
	//m_pSSData->setTxTapSettingManual(true); // Commented by Subir on 01.07.2011 for bug fix
	//CString strSSType;
	//m_pSSData->getSSType(strSSType);
	//if(strSSType.IsEmpty()==false)
	//{
	//	InitSSControls();
	//	UpdateSubstation();
	//	m_bCallRecursive = true;
	//	m_EditTXTapSetting.SetWindowText(strTxTapSetting);
	//}
	m_pSSData->setTXTapSetting(_wtoi(strTxTapSetting));
}

void CNSMainSubstaionDlg::OnCbnEditchangeLVCFuseA()
{
	CString strLVCFuseA;
	m_CbLVCFuseA.GetWindowText(strLVCFuseA);
	if(m_CbLVCFuseA.SelectString(-1,strLVCFuseA)!=CB_ERR)
	{
		int icurTextLen = strLVCFuseA.GetLength();
		m_CbLVCFuseA.GetLBText(m_CbLVCFuseA.GetCurSel(),strLVCFuseA);
		if(m_pSSData->setLVCFuseA(m_TabDistributor.GetCurSel(),_wtoi(strLVCFuseA)))
		{
			UpdateSubstation();
		}
		m_CbLVCFuseA.SetEditSel(icurTextLen, -1);
		return;
	}
	m_CbLVCFuseA.SetWindowText(strLVCFuseA);
	m_CbLVCFuseA.SetEditSel(strLVCFuseA.GetLength(), -1);
	m_pSSData->setLVCFuseAManual(m_TabDistributor.GetCurSel(),true);
	m_pSSData->setLVCFuseManual(m_TabDistributor.GetCurSel(),true);
	m_pSSData->setLVCFuseA(m_TabDistributor.GetCurSel(),_wtoi(strLVCFuseA));
	
}

void CNSMainSubstaionDlg::OnCbnEditchangeLVCFuse()
{	
	CString strLVCFuse;
	m_CbLVCFuse.GetWindowText(strLVCFuse);
	if(m_CbLVCFuse.SelectString(-1,strLVCFuse)!=CB_ERR)
	{
		int icurTextLen = strLVCFuse.GetLength();
		m_CbLVCFuse.GetLBText(m_CbLVCFuse.GetCurSel(),strLVCFuse);
		if(m_pSSData->setLVCFuse(m_TabDistributor.GetCurSel(),strLVCFuse))
		{
			UpdateSubstation();
		}
		m_CbLVCFuse.SetEditSel(icurTextLen, -1);
		return;
	}
	m_CbLVCFuse.SetWindowText(strLVCFuse);
	m_CbLVCFuse.SetEditSel(strLVCFuse.GetLength(), -1);
	m_pSSData->setLVCFuseManual(m_TabDistributor.GetCurSel(),true);
	m_pSSData->setLVCFuse(m_TabDistributor.GetCurSel(),strLVCFuse);

}
void CNSMainSubstaionDlg::resetDescnStockType()
{
	CString strSSType,strSSSize;
	m_pSSData->getSSType(strSSType);
	m_pSSData->getSSSize(strSSSize);
	m_pSSData->setSSDescription("");
	m_pSSData->setSSStockType("");
	InitSSControls(strSSType,strSSSize);
	UpdateSubstation();
}

void CNSMainSubstaionDlg::OnBnClickedClear()
{
	InitSSControls();
	m_strSSPrefix = "";
	m_strSSNumber = "";
	m_strSSName = "";
	m_strHVCSideA = "";
	m_strHVCSideB = "";
	m_strHVCNumber = "";
	m_strSSOptions = "";
	m_strSSName = "";
	m_iHVCEfiA = 0;
	m_iHVCEfiB = 0;
	UpdateData(FALSE);
	UpdateSubstation();
}

BOOL CNSMainSubstaionDlg::PreTranslateMessage(MSG* pMsg) 
{ 
   if(pMsg->message==WM_KEYDOWN) 
   { 
	   HWND hnd1,hnd2;
	   GetDlgItem(IDC_EDITSIDEA,&hnd1);
	   GetDlgItem(IDC_EDITSIDEB,&hnd2);
	   if ((pMsg->hwnd == hnd1) || (pMsg->hwnd == hnd2))
			return CDialog::PreTranslateMessage(pMsg); 

      if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE) 
      { 
         pMsg->wParam=NULL ; 
      } 
   } 
   return CDialog::PreTranslateMessage(pMsg); 
}



