#pragma once

#include "dlgs.h"

// CSelectDXFDlg

class CSelectDXFDlg : public CFileDialog
{
  DECLARE_DYNAMIC(CSelectDXFDlg)

public:
  CString m_csScale;
  CString m_csConversion;
  CString m_csTemplate;

  CStatic m_stScale;
  CComboBox m_cbScale;
  CStatic m_stTemplate1;
  CEdit m_edTemplate2;
  CButton m_btnTemplate;
  CButton m_btnHelp;
  CButton m_btnUseTemplate;
  CButton m_btnImportInto;
  CButton m_btnRunOverkill;
  CEdit m_edImportInto;

  BOOL m_bIntoCurrent;
  BOOL m_bUseTemplate;
  BOOL m_bRunOverkill;

  CSelectDXFDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL, BOOL bVistaStyle = TRUE);
	virtual ~CSelectDXFDlg();

protected:
  virtual BOOL OnInitDialog();
  virtual BOOL OnFileNameOK();
  afx_msg void OnBnClickedHelp();
  afx_msg void OnBnClickedTemplate();
  afx_msg void OnBnClickedImportInto();
  afx_msg void OnBnClickedUseTemplate();
  afx_msg void OnBnClickedRunOverkill();

  DECLARE_MESSAGE_MAP()

public:
  static CSelectDXFDlg *m_pThis;
  static WNDPROC m_wndProc;
  static LRESULT CALLBACK WindowProcNew(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
  

