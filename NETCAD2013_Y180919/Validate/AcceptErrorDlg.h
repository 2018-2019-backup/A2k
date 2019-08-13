#pragma once


// CAcceptErrorDlg dialog

class CAcceptErrorDlg : public CDialog
{
	DECLARE_DYNAMIC(CAcceptErrorDlg)

public:
	CAcceptErrorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAcceptErrorDlg();

// Dialog Data
	enum { IDD = IDD_VALIDATE_ACCEPT };

  CString m_csErrNo;
  CString m_csErrType;
  CString m_csErrReason;

  BOOL m_bEditing;

protected:
  virtual BOOL OnInitDialog();
  afx_msg void OnBnClickedOk();

	DECLARE_MESSAGE_MAP()
};
