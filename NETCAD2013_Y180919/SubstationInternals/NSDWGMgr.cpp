#include "StdAfx.h"
#include "NSDWGMgr.h"
#include "NSMainSubstaionDlg.h"

CNSDWGMgr::CNSDWGMgr(void)
{
}

CNSDWGMgr::~CNSDWGMgr(void)
{
}

bool CNSDWGMgr::CheckDWTFileStatus()
{
	// This checking criteria is as per existing VB.Net project
	// If some specific criteria is available, it can be implemented

	if (hasBlock(_T("SCHEM_SUB_KIOSK_DIAG")) || hasBlock(_T("SCHEM_SUB_POLE_DIAG")))
		return true;
	else
		return false;

}
void CNSDWGMgr::UpdateDynamicProperties(AcDbBlockReference *pBlkRef,CSSData *m_pSSData,bool ModelSpace)
{
		// Check to make sure it is a dynamic block
				AcDbDynBlockReference dynBlkRef(pBlkRef->id());
				if(dynBlkRef.isDynamicBlock())
				{
				
					AcDbDynBlockReferencePropertyArray propArr;
					dynBlkRef.getBlockProperties(propArr);

          int numprops = propArr.length();

					for(int i = 0; i < propArr.length(); i++)
					{
						AcDbDynBlockReferenceProperty prop = propArr.at(i);
						AcDbEvalVariant val = prop.value();

						AcString prop_description = prop.description();
						AcString prop_Name = prop.propertyName();

                /////////////////////////////////////////////////////////////////////
                // Code changed ~SJ, DCS, 01.07.2013
                // Simplified string compare operator, as the earlier method is throwing an error
                //
								//if((NSSTRCMP(prop_Name.mpwszData, _T("Visibility")) == 0))								
                //
                if (prop_Name == _T("Visibility"))
                // End code changed ~SJ, DCS, 01.07.2013
								{
										CString strSchemVisibility;

										if(ModelSpace)
											m_pSSData->getGeoVisibility(strSchemVisibility);
										else
											m_pSSData->getSchemVisibility(strSchemVisibility);
										

										TCHAR *szVisName(strSchemVisibility.GetBuffer());

										 AcArray<TCHAR *> visNames;
										 visNames.append(szVisName);

										  try
											{
											AcDbEvalVariant visVal(visNames.at(0));
											prop.setValue(visVal);
											}
										  catch (...)
											{												
											}											
								}

								//if((NSSTRCMP(prop_Name.mpwszData, _T("Bus Length")) == 0))								
                if (prop_Name == _T("Bus Length"))
								{

										CString strSchemLookupProp;
										m_pSSData->getSchemLookupProp(strSchemLookupProp);

										TCHAR *szLookUpName(strSchemLookupProp.GetBuffer());

										 AcArray<TCHAR *> LookUpNames;
										 LookUpNames.append(szLookUpName);

										  try
											{
											AcDbEvalVariant LookUpVal(LookUpNames.at(0));
											prop.setValue(LookUpVal);
											}
										  catch (...)
											{												
											}											
								}

								
					}
			}
}

int CNSDWGMgr::getBlock(const TCHAR* p_szBlkname, AcDbObjectId& blockId)
{
	blockId = AcDbObjectId::kNull;

	AcDbBlockTable *pBlockTable = NULL;
	if(Acad::eOk != acdbHostApplicationServices ()->workingDatabase ()->getSymbolTable(pBlockTable, AcDb::kForRead)) //get block table
		return NS_FAIL;
	AcDbBlockTableRecord* pBlkTableRecord = NULL;
	if(Acad::eOk != pBlockTable->getAt(p_szBlkname, pBlkTableRecord, AcDb::kForRead)) // get block table record of the given blockname
	{
		pBlockTable->close();
		return NS_FAIL;
	}

	blockId = pBlkTableRecord->objectId();    // get object Id of the retrieved block table record. It will be returned.
		
	pBlkTableRecord->close();
	pBlockTable->close();
	
	return ((blockId != AcDbObjectId::kNull) ? NS_SUCCESS : NS_FAIL);
}

bool CNSDWGMgr::hasBlock(const TCHAR* p_szBlkname)
{
	AcDbBlockTable *pBlockTable ;
	Acad::ErrorStatus es ;
	if ( (es =acdbHostApplicationServices ()->workingDatabase ()->getBlockTable (pBlockTable, AcDb::kForRead)) != Acad::eOk )
		return false ;

	if ( pBlockTable->has (p_szBlkname) == Adesk::kTrue )
	{
		pBlockTable->close () ;
		return true ;
	}

	return false ;
}

bool CNSDWGMgr::SubStnBlokEdit(AcDbEntity *pEnt)
{
		MYSTRINGSTRINGMAP mapOfAttributesFromBlock;
		NSSTRING szName;
		AcDbHandle handle;
		//ads_name en ;
		//ads_point pt ;

		//if ( acedEntSel (NULL, en, pt) != RTNORM )
		//	return false;

		//AcDbObjectId objID;
		//acdbGetObjectId(objID,en );

		//AcDbEntity *pEnt;        
		//acdbOpenAcDbEntity(pEnt,objID,AcDb::kForRead);
		
				if (pEnt->isKindOf(AcDbBlockReference::desc()))
				{
						AcDbBlockReference *pBlkRef;
						pBlkRef = AcDbBlockReference::cast(pEnt);

						AcDbObjectId blkID = pBlkRef->blockTableRecord();

									AcDbObject *pObj;
									Acad::ErrorStatus es = acdbOpenAcDbObject(pObj,blkID,AcDb::kForRead);
									if(pObj)
									{
										AcDbBlockTableRecord *pBTRecod = (AcDbBlockTableRecord*)pObj;

										getEffectiveName(pBTRecod,pBlkRef,szName);
										//acutPrintf(_T("\n%s"),szName.c_str());
									}
									pObj->close();

						

						AcDbObjectId blockDefId = NULL;
						if(0 != getBlock(szName.c_str(), blockDefId))
						{
							acutPrintf(_T("%s block not present. Template may not be correct."),szName);
							return false;
						}

						

						getAttributeInfoFromWithInBlock(pBlkRef,mapOfAttributesFromBlock);	
						//getTableInfoFromWithInBlock(pBlkRef);
						
											
						//pt = pBlkRef->position();
						pBlkRef->close();

						//AcDbBlockTableRecord *pBlockDef ; //= pBlkRef->blockTableRecord();
						//acdbOpenObject(pBlockDef, blockDefId, AcDb::kForRead);

						

				}	
				
				
				pEnt->getAcDbHandle(handle);

				pEnt->close();

				// Invoke Main UI in Edit mode
				CAcModuleResourceOverride myResources;
				CNSMainSubstaionDlg* objDlg = new CNSMainSubstaionDlg(mapOfAttributesFromBlock,handle,acedGetAcadFrame());
				if(objDlg!=NULL)
				{
					objDlg->DoModal();
					delete objDlg;
				}
}

void CNSDWGMgr::getEffectiveName( AcDbBlockTableRecord *blkTblRecord, AcDbBlockReference *pBlockRef,   NSSTRING &strTempBlkName/*, TCHAR* pszInsertedBlockName*/)
{
	 
		TCHAR *pName = NULL;
		//if(blkTblRecord->isAnonymous())
		//{
			AcDbDynBlockReference* pDynObj = new AcDbDynBlockReference(pBlockRef);
			if(pDynObj)
			{
				AcDbObjectId objDynBlkId = pDynObj->dynamicBlockTableRecord();
				if(!objDynBlkId.isNull())
				{
					AcDbObject* pObj;
					Acad::ErrorStatus er = acdbOpenAcDbObject(pObj,objDynBlkId,AcDb::kForRead);
					if(pObj)
					{
						AcDbBlockTableRecord* pBTRecord = (AcDbBlockTableRecord*)pObj;
						if(pBTRecord)
						{							
							pBTRecord->getName(pName);
							strTempBlkName = pName;

							acdbFree(pName);
						}
						pBTRecord->close();
					}
					pObj->close();
				}
			}			
			delete pDynObj;
		//}
	
}

void CNSDWGMgr::getTableInfoFromWithInBlock(AcDbBlockReference *pBlkRef )
{

	AcDbObjectId blkID = pBlkRef->blockTableRecord();

	AcDbBlockTableRecord *pBlockDef ; 
	acdbOpenObject(pBlockDef, blkID, AcDb::kForRead);

		AcDbBlockTableRecordIterator *pIterator;
		pBlockDef->newIterator(pIterator);
        
				for (pIterator->start(); !pIterator->done();  pIterator->step())
				{
					AcDbEntity *pEnt;
					pIterator->getEntity(pEnt, AcDb::kForWrite);

							if (pEnt->isKindOf(AcDbTable::desc()))
							{
								AcDbTable *pTable;
								pTable = AcDbTable::cast(pEnt);			

								int nLV_Out = 3;
								for(int row = 1 ; row <= nLV_Out ; row++)
								{
									CString st1,st2 ;
									st1 = pTable->textString(row,1);
									st2 = pTable->textString(row,2);

									//acutPrintf(_T("\n%s %s"),pTable->textString(row,1),pTable->textString(row,2));
								}

								//pTable->deleteRows(nLV_Out+1,5-nLV_Out);

								pTable->close();
							}
					pEnt->close();

				}

		delete pIterator;

	pBlockDef->close();


}
int CNSDWGMgr::getAttributeInfoFromWithInBlock(AcDbBlockReference *pBlkRef,MYSTRINGSTRINGMAP &mapOfAttributesFromBlock)//pBlkRef
{
			AcDbObjectIterator* pIter = pBlkRef->attributeIterator();			

			for(pIter->start(); !pIter->done(); pIter->step())
			{
				AcDbObjectId AttrId = pIter->objectId();

				AcDbAttribute *pAtt;

						if (Acad::eOk != pBlkRef->openAttribute(pAtt, AttrId, AcDb::kForRead))
						{
							pBlkRef->close();
							return NS_FAIL;
						}

						const TCHAR* pszAttName = pAtt->tag();
						NSSTRING szAttName = pszAttName;
						NSSTRING szAttValue = pAtt->textString();

						
						mapOfAttributesFromBlock.insert( MYSTRINGSTRINGMAP::value_type(szAttName,szAttValue));

						//acutPrintf(_T("\n%s"),szAttName.c_str()); 
						//acutPrintf(_T(" : %s"),szAttValue.c_str());							

				pAtt->close();

			}
			delete pIter;

}

void CNSDWGMgr::setTableInfoWithInBlock(AcDbBlockReference *pBlkRef,CSSData *m_pSSData )
{

	AcDbObjectId blkID = pBlkRef->blockTableRecord();

	AcDbBlockTableRecord *pBlockDef ; 
	acdbOpenObject(pBlockDef, blkID, AcDb::kForRead);

		AcDbBlockTableRecordIterator *pIterator;
		pBlockDef->newIterator(pIterator);
        
				for (pIterator->start(); !pIterator->done();  pIterator->step())
				{
					AcDbEntity *pEnt;
					pIterator->getEntity(pEnt, AcDb::kForWrite);

							if(pEnt)
							{
														if (pEnt->isKindOf(AcDbTable::desc()))
														{
															AcDbTable *pTable;
															pTable = AcDbTable::cast(pEnt);			

															CString strDisName;			

															int nLV_Out = m_pSSData->getLVCNoOfDistributors(); 

															for(int row = 1 ; row <= nLV_Out ; row++)
															{				
																m_pSSData->getLVCDistributorName(row-1,strDisName);
																pTable->setTextString(row,1,strDisName);

																m_pSSData->getLVCDistributorName(row-1,strDisName);
																pTable->setTextString(row,2,strDisName);
															}

															pTable->deleteRows(nLV_Out+1,5-nLV_Out);
														}
							pEnt->close();
							}

					

				}

		delete pIterator;

	pBlockDef->close();


}

AcDbObjectId CNSDWGMgr::InsertSubstationBlock(const TCHAR* p_szBlkname,AcGePoint3d basePoint,CSSData *m_pSSData,MYSTRINGSTRINGMAP mapOfAttributes,bool ModelSpace)
{
	Acad::ErrorStatus es;

		AcDbObjectId blockDefId = NULL;
		if(0 != getBlock(p_szBlkname, blockDefId))
		{
			acutPrintf(_T("\n%s block not present. Template may not be correct."),p_szBlkname);
			return NULL;
		}//error handeling required

    AcDbBlockReference *pBlkRef = new AcDbBlockReference;

    pBlkRef->setBlockTableRecord(blockDefId);

    resbuf to, from;

    from.restype = RTSHORT;
    from.resval.rint = 1; // UCS
    to.restype = RTSHORT;
    to.resval.rint = 0; // WCS

    AcGeVector3d normal(0.0, 0.0, 1.0);
    acedTrans(&(normal.x), &from, &to, Adesk::kTrue,&(normal.x));


    pBlkRef->setPosition(basePoint);


    pBlkRef->setRotation(0.0);
    pBlkRef->setNormal(normal);


    AcDbBlockTable *pBlockTable;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

    AcDbBlockTableRecord *pBlockTableRecord;

	if(ModelSpace)
	{
		pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,AcDb::kForWrite);
	}
	else
	{
		pBlockTable->getAt(ACDB_PAPER_SPACE, pBlockTableRecord,AcDb::kForWrite);
	}


    pBlockTable->close();

    AcDbObjectId newBlkRefId;
    pBlockTableRecord->appendAcDbEntity(newBlkRefId, pBlkRef);
    pBlockTableRecord->close();

	AcDbBlockTableRecord *pBlockDef;
    acdbOpenObject(pBlockDef, blockDefId, AcDb::kForRead);

    AcDbBlockTableRecordIterator *pIterator;
    pBlockDef->newIterator(pIterator);

    for (pIterator->start(); !pIterator->done();  pIterator->step())
    {

		AcDbEntity *pEnt;
        pIterator->getEntity(pEnt, AcDb::kForWrite);

		
		if (pEnt->isKindOf(AcDbAttributeDefinition::desc()))
		{
				AcDbAttributeDefinition *pAttdef;
				pAttdef = AcDbAttributeDefinition::cast(pEnt);

				if (pAttdef != NULL && !pAttdef->isConstant()) 
				{

					// We have a non-constant attribute definition,
					// so build an attribute entity.
					//
					AcDbAttribute *pAtt = new AcDbAttribute();
					pAtt->setPropertiesFrom(pAttdef);
					pAtt->setInvisible(pAttdef->isInvisible());

					bool bIsTheAttributeMultiLine = pAttdef->isMTextAttributeDefinition();//isMTextAttribute

					

					// Translate the attribute by block reference.
					// To be really correct, the entire block
					// reference transform should be applied here.
					//
					///////////////////////////////////////////////////////////////////////////
					/*AcGePoint3d alignPt;
					alignPt.x = pAttdef->alignmentPoint().x + basePoint.x;
					alignPt.y = pAttdef->alignmentPoint().y + basePoint.y;
					alignPt.z = pAttdef->alignmentPoint().z + basePoint.z;						
					pAtt->setAlignmentPoint(alignPt);*/

					AcDb::TextHorzMode h;
					AcDb::TextVertMode v;

					h = pAttdef->horizontalMode();
					v = pAttdef->verticalMode();

					pAtt->setHorizontalMode(h);
					pAtt->setVerticalMode(v);

					pAtt->adjustAlignment(acdbHostApplicationServices()->workingDatabase());

					AcGePoint3d AttPt;
					AttPt.x = pAttdef->position().x + basePoint.x;
					AttPt.y = pAttdef->position().y + basePoint.y;
					AttPt.z = pAttdef->position().z + basePoint.z;					
					
					pAtt->setAlignmentPoint(AttPt); 
					pAtt->setPosition(AttPt);
					///////////////////////////////////////////////////////////////////////////
			
					pAtt->setHeight(pAttdef->height());
					pAtt->setRotation(pAttdef->rotation());

					const TCHAR *pStr = pAttdef->tagConst();
					pAtt->setTag(pStr);
					
					pAtt->setFieldLength(pAttdef->fieldLength());					

					if(bIsTheAttributeMultiLine)
					{
						es = pAtt->convertIntoMTextAttribute();
						es = pAtt->updateMTextAttribute();						
					}

					pAtt->setFieldLength(pAttdef->fieldLength());

					// The database column value should be displayed.
					// INSERT prompts for this.
					//
					pAtt->setTextString(_T(" "));

					   MYSTRINGSTRINGMAP::iterator m_AttMapIterator;
					   m_AttMapIterator = mapOfAttributes.find(pStr);
					   if(m_AttMapIterator != mapOfAttributes.end())
					   {
						   NSSTRING tagname =  (*m_AttMapIterator).first;
						   NSSTRING attVal =  (*m_AttMapIterator).second;

						   pAtt->setTextString(attVal.c_str());
					   }
					   else
					   {
						   
					   }
					
					AcDbObjectId attId;

					pBlkRef->appendAttribute(attId, pAtt);


					AcDbObjectIdArray ObjIDs;
					ObjIDs.append(pAtt->objectId());

					

					pAtt->close();					

					acdbForceTextAdjust(ObjIDs);

				}
				pAttdef->close();
		}//pEnt->isKindOf(AcDbAttributeDefinition::desc()
		
        pEnt->close(); // use pEnt... pAttdef might be NULL

    }//pIterator->start(); !pIterator->done();  pIterator->step()

    delete pIterator;
    pBlockDef->close();

	ads_name Blok;
	acdbGetAdsName(Blok,newBlkRefId);
	acdbEntUpd(Blok);

	
    pBlkRef->close();
	UpdateDynamicProperties(pBlkRef,m_pSSData,ModelSpace);

	

	acedRedraw(Blok,1);
	acedUpdateDisplay();
	//acedRedraw();
	//acedUpdate(0,pt1,pt2);
	//acedDisplay();


return newBlkRefId;
	
}

void CNSDWGMgr::InsertDistributorBlock(const TCHAR* p_szBlkname,TCHAR* p_szVisName,int LV_SEQUENCE,AcGePoint3d basePoint,CString &DistHandle)
{
	AcDbHandle handle;

		AcDbObjectId blockDefId = NULL;
		if(0 != getBlock(p_szBlkname, blockDefId))
		{
			acutPrintf(_T("%s block not present. Template may not be correct."),p_szBlkname);
			return ;
		}//error handeling required

    AcDbBlockReference *pBlkRef = new AcDbBlockReference;


    pBlkRef->setBlockTableRecord(blockDefId);


    resbuf to, from;

    from.restype = RTSHORT;
    from.resval.rint = 1; // UCS
    to.restype = RTSHORT;
    to.resval.rint = 0; // WCS

    AcGeVector3d normal(0.0, 0.0, 1.0);
    acedTrans(&(normal.x), &from, &to, Adesk::kTrue,&(normal.x));


    pBlkRef->setPosition(basePoint);


    pBlkRef->setRotation(0.0);
    pBlkRef->setNormal(normal);


    AcDbBlockTable *pBlockTable;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

    AcDbBlockTableRecord *pBlockTableRecord;

	//if(ModelSpace)
	//{
	//	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,AcDb::kForWrite);
	//}
	//else
	//{
		pBlockTable->getAt(ACDB_PAPER_SPACE, pBlockTableRecord,AcDb::kForWrite);//Distributor is always placed with Schematics in paperspace
	//}


    pBlockTable->close();

    AcDbObjectId newBlkRefId;
    pBlockTableRecord->appendAcDbEntity(newBlkRefId, pBlkRef);
    pBlockTableRecord->close();

	AcDbBlockTableRecord *pBlockDef;
    acdbOpenObject(pBlockDef, blockDefId, AcDb::kForRead);

    AcDbBlockTableRecordIterator *pIterator;
    pBlockDef->newIterator(pIterator);

    for (pIterator->start(); !pIterator->done();  pIterator->step())
    {

		AcDbEntity *pEnt;
        pIterator->getEntity(pEnt, AcDb::kForWrite);

		
		if (pEnt->isKindOf(AcDbAttributeDefinition::desc()))
		{
				AcDbAttributeDefinition *pAttdef;
				pAttdef = AcDbAttributeDefinition::cast(pEnt);

				if (pAttdef != NULL && !pAttdef->isConstant()) 
				{

					// We have a non-constant attribute definition,
					// so build an attribute entity.
					//
					AcDbAttribute *pAtt = new AcDbAttribute();
					pAtt->setPropertiesFrom(pAttdef);
					pAtt->setInvisible(pAttdef->isInvisible());

					// Translate the attribute by block reference.
					// To be really correct, the entire block
					// reference transform should be applied here.
					//
					///////////////////////////////////////////////////////////////////////////
					/*AcGePoint3d alignPt;
					alignPt.x = pAttdef->alignmentPoint().x + basePoint.x;
					alignPt.y = pAttdef->alignmentPoint().y + basePoint.y;
					alignPt.z = pAttdef->alignmentPoint().z + basePoint.z;						
					pAtt->setAlignmentPoint(alignPt);*/

					AcDb::TextHorzMode h;
					AcDb::TextVertMode v;

					h = pAttdef->horizontalMode();
					v = pAttdef->verticalMode();

					pAtt->setHorizontalMode(h);
					pAtt->setVerticalMode(v);

					pAtt->adjustAlignment(acdbHostApplicationServices()->workingDatabase());

					AcGePoint3d AttPt;
					AttPt.x = pAttdef->position().x + basePoint.x;
					AttPt.y = pAttdef->position().y + basePoint.y;
					AttPt.z = pAttdef->position().z + basePoint.z;					
					
					pAtt->setAlignmentPoint(AttPt); 
					pAtt->setPosition(AttPt);
					///////////////////////////////////////////////////////////////////////////
					

					pAtt->setHeight(pAttdef->height());
					pAtt->setRotation(pAttdef->rotation());
					
					pAtt->setFieldLength(pAttdef->fieldLength());	

					const TCHAR *pStr = pAttdef->tagConst();
					pAtt->setTag(pStr);					

					pAtt->setFieldLength(pAttdef->fieldLength());


					CString iLV = _T("");
					iLV.FormatMessage(L"%1!d!",LV_SEQUENCE);


					if((NSSTRCMP(pStr, _T("DISTRIBUTOR_NUMBER")) == 0))	
					{
						pAtt->setTextString(iLV);
					}

					
					AcDbObjectId attId;

					pBlkRef->appendAttribute(attId, pAtt);

					AcDbObjectIdArray ObjIDs;
					ObjIDs.append(pAtt->objectId());
					pAtt->close();					

					acdbForceTextAdjust(ObjIDs);

					
				}

				pAttdef->close();
		}//pEnt->isKindOf(AcDbAttributeDefinition::desc()
		
		
		pBlkRef->getAcDbHandle(handle);
        pEnt->close(); // use pEnt... pAttdef might be NULL

    }//pIterator->start(); !pIterator->done();  pIterator->step()

    delete pIterator;
    pBlockDef->close();

	ads_name Blok;
	acdbGetAdsName(Blok,newBlkRefId);
	acdbEntUpd(Blok);

	
    pBlkRef->close();
	SetVisibilityProperties(pBlkRef,p_szVisName);


	//UpdateDynamicProperties(pBlkRef,m_pSSData);
	//SetVisibilityProperties(pBlkRef,p_szVisName);

	acedRedraw(Blok,1);
	acedUpdateDisplay();
	//acedRedraw();
	//acedUpdate(0,pt1,pt2);
	//acedDisplay();

ACHAR *szhandle = new ACHAR[20];//*subir
//handle.getIntoAsciiBuffer(szhandle);
handle.getIntoAsciiBuffer(szhandle,sizeof(szhandle));
DistHandle = szhandle;
delete szhandle;//*subir

}

void CNSDWGMgr::SetVisibilityProperties(AcDbBlockReference *pBlkRef,TCHAR *szVisName)
{
				AcDbDynBlockReference dynBlkRef(pBlkRef->id());
				if(dynBlkRef.isDynamicBlock())
				{
				
					AcDbDynBlockReferencePropertyArray propArr;
					dynBlkRef.getBlockProperties(propArr);

					for(int i = 0; i < propArr.length(); i++)
					{
						AcDbDynBlockReferenceProperty prop = propArr.at(i);
						AcDbEvalVariant val = prop.value();

						AcString prop_description = prop.description();
						AcString prop_Name = prop.propertyName();

								//Commented for ACAD 2018
								//if((NSSTRCMP(prop_Name.mpwszData, _T("Distributor Type")) == 0))								
								if ((NSSTRCMP(prop.propertyName(), _T("Distributor Type")) == 0))
								{
										 AcArray<TCHAR *> visNames;
										 visNames.append(szVisName);

										  try
											{
												AcDbEvalVariant visVal(visNames.at(0));
												prop.setValue(visVal);
											}
										  catch (...)
											{												
											}											
								}								
					}
			}
}


void CNSDWGMgr::UpdateSubstationBlock(CSSData *m_pSSData,MYSTRINGSTRINGMAP mapOfAttributes,AcDbHandle handle)
{
	ads_name entName;

	ACHAR *szhandle = new ACHAR[20];
	//handle.getIntoAsciiBuffer(szhandle);
	handle.getIntoAsciiBuffer(szhandle,sizeof(szhandle));
	acdbHandEnt(szhandle,entName);
	delete szhandle;//*subir

	AcDbObjectId objID;
	acdbGetObjectId(objID,entName );

		AcDbEntity *pEnt;        
		acdbOpenAcDbEntity(pEnt,objID,AcDb::kForWrite);


AcDbBlockReference *pBlkRef;
AcDbObjectId blockDefId;
if (pEnt->isKindOf(AcDbBlockReference::desc()))
{						
						pBlkRef = AcDbBlockReference::cast(pEnt);
						blockDefId = pBlkRef->blockTableRecord();
}

    //AcDbBlockReference *pBlkRef = new AcDbBlockReference;

    //pBlkRef->setBlockTableRecord(blockDefId);

    //resbuf to, from;

    //from.restype = RTSHORT;
    //from.resval.rint = 1; // UCS
    //to.restype = RTSHORT;
    //to.resval.rint = 0; // WCS

    //AcGeVector3d normal(0.0, 0.0, 1.0);
    //acedTrans(&(normal.x), &from, &to, Adesk::kTrue,&(normal.x));


    //pBlkRef->setPosition(basePoint);


 /*   pBlkRef->setRotation(0.0);
    pBlkRef->setNormal(normal);*/


    //AcDbBlockTable *pBlockTable;
    //acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

    //AcDbBlockTableRecord *pBlockTableRecord;

	/*if(ModelSpace)
	{
		pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,AcDb::kForWrite);
	}
	else
	{
		pBlockTable->getAt(ACDB_PAPER_SPACE, pBlockTableRecord,AcDb::kForWrite);
	}*/


    /*pBlockTable->close();

    AcDbObjectId newBlkRefId;
    pBlockTableRecord->appendAcDbEntity(newBlkRefId, pBlkRef);
    pBlockTableRecord->close();

	setTableInfoWithInBlock(pBlkRef,m_pSSData);*/

    AcDbBlockTableRecord *pBlockDef;
    acdbOpenObject(pBlockDef, blockDefId, AcDb::kForRead);

    AcDbBlockTableRecordIterator *pIterator;
    pBlockDef->newIterator(pIterator);

    for (pIterator->start(); !pIterator->done();  pIterator->step())
    {

		AcDbEntity *pEnt;
        pIterator->getEntity(pEnt, AcDb::kForWrite);

		
		if (pEnt->isKindOf(AcDbAttributeDefinition::desc()))
		{
				AcDbAttributeDefinition *pAttdef;
				pAttdef = AcDbAttributeDefinition::cast(pEnt);

				if (pAttdef != NULL && !pAttdef->isConstant()) 
				{

					// We have a non-constant attribute definition,
					// so build an attribute entity.
					//
					AcDbAttribute *pAtt = new AcDbAttribute();
					pAtt->setPropertiesFrom(pAttdef);
					pAtt->setInvisible(pAttdef->isInvisible());

					// Translate the attribute by block reference.
					// To be really correct, the entire block
					// reference transform should be applied here.
					//
					/*basePoint = pAttdef->position();
					basePoint += pBlkRef->position().asVector();*/
					//pAtt->setPosition(basePoint);

					pAtt->setHeight(pAttdef->height());
					pAtt->setRotation(pAttdef->rotation());

					pAtt->setTag(_T("Tag"));
					pAtt->setFieldLength(25);

					const TCHAR *pStr = pAttdef->tagConst();
					pAtt->setTag(pStr);

					//acutPrintf(_T("\n%s"),pStr);

					pAtt->setFieldLength(pAttdef->fieldLength());

					// The database column value should be displayed.
					// INSERT prompts for this.
					//
					pAtt->setTextString(_T(" "));

					   MYSTRINGSTRINGMAP::iterator m_AttMapIterator;
					   m_AttMapIterator = mapOfAttributes.find(pStr);
					   if(m_AttMapIterator != mapOfAttributes.end())
					   {
						   NSSTRING tagname =  (*m_AttMapIterator).first;
						   NSSTRING attVal =  (*m_AttMapIterator).second;

						   pAtt->setTextString(attVal.c_str());
					   }
					   else
					   {
						   
					   }
					
					AcDbObjectId attId;

					pBlkRef->appendAttribute(attId, pAtt);
					pAtt->close();
				}

		}//pEnt->isKindOf(AcDbAttributeDefinition::desc()

        pEnt->close(); // use pEnt... pAttdef might be NULL

    }//pIterator->start(); !pIterator->done();  pIterator->step()

    delete pIterator;
    pBlockDef->close();

	/*ads_name Blok;
	acdbGetAdsName(Blok,newBlkRefId);
	acdbEntUpd(Blok);*/

	
    pBlkRef->close();
	UpdateDynamicProperties(pBlkRef,m_pSSData,true);

	

//	acedRedraw(Blok,1);
	acedUpdateDisplay();
	//acedRedraw();
	//acedUpdate(0,pt1,pt2);
	//acedDisplay();


	
}

int CNSDWGMgr::addXDATA(AcDbObjectId ObjectId, NSSTRING p_szAppName, NSSTRING p_szAppValue)
{
	Acad::ErrorStatus es = Acad::eOk;
	
	//Open object for write
	AcDbObject *pObj;
	if(Acad::eOk != acdbOpenAcDbObject(pObj, ObjectId, AcDb::kForWrite))	
		return NS_FAIL;

	struct  resbuf  *pRb, *pTemp;    
	pRb = pObj->xData(p_szAppName.c_str());
	if (pRb != NULL) 
	{
		for (pTemp = pRb; pTemp->rbnext != NULL; pTemp = pTemp->rbnext);
	}
	else
	{
		//register application name (XDATA-Key)
		registerApplication(acdbHostApplicationServices()->workingDatabase(), p_szAppName.c_str());

		pRb = acutNewRb(AcDb::kDxfRegAppName);
		pTemp = pRb;

		pTemp->resval.rstring =  (ACHAR*) new ACHAR[(NSSTRLEN(p_szAppName.c_str())+1)];
		#ifdef _VS_2002
			NSSTRCPY(pTemp->resval.rstring,(ACHAR*)p_szAppName.c_str());
		#else
			NSSTRCPY(pTemp->resval.rstring,NSSTRLEN(p_szAppName.c_str())+1, (ACHAR*)p_szAppName.c_str());
		#endif
	}

	pTemp->rbnext = acutNewRb(AcDb::kDxfXdAsciiString);
	pTemp = pTemp->rbnext;

	pTemp->resval.rstring =  (ACHAR*) new ACHAR[(NSSTRLEN(p_szAppValue.c_str())+1)];

	#ifdef _VS_2002
		NSSTRCPY(pTemp->resval.rstring,(ACHAR*)p_szAppValue.c_str());
	#else
		NSSTRCPY(pTemp->resval.rstring,NSSTRLEN(p_szAppValue.c_str())+1, (ACHAR*)p_szAppValue.c_str());
	#endif

	es = pObj->setXData(pRb);

	pObj->close();
	//acutRelRb(pTemp);
#ifndef _DEBUG
	acutRelRb(pRb);
#endif

	return ((es == Acad::eOk) ? NS_SUCCESS : NS_FAIL);
}

void CNSDWGMgr::registerApplication(AcDbDatabase* pDatabase, NSSTRING strAppName)
{
    AcDbRegAppTable *pRegAppTable; 
    AcDbObjectId blockId;
    if (pDatabase->getRegAppTable(pRegAppTable, AcDb::kForWrite) == Acad::eOk)
    {
        AcDbRegAppTableRecord *pRecord = new AcDbRegAppTableRecord; 
        if (pRecord)
        {
			pRecord->setName(strAppName.c_str());
            if (pRegAppTable->add(blockId, pRecord) == Acad::eOk)
				pRecord->close();
            else 
				delete pRecord;
        }         
		pRegAppTable->close();
		//pRecord->close();
    }
}






void CNSDWGMgr::PunchNSStampToLastEntityCreated()
{
	ads_name sset;
	const ACHAR *str = _T("L");
	int err = acedSSGet(str, NULL, NULL, NULL, sset);
	if (err != RTNORM) {return;}

	//long i, length;
	long i;
	int length;
	ads_name ename;
	AcDbObjectId entId;
	acedSSLength(sset, &length);
	for (i = 0; i < length; i++) 
	{
		acedSSName(sset, i, ename);
		acdbGetObjectId(entId, ename);		
		addXDATA(entId,_T("IsSubstation"),_T("NS::Okey"));
	}		
	acedSSFree(sset);
}

int CNSDWGMgr::fillXDATAMAP(AcDbObjectId ObjectId, std::map<NSSTRING, NSSTRING> &mapXData)
{
	//Open object for write
	AcDbObject *pObj;
	if(Acad::eOk != acdbOpenAcDbObject(pObj, ObjectId, AcDb::kForRead))	
		return NS_FAIL;
	
	struct  resbuf  *pRb, *pTemp; 
	pRb = pObj->xData(NULL);


	// If xdata is present, then retrieve all key-value pairs
	for (pTemp = pRb; pTemp != NULL;)
	{
		if(pTemp->restype == AcDb::kDxfRegAppName)
		{
			NSSTRING strAppName = pTemp->resval.rstring;
			pTemp = pTemp->rbnext;

			////////////////// NEW CODE TO INCORPORATE MANUAL XDATA ENTRY
			if(pTemp->restype == AcDb::kDxfXdControlString)
			{
				pTemp = pTemp->rbnext;
			}
			////////////////// NEW CODE TO INCORPORATE MANUAL XDATA ENTRY

			if(pTemp!= NULL && pTemp->restype == AcDb::kDxfXdAsciiString)
			{
				NSSTRING strAppValue = pTemp->resval.rstring;
				mapXData.insert(std::map<NSSTRING, NSSTRING>::value_type(strAppName, strAppValue));
				pTemp = pTemp->rbnext;
			}
			//else
			//{
			//	pTemp = pTemp->rbnext;
			//}
		}
		else
		{
			pTemp = pTemp->rbnext;
		}
	}
	pObj->close();
	return NS_SUCCESS;
}

void CNSDWGMgr::PostXDATAfromUI(AcDbObjectId ObjectId,MYSTRINGSTRINGMAP mapOfAttributes)
{
	std::map<NSSTRING,NSSTRING>::iterator itr;
	for(itr = mapOfAttributes.begin(); itr != mapOfAttributes.end(); itr++)
	{
		NSSTRING strKey = (*itr).first;
		NSSTRING strValue = (*itr).second;

		addXDATA(ObjectId,strKey,strValue);

		//int nIndex = (int)strKey.find(szSearchString);
		//if(nIndex != -1 && nIndex == 0)
		//{
		//	//m_mapofURL.insert(std::map<NSSTRING,NSSTRING>::value_type(strKey, strValue));
		//}
	}

}

AcDbObjectId CNSDWGMgr::createGeoGraphicTable(AcGePoint3d insPt,CSSData *m_pSSData)//,LVDistdata lvData
{
		AcDbTable *pTable = new AcDbTable();
		pTable->setSize(6,3);	
		pTable->setPosition(insPt);
		pTable->setWidth(150);

		AcDbBlockTable *pBlockTable;
		acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

		AcDbBlockTableRecord *pBlockTableRecord;
		pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,AcDb::kForWrite);
		pBlockTable->close();

		AcDbObjectId tableID;
		pBlockTableRecord->appendAcDbEntity(tableID, pTable);

		pBlockTableRecord->close();
	    

		Acad::ErrorStatus es ;
		es = pTable->setTextString(0,0,_T("No"));
		es = pTable->setTextString(0,1,_T("LV Circuit"));
		es = pTable->setTextString(0,2,_T("Distributors"));

		es = pTable->setTextString(1,0,_T("1"));
		es = pTable->setTextString(2,0,_T("2"));
		es = pTable->setTextString(3,0,_T("3"));
		es = pTable->setTextString(4,0,_T("4"));
		es = pTable->setTextString(5,0,_T("5"));

		int nLV_Out = m_pSSData->getLVCNoOfDistributors(); 

		CString strVal;		
		for(int row = 1 ; row <= nLV_Out ; row++)
		{				
			m_pSSData->getLVCDistCaption(row-1,strVal);
			pTable->setTextString(row,1,strVal);

			m_pSSData->getLVCDistributorName(row-1,strVal);
			pTable->setTextString(row,2,strVal);
		}

		pTable->deleteRows(nLV_Out+1,5-nLV_Out);

		pTable->close();

		return tableID;
}




AcDbObjectId CNSDWGMgr::createSchematicTable(AcGePoint3d insPt,CSSData *m_pSSData,CString &DistHandle)//,LVDistdata lvData
{
		AcDbTable *pTable = new AcDbTable();
		pTable->setSize(6,5);	
		pTable->setPosition(insPt);
		pTable->setWidth(250);

		AcDbBlockTable *pBlockTable;
		acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

		AcDbBlockTableRecord *pBlockTableRecord;
		pBlockTable->getAt(ACDB_PAPER_SPACE, pBlockTableRecord,AcDb::kForWrite);
		pBlockTable->close();

		AcDbObjectId tableID;
		pBlockTableRecord->appendAcDbEntity(tableID, pTable);

		pBlockTableRecord->close();
	    

		Acad::ErrorStatus es ;
		es = pTable->setTextString(0,0,_T("No."));
		es = pTable->setTextString(0,1,_T("Distributor Name"));
		es = pTable->setTextString(0,2,_T("Panel(A )"));
		es = pTable->setTextString(0,3,_T("Fuse(A )"));
		es = pTable->setTextString(0,4,_T("Fuse/Arm Type (A )"));

		int nLV_Out = m_pSSData->getLVCNoOfDistributors(); 

		CString strVal;		
		for(int row = 1 ; row <= nLV_Out ; row++)
		{				
			m_pSSData->getLVCDistCaption(row-1,strVal);
			pTable->setTextString(row,0,strVal);

			m_pSSData->getLVCDistributorName(row-1,strVal);
			pTable->setTextString(row,1,strVal);

			int iLVCPanelRating = m_pSSData->getLVCPanelRating(row-1);
			if(iLVCPanelRating != -1)
			{
				strVal.FormatMessage(L"%1!d!",iLVCPanelRating);
				pTable->setTextString(row,2,strVal);
			}

			int iLVCFuseA = m_pSSData->getLVCFuseA(row-1);
			if(iLVCFuseA != -1)
			{
				strVal.FormatMessage(L"%1!d!",iLVCFuseA);
				pTable->setTextString(row,3,strVal);
			}

			m_pSSData->getLVCFuse(row-1,strVal);
			pTable->setTextString(row,4,strVal);
		}

		pTable->deleteRows(nLV_Out+1,5-nLV_Out);

		pTable->close();

AcDbHandle handle;
pTable->getAcDbHandle(handle);
ACHAR *szhandle = new ACHAR[20];//*subir
//handle.getIntoAsciiBuffer(szhandle);
handle.getIntoAsciiBuffer(szhandle,sizeof(szhandle));
DistHandle = szhandle;
delete szhandle;//*subir

		return tableID;
}

