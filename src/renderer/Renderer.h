#pragma once

class CEntity;

#ifdef FIX_BUGS
#define LOD_DISTANCE (300.0f*TheCamera.LODDistMultiplier)
#else
#define LOD_DISTANCE 300.0f
#endif
#define FADE_DISTANCE 20.0f
#define STREAM_DISTANCE 30.0f

extern bool gbShowPedRoadGroups;
extern bool gbShowCarRoadGroups;
extern bool gbShowCollisionPolys;
extern bool gbShowCollisionLines;
extern bool gbBigWhiteDebugLightSwitchedOn;

extern bool gbDontRenderBuildings;
extern bool gbDontRenderBigBuildings;
extern bool gbDontRenderPeds;
extern bool gbDontRenderObjects;
extern bool gbDontRenderVehicles;

class CVehicle;
class CPtrList;

// unused
struct BlockedRange
{
	float a, b;	// unknown
	BlockedRange *prev, *next;
};

class CRenderer
{
	static int32 ms_nNoOfVisibleEntities;
	static CEntity *ms_aVisibleEntityPtrs[NUMVISIBLEENTITIES];
	static int32 ms_nNoOfInVisibleEntities;
	static CEntity *ms_aInVisibleEntityPtrs[NUMINVISIBLEENTITIES];
#ifdef NEW_RENDERER
	static int32 ms_nNoOfVisibleVehicles;
	static CEntity *ms_aVisibleVehiclePtrs[NUMVISIBLEENTITIES];
	// for cWorldStream emulation
	static int32 ms_nNoOfVisibleBuildings;
	static CEntity *ms_aVisibleBuildingPtrs[NUMVISIBLEENTITIES];
#endif

	static CVector ms_vecCameraPosition;
	static CVehicle *m_pFirstPersonVehicle;

	// unused
	static BlockedRange aBlockedRanges[16];
	static BlockedRange *pFullBlockedRanges;
	static BlockedRange *pEmptyBlockedRanges;
public:
	static float ms_lodDistScale;
	static bool m_loadingPriority;

	static void Init(void);
	static void Shutdown(void);
	static void PreRender(void);

	static void RenderRoads(void);
	static void RenderFadingInEntities(void);
	static void RenderFadingInUnderwaterEntities(void);
	static void RenderEverythingBarRoads(void);
	static void RenderBoats(void);
	static void RenderOneRoad(CEntity *);
	static void RenderOneNonRoad(CEntity *);
	static void RenderFirstPersonVehicle(void);

	static void RenderCollisionLines(void);

	static int32 SetupEntityVisibility(CEntity *ent);
	static int32 SetupBigBuildingVisibility(CEntity *ent);

	static void ConstructRenderList(void);
	static void ScanWorld(void);
	static void RequestObjectsInFrustum(void);
	static void ScanSectorPoly(RwV2d *poly, int32 numVertices, void (*scanfunc)(CPtrList *));
	static void ScanBigBuildingList(CPtrList &list);
	static void ScanSectorList(CPtrList *lists);
	static void ScanSectorList_Priority(CPtrList *lists);
	static void ScanSectorList_Subway(CPtrList *lists);
	static void ScanSectorList_RequestModels(CPtrList *lists);

	static void SortBIGBuildings(void);
	static void SortBIGBuildingsForSectorList(CPtrList *list);

	static bool ShouldModelBeStreamed(CEntity *ent, const CVector &campos);

	static void RemoveVehiclePedLights(CEntity *ent, bool reset);


#ifdef NEW_RENDERER
	static void ClearForFrame(void);
	static void RenderPeds(void);
	static void RenderVehicles(void);	// also renders peds in LCS
	static void RenderOneBuilding(CEntity *ent, float camdist = 0.0f);
	static void RenderWorld(int pass);	// like cWorldStream::Render(int)
	static void RenderTransparentWater(void);	// keep-out polys and transparent water
#endif
	static void InsertEntityIntoList(CEntity *ent);
};
