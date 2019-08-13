////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : Validate.cpp
// Created          : 23rd January 2008
// Created by       : S. Jaisimha
// Description      : Validates the drawing for conformity with standards
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "ValidateOptionsDlg.h"
#include "ValidateResultsDlg.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defined in DXFin.cpp
extern void SnapLineEndPoints(); // JoinSegments.cpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Validation results modeless dialog global pointer
int g_iErrorNo = 0;
CString g_csError  = _T("VALIDATION_ERROR");
CString g_csAccept = _T("VALIDATION_ERROR_ACCEPT");
CValidateResultsDlg *g_pdlgVR = NULL;

CStringArray csaTempX;
CStringArray g_csaAccErrNos, g_csaAccErrTypes, g_csaAccErrHandles, g_csaAccErrLayers, g_csaAccErrReasons;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : GetLineTypeName
// Arguments    : Object id of line type, as AcDbObjectId
// Called from  : ValidateLayerSettings
// Description  : Returns the name of the linetype
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString GetLineTypeName(AcDbObjectId objLType)
{
  CString csLTName;
  AcDbLinetypeTableRecord *pLTRec; if (acdbOpenObject(pLTRec, objLType, AcDb::kForRead) != Acad::eOk) return csLTName;
  ACHAR *pLTName; if (pLTRec->getName(pLTName) != Acad::eOk) { pLTRec->close(); return csLTName; }
  pLTRec->close();
  csLTName = pLTName;
  return csLTName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : InsertErrorBlocks
// Arguments    : 1. Array of points, as reference to AcGePoint3dArray
//                2. Array of handles, as reference to CStringArray
//                3. Array of error numbers, as reference to CStringArray
//                4. Array of entity handles, as reference to CStringArray
//
// Called from  : The "Validate" functions
// Description  : Inserts the error blocks on the error layer for each point in the array and fills their handles in the other array
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InsertErrorBlocks(AcGePoint3dArray& geaErrors, CStringArray& csaHandles, CStringArray& csaErrorNos, CStringArray& csaErrorEntHandles)
{
  BOOL bLocOK = FALSE;
  ads_name ssExist; 
  struct resbuf *rbpFilter = NULL;
  CString csErrorNo;
  ads_name enEnt;
  AcGePoint3d gePos;
  struct resbuf *rbpEnt = NULL;

  // For each error in the array
  for (int iCtr = 0; iCtr < geaErrors.length(); iCtr++, g_iErrorNo++)
  {
    // Get its position
    gePos = geaErrors.at(iCtr);

    // Ensure that no other error block is in the same position
    bLocOK = FALSE;
    while (bLocOK == FALSE)
    {
      rbpFilter = acutBuildList(RTDXF0, _T("INSERT"), -4, _T("<OR"), 2, g_csError, 2, g_csAccept, -4, _T("OR>"), 8, g_csError, 10, asDblArray(gePos), NULL);
      if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssExist) == RTNORM) gePos.x += 2.0; // If there is one move the location 2 mm to the right
      else bLocOK = TRUE;
      acedSSFree(ssExist);
    }

    // Insert the error block and move it to the ERROR layer
    csErrorNo.Format(_T("%d"), g_iErrorNo);
    acedCommandS(RTSTR, _T(".INSERT"), RTSTR, g_csError, RTPOINT, asDblArray(gePos), RTREAL, 1.0, RTREAL, 1.0, RTREAL, 0.0, RTSTR, csErrorNo, NULL);
    acdbEntLast(enEnt);
    acedCommandS(RTSTR, _T(".CHPROP"), RTENAME, enEnt, RTSTR, _T(""), RTSTR, _T("LA"), RTSTR, g_csError, RTSTR, _T(""), NULL);

    // Get its handle into the array
    rbpEnt = acdbEntGet(enEnt);
    csaHandles.Add(Assoc(rbpEnt, 5)->resval.rstring);
    csaErrorNos.Add(csErrorNo);
    acutRelRb(rbpEnt);

    // If the parent handle is valid
    if (iCtr < csaErrorEntHandles.GetSize())
    {
      // Attach that to the error block as XData
      rbpEnt = acutBuildList(AcDb::kDxfRegAppName, _T("Parent"), AcDb::kDxfXdHandle, csaErrorEntHandles.GetAt(iCtr), NULL);
      addXDataToEntity(enEnt, rbpEnt);
      acutRelRb(rbpEnt);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : GetEntityPosition
// Arguments    : Entity name, as ads_name
// Called from  : The "Validate" functions
// Description  : Retrieves the "base" position of the entity, depending on the entity type
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AcGePoint3d GetEntityPosition(ads_name enEnt)
{
  AcDbEntity *pEnt = NULL;
  AcDbArc *pArc;
  AcDbText *pText;
  AcDbLine *pLine;
  AcDbMline *pMLine;
  AcDbCircle *pCircle;
  AcDbHandle dbHandle;
  AcDbPolyline *pPline;
  AcDb2dPolyline *pPline2d;
  AcDbBlockReference *pRef;
  AcGePoint3d geLoc;
  AcDbObjectId objId;

  // Open the object for reading
  if (acdbGetObjectId(objId, enEnt) != Acad::eOk) return geLoc;
  if (acdbOpenObject(pEnt, objId, AcDb::kForRead) != Acad::eOk) return geLoc;

  // Get its position
  /**/ if (pArc     = AcDbArc::cast(pEnt))            geLoc = pArc->center();
  else if (pCircle  = AcDbCircle::cast(pEnt))         geLoc = pCircle->center();
  else if (pText    = AcDbText::cast(pEnt))           geLoc = pText->position();
  else if (pLine    = AcDbLine::cast(pEnt))           geLoc = pLine->startPoint();
  else if (pRef     = AcDbBlockReference::cast(pEnt)) geLoc = pRef->position();
  else if (pMLine   = AcDbMline::cast(pEnt))          geLoc = pMLine->vertexAt(0);
  else if (pPline   = AcDbPolyline::cast(pEnt))       pPline->getPointAt(0, geLoc);
  else if (pPline2d = AcDb2dPolyline::cast(pEnt))
  {
    int iNumVerts = 0;
    AcDb2dVertex *pVert;
    AcGePoint3d geVert, geVert1, geVert2;
    AcDbObjectIterator *pIter = pPline2d->vertexIterator();
    if (acdbOpenObject(pVert, pIter->objectId(), AcDb::kForRead) == Acad::eOk)
    {
      geLoc = pVert->position();
      pVert->close();
    }
    delete pIter;
  }

  // Close the entity
  pEnt->close();

  // Return the stored position
  return geLoc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ValidateAndSynchronizeAttributes
// Called from  : Command_Validate
// Description  : Checks every block defined in the database for attribute to xdata link. /* Also checks for mandatory attributes. */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ValidateAndSynchronizeAttributes(CStringArray& csaMAHandles, CStringArray& csaMANames, CStringArray& csaMAAttribs, CStringArray& csaMALayers, CStringArray& csaMAErrorNos)
{
  // Get the information from the database
  CString csSQL;
  CQueryTbl tblBlocks;
  CStringArray *pcsaData = NULL;
  csSQL.Format(_T("SELECT [Block], [Tag], [Desc], [Mandatory] FROM tblBlockProps ORDER BY [Block], [Sequence], [Tag]"));
  if (!tblBlocks.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, _T("SynchronizeAttributes"),true)) return FALSE;
  if (tblBlocks.GetRows() <= 0) return TRUE;
  CStringArray csaBlocks; tblBlocks.GetColumnAt(0, csaBlocks);
  CStringArray csaTags;   tblBlocks.GetColumnAt(1, csaTags);
  CStringArray csaDescs;  tblBlocks.GetColumnAt(2, csaDescs);
  CStringArray csaMands;  tblBlocks.GetColumnAt(3, csaMands);

  // Collect unique block names (not using a DISTINCT query to avoid two trips to the database)
  CString csBlock;
  CStringArray csaUniqueBlocks;
  for (int iCtr = 0; iCtr < csaBlocks.GetSize(); iCtr++)
  {
    if (csBlock != csaBlocks.GetAt(iCtr))
    {
      csBlock = csaBlocks.GetAt(iCtr);
      csaUniqueBlocks.Add(csBlock);
    }
  }

  //long lBlocks = 0L;
  int lBlocks = 0L;
  CString csTag, csValue, csUnfilled;
  ads_name ssBlocks, enRef;
  AcGePoint3d gePos;
  AcDbObjectId objRef, objAtt;
  AcDbAttribute *pAtt;
  AcDbBlockReference *pRef;
  AcDbObjectIterator *pIter;
  AcGePoint3dArray geaErrors;
  struct resbuf *rbpXData;

  // For each unique block in the array
  for (int iCtr = 0; iCtr < csaUniqueBlocks.GetSize(); iCtr++)
  {
    // Make a selection set
    csBlock = csaUniqueBlocks.GetAt(iCtr);
    if (acedSSGet(_T("X"), NULL, NULL, acutBuildList(2, csBlock, NULL), ssBlocks) != RTNORM) continue;
    if ((acedSSLength(ssBlocks, &lBlocks) != RTNORM) || (lBlocks == 0L)) { acedSSFree(ssBlocks); continue; }

    // For each insertion
    for (long lCtr = 0L; lCtr < lBlocks; lCtr++)
    {
      // Get the block reference's attribute iterator
      if (acedSSName(ssBlocks, lCtr, enRef) != RTNORM) continue;
      if (acdbGetObjectId(objRef, enRef) != Acad::eOk) continue;
      if (acdbOpenObject(pRef, objRef, AcDb::kForRead) != Acad::eOk) continue;
      gePos = pRef->position();
      pIter = pRef->attributeIterator();
      pRef->close();

      // Loop through the attributes
      csUnfilled.Empty();
      for (pIter->start(); !pIter->done(); pIter->step())
      {
        // Get the attribute tag and value
        if (acdbOpenObject(pAtt, pIter->objectId(), AcDb::kForRead) != Acad::eOk) continue;
        csTag = pAtt->tag();
        csValue = pAtt->textString();
        pAtt->close();

        // Remove leading and trailing spaces from the value
        csValue.TrimLeft();
        csValue.TrimRight();

        // Loop through the arrays retrieved from the database
        for (int iBlock = 0; iBlock < csaBlocks.GetSize(); iBlock++)
        {
          // If the block name and tag matches
          if ((csBlock == csaBlocks.GetAt(iBlock)) && (csTag == csaTags.GetAt(iBlock)))
          {
            // If the value is empty
            if (csValue.IsEmpty()) 
            {
              /*
              // If this is a mandatory attribute
              if (csaMands.GetAt(iBlock) == _T("1"))
              {
                // If this is the first one
                if (csUnfilled.IsEmpty())
                {
                  // Add all details to the arrays
                  csUnfilled = csTag;
                  csaMANames.Add(csBlock);
                  csaMALayers.Add(csLayer);
                  csaMAAttribs.Add(csUnfilled);
                  geaErrors.append(gePos);
                }
                // Otherwise
                else
                {
                  // Update the value in the array
                  csUnfilled += (_T(", ") + csTag);
                  csaMAAttribs.SetAt(csaMAAttribs.GetUpperBound(), csUnfilled);
                }
              }
              */

              // Form the xdata to clear any previously attached xdata of the same name
              rbpXData = acutBuildList(AcDb::kDxfRegAppName, csaDescs.GetAt(iBlock), NULL);
            }
            // Form the xdata in the name of the associated description for the value in the attribute
            else rbpXData = acutBuildList(AcDb::kDxfRegAppName, csaDescs.GetAt(iBlock), AcDb::kDxfXdAsciiString, csValue, NULL);

            // Add it to the parent block with the value
            addXDataToEntity(enRef, rbpXData);
            acutRelRb(rbpXData);

            // Stop processing this loop
            break;
          }
        }
      }

      // Delete the iterator
      delete pIter;
    }

    // Free the selection set
    acedSSFree(ssBlocks);
  }

  // For every entry in the array, insert the error block and store its handle
  InsertErrorBlocks(geaErrors, csaMAHandles, csaMAErrorNos, csaTempX);

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ValidateMandatoryAttributes
// Called from  : Command_Validate
// Description  : Checks every block and linear feature on "PROP" layers for unfilled mandatory attributes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ValidateMandatoryAttributes(CStringArray& csaMAHandles, CStringArray& csaMANames, CStringArray& csaMAAttribs, CStringArray& csaMALayers, CStringArray& csaMAErrorNos, CStringArray& csaMAEntHandles)
{
  CString csSQL;
  CQueryTbl tblStd;

  // Collect all linear and block reference entities on "*PROP*" layers
  struct resbuf *rbpFilter = acutBuildList(-4, _T("<OR"), RTDXF0, _T("INSERT"), RTDXF0, _T("LINE"), RTDXF0, _T("ARC"), RTDXF0, _T("CIRCLE"), RTDXF0, _T("LWPOLYLINE"), RTDXF0, _T("POLYLINE"), -4, _T("OR>"), 8, _T("*PROP*"), NULL);
  ads_name ssProps; if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssProps) != RTNORM) return TRUE;
  //long lProps; if ((acedSSLength(ssProps, &lProps) != RTNORM) || (lProps == 0L)) return TRUE;
  int lProps; if ((acedSSLength(ssProps, &lProps) != RTNORM) || (lProps == 0L)) return TRUE;

  // Collect the block data
  csSQL.Format(_T("SELECT [Block], [Tag], [Desc] FROM tblBlockProps WHERE [Mandatory] <> 0 ORDER BY [Block], [Tag]"));
  if (!tblStd.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, "VMA",true)) return FALSE;
  if (tblStd.GetRows() <= 0) return FALSE;
  CStringArray csaBNames; tblStd.GetColumnAt(0, csaBNames);
  CStringArray csaBTags;  tblStd.GetColumnAt(1, csaBTags);
  CStringArray csaBDescs; tblStd.GetColumnAt(2, csaBDescs);

  ACHAR szHandle[17];
  bool bMandatory = false;
  CString csLayer, csBlock, csHandle;
  ads_name enProp;
  AcGePoint3d gePos;
  AcDbObjectId objProp;
  AcDbHandle dbHandle;
  AcDbEntity *pProp;
  AcDbBlockReference *pRef;
  AcGePoint3dArray geaErrors;
  struct resbuf *rbpAcc = NULL;

  // For each entity
  for (long lCtr = 0L; lCtr < lProps; lCtr++)
  {
    // Get the entity name
    if (acedSSName(ssProps, lCtr, enProp) != RTNORM) continue;

    // Get the entity details
    if (acdbGetObjectId(objProp, enProp) != Acad::eOk) continue;
    if (acdbOpenObject(pProp, objProp, AcDb::kForRead) != Acad::eOk) continue;
    pProp->getAcDbHandle(dbHandle);
    dbHandle.getIntoAsciiBuffer(szHandle);
    csHandle = szHandle;
    csLayer = pProp->layer();

    // If the layer contains "BASE_PROP-", we need not check this object
    if (csLayer.Find(_T("BASE_PROP-")) > -1) { pProp->close(); continue; }

    // If the layer contains "PROP-WORK_", we need not check this object
    if (csLayer.Find(_T("PROP-WORK_")) > -1) { pProp->close(); continue; }

    // If this is a block reference
    if (pRef = AcDbBlockReference::cast(pProp))
    {
      // Get the block name, its position and its attribute iterator
      csBlock = Assoc(acdbEntGet(enProp), 2)->resval.rstring;
      gePos = pRef->position();

      // Loop through the block names
      for (int iCtr = 0; iCtr < csaBNames.GetSize(); iCtr++)
      {
        // If the block name matches
        if (csaBNames.GetAt(iCtr).CompareNoCase(csBlock) == 0)
        {
          // Check if there is an xdata in the entity
          if (getXDataFromEntity(enProp, csaBDescs.GetAt(iCtr)) == NULL)
          {
            // If it has the "Accepted" xdata
            CString csErrType;
            rbpAcc = getXDataFromEntity(enProp, XDATA_Accepted);
            if (rbpAcc) csErrType = rbpAcc->rbnext->rbnext->resval.rstring;

            // If this is the same error type
            if (csErrType == _T("UNFILLED ATTRIBUTE"))
            {
              // Add the values to the global arrays
              g_csaAccErrNos.Add(rbpAcc->rbnext->resval.rstring);
              g_csaAccErrTypes.Add(rbpAcc->rbnext->rbnext->resval.rstring);
              g_csaAccErrLayers.Add(rbpAcc->rbnext->rbnext->rbnext->resval.rstring);
              g_csaAccErrReasons.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->resval.rstring);
              g_csaAccErrHandles.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rstring);
            }
            else
            {
              csaMAEntHandles.Add(csHandle);
              csaMANames.Add(csBlock);
              csaMALayers.Add(csLayer);
              csaMAAttribs.Add(csaBDescs.GetAt(iCtr));
              geaErrors.append(gePos);
            }
          }
        }
      }
    }
    // Otherwise, this is a linear feature
    else
    {
      // Decide the position
      if (AcDbLine::cast(pProp))
      {
        AcDbLine *pLine = AcDbLine::cast(pProp);
        gePos = pLine->startPoint();
      }
      else if (AcDbArc::cast(pProp))
      {
        AcDbArc *pArc = AcDbArc::cast(pProp);
        gePos = pArc->center();
      }
      else if (AcDbCircle::cast(pProp))
      {
        AcDbCircle *pCircle = AcDbCircle::cast(pProp);
        gePos = pCircle->center();
      }
      else if (AcDbPolyline::cast(pProp))
      {
        AcDbPolyline *pPoly = AcDbPolyline::cast(pProp);
        pPoly->getPointAt(0, gePos);
      }
      else if (AcDb2dPolyline::cast(pProp))
      {
        AcDb2dVertex *pVert;
        AcDb2dPolyline *p2dPoly = AcDb2dPolyline::cast(pProp);
        AcDbObjectIterator *pIter = p2dPoly->vertexIterator();
        if (acdbOpenObject(pVert, pIter->objectId(), AcDb::kForRead) != Acad::eOk) continue;
        gePos = pVert->position();
        pVert->close();
        delete pIter;
      }

      // Check if the entity has the "Code" xdata
      if (getXDataFromEntity(enProp, _T("Code")) == NULL)
      {
        // If it has the "Accepted" xdata
        CString csErrType;
        rbpAcc = getXDataFromEntity(enProp, XDATA_Accepted);
        if (rbpAcc) csErrType = rbpAcc->rbnext->rbnext->resval.rstring;

        // If this is the same error type
        if (csErrType == _T("UNFILLED ATTRIBUTE"))
        {
          // Add the values to the global arrays
          g_csaAccErrNos.Add(rbpAcc->rbnext->resval.rstring);
          g_csaAccErrTypes.Add(rbpAcc->rbnext->rbnext->resval.rstring);
          g_csaAccErrLayers.Add(rbpAcc->rbnext->rbnext->rbnext->resval.rstring);
          g_csaAccErrReasons.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->resval.rstring);
          g_csaAccErrHandles.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rstring);
        }
        else
        {
          csaMAEntHandles.Add(csHandle);
          csaMANames.Add(Assoc(acdbEntGet(enProp), 0)->resval.rstring);
          csaMALayers.Add(csLayer);
          csaMAAttribs.Add(_T("Code"));
          geaErrors.append(gePos);
        }
      }
    }

    // Close the entity
    pProp->close();
  }


  // For every entry in the array, insert the error block and store its handle
  InsertErrorBlocks(geaErrors, csaMAHandles, csaMAErrorNos, csaMAEntHandles);

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Unused_ValidateLayerSettings
// Called from  : Command_Validate
// Description  : ***** THIS FUNCTION IS NO LONGER USED *****
//                Validates all layers in the drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Unused_ValidateLayerSettings(CStringArray& csaActNames, CStringArray& csaActColors, CStringArray& csaStdColors, CStringArray& csaActLTypes, CStringArray& csaStdLTypes, CStringArray& csaActWeights, CStringArray& csaStdWeights, CStringArray& csaLayErrorNos)
{
  // Read the layer configuration data
  CString csSQL;
  CQueryTbl tblLayers;
  csSQL.Format(_T("SELECT [Prefix], [Mid1], [Mid2], [Suffix], [Colour], [LineType], [LineWeight] FROM tblLayers ORDER BY [Prefix], [Mid1], [Mid2], [Suffix]"));
  if (!tblLayers.SqlRead(DSN_ECapture, csSQL, __LINE__, __FILE__, _T("ChangeLayerSettings"),true)) return FALSE;
  if (tblLayers.GetRows() <= 0) { appMessage(_T("Layer settings have not been defined.\nPlease contact your System Administrator.")); return FALSE; }
  CStringArray csaPrefix;  tblLayers.GetColumnAt(0, csaPrefix);
  CStringArray csaMid1;    tblLayers.GetColumnAt(1, csaMid1);
  CStringArray csaMid2;    tblLayers.GetColumnAt(2, csaMid2);
  CStringArray csaSuffix;  tblLayers.GetColumnAt(3, csaSuffix);
  CStringArray csaColors;  tblLayers.GetColumnAt(4, csaColors);
  CStringArray csaLTypes;  tblLayers.GetColumnAt(5, csaLTypes);
  CStringArray csaWeights; tblLayers.GetColumnAt(6, csaWeights);

  // Get the layer table iterator
  AcDbLayerTable *pLayerTbl; acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForWrite);
  AcDbLayerTableIterator *pLayerIterator; pLayerTbl->newIterator(pLayerIterator);
  pLayerTbl->close();

  // Loop through the layers in the current drawing
  int iIndex = 0;
  ACHAR *pLayerName;
  CString csName, csColor, csLineType, csLineWeight, csErrorNo;
  AcCmColor cmColor;
  AcDb::LineWeight lwValue;
  AcDbLayerTableRecord *pLayerTblRecord;
  for (; !pLayerIterator->done(); pLayerIterator->step())
  {  
    // Get the layer name, color, line type and line weight settings
    pLayerIterator->getRecord(pLayerTblRecord, AcDb::kForRead);
    pLayerTblRecord->getName(pLayerName);
    cmColor    = pLayerTblRecord->color();
    lwValue    = pLayerTblRecord->lineWeight();
    pLayerTblRecord->close();

    csName.Format(_T("%s"), pLayerName);
    csColor.Format(_T("%d"), cmColor.colorIndex());
    csLineType = GetLineTypeName(pLayerTblRecord->linetypeObjectId());
    csLineWeight.Format(_T("%s"), suppressZero(lwValue / 100.0));

    // If it matches a record in the configuration
    if ((iIndex = MatchLayerName(pLayerName, csaPrefix, csaMid1, csaMid2, csaSuffix)) > -1)
    {
      // If one of the parameters is not matching the standard
      if ((csColor.CompareNoCase(csaColors.GetAt(iIndex)) != 0) || (csLineType.CompareNoCase(csaLTypes.GetAt(iIndex)) != 0) || (_tstof(csLineWeight) != _tstof(csaWeights.GetAt(iIndex))))
      {
        // Add the values to the arrays
        csErrorNo.Format(_T("%d"), g_iErrorNo++);
        csaActNames.Add(csName);
        csaActColors.Add(csColor);
        csaStdColors.Add(csaColors.GetAt(iIndex));
        csaActLTypes.Add(csLineType);
        csaStdLTypes.Add(csaLTypes.GetAt(iIndex));
        csaActWeights.Add(csLineWeight);
        csaStdWeights.Add(csaWeights.GetAt(iIndex));
        csaLayErrorNos.Add(csErrorNo);
      }
    }
  }

  // Delete the iterator
  delete pLayerIterator;

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ValidateLayerSettings
// Called from  : Command_Validate
// Description  : Validates all layers in the drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ValidateLayerSettings(CStringArray& csaActNames, CStringArray& csaActColors, CStringArray& csaStdColors, CStringArray& csaActLTypes, CStringArray& csaStdLTypes, CStringArray& csaActWeights, CStringArray& csaStdWeights, CStringArray& csaLayErrorNos, bool bExcludeOff)
{
  // Open the standards drawing
  Acad::ErrorStatus es;
  AcDbDatabase *pStdDb = new AcDbDatabase;
  //Commented for ACAD 2018
  //if ((es = pStdDb->readDwgFile(g_csLocalDWT, _SH_DENYWR))  != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pStdDb->readDwgFile(g_csLocalDWT, AcDbDatabase::OpenMode::kForReadAndReadShare)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }

  // Get the layer table iterator from the standards drawing
  AcDbLayerTable *pStdLayerTbl; if ((es = pStdDb->getLayerTable(pStdLayerTbl, AcDb::kForRead)) != Acad::eOk) { delete pStdDb; return FALSE; }
  AcDbLayerTableIterator *pStdLayerIter; pStdLayerTbl->newIterator(pStdLayerIter);
  pStdLayerTbl->close();

  // Get the layer table iterator from the current drawing
  AcDbLayerTable *pCurLayerTbl; acdbHostApplicationServices()->workingDatabase()->getLayerTable(pCurLayerTbl, AcDb::kForRead);
  AcDbLayerTableIterator *pCurLayerIter; pCurLayerTbl->newIterator(pCurLayerIter);

  int iStdColor = 0, iCurColor = 0;
  bool bFound = false;
  ACHAR *pCurLayerName, *pStdLayerName;
  CString csErrorNo, csCurLayer, csStdLayer, csCurColor, csStdColor, csCurLType, csStdLType, csCurWeight, csStdWeight;
  AcDb::LineWeight lwCurWeight, lwStdWeight;
  AcDbLayerTableRecord *pCurLayerTblRecord, *pStdLayerTblRecord;

  // Loop through the layers in the current drawing
  for (; !pCurLayerIter->done(); pCurLayerIter->step())
  {  
    // Get the current layer name
    if (pCurLayerIter->getRecord(pCurLayerTblRecord, AcDb::kForWrite) != Acad::eOk) continue;
    pCurLayerTblRecord->getName(pCurLayerName);
    iCurColor  = pCurLayerTblRecord->color().colorIndex();
    lwCurWeight = pCurLayerTblRecord->lineWeight();
    csCurLType = GetLineTypeName(pCurLayerTblRecord->linetypeObjectId());
    pCurLayerTblRecord->close();

    // Convert the current data
    csCurLayer = pCurLayerName;
    csCurColor.Format(_T("%d"), iCurColor);
    csCurWeight.Format(_T("%.2f"), lwCurWeight);

    // If the layer begins with "XT_", ignore it
    if (csCurLayer.Mid(0, 3) == _T("XT_")) continue;

    // Reset the flag
    bFound = false;

    // Get the corresponding layer in the standards
    for (pStdLayerIter->start(); !pStdLayerIter->done(); pStdLayerIter->step())
    {
      if (pStdLayerIter->getRecord(pStdLayerTblRecord, AcDb::kForRead) != Acad::eOk) continue;
      pStdLayerTblRecord->getName(pStdLayerName);
      iStdColor  = pStdLayerTblRecord->color().colorIndex();
      lwStdWeight = pStdLayerTblRecord->lineWeight();
      csStdLType = GetStdLineTypeName(pStdDb, pStdLayerTblRecord->linetypeObjectId());
      pStdLayerTblRecord->close();

      // Convert the standard data
      csStdLayer = pStdLayerName;
      csStdColor.Format(_T("%d"), iStdColor);
      csStdWeight.Format(_T("%.2f"), lwStdWeight);

      // If it is the same
      if (csStdLayer == csCurLayer)
      {
        // Set the flag
        bFound = true;

        // Check if the color, linetype and line weight matches
        if ((csCurColor != csStdColor) || (csCurWeight != csStdWeight) || (csCurLType != csStdLType))
        {
          // Add the values to the arrays
          csErrorNo.Format(_T("0"));
          csaActNames.Add(csCurLayer);
          csaActColors.Add(csCurColor);
          csaStdColors.Add(csStdColor);
          csaActLTypes.Add(csCurLType);
          csaStdLTypes.Add(csStdLType);
          csaActWeights.Add(csCurWeight);
          csaStdWeights.Add(csStdWeight);
          csaLayErrorNos.Add(csErrorNo);
        }
      }
    }

    // If the layer is still not found
    if (bFound == false)
    {
      csStdColor = csStdLType = csStdWeight = _T("N.A.");

      // If this is an "_OFF" layer
      if (csCurLayer.Right(4) == _T("_OFF"))
      {
        // Check if the corresponding layer (without the _OFF) is present
        CString csCurLayerNoOff = csCurLayer.Mid(0, csCurLayer.GetLength() - 4);
        if (pCurLayerTbl->getAt(csCurLayerNoOff, pCurLayerTblRecord, AcDb::kForRead) == Acad::eOk)
        {
          iStdColor  = pCurLayerTblRecord->color().colorIndex();
          lwStdWeight = pCurLayerTblRecord->lineWeight();
          csStdLType = GetStdLineTypeName(acdbHostApplicationServices()->workingDatabase(), pCurLayerTblRecord->linetypeObjectId());
          pCurLayerTblRecord->close();

          // Convert the standard data
          csStdLayer = csCurLayer;
          csStdColor.Format(_T("%d"), iStdColor);
          csStdWeight.Format(_T("%.2f"), lwStdWeight);

          // If the color, linetype and line weight matches, set the flag
          if ((csCurColor == csStdColor) && (csCurWeight == csStdWeight) && (csCurLType == csStdLType)) bFound = true;

          // If we need not check for "_OFF", set the flag anyway
          if (bExcludeOff == true) bFound = true;
        }
      }

      // If it is still not found
      if (bFound == false)
      {
        // Add the values to the arrays
        csErrorNo.Format(_T("0"));
        csaActNames.Add(csCurLayer);
        csaActColors.Add(csCurColor);
        csaStdColors.Add(csStdColor);
        csaActLTypes.Add(csCurLType);
        csaStdLTypes.Add(csStdLType);
        csaActWeights.Add(csCurWeight);
        csaStdWeights.Add(csStdWeight);
        csaLayErrorNos.Add(csErrorNo);
      }
    }
  }

  // Close the layer table
  pCurLayerTbl->close();

  // Delete the iterators
  delete pStdLayerIter;
  delete pCurLayerIter;

  // Delete the standard drawing pointer
  delete pStdDb;

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ValidateObjectLayers
// Called from  : Command_Validate
// Description  : Validates whether the objects are on their respective layers in the drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ValidateObjectLayers(CStringArray& csaObjTypes, CStringArray& csaObjHandles, CStringArray& csaObjActLayers, CStringArray& csaObjStdLayers, CStringArray& csaObjErrorNos, CStringArray& csaObjErrorHandles, bool bExcludeOff)
{
  // Collect all entities with the "DXFTRANS" xdata
  struct resbuf *rbpFilter = NULL;
  if (bExcludeOff == true) rbpFilter = acutBuildList(-4, _T("<NOT"), 8, _T("*_OFF"), -4, _T("NOT>"), AcDb::kDxfRegAppName, XDATA_Translate, NULL);
  else rbpFilter = acutBuildList(AcDb::kDxfRegAppName, XDATA_Translate, NULL);
  ads_name ssTrans; if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssTrans) != RTNORM) { acutRelRb(rbpFilter); return TRUE; }
  //long lTrans = 0L; if ((acedSSLength(ssTrans, &lTrans) != RTNORM) || (lTrans == 0L)) { acutRelRb(rbpFilter); return TRUE; }
  int lTrans = 0L; if ((acedSSLength(ssTrans, &lTrans) != RTNORM) || (lTrans == 0L)) { acutRelRb(rbpFilter); return TRUE; }
  acutRelRb(rbpFilter);

  // Loop through each entity in the selection set
  ads_name enTrans;
  AcGePoint3d gePos;
  AcGePoint3dArray geaErrors;
  CString csCurLayer, csOrgLayer, csType, csHandle;
  struct resbuf *rbpXData = NULL, *rbpEnt = NULL, *rbpAcc = NULL;
  for (long lCtr = 0L; lCtr < lTrans; lCtr++)
  {
    // Get the entity name
    acedSSName(ssTrans, lCtr, enTrans);

    // Get the position of the entity
    gePos = GetEntityPosition(enTrans);

    // Get the entity data from its properties
    rbpEnt     = acdbEntGet(enTrans);
    csType     = Assoc(rbpEnt, 0)->resval.rstring;
    csHandle   = Assoc(rbpEnt, 5)->resval.rstring;
    csCurLayer = Assoc(rbpEnt, 8)->resval.rstring;

    // Get the original layer from the xdata
    rbpXData = getXDataFromEntity(enTrans, XDATA_Translate);
    csOrgLayer = rbpXData->rbnext->resval.rstring;

    // If they are not same, add the values to the arrays
    if (csCurLayer != csOrgLayer)
    {
      // If it has the "Accepted" xdata
      CString csErrType;
      rbpAcc = getXDataFromEntity(enTrans, XDATA_Accepted);
      if (rbpAcc) csErrType = rbpAcc->rbnext->rbnext->resval.rstring;

      // If this is the same error type
      if (csErrType == _T("CHANGED LAYER"))
      {
        // Add the values to the global arrays
        g_csaAccErrNos.Add(rbpAcc->rbnext->resval.rstring);
        g_csaAccErrTypes.Add(rbpAcc->rbnext->rbnext->resval.rstring);
        g_csaAccErrLayers.Add(rbpAcc->rbnext->rbnext->rbnext->resval.rstring);
        g_csaAccErrReasons.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->resval.rstring);
        g_csaAccErrHandles.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rstring);
      }
      else
      {
        csaObjTypes.Add(csType);
        csaObjHandles.Add(csHandle);
        csaObjActLayers.Add(csCurLayer);
        csaObjStdLayers.Add(csOrgLayer);
        geaErrors.append(gePos);
      }
    }
  }

  // Free the selection set
  acedSSFree(ssTrans);

  // For every entry in the array, insert the error block and store its handle
  InsertErrorBlocks(geaErrors, csaObjErrorHandles, csaObjErrorNos, csaObjHandles);

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ValidateNonStdBlocks
// Called from  : Command_Validate
// Description  : Validates for inserts with non-standard block names in the drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ValidateNonStdBlocks(CStringArray& csaNSBHandles, CStringArray& csaNSBNames, CStringArray& csaNSBLayers, CStringArray& csaNSBErrorNos, CStringArray& csaNSBEntHandles)
{
  // Collect all INSERTs in the current drawing
  struct resbuf *rbpFilter = acutBuildList(RTDXF0, _T("INSERT"), NULL);
  ads_name ssInserts; if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssInserts) != RTNORM) return TRUE;
  //long lInserts = 0L; if ((acedSSLength(ssInserts, &lInserts) != RTNORM) || (lInserts == 0L)) { acedSSFree(ssInserts); acutRelRb(rbpFilter); return TRUE; }
  int lInserts = 0L; if ((acedSSLength(ssInserts, &lInserts) != RTNORM) || (lInserts == 0L)) { acedSSFree(ssInserts); acutRelRb(rbpFilter); return TRUE; }

  // Open the standards drawing for reading and its block table
  Acad::ErrorStatus es;
  AcDbBlockTable *pStdBT;
  AcDbDatabase *pStdDb = new AcDbDatabase;
  //Commented for ACAD 2018
  //if ((es = pStdDb->readDwgFile(g_csLocalDWT, _SH_DENYWR))  != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pStdDb->readDwgFile(g_csLocalDWT, AcDbDatabase::OpenMode::kForReadAndReadShare)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pStdDb->getSymbolTable(pStdBT, AcDb::kForRead)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s getting block table from standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }

  // For each entity in the selection set
  ads_name enIns;
  CString csBlock, csLayer, csHandle;
  AcGePoint3d gePos;
  AcGePoint3dArray geaErrors;
  struct resbuf *rbpEnt = NULL,  *rbpAcc = NULL;
  for (long lCtr = 0; lCtr < lInserts; lCtr++)
  {
    // Get the block name
    acedSSName(ssInserts, lCtr, enIns);
    rbpEnt   = acdbEntGet(enIns);
    csBlock  = Assoc(rbpEnt, 2)->resval.rstring;
    csHandle = Assoc(rbpEnt, 5)->resval.rstring;
    csLayer  = Assoc(rbpEnt, 8)->resval.rstring;
    gePos    = asPnt3d(Assoc(rbpEnt, 10)->resval.rpoint);

    // If this block is an unnamed one, no check is required
    if (csBlock[0] == _T('*')) continue;

    // If this block is not present the standards drawing
    if ((csLayer.Mid(0, 3) != _T("XT_")) && (pStdBT->has(csBlock) == false))
    { 
      // If it has the "Accepted" xdata
      CString csErrType;
      rbpAcc = getXDataFromEntity(enIns, XDATA_Accepted);
      if (rbpAcc) csErrType = rbpAcc->rbnext->rbnext->resval.rstring;

      // If this is the same error type
      if (csErrType == _T("NON-STD BLOCK"))
      {
        // Add the values to the global arrays
        g_csaAccErrNos.Add(rbpAcc->rbnext->resval.rstring);
        g_csaAccErrTypes.Add(rbpAcc->rbnext->rbnext->resval.rstring);
        g_csaAccErrLayers.Add(rbpAcc->rbnext->rbnext->rbnext->resval.rstring);
        g_csaAccErrReasons.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->resval.rstring);
        g_csaAccErrHandles.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rstring);
      }
      else
      {
        // Store the data in the array
        csaNSBNames.Add(csBlock);
        csaNSBEntHandles.Add(csHandle);
        csaNSBLayers.Add(csLayer);
        geaErrors.append(gePos);
      }
    }
  }

  // Close the block table and delete the standards drawing pointer
  pStdBT->close();
  delete pStdDb;

  // Free the selection set
  acedSSFree(ssInserts);

  // For every entry in the array, insert the error block and store its handle
  InsertErrorBlocks(geaErrors, csaNSBHandles, csaNSBErrorNos, csaNSBEntHandles);

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ValidateLayer0Geometry
// Called from  : Command_Validate
// Description  : Validates for entities on layer "0"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ValidateLayer0Geometry(CStringArray& csa0Handles, CStringArray& csa0Names, CStringArray& csa0ErrorNos, CStringArray& csa0EntHandles)
{
  // Collect all entities on layer "0" in the current drawing's model space (used "NOT" since we can't put (67, 0) as 0 is same as NULL and RTDXF0 is 5020)
  struct resbuf *rbpFilter = acutBuildList(8, _T("0"), -4, _T("<NOT"), 67, 1, -4, _T("NOT>"), NULL);
  ads_name ssLayer0; if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssLayer0) != RTNORM) { acutRelRb(rbpFilter); return TRUE; }
  //long lLayer0 = 0L; if ((acedSSLength(ssLayer0, &lLayer0) != RTNORM) || (lLayer0 == 0L)) { acedSSFree(ssLayer0); acutRelRb(rbpFilter); return TRUE; }
  int lLayer0 = 0L; if ((acedSSLength(ssLayer0, &lLayer0) != RTNORM) || (lLayer0 == 0L)) { acedSSFree(ssLayer0); acutRelRb(rbpFilter); return TRUE; }

  // Get the entity name and handle of each of them
  ads_name enLayer0;
  AcGePoint3d gePos;
  AcGePoint3dArray geaErrors;
  struct resbuf *rbpEnt = NULL, *rbpAcc = NULL;
  for (long lCtr = 0L; lCtr < lLayer0; lCtr++)
  {
    acedSSName(ssLayer0, lCtr, enLayer0);

    // If it has the "Accepted" xdata
    CString csErrType;
    rbpAcc = getXDataFromEntity(enLayer0, XDATA_Accepted);
    if (rbpAcc) csErrType = rbpAcc->rbnext->rbnext->resval.rstring;

    // If this is the same error type
    if (csErrType == _T("ON LAYER 0"))
    {
      // Add the values to the global arrays
      g_csaAccErrNos.Add(rbpAcc->rbnext->resval.rstring);
      g_csaAccErrTypes.Add(rbpAcc->rbnext->rbnext->resval.rstring);
      g_csaAccErrLayers.Add(rbpAcc->rbnext->rbnext->rbnext->resval.rstring);
      g_csaAccErrReasons.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->resval.rstring);
      g_csaAccErrHandles.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rstring);

      continue;
    }

    gePos = GetEntityPosition(enLayer0);
    rbpEnt = acdbEntGet(enLayer0);

    csa0Names.Add(Assoc(rbpEnt, 0)->resval.rstring);    
    csa0EntHandles.Add(Assoc(rbpEnt, 5)->resval.rstring);
    geaErrors.append(gePos);
    
    acutRelRb(rbpEnt);
  }

  // Free the selection set
  acedSSFree(ssLayer0);

  // For every entry in the array, insert the error block and store its handle
  InsertErrorBlocks(geaErrors, csa0Handles, csa0ErrorNos, csa0EntHandles);

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : NotWitinMGA
// Called from  : ValidateAssetMovement
// Description  : Checks if given point is with the standard Mapping Grid of Australia co-ordinates
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL NotWithinMGA(AcGePoint3d geLoc)
{
  AcGePoint3d geMGAMin(210000.0, 6200000.0, 0.0), geMGAMax(426000.0, 6500000.0, 0.0);

  if (geLoc.x < geMGAMin.x) return TRUE;
  if (geLoc.x > geMGAMax.x) return TRUE;
  if (geLoc.y < geMGAMin.y) return TRUE;
  if (geLoc.y > geMGAMax.y) return TRUE;

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : AddPointToArray
// Called from  : ValidateAssetMovement
// Description  : Adds the point to the array, with the other details
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AddPointToArray(ads_name enTrans, CStringArray& csaMovEntHandles, CStringArray& csaMovNames, CStringArray& csaMovLayers, CStringArray& csaMovType, AcGePoint3dArray& geaErrors, 
                     CString csEntHandle, CString csName, CString csLayer, AcGePoint3d gePos, CString csType
                    )
{
  // If it has the "Accepted" xdata
  CString csErrType;
  struct resbuf *rbpAcc = getXDataFromEntity(enTrans, XDATA_Accepted);
  if (rbpAcc) csErrType = rbpAcc->rbnext->rbnext->resval.rstring;

  // If this is the same error type
  if (csErrType == _T("MOVED ASSET"))
  {
    // Add the values to the global arrays
    g_csaAccErrNos.Add(rbpAcc->rbnext->resval.rstring);
    g_csaAccErrTypes.Add(rbpAcc->rbnext->rbnext->resval.rstring);
    g_csaAccErrLayers.Add(rbpAcc->rbnext->rbnext->rbnext->resval.rstring);
    g_csaAccErrReasons.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->resval.rstring);
    g_csaAccErrHandles.Add(rbpAcc->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rstring);
  }
  else
  {
    csaMovEntHandles.Add(csEntHandle);
    csaMovNames.Add(csName);
    csaMovLayers.Add(csLayer);
    csaMovType.Add(csType);
    geaErrors.append(gePos);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ValidateAssetMovement
// Called from  : Command_Validate
// Description  : Validates for movement of assets and location of all co-ordinates within the MGA co-ordinates
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ValidateAssetMovement(CStringArray& csaMovEntHandles, CStringArray& csaMovHandles, CStringArray& csaMovNames, CStringArray& csaMovLayers, CStringArray& csaMovType, CStringArray& csaMovErrorNos, bool bBlockMovement, bool bOtherMovement, bool bExcludeOff)
{
  // If both movement flags are false, return now
  if ((bBlockMovement == false) && (bOtherMovement == false)) return TRUE;

  // Collect all entities with the "DXFTRANS" xdata
  struct resbuf *rbpFilter = NULL;
  if (bExcludeOff == true) rbpFilter = acutBuildList(-4, _T("<NOT"), 8, _T("*_OFF"), -4, _T("NOT>"), AcDb::kDxfRegAppName, XDATA_Translate, NULL);
  else rbpFilter = acutBuildList(AcDb::kDxfRegAppName, XDATA_Translate, NULL);
  ads_name ssTrans; if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssTrans) != RTNORM) { acutRelRb(rbpFilter); return TRUE; }
  //long lTrans = 0L; if ((acedSSLength(ssTrans, &lTrans) != RTNORM) || (lTrans == 0L)) { acutRelRb(rbpFilter); return TRUE; }
  int lTrans = 0L; if ((acedSSLength(ssTrans, &lTrans) != RTNORM) || (lTrans == 0L)) { acutRelRb(rbpFilter); return TRUE; }
  acutRelRb(rbpFilter);

  // Loop through each entity in the selection set
  ACHAR szHandle[17];
  ads_name enTrans;
  CString csType, csHandle, csLayer;
  AcDbEntity *pEnt = NULL;
  AcDbArc *pArc;
  AcDbText *pText;
  AcDbLine *pLine;
  AcDbMline *pMLine;
  AcDbCircle *pCircle;
  AcDbHandle dbHandle;
  AcDbPolyline *pPline;
  AcDb2dPolyline *pPline2d;
  AcDbBlockReference *pRef;
  AcDbObjectId objId;
  AcGePoint3d gePos, geLoc;
  AcGePoint3dArray geaErrors;
  struct resbuf *rbpXData = NULL, *rbpEnt = NULL;
  for (long lCtr = 0L; lCtr < lTrans; lCtr++)
  {
    // Get the entity name and store its position from the XData
    acedSSName(ssTrans, lCtr, enTrans);
    rbpXData = getXDataFromEntity(enTrans, XDATA_Translate);
    gePos = asPnt3d(rbpXData->rbnext->rbnext->rbnext->resval.rpoint);

    // Open the object for reading
    if (acdbGetObjectId(objId, enTrans) != Acad::eOk) continue;
    if (acdbOpenObject(pEnt, objId, AcDb::kForRead) != Acad::eOk) continue;

    // Get its handle and layer
    csLayer = pEnt->layer();
    pEnt->getAcDbHandle(dbHandle);
    dbHandle.getIntoAsciiBuffer(szHandle);
    csHandle = szHandle;

    // If we have to check for non-block movement
    if (bOtherMovement == true)
    {
      // If this is an ARC
      if (pArc = AcDbArc::cast(pEnt))
      {
        geLoc = pArc->center();
        if (geLoc != gePos)      AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Arc"), csLayer, geLoc, _T("Center point moved."));
        if (NotWithinMGA(geLoc)) AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Arc"), csLayer, geLoc, _T("Center point outside of Ausgrid area."));
      }
      // If this is a CIRCLE
      else if (pCircle = AcDbCircle::cast(pEnt))
      {
        geLoc = pCircle->center();
        if (geLoc != gePos)      AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Circle"), csLayer, geLoc, _T("Center point moved."));
        if (NotWithinMGA(geLoc)) AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Circle"), csLayer, geLoc, _T("Center point outside of Ausgrid area."));
      }
      // If this is a TEXT
      else if (pText = AcDbText::cast(pEnt))
      {
        geLoc = pText->alignmentPoint();
        if ((geLoc.x == 0.0) && (geLoc.y == 0.0)) geLoc = pText->position();
        if (geLoc != gePos)      AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Text"), csLayer, geLoc, _T("Insertion point moved."));
        if (NotWithinMGA(geLoc)) AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Text"), csLayer, geLoc, _T("Insertion point outside of Ausgrid area."));
      }
      else if (pLine = AcDbLine::cast(pEnt))
      {
        AcGePoint3d geEnd      = asPnt3d(rbpXData->rbnext->rbnext->rbnext->rbnext->resval.rpoint);
        AcGePoint3d geActStart = pLine->startPoint();
        AcGePoint3d geActEnd   = pLine->endPoint();
        if (geActStart != gePos)      AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Line"), csLayer, geActStart, _T("Start point moved."));
        if (geActEnd   != geEnd)      AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Line"), csLayer, geActEnd,   _T("End point moved."));
        if (NotWithinMGA(geActStart)) AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Line"), csLayer, geActStart, _T("Start point outside of Ausgrid area."));
        if (NotWithinMGA(geActEnd))   AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Line"), csLayer, geActEnd,   _T("End point outside of Ausgrid area."));
      }
      // If this is a MLINE
      else if (pMLine = AcDbMline::cast(pEnt))
      {
        int iNumOldVerts    = rbpXData->rbnext->rbnext->rbnext->rbnext->resval.rint;
        int iNumVerts       = pMLine->numVertices();
        AcGePoint3d geVert1 = pMLine->vertexAt(0);
        AcGePoint3d geVert2 = pMLine->vertexAt(iNumVerts - 1);
        AcGePoint3d geEnd   = asPnt3d(rbpXData->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rpoint);

        if (iNumVerts != iNumOldVerts) AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("MultiLine"), csLayer, geVert1, _T("Number of vertices not same."));
        if (geVert1   != gePos)        AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("MultiLine"), csLayer, geVert1, _T("Start point moved."));
        if (geVert2   != geEnd)        AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("MultiLine"), csLayer, geVert2, _T("End point moved."));
        if (NotWithinMGA(geVert1))     AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("MultiLine"), csLayer, geVert1, _T("Start point outside of Ausgrid area."));
        if (NotWithinMGA(geVert2))     AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("MultiLine"), csLayer, geVert2, _T("End point outside of Ausgrid area."));
      }
      // If this is a LWPOLYLINE
      else if (pPline = AcDbPolyline::cast(pEnt))
      {
        int iNumOldVerts    = rbpXData->rbnext->rbnext->rbnext->rbnext->resval.rint;
        int iNumVerts       = pPline->numVerts();
        AcGePoint3d geVert1; pPline->getPointAt(0, geVert1);
        AcGePoint3d geVert2; pPline->getPointAt(iNumVerts - 1, geVert2);
        AcGePoint3d geEnd   = asPnt3d(rbpXData->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rpoint);

        if (iNumVerts != iNumOldVerts) AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("PolyLine"), csLayer, geVert1, _T("Number of vertices not same."));
        if (geVert1   != gePos)        AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("PolyLine"), csLayer, geVert1, _T("Start point moved."));
        if (geVert2   != geEnd)        AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("PolyLine"), csLayer, geVert2, _T("End point moved."));
        if (NotWithinMGA(geVert1))     AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("PolyLine"), csLayer, geVert1, _T("Start point outside of Ausgrid area."));
        if (NotWithinMGA(geVert2))     AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("PolyLine"), csLayer, geVert2, _T("End point outside of Ausgrid area."));
      }
      // If this is a POLYLINE (old style)
      else if (pPline2d = AcDb2dPolyline::cast(pEnt))
      {
        int iNumOldVerts    = rbpXData->rbnext->rbnext->rbnext->rbnext->resval.rint;
        AcGePoint3d geEnd   = asPnt3d(rbpXData->rbnext->rbnext->rbnext->rbnext->rbnext->resval.rpoint);

        int iNumVerts = 0;
        AcDb2dVertex *pVert;
        AcGePoint3d geVert, geVert1, geVert2;
        AcDbObjectIterator *pIter = pPline2d->vertexIterator();
        for (pIter->start(); !pIter->done(); pIter->step(), iNumVerts++) 
        {
          if (acdbOpenObject(pVert, pIter->objectId(), AcDb::kForRead) != Acad::eOk) continue;
          geVert = pVert->position();
          pVert->close();

          if (iNumVerts == 0) geVert1 = geVert;
          else geVert2 = geVert;
        }
        delete pIter;

        if (iNumVerts != iNumOldVerts) AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("2D PolyLine"), csLayer, geVert1, _T("Number of vertices not same."));
        if (geVert1   != gePos)        AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("2D PolyLine"), csLayer, geVert1, _T("Start point moved."));
        if (geVert2   != geEnd)        AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("2D PolyLine"), csLayer, geVert2, _T("End point moved."));
        if (NotWithinMGA(geVert1))     AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("2D PolyLine"), csLayer, geVert1, _T("Start point outside of Ausgrid area."));
        if (NotWithinMGA(geVert2))     AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("2D PolyLine"), csLayer, geVert2, _T("End point outside of Ausgrid area."));
      }
    }

    // If we have to check for block movement
    if (bBlockMovement == true)
    {
      // If this is an INSERT
      if (pRef = AcDbBlockReference::cast(pEnt))
      {
        geLoc = pRef->position();
        if (geLoc != gePos)      AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Insert"), csLayer, geLoc, _T("Insertion point moved."));
        if (NotWithinMGA(geLoc)) AddPointToArray(enTrans, csaMovEntHandles, csaMovNames, csaMovLayers, csaMovType, geaErrors, csHandle, _T("Insert"), csLayer, geLoc, _T("Insertion point outside of Ausgrid area."));
      }
    }

    // Close the entity
    pEnt->close();
  }

  // Free the selection set
  acedSSFree(ssTrans);

  // For every entry in the array, insert the error block and store its handle
  InsertErrorBlocks(geaErrors, csaMovHandles, csaMovErrorNos, csaMovEntHandles);

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CloseResultsDialog
// Called from  : CValidateResultsDlg::OnCancel
// Description  : Deletes the global pointer for the modeless dialog
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CloseResultsDialog()
{
  // Delete the pointer
  delete g_pdlgVR;
  g_pdlgVR = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_Validate
// Called from  : User, via the "EA_VALIDATE" command
// Description  : Validates the drawing for conformity with standards
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_Validate()
{
  // Close any previous instance of the validation results dialog
  if (g_pdlgVR) CloseResultsDialog();

  // Clear the global arrays
  g_csaAccErrNos.RemoveAll();
  g_csaAccErrTypes.RemoveAll();
  g_csaAccErrLayers.RemoveAll();
  g_csaAccErrReasons.RemoveAll();
  g_csaAccErrHandles.RemoveAll();

  // By default, movements will not be validated
  BOOL bBlockMovement = FALSE, bOtherMovement = FALSE, bExcludeOff = FALSE;

  // Set a few variables
  struct resbuf rbVar;
  rbVar.restype = RTSHORT;
  rbVar.resval.rint = 0; acedSetVar(_T("CMDECHO"),  &rbVar);  // Command echo OFF
  rbVar.resval.rint = 1; acedSetVar(_T("TILEMODE"), &rbVar);  // Model space ON

  // Ensure presence of validation block and layer
  if (!acdbTblSearch(_T("BLOCK"), g_csError, 0))  { displayMessage(_T("Error block %s not defined in the standards drawing."), g_csError);  return; }
  if (!acdbTblSearch(_T("LAYER"), g_csError, 0))  { displayMessage(_T("Error layer %s not defined in the standards drawing."), g_csError);  return; }
  if (!acdbTblSearch(_T("BLOCK"), g_csAccept, 0)) { displayMessage(_T("Error block %s not defined in the standards drawing."), g_csAccept); return; }

  // Ensure ATTREQ is ON and ATTDIA is OFF
  struct resbuf rbAtt; rbAtt.restype = RTSHORT; 
  rbAtt.resval.rint = 1; acedSetVar(_T("ATTREQ"), &rbAtt);
  rbAtt.resval.rint = 0; acedSetVar(_T("ATTDIA"), &rbAtt);

  // Clear the error blocks on the validation errors layer
  ads_name ssErrors;
  struct resbuf *rbpFilter = acutBuildList(RTDXF0, _T("INSERT"), 2, g_csError, 8, g_csError, NULL);
  if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssErrors) == RTNORM) acedCommandS(RTSTR, _T(".ERASE"), RTPICKS, ssErrors, RTSTR, _T(""), NULL);
  acedSSFree(ssErrors);

  // Reset the global error number
  g_iErrorNo = 1;

  // If we must validate movement, show the options dialog
  if (g_csValidateMove == _T("1"))
  {
    CValidateOptionsDlg dlgVO;
    if (dlgVO.DoModal() == IDCANCEL) return;
    bBlockMovement = dlgVO.m_bBlocks;
    bOtherMovement = dlgVO.m_bOthers;
    bExcludeOff    = (dlgVO.m_bExclude ? FALSE : TRUE); // Due to name change from "Exclude" to "Validate"
  }

  // If the connection gap specification in the INI is greater than 0.0
  if (_tstof(g_csMaxConnGap) > 0.0) 
  {
    // Join line end points with the connectivity limit into single polylines
    acutPrintf(_T("\nConverting connection lines into polylines...\n"));
    SnapLineEndPoints();
  }

  // Validate for innocent attribute changes and silently synchronize them (attributes get preference)
  CStringArray csaMAHandles, csaMANames, csaMAAttribs, csaMALayers, csaMAErrorNos, csaMAEntHandles;
  acutPrintf(_T("Synchronizing attribute to xdata links...\n"));
  if (ValidateAndSynchronizeAttributes(csaMAHandles, csaMANames, csaMAAttribs, csaMALayers, csaMAErrorNos) == FALSE) return;

  // Validate attributes in blocks and linear features which are not coming via the DXF
  acutPrintf(_T("Validating mandatory attributes in blocks and linear features...\n"));
  if (ValidateMandatoryAttributes(csaMAHandles, csaMANames, csaMAAttribs, csaMALayers, csaMAErrorNos, csaMAEntHandles) == FALSE) return;

  // Validate layer settings
  CStringArray csaActNames, csaActColors, csaStdColors, csaActLTypes, csaStdLTypes, csaActWeights, csaStdWeights, csaLayErroNos;
  acutPrintf(_T("\nValidating layer settings...\n"));
  if (ValidateLayerSettings(csaActNames, csaActColors, csaStdColors, csaActLTypes, csaStdLTypes, csaActWeights, csaStdWeights, csaLayErroNos, bExcludeOff) == FALSE) return;

  // Validate object layers
  CStringArray csaObjTypes, csaObjHandles, csaObjActLayers, csaObjStdLayers, csaObjErrorNos, csaObjErrorHandles;
  acutPrintf(_T("Validating object layers...\n"));
  if (ValidateObjectLayers(csaObjTypes, csaObjHandles, csaObjActLayers, csaObjStdLayers, csaObjErrorNos, csaObjErrorHandles, bExcludeOff) == FALSE) return;

  // Validate for non-standard blocks
  CStringArray csaNSBHandles, csaNSBNames, csaNSBLayers, csaNSBErrorNos, csaNSBEntHandles;
  acutPrintf(_T("Checking for non-standard blocks...\n"));
  if (ValidateNonStdBlocks(csaNSBHandles, csaNSBNames, csaNSBLayers, csaNSBErrorNos, csaNSBEntHandles) == FALSE) return;

  // Validate for entities on layer "0"
  CStringArray csa0Handles, csa0Names, csa0ErrorNos, csa0EntHandles;
  acutPrintf(_T("Checking for geometry on layer 0...\n"));
  if (ValidateLayer0Geometry(csa0Handles, csa0Names, csa0ErrorNos, csa0EntHandles) == FALSE) return;

  // Validate for asset movement
  CStringArray csaMovHandles, csaMovNames, csaMovLayers, csaMovType, csaMovErrorNos, csaMovEntHandles;
  if (bBlockMovement || bOtherMovement) 
  {
    acutPrintf(_T("Checking movement of assets...\n"));
    if (ValidateAssetMovement(csaMovEntHandles, csaMovHandles, csaMovNames, csaMovLayers, csaMovType, csaMovErrorNos, bBlockMovement, bOtherMovement, bExcludeOff) == FALSE) return;
  }

  // If the error number is still 1 or there is nothing in the layers array (since they don't have error numbers), then there have been no errors
  if ((g_iErrorNo == 1) && (csaActNames.GetSize() == 0)) { appMessage(_T("Validation completed and no errors were found.")); return; }

  // Initialize the results dialog
  g_pdlgVR = new CValidateResultsDlg;

  // Copy the mismatching layers
  g_pdlgVR->m_dlgLayers.m_csaActNames.Copy(csaActNames);
  g_pdlgVR->m_dlgLayers.m_csaActColors.Copy(csaActColors);
  g_pdlgVR->m_dlgLayers.m_csaStdColors.Copy(csaStdColors);
  g_pdlgVR->m_dlgLayers.m_csaActLTypes.Copy(csaActLTypes);
  g_pdlgVR->m_dlgLayers.m_csaStdLTypes.Copy(csaStdLTypes);
  g_pdlgVR->m_dlgLayers.m_csaActWeights.Copy(csaActWeights);
  g_pdlgVR->m_dlgLayers.m_csaStdWeights.Copy(csaStdWeights);
  g_pdlgVR->m_dlgLayers.m_csaErrorNos.Copy(csaLayErroNos);
  
  // Copy the mismatching objects
  g_pdlgVR->m_dlgObjects.m_csaObjTypes.Copy(csaObjTypes);
  g_pdlgVR->m_dlgObjects.m_csaObjHandles.Copy(csaObjHandles);
  g_pdlgVR->m_dlgObjects.m_csaObjActLayers.Copy(csaObjActLayers);
  g_pdlgVR->m_dlgObjects.m_csaObjStdLayers.Copy(csaObjStdLayers);
  g_pdlgVR->m_dlgObjects.m_csaErrorNos.Copy(csaObjErrorNos);
  g_pdlgVR->m_dlgObjects.m_csaErrorHandles.Copy(csaObjErrorHandles);

  // Copy the non-standard blocks
  g_pdlgVR->m_dlgBlocks.m_csaNSBNames.Copy(csaNSBNames);
  g_pdlgVR->m_dlgBlocks.m_csaNSBLayers.Copy(csaNSBLayers);
  g_pdlgVR->m_dlgBlocks.m_csaNSBHandles.Copy(csaNSBHandles);
  g_pdlgVR->m_dlgBlocks.m_csaErrorNos.Copy(csaNSBErrorNos);

  // Copy the unfilled attributes
  g_pdlgVR->m_dlgAttribs.m_csaMANames.Copy(csaMANames);
  g_pdlgVR->m_dlgAttribs.m_csaMALayers.Copy(csaMALayers);
  g_pdlgVR->m_dlgAttribs.m_csaMAAttribs.Copy(csaMAAttribs);
  g_pdlgVR->m_dlgAttribs.m_csaMAHandles.Copy(csaMAHandles);
  g_pdlgVR->m_dlgAttribs.m_csaErrorNos.Copy(csaMAErrorNos);

  // Copy the objects on layer 0
  g_pdlgVR->m_dlgLayer0.m_csa0Names.Copy(csa0Names);
  g_pdlgVR->m_dlgLayer0.m_csa0Handles.Copy(csa0Handles);
  g_pdlgVR->m_dlgLayer0.m_csaErrorNos.Copy(csa0ErrorNos);
  g_pdlgVR->m_dlgLayer0.m_csaErrHandles.Copy(csa0EntHandles);

  // Copy the moved assets
  g_pdlgVR->m_dlgMoved.m_csaMovNames.Copy(csaMovNames);
  g_pdlgVR->m_dlgMoved.m_csaMovLayers.Copy(csaMovLayers);
  g_pdlgVR->m_dlgMoved.m_csaMovType.Copy(csaMovType);
  g_pdlgVR->m_dlgMoved.m_csaMovHandles.Copy(csaMovHandles);
  g_pdlgVR->m_dlgMoved.m_csaErrorNos.Copy(csaMovErrorNos);
  
  // Display the dialog
  g_pdlgVR->Create(IDD_VALIDATE_RESULTS, NULL);
  g_pdlgVR->ShowWindow(SW_SHOW);
}
