#pragma once

#include "Vehicle.h"
#include "Door.h"

enum eBoatNodes
{
	BOAT_MOVING = 1,
	BOAT_WINDSCREEN,
	BOAT_RUDDER,
	BOAT_FLAP_LEFT,
	BOAT_FLAP_RIGHT,
	BOAT_REARFLAP_LEFT,
	BOAT_REARFLAP_RIGHT,
	NUM_BOAT_NODES
};

class CBoat : public CVehicle
{
public:
	float m_fMovingRotation;
	float m_fMovingSpeed;
	int32 m_boat_unused1;
	RwFrame *m_aBoatNodes[NUM_BOAT_NODES];
	CDoor m_boom;
	tBoatHandlingData *pBoatHandling;
	uint8 bBoatInWater : 1;
	uint8 bPropellerInWater : 1;
	bool m_bIsAnchored;
	float m_fOrientation;
	uint32 m_nPoliceShoutTimer;
	int32 m_boat_unused2;
	float m_fDamage;
	CEntity *m_pSetOnFireEntity;
	float m_skimmerThingTimer;
	bool m_boat_unused3;
	float m_fAccelerate;
	float m_fBrake;
	float m_fSteeringLeftRight;
	uint8 m_nPadID;
	int32 m_boat_unused4;
	float m_fVolumeUnderWater;
	CVector m_vecBuoyancePoint;
	float m_fPrevVolumeUnderWater;
	int16 m_nDeltaVolumeUnderWater;
	uint16 m_nNumWakePoints;
	CVector2D m_avec2dWakePoints[32];
	float m_afWakePointLifeTime[32];

	static float MAX_WAKE_LENGTH;
	static float MIN_WAKE_INTERVAL;
	static float WAKE_LIFETIME;

	CBoat(int, uint8);

	virtual void SetModelIndex(uint32 id);
	virtual void ProcessControl();
	virtual void Teleport(CVector v);
	virtual void PreRender(void);
	virtual void Render(void);
	virtual void ProcessControlInputs(uint8);
	virtual void GetComponentWorldPosition(int32 component, CVector &pos);
	virtual bool IsComponentPresent(int32 component) { return true; }
	virtual void BlowUpCar(CEntity *ent);

	void RenderWaterOutPolys(void);
	void ApplyWaterResistance(void);
	void SetupModelNodes();
	void PruneWakeTrail(void);
	void AddWakePoint(CVector point);
	void DoDriveByShootings(void);

	static CBoat *apFrameWakeGeneratingBoats[4];
	
	static bool IsSectorAffectedByWake(CVector2D sector, float fSize, CBoat **apBoats);
	static float IsVertexAffectedByWake(CVector vecVertex, CBoat *pBoat);
	static void FillBoatList(void);

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif
	static const uint32 nSaveStructSize;

};