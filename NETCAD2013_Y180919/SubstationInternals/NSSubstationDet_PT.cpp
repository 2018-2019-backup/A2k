// NSSubstationDet_PT.cpp : implementation file
//

#include "stdafx.h"
#include "NSSubstationDet_PT.h"


// CNSSubstationDet_PT dialog

IMPLEMENT_DYNAMIC(CNSSubstationDet_PT, CDialog)

CNSSubstationDet_PT::CNSSubstationDet_PT(CString &strSSOptions, CSSData *pSSData, CWnd* pParent /*=NULL*/)
	: CDialog(CNSSubstationDet_PT::IDD, pParent)
	, m_pSSData(pSSData)
	,m_strSSOptions(strSSOptions)
	,m_strMasterOptions(strSSOptions)
{

	m_pSSData->getSSSize(m_strSSSize);
	m_pSSData->getSSType(m_strSSType);
	m_pSSData->getSSNumber(m_strSSNumber);
	m_pSSData->getSSName(m_strSSName);
	m_pSSData->getSSPrefix(m_strSSPrefix);
	m_pSSData->getTXRating(m_strTXRating);
	m_pSSData->getTXPoleLength(m_strTxPoleLength);
	m_pSSData->getTXPoleStrength(m_strTxPoleStrength);
}

CNSSubstationDet_PT::~CNSSubstationDet_PT()
{
}

void CNSSubstationDet_PT::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_EDITPTSIZE, m_strSSSize);
	DDX_Text(pDX, IDC_EDITPTTYPE, m_strSSType);
	DDX_Text(pDX, IDC_EDITPTNUMBER, m_strSSNumber);
	DDX_Text(pDX, IDC_EDITPTSSNAME, m_strSSName);
	DDX_Text(pDX, IDC_EDITPTPREFIX, m_strSSPrefix);
	DDX_Text(pDX, IDC_EDITPTTXRATING, m_strTXRating);
	DDX_Text(pDX, IDC_EDITDETAILS1, m_strDetails[0]);
	DDX_Text(pDX, IDC_EDITDETAILS2, m_strDetails[1]);
	DDX_Text(pDX, IDC_EDITDETAILS3, m_strDetails[2]);
	DDX_Text(pDX, IDC_EDITDETAILS4, m_strDetails[3]);
	DDX_Text(pDX, IDC_EDITDETAILS5, m_strDetails[4]);
	DDX_Text(pDX, IDC_EDITDETAILS6, m_strDetails[5]);
	DDX_Text(pDX, IDC_EDITDETAILS7, m_strDetails[6]);
	DDX_Text(pDX, IDC_EDITDETAILS8, m_strDetails[7]);
	DDX_Text(pDX, IDC_EDITDETAILS9, m_strDetails[8]);
	DDX_Text(pDX, IDC_EDITDETAILS10, m_strDetails[9]);
	DDX_Text(pDX, IDC_EDITDETAILS11, m_strDetails[10]);
	DDX_Text(pDX, IDC_EDITDETAILS12, m_strDetails[11]);
	DDX_Text(pDX, IDC_EDITDETAILS13, m_strDetails[12]);
	DDX_Text(pDX, IDC_EDITDETAILS14, m_strDetails[13]);
	DDX_Text(pDX, IDC_EDITDETAILS15, m_strDetails[14]);
	DDX_Text(pDX, IDC_EDITMASTEROPTIONS, m_strMasterOptions);
	
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CBPTLENGTH, m_cbPTLength);
	DDX_Control(pDX, IDC_CBPTSTRENGTH, m_CbPTStrength);
	DDX_Control(pDX, IDC_CBITEM1, m_CbItem[0]);
	DDX_Control(pDX, IDC_CBITEM2, m_CbItem[1]);
	DDX_Control(pDX, IDC_CBITEM3, m_CbItem[2]);
	DDX_Control(pDX, IDC_CBITEM4, m_CbItem[3]);
	DDX_Control(pDX, IDC_CBITEM5, m_CbItem[4]);
	DDX_Control(pDX, IDC_CBITEM6, m_CbItem[5]);
	DDX_Control(pDX, IDC_CBITEM7, m_CbItem[6]);
	DDX_Control(pDX, IDC_CBITEM8, m_CbItem[7]);
	DDX_Control(pDX, IDC_CBITEM9, m_CbItem[8]);
	DDX_Control(pDX, IDC_CBITEM10, m_CbItem[9]);
	DDX_Control(pDX, IDC_CBITEM11, m_CbItem[10]);
	DDX_Control(pDX, IDC_CBITEM12, m_CbItem[11]);
	DDX_Control(pDX, IDC_CBITEM13, m_CbItem[12]);
	DDX_Control(pDX, IDC_CBITEM14, m_CbItem[13]);
	DDX_Control(pDX, IDC_CBITEM15, m_CbItem[14]);
}


BEGIN_MESSAGE_MAP(CNSSubstationDet_PT, CDialog)
	ON_CBN_SELCHANGE(IDC_CBITEM1, &CNSSubstationDet_PT::OnCbnSelchangeItem1)
	ON_CBN_SELCHANGE(IDC_CBITEM2, &CNSSubstationDet_PT::OnCbnSelchangeItem2)
	ON_CBN_SELCHANGE(IDC_CBITEM3, &CNSSubstationDet_PT::OnCbnSelchangeItem3)
	ON_CBN_SELCHANGE(IDC_CBITEM4, &CNSSubstationDet_PT::OnCbnSelchangeItem4)
	ON_CBN_SELCHANGE(IDC_CBITEM5, &CNSSubstationDet_PT::OnCbnSelchangeItem5)
	ON_CBN_SELCHANGE(IDC_CBITEM6, &CNSSubstationDet_PT::OnCbnSelchangeItem6)
	ON_CBN_SELCHANGE(IDC_CBITEM7, &CNSSubstationDet_PT::OnCbnSelchangeItem7)
	ON_CBN_SELCHANGE(IDC_CBITEM8, &CNSSubstationDet_PT::OnCbnSelchangeItem8)
	ON_CBN_SELCHANGE(IDC_CBITEM9, &CNSSubstationDet_PT::OnCbnSelchangeItem9)
	ON_CBN_SELCHANGE(IDC_CBITEM10, &CNSSubstationDet_PT::OnCbnSelchangeItem10)
	ON_CBN_SELCHANGE(IDC_CBITEM11, &CNSSubstationDet_PT::OnCbnSelchangeItem11)
	ON_CBN_SELCHANGE(IDC_CBITEM12, &CNSSubstationDet_PT::OnCbnSelchangeItem12)
	ON_CBN_SELCHANGE(IDC_CBITEM13, &CNSSubstationDet_PT::OnCbnSelchangeItem13)
	ON_CBN_SELCHANGE(IDC_CBITEM14, &CNSSubstationDet_PT::OnCbnSelchangeItem14)
	ON_CBN_SELCHANGE(IDC_CBITEM15, &CNSSubstationDet_PT::OnCbnSelchangeItem15)
	ON_BN_CLICKED(IDOK, &CNSSubstationDet_PT::OnBnClickedOk)
	ON_CBN_EDITCHANGE(IDC_CBPTLENGTH, &CNSSubstationDet_PT::OnCbnEditchangePTLength)
	ON_CBN_EDITCHANGE(IDC_CBPTSTRENGTH, &CNSSubstationDet_PT::OnCbnEditchangePTStrength)
	ON_CBN_SELCHANGE(IDC_CBPTLENGTH, &CNSSubstationDet_PT::OnCbnSelchangePTLength)
	ON_CBN_SELCHANGE(IDC_CBPTSTRENGTH, &CNSSubstationDet_PT::OnCbnSelchangePTStrength)
END_MESSAGE_MAP()


// CNSSubstationDet_PT message handlers

BOOL CNSSubstationDet_PT::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	UpdateData(FALSE);
	std::vector<variant_t> vData;
	std::vector<variant_t>::iterator iter;
	m_pSSData->getTXPoleLengths(vData,m_strSSSize, m_strTXRating);
	ResetCBcontrol(m_cbPTLength,vData);
	m_pSSData->getTXPoleStrengths(vData,m_strSSSize, m_strTXRating);
	ResetCBcontrol(m_CbPTStrength,vData);
	int nPos;
	if((nPos = m_cbPTLength.FindStringExact(-1,m_strTxPoleLength))!= CB_ERR)
	{
		m_cbPTLength.SetCurSel(nPos);
	}
	if((nPos = m_CbPTStrength.FindStringExact(-1,m_strTxPoleStrength))!= CB_ERR)
	{
		m_CbPTStrength.SetCurSel(nPos);
	}

	for(int index = 0; index < 15; index++)
	{
		variant_t vtVal;
		vtVal.SetString("");
		m_pSSData->getSSOptionsFromDB(vData,m_strSSType,m_strSSSize,index + 1);
		vData.push_back(vtVal);
		ResetCBcontrol(m_CbItem[index],vData);	
		CString ssOption;
		int i=0;
		for( iter = vData.begin(); iter != vData.end(); iter++,i++)
		{
			ssOption = iter->bstrVal;
			if(m_strSSOptions.Find(ssOption)!=-1)
			{
				m_CbItem[index].SetCurSel(i);
				updateSubstationOptions(index+1);
				break;
			}
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void  CNSSubstationDet_PT::ResetCBcontrol(CComboBox &CbControl,const std::vector<variant_t> &vData)
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
}
void CNSSubstationDet_PT::updateSubstationOptions(int iCurItem)
{
	CString strOption;
	m_CbItem[iCurItem-1].GetLBText(m_CbItem[iCurItem-1].GetCurSel(),strOption);
	m_pSSData->getSSDetails(m_strDetails[iCurItem-1], strOption, m_strSSType, m_strSSSize);
	CString strVal;
	m_strMasterOptions = "";
	for (int index=0; index<15; index++)
	{
		int nPos;
		strVal = "";
		if((nPos=m_CbItem[index].GetCurSel())!=CB_ERR)
		{
			m_CbItem[index].GetLBText(nPos,strVal);
		}
		if(strVal.IsEmpty() == false)
		{
			if(m_strMasterOptions.IsEmpty() == false)
				m_strMasterOptions = m_strMasterOptions + "/";
			m_strMasterOptions = m_strMasterOptions + strVal;
		}
	}
	UpdateData(FALSE);	
}

void CNSSubstationDet_PT::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString strVal;
	m_strSSOptions = m_strMasterOptions;
	m_pSSData->setTXPoleLength(m_strTxPoleLength);
	m_pSSData->setTXPoleStrength(m_strTxPoleStrength);
	OnOK();
}
void CNSSubstationDet_PT::OnCbnEditchangePTLength()
{
	m_cbPTLength.GetWindowText(m_strTxPoleLength);
	int icurTextLen = m_strTxPoleLength.GetLength();
	if(m_cbPTLength.SelectString(-1,m_strTxPoleLength)!=CB_ERR)
	{
		m_cbPTLength.GetLBText(m_cbPTLength.GetCurSel(),m_strTxPoleLength);
		m_cbPTLength.SetEditSel(icurTextLen, -1);
		return;
	}
	m_cbPTLength.SetWindowText(m_strTxPoleLength);
	m_cbPTLength.SetEditSel(icurTextLen, -1);
}

void CNSSubstationDet_PT::OnCbnEditchangePTStrength()
{
	m_CbPTStrength.GetWindowText(m_strTxPoleStrength);
	int icurTextLen = m_strTxPoleStrength.GetLength();
	if(m_CbPTStrength.SelectString(-1,m_strTxPoleStrength)!=CB_ERR)
	{
		m_CbPTStrength.GetLBText(m_CbPTStrength.GetCurSel(),m_strTxPoleStrength);
		m_CbPTStrength.SetEditSel(icurTextLen, -1);
		return;
	}
	m_CbPTStrength.SetWindowText(m_strTxPoleStrength);
	m_CbPTStrength.SetEditSel(icurTextLen, -1);
}

void CNSSubstationDet_PT::OnCbnSelchangePTLength()
{
	CString strPoleLength;
	m_cbPTLength.GetLBText(m_cbPTLength.GetCurSel(),m_strTxPoleLength);
}

void CNSSubstationDet_PT::OnCbnSelchangePTStrength()
{
	CString strPoleStrength;
	m_CbPTStrength.GetLBText(m_CbPTStrength.GetCurSel(),m_strTxPoleStrength);
}
