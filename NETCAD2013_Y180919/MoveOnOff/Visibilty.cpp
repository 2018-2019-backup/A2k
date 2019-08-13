#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////////
// Function name: Command_Visibility()
// Description  : Called when "NMOF" is issued at command prompt.
//////////////////////////////////////////////////////////////////////////
void Command_Visibility()
{
	// Switch off certain system variables
	switchOff();

	// Print command version
	acutPrintf(L"\nVersion: V2.0");
				
	acutPrintf(_T("\r\nSelect objects to change its visibility...\n"));
	// while(T)
	{
		// Allow user to select entities
		ads_name ssSel;
		if (acedSSGet(NULL, NULL, NULL, NULL, ssSel) != RTNORM) { acedSSFree(ssSel); return; }

		// For each entity in the selection set, create the visibility layer and move it to that layer
		ads_name enEntity;
		AcDbObjectId objId;
		AcDbEntity *pEntity = NULL;
		Acad::ErrorStatus es;

		AcDbLayerTable *pLayerTbl;
		AcDbLayerTableRecord *pLayerTblRecord;

		bool bIsVisible;
		//long lLength = 0L; acedSSLength(ssSel, &lLength);
		int  lLength = 0L; acedSSLength(ssSel, &lLength);
		
		for (long lCtr = 0L; lCtr < lLength; lCtr++)
		{
			// Get the entity name
			acedSSName(ssSel, lCtr, enEntity);

			// Get the object id
			acdbGetObjectId(objId, enEntity);

			// Get the layer on which it is placed
			es = acdbOpenObject(pEntity, objId, AcDb::kForWrite);
			if (es != Acad::eOk) { acutPrintf(_T("\nERROR: %s"), acadErrorStatusText(es)); acedSSFree(ssSel); return; }
			CString csLayer = pEntity->layer();

			// Check if the entity is already on VISIBILITY layer with "_OFF" as its suffix
			bIsVisible = true;
			/**/ if ((csLayer.GetLength() > 5) && (csLayer.Find(L"_OFF_") != -1)) 
			{
				bIsVisible = false; // On the _OFF_ layer
				csLayer = csLayer.Mid(0, csLayer.Find(_T("_OFF_"))) + L"_" + csLayer.Mid((csLayer.Find(L"_OFF_") + 5));
			}
			else if ((csLayer.GetLength() > 4) && (csLayer.Find(_T("_OFF")) != -1))  
			{
				bIsVisible = false; // On the _OFF layer
				csLayer = csLayer.Mid(0, csLayer.Find(_T("_OFF")));
			}
			
			// Create the corresponding visibility layer
			if (bIsVisible)	csLayer = csLayer + _T("_OFF"); 

			//////////////////////////////////////////////////////////////////////////
			// Check if the layer exists
			//////////////////////////////////////////////////////////////////////////

			// If a new "_OFF" is created, the properties of the new layer are set to the properties of the layer that the object is currently on.
			bool bLayerExists = true;
			if (acdbTblSearch(L"LAYER", csLayer, TRUE) == NULL)
			{
				bLayerExists = false;

				// Get this drawing's layer table pointer
				acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead);

				// Get the layer table record of the layer on which the selected entity is
				if (pLayerTbl->getAt(pEntity->layer(), pLayerTblRecord, AcDb::kForRead) == Acad::eOk)
				{
					AcDbLinetypeTableRecord *pLTypeTblRcd;
					if (acdbOpenObject(pLTypeTblRcd, pLayerTblRecord->linetypeObjectId(), AcDb::kForRead) == Acad::eOk)
					{
						// Get the linetype name
						AcString acsName; pLTypeTblRcd->getName(acsName); pLTypeTblRcd->close(); 
						CString csLType;  csLType.Format(_T("%s"), acsName.kTCharPtr()); csLType = csLType.MakeUpper();

						// Get the color
						int iColorIndex = pLayerTblRecord->entityColor().colorIndex();

						// Get the line weight
						AcDb::LineWeight lineWt = pLayerTblRecord->lineWeight();
						pLayerTblRecord->close();
						pLayerTbl->close();

						// Create the new layer
						createLayer(csLayer, Adesk::kFalse, Adesk::kFalse, iColorIndex, csLType, lineWt);
					}
				}
				else
				{
					createLayer(csLayer, Adesk::kFalse, Adesk::kFalse);
				}
			}
						
			// Move the entity to the visibility layer
			pEntity->setLayer(csLayer);
			pEntity->close();

			//////////////////////////////////////////////////////////////////////////
			// Turn on the RESULTANT LAYER if they don't contain _OFF. Else turn it OFF.
			//////////////////////////////////////////////////////////////////////////
			// Get the layer table record of the layer on which the selected entity is and set its visibility state (Provided the layer is created afresh)
			if (!bLayerExists && acdbHostApplicationServices()->workingDatabase()->getLayerTable(pLayerTbl, AcDb::kForRead) == Acad::eOk)
			{
				if (pLayerTbl->getAt(csLayer, pLayerTblRecord, AcDb::kForWrite) == Acad::eOk)
				{
					if (csLayer.Find(L"_OFF") != -1) pLayerTblRecord->setIsOff(true);
					pLayerTblRecord->close();
				}

				pLayerTbl->close();
			}
		}

		// Free the selection set
		acedSSFree(ssSel);
	}
}
