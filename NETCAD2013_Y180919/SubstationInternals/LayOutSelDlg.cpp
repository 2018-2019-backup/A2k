// LayOutSelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LayOutSelDlg.h"


// CLayOutSelDlg dialog

IMPLEMENT_DYNAMIC(CLayOutSelDlg, CDialog)

CLayOutSelDlg::CLayOutSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLayOutSelDlg::IDD, pParent)
	, strAvailableLayOutNames(_T(""))
{
	strSeletedLayOutName = _T("");
}

CLayOutSelDlg::~CLayOutSelDlg()
{
}

void CLayOutSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_COMBO_LAYOUTS_NEW, strAvailableLayOutNames);
	DDX_Control(pDX, IDC_COMBO_LAYOUTS_NEW, m_AvailableLayOutNames);
}

BOOL CLayOutSelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//m_AvailableLayOutNames.AddString(_T("Lay 1"));
	PopulateLayOutNames();

	return TRUE;  
}

//int CLayOutSelDlg::PopulateLayOutNames()
//{
//	AcDbDictionary* pDict;
//	AcDbLayout* layout;
//	const TCHAR* name;
//	Acad::ErrorStatus es = acdbHostApplicationServices()->workingDatabase()->getLayoutDictionary(pDict,AcDb::kForRead);
//	AcDbDictionaryIterator* dicIter = pDict->newIterator();
//
//	for(;!dicIter->done();dicIter->next())
//	{		
//		dicIter->getObject((AcDbObject*&)layout,AcDb::kForRead);
//		layout->getLayoutName(name);
//
//		if((NSSTRCMP(name, _T("Model")) != 0))
//		{
//			m_AvailableLayOutNames.AddString(name);	
//		}
//	}
//
//	delete dicIter;
//	//dicIter->close();
//
//	//delete layout;
//	layout->close();
//	
//	//delete pDict;
//	pDict->close();
//
//	
//	dicIter = NULL;
//	layout = NULL;
//	pDict = NULL;
//
//	return NS_SUCCESS;
//}
//
//
//

int CLayOutSelDlg::PopulateLayOutNames()
{
	Acad::ErrorStatus es;
	const TCHAR* name;

	AcDbDictionary* pDict;
	es = acdbHostApplicationServices()->workingDatabase()->getLayoutDictionary(pDict,AcDb::kForRead);


			AcDbDictionaryIterator* dicIter = pDict->newIterator();

					for(;!dicIter->done();dicIter->next())
					{		
						AcDbLayout* layout;
						dicIter->getObject((AcDbObject*&)layout,AcDb::kForRead);
						layout->getLayoutName(name);

								if((NSSTRCMP(name, _T("Model")) != 0))
								{
									m_AvailableLayOutNames.AddString(name);	
								}
						
								
						//delete layout;
						layout->close();
					}

			delete dicIter;
			//dicIter->close();	
	
	//delete pDict;
	pDict->close();

	
	//dicIter = NULL;
	//layout = NULL;
	//pDict = NULL;

	m_AvailableLayOutNames.SetCurSel(0);	
	return NS_SUCCESS;
}





BEGIN_MESSAGE_MAP(CLayOutSelDlg, CDialog)
//	ON_BN_CLICKED(IDOK, &CLayOutSelDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CLayOutSelDlg message handlers

void CLayOutSelDlg::OnBnClickedOk()
{
  //////////////////////////////////////////////////////////////////
  // Code commented ~SJ, DCS, 01.07.2013
  // This is not required since strAvailableLayOutNames is already mapped
  //
	//m_AvailableLayOutNames.GetWindowTextW(strSeletedLayOutName);
  //
  // End code commented ~SJ, DCS, 01.07.2013

	OnOK();
}
