#pragma once
#include "ParticleMgr.h"


class CEntity;

class CParticle
{
public:
	enum
	{
		RAND_TABLE_SIZE    = 20,
		SIN_COS_TABLE_SIZE = 1024
	};

	CVector   m_vecPosition;
	CVector   m_vecVelocity;
	uint32    m_nTimeWhenWillBeDestroyed;
	uint32    m_nTimeWhenColorWillBeChanged;
	float     m_fZGround;
	CVector   m_vecParticleMovementOffset;
	int16     m_nCurrentZRotation;
	uint16    m_nZRotationTimer;
	float     m_fCurrentZRadius;
	uint16    m_nZRadiusTimer;
	uint8     m_nColorIntensity;
	uint8     m_nAlpha;
	float     m_fSize;
	float     m_fExpansionRate;
	int16     m_nFadeToBlackTimer;
	int16     m_nFadeAlphaTimer;
	int16     m_nAnimationSpeedTimer;
	int16     m_nRotationStep;
	int16     m_nRotation;
	uint8     m_nCurrentFrame;
	RwRGBA    m_Color;
	CParticle *m_pNext;
	
	CParticle()
	{
		;
	}
	
	~CParticle()
	{
		;
	}

	static float      ms_afRandTable[RAND_TABLE_SIZE];
	static CParticle *m_pUnusedListHead;
	
	static float      m_SinTable[SIN_COS_TABLE_SIZE];
	static float      m_CosTable[SIN_COS_TABLE_SIZE];
	
	static float Sin(int32 value) { return m_SinTable[value]; }
	static float Cos(int32 value) { return m_CosTable[value]; }
	
	static void ReloadConfig();
	static void Initialise();
	static void Shutdown();
	
	static void AddParticlesAlongLine(tParticleType type, CVector const &vecStart, CVector const &vecEnd, CVector const &vecDir, float fPower, CEntity *pEntity = nil, float fSize = 0.0f,                     int32 nRotationSpeed = 0, int32 nRotation = 0, int32 nCurFrame = 0, int32 nLifeSpan = 0);	
	static void AddParticlesAlongLine(tParticleType type, CVector const &vecStart, CVector const &vecEnd, CVector const &vecDir, float fPower, CEntity *pEntity,       float fSize, RwRGBA const&color,        int32 nRotationSpeed = 0, int32 nRotation = 0, int32 nCurFrame = 0, int32 nLifeSpan = 0);

	static CParticle *AddParticle(tParticleType type, CVector const &vecPos, CVector const &vecDir, CEntity *pEntity = nil, float fSize = 0.0f,               int32 nRotationSpeed = 0, int32 nRotation = 0, int32 nCurFrame = 0, int32 nLifeSpan = 0);
	static CParticle *AddParticle(tParticleType type, CVector const &vecPos, CVector const &vecDir, CEntity *pEntity,       float fSize, RwRGBA const &color, int32 nRotationSpeed = 0, int32 nRotation = 0, int32 nCurFrame = 0, int32 nLifeSpan = 0);

	static void Update();
	static void Render();

	static void RemovePSystem(tParticleType type);
	static void RemoveParticle(CParticle *pParticle, CParticle *pPrevParticle, tParticleSystemData *pPSystemData);
	
	static void _Next(CParticle *&pParticle, CParticle *&pPrevParticle, tParticleSystemData *pPSystemData, bool bRemoveParticle)
	{
		if ( bRemoveParticle )
		{
			RemoveParticle(pParticle, pPrevParticle, pPSystemData);
					
			if ( pPrevParticle )
				pParticle = pPrevParticle->m_pNext;
			else
				pParticle = pPSystemData->m_pParticles;
		}
		else
		{
			pPrevParticle = pParticle;
			pParticle = pParticle->m_pNext;
		}
	}

	static void AddJetExplosion(CVector const &vecPos, float fPower, float fSize);
	static void AddYardieDoorSmoke(CVector const &vecPos, CMatrix const &matMatrix);
	static void CalWindDir(CVector *vecDirIn, CVector *vecDirOut);
	
	static void HandleShipsAtHorizonStuff();
	static void HandleShootableBirdsStuff(CEntity *entity, CVector const&camPos);
};

extern bool clearWaterDrop;
extern int32 numWaterDropOnScreen;
extern RwRaster *gpCarSplashRaster[];
extern RwRaster *gpHeatHazeRaster;
extern RwRaster *gpDotRaster;
extern RwRaster *gpRainDripRaster[];
extern RwRaster *gpRainDripDarkRaster[];

VALIDATE_SIZE(CParticle, 0x58);
