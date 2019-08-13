#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "SystemImageList.h"

// CFileVersionsDlg dialog

class CFileVersionsDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileVersionsDlg)

public:
	CFileVersionsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileVersionsDlg();

// Dialog Data
	enum { IDD = IDD_VERSION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnSortList(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnBnClickedSave();
	DECLARE_MESSAGE_MAP()

public:
  CStatic m_stUpdate;
  CListCtrl m_lcFiles;
  CSystemImageList m_silFiles;

private:

  static int m_iSortColumn;
  static bool m_bAscendingSort;
  static int CALLBACK CompareItemsProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

  void AddFileDetails(CString csFilePath);
};
