////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : DXFIn.cpp
// Created          : 18th January 2008
// Created by       : S. Jaisimha
// Description      : Allows the user to import a DXF file into the current drawing and runs the post processor on it
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "SelectDXFDlg.h"
#include "io.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables
BOOL g_bIsDXF, g_bIntoCurrent, g_bUseTemplate, g_bRunOverkill;
double  g_dDXFScale;
CString g_csDXFPath, g_csDXFScale;
CStringArray g_csaBlocks, g_csaLayers, g_csaTextStyles, g_csaLineStyles;
AcDbObjectIdArray g_objsBeforeTranslate;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// External (not declared in StdAfx.h since it is used only here)
extern CString g_csTranslateScr; // ReadINI.cpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// External functions
extern void SnapLineEndPoints(); // JoinSegments.cpp

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Unused_ChangeLayerSettings
// Called from  : Command_DXFIn
// Description  : ****** THIS FUNCTION IS NO LONGER USED ******
//                Reads the "tblLayers" table and changes the layer settings accordingly
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Unused_ChangeLayerSettings(bool bShowMessage)
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

  // Loop through the layers in the current drawing
  int iIndex = 0;
  bool bMatched = false;
  ACHAR *pLayerName;
  AcDbLayerTable *pLayerTbl; acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForWrite);
  AcDbLayerTableIterator *pLayerIterator; pLayerTbl->newIterator(pLayerIterator);
  AcDbLayerTableRecord *pLayerTblRecord;
  for (; !pLayerIterator->done(); pLayerIterator->step())
  {  
    // Get the layer name
    pLayerIterator->getRecord(pLayerTblRecord, AcDb::kForWrite);
    pLayerTblRecord->getName(pLayerName);

    // If it matches a record in the configuration
    if (bShowMessage) acutPrintf(_T("  Updating layer %s ... "), pLayerName);
    if ((iIndex = MatchLayerName(pLayerName, csaPrefix, csaMid1, csaMid2, csaSuffix)) > -1)
    {
      // Change the properties accordingly
      AcCmColor cmColor; cmColor.setColorIndex(_tstoi(csaColors.GetAt(iIndex)));
      AcDbObjectId objLType = GetLineTypeObjectId(csaLTypes.GetAt(iIndex));
      
      pLayerTblRecord->setColor(cmColor);
      if (objLType.isValid()) pLayerTblRecord->setLinetypeObjectId(objLType);
      pLayerTblRecord->setLineWeight(AcDb::LineWeight(int(_tstof(csaWeights.GetAt(iIndex)) * 100.0))); // Since line weights are defined as 1 for 0.01 and 200 for 2.00
      if (bShowMessage) acutPrintf(_T("done.\n"), pLayerName);
    }
    else if (bShowMessage) acutPrintf(_T("ignored.\n"), pLayerName);

    // Close the layer
    pLayerTblRecord->close();
  }

  // Delete the layer iterator and close the Layer Table pointer
  delete pLayerIterator;
  pLayerTbl->close();

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : CheckForNonStdLayers
// Description  : Checks if there are any layers in the DXF that are not in the template and if found, prefixes those layers with "XT_"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CheckForNonStdLayers()
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
  pCurLayerTbl->close();

  bool bFound = false;
  ACHAR *pCurLayerName, *pStdLayerName;
  CString csCurLayer, csStdLayer, csNewCurLayer;
  AcDbLayerTableRecord *pCurLayerTblRecord, *pStdLayerTblRecord;

  // Loop through the layers in the current drawing
  for (; !pCurLayerIter->done(); pCurLayerIter->step())
  {  
    // Get the current layer name
    if (pCurLayerIter->getRecord(pCurLayerTblRecord, AcDb::kForWrite) != Acad::eOk) continue;
    pCurLayerTblRecord->getName(pCurLayerName);
    csCurLayer = pCurLayerName;

    // Reset the flag
    bFound = false;

    // Check if there is a corresponding layer in the standards
    for (pStdLayerIter->start(); !pStdLayerIter->done(); pStdLayerIter->step())
    {
      // Get the standard layer name
      if (pStdLayerIter->getRecord(pStdLayerTblRecord, AcDb::kForRead) != Acad::eOk) continue;
      pStdLayerTblRecord->getName(pStdLayerName);
      pStdLayerTblRecord->close();
      csStdLayer = pStdLayerName;

      // If the layer name is same, set the flag and stop the loop
      if (csCurLayer.CompareNoCase(csStdLayer) == 0) 
      { 
        bFound = true;
        break;
      }
    }

    // If the layer is not found and the current layer is not starting with "XT_"
    if ((bFound == false) && (csCurLayer.Mid(0, 3) != _T("XT_")))
    {
      // Prefix the layer name with "XT_"
      csNewCurLayer.Format(_T("XT_%s"), csCurLayer);
      pCurLayerTblRecord->setName(csNewCurLayer);
      acutPrintf(_T("    Layer %s renamed to %s\n"), csCurLayer, csNewCurLayer);
    }

    // Close the record
    pCurLayerTblRecord->close();
  }

  // Delete the standards drawing pointer
  delete pStdDb;

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ReplaceBlocks
// Argument     : 1. Entity name of the last entity before importing the DXF, as ads_name (WARNING: This can be NULL)
//                2. New scale for the blocks, as double
//
// Called from  : Command_DXFIn
// Description  : Replaces all blocks in the imported DXF with the corresponding block in the template, re-scaled to the selected scale
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL ReplaceBlocks(ads_name enBeforeInsert, double dScale)
{
  bool bIsInsert = false;
  //long lInserts = 0L;
  int lInserts = 0L;
  double dRot = 0.0;
  CString csLayer, csProgress;
  ads_name ssInserts, enInsert;
  AcGePoint3d gePos;
  AcDbObjectId objId;
  AcDbEntity *pEnt = NULL;
  AcDbBlockReference *pRef = NULL;
  CStringArray csaUpdatedBlocks, csaNotUpdatedBlocks, csaProcessedBlocks;
  struct resbuf *rbpFilter = NULL;

  // Set the "PROXYNOTICE" & "ATTREQ" system variables to OFF
  CString csProxyNotice = _T("PROXYNOTICE"), csAttReq = _T("ATTREQ");
  struct resbuf rbProxyNotice, rbAttReq;
  acedGetVar(csProxyNotice, &rbProxyNotice);
  int iOldProxyNotice = rbProxyNotice.resval.rint;
  rbProxyNotice.resval.rint = 0;
  acedSetVar(csProxyNotice, &rbProxyNotice);

  acedGetVar(csAttReq, &rbAttReq);
  int iOldAttReq = rbAttReq.resval.rint;
  rbAttReq.resval.rint = 0;
  acedSetVar(csAttReq, &rbAttReq);

  // Copy the entity name
  ads_name enNext = { enBeforeInsert[0], enBeforeInsert[1] };

  // Loop through the database starting from the last entity before import
  while (acdbEntNext(enNext, enNext) == RTNORM)
  {
    // Check if this is a block reference
    acdbGetObjectId(objId, enNext);
    acdbOpenObject(pEnt, objId, AcDb::kForRead);
    bIsInsert = (AcDbBlockReference::cast(pEnt) ? true : false);
    pEnt->close();

    // If it is
    if (bIsInsert)
    {
      // Get the block name from the reference object id
      ads_name enRef; acdbGetAdsName(enRef, objId);
      struct resbuf *rbpRef = acdbEntGet(enRef);
      CString csBlockName = Assoc(rbpRef, 2)->resval.rstring;

      // If this block has not already been updated
      if (CheckForDuplication(csBlockName, csaProcessedBlocks) == FALSE) 
      {
        // If this symbol name is in the array, we need not update it
        acutPrintf(_T("\n  Updating block [%s]..."), csBlockName);
        if (CheckForDuplication(csBlockName, g_csaBlocks) == FALSE)
        {
          // Get its definition from the standards drawing
          int iRet = UpdateBlockFromStandards(csBlockName);

          // If this block was not updated, add the block name to the other array
          if (iRet == 0) csaNotUpdatedBlocks.Add(csBlockName);

          // If this block was updated, add the block name to the array
          if (iRet <  2) csaUpdatedBlocks.Add(csBlockName);
        }
        // We just say already present
        else 
        {
          acutPrintf(_T(" already present."));
          csaUpdatedBlocks.Add(csBlockName);
        }

        // Add the block name to the processed array
        csaProcessedBlocks.Add(csBlockName);
      }
    }
  }

  // Open the model space block table record for writing
  acutPrintf(_T("\nRe-inserting updated blocks...\n"));

  // For each updated block in the array
  BOOL bIsNotUpdated = FALSE;
  for (int iCtr = 0; iCtr < csaUpdatedBlocks.GetSize(); iCtr++)
  {
    // Make a selection set of references to this block in Model Space only
    rbpFilter = acutBuildList(RTDXF0, _T("INSERT"), 2, csaUpdatedBlocks.GetAt(iCtr), -4, _T("<NOT"), 67, 1, -4, _T("NOT>"), NULL);
    if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssInserts) != RTNORM) { acutRelRb(rbpFilter); continue; }
    if ((acedSSLength(ssInserts, &lInserts) != RTNORM) || (lInserts == 0L)) { acedSSFree(ssInserts); acutRelRb(rbpFilter); continue; }
    acutRelRb(rbpFilter);

    // Set the progress bar status
    csProgress.Format(_T("[%ld of %ld] Updating %s (%ld found)"), iCtr + 1, csaUpdatedBlocks.GetSize(), csaUpdatedBlocks.GetAt(iCtr), lInserts);
    acutPrintf(_T("%s\n"), csProgress);
    acedSetStatusBarProgressMeter(csProgress, 1, int(lInserts));

    // Check if this block is in the not updated array and set the flag
    bIsNotUpdated = CheckForDuplication(csaUpdatedBlocks.GetAt(iCtr), csaNotUpdatedBlocks);

    // For each reference in the selection set
    double dActScale = 0.0, dBlockScale = 0.0;
    for (long lCtr = 0L; lCtr < lInserts; lCtr++)
    {
      acedSSName(ssInserts, lCtr, enInsert);
      acdbGetObjectId(objId, enInsert);

      // If the object was present before translation
      if (g_objsBeforeTranslate.contains(objId)) 
      {
        // Update the progress bar
        acedSetStatusBarProgressMeterPos(int(lCtr + 1));

        // Loop back
        continue;
      }

      // If this block is not updated, scale will be current scale multiplied by the translation scale
      if (bIsNotUpdated == TRUE) dActScale = dScale * dBlockScale;
      // Otherwise, use the translation scale
      else dActScale = dScale;

      // Open the block for writing
      if (acdbOpenObject(pRef, objId, AcDb::kForWrite) == Acad::eOk)
      {
        //csLayer = pRef->layer();
        //gePos = pRef->position();
        //dRot = pRef->rotation();
        //dBlockScale = pRef->scaleFactors().sx; // Assumes an equally scaled block
        //pRef->erase();

        // Update the scale factors of the block and close it
        Acad::ErrorStatus es = pRef->setScaleFactors(AcGeScale3d(dActScale, dActScale, dActScale));
        pRef->close();
      }

      /*
      // Re-insert the new definition and change its layer (the pure ObjectARX method doesn't seem to work, it still inserts the old definition only)
      acedCommand(RTSTR, _T(".INSERT"), RTSTR, csaUpdatedBlocks.GetAt(iCtr), RTSTR, _T("S"), RTREAL, dActScale, RTPOINT, asDblArray(gePos), RTREAL, RTD(dRot), NULL);
      acdbEntLast(enInsert);
      acedCommand(RTSTR, _T(".CHPROP"), RTENAME, enInsert, RTSTR, _T(""), RTSTR, _T("LA"), RTSTR, csLayer, RTSTR, _T(""), NULL);
      */

      // Update the progress bar
      acedSetStatusBarProgressMeterPos(int(lCtr + 1));
    }

    // Free the selection set
    acedSSFree(ssInserts);

    // Restore the status bar
    acedRestoreStatusBar();
  }

  // Reset the "PROXYNOTICE" & "ATTREQ" system variable
  rbProxyNotice.resval.rint = iOldProxyNotice; acedSetVar(csProxyNotice, &rbProxyNotice);
  rbAttReq.resval.rint      = iOldAttReq;      acedSetVar(csAttReq,      &rbAttReq);

  // Send all blocks to the "Back" of the drawing
  if (acedSSGet(_T("X"), NULL, NULL, acutBuildList(RTDXF0, _T("INSERT"), NULL), ssInserts) == RTNORM) 
  {
    acedCommandS(RTSTR, _T(".DRAWORDER"), RTPICKS, ssInserts, RTSTR, _T(""), RTSTR, _T("Back"), NULL);
    acedSSFree(ssInserts);
  }

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : StoreSpatialPositions
// Argument     : Entity name of the last entity before importing the DXF, as ads_name (WARNING: This can be NULL)
// Called from  : Command_DXFIn
// Description  : Attaches the current position as xdata to all handled entity types.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL StoreSpatialPositions(ads_name enBeforeInsert)
{
  AcDbObjectId objId;
  AcDbEntity *pEnt;
  AcDbArc *pArc;
  AcDbText *pText;
  AcDbLine *pLine;
  AcDbMline *pMLine;
  AcDbCircle *pCircle;
  AcDbPolyline *pPline;
  AcDb2dPolyline *pPline2d;
  AcDbBlockReference *pRef;
  struct resbuf *rbpXData = NULL;
  CString csXData = XDATA_Translate;

  // Copy the entity name
  ads_name enNext = { enBeforeInsert[0], enBeforeInsert[1] };

  // Loop through the database starting from the last entity before import
  while (acdbEntNext(enNext, enNext) == RTNORM)
  {
    // Open the entity for reading
    acdbGetObjectId(objId, enNext);
    acdbOpenObject(pEnt, objId, AcDb::kForRead);

    // Get the common properties
    CString csLayer = pEnt->layer();
    Adesk::UInt16 auiColor = pEnt->colorIndex();
    
    // If this is an ARC
    if (pArc = AcDbArc::cast(pEnt))
    {
      AcGePoint3d geCen = pArc->center();
      double dRad       = pArc->radius();
      double dSAng      = pArc->startAngle();
      double dEAng      = pArc->endAngle();
      rbpXData = acutBuildList(AcDb::kDxfRegAppName, csXData, AcDb::kDxfXdAsciiString, csLayer, AcDb::kDxfXdInteger16, auiColor, 
                               AcDb::kDxfXdXCoord, asDblArray(geCen), AcDb::kDxfXdReal, dRad, AcDb::kDxfXdReal, dSAng, AcDb::kDxfXdReal, dEAng, 
                               NULL
                              );
    }
    // If this is a CIRCLE
    else if (pCircle = AcDbCircle::cast(pEnt))
    {
      AcGePoint3d geCen = pCircle->center();
      double dRad       = pCircle->radius();
      rbpXData = acutBuildList(AcDb::kDxfRegAppName, csXData, AcDb::kDxfXdAsciiString, csLayer, AcDb::kDxfXdInteger16, auiColor, 
                               AcDb::kDxfXdXCoord, asDblArray(geCen), AcDb::kDxfXdReal, dRad, 
                               NULL
                              );
    }
    // If this is a TEXT
    else if (pText = AcDbText::cast(pEnt))
    {
      AcGePoint3d gePos = pText->alignmentPoint();
      double dHeight    = pText->height();
      CString csText    = pText->textString();
      if ((gePos.x == 0.0) && (gePos.y == 0.0)) gePos = pText->position();
      rbpXData = acutBuildList(AcDb::kDxfRegAppName, csXData, AcDb::kDxfXdAsciiString, csLayer, AcDb::kDxfXdInteger16, auiColor, 
                               AcDb::kDxfXdXCoord, asDblArray(gePos), AcDb::kDxfXdReal, dHeight, AcDb::kDxfXdAsciiString, csText, 
                               NULL
                              );
    }
    // If this is a LINE
    else if (pLine = AcDbLine::cast(pEnt))
    {
      AcGePoint3d geStart = pLine->startPoint();
      AcGePoint3d geEnd   = pLine->endPoint();
      rbpXData = acutBuildList(AcDb::kDxfRegAppName, csXData, AcDb::kDxfXdAsciiString, csLayer, AcDb::kDxfXdInteger16, auiColor, 
                               AcDb::kDxfXdXCoord, asDblArray(geStart), AcDb::kDxfXdXCoord, asDblArray(geEnd), 
                               NULL
                              );
    }
    // If this is a MLINE
    else if (pMLine = AcDbMline::cast(pEnt))
    {
      int iNumVerts       = pMLine->numVertices();
      AcGePoint3d geVert1 = pMLine->vertexAt(0);
      AcGePoint3d geVert2 = pMLine->vertexAt(iNumVerts - 1);
      rbpXData = acutBuildList(AcDb::kDxfRegAppName, csXData, AcDb::kDxfXdAsciiString, csLayer, AcDb::kDxfXdInteger16, auiColor, 
                               AcDb::kDxfXdXCoord, asDblArray(geVert1), AcDb::kDxfXdInteger16, iNumVerts, AcDb::kDxfXdXCoord, asDblArray(geVert2),
                               NULL
                              );
    }
    // If this is a LWPOLYLINE
    else if (pPline = AcDbPolyline::cast(pEnt))
    {
      int iNumVerts       = pPline->numVerts();
      AcGePoint3d geVert1; pPline->getPointAt(0, geVert1);
      AcGePoint3d geVert2; pPline->getPointAt(iNumVerts - 1, geVert2);
      rbpXData = acutBuildList(AcDb::kDxfRegAppName, csXData, AcDb::kDxfXdAsciiString, csLayer, AcDb::kDxfXdInteger16, auiColor, 
                               AcDb::kDxfXdXCoord, asDblArray(geVert1), AcDb::kDxfXdInteger16, iNumVerts, AcDb::kDxfXdXCoord, asDblArray(geVert2),
                               NULL
                              );
    }
    // If this is a POLYLINE (old style)
    else if (pPline2d = AcDb2dPolyline::cast(pEnt))
    {
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

      rbpXData = acutBuildList(AcDb::kDxfRegAppName, csXData, AcDb::kDxfXdAsciiString, csLayer, AcDb::kDxfXdInteger16, auiColor, 
                               AcDb::kDxfXdXCoord, asDblArray(geVert1), AcDb::kDxfXdInteger16, iNumVerts, AcDb::kDxfXdXCoord, asDblArray(geVert2),
                               NULL
                              );
    }
    // If this is an INSERT
    else if (pRef = AcDbBlockReference::cast(pEnt))
    {
      AcGePoint3d gePos   = pRef->position();
      AcGeScale3d geScale = pRef->scaleFactors();
      double dRot         = pRef->rotation();
      rbpXData = acutBuildList(AcDb::kDxfRegAppName, csXData, AcDb::kDxfXdAsciiString, csLayer, AcDb::kDxfXdInteger16, auiColor, 
                               AcDb::kDxfXdXCoord, asDblArray(gePos), AcDb::kDxfXdXCoord, asDblArray(AcGePoint3d(geScale.sx, geScale.sy, geScale.sz)), AcDb::kDxfXdReal, dRot,
                               NULL
                              );
    }

    // Close the entity
    pEnt->close();

    // Add the xdata to the entity
    if (rbpXData)
    {
      addXDataToEntity(enNext, rbpXData);
      acutRelRb(rbpXData);
      rbpXData = NULL;
    }
    // In case we encounter an un-handled entity
    else 
    {
      CString csEntType = Assoc(acdbEntGet(enNext), 0)->resval.rstring;
      if ((csEntType != _T("VERTEX")) && (csEntType != _T("SEQEND"))) acutPrintf(_T("Unhandled entity %s\n"), csEntType);
    }
  }

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : PurgeUnusedLayers
// Argument     : None
// Called from  : Command_DXFIn
// Description  : Purges unused layers in the drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PurgeUnusedLayers()
{
  Acad::ErrorStatus es;

  // Open the layer table in the current drawing
  AcDbLayerTable *pLT;
  if ((es = acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLT, AcDb::kForRead)) != Acad::eOk) return;

  // Create a new iterator
  AcDbLayerTableIterator *pLTIter;
  if ((es = pLT->newIterator(pLTIter)) != Acad::eOk) { pLT->close(); return; }
  pLT->close();

  // Iterate through the all layers and collect their ids
  AcDbObjectId objLayer;
  AcDbObjectIdArray oaLayers;
  for (pLTIter->start(); !pLTIter->done(); pLTIter->step()) 
  {
    if ((es = pLTIter->getRecordId (objLayer)) != Acad::eOk) continue;
    oaLayers.append(objLayer);
  }
  delete pLTIter;

  // This will remove ids with have hard references on them and what remains are the ids of the unused layers
  if (acdbHostApplicationServices()->workingDatabase()->purge(oaLayers) != Acad::eOk ) return;

  // Open each remaining object in the array and erase it
  ACHAR *pName = NULL;
  CString csName;
  AcDbLayerTableRecord *pLayer;
  for (int iCtr = 0; iCtr < oaLayers.length(); iCtr++)
  {
    objLayer = oaLayers.at(iCtr);
    if (acdbOpenObject(pLayer, objLayer, AcDb::kForWrite) != Acad::eOk) continue;

    // Retain "VALIDATION_ERROR" layer
    pLayer->getName(pName); csName = pName;
    if (csName.CompareNoCase(_T("VALIDATION_ERROR")) != 0) pLayer->erase();
    pLayer->close();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : PurgeUnusedBlocks
// Argument     : None
// Called from  : Command_DXFIn
// Description  : Purges unused blocks in the drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PurgeUnusedBlocks()
{
  Acad::ErrorStatus es;

  // Open the Block table in the current drawing
  AcDbBlockTable *pLT;
  if ((es = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pLT, AcDb::kForRead)) != Acad::eOk) return;

  // Create a new iterator
  AcDbBlockTableIterator *pLTIter;
  if ((es = pLT->newIterator(pLTIter)) != Acad::eOk) { pLT->close(); return; }
  pLT->close();

  // Iterate through the all Blocks and collect their ids
  AcDbObjectId objBlock;
  AcDbObjectIdArray oaBlocks;
  for (pLTIter->start(); !pLTIter->done(); pLTIter->step()) 
  {
    if ((es = pLTIter->getRecordId (objBlock)) != Acad::eOk) continue;
    oaBlocks.append(objBlock);
  }
  delete pLTIter;

  // This will remove ids with have hard references on them and what remains are the ids of the unused Blocks
  if (acdbHostApplicationServices()->workingDatabase()->purge(oaBlocks) != Acad::eOk ) return;

  // Open each remaining object in the array and erase it
  ACHAR *pName = NULL;
  CString csName;
  AcDbBlockTableRecord *pBlock;
  for (int iCtr = 0; iCtr < oaBlocks.length(); iCtr++)
  {
    objBlock = oaBlocks.at(iCtr);
    if (acdbOpenObject(pBlock, objBlock, AcDb::kForWrite) != Acad::eOk) continue;

    // Retain "VALIDATION_ERROR" Block
    pBlock->getName(pName); csName = pName;
    if (csName.CompareNoCase(_T("VALIDATION_ERROR")) != 0) pBlock->erase();
    pBlock->close();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : PurgeUnusedTextStyles
// Argument     : None
// Called from  : Command_DXFIn
// Description  : Purges unused text styles in the drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PurgeUnusedTextStyles()
{
  Acad::ErrorStatus es;

  // Open the Text Style table in the current drawing
  AcDbTextStyleTable *pLT;
  if ((es = acdbHostApplicationServices()->workingDatabase()->getTextStyleTable(pLT, AcDb::kForRead)) != Acad::eOk) return;

  // Create a new iterator
  AcDbTextStyleTableIterator *pLTIter;
  if ((es = pLT->newIterator(pLTIter)) != Acad::eOk) { pLT->close(); return; }
  pLT->close();

  // Iterate through the all Text Styles and collect their ids
  AcDbObjectId objTextStyle;
  AcDbObjectIdArray oaTextStyles;
  for (pLTIter->start(); !pLTIter->done(); pLTIter->step()) 
  {
    if ((es = pLTIter->getRecordId (objTextStyle)) != Acad::eOk) continue;
    oaTextStyles.append(objTextStyle);
  }
  delete pLTIter;

  // This will remove ids with have hard references on them and what remains are the ids of the unused Text Styles
  if (acdbHostApplicationServices()->workingDatabase()->purge(oaTextStyles) != Acad::eOk ) return;

  // Open each remaining object in the array and erase it
  AcDbTextStyleTableRecord *pTextStyle;
  for (int iCtr = 0; iCtr < oaTextStyles.length(); iCtr++)
  {
    objTextStyle = oaTextStyles.at(iCtr);
    if (acdbOpenObject(pTextStyle, objTextStyle, AcDb::kForWrite) != Acad::eOk) continue;
    pTextStyle->erase();
    pTextStyle->close();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : PurgeUnusedLineTypes
// Argument     : None
// Called from  : Command_DXFIn
// Description  : Purges unused line types in the drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PurgeUnusedLineTypes()
{
  Acad::ErrorStatus es;

  // Open the Line type table in the current drawing
  AcDbLinetypeTable *pLT;
  if ((es = acdbHostApplicationServices()->workingDatabase()->getLinetypeTable(pLT, AcDb::kForRead)) != Acad::eOk) return;

  // Create a new iterator
  AcDbLinetypeTableIterator *pLTIter;
  if ((es = pLT->newIterator(pLTIter)) != Acad::eOk) { pLT->close(); return; }
  pLT->close();

  // Iterate through the all Line types and collect their ids
  AcDbObjectId objLinetype;
  AcDbObjectIdArray oaLinetypes;
  for (pLTIter->start(); !pLTIter->done(); pLTIter->step()) 
  {
    if ((es = pLTIter->getRecordId (objLinetype)) != Acad::eOk) continue;
    oaLinetypes.append(objLinetype);
  }
  delete pLTIter;

  // This will remove ids with have hard references on them and what remains are the ids of the unused Line types
  if (acdbHostApplicationServices()->workingDatabase()->purge(oaLinetypes) != Acad::eOk ) return;

  // Open each remaining object in the array and erase it
  AcDbLinetypeTableRecord *pLinetype;
  for (int iCtr = 0; iCtr < oaLinetypes.length(); iCtr++)
  {
    objLinetype = oaLinetypes.at(iCtr);
    if (acdbOpenObject(pLinetype, objLinetype, AcDb::kForWrite) != Acad::eOk) continue;
    pLinetype->erase();
    pLinetype->close();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : StoreCurrentDrawingDetails
// Argument     : None
// Called from  : Command_ContinueDXFIn
// Description  : Store the blocks, layers, text styles and linetype styles in the current drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StoreCurrentDrawingDetails()
{
  Acad::ErrorStatus es;

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Collect blocks in the current drawing
  ACHAR *pszBlkName;
  AcDbBlockTable *pBT;
  AcDbBlockTableIterator *pBTIter;
  AcDbBlockTableRecord *pBTR;
  if ((es = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBT, AcDb::kForRead)) != Acad::eOk) return;
  if ((es = pBT->newIterator(pBTIter)) != Acad::eOk) { pBT->close(); return; }
  pBT->close();
  for (pBTIter->start(); !pBTIter->done(); pBTIter->step()) 
  {
    if ((es = pBTIter->getRecord(pBTR, AcDb::kForRead)) != Acad::eOk) continue;
    pBTR->getName(pszBlkName);
    pBTR->close();
    if (pszBlkName[0] != '*') g_csaBlocks.Add(pszBlkName); // Unnamed blocks can be ignored
  }
  delete pBTIter;
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Collect layers in the current drawing
  ACHAR *pszLayerName;
  AcDbLayerTable *pLT;
  AcDbLayerTableIterator *pLTIter;
  AcDbLayerTableRecord *pLTR;
  if ((es = acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLT, AcDb::kForRead)) != Acad::eOk) return;
  if ((es = pLT->newIterator(pLTIter)) != Acad::eOk) { pLT->close(); return; }
  pLT->close();
  for (pLTIter->start(); !pLTIter->done(); pLTIter->step()) 
  {
    if ((es = pLTIter->getRecord(pLTR, AcDb::kForRead)) != Acad::eOk) continue;
    pLTR->getName(pszLayerName);
    pLTR->close();
    g_csaLayers.Add(pszLayerName);
  }
  delete pLTIter;
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Collect text styles in the current drawing
  ACHAR *pszTextStyleName;
  AcDbTextStyleTable *pTST;
  AcDbTextStyleTableIterator *pTSTIter;
  AcDbTextStyleTableRecord *pTSTR;
  if ((es = acdbHostApplicationServices()->workingDatabase()->getTextStyleTable(pTST, AcDb::kForRead)) != Acad::eOk) return;
  if ((es = pTST->newIterator(pTSTIter)) != Acad::eOk) { pTST->close(); return; }
  pTST->close();
  for (pTSTIter->start(); !pTSTIter->done(); pTSTIter->step()) 
  {
    if ((es = pTSTIter->getRecord(pTSTR, AcDb::kForRead)) != Acad::eOk) continue;
    pTSTR->getName(pszTextStyleName);
    pTSTR->close();
    g_csaTextStyles.Add(pszTextStyleName);
  }
  delete pTSTIter;
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Collect line styles in the current drawing
  ACHAR *pszLineStyleName;
  AcDbLinetypeTable *pLTT;
  AcDbLinetypeTableIterator *pLTTIter;
  AcDbLinetypeTableRecord *pLTTR;
  if ((es = acdbHostApplicationServices()->workingDatabase()->getLinetypeTable(pLTT, AcDb::kForRead)) != Acad::eOk) return;
  if ((es = pLTT->newIterator(pLTTIter)) != Acad::eOk) { pLTT->close(); return; }
  pLTT->close();
  for (pLTTIter->start(); !pLTTIter->done(); pLTTIter->step()) 
  {
    if ((es = pLTTIter->getRecord(pLTTR, AcDb::kForRead)) != Acad::eOk) continue;
    pLTTR->getName(pszLineStyleName);
    pLTTR->close();
    g_csaLineStyles.Add(pszLineStyleName);
  }
  delete pLTTIter;
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Collect all entities in the current drawing
  ads_name ssCurrent;
  if (acedSSGet(_T("X"), NULL, NULL, NULL, ssCurrent) == RTNORM) 
  {
    //long lCurrent = 0L;
	int lCurrent = 0L;
    ads_name enCurrent;
    AcDbObjectId objCurrent;
    acedSSLength(ssCurrent, &lCurrent);
    for (long lCtr = 0L; lCtr < lCurrent; lCtr++)
    {
      acedSSName(ssCurrent, lCtr, enCurrent);
      acdbGetObjectId(objCurrent, enCurrent);
      g_objsBeforeTranslate.append(objCurrent);
    }
    acedSSFree(ssCurrent);
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Zoom to a blank area in the drawing as this will GREATLY speed up the translation process
  struct resbuf rbExtMax; acedGetVar(_T("EXTMAX"), &rbExtMax);
  ads_point ptMin = { rbExtMax.resval.rpoint[X] + 1000.0, rbExtMax.resval.rpoint[Y] + 1000.0, 0.0 };
  ads_point ptMax = { rbExtMax.resval.rpoint[X] + 1001.0, rbExtMax.resval.rpoint[Y] + 1001.0, 0.0 };
  acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("W"), RTPOINT, ptMin, RTPOINT, ptMax, NULL);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_ContinueDXFIn
// Called from  : Command_DXFIn, via the "EA_CONTINUE_TRANSLATE" command
// Description  : Imports DXF into current drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_ContinueDXFIn()
{
  // If the path is empty, show a dummy unknown command to the user
  if (g_csDXFPath.IsEmpty()) { acutPrintf(_T("Unknown command \"EA_CONTINUE_TRANSLATE\".  Press F1 for help.\n")); return; }

  // Set a few variables
  struct resbuf rbVar;
  rbVar.restype = RTSHORT;
  rbVar.resval.rint = 0; acedSetVar(_T("CMDECHO"),  &rbVar);  // Command echo OFF
  rbVar.resval.rint = 1; acedSetVar(_T("PLINEGEN"), &rbVar);  // Polyline linetype generation ON
  rbVar.resval.rint = 1; acedSetVar(_T("TILEMODE"), &rbVar);  // Model space ON

  // Clear the global arrays
  g_csaBlocks.RemoveAll();
  g_csaLayers.RemoveAll(); 
  g_csaTextStyles.RemoveAll();
  g_csaLineStyles.RemoveAll();

  // If we are translating into the current drawing, store relevant details
  if (g_bIntoCurrent == TRUE) StoreCurrentDrawingDetails();

  // Import the specified DXF or read the specified drawing into a temporary database
  CString csError;
  AcDbDatabase *pImportDb = new AcDbDatabase(false, true);
  Acad::ErrorStatus es = (g_bIsDXF ? pImportDb->dxfIn(g_csDXFPath) : pImportDb->readDwgFile(g_csDXFPath));
  if (es != Acad::eOk) 
  { 
    csError.Format(_T("Error importing file\n%s\n\n%s"), g_csDXFPath, acadErrorStatusText(es));
    g_csDXFPath.Empty();
    appMessage(csError, MB_ICONSTOP); 
    delete pImportDb; 
    return; 
  }

  // Insert the temporary database in the current drawing
  AcDbObjectId objImport;
  if ((es = acdbHostApplicationServices()->workingDatabase()->insert(objImport, _T("DXF_IMPORT"), pImportDb)) != Acad::eOk)
  {
    csError.Format(_T("Error inserting temporary database into current drawing.\n\n%s"), acadErrorStatusText(es));
    g_csDXFPath.Empty();
    appMessage(csError, MB_ICONSTOP); 
    delete pImportDb; 
    return; 
  }

  // Delete the temporary DXF database
  delete pImportDb;

  // Empty the path, so that the command will not be executed accidentally
  g_csDXFPath.Empty();

  // Create a block reference to the DXF and explode it without adding it to the database
  AcDbVoidPtrArray ptrEnts;
  AcDbBlockReference *pDXFIns = new AcDbBlockReference;
  pDXFIns->setBlockTableRecord(objImport);
  pDXFIns->setScaleFactors(AcGeScale3d(1.0, 1.0, 1.0));
  pDXFIns->setPosition(AcGePoint3d(0, 0, 0));
  pDXFIns->setRotation(0.0);
  pDXFIns->explode(ptrEnts);
  pDXFIns->close();
  delete pDXFIns;

  // Create a point and store it as the current last entity in the database
  acedCommandS(RTSTR, _T(".POINT"), RTSTR, _T("-100000,-100000,0"), NULL);
  ads_name enBeforeImport; acdbEntLast(enBeforeImport);

  // Open the block table record of the current drawing
  AcDbBlockTable *pBT; acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBT, AcDb::kForRead); 
  AcDbBlockTableRecord *pBTR; pBT->getAt(ACDB_MODEL_SPACE, pBTR, AcDb::kForWrite); 
  pBT->close(); 

  // For each entity in the array
  AcDbEntity *pDXFEnt = NULL;
  for (long lCtr = 0L; lCtr < ptrEnts.length(); lCtr++)
  {
    // Get the entity from the array and add it to the database
    pDXFEnt = (AcDbEntity *)ptrEnts.at(lCtr);
    pBTR->appendAcDbEntity(pDXFEnt); 
    pDXFEnt->close();
  }

  // Close the block table record
  pBTR->close();

  // Store the selected scale, for future use
  struct resbuf *rbpScale = acutBuildList(AcDb::kDxfReal, g_dDXFScale, AcDb::kDxfReal, _tstof(g_csDXFScale), NULL);
  addXRecordToDictionary(XDICT_ECapture, _T("DXF Scale"), rbpScale);
  acutRelRb(rbpScale);

  // Process the layer settings (no longer required, since all settings are done in the DWT file itself)
  //acutPrintf(_T("\nUpdating layers from standards drawing...\n"));
  //if (ChangeLayerSettings(true) == FALSE) { acdbEntDel(enBeforeImport); return; }

  // If we are using a template
  if (g_bUseTemplate == TRUE)
  {
    // Check for non-standard layer names
    acutPrintf(_T("\nChecking for non-standard layers...\n"));
    if (CheckForNonStdLayers() == FALSE) { acdbEntDel(enBeforeImport); return; }

    // Change block scales for all references after the previous last entity
    acutPrintf(_T("\nUpdating blocks from standards drawing...\n"));
    if (ReplaceBlocks(enBeforeImport, g_dDXFScale) == FALSE) { acdbEntDel(enBeforeImport); return; }
  }

  // If the connection gap specification in the INI is greater than 0.0
  if (_tstof(g_csMaxConnGap) > 0.0) 
  {
    // Join line end points with the connectivity limit into single polylines
    acutPrintf(_T("\nConverting connection lines into polylines...\n"));
    SnapLineEndPoints();
  }

  // Store spatial positions of all imported objects
  acutPrintf(_T("\nStoring spatial positions for later validation...\n"));
  if (StoreSpatialPositions(enBeforeImport) == FALSE) { acdbEntDel(enBeforeImport); return; }

  // Set USERI5 to 1423, so that NET_ADDRESSNODIALOG command will run
  struct resbuf rbUserI5;
  acedGetVar(_T("USERI5"), &rbUserI5);
  int iOldUserI5 = rbUserI5.resval.rint;
  rbUserI5.resval.rint = 1423;
  acedSetVar(_T("USERI5"), &rbUserI5);
  
  // Update the address and logo info, if they have been previously specified
  acutPrintf(_T("\nUpdating address and logo information in all sheets...\n"));
  acedCommandS(RTSTR, _T("NET_ADDRESSNODIALOG"), NULL);

  // Restore old value of USERI5
  rbUserI5.resval.rint = iOldUserI5;
  acedSetVar(_T("USERI5"), &rbUserI5);

  // Erase the temporary POINT object
  acdbEntDel(enBeforeImport);

  // If we must purge unused blocks (called thrice to ensure unused nested blocks are also purged)
  // We must do this first, otherwise unused layers will not be purged due to unused block definitions on them
  if (g_csPurgeBlocks == "1") { acutPrintf(_T("\nPurging unused blocks...\n")); PurgeUnusedBlocks(); PurgeUnusedBlocks(); PurgeUnusedBlocks(); }

  // If we must purge unused layers
  if (g_csPurgeLayers == "1") { acutPrintf(_T("\nPurging unused layers...\n")); PurgeUnusedLayers(); }

  // If we must purge unused text styles
  if (g_csPurgeText == "1")   { acutPrintf(_T("\nPurging unused text styles...\n")); PurgeUnusedTextStyles(); }

  // If we must purge unused layers
  if (g_csPurgeLTypes == "1") { acutPrintf(_T("\nPurging unused line types...\n")); PurgeUnusedLineTypes(); }

  // Switch on all layers
  acedCommandS(RTSTR, _T(".LAYER"), RTSTR, _T("T"), RTSTR, _T("*"), RTSTR, _T("ON"), RTSTR, _T("*"), RTSTR, _T(""), NULL);

  // If we have to run a script at the end of translation
  if (!g_csTranslateScr.IsEmpty()) 
  {
    CString csFinalScr = g_csTranslateScr;

    // If overkill is also required
    if (g_bRunOverkill)
    {
      // Add "WithOverkill" to the file name
      CString csTemp = csFinalScr;
      csTemp.MakeLower();
      csTemp.Replace(_T(".scr"), _T(""));
      csTemp += "WithOverkill.scr";
      csFinalScr = csTemp;
    }
    
    // Check if the file is present and run it
    if (_waccess(csFinalScr, 00) == -1) appMessage(csFinalScr + _T("\n\nUnable to find script file."), MB_ICONSTOP);
    else acedCommandS(RTSTR, _T(".SCRIPT"), RTSTR, csFinalScr, NULL);
  }
  // Otherwise, if we have to run the OVERKILL command, do so now
  else if (g_bRunOverkill == TRUE) 
  {
    CString csOverkillScr;
    csOverkillScr.Format(_T("%s\\Overkill.scr"), g_csHome);
    if (_waccess(csOverkillScr, 00) == -1) appMessage(csOverkillScr + _T("\n\nUnable to find script file."), MB_ICONSTOP);
    else acedCommandS(RTSTR, _T(".SCRIPT"), RTSTR, csOverkillScr, NULL);
   }

  // Ensure that everything is visible and save the drawing
  acedCommandS(RTSTR, _T(".ZOOM"), RTSTR, _T("E"), RTSTR, _T(".QSAVE"), NULL);

  // Success
  acutPrintf(_T("\nTranslation complete.\n"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_DXFIn
// Called from  : User, via the "EA_TRANSLATE" command
// Description  : Allows user to select a DXF to import and creates a new drawing in the same location with the standard template. 
//                After this, the new drawing and the selected DXF is imported into it.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_DXFIn()
{
  // Check if the template file is present
  if (_taccess(g_csLocalDWT, 00) == -1) { displayMessage(_T("%s\n\nLocal template file not found. Please close and open AutoCAD again."), g_csLocalDWT); return; }

  // Display the custom DXF file selection with scale dialog
  CString csFilter = _T("AutoCAD Drawing Interchange Files (*.dxf)|*.dxf|AutoCAD Drawing (*.dwg)|*.dwg||");
  CSelectDXFDlg dlgDXF(TRUE, _T(""), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, csFilter, NULL, FALSE);
  dlgDXF.m_csTemplate = g_csLocalDWT; // The configured template will be used by default
  if (dlgDXF.DoModal() == IDCANCEL) return;

  // Store the input file name and scale
  CString csDXFFile  = dlgDXF.GetFileTitle();
  CString csDXFPath  = dlgDXF.GetPathName();
  double dDXFScale   = _tstof(dlgDXF.m_csConversion);
  CString csDXFScale = dlgDXF.m_csScale;
  CString csFileExt  = csDXFPath.Right(3);

  // Store the global variables
  g_bIsDXF       = (csFileExt.CompareNoCase(_T("dxf")) ? FALSE : TRUE);
  g_dDXFScale    = dDXFScale;
  g_csDXFPath    = csDXFPath;
  g_csDXFScale   = csDXFScale;
  g_bUseTemplate = dlgDXF.m_bUseTemplate;
  g_bIntoCurrent = dlgDXF.m_bIntoCurrent;
  g_bRunOverkill = dlgDXF.m_bRunOverkill;

  // Clear the global object id array
  g_objsBeforeTranslate.removeAll();
  g_objsBeforeTranslate.setPhysicalLength(0);
  g_objsBeforeTranslate.setLogicalLength(0);

  // If we are not translating into the current drawing
  if (g_bIntoCurrent == FALSE)
  {
    // Copy the template drawing to the DXF folder in the same name as the DXF file
    CString csDXFFolder = csDXFPath; csDXFFolder.Replace(dlgDXF.GetFileName(), _T(""));
    CString csNewDwg; csNewDwg.Format(_T("%s%s%s.dwg"), csDXFFolder, dlgDXF.GetFileTitle(), (g_bIsDXF ? _T("") : _T("_Translated")));
    if (CopyFile(dlgDXF.m_csTemplate, csNewDwg, FALSE) == FALSE)
    {
      LPVOID lpMsgBuf;
      TCHAR szBuf[201]; 
      DWORD dw = GetLastError(); 
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
      wsprintf(szBuf, _T("%s\n\nCould not create new drawing file.\nError %d: %s"), csNewDwg, dw, lpMsgBuf); 
      appMessage(szBuf, MB_ICONSTOP);
      LocalFree(lpMsgBuf);
      return;
    }

    // Open the newly created drawing and run the CONTINUE_TRANSLATE command
    OpenDrawing(csNewDwg, _T("NET_CONTINUE_TRANSLATE\n"));
  }
  // Otherwise, we can continue the translation in the current drawing
  else 
  {
    // Directly run the translator
    Command_ContinueDXFIn();
  }
}
