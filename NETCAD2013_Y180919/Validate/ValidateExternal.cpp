////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Project name     : eCapture
// Client           : Energy Australia, via AEC Systems, Sydney
//
// File name        : ValidateExtDlg.cpp
// Created          : 20th February 2008
// Created by       : S. Jaisimha
// Description      : External Service Provider data validation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Resource.h"
#include "ValidateExtOptionsDlg.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables
CStringArray g_csaVTypes, g_csaESPStd, g_csaESPLayer, g_csaEAStd, g_csaEALayer;

// Undocumented
void ads_regen();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : UpdateLayerFromStandards
// Description  : Clones the layer from the standard drawing into the current drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UpdateLayerFromStandards(CString csESPLayer, CString csEALayer)
{
  Acad::ErrorStatus es;
  AcDbLayerTable *pCurLT, *pStdLT;
  AcDbLayerTableRecord *pCurLTR, *pStdLTR;

  // Open the current drawing for reading and get the layer data
  AcDbDatabase *pCurDb = acdbHostApplicationServices()->workingDatabase();
  if ((es = pCurDb->getLayerTable(pCurLT, AcDb::kForRead)) != Acad::eOk)        { acutPrintf(_T("\nError %s getting layer table from current drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pCurLT->getAt(csESPLayer, pCurLTR, AcDb::kForWrite))  != Acad::eOk) { acutPrintf(_T("\nError %s getting layer data for %s from current drawing.\n"), acadErrorStatusText(es), csESPLayer); return FALSE; }
  pCurLT->close();

  // Open the standards drawing for reading and get the layer data
  AcDbDatabase *pStdDb = new AcDbDatabase;
  //Commented for ACAD 2018
  //if ((es = pStdDb->readDwgFile(g_csLocalDWT, _SH_DENYWR)) != Acad::eOk)      { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); }
  if ((es = pStdDb->readDwgFile(g_csLocalDWT, AcDbDatabase::OpenMode::kForReadAndReadShare)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); }
  if ((es = pStdDb->getLayerTable(pStdLT, AcDb::kForRead)) != Acad::eOk)      { delete pStdDb; acutPrintf(_T("\nError %s getting layer table from standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pStdLT->getAt(csEALayer, pStdLTR, AcDb::kForRead))  != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s getting layer data for %s from standards drawing.\n"), acadErrorStatusText(es), csEALayer); return FALSE; }
  pStdLT->close();

  // Set the current layer data as per the standards
  pCurLTR->setName(csEALayer);
  pCurLTR->setColor(pStdLTR->color());
  pCurLTR->setLineWeight(pStdLTR->lineWeight());
  //pCurLTR->setLinetypeObjectId(pStdLTR->linetypeObjectId());

  // Close the records
  pStdLTR->close();
  pCurLTR->close();

  // Delete the standards drawing pointer
  delete pStdDb; 

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : UpdateTextStyleFromStandards
// Description  : Clones the text style from the standard drawing into the current drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UpdateTextStyleFromStandards(CString csESPStyle, CString csEAStyle)
{
  AcDbObjectId objEAStyle;
  Acad::ErrorStatus es;
  AcDbTextStyleTable *pCurTST, *pStdTST;
  AcDbTextStyleTableRecord *pCurTSTR, *pStdTSTR;

  // Get a pointer to the current drawing and its block table's object ID
  AcDbDatabase *pCurDb = acdbHostApplicationServices()->workingDatabase();
  if ((es = pCurDb->getTextStyleTable(pCurTST, AcDb::kForRead)) != Acad::eOk)    { acutPrintf(_T("\nError %s getting style table from current drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pCurTST->getAt(csESPStyle, pCurTSTR, AcDb::kForWrite)) != Acad::eOk) { acutPrintf(_T("\nError %s getting style data for %s from current drawing.\n"), acadErrorStatusText(es), csESPStyle); return FALSE; }
  if ((es = pCurTST->getAt(csEAStyle, objEAStyle)) == Acad::eOk)                 { pCurTST->close(); acutPrintf(_T("already exists.\n")); return FALSE; }
  pCurTST->close();

  // Open the standards drawing for reading and its block table
  AcDbDatabase *pStdDb = new AcDbDatabase;
  //Commented for ACAD 2018
  //if ((es = pStdDb->readDwgFile(g_csLocalDWT, _SH_DENYWR)) != Acad::eOk)        { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); }
  if ((es = pStdDb->readDwgFile(g_csLocalDWT, AcDbDatabase::OpenMode::kForReadAndReadShare)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); }
  if ((es = pStdDb->getTextStyleTable(pStdTST, AcDb::kForRead)) != Acad::eOk)   { delete pStdDb; acutPrintf(_T("\nError %s getting style table from standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pStdTST->getAt(csEAStyle, pStdTSTR, AcDb::kForRead))  != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s getting style data for %s from standards drawing.\n"), acadErrorStatusText(es), csEAStyle); return FALSE; }
  pStdTST->close();

  // Copy the data from the standard style to the current style
  //int iCharset, iPitch;	//Commented for ACAD 2018
  Adesk::Boolean bBold, bItalic;
  ACHAR *pszBigName;  pStdTSTR->bigFontFileName(pszBigName); pCurTSTR->setBigFontFileName(pszBigName);
  ACHAR *pszFileName; pStdTSTR->fileName(pszFileName);       pCurTSTR->setFileName(pszFileName);
  ACHAR *pszFont; 
  //Commented for ACAD 2018
  //pStdTSTR->font(pszFont, bBold, bItalic, iCharset, iPitch); pCurTSTR->setFont(pszFont, bBold, bItalic, iCharset, iPitch);
  Charset iCharset;
  Autodesk::AutoCAD::PAL::FontUtils::FontPitch iPitch;
  Autodesk::AutoCAD::PAL::FontUtils::FontFamily family;
  pStdTSTR->font(pszFont, bBold, bItalic, iCharset, iPitch, family); pCurTSTR->setFont(pszFont, bBold, bItalic, iCharset, iPitch, family);
  pCurTSTR->setFlagBits(pStdTSTR->flagBits());
  pCurTSTR->setIsShapeFile(pStdTSTR->isShapeFile());
  pCurTSTR->setIsVertical(pStdTSTR->isVertical());
  pCurTSTR->setObliquingAngle(pStdTSTR->obliquingAngle());
  pCurTSTR->setTextSize(pStdTSTR->textSize());
  pCurTSTR->setXScale(pStdTSTR->xScale());
  pCurTSTR->setName(csEAStyle);

  // Close both records
  pCurTSTR->close();
  pStdTSTR->close();

  // Delete the standards drawing pointer
  delete pStdDb; 

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : UpdateLineTypeFromStandards
// Description  : Clones the line type from the standard drawing into the current drawing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UpdateLineTypeFromStandards(CString csESPLType, CString csEALType)
{
  AcDbObjectId objEALType;
  Acad::ErrorStatus es;
  AcDbLinetypeTable *pCurLTT, *pStdLTT;
  AcDbLinetypeTableRecord *pCurLTTR, *pStdLTTR;

  // Get a pointer to the current drawing and its block table's object ID
  AcDbDatabase *pCurDb = acdbHostApplicationServices()->workingDatabase();
  if ((es = pCurDb->getLinetypeTable(pCurLTT, AcDb::kForRead)) != Acad::eOk)     { acutPrintf(_T("\nError %s getting linetype table from current drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pCurLTT->getAt(csESPLType, pCurLTTR, AcDb::kForWrite)) != Acad::eOk) { acutPrintf(_T("\nError %s getting linetype data for %s from current drawing.\n"), acadErrorStatusText(es), csESPLType); return FALSE; }
  if ((es = pCurLTT->getAt(csEALType, objEALType)) == Acad::eOk)                 { pCurLTT->close(); acutPrintf(_T("already exists.\n")); return FALSE; }
  pCurLTT->close();

  // Open the standards drawing for reading and its block table
  AcDbDatabase *pStdDb = new AcDbDatabase;
  //Commented for ACAD 2018
  //if ((es = pStdDb->readDwgFile(g_csLocalDWT, _SH_DENYWR)) != Acad::eOk)        { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); }
  if ((es = pStdDb->readDwgFile(g_csLocalDWT, AcDbDatabase::OpenMode::kForReadAndReadShare)) != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s reading standards drawing.\n"), acadErrorStatusText(es)); }
  if ((es = pStdDb->getLinetypeTable(pStdLTT, AcDb::kForRead)) != Acad::eOk)    { delete pStdDb; acutPrintf(_T("\nError %s getting linetype table from standards drawing.\n"), acadErrorStatusText(es)); return FALSE; }
  if ((es = pStdLTT->getAt(csEALType, pStdLTTR, AcDb::kForRead))  != Acad::eOk) { delete pStdDb; acutPrintf(_T("\nError %s getting linetype data for %s from standards drawing.\n"), acadErrorStatusText(es), csEALType); return FALSE; }
  pStdLTT->close();

  // Copy the data from the standard linetype to the current linetype
  ACHAR *pszText;
  ACHAR *pszAscii;    pStdLTTR->asciiDescription(pszAscii); pCurLTTR->setAsciiDescription(pszAscii);
  ACHAR *pszComments; pStdLTTR->comments(pszComments);      pCurLTTR->setComments(pszComments);
  pCurLTTR->setIsScaledToFit(pStdLTTR->isScaledToFit());
  pCurLTTR->setPatternLength(pStdLTTR->patternLength());
  pCurLTTR->setNumDashes(pStdLTTR->numDashes());
  pCurLTTR->setName(csEALType);
  for (int iCtr = 0; iCtr < pStdLTTR->numDashes(); iCtr++)
  {
    pCurLTTR->setDashLengthAt(iCtr, pStdLTTR->dashLengthAt(iCtr));
    pCurLTTR->setShapeIsUcsOrientedAt(iCtr, pStdLTTR->shapeIsUcsOrientedAt(iCtr));
    pCurLTTR->setShapeNumberAt(iCtr, pStdLTTR->shapeNumberAt(iCtr));
    pCurLTTR->setShapeOffsetAt(iCtr, pStdLTTR->shapeOffsetAt(iCtr));
    pCurLTTR->setShapeRotationAt(iCtr, pStdLTTR->shapeRotationAt(iCtr));
    pCurLTTR->setShapeScaleAt(iCtr, pStdLTTR->shapeScaleAt(iCtr));
    pCurLTTR->setShapeStyleAt(iCtr, pStdLTTR->shapeStyleAt(iCtr));
    pStdLTTR->textAt(iCtr, pszText); pCurLTTR->setTextAt(iCtr, pszText);
  }

  // Close both records
  pCurLTTR->close();
  pStdLTTR->close();

  // Delete the standards drawing pointer
  delete pStdDb; 

  // Success
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : MoveBlockToLayer
// Description  : Moves the given blocks on the ESP layer to the EA layer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MoveBlockToLayer(CString csBlock, CString& csESPLayer, CString& csEALayer)
{
  // Check for validity
  if (csESPLayer == _T("any layer")) csESPLayer = _T("*");

  // Ensure destination layer exists
  if (acdbTblSearch(_T("LAYER"), csEALayer, 0) == NULL) { acutPrintf(_T("  Layer %s not found.\n"), csEALayer); return; }

  // Make a selection set
  struct resbuf *rbpFilter = acutBuildList(RTDXF0, _T("INSERT"), 2, csBlock, 8, csESPLayer, NULL);
  ads_name ssBlocks; if (acedSSGet(_T("X"), NULL, NULL, rbpFilter, ssBlocks) != RTNORM) { acutRelRb(rbpFilter); return; }
  acutRelRb(rbpFilter);

  // Change the properties
  acedCommandS(RTSTR, _T(".CHPROP"), RTPICKS, ssBlocks, RTSTR, _T(""), RTSTR, _T("LA"), RTSTR, csEALayer, RTSTR, _T(""), NULL);

  // Free the selection set
  acedSSFree(ssBlocks);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ExtValidateBlocks
// Arguments    : Type of change to be performed, as integer
// Called from  : Command_ExtValidate
// Description  : Validates the blocks in the current drawing against the standards
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ExtValidateBlocks(int iCheckType)
{
  CString csESPBlock, csESPLayer, csEABlock, csEALayer;
  AcDbBlockTable *pBT; 
  AcDbBlockTableRecord *pBTR;

  acutPrintf(_T("\nValidating blocks...\n"));

  // For "Block" values in the array
  for (int iCtr = 0; iCtr < g_csaVTypes.GetSize(); iCtr++)
  {
    if (g_csaVTypes.GetAt(iCtr).CompareNoCase(_T("Block")) == 0)
    {
      // Get the ESP and EA standard block and layer names
      csESPBlock = g_csaESPStd.GetAt(iCtr);
      csESPLayer = g_csaESPLayer.GetAt(iCtr);
      csEABlock  = g_csaEAStd.GetAt(iCtr);
      csEALayer  = g_csaEALayer.GetAt(iCtr);

      // Safety check
      if (csESPBlock.IsEmpty() || csEABlock.IsEmpty()) continue;

      // Fix for unspecified layers
      if (csESPLayer.IsEmpty()) csESPLayer = _T("any layer");
      if (csEALayer.IsEmpty())  csEALayer  = _T("same layer");

      acutPrintf(_T("  Updating block %s on %s to %s on %s...\n"), csESPBlock, csESPLayer, csEABlock, csEALayer);

      // Rename the block to EA standards
      if (acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBT, AcDb::kForRead) != Acad::eOk) continue;
      if (pBT->getAt(csESPBlock, pBTR, AcDb::kForWrite) != Acad::eOk) { pBT->close(); continue; }
      pBTR->setName(csEABlock);
      pBT->close();
      pBTR->close();

      // If we must replace the block, update the block from the standards
      if (iCheckType == 2) UpdateBlockFromStandards(csEABlock);

      // Move block to proper layer, if necessary
      if (csEALayer != _T("same layer")) MoveBlockToLayer(csEABlock, csESPLayer, csEALayer);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ExtValidateLayers
// Arguments    : None
// Called from  : Command_ExtValidate
// Description  : Validates the layers in the current drawing against the standards
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ExtValidateLayers()
{
  CString csESPLayer, csEALayer;

  acutPrintf(_T("\nValidating layers...\n"));

  // For "Layer" values in the array
  for (int iCtr = 0; iCtr < g_csaVTypes.GetSize(); iCtr++)
  {
    if (g_csaVTypes.GetAt(iCtr).CompareNoCase(_T("Layer")) == 0)
    {
      // Get the ESP and EA standard layer names
      csESPLayer = g_csaESPStd.GetAt(iCtr);
      csEALayer  = g_csaEAStd.GetAt(iCtr);
      acutPrintf(_T("  Updating layer %s to %s..."), csESPLayer, csEALayer);

      // Change the layer settings according to EA standards
      if (UpdateLayerFromStandards(csESPLayer, csEALayer) == TRUE) acutPrintf(_T("done.\n"));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ExtValidateTextStyles
// Arguments    : None
// Called from  : Command_ExtValidate
// Description  : Validates the text styles in the current drawing against the standards
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ExtValidateTextStyles()
{
  CString csESPStyle, csEAStyle;

  acutPrintf(_T("\nValidating text styles...\n"));

  // For "Layer" values in the array
  for (int iCtr = 0; iCtr < g_csaVTypes.GetSize(); iCtr++)
  {
    if (g_csaVTypes.GetAt(iCtr).CompareNoCase(_T("Text style")) == 0)
    {
      // Get the ESP and EA standard text style names
      csESPStyle = g_csaESPStd.GetAt(iCtr);
      csEAStyle  = g_csaEAStd.GetAt(iCtr);
      acutPrintf(_T("  Updating style %s to %s...\n"), csESPStyle, csEAStyle);

      // Update the text style from the standards
      UpdateTextStyleFromStandards(csESPStyle, csEAStyle);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ExtValidateLineTypes
// Arguments    : None
// Called from  : Command_ExtValidate
// Description  : Validates the line types in the current drawing against the standards
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ExtValidateLineTypes()
{
  CString csESPType, csEAType;

  acutPrintf(_T("\nValidating line types...\n"));

  // For "Layer" values in the array
  for (int iCtr = 0; iCtr < g_csaVTypes.GetSize(); iCtr++)
  {
    if (g_csaVTypes.GetAt(iCtr).CompareNoCase(_T("Linetype")) == 0)
    {
      // Get the ESP and EA standard text style names
      csESPType = g_csaESPStd.GetAt(iCtr);
      csEAType  = g_csaEAStd.GetAt(iCtr);
      acutPrintf(_T("  Updating linetype %s to %s...\n"), csESPType, csEAType);

      // Update the text style from the standards
      UpdateLineTypeFromStandards(csESPType, csEAType);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : setSysVar
// Arguments    : Varies
// Called from  : ExtValidateDimStyles
// Description  : Overloaded helpers for setting dimension variables
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus setSysVar(LPCTSTR varName, int val)
{
  ASSERT(varName != NULL);
  resbuf rb;
  rb.restype = RTSHORT;
  rb.resval.rint = val;
  acedSetVar(varName, &rb);
  return Acad::eOk;
}

Acad::ErrorStatus setSysVar(LPCTSTR varName, double val)
{
  ASSERT(varName != NULL);
  resbuf rb;
  rb.restype = RTREAL;
  rb.resval.rreal = val;
  acedSetVar(varName, &rb);
  return Acad::eOk;
}

Acad::ErrorStatus setSysVar(LPCTSTR varName, const TCHAR* val)
{
  ASSERT(varName != NULL);
  resbuf rb;
  rb.restype = RTSTR;
  rb.resval.rstring = const_cast<TCHAR*>(val);
  acedSetVar(varName, &rb);
  return Acad::eOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : ExtValidateDimStyles
// Arguments    : None
// Called from  : Command_ExtValidate
// Description  : Validates the dimension variables in the current drawing against the standards
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ExtValidateDimStyles()
{
  // Get the dimension variables and its proposed values from the table and set them all
  CQueryTbl tblDimensions;
  if (!tblDimensions.SqlRead(DSN_ECapture, _T("SELECT [VarType], [DimVar], [DimValue] FROM tblDimensions"), __LINE__, __FILE__, _T("ExtValidateDimStyles"),true)) return;
  struct resbuf rbStrVar; rbStrVar.restype = RTSTR;
  struct resbuf rbIntVar; rbStrVar.restype = RTSHORT;
  struct resbuf rbFltVar; rbStrVar.restype = RTREAL;
  Acad::ErrorStatus es;
  LPCTSTR csDimVar, csDimVal, csVarTyp;
  for (int iRow = 0; iRow < tblDimensions.GetRows(); iRow++)
  {
    csVarTyp = tblDimensions.GetRowAt(iRow)->GetAt(0);
    csDimVar = tblDimensions.GetRowAt(iRow)->GetAt(1);
    csDimVal = tblDimensions.GetRowAt(iRow)->GetAt(2);
    /**/ if (!_tcsicmp(csVarTyp, _T("INT")))    es = setSysVar(csDimVar, _ttoi(csDimVal));
    else if (!_tcsicmp(csVarTyp, _T("FLOAT")))  es = setSysVar(csDimVar, _tstof(csDimVal));
    else if (!_tcsicmp(csVarTyp, _T("STRING"))) es = setSysVar(csDimVar, csDimVal);
  }

  if (!acdbTblSearch(_T("DIMSTYLE"), _T("DIMENSION"), FALSE))
  {
    struct resbuf rbSetvar;
    rbSetvar.restype = RTSTR; rbSetvar.resval.rstring = _T("STANDARD"); acedSetVar(_T("DIMTXSTY"), &rbSetvar);
    rbSetvar.restype = RTSHORT; rbSetvar.resval.rint = 5; acedSetVar(_T("EXPERT"), &rbSetvar);
    acedCommandS(RTSTR, _T(".DIM1"), RTSTR, _T("SAVE"), RTSTR, _T("STANDARD"), NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function     : Command_ExtValidate
// Called from  : User, via the "EA_EXTVALIDATE" command
// Description  : Validates the external service provider drawing for conformity with standards
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Command_ExtValidate()
{
  // Display the external validation options dialog
  CValidateExtOptionsDlg dlgEV;
  if (dlgEV.DoModal() == IDCANCEL) return;

  // Set a few variables
  struct resbuf rbVar;
  rbVar.restype = RTSHORT;
  rbVar.resval.rint = 0; acedSetVar(_T("CMDECHO"),  &rbVar);  // Command echo OFF
  rbVar.resval.rint = 1; acedSetVar(_T("TILEMODE"), &rbVar);  // Model space ON

  // Get the data from the ESP's data table
  CString csSQL;
  CQueryTbl tblStd;
  csSQL.Format(_T("SELECT [VType], [ESPStd], [ESPLayer], [EAStd], [EALayer] FROM %s ORDER BY [VType], [ESPStd], [ESPLayer]"), dlgEV.m_csESPTable);
  if (!tblStd.SqlRead(DSN_ECapture_Ext, csSQL, __LINE__, __FILE__, _T("Command_ExtValidate"),true)) return;
  if (tblStd.GetRows() <= 0) { appMessage(_T("There are no configured values for this External Service Provider.\nPlease contact your Systems Administrator.")); return; }
  tblStd.GetColumnAt(0, g_csaVTypes);
  tblStd.GetColumnAt(1, g_csaESPStd);
  tblStd.GetColumnAt(2, g_csaESPLayer);
  tblStd.GetColumnAt(3, g_csaEAStd);
  tblStd.GetColumnAt(4, g_csaEALayer);

  // If we must validate layers
  if (dlgEV.m_bLayers == TRUE) ExtValidateLayers();

  // If we must validate blocks
  if (dlgEV.m_bBlocks == TRUE) ExtValidateBlocks(dlgEV.m_iBlockCheckType);

  // If we must validate text styles
  if (dlgEV.m_bTextStyles == TRUE) ExtValidateTextStyles();

  // If we must validate line types
  //if (dlgEV.m_bLineTypes == TRUE) ExtValidateLineTypes();

  // If we must validate dimension settings
  if (dlgEV.m_bDimStyles == TRUE) ExtValidateDimStyles();

  // Regenerate the drawing
  ads_regen();

  // Success
  acutPrintf(_T("\nExternal validation completed.\n\n"));
}
