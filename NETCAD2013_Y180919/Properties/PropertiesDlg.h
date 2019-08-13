#pragma once
#include "afxwin.h"
#include "PropList.h"

// CPropertiesDlg dialog

class CPropertiesDlg : public CDialog
{
	DECLARE_DYNAMIC(CPropertiesDlg)

public:
	CPropertiesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPropertiesDlg();

// Dialog Data
	enum { IDD = IDD_PROPERTIES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  afx_msg void OnSelectObjectType();
  afx_msg void OnSelectUse();
  afx_msg void OnSelectIns();
  afx_msg void OnSelectCode();
  afx_msg void OnSelectDesc();
  afx_msg void OnSelectName();
  afx_msg void OnCheckDrawDesc();
  afx_msg void OnCheckDrawName();
  afx_msg void OnBnClickedReset();
  afx_msg void OnBnClickedRemove();
  afx_msg void OnBnClickedApply();
  afx_msg void OnBnClickedSelect();
  afx_msg void OnBnClickedRegen();
  afx_msg void OnBnClickedHelp();

  afx_msg void OnPropertyChanged(LPCTSTR PropertyName, const VARIANT FAR& NewValue);
  DECLARE_EVENTSINK_MAP()

  DECLARE_MESSAGE_MAP()

public:
  CComboBox m_cbType;
  CPropList m_plList;
  CButton m_btnApply;
  CButton m_btnRemove;

  CStatic m_stCondLabel, m_stCondLine;
  CStatic m_stMandLabel, m_stMandLine;
  CStatic m_stLabelUse, m_stLabelIns;
  CStatic m_stLabelCode, m_stLabelDesc, m_stLabelName;
  CComboBox m_cbUse, m_cbIns;
  CComboBox m_cbCode, m_cbDesc, m_cbName;
  CButton m_btnDrawDesc;
  CButton m_btnDrawName;
  CButton m_btnReset;

  BOOL m_bDrawDesc;
  BOOL m_bDrawName;
  CStringArray m_csaPickedHandles;

private:
  int m_iObjectType;
  CString m_csUse, m_csIns, m_csCode, m_csDesc, m_csName, m_csWriteText;
  CUIntArray m_uiaBlockQtys;
  CStringArray m_csaLayers, m_csaBlockNames, m_csaTypes, m_csaHandles, m_csaSelHandles, m_csaSelLayers, m_csaSelLinearHandles;
  CStringArray m_csaPropTags, m_csaPropNames, m_csaPropValues, m_csaPropMand, m_csaPropUnique;
  CStringArray m_csaCodes, m_csaDescs, m_csaNames;

  void ResetArrays();
  void SetLinearVisibility(int iVis);
  void SetSelectedHandlesAndLayers(CString csType);
  void SetLinearObjectHandles(CString csUse, CString csIns);
  BOOL GetLinearPropertyValue(CString csProp, CString& csValue);
  BOOL GetBlockPropertyValue(CString csTag, CString& csValue);
};
