// This file is included in StdAfx.h
#define PI            3.1415926535897932384626433832795
#define PIby2         (PI / 2.0)
#define MinusPIby2    (PI / -2.0)

#define DTR(x) ((x) * (PI / 180.0))
#define RTD(x) ((x) * (180.0 / PI))

// Sets up a DSN to an MS-Access database.
BOOL SetupDataSource                      (CString csDSN, CString csMDB);

// Message utilities
void appMessage                           (CString sMessage, int iIcon = 0);
void appError                             (CString sFileName, CString sFuncName, int iLine, CString sErrorMsg);
void displayMessage                       (LPTSTR fmt, ...);
int getConfirmation                       (CString csQuestion);

// AutoCAD version
BOOL isAcad2011                           (CString csCommand);

// System vars off/on
void switchOff				                    (void);
void switchOn 				                    (void);

// System date.
TCHAR *getSystemDate                      (void);

// Assoc in a struct resbuf
struct resbuf *Assoc                      (struct resbuf*, int);

// XData write/retrieve
void addXDataToEntity                     (ads_name, struct resbuf *);
struct resbuf *getXDataFromEntity         (ads_name, const TCHAR*);

// Dictionary Xrecord.
void addXRecordToDictionary               (const TCHAR *sDictName, const TCHAR *sXrecName, struct resbuf *rbpXR);
void addXRecordToDictInDwg                (AcDbDatabase *pDwgDb,  const TCHAR *sDictName, const TCHAR *sXrecName, struct resbuf *rbpXR);
void DeleteXRecord                        (const TCHAR *sDictName, const TCHAR *sXrecName);
struct resbuf *getXRecordFromDictionary   (const TCHAR *sDictName, const TCHAR *sXrecName);
struct resbuf *getXRecordFromFile         (const TCHAR *sFileName, const TCHAR *sDictName, const TCHAR *sXrecName);
void appendXRecordToDictionary            (const TCHAR *sDictName, const TCHAR *sXrecName, struct resbuf *rbpAppend);
void DeleteDictionary                     (CString csDictName, CString csDwg = _T(""));

// General
CString suppressZero                      (double dValue);

// Layer related.
void setLayer                             (const TCHAR *, Adesk::Boolean);
void createLayer                          (const TCHAR* sLayerName, Adesk::Boolean iFreezeRest = Adesk::kTrue, Adesk::Boolean doSetLayer = Adesk::kTrue, int iColor = 0, CString csLtype = _T(""), int iLTypeWt = -1);
BOOL purgeLayer                           (const TCHAR *);
void eraseAllEntities                     (CString csLayerName = _T(""));

// Drawing access functions
BOOL OpenDrawing                          (const TCHAR *, const TCHAR * = NULL);

// Block insertion.
BOOL InsertBlock                          (const TCHAR*, ads_point, double, double, CStringArray &);
BOOL InsertBlock                          (const TCHAR*, ads_point, double, double);

// Attribute functions
int  BuildAttribsList                     (CString, CStringArray&, CStringArray&, CStringArray&, BOOL);
void RotateAttribs                        (ads_name, double);
void ChangeAttribsLayer                   (AcDbObjectId, TCHAR *);

// File related
BOOL CheckFileAccess                      (CString sFileName, CString sFuncName, int iLine, CString szFilePath, int iMode);
BOOL CheckFileAccess                      (CString szFilePath, int iMode);
void ShowBalloon													(CString csMessage, CWnd *pParent, UINT nCtrlID, CString csTitle = _T("Invalid!"));
BOOL deleteXRecordFromDictInDwg           (AcDbDatabase *pCurDwg, const TCHAR *sDictName, const TCHAR *sXrecName);

BOOL getDocFromFilename                   (CString csFileName, AcApDocument* &pNewDocument);
BOOL GetDictionaryName                    (const TCHAR *sDictName);
BOOL IsDrawingOpen                        (TCHAR * sSrcFile, CString csMsg);
BOOL CheckForDuplication                  (CString csCheck, CStringArray& csaCheckArray);
BOOL CheckForDuplication                  (CString csCheck, CStringArray& csaCheckArray, int& iIndex);

// Change properties for entity or selection set
BOOL ChangeProperties(BOOL bIsSSet, ads_name anEnOrSS, int iNewColor = 0, const TCHAR *pszNewLayer = NULL, const TCHAR *pszNewLType = NULL, double dNewLTScale = 1.0);

// Show or hide entity or entities in selection set
BOOL SetVisibility(BOOL bIsSSet, ads_name anEnOrSS, AcDb::Visibility vShow);

// Calculate the extents and center point of a selection set
BOOL GetSSExtents(ads_name ssSet, ads_point ptLL, ads_point ptUR, ads_point ptCenter);

// Help
void displayHelp(DWORD dwTopic);

// Wild card matching
bool WildMatch(const TCHAR *pattern, const TCHAR *str);
int MatchLayerName(CString csLayerName, CStringArray& csaPrefix, CStringArray& csaMid1, CStringArray& csaMid2, CStringArray& csaSuffix);

// Line type object id
AcDbObjectId GetLineTypeObjectId(CString csLineType);

// Update from standards
int UpdateBlockFromStandards(CString csSymbol);

//////////////////////// Added for DwgTools compatibility /////////////////////////////

// Add entity to drawing
bool UpdateLayerFromStandards(CString csLayerName);
AcDbObjectId appendEntityToDatabase(AcDbEntity *pEntity, BOOL bIsMSpace = TRUE);
bool DisplayHelp(DWORD dwTopic);
void saveOSMode();
void restoreOSMode();
void saveUCSMode();
void restoreUCSMode();
void appErrorTxt(CString csFile, int iLine, CString csMsg);
BOOL ensureSymbolAvailability(CString csSymbol, AcDbObjectId& objBlock);
BOOL GetSymbolLayer(CString csBlockName, CString &csLayerName);
BOOL insertBlock(CString csBlockName, CString csLayerName, ads_point ptIns, double dScaleX, double dScaleY, double dHeight, double dRotation, CString csTextStyle, AcDbObjectId& objInsert, BOOL bIsMSpace);
BOOL CheckForDuplication(CStringArray& csaArray, CString csCheck, int &iIndex);
BOOL CheckForDuplication(CStringArray& csaArray, CString csCheck);
bool GetPoleExtents(AcDbObjectId objPole, AcDbExtents &exBounds);
void deleteArray(AcDbVoidPtrArray entities);
void deleteArray(AcDbObjectIdArray objectIds);
int GetParameterValue(CStringArray &csaParameter, CString csParameter, CString &csValue, int iSearch);
CString GetStdLineTypeName(AcDbDatabase *pStdDb, AcDbObjectId objStdLType);
