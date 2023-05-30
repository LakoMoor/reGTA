#pragma once

class CEntity;

class CFire
{
public:
	bool m_bIsOngoing;
	bool m_bIsScriptFire;
	bool m_bPropagationFlag;
	bool m_bAudioSet;
	CVector m_vecPos;
	CEntity *m_pEntity;
	CEntity *m_pSource;
	uint32 m_nExtinguishTime;
	uint32 m_nStartTime;
	uint32 m_nNextTimeToAddFlames;
	float m_fStrength;
	float m_fWaterExtinguishCountdown;
	bool m_bExtinguishedWithWater;

	CFire();
	~CFire();
	void ProcessFire(void);
	void ReportThisFire(void);
	void Extinguish(void);
};

class CFireManager
{
	enum {
		MAX_FIREMEN_ATTENDING = 2,
	};
public:
	uint32 m_nTotalFires;
	CFire m_aFires[NUM_FIRES];

	void StartFire(CVector pos, float size, uint8 propagation);
	CFire *StartFire(CEntity *entityOnFire, CEntity *fleeFrom, float strength, uint8 propagation);
	void Update(void);
	CFire *FindFurthestFire_NeverMindFireMen(CVector coords, float minRange, float maxRange);
	CFire *FindNearestFire(CVector vecPos, float *pDistance);
	CFire *GetNextFreeFire(void);
	uint32 GetTotalActiveFires() const;
	void ExtinguishPoint(CVector point, float range);
	bool ExtinguishPointWithWater(CVector point, float range);
	int32 StartScriptFire(const CVector &pos, CEntity *target, float strength, uint8 propagation);
	bool IsScriptFireExtinguish(int16 index);
	void RemoveAllScriptFires(void);
	void RemoveScriptFire(int16 index);
	void SetScriptFireAudio(int16 index, bool state);
};
extern CFireManager gFireManager;
