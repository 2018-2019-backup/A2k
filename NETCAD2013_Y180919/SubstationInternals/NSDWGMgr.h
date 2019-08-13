#pragma once
#include "SubstationData.h"

class CNSDWGMgr
{
public:
	CNSDWGMgr(void);
	virtual ~CNSDWGMgr(void);

	bool CheckDWTFileStatus();
	void UpdateSubstationBlock(CSSData *m_pSSData,MYSTRINGSTRINGMAP mapOfAttributes,AcDbHandle handle);
	AcDbObjectId InsertSubstationBlock(const TCHAR* p_szBlkname,AcGePoint3d basePoint,CSSData *m_pSSData,MYSTRINGSTRINGMAP mapOfAttributes,bool ModelSpace);
	void InsertDistributorBlock(const TCHAR* p_szBlkname, TCHAR* p_szVisName,int LV_SEQUENCE, AcGePoint3d basePoint,CString &DistHandle);
	void UpdateDynamicProperties(AcDbBlockReference *pBlkRef,CSSData *m_pSSData,bool ModelSpace);
	void SetVisibilityProperties(AcDbBlockReference *pBlkRef,TCHAR *szVisName);
	int  getBlock(const TCHAR* p_szBlkname, AcDbObjectId& blockId);
	bool hasBlock(const TCHAR* p_szBlkname);
	bool SubStnBlokEdit(AcDbEntity *pEnt);
	void getEffectiveName( AcDbBlockTableRecord *blkTblRecord, AcDbBlockReference *pBlockRef, NSSTRING &strTempBlkName/*, TCHAR* pszInsertedBlockName*/);
	void getTableInfoFromWithInBlock(AcDbBlockReference *pBlkRef);
	int getAttributeInfoFromWithInBlock(AcDbBlockReference *pBlkRef,MYSTRINGSTRINGMAP &mapOfAttributesFromBlock);
	void setTableInfoWithInBlock(AcDbBlockReference *pBlkRef,CSSData *m_pSSData);
	int addXDATA(AcDbObjectId ObjectId, NSSTRING p_szAppName, NSSTRING p_szAppValue);
	void registerApplication(AcDbDatabase* pDatabase, NSSTRING strAppName);
	void PunchNSStampToLastEntityCreated();
	int fillXDATAMAP(AcDbObjectId ObjectId, std::map<NSSTRING, NSSTRING> &mapXData);

	void PostXDATAfromUI(AcDbObjectId ObjectId,MYSTRINGSTRINGMAP mapOfAttributes);

	AcDbObjectId createGeoGraphicTable(AcGePoint3d insPt,CSSData *m_pSSData);
	AcDbObjectId createSchematicTable(AcGePoint3d insPt,CSSData *m_pSSData,CString &DistHandle);

};
