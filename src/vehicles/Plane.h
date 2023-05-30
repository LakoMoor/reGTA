#pragma once

#include "Vehicle.h"

enum ePlaneNodes
{
#ifdef CPLANE_ROTORS
	// for heli
	PLANE_TOPROTOR,
	PLANE_BACKROTOR,
#endif
	PLANE_WHEEL_FRONT = 2,
	PLANE_WHEEL_READ,
	NUM_PLANE_NODES
};

struct CPlaneNode
{
	CVector p;	// position
	float t;	// xy-distance from start on path
	bool bOnGround;	// i.e. not flying
};

struct CPlaneInterpolationLine
{
	uint8 type;
	float time;	// when does this keyframe start
	// initial values at start of frame
	float position;
	float speed;
	float acceleration;
};

class CPlane : public CVehicle
{
public:
#ifdef CPLANE_ROTORS
	RwFrame *m_aPlaneNodes[NUM_PLANE_NODES];
	float m_fRotorRotation;
#endif
	int16 m_nPlaneId;
	int16 m_isFarAway;
	int16 m_nCurPathNode;
	float m_fSpeed;
	uint32 m_nFrameWhenHit;
	bool m_bHasBeenHit;
	bool m_bIsDrugRunCesna;
	bool m_bIsDropOffCesna;
	bool m_bTempPlane;

	CPlane(int32 id, uint8 CreatedBy);
	~CPlane(void);

	// from CEntity
	void SetModelIndex(uint32 id);
	void DeleteRwObject(void);
	void ProcessControl(void);
	void PreRender(void);
	void Render(void);
	void FlagToDestroyWhenNextProcessed() { bRemoveFromWorld = true; }

	static void InitPlanes(void);
	static void Shutdown(void);
	static CPlaneNode *LoadPath(char const *filename, int32 &numNodes, float &totalLength, bool loop);
	static void RemoveTemporaryPlanes(void);
	static void UpdatePlanes(void);
	static bool TestRocketCollision(CVector *rocketPos);
	static void CreateIncomingCesna(void);
	static void CreateDropOffCesna(void);
	static const CVector FindDrugPlaneCoordinates(void);
	static const CVector FindDropOffCesnaCoordinates(void);
	static bool HasCesnaLanded(void);
	static bool HasCesnaBeenDestroyed(void);
	static bool HasDropOffCesnaBeenShotDown(void);
	static void Load(void);
	static void Save(void);
};

extern float LandingPoint;
extern float TakeOffPoint;
extern float PlanePathPosition[3];
