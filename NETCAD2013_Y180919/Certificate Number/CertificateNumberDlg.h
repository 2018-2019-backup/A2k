#pragma once


// CCertificateNumberDlg dialog

class CCertificateNumberDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCertificateNumberDlg)

public:
	CCertificateNumberDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCertificateNumberDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CERTIFICATENUMBER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnUpdate();
	CString m_csNewNumber;
};
