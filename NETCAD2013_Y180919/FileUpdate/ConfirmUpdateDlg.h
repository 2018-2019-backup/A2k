#pragma once


// CConfirmUpdateDlg dialog

class CConfirmUpdateDlg : public CDialog
{
	DECLARE_DYNAMIC(CConfirmUpdateDlg)

public:
	CConfirmUpdateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfirmUpdateDlg();

// Dialog Data
	enum { IDD = IDD_CONFIRM_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  afx_msg void OnBnClickedOk();
  virtual BOOL OnInitDialog();

  DECLARE_MESSAGE_MAP()

public:
  BOOL m_bRemember;
  CString m_csRegSection;
  afx_msg void OnBnClickedCancel();
};
