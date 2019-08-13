// NSSubstationDet_Kiosk.cpp : implementation file
//

#include "stdafx.h"
#include "NSSubstationDet_Kiosk.h"


// CNSSubstationDet_Kiosk dialog

IMPLEMENT_DYNAMIC(CNSSubstationDet_Kiosk, CDialog)

CNSSubstationDet_Kiosk::CNSSubstationDet_Kiosk(CString &strSSOptions, CSSData *pSSData, CWnd* pParent /*=NULL*/)
	: CDialog(CNSSubstationDet_Kiosk::IDD, pParent)
	, m_pSSData(pSSData)
	,m_strSSOptions(strSSOptions)
{
	m_pSSData->getSSSize(m_strSSSize);
	m_pSSData->getSSType(m_strSSType);
	m_pSSData->getSSNumber(m_strSSNumber);
	m_pSSData->getSSName(m_strSSName);
	m_pSSData->getSSPrefix(m_strSSPrefix);
	m_strEA = m_strSSOptions;

}

CNSSubstationDet_Kiosk::~CNSSubstationDet_Kiosk()
{
}

void CNSSubstationDet_Kiosk::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_EDITKSSSIZE, m_strSSSize);
	DDX_Text(pDX, IDC_EDITKSSTYPE, m_strSSType);
	DDX_Text(pDX, IDC_EDITKSSNUMBER, m_strSSNumber);
	DDX_Text(pDX, IDC_EDITKSSNAME, m_strSSName);
	DDX_Text(pDX, IDC_EDITKSSPREFIX, m_strSSPrefix);
	DDX_Text(pDX, IDC_EDITKSSEA, m_strEA);
	DDX_Control(pDX, IDC_CBKSSOPTION, m_CbKSSOption);
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNSSubstationDet_Kiosk, CDialog)
	ON_CBN_SELCHANGE(IDC_CBKSSOPTION, &CNSSubstationDet_Kiosk::OnCbnSelchangeKSSOption)
	ON_BN_CLICKED(IDOK, &CNSSubstationDet_Kiosk::OnBnClickedOk)
END_MESSAGE_MAP()


// CNSSubstationDet_Kiosk message handlers

BOOL CNSSubstationDet_Kiosk::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(FALSE);
	std::vector<variant_t> vOptions;
	m_pSSData->getSSOptionsFromDB(vOptions, m_strSSType, m_strSSSize,1);
	std::vector<variant_t>::iterator iter;
	int index;	
	for( iter = vOptions.begin(),index=0; iter != vOptions.end(); iter++)
	{
		m_CbKSSOption.InsertString(index++,CString(iter->bstrVal));
	}
	m_CbKSSOption.InsertString(index,L"");
	if(m_strSSOptions.IsEmpty() == false)
	{
		CString strSSDetails;
		for( iter = vOptions.begin(),index=0; iter != vOptions.end(); iter++,index++)
		{
			m_pSSData->getSSDetails(strSSDetails,iter->bstrVal,m_strSSType,m_strSSSize);
			if(strSSDetails == m_strSSOptions)
			{
				m_CbKSSOption.SetCurSel(index);
				break;
			}
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CNSSubstationDet_Kiosk::OnCbnSelchangeKSSOption()
{
	CString strOption;
	m_CbKSSOption.GetLBText(m_CbKSSOption.GetCurSel(),strOption);
	m_pSSData->getSSDetails(m_strEA,strOption,m_strSSType,m_strSSSize);
	UpdateData(FALSE);	
}

void CNSSubstationDet_Kiosk::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	m_strSSOptions = m_strEA;
	OnOK();
}
