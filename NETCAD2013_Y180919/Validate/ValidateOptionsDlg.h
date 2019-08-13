#pragma once


// CValidateOptionsDlg dialog

class CValidateOptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CValidateOptionsDlg)

public:
	CValidateOptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CValidateOptionsDlg();

// Dialog Data
	enum { IDD = IDD_VALIDATE_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  afx_msg void OnBnClickedHelp();

	DECLARE_MESSAGE_MAP()

public:
  BOOL m_bBlocks;
  BOOL m_bOthers;
  BOOL m_bExclude;
};
