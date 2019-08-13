#pragma once
#include "afxwin.h"


// CValidateExtOptionsDlg dialog

class CValidateExtOptionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CValidateExtOptionsDlg)

public:
	CValidateExtOptionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CValidateExtOptionsDlg();

// Dialog Data
	enum { IDD = IDD_EXT_VALIDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnBnClickedExtBlocks();
  afx_msg void OnBnClickedExtBlocksRename();
  afx_msg void OnBnClickedExtBlocksReplace();
  afx_msg void OnBnClickedOk();
	DECLARE_MESSAGE_MAP()

public:
  CComboBox m_cbESPName;
  CButton m_btnBlocks;
  int m_iBlockCheckType;
  BOOL m_bBlocks;
  BOOL m_bLayers;
  BOOL m_bTextStyles;
  BOOL m_bLineTypes;
  BOOL m_bDimStyles;
  CString m_csESPName, m_csESPTable;

  CStringArray m_csaESPTables;
};
