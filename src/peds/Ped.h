#pragma once

#include "RwHelper.h"
#include "AnimManager.h"
#include "Crime.h"
#include "EventList.h"
#include "PedIK.h"
#include "PedType.h"
#include "Physical.h"
#include "Weapon.h"
#include "WeaponInfo.h"
#include "PathFind.h"
#include "Collision.h"

#define FEET_OFFSET	1.04f
#define CHECK_NEARBY_THINGS_MAX_DIST	15.0f
#define ENTER_CAR_MAX_DIST	30.0f
#define CAN_SEE_ENTITY_ANGLE_THRESHOLD	DEGTORAD(60.0f)

class CAccident;
class CObject;
class CFire;
struct AnimBlendFrameData;
class CAnimBlendAssociation;
class CPedAttractor;

struct PedAudioData
{
	int m_nFixedDelayTime;
	int m_nOverrideFixedDelayTime;
	int m_nOverrideMaxRandomDelayTime;
	int m_nMaxRandomDelayTime;
};

enum
{
	ATTACK_IN_PROGRESS,
	CANT_ATTACK,
	WATCH_UNTIL_HE_DISAPPEARS,
};

enum eFormation
{
	FORMATION_UNDEFINED,
	FORMATION_REAR,
	FORMATION_REAR_LEFT,
	FORMATION_REAR_RIGHT,
	FORMATION_FRONT_LEFT,
	FORMATION_FRONT_RIGHT,
	FORMATION_LEFT,
	FORMATION_RIGHT,
	FORMATION_FRONT
};

enum FightState {
	FIGHTSTATE_MOVE_FINISHED = -2,
	FIGHTSTATE_JUST_ATTACKED,
	FIGHTSTATE_NO_MOVE,
	FIGHTSTATE_1
};

enum
{
	ENDFIGHT_NORMAL,
	ENDFIGHT_WITH_A_STEP,
	ENDFIGHT_FAST
};

enum PedRouteType
{
	PEDROUTE_STOP_WHEN_DONE = 1,
	PEDROUTE_GO_BACKWARD_WHEN_DONE,
	PEDROUTE_GO_TO_START_WHEN_DONE
};

enum FightMoveHitLevel
{
	HITLEVEL_NULL,
	HITLEVEL_GROUND,
	HITLEVEL_LOW,
	HITLEVEL_MEDIUM,
	HITLEVEL_HIGH
};

struct FightMove
{
	AnimationId animId;
	float startFireTime;
	float endFireTime;
	float comboFollowOnTime;
	float strikeRadius;
	float extendReachMultiplier;
	uint8 hitLevel; // FightMoveHitLevel
	uint8 damage;
	uint8 flags;
};

// TODO: This is eFightState on mobile.
enum PedFightMoves
{
	FIGHTMOVE_NULL,
	// Attacker
	FIGHTMOVE_STDPUNCH,
	FIGHTMOVE_IDLE,
	FIGHTMOVE_SHUFFLE_F,
	FIGHTMOVE_KNEE,
	FIGHTMOVE_PUNCHHOOK,
	FIGHTMOVE_PUNCHJAB,
	FIGHTMOVE_PUNCH,
	FIGHTMOVE_LONGKICK,
	FIGHTMOVE_ROUNDHOUSE,
	// Directionals
	FIGHTMOVE_FWDLEFT,
	FIGHTMOVE_FWDRIGHT,
	FIGHTMOVE_BACKKICK,
	FIGHTMOVE_BACKFLIP,
	FIGHTMOVE_BACKLEFT,
	FIGHTMOVE_BACKRIGHT,
	FIGHTMOVE_RIGHTSWEEP,
	// Special
	FIGHTMOVE_GROUNDKICK,
	// Opponent
	FIGHTMOVE_HITFRONT,
	FIGHTMOVE_HITBACK,
	FIGHTMOVE_HITRIGHT,
	FIGHTMOVE_HITLEFT,
	FIGHTMOVE_HITBODY,
	FIGHTMOVE_HITCHEST,
	FIGHTMOVE_HITHEAD,
	FIGHTMOVE_HITBIGSTEP,
	FIGHTMOVE_HITONFLOOR,
	FIGHTMOVE_HITBEHIND,
	FIGHTMOVE_MELEE1,
	FIGHTMOVE_MELEE2,
	FIGHTMOVE_MELEE3,
	FIGHTMOVE_IDLE2NORM,
	NUM_FIGHTMOVES
};

enum ePedPieceTypes
{
	PEDPIECE_TORSO,
	PEDPIECE_MID,
	PEDPIECE_LEFTARM,
	PEDPIECE_RIGHTARM,
	PEDPIECE_LEFTLEG,
	PEDPIECE_RIGHTLEG,
	PEDPIECE_HEAD,
};

enum eWaitState {
	WAITSTATE_FALSE,
	WAITSTATE_TRAFFIC_LIGHTS,
	WAITSTATE_CROSS_ROAD,
	WAITSTATE_CROSS_ROAD_LOOK,
	WAITSTATE_LOOK_PED,
	WAITSTATE_LOOK_SHOP,
	WAITSTATE_LOOK_ACCIDENT,
	WAITSTATE_FACEOFF_GANG,
	WAITSTATE_DOUBLEBACK,
	WAITSTATE_HITWALL,
	WAITSTATE_TURN180,
	WAITSTATE_SURPRISE,
	WAITSTATE_STUCK,
	WAITSTATE_LOOK_ABOUT,
	WAITSTATE_PLAYANIM_DUCK,
	WAITSTATE_PLAYANIM_COWER,
	WAITSTATE_PLAYANIM_TAXI,
	WAITSTATE_PLAYANIM_HANDSUP,
	WAITSTATE_PLAYANIM_HANDSCOWER,
	WAITSTATE_PLAYANIM_CHAT,
	WAITSTATE_FINISH_FLEE,
	WAITSTATE_SIT_DOWN,
	WAITSTATE_SIT_DOWN_RVRS,
	WAITSTATE_SIT_UP,
	WAITSTATE_SIT_IDLE,
	WAITSTATE_USE_ATM,
	WAITSTATE_SUN_BATHE_PRE,
	WAITSTATE_SUN_BATHE_DOWN,
	WAITSTATE_SUN_BATHE_IDLE,
	WAITSTATE_RIOT,
	WAITSTATE_FAST_FALL,
	WAITSTATE_BOMBER,
	WAITSTATE_STRIPPER,
	WAITSTATE_GROUND_ATTACK,
	WAITSTATE_LANCESITTING,
	WAITSTATE_PLAYANIM_HANDSUP_SIMPLE,
};

enum eObjective {
	OBJECTIVE_NONE,
	OBJECTIVE_WAIT_ON_FOOT,
	OBJECTIVE_WAIT_ON_FOOT_FOR_COP,
	OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE,
	OBJECTIVE_GUARD_SPOT,
	OBJECTIVE_GUARD_AREA,
	OBJECTIVE_WAIT_IN_CAR,
	OBJECTIVE_WAIT_IN_CAR_THEN_GET_OUT,
	OBJECTIVE_KILL_CHAR_ON_FOOT,
	OBJECTIVE_KILL_CHAR_ANY_MEANS,
	OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE,
	OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS,
	OBJECTIVE_GOTO_CHAR_ON_FOOT,
	OBJECTIVE_GOTO_CHAR_ON_FOOT_WALKING,
	OBJECTIVE_HASSLE_CHAR,
	OBJECTIVE_FOLLOW_CHAR_IN_FORMATION,
	OBJECTIVE_LEAVE_CAR,
	OBJECTIVE_ENTER_CAR_AS_PASSENGER,
	OBJECTIVE_ENTER_CAR_AS_DRIVER,
	OBJECTIVE_FOLLOW_CAR_IN_CAR,
	OBJECTIVE_FIRE_AT_OBJECT_FROM_VEHICLE,
	OBJECTIVE_DESTROY_OBJECT,
	OBJECTIVE_DESTROY_CAR,
	OBJECTIVE_GOTO_AREA_ANY_MEANS,
	OBJECTIVE_GOTO_AREA_ON_FOOT,
	OBJECTIVE_RUN_TO_AREA,
	OBJECTIVE_GOTO_AREA_IN_CAR,
	OBJECTIVE_FOLLOW_CAR_ON_FOOT_WITH_OFFSET,
	OBJECTIVE_GUARD_ATTACK,
	OBJECTIVE_SET_LEADER,
	OBJECTIVE_FOLLOW_ROUTE,
	OBJECTIVE_SOLICIT_VEHICLE,
	OBJECTIVE_HAIL_TAXI,
	OBJECTIVE_CATCH_TRAIN,
	OBJECTIVE_BUY_ICE_CREAM,
	OBJECTIVE_STEAL_ANY_CAR,
	OBJECTIVE_STEAL_ANY_MISSION_CAR,
	OBJECTIVE_MUG_CHAR,
	OBJECTIVE_LEAVE_CAR_AND_DIE,
	OBJECTIVE_GOTO_SEAT_ON_FOOT,
	OBJECTIVE_GOTO_ATM_ON_FOOT,
	OBJECTIVE_FLEE_CAR,
	OBJECTIVE_SUN_BATHE,
	OBJECTIVE_GOTO_BUS_STOP_ON_FOOT,
	OBJECTIVE_GOTO_PIZZA_ON_FOOT,
	OBJECTIVE_GOTO_SHELTER_ON_FOOT,
	OBJECTIVE_AIM_GUN_AT,
	OBJECTIVE_WANDER,
	OBJECTIVE_WAIT_ON_FOOT_AT_SHELTER,
	OBJECTIVE_SPRINT_TO_AREA,
	OBJECTIVE_KILL_CHAR_ON_BOAT,
	OBJECTIVE_SOLICIT_FOOT,
	OBJECTIVE_WAIT_ON_FOOT_AT_BUS_STOP,
	OBJECTIVE_GOTO_ICE_CREAM_VAN_ON_FOOT,
	OBJECTIVE_WAIT_ON_FOOT_AT_ICE_CREAM_VAN,
	OBJ_55,
	OBJ_56,
	OBJ_57,
	OBJ_58,
	OBJ_59

};

enum {
	RANDOM_CHAR = 1,
	MISSION_CHAR,
	UNK_CHAR,
};

enum PedLineUpPhase {
	LINE_UP_TO_CAR_START,
	LINE_UP_TO_CAR_END,
	LINE_UP_TO_CAR_2, // Buggy. Used for cops arresting you from passenger door
	LINE_UP_TO_CAR_FALL
};

enum PedOnGroundState {
	NO_PED,
	PED_IN_FRONT_OF_ATTACKER,
	PED_ON_THE_FLOOR,
	PED_DEAD_ON_THE_FLOOR
};

enum PointBlankNecessity {
	NO_POINT_BLANK_PED,
	POINT_BLANK_FOR_WANTED_PED,
	POINT_BLANK_FOR_SOMEONE_ELSE
};

enum PedState
{
	PED_NONE,
	PED_IDLE,
	PED_LOOK_ENTITY,
	PED_LOOK_HEADING,
	PED_WANDER_RANGE,
	PED_WANDER_PATH,
	PED_SEEK_POS,
	PED_SEEK_ENTITY,
	PED_FLEE_POS,
	PED_FLEE_ENTITY,
	PED_PURSUE,
	PED_FOLLOW_PATH,
	PED_SNIPER_MODE,
	PED_ROCKET_MODE,
	PED_DUMMY,
	PED_PAUSE,
	PED_ATTACK,
	PED_FIGHT,
	PED_FACE_PHONE,
	PED_MAKE_CALL,
	PED_CHAT,
	PED_MUG,
	PED_AIM_GUN,
	PED_AI_CONTROL,
	PED_SEEK_CAR,
	PED_SEEK_IN_BOAT,
	PED_FOLLOW_ROUTE,
	PED_CPR,
	PED_SOLICIT,
	PED_BUY_ICECREAM,
	PED_INVESTIGATE,
	PED_STEP_AWAY,
	PED_ON_FIRE,
	PED_SUN_BATHE,
	PED_FLASH,
	PED_JOG,
	PED_ANSWER_MOBILE,

	PED_UNKNOWN,	// Same with IDLE, but also infects up to 5 peds with same pedType and WANDER_PATH, so they become stone too. HANG_OUT in Fire_Head's idb

	PED_STATES_NO_AI,

	PED_ABSEIL,
	PED_SIT,
	PED_JUMP,
	PED_FALL,
	PED_GETUP,
	PED_STAGGER,
	PED_DIVE_AWAY,

	PED_STATES_NO_ST,
	PED_ENTER_TRAIN,
	PED_EXIT_TRAIN,
	PED_ARREST_PLAYER,

	PED_DRIVING,
	PED_PASSENGER,
	PED_TAXI_PASSENGER,
	PED_OPEN_DOOR,
	PED_DIE,
	PED_DEAD,
	PED_CARJACK,
	PED_DRAG_FROM_CAR,
	PED_ENTER_CAR,
	PED_STEAL_CAR,
	PED_EXIT_CAR,
	PED_HANDS_UP,
	PED_ARRESTED,
	PED_DEPLOY_STINGER
};

enum eMoveState {
	PEDMOVE_NONE,
	PEDMOVE_STILL,
	PEDMOVE_WALK,
	PEDMOVE_JOG,
	PEDMOVE_RUN,
	PEDMOVE_SPRINT,
	PEDMOVE_THROWN
};

extern float gfTommyFatness;

class CVehicle;

class CPed : public CPhysical
{
public:
#ifdef USE_CUTSCENE_SHADOW_FOR_PED
	class CCutsceneShadow *m_pRTShadow;
#endif
	// 0x128
	CStoredCollPoly m_collPoly;
	float m_fCollisionSpeed;

	// cf. https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CPed.h from R*
	uint32 bIsStanding : 1;
	uint32 bWasStanding : 1;
	uint32 bIsAttacking : 1;		// doesn't reset after fist fight
	uint32 bIsPointingGunAt : 1;
	uint32 bIsLooking : 1;
	uint32 bKeepTryingToLook : 1; // if we can't look somewhere due to unreachable angles
	uint32 bIsRestoringLook : 1;
	uint32 bIsAimingGun : 1;

	uint32 bIsRestoringGun : 1;
	uint32 bCanPointGunAtTarget : 1;
	uint32 bIsTalking : 1;
	uint32 bIsInTheAir : 1;
	uint32 bIsLanding : 1;
	uint32 bIsRunning : 1; // on some conditions
	uint32 bHitSomethingLastFrame : 1;
	uint32 bVehEnterDoorIsBlocked : 1; // because someone else enters/exits from there

	uint32 bCanPedEnterSeekedCar : 1;
	uint32 bRespondsToThreats : 1;
	uint32 bRenderPedInCar : 1;
	uint32 bChangedSeat : 1;
	uint32 bUpdateAnimHeading : 1;
	uint32 bBodyPartJustCameOff : 1;
	uint32 bIsShooting : 1;
	uint32 bFindNewNodeAfterStateRestore : 1;

	uint32 bHasACamera : 1; // does ped possess a camera to document accidents involves fire/explosion
	uint32 bGonnaInvestigateEvent : 1;
	uint32 bPedIsBleeding : 1;
	uint32 bStopAndShoot : 1; // Ped cannot reach target to attack with fist, need to use gun
	uint32 bIsPedDieAnimPlaying : 1;
	uint32 bUsePedNodeSeek : 1;
	uint32 bObjectiveCompleted : 1;
	uint32 bScriptObjectiveCompleted : 1;

	uint32 bKindaStayInSamePlace : 1;
	uint32 bBeingChasedByPolice : 1;
	uint32 bNotAllowedToDuck : 1;
	uint32 bCrouchWhenShooting : 1;
	uint32 bIsDucking : 1;
	uint32 bGetUpAnimStarted : 1;
	uint32 bDoBloodyFootprints : 1;
	uint32 bFleeAfterExitingCar : 1;

	uint32 bWanderPathAfterExitingCar : 1;
	uint32 bIsLeader : 1;
	uint32 bDontDragMeOutCar : 1; // unfinished feature
	uint32 m_ped_flagF8 : 1;
	uint32 bWillBeQuickJacked : 1;
	uint32 bCancelEnteringCar : 1; // after door is opened or couldn't be opened due to it's locked
	uint32 bObstacleShowedUpDuringKillObjective : 1;
	uint32 bDuckAndCover : 1;

	uint32 bStillOnValidPoly : 1; // set if the polygon the ped is on is still valid for collision
	uint32 bAllowMedicsToReviveMe : 1;
	uint32 bResetWalkAnims : 1;
	uint32 bStartWanderPathOnFoot : 1; // exits the car if he's in it, reset after path found
	uint32 bOnBoat : 1; // not just driver, may be just standing
	uint32 bBusJacked : 1;
	uint32 bGonnaKillTheCarJacker : 1; // only set when car is jacked from right door and when arrested by police
	uint32 bFadeOut : 1;

	uint32 bKnockedUpIntoAir : 1; // has ped been knocked up into the air by a car collision
	uint32 bHitSteepSlope : 1; // has ped collided/is standing on a steep slope (surface type)
	uint32 bCullExtraFarAway : 1; // special ped only gets culled if it's extra far away (for roadblocks)
	uint32 bClearObjective : 1;
	uint32 bTryingToReachDryLand : 1; // has ped just exited boat and trying to get to dry land
	uint32 bCollidedWithMyVehicle : 1;
	uint32 bRichFromMugging : 1; // ped has lots of cash cause they've been mugging people
	uint32 bChrisCriminal : 1; // Is a criminal as killed during Chris' police mission (should be counted as such)

	uint32 bShakeFist : 1;  // test shake hand at look entity
	uint32 bNoCriticalHits : 1; // if set, limbs won't came off
	uint32 bVehExitWillBeInstant : 1;
	uint32 bHasAlreadyBeenRecorded : 1;
	uint32 bFallenDown : 1;
	uint32 bDontAcceptIKLookAts : 1;
	uint32 bReachedAttractorHeadingTarget : 1;
	uint32 bTurnedAroundOnAttractor : 1;

	uint32 bHasAlreadyUsedAttractor : 1;
	uint32 bHasAlreadyStoleACar : 1;
	uint32 bCarPassenger : 1;
	uint32 bFleeWhenStanding : 1;
	uint32 bGotUpOfMyOwnAccord : 1;
	uint32 bMiamiViceCop : 1;
	uint32 bMoneyHasBeenGivenByScript : 1; //
	uint32 bHasBeenPhotographed : 1;  //

	uint32 bIsDrowning : 1;
	uint32 bDrownsInWater : 1;
	uint32 bWaitForLeaderToComeCloser : 1;
	uint32 bHeldHostageInCar : 1;
	uint32 bIsPlayerFriend : 1;
	uint32 bHeadStuckInCollision : 1;
	uint32 bDeadPedInFrontOfCar : 1;
	uint32 bStayInCarOnJack : 1;

	uint32 bDontFight : 1;
	uint32 bDoomAim : 1;
	uint32 bCanBeShotInVehicle : 1;
	uint32 bCanGiveUpSunbathing : 1;
	uint32 bMakeFleeScream : 1;
	uint32 bPushedAlongByCar : 1;
	uint32 bRemoveMeWhenIGotIntoCar : 1;
	uint32 bIgnoreThreatsBehindObjects : 1;

	uint32 bNeverEverTargetThisPed : 1;
	uint32 bCrouchWhenScared : 1;
	uint32 bKnockedOffBike : 1;
	uint32 b158_8 : 1;
	uint32 bCollectBusFare : 1;
	uint32 bBoughtIceCream : 1;
	uint32 bDonePositionOutOfCollision : 1;
	uint32 bCanAttackPlayerWithCops : 1;

#ifdef KANGAROO_CHEAT
	// our own flags
	uint32 m_ped_flagI80 : 1; // KANGAROO_CHEAT define makes use of this as cheat toggle 
#endif

	uint8 m_gangFlags;
	uint8 m_unused15D; // these 3 can't be padding but had to actually have been members ...
	uint8 m_unused15E;
	uint8 m_unused15F;
	uint8 CharCreatedBy;
	eObjective m_objective;
	eObjective m_prevObjective;
	CPed *m_pedInObjective;
	CVehicle *m_carInObjective;
	CVector m_nextRoutePointPos;
	float m_attractorHeading;
	CPed *m_leader;
	eFormation m_pedFormation;
	uint32 m_fearFlags;
	CEntity *m_threatEntity;
	CVector2D m_eventOrThreat;
	uint32 m_eventType;
	CEntity* m_pEventEntity;
	float m_fAngleToEvent;
	AnimBlendFrameData *m_pFrames[PED_NODE_MAX];
	RpAtomic *m_pWeaponModel;
	AssocGroupId m_animGroup;
	CAnimBlendAssociation *m_pVehicleAnim;
	CVector2D m_vecAnimMoveDelta;
	CVector m_vecOffsetSeek;
	CPedIK m_pedIK; 
	float m_actionX;
	float m_actionY;
	uint32 m_nPedStateTimer;
	PedState m_nPedState;
	PedState m_nLastPedState;
	eMoveState m_nMoveState;
	int32 m_nStoredMoveState;
	int32 m_nPrevMoveState;
	eWaitState m_nWaitState;
	uint32 m_nWaitTimer;
	CPathNode* m_pathNodesToGo[8];
	int16 m_nNumPathNodes;
	int16 m_nCurPathNodeId;
	CEntity* m_followPathWalkAroundEnt;
	CEntity* m_followPathTargetEnt;
	uint32 m_pathNodeTimer;
	CPathNode m_pathNodeObjPool[8];
	CPathNode* m_pCurPathNode;
	int8 m_nPathDir;
	CPathNode* m_pLastPathNode;
	CPathNode* m_pNextPathNode;
	CVector m_followPathDestPos;
	float m_followPathAbortDist;
	eMoveState m_followPathMoveState;
	float m_fHealth;
	float m_fArmour;
	uint32 m_nExtendedRangeTimer;
	int16 m_routeLastPoint;
	uint16 m_routeStartPoint;
	int16 m_routePointsPassed;
	int16 m_routeType;	// See PedRouteType
	int16 m_routePointsBeingPassed;
	CVector2D m_moved;
	float m_fRotationCur;
	float m_fRotationDest;
	float m_headingRate;
	uint16 m_vehDoor;
	int16 m_walkAroundType;
	CPhysical *m_pCurrentPhysSurface;
	CVector m_vecOffsetFromPhysSurface;
	CEntity *m_pCurSurface;
	CVector m_vecSeekPos;
	CEntity *m_pSeekTarget;
	CVehicle *m_pMyVehicle;
	bool bInVehicle;
	float m_distanceToCountSeekDone;
	float m_acceptableHeadingOffset;
	CVehicle* m_vehicleInAccident;
	CPedAttractor* m_attractor;
	int32 m_positionInQueue;
	bool bRunningToPhone;
	int16 m_phoneId;
	eCrimeType m_crimeToReportOnPhone;
	uint32 m_phoneTalkTimer;
	CAccident *m_lastAccident;
	uint32 m_nPedType;
	CPedStats *m_pedStats;
	CVector2D m_fleeFromPos;
	CEntity *m_fleeFrom;
	uint32 m_fleeTimer;
	CEntity* m_threatEx; // TODO(Miami): What is this?
	CEntity* m_collidingEntityWhileFleeing;
	uint32 m_collidingThingTimer;
	CEntity *m_pCollidingEntity;
	uint8 m_stateUnused;
	uint32 m_timerUnused;
	class CRange2D *m_wanderRangeBounds;
	CWeapon m_weapons[TOTAL_WEAPON_SLOTS];
	eWeaponType m_storedWeapon;
	eWeaponType m_delayedWeapon;
	uint32 m_delayedWeaponAmmo;
	uint8 m_currentWeapon;			// eWeaponType
	uint8 m_maxWeaponTypeAllowed;	// eWeaponType
	uint8 m_wepSkills;
	uint8 m_wepAccuracy;
	CEntity *m_pPointGunAt;
	CVector m_vecHitLastPos;
	uint32 m_curFightMove;
	uint32 m_lastFightMove;
	uint8 m_fightButtonPressure;
	int8 m_fightState;
	bool m_takeAStepAfterAttack;
	uint8 m_bleedCounter;
	CFire *m_pFire;
	CEntity *m_pLookTarget;
	float m_fLookDirection;
	int32 m_wepModelID;
	uint32 m_leaveCarTimer;
	uint32 m_getUpTimer;
	uint32 m_lookTimer;
	uint32 m_chatTimer;
	uint32 m_attackTimer;
	uint32 m_shootTimer; // shooting is a part of attack
	uint32 m_carJackTimer;
	uint32 m_objectiveTimer;
	uint32 m_duckTimer;
	uint32 m_duckAndCoverTimer;
	uint32 m_bloodyFootprintCountOrDeathTime;	// Death time when bDoBloodyFootprints is false. Weird decision
	uint32 m_shotTime;
	uint32 m_ceaseAttackTimer;
	uint8 m_panicCounter;
	bool m_deadBleeding;
	int8 m_bodyPartBleeding;		// PedNode, but -1 if there isn't
	CPed *m_nearPeds[10];
	uint16 m_numNearPeds;
	uint16 m_nPedMoney;
 	int8 m_lastWepDam;
	CEntity *m_lastDamEntity;
	CEntity *m_attachedTo;
	CVector m_vecAttachOffset;
	uint16 m_attachType;
	float m_attachRotStep;
	uint32 m_attachWepAmmo;
	uint32 m_threatFlags;
	uint32 m_threatCheckTimer;
	uint32 m_threatCheckInterval;
	int32 m_delayedSoundID;
	uint32 m_delayedSoundTimer;
	uint32 m_lastSoundStart;
	uint32 m_soundStart;
	uint16 m_lastQueuedSound;
	uint16 m_queuedSound;
	bool m_canTalk;
	uint32 m_lastComment;
	CVector m_vecSpotToGuard;
	float m_radiusToGuard;

	static void *operator new(size_t) throw();
	static void *operator new(size_t, int) throw();
	static void operator delete(void*, size_t) throw();
	static void operator delete(void*, int) throw();

	CPed(uint32 pedType);
	~CPed(void);

	void DeleteRwObject();
	void SetModelIndex(uint32 mi);
	void ProcessControl(void);
	void Teleport(CVector);
	void PreRender(void);
	void Render(void);
	bool SetupLighting(void);
	void RemoveLighting(bool);
	void FlagToDestroyWhenNextProcessed(void);
	int32 ProcessEntityCollision(CEntity*, CColPoint*);

	virtual void SetMoveAnim(void);

	void AddWeaponModel(int id);
	void AimGun(void);
	void KillPedWithCar(CVehicle *veh, float impulse);
	void Say(uint16 audio);
	void Say(uint16 audio, int32 time);
	void SetLookFlag(CEntity* target, bool keepTryingToLook, bool cancelPrevious = false);
	void SetLookFlag(float direction, bool keepTryingToLook, bool cancelPrevious = false);
	void SetLookTimer(int time);
	void SetDie(AnimationId anim = ANIM_STD_KO_FRONT, float arg1 = 4.0f, float arg2 = 0.0f);
	void SetDead(void);
	void ApplyHeadShot(eWeaponType weaponType, CVector pos, bool evenOnPlayer);
	void RemoveBodyPart(PedNode nodeId, int8 direction);
	bool OurPedCanSeeThisOne(CEntity *target, bool shootablesDoBlock = false);
	void Avoid(void);
	void Attack(void);
	void ClearAimFlag(void);
	void ClearLookFlag(void);
	void RestorePreviousState(void);
	void ClearAttack(void);
	bool IsPedHeadAbovePos(float zOffset);
	void RemoveWeaponModel(int modelId);
	void SetCurrentWeapon(eWeaponType weaponType);
	void SetCurrentWeapon(int weapon);
	void Duck(void);
	void ClearDuck(bool = false);
	void ClearPointGunAt(void);
	void BeingDraggedFromCar(void);
	void RestartNonPartialAnims(void);
	void LineUpPedWithCar(PedLineUpPhase phase);
	void SetPedPositionInCar(void);
	void PlayFootSteps(void);
	void QuitEnteringCar(void);
	void BuildPedLists(void);
	int32 GiveWeapon(eWeaponType weaponType, uint32 ammo, bool unused = true);
	void CalculateNewOrientation(void);
	float WorkOutHeadingForMovingFirstPerson(float);
	void CalculateNewVelocity(void);
	bool CanSeeEntity(CEntity*, float threshold = CAN_SEE_ENTITY_ANGLE_THRESHOLD);
	void RestorePreviousObjective(void);
	void SetIdle(void);
#ifdef _MSC_VER
#if _MSC_VER >= 1920 && _MSC_VER < 1929
	__declspec(noinline) // workaround for a compiler bug, hooray MS :P
#endif
#endif
	void SetObjective(eObjective, void*);
	void SetObjective(eObjective);
	void SetObjective(eObjective, int16, int16);
	void SetObjective(eObjective, CVector);
	void SetObjective(eObjective, float, const CVector&);
	void ClearChat(void);
	void InformMyGangOfAttack(CEntity*);
	void ReactToAttack(CEntity*);
	void SetDuck(uint32, bool = false);
	void RegisterThreatWithGangPeds(CEntity*);
	bool TurnBody(void);
	void Chat(void);
	void CheckAroundForPossibleCollisions(void);
	void SetSeek(CVector, float);
	void SetSeek(CEntity*, float);
	bool MakePhonecall(void);
	bool FacePhone(void);
	CPed *CheckForDeadPeds(void);
	bool CheckForExplosions(CVector2D &area);
	CPed *CheckForGunShots(void);
	uint8 CheckForPointBlankPeds(CPed*);
	bool CheckIfInTheAir(void);
	void ClearAll(void);
	void SetPointGunAt(CEntity*);
	bool Seek(void);
	bool SetWanderPath(int8);
	bool SetFollowPath(CVector dest, float radius, eMoveState state, CEntity*, CEntity*, int);
	bool SetFollowPathStatic(void);
	bool SetFollowPathDynamic(void);
	void ClearAttackByRemovingAnim(void);
	void SetStoredState(void);
	void StopNonPartialAnims(void);
	bool InflictDamage(CEntity*, eWeaponType, float, ePedPieceTypes, uint8);
	void ClearFlee(void);
	void ClearFall(void);
	void SetGetUp(void);
	void ClearInvestigateEvent(void);
	void ClearLeader(void);
	void ClearLook(void);
	void ClearObjective(void);
	void ClearPause(void);
	void ClearSeek(void);
	void ClearWeapons(void);
	void RestoreGunPosition(void);
	void RestoreHeadingRate(void);
	void SetAimFlag(CEntity* to);
	void SetAimFlag(float angle);
	void SetAmmo(eWeaponType weaponType, uint32 ammo);
	void SetEvasiveStep(CPhysical*, uint8);
	void GrantAmmo(eWeaponType, uint32);
	void SetEvasiveDive(CPhysical*, uint8);
	void SetAttack(CEntity*);
	void StartFightAttack(uint8);
	void SetWaitState(eWaitState, void*);
	bool FightStrike(CVector&, bool);
	void FightHitPed(CPed*, CVector&, CVector&, int16);
	int32 ChooseAttackPlayer(uint8, bool);
	int32 ChooseAttackAI(uint8, bool);
	int GetLocalDirection(const CVector2D &);
	void StartFightDefend(uint8, uint8, uint8);
	void PlayHitSound(CPed*);
	void SetFall(int, AnimationId, uint8);
	void SetFlee(CEntity*, int);
	void SetFlee(CVector2D const &, int);
	void RemoveDrivebyAnims(void);
	void RemoveInCarAnims(void);
	void CollideWithPed(CPed*);
	void SetDirectionToWalkAroundObject(CEntity*);
	bool SetDirectionToWalkAroundVehicle(CVehicle*);
	void RemoveWeaponAnims(int, float);
	void CreateDeadPedMoney(void);
	void CreateDeadPedWeaponPickups(void);
	void CreateDeadPedPickupCoors(float *x, float *y, float *z);
	void SetAttackTimer(uint32);
	void SetBeingDraggedFromCar(CVehicle*, uint32, bool);
	void SetRadioStation(void);
	void SetBuyIceCream(void);
	void SetChat(CEntity*, uint32);
	void DeadPedMakesTyresBloody(void);
	void MakeTyresMuddySectorList(CPtrList&);
	bool DuckAndCover(void);
	void EndFight(uint8);
	void EnterCar(void);
	uint8 GetNearestTrainPedPosition(CVehicle*, CVector&);
	uint8 GetNearestTrainDoor(CVehicle*, CVector&);
	void ExitCar(void);
	void Fight(void);
	bool FindBestCoordsFromNodes(CVector, CVector*);
	void Wait(void);
	void ProcessObjective(void);
	CVector *SeekFollowingPath(void);
	void Flee(void);
	void FollowPath(void);
	CVector GetFormationPosition(void);
	void GetNearestDoor(CVehicle*, CVector&);
	bool GetNearestPassengerDoor(CVehicle*, CVector&);
	int GetNextPointOnRoute(void);
	int GetWeaponSlot(eWeaponType);
	bool CanWeRunAndFireWithWeapon(void);
	void GoToNearestDoor(CVehicle*);
	bool HaveReachedNextPointOnRoute(float);
	void Idle(void);
	void InTheAir(void);
	void SetLanding(void);
	void InvestigateEvent(void);
	bool IsPedDoingDriveByShooting(void);
	bool IsRoomToBeCarJacked(void);
	void SetInvestigateEvent(eEventType, CVector2D, float, uint16, float);
	bool LookForInterestingNodes(void);
	void LookForSexyCars(void);
	void LookForSexyPeds(void);
	void Mug(void);
	void MoveHeadToLook(void);
	void Pause(void);
	void ProcessBuoyancy(void);
	void ServiceTalking(void);
	void SetJump(void);
	void WanderPath(void);
	void ReactToPointGun(CEntity*);
	void SeekCar(void);
	bool PositionPedOutOfCollision(void);
	bool PositionAnyPedOutOfCollision(void);
	bool RunToReportCrime(eCrimeType);
	bool PlacePedOnDryLand(void);
	bool PossiblyFindBetterPosToSeekCar(CVector*, CVehicle*);
	void UpdateFromLeader(void);
	uint32 ScanForThreats(void);
	void SetEnterCar(CVehicle*, uint32);
	bool WarpPedToNearEntityOffScreen(CEntity*);
	void SetExitCar(CVehicle*, uint32);
	void SetFormation(eFormation);
	bool WillChat(CPed*);
	void SetEnterCar_AllClear(CVehicle*, uint32, uint32);
	void SetSolicit(uint32 time);
	void ScanForInterestingStuff(void);
	void WarpPedIntoCar(CVehicle*);
	void SetCarJack(CVehicle*);
	bool WarpPedToNearLeaderOffScreen(void);
	void Solicit(void);
	void SetExitBoat(CVehicle*);
	void ClearFollowPath();
	void GiveDelayedWeapon(eWeaponType weapon, uint32 ammo);
	void RequestDelayedWeapon();
	void AddInCarAnims(CVehicle* car, bool isDriver);
	bool CanBeDamagedByThisGangMember(CPed*);
	void AnswerMobile(void);
	void BuyIceCream(void);
	void CheckThreatValidity(void);
	void ClearAnswerMobile(void);
	void SetAnswerMobile(void);
	void AttachPedToEntity(CEntity*, CVector, uint16, float, eWeaponType);
	void DettachPedFromEntity();
	void PedShuffle();
	void DriveVehicle();
	void PositionAttachedPed();
	bool CanUseTorsoWhenLooking();
	void ScanForDelayedResponseThreats();

	// Static methods
	static CVector GetLocalPositionToOpenCarDoor(CVehicle *veh, uint32 component, float offset);
	static CVector GetPositionToOpenCarDoor(CVehicle *veh, uint32 component, float seatPosMult);
	static CVector GetPositionToOpenCarDoor(CVehicle* veh, uint32 component);
	static void Initialise(void);
	static void SetAnimOffsetForEnterOrExitVehicle(void);
	static void LoadFightData(void);

	// Callbacks
	static void PedGetupCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedStaggerCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedEvadeCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishDieAnimCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishedWaitCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishLaunchCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishHitHeadCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimGetInCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorOpenCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimPullPedOutCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorCloseCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetInCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimAlignCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetDraggedOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimStepOutCarCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetInTrainCB(CAnimBlendAssociation *assoc, void *arg);
#ifdef GTA_TRAIN
	static void PedSetOutTrainCB(CAnimBlendAssociation *assoc, void *arg);
#endif
	static void FinishedAttackCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishedReloadCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishFightMoveCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimDoorCloseRollingCB(CAnimBlendAssociation *assoc, void *arg);
	static void FinishJumpCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedLandCB(CAnimBlendAssociation *assoc, void *arg);
	static void RestoreHeadingRateCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetQuickDraggedOutCarPositionCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetDraggedOutCarPositionCB(CAnimBlendAssociation *assoc, void *arg);
	static void DeleteSunbatheIdleAnimCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetPreviousStateCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedAnimShuffleCB(CAnimBlendAssociation *assoc, void *arg);
	static void PedSetGetInCarPositionCB(CAnimBlendAssociation* assoc, void* arg);

	bool IsPlayer(void) const;
	bool IsFemale(void) { return m_nPedType == PEDTYPE_CIVFEMALE || m_nPedType == PEDTYPE_PROSTITUTE; }
	bool UseGroundColModel(void);
	bool CanSetPedState(void);
	bool IsPedInControl(void);
	bool CanPedDriveOff(void);
	bool CanBeDeleted(void);
	bool CanBeDeletedEvenInVehicle(void);
	bool CanStrafeOrMouseControl(void);
	bool CanPedReturnToState(void);
	void SetMoveState(eMoveState);
	bool IsTemporaryObjective(eObjective objective);
	void SetObjectiveTimer(int);
	bool SelectGunIfArmed(void);
	bool IsPointerValid(void);
	void SortPeds(CPed**, int, int);
	void ForceStoredObjective(eObjective);
	void SetStoredObjective(void);
	void SetLeader(CEntity* leader);
	void SetPedStats(ePedStats);
	bool IsGangMember(void) const;
	void Die(void);
#ifdef GTA_TRAIN
	void EnterTrain(void);
	void ExitTrain(void);
	void SetExitTrain(CVehicle*);
	void SetPedPositionInTrain(void);
	void LineUpPedWithTrain(void);
	void SetEnterTrain(CVehicle*, uint32);
#endif
	void Fall(void);
	bool IsPedShootable(void);
	void Look(void);
	void SetInTheAir(void);
	void RestoreHeadPosition(void);
	void PointGunAt(void);
	bool ServiceTalkingWhenDead(void);
	void SetShootTimer(uint32);
	void SetSeekCar(CVehicle*, uint32);
	void SetSeekBoatPosition(CVehicle*);
	void WanderRange(void);
	void SetFollowRoute(int16, int16);
	void SeekBoatPosition(void);
	void UpdatePosition(void);
	CObject *SpawnFlyingComponent(int, int8);
	void SetCarJack_AllClear(CVehicle*, uint32, uint32);
	bool CanPedJumpThis(CEntity *unused, CVector *damageNormal = nil);
	void SetNewAttraction(CPedAttractor* pAttractor, const CVector& pos, float, float, int);
	void ClearWaitState(void);
	void Undress(const char*);
	void Dress(void);
	int32 KillCharOnFootMelee(CVector&, CVector&, CVector&);
	int32 KillCharOnFootArmed(CVector&, CVector&, CVector&);
	void SetLook(CEntity* to);
	void SetLook(float direction);

	bool HasWeaponSlot(uint8 slot) { return m_weapons[slot].m_eWeaponType != WEAPONTYPE_UNARMED; }
	CWeapon& GetWeapon(uint8 slot) { return m_weapons[slot]; }
	CWeapon *GetWeapon(void) { return &m_weapons[m_currentWeapon]; }

	PedState GetPedState(void) { return m_nPedState; }
	void SetPedState(PedState state) 
	{
		if (GetPedState() == PED_FOLLOW_PATH && state != PED_FOLLOW_PATH)
			ClearFollowPath();
		m_nPedState = state;
	}
	bool Dead(void) { return m_nPedState == PED_DEAD; }
	bool Dying(void) { return m_nPedState == PED_DIE; }
	bool DyingOrDead(void) { return m_nPedState == PED_DIE || m_nPedState == PED_DEAD; }
	bool OnGround(void) { return m_nPedState == PED_FALL || m_nPedState == PED_DIE || m_nPedState == PED_DEAD; }
	bool OnGroundOrGettingUp(void) { return OnGround() || m_nPedState == PED_GETUP; }
	
	bool Driving(void) { return m_nPedState == PED_DRIVING; }
	bool InVehicle(void) { return bInVehicle && m_pMyVehicle; } // True when ped is sitting/standing in vehicle, not in enter/exit state.
	bool EnteringCar(void) { return m_nPedState == PED_ENTER_CAR || m_nPedState == PED_CARJACK; }
	bool HasAttractor(void);
	bool IsUseAttractorObjective(eObjective obj) {
		return obj == OBJECTIVE_GOTO_ATM_ON_FOOT || obj == OBJECTIVE_GOTO_ICE_CREAM_VAN_ON_FOOT ||
			obj == OBJECTIVE_GOTO_PIZZA_ON_FOOT || obj == OBJECTIVE_GOTO_SEAT_ON_FOOT ||
			obj == OBJECTIVE_GOTO_SHELTER_ON_FOOT || obj == OBJECTIVE_GOTO_BUS_STOP_ON_FOOT;
	}

	void ReplaceWeaponWhenExitingVehicle(void);
	void RemoveWeaponWhenEnteringVehicle(void);
	bool IsNotInWreckedVehicle()
	{
		return m_pMyVehicle != nil && ((CEntity*)m_pMyVehicle)->GetStatus() != STATUS_WRECKED;
	}

	// My names. Inlined in VC
	AnimationId GetFireAnimNotDucking(CWeaponInfo* weapon) {
		if (m_nPedType == PEDTYPE_COP && weapon->IsFlagSet(WEAPONFLAG_COP3_RD))
			return Get3rdFireAnim(weapon);
		else
			return GetPrimaryFireAnim(weapon);
	}

	static AnimationId Get3rdFireAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_COP3_RD))
			return ANIM_WEAPON_FIRE_3RD;
		else
			return (AnimationId)0;
	}

	static AnimationId GetFireAnimGround(CWeaponInfo* weapon, bool kickFloorIfNone = true) {
		if (weapon->IsFlagSet(WEAPONFLAG_GROUND_2ND))
			return ANIM_WEAPON_CROUCHFIRE;
		else if (weapon->IsFlagSet(WEAPONFLAG_GROUND_3RD))
			return ANIM_WEAPON_FIRE_3RD;
		else if (kickFloorIfNone)
			return ANIM_STD_KICKGROUND;
		else
			return (AnimationId)0;
	}

	static AnimationId GetPrimaryFireAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_ANIMDETONATE))
			return ANIM_STD_DETONATE;
		else
			return ANIM_WEAPON_FIRE;
	}

	static AnimationId GetCrouchReloadAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_RELOAD))
			return ANIM_WEAPON_CROUCHRELOAD;
		else
			return (AnimationId)0;
	}

	static AnimationId GetCrouchFireAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_CROUCHFIRE))
			return ANIM_WEAPON_CROUCHFIRE;
		else
			return (AnimationId)0;
	}

	static AnimationId GetReloadAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_RELOAD))
			return ANIM_WEAPON_RELOAD;
		else
			return (AnimationId)0;
	}

	static AnimationId GetFightIdleWithMeleeAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_FIGHTMODE))
			return ANIM_MELEE_IDLE_FIGHTMODE;
		else
			return (AnimationId)0;
	}

	static AnimationId GetFinishingAttackAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_FINISH_3RD))
			return ANIM_MELEE_ATTACK_FINISH;
		else
			return (AnimationId)0;
	}

	static AnimationId GetSecondFireAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_USE_2ND))
			return ANIM_WEAPON_FIRE_2ND;
		else
			return (AnimationId)0;
	}
	
	static AnimationId GetMeleeStartAnim(CWeaponInfo* weapon) {
		if (weapon->IsFlagSet(WEAPONFLAG_PARTIALATTACK))
			return ANIM_MELEE_ATTACK_START;
		else
			return (AnimationId)0;
	}

	static AnimationId GetThrowAnim(CWeaponInfo *weapon)
	{
		if (weapon->IsFlagSet(WEAPONFLAG_THROW))
			return ANIM_THROWABLE_START_THROW;
		else
			return (AnimationId)0;
	}
	// --

	// My additions, because there were many, many instances of that.
	inline void SetFindPathAndFlee(CEntity *fleeFrom, int time, bool walk = false)
	{
		SetFlee(fleeFrom, time);
		bUsePedNodeSeek = true;
		m_pNextPathNode = nil;
		if (walk)
			SetMoveState(PEDMOVE_WALK);
	}

	inline void SetFindPathAndFlee(CVector2D const &from, int time, bool walk = false)
	{
		SetFlee(from, time);
		bUsePedNodeSeek = true;
		m_pNextPathNode = nil;
		if (walk)
			SetMoveState(PEDMOVE_WALK);
	}
	// --

	inline void SetWeaponLockOnTarget(CEntity *target)
	{
		if (m_pPointGunAt)
			m_pPointGunAt->CleanUpOldReference(&m_pPointGunAt);

		m_pPointGunAt = (CPed*)target;
		if (target)
			((CEntity*)target)->RegisterReference(&m_pPointGunAt);
	}

	// Using this to abstract nodes of skinned and non-skinned meshes
	CVector GetNodePosition(int32 node)
	{
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx = RpHAnimIDGetIndex(hier, m_pFrames[node]->nodeID);
		RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
		pos = mats[idx].pos;
		return pos;
	}
	void TransformToNode(CVector &pos, int32 node)
	{
		RpHAnimHierarchy *hier = GetAnimHierarchyFromSkinClump(GetClump());
		int32 idx = RpHAnimIDGetIndex(hier, m_pFrames[node]->nodeID);
		RwMatrix *mats = RpHAnimHierarchyGetMatrixArray(hier);
		RwV3dTransformPoints(&pos, &pos, 1, &mats[idx]);
	}

	// set by 0482:set_threat_reaction_range_multiplier opcode
	static uint16 nThreatReactionRangeMultiplier;

	// set by 0481:set_enter_car_range_multiplier opcode
	static uint16 nEnterCarRangeMultiplier;

	static bool bNastyLimbsCheat;
	static bool bFannyMagnetCheat;
	static bool bPedCheat3;
	static CVector2D ms_vec2DFleePosition;

#ifndef MASTER
	// Mobile things
	void DebugDrawPedDestination(CPed *, int, int);
	void DebugDrawPedDesiredHeading(CPed *, int, int);
	void DebugDrawCollisionRadius(float, float, float, float, int);
	void DebugDrawVisionRange(CVector, float);
	void DebugDrawVisionSimple(CVector, float);
	void DebugDrawLook();
	void DebugDrawPedPsyche();
	void DebugDrawDebugLines();

	static void SwitchDebugDisplay(void);
	static int GetDebugDisplay(void);

	void DebugDrawLookAtPoints();
	void DebugRenderOnePedText(void);
	void DebugRenderClosePedText();
#endif

#ifdef COMPATIBLE_SAVES
	virtual void Save(uint8*& buf);
	virtual void Load(uint8*& buf);
#endif
};

void FinishTalkingOnMobileCB(CAnimBlendAssociation* assoc, void* arg);
void StartTalkingOnMobileCB(CAnimBlendAssociation* assoc, void* arg);
void PlayRandomAnimationsFromAnimBlock(CPed* ped, AssocGroupId animGroup, uint32 first, uint32 amount);

VALIDATE_SIZE(CPed, 0x5F4);

bool IsPedPointerValid(CPed*);
bool IsPedPointerValid_NotInWorld(CPed*);
