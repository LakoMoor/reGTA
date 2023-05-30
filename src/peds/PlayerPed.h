#pragma once

#include "Ped.h"

class CPad;
class CCopPed;
class CWanted;

class CPlayerPed : public CPed
{
public:
	CWanted *m_pWanted;
	CCopPed *m_pArrestingCop;
	float m_fMoveSpeed;
	float m_fCurrentStamina;
	float m_fMaxStamina;
	float m_fStaminaProgress;
	int8 m_nSelectedWepSlot;
	bool m_bSpeedTimerFlag;
	uint8 m_nEvadeAmount;
	uint32 m_nSpeedTimer; // m_nStandStillTimer?
	uint32 m_nHitAnimDelayTimer; // m_nShotDelay?
	float m_fAttackButtonCounter;
	bool m_bHaveTargetSelected;	// may have better name
	CEntity *m_pEvadingFrom;	// is this CPhysical?
	int32 m_nTargettableObjects[4];
	uint32 m_nAdrenalineTime;
	uint8 m_nDrunkenness;             // Needed to work out whether we lost target this frame
	uint8 m_nFadeDrunkenness;
	uint8 m_nDrunkCountdown; //countdown in frames when the drunk effect ends
	bool m_bAdrenalineActive;
	bool m_bHasLockOnTarget;
	bool m_bCanBeDamaged;
	bool m_bNoPosForMeleeAttack;
	bool unk1;
	CVector m_vecSafePos[6]; // safe places from the player, for example behind a tree
	CPed *m_pPedAtSafePos[6];
	CPed *m_pMeleeList[6]; // reachable peds at each direction(6)
	int16 m_nAttackDirToCheck;
	float m_fWalkAngle; //angle between heading and walking direction
	float m_fFPSMoveHeading;
	RpAtomic* m_pMinigunTopAtomic; //atomic for the spinning part of the minigun model
	float m_fGunSpinSpeed; // for minigun
	float m_fGunSpinAngle;
	unsigned int m_nPadDownPressedInMilliseconds;
	unsigned int m_nLastBusFareCollected;
#ifdef FREE_CAM
	bool m_bFreeAimActive;
	CVector m_cachedCamSource;
	CVector m_cachedCamFront;
	CVector m_cachedCamUp;
#endif

	static bool bDontAllowWeaponChange;
#ifndef MASTER
	static bool bDebugPlayerInfo;
#endif

	CPlayerPed();
	~CPlayerPed();
	void SetMoveAnim() { };

	void ReApplyMoveAnims(void);
	void ClearWeaponTarget(void);
	void SetWantedLevel(int32 level);
	void SetWantedLevelNoDrop(int32 level);
	void KeepAreaAroundPlayerClear(void);
	void AnnoyPlayerPed(bool);
	void MakeChangesForNewWeapon(int32);
	void MakeChangesForNewWeapon(eWeaponType);
	void SetInitialState(void);
	void ProcessControl(void);
	void ClearAdrenaline(void);
	void UseSprintEnergy(void);
	class CPlayerInfo *GetPlayerInfoForThisPlayerPed();
	void SetRealMoveAnim(void);
	void RestoreSprintEnergy(float);
	float DoWeaponSmoothSpray(void);
	void DoStuffToGoOnFire(void);
	bool DoesTargetHaveToBeBroken(CVector, CWeapon*);
	void RunningLand(CPad*);
	bool IsThisPedAnAimingPriority(CPed*);
	void PlayerControlSniper(CPad*);
	void PlayerControlM16(CPad*);
	void PlayerControlFighter(CPad*);
	void ProcessWeaponSwitch(CPad*);
	void MakeObjectTargettable(int32);
	void PlayerControl1stPersonRunAround(CPad *padUsed);
	void EvaluateNeighbouringTarget(CEntity*, CEntity**, float*, float, float, bool, bool);
	void EvaluateTarget(CEntity*, CEntity**, float*, float, float, bool);
	bool FindNextWeaponLockOnTarget(CEntity*, bool);
	bool FindWeaponLockOnTarget(void);
	void ProcessAnimGroups(void);
	void ProcessPlayerWeapon(CPad*);
	void PlayerControlZelda(CPad*);
	bool DoesPlayerWantNewWeapon(eWeaponType, bool);
	void PlayIdleAnimations(CPad*);
	void RemovePedFromMeleeList(CPed*);
	void GetMeleeAttackCoords(CVector&, int8, float);
	int32 FindMeleeAttackPoint(CPed*, CVector&, uint32&);
	bool CanIKReachThisTarget(CVector, CWeapon*, bool);
	void RotatePlayerToTrackTarget(void);
	bool MovementDisabledBecauseOfTargeting(void);
	void FindNewAttackPoints(void);
	void SetNearbyPedsToInteractWithPlayer(void);
	void UpdateMeleeAttackers(void);

	static void SetupPlayerPed(int32);
	static void DeactivatePlayerPed(int32);
	static void ReactivatePlayerPed(int32);

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif

	static const uint32 nSaveStructSize;
};

//VALIDATE_SIZE(CPlayerPed, 0x5F0);
