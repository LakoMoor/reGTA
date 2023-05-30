#pragma once
#include "Font.h"
#include "PedType.h"
#include "Text.h"
#include "Sprite2d.h"

class CEntity;
class CBuilding;
class CPhysical;
class CVehicle;
class CPed;
class CObject;
class CPlayerInfo;

class CRunningScript;

extern int32 ScriptParams[32];

void FlushLog();
#define script_assert(_Expression) FlushLog(); assert(_Expression);

#define PICKUP_PLACEMENT_OFFSET (0.5f)
#define PED_FIND_Z_OFFSET (5.0f)
#define COP_PED_FIND_Z_OFFSET (10.0f)

#define UPSIDEDOWN_UP_THRESHOLD (-0.97f)
#define UPSIDEDOWN_MOVE_SPEED_THRESHOLD (0.01f)
#define UPSIDEDOWN_TURN_SPEED_THRESHOLD (0.02f)
#define UPSIDEDOWN_TIMER_THRESHOLD (1000)

#define SPHERE_MARKER_R (252)
#define SPHERE_MARKER_G (138)
#define SPHERE_MARKER_B (242)
#define SPHERE_MARKER_A (228)
#define SPHERE_MARKER_PULSE_PERIOD 2048
#define SPHERE_MARKER_PULSE_FRACTION 0.1f

#ifdef USE_PRECISE_MEASUREMENT_CONVERTION
#define MILES_IN_METER (0.000621371192f)
#define METERS_IN_FOOT (0.3048f)
#define FEET_IN_METER (3.28084f)
#else
#define MILES_IN_METER (1 / 1670.f)
#define METERS_IN_FOOT (0.3f)
#define FEET_IN_METER (3.33f)
#endif

#define KEY_LENGTH_IN_SCRIPT (8)

//#define GTA_SCRIPT_COLLECTIVE

struct intro_script_rectangle 
{
	bool m_bIsUsed;
	bool m_bBeforeFade;
	int16 m_nTextureId;
	CRect m_sRect;
	CRGBA m_sColor;
	
	intro_script_rectangle() { }
	~intro_script_rectangle() { }
};

VALIDATE_SIZE(intro_script_rectangle, 0x18);

enum {
	SCRIPT_TEXT_MAX_LENGTH = 100
};

struct intro_text_line 
{
	float m_fScaleX;
	float m_fScaleY;
	CRGBA m_sColor;
	bool m_bJustify;
	bool m_bCentered;
	bool m_bBackground;
	bool m_bBackgroundOnly;
	float m_fWrapX;
	float m_fCenterSize;
	CRGBA m_sBackgroundColor;
	bool m_bTextProportional;
	bool m_bTextBeforeFade;
	bool m_bRightJustify;
	int32 m_nFont;
	float m_fAtX;
	float m_fAtY;
	wchar m_Text[SCRIPT_TEXT_MAX_LENGTH];

	intro_text_line() { }
	~intro_text_line() { }
	
	void Reset()
	{
		m_fScaleX = 0.48f;
		m_fScaleY = 1.12f;
		m_sColor = CRGBA(225, 225, 225, 255);
		m_bJustify = false;
		m_bRightJustify = false;
		m_bCentered = false;
		m_bBackground = false;
		m_bBackgroundOnly = false;
		m_fWrapX = 182.0f;
		m_fCenterSize = DEFAULT_SCREEN_WIDTH;
		m_sBackgroundColor = CRGBA(128, 128, 128, 128);
		m_bTextProportional = true;
		m_bTextBeforeFade = false;
		m_nFont = FONT_STANDARD;
		m_fAtX = 0.0f;
		m_fAtY = 0.0f;
		memset(&m_Text, 0, sizeof(m_Text));
	}
};

VALIDATE_SIZE(intro_text_line, 0x414);

struct script_sphere_struct
{
	bool m_bInUse;
	uint16 m_Index;
	uint32 m_Id;
	CVector m_vecCenter;
	float m_fRadius;
	
	script_sphere_struct() { }
};

struct CStoredLine
{
	CVector vecInf;
	CVector vecSup;
	uint32 color1;
	uint32 color2;
};

enum {
	CLEANUP_UNUSED = 0,
	CLEANUP_CAR,
	CLEANUP_CHAR,
	CLEANUP_OBJECT
};

struct cleanup_entity_struct
{
	uint8 type;
	int32 id;
};

enum {
	MAX_CLEANUP = 50,
	MAX_UPSIDEDOWN_CAR_CHECKS = 6,
	MAX_STUCK_CAR_CHECKS = 16
};

class CMissionCleanup
{
public:
	cleanup_entity_struct m_sEntities[MAX_CLEANUP];
	uint8 m_nCount;

	CMissionCleanup();

	void Init();
	cleanup_entity_struct* FindFree();
	void AddEntityToList(int32, uint8);
	void RemoveEntityFromList(int32, uint8);
	void Process();
	void CheckIfCollisionHasLoadedForMissionObjects();
};

struct upsidedown_car_data
{
	int32 m_nVehicleIndex;
	uint32 m_nUpsideDownTimer;
};

class CUpsideDownCarCheck
{
	upsidedown_car_data m_sCars[MAX_UPSIDEDOWN_CAR_CHECKS];

public:
	void Init();
	bool IsCarUpsideDown(int32);
	bool IsCarUpsideDown(CVehicle*);
	void UpdateTimers();
	bool AreAnyCarsUpsideDown();
	void AddCarToCheck(int32);
	void RemoveCarFromCheck(int32);
	bool HasCarBeenUpsideDownForAWhile(int32);
};

struct stuck_car_data
{
	int32 m_nVehicleIndex;
	CVector m_vecPos;
	int32 m_nLastCheck;
	float m_fRadius;
	uint32 m_nStuckTime;
	bool m_bStuck;

	stuck_car_data() { }
	void Reset();
};

class CStuckCarCheck
{
	stuck_car_data m_sCars[MAX_STUCK_CAR_CHECKS];

public:
	void Init();
	void Process();
	void AddCarToCheck(int32, float, uint32);
	void RemoveCarFromCheck(int32);
	bool HasCarBeenStuckForAWhile(int32);
};

enum {
	ARGUMENT_END = 0,
	ARGUMENT_INT32,
	ARGUMENT_GLOBALVAR,
	ARGUMENT_LOCALVAR,
	ARGUMENT_INT8,
	ARGUMENT_INT16,
	ARGUMENT_FLOAT
};

struct tCollectiveData
{
	int32 colIndex;
	int32 pedIndex;
};

enum {
	USED_OBJECT_NAME_LENGTH = 24
};

struct tUsedObject
{
	char name[USED_OBJECT_NAME_LENGTH];
	int32 index;
};

struct tBuildingSwap
{
	CBuilding* m_pBuilding;
	int32 m_nNewModel;
	int32 m_nOldModel;
};


enum {
	MAX_STACK_DEPTH = 6,
	NUM_LOCAL_VARS = 16,
	NUM_TIMERS = 2
};

class CRunningScript
{
	enum {
		ANDOR_NONE = 0,
		ANDS_1 = 1,
		ANDS_2,
		ANDS_3,
		ANDS_4,
		ANDS_5,
		ANDS_6,
		ANDS_7,
		ANDS_8,
		ORS_1 = 21,
		ORS_2,
		ORS_3,
		ORS_4,
		ORS_5,
		ORS_6,
		ORS_7,
		ORS_8
	};

public:
	CRunningScript* next;
	CRunningScript* prev;
	char m_abScriptName[8];
	uint32 m_nIp;
	uint32 m_anStack[MAX_STACK_DEPTH];
	uint16 m_nStackPointer;
	int32 m_anLocalVariables[NUM_LOCAL_VARS + NUM_TIMERS];
	bool m_bIsActive;
	bool m_bCondResult;
	bool m_bIsMissionScript;
	bool m_bSkipWakeTime;
	uint32 m_nWakeTime;
	uint16 m_nAndOrState;
	bool m_bNotFlag;
	bool m_bDeatharrestEnabled;
	bool m_bDeatharrestExecuted;
	bool m_bMissionFlag;

public:
	void SetIP(uint32 ip) { m_nIp = ip; }
	CRunningScript* GetNext() const { return next; }

	void Save(uint8*& buf);
	void Load(uint8*& buf);

	void UpdateTimers(float timeStep) {
		m_anLocalVariables[NUM_LOCAL_VARS] += timeStep;
		m_anLocalVariables[NUM_LOCAL_VARS + 1] += timeStep;
	}

	void Init();
	void Process();

	void RemoveScriptFromList(CRunningScript**);
	void AddScriptToList(CRunningScript**);

	static const uint32 nSaveStructSize;

	void CollectParameters(uint32*, int16);
	int32 CollectNextParameterWithoutIncreasingPC(uint32);
	int32* GetPointerToScriptVariable(uint32*, int16);
	void StoreParameters(uint32*, int16);

	int8 ProcessOneCommand();
	void DoDeatharrestCheck();
	void UpdateCompareFlag(bool);
	int16 GetPadState(uint16, uint16);

	int8 ProcessCommands0To99(int32);
	int8 ProcessCommands100To199(int32);
	int8 ProcessCommands200To299(int32);
	int8 ProcessCommands300To399(int32);
	int8 ProcessCommands400To499(int32);
	int8 ProcessCommands500To599(int32);
	int8 ProcessCommands600To699(int32);
	int8 ProcessCommands700To799(int32);
	int8 ProcessCommands800To899(int32);
	int8 ProcessCommands900To999(int32);
	int8 ProcessCommands1000To1099(int32);
	int8 ProcessCommands1100To1199(int32);
	int8 ProcessCommands1200To1299(int32);
	int8 ProcessCommands1300To1399(int32);
	int8 ProcessCommands1400To1499(int32);

	void LocatePlayerCommand(int32, uint32*);
	void LocatePlayerCharCommand(int32, uint32*);
	void LocatePlayerCarCommand(int32, uint32*);
	void LocateCharCommand(int32, uint32*);
	void LocateCharCharCommand(int32, uint32*);
	void LocateCharCarCommand(int32, uint32*);
	void LocateCharObjectCommand(int32, uint32*);
	void LocateCarCommand(int32, uint32*);
	void LocateSniperBulletCommand(int32, uint32*);
	void PlayerInAreaCheckCommand(int32, uint32*);
	void PlayerInAngledAreaCheckCommand(int32, uint32*);
	void CharInAreaCheckCommand(int32, uint32*);
	void CarInAreaCheckCommand(int32, uint32*);
	void LocateObjectCommand(int32, uint32*);
	void ObjectInAreaCheckCommand(int32, uint32*);

#ifdef GTA_SCRIPT_COLLECTIVE
	void LocateCollectiveCommand(int32, uint32*);
	void LocateCollectiveCharCommand(int32, uint32*);
	void LocateCollectiveCarCommand(int32, uint32*);
	void LocateCollectivePlayerCommand(int32, uint32*);
	void CollectiveInAreaCheckCommand(int32, uint32*);
#endif

#ifdef MISSION_REPLAY
	bool CanAllowMissionReplay();
#endif

#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	int CollectParameterForDebug(char* buf, bool& var);
	void GetStoredParameterForDebug(char* buf);
	void LogOnStartProcessing();
	void LogBeforeProcessingCommand(int32 command);
	void LogAfterProcessingCommand(int32 command);

	static char commandInfo[];
	static uint32 storedIp;

#endif

	float LimitAngleOnCircle(float angle) { return angle < 0.0f ? angle + 360.0f : angle; }

	bool ThisIsAValidRandomCop(uint32 mi, int cop, int swat, int fbi, int army, int miami);
	bool ThisIsAValidRandomPed(uint32 pedtype, int civ, int gang, int criminal);

	bool CheckDamagedWeaponType(int32 actual, int32 type);
	
};


enum {
	VAR_LOCAL = 1,
	VAR_GLOBAL = 2,
};

enum {
#ifdef PS2
	SIZE_MAIN_SCRIPT = 205512,
#else
	SIZE_MAIN_SCRIPT = 225512,
#endif
	SIZE_MISSION_SCRIPT = 35000,
	SIZE_SCRIPT_SPACE = SIZE_MAIN_SCRIPT + SIZE_MISSION_SCRIPT
};

enum {
	MAX_NUM_SCRIPTS = 128,
	MAX_NUM_INTRO_TEXT_LINES = 48,
	MAX_NUM_INTRO_RECTANGLES = 16,
	MAX_NUM_SCRIPT_SRPITES = 16,
	MAX_NUM_SCRIPT_SPHERES = 16,
	MAX_NUM_USED_OBJECTS = 220,
	MAX_NUM_MISSION_SCRIPTS = 120,
	MAX_NUM_BUILDING_SWAPS = 25,
	MAX_NUM_INVISIBILITY_SETTINGS = 20,
	MAX_NUM_STORED_LINES = 1024
};

class CTheScripts
{
public:
	static uint8 ScriptSpace[SIZE_SCRIPT_SPACE];
	static CRunningScript ScriptsArray[MAX_NUM_SCRIPTS];
	static intro_text_line IntroTextLines[MAX_NUM_INTRO_TEXT_LINES];
	static intro_script_rectangle IntroRectangles[MAX_NUM_INTRO_RECTANGLES];
	static CSprite2d ScriptSprites[MAX_NUM_SCRIPT_SRPITES];
	static script_sphere_struct ScriptSphereArray[MAX_NUM_SCRIPT_SPHERES];
	static tUsedObject UsedObjectArray[MAX_NUM_USED_OBJECTS];
	static int32 MultiScriptArray[MAX_NUM_MISSION_SCRIPTS];
	static tBuildingSwap BuildingSwapArray[MAX_NUM_BUILDING_SWAPS];
	static CEntity* InvisibilitySettingArray[MAX_NUM_INVISIBILITY_SETTINGS];
	static CStoredLine aStoredLines[MAX_NUM_STORED_LINES];
	static bool DbgFlag;
	static uint32 OnAMissionFlag;
	static CMissionCleanup MissionCleanUp;
	static CStuckCarCheck StuckCars;
	static CUpsideDownCarCheck UpsideDownCars;
	static int32 StoreVehicleIndex;
	static bool StoreVehicleWasRandom;
	static CRunningScript *pIdleScripts;
	static CRunningScript *pActiveScripts;
	static int32 NextFreeCollectiveIndex;
	static int32 LastRandomPedId;
	static uint16 NumberOfUsedObjects;
	static bool bAlreadyRunningAMissionScript;
	static bool bUsingAMultiScriptFile;
	static uint16 NumberOfMissionScripts;
	static uint32 LargestMissionScriptSize;
	static uint32 MainScriptSize;
	static uint8 FailCurrentMission;
	static uint16 NumScriptDebugLines;
	static uint16 NumberOfIntroRectanglesThisFrame;
	static uint16 NumberOfIntroTextLinesThisFrame;
	static uint8 UseTextCommands;
	static uint16 CommandsExecuted;
	static uint16 ScriptsUpdated;
	static uint32 LastMissionPassedTime;
	static uint16 NumberOfExclusiveMissionScripts;
#if (defined GTA_PC && !defined GTAVC_JP_PATCH || defined GTA_XBOX || defined SUPPORT_XBOX_SCRIPT || defined GTA_MOBILE || defined SUPPORT_MOBILE_SCRIPT)
#define CARDS_IN_SUIT (13)
#define NUM_SUITS (4)
#define MAX_DECKS (6)
#define CARDS_IN_DECK (CARDS_IN_SUIT * NUM_SUITS)
#define CARDS_IN_STACK (CARDS_IN_DECK * MAX_DECKS)
	static int16 CardStack[CARDS_IN_STACK];
	static int16 CardStackPosition;
#endif
	static bool bPlayerIsInTheStatium;
	static uint8 RiotIntensity;
	static bool bPlayerHasMetDebbieHarry;

	static void Init();
	static void Process();

	static CRunningScript* StartTestScript();
	static bool IsPlayerOnAMission();
	static void ClearSpaceForMissionEntity(const CVector&, CEntity*);

	static void UndoBuildingSwaps();
	static void UndoEntityInvisibilitySettings();

	static void ScriptDebugLine3D(float x1, float y1, float z1, float x2, float y2, float z2, uint32 col, uint32 col2);
	static void RenderTheScriptDebugLines();

	static void SaveAllScripts(uint8*, uint32*);
	static void LoadAllScripts(uint8*, uint32);

	static bool IsDebugOn() { return DbgFlag; };
	static void InvertDebugFlag() { DbgFlag = !DbgFlag; }

	static int32* GetPointerToScriptVariable(int32 offset) { assert(offset >= 8 && offset < CTheScripts::GetSizeOfVariableSpace()); return (int32*)&ScriptSpace[offset]; }

	static int32 Read4BytesFromScript(uint32* pIp) {
		int32 retval = ScriptSpace[*pIp + 3] << 24 | ScriptSpace[*pIp + 2] << 16 | ScriptSpace[*pIp + 1] << 8 | ScriptSpace[*pIp];
		*pIp += 4;
		return retval;
	}
	static int16 Read2BytesFromScript(uint32* pIp) {
		int16 retval = ScriptSpace[*pIp + 1] << 8 | ScriptSpace[*pIp];
		*pIp += 2;
		return retval;
	}
	static int8 Read1ByteFromScript(uint32* pIp) {
		int8 retval = ScriptSpace[*pIp];
		*pIp += 1;
		return retval;
	}
	static float ReadFloatFromScript(uint32* pIp) {
		return Read2BytesFromScript(pIp) / 16.0f;
	}
	static void ReadTextLabelFromScript(uint32* pIp, char* buf) {
		strncpy(buf, (const char*)&CTheScripts::ScriptSpace[*pIp], KEY_LENGTH_IN_SCRIPT);
	}
	static wchar* GetTextByKeyFromScript(uint32* pIp) {
		wchar* text = TheText.Get((const char*)&CTheScripts::ScriptSpace[*pIp]);
		*pIp += KEY_LENGTH_IN_SCRIPT;
		return text;
	}
	static int32 GetSizeOfVariableSpace()
	{
		uint32 tmp = 3;
		return Read4BytesFromScript(&tmp);
	}

	static CRunningScript* StartNewScript(uint32);

	static void CleanUpThisVehicle(CVehicle*);
	static void CleanUpThisPed(CPed*);
	static void CleanUpThisObject(CObject*);

	static bool IsPedStopped(CPed*);
	static bool IsPlayerStopped(CPlayerInfo*);
	static bool IsVehicleStopped(CVehicle*);

	static void PrintListSizes();
	static void ReadObjectNamesFromScript();
	static void UpdateObjectIndices();
	static void ReadMultiScriptFileOffsetsFromScript();
	static void DrawScriptSpheres();
	static void HighlightImportantArea(uint32, float, float, float, float, float);
	static void HighlightImportantAngledArea(uint32, float, float, float, float, float, float, float, float, float);
	static void DrawDebugSquare(float, float, float, float);
	static void DrawDebugAngledSquare(float, float, float, float, float, float, float, float);
	static void DrawDebugCube(float, float, float, float, float, float);
	static void DrawDebugAngledCube(float, float, float, float, float, float, float, float, float, float);

	static void AddToInvisibilitySwapArray(CEntity*, bool);
	static void AddToBuildingSwapArray(CBuilding*, int32, int32);

	static int32 GetActualScriptSphereIndex(int32 index);
	static int32 AddScriptSphere(int32 id, CVector pos, float radius);
	static int32 GetNewUniqueScriptSphereIndex(int32 index);
	static void RemoveScriptSphere(int32 index);
	static void RemoveScriptTextureDictionary();
public:
	static void RemoveThisPed(CPed* pPed);

	static uint32& GetLastMissionPassedTime() { return LastMissionPassedTime; }
#ifdef MISSION_SWITCHER
	static void SwitchToMission(int32 mission);
#endif

#ifdef GTA_SCRIPT_COLLECTIVE
	static void AdvanceCollectiveIndex()
	{
		if (NextFreeCollectiveIndex == INT32_MAX)
			NextFreeCollectiveIndex = 0;
		else
			NextFreeCollectiveIndex++;
	}

	static int AddPedsInVehicleToCollective(int);
	static int AddPedsInAreaToCollective(float, float, float, float);
	static int FindFreeSlotInCollectiveArray();
	static void SetObjectiveForAllPedsInCollective(int, eObjective, int16, int16);
	static void SetObjectiveForAllPedsInCollective(int, eObjective, CVector, float);
	static void SetObjectiveForAllPedsInCollective(int, eObjective, CVector);
	static void SetObjectiveForAllPedsInCollective(int, eObjective, void*);
	static void SetObjectiveForAllPedsInCollective(int, eObjective);
#endif

#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
	static bool MissionSupportsMissionReplay(int index)
	{
		return index >= 3 && index <= 35 || index >= 51 && index <= 65 || index >= 67 && index <= 74 || index >= 83 && index <= 87;
	}
#endif

#ifdef USE_DEBUG_SCRIPT_LOADER
	static int ScriptToLoad;
	static int OpenScript();
#endif

#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	static void LogAfterScriptInitializing();
	static void LogBeforeScriptProcessing();
	static void LogAfterScriptProcessing();
#endif
};

#ifdef MISSION_REPLAY
extern int AllowMissionReplay;
extern uint32 WaitForMissionActivate;
extern uint32 WaitForSave;
extern uint32 MissionStartTime;
extern int missionRetryScriptIndex;
extern bool doingMissionRetry;
extern bool gbTryingPorn4Again;
extern int IsInAmmunation;
extern int MissionSkipLevel;

#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
extern bool UsingMobileScript;
extern bool AlreadySavedGame;
#endif

uint32 AddExtraDeathDelay();
void RetryMission(int, int unk = 0);

enum {
	MISSION_RETRY_TYPE_SUGGEST_TO_PLAYER = 0,
	MISSION_RETRY_TYPE_1,
	MISSION_RETRY_TYPE_BEGIN_RESTARTING
};

enum {
	MISSION_RETRY_STAGE_NORMAL = 0,
	MISSION_RETRY_STAGE_WAIT_FOR_SCRIPT_TO_TERMINATE,
	MISSION_RETRY_STAGE_START_PROCESSING,
	MISSION_RETRY_STAGE_WAIT_FOR_DELAY,
	MISSION_RETRY_STAGE_WAIT_FOR_MENU,
	MISSION_RETRY_STAGE_WAIT_FOR_USER,
	MISSION_RETRY_STAGE_START_RESTARTING,
	MISSION_RETRY_STAGE_WAIT_FOR_TIMER_AFTER_RESTART,
};
#endif
