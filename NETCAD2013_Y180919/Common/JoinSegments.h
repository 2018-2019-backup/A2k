#include <vector>

class CAsdkBasicEntityInfo
{
public:
	CAsdkBasicEntityInfo ()
	{
		m_bulge = 0.0;
	};

	CAsdkBasicEntityInfo (const CAsdkBasicEntityInfo &info)
	{
		*this = info;
	};

	// Entity that the m_from and m_to points belong
	AcDbObjectId m_objectId;
	// Start point of the entity
	AcGePoint3d	m_from;
	// End point of the entity
	AcGePoint3d	m_to;
	// Bulge factor i.e. the angle at which the arc goes off at
	ads_real m_bulge; 

	void operator=(const CAsdkBasicEntityInfo &info)
	{
		// Entity that the m_from and m_to points belong
		m_objectId = info.m_objectId;
		// Start point of the entity
		m_from = info.m_from;
		// End point of the entity
		m_to = info.m_to;
		// Bulge factor i.e. the angle at which the arc goes off at
		m_bulge = info.m_bulge;
	};
};

//////////////////////////////////////////////////////////////////////////
// Create a vector data type of CAsdkBasicEntityInfo's
//////////////////////////////////////////////////////////////////////////
using namespace std;
typedef vector<CAsdkBasicEntityInfo> VAsdkBasicEntityInfo;

//////////////////////////////////////////////////////////////////////////
// Specify a global fuzz value
//////////////////////////////////////////////////////////////////////////
double gFuzz = 1;         // OR one can use AcGeContext::gTol.equalPoint()

// Gets the start and end point into a vector array
Adesk::Boolean GatherSelectionSetDetails (VAsdkBasicEntityInfo &entityInfo, ads_name ss);

// Loops a selection set and erases entities within
Adesk::Boolean DeleteEntsInSS (ads_name &ss, VAsdkBasicEntityInfo mArrEntityInfo);

// Add entity to the database
Adesk::Boolean fPostToMs (AcDbEntity *pEnt, AcDbDatabase *pDb);

// Create a polyline entity from the VAsdkBasicEntityInfo
Adesk::Boolean CreatePline (VAsdkBasicEntityInfo &mArrEntInfo, CString csLayer);

// Gets next point in the VAsdkBasicEntityInfo given a search point
long fGetNextPoint(AcGePoint3d &mSearchPt,const VAsdkBasicEntityInfo &mArrEntInfo, double &mBulge, long mSkip = -1);
