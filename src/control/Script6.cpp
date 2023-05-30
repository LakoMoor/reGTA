#include "common.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "Bike.h"
#include "CarCtrl.h"
#include "Cranes.h"
#include "Credits.h"
#include "CutsceneMgr.h"
#include "DMAudio.h"
#include "FileMgr.h"
#include "Fire.h"
#include "Frontend.h"
#include "Garages.h"
#include "General.h"
#ifdef MISSION_REPLAY
#include "GenericGameStorage.h"
#endif
#include "Messages.h"
#include "Pad.h"
#include "Particle.h"
#include "Phones.h"
#include "Population.h"
#include "Pools.h"
#include "Record.h"
#include "Remote.h"
#include "Restart.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "Streaming.h"
#include "Weather.h"
#include "Zones.h"
#include "main.h"
#include "GameLogic.h"
#include "Sprite.h"
#include "CarAI.h"
#include "Pickups.h"
#include "Fluff.h"

#ifdef USE_DEBUG_SCRIPT_LOADER
extern const char* scriptfile;
#endif

bool CRunningScript::ThisIsAValidRandomCop(uint32 mi, int cop, int swat, int fbi, int army, int miami)
{
	switch (mi)
	{
	case MI_COP: if (cop) return true; break;
	case MI_SWAT: if (swat) return true; break;
	case MI_FBI: if (fbi) return true; break;
	case MI_ARMY: if (army) return true; break;
	default: if (mi >= MI_VICE1 && mi <= MI_VICE8 && miami) return true; break;
	}
	return false;
}

bool CRunningScript::ThisIsAValidRandomPed(uint32 pedtype, int civ, int gang, int criminal)
{
    switch (pedtype) {
    case PEDTYPE_CIVMALE:
    case PEDTYPE_CIVFEMALE:
        return civ;
    case PEDTYPE_GANG1:
    case PEDTYPE_GANG2:
    case PEDTYPE_GANG3:
    case PEDTYPE_GANG4:
    case PEDTYPE_GANG5:
    case PEDTYPE_GANG6:
    case PEDTYPE_GANG7:
    case PEDTYPE_GANG8:
    case PEDTYPE_GANG9:
        return gang;
    case PEDTYPE_CRIMINAL:
    case PEDTYPE_PROSTITUTE:
        return criminal;
    default:
        return false;
    }
}

int8 CRunningScript::ProcessCommands1000To1099(int32 command)
{
	switch (command) {
	//case COMMAND_FLASH_RADAR_BLIP:
	/*
	case COMMAND_IS_CHAR_IN_CONTROL:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(pPed->IsPedInControl());
		return 0;
	}
	*/
	case COMMAND_SET_GENERATE_CARS_AROUND_CAMERA:
		CollectParameters(&m_nIp, 1);
		CCarCtrl::bCarsGeneratedAroundCamera = (ScriptParams[0] != 0);
		return 0;
	case COMMAND_CLEAR_SMALL_PRINTS:
		CMessages::ClearSmallMessagesOnly();
		return 0;
	/*
	case COMMAND_HAS_MILITARY_CRANE_COLLECTED_ALL_CARS:
		UpdateCompareFlag(CCranes::HaveAllCarsBeenCollectedByMilitaryCrane());
		return 0;
	*/
	case COMMAND_SET_UPSIDEDOWN_CAR_NOT_DAMAGED:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		CAutomobile* pCar = (CAutomobile*)pVehicle;
		pCar->bNotDamagedUpsideDown = (ScriptParams[1] != 0);
		return 0;
	}
	case COMMAND_CAN_PLAYER_START_MISSION:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerPed* pPlayerPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPlayerPed);
		UpdateCompareFlag(pPlayerPed->IsPedInControl() || pPlayerPed->m_nPedState == PED_DRIVING);
		return 0;
	}
	case COMMAND_MAKE_PLAYER_SAFE_FOR_CUTSCENE:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
		CPad::GetPad(ScriptParams[0])->SetDisablePlayerControls(PLAYERCONTROL_CUTSCENE);
		pPlayerInfo->MakePlayerSafe(true);
		CCutsceneMgr::StartCutsceneProcessing();
		return 0;
	}
	case COMMAND_USE_TEXT_COMMANDS:
		CollectParameters(&m_nIp, 1);
		CTheScripts::UseTextCommands = (ScriptParams[0] != 0) ? 2 : 1;
		return 0;
	case COMMAND_SET_THREAT_FOR_PED_TYPE:
		CollectParameters(&m_nIp, 2);
		CPedType::AddThreat(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_CLEAR_THREAT_FOR_PED_TYPE:
		CollectParameters(&m_nIp, 2);
		CPedType::RemoveThreat(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_GET_CAR_COLOURS:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		ScriptParams[0] = pVehicle->m_currentColour1;
		ScriptParams[1] = pVehicle->m_currentColour2;
		StoreParameters(&m_nIp, 2);
		return 0;
	}
	case COMMAND_SET_ALL_CARS_CAN_BE_DAMAGED:
		CollectParameters(&m_nIp, 1);
		CWorld::SetAllCarsCanBeDamaged(ScriptParams[0] != 0);
		if (!ScriptParams[0])
			CWorld::ExtinguishAllCarFiresInArea(FindPlayerCoors(), 4000.0f);
		return 0;
	case COMMAND_SET_CAR_CAN_BE_DAMAGED:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		pVehicle->bCanBeDamaged = ScriptParams[1] != 0;
		if (!ScriptParams[1])
			pVehicle->ExtinguishCarFire();
		return 0;
	}
	//case COMMAND_MAKE_PLAYER_UNSAFE:
	/*
	case COMMAND_LOAD_COLLISION:
	{
		CollectParameters(&m_nIp, 1);
		CTimer::Stop();
		CGame::currLevel = (eLevelName)ScriptParams[0];
		ISLAND_LOADING_IS(LOW)
		{
			CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
			CStreaming::RemoveUnusedBuildings(CGame::currLevel);
		}
		CCollision::SortOutCollisionAfterLoad();
		ISLAND_LOADING_ISNT(HIGH)
		{
			CStreaming::RequestIslands(CGame::currLevel);
			CStreaming::LoadAllRequestedModels(true);
		}
		CTimer::Update();
		return 0;
	}
	case COMMAND_GET_BODY_CAST_HEALTH:
	//	ScriptParams[0] = CObject::nBodyCastHealth;
	//	StoreParameters(&m_nIp, 1);
		return 0;
	*/
	case COMMAND_SET_CHARS_CHATTING:
	{
		CollectParameters(&m_nIp, 3);
		CPed* pPed1 = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CPed* pPed2 = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pPed1 && pPed2);
		pPed1->SetChat(pPed2, ScriptParams[2]);
		pPed2->SetChat(pPed1, ScriptParams[2]);
		return 0;
	}
	//case COMMAND_MAKE_PLAYER_SAFE:
	/*
	case COMMAND_SET_CAR_STAYS_IN_CURRENT_LEVEL:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		if (ScriptParams[1])
			pVehicle->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pVehicle->GetPosition());
		else
			pVehicle->m_nZoneLevel = LEVEL_GENERIC;
		return 0;
	}
	case COMMAND_SET_CHAR_STAYS_IN_CURRENT_LEVEL:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (ScriptParams[1])
			pPed->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pPed->GetPosition());
		else
			pPed->m_nZoneLevel = LEVEL_GENERIC;
		return 0;
	}
	*/
	case COMMAND_SET_DRUNK_INPUT_DELAY:
	{
		CollectParameters(&m_nIp, 2);
		assert(ScriptParams[1] < CPad::DRUNK_STEERING_BUFFER_SIZE);
		CPad::GetPad(ScriptParams[0])->SetDrunkInputDelay(ScriptParams[1]);
		return 0;
	}
	case COMMAND_SET_CHAR_MONEY:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->m_nPedMoney = ScriptParams[1];
		pPed->bMoneyHasBeenGivenByScript = true;
		return 0;
	}
	//case COMMAND_INCREASE_CHAR_MONEY:
	case COMMAND_GET_OFFSET_FROM_OBJECT_IN_WORLD_COORDS:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CVector result = Multiply3x3(pObject->GetMatrix(), *(CVector*)&ScriptParams[1]) + pObject->GetPosition();
		*(CVector*)&ScriptParams[0] = result;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_REGISTER_LIFE_SAVED:
		CStats::AnotherLifeSavedWithAmbulance();
		return 0;
	case COMMAND_REGISTER_CRIMINAL_CAUGHT:
		CStats::AnotherCriminalCaught();
		return 0;
	case COMMAND_REGISTER_AMBULANCE_LEVEL:
		CollectParameters(&m_nIp, 1);
		CStats::RegisterLevelAmbulanceMission(ScriptParams[0]);
		return 0;
	case COMMAND_REGISTER_FIRE_EXTINGUISHED:
		CStats::AnotherFireExtinguished();
		return 0;
	case COMMAND_TURN_PHONE_ON:
		CollectParameters(&m_nIp, 1);
		gPhoneInfo.m_aPhones[ScriptParams[0]].m_nState = PHONE_STATE_9;
		return 0;
	/*
	case COMMAND_REGISTER_LONGEST_DODO_FLIGHT:
		CollectParameters(&m_nIp, 1);
		CStats::RegisterLongestFlightInDodo(ScriptParams[0]);
		return 0;
	*/
	case COMMAND_GET_OFFSET_FROM_CAR_IN_WORLD_COORDS:
	{
		CollectParameters(&m_nIp, 4);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CVector result = Multiply3x3(pVehicle->GetMatrix(), *(CVector*)&ScriptParams[1]) + pVehicle->GetPosition();
		*(CVector*)&ScriptParams[0] = result;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_TOTAL_NUMBER_OF_KILL_FRENZIES:
		CollectParameters(&m_nIp, 1);
		CStats::SetTotalNumberKillFrenzies(ScriptParams[0]);
		return 0;
	case COMMAND_BLOW_UP_RC_BUGGY:
		CWorld::Players[CWorld::PlayerInFocus].BlowUpRCBuggy(true);
		return 0;
	/*
	case COMMAND_REMOVE_CAR_FROM_CHASE:
		CollectParameters(&m_nIp, 1);
		CRecordDataForChase::RemoveCarFromChase(ScriptParams[0]);
		return 0;
	*/
	case COMMAND_IS_FRENCH_GAME:
		UpdateCompareFlag(CGame::frenchGame);
		return 0;
	case COMMAND_IS_GERMAN_GAME:
		UpdateCompareFlag(CGame::germanGame);
		return 0;
	case COMMAND_CLEAR_MISSION_AUDIO:
		CollectParameters(&m_nIp, 1);
		DMAudio.ClearMissionAudio(ScriptParams[0] - 1);
		return 0;
	/*
	case COMMAND_SET_FADE_IN_AFTER_NEXT_ARREST:
		CollectParameters(&m_nIp, 1);
		CRestart::bFadeInAfterNextArrest = !!ScriptParams[0];
		return 0;
	case COMMAND_SET_FADE_IN_AFTER_NEXT_DEATH:
		CollectParameters(&m_nIp, 1);
		CRestart::bFadeInAfterNextDeath = !!ScriptParams[0];
		return 0;
	case COMMAND_SET_GANG_PED_MODEL_PREFERENCE:
		CollectParameters(&m_nIp, 2);
		CGangs::SetGangPedModelOverride(ScriptParams[0], ScriptParams[1]);
		return 0;
	*/
	case COMMAND_SET_CHAR_USE_PEDNODE_SEEK:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (ScriptParams[1])
			pPed->m_pNextPathNode = nil;
		pPed->bUsePedNodeSeek = !!ScriptParams[1];
		return 0;
	}
	/*
	case COMMAND_SWITCH_VEHICLE_WEAPONS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->bGunSwitchedOff = !ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_GET_OUT_OF_JAIL_FREE:
		CollectParameters(&m_nIp, 2);
		CWorld::Players[ScriptParams[0]].m_bGetOutOfJailFree = !!ScriptParams[1];
		return 0;
	*/
	case COMMAND_SET_FREE_HEALTH_CARE:
		CollectParameters(&m_nIp, 2);
		CWorld::Players[ScriptParams[0]].m_bGetOutOfHospitalFree = !!ScriptParams[1];
		return 0;
	/*
	case COMMAND_IS_CAR_DOOR_CLOSED:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(!pVehicle->IsDoorMissing((eDoors)ScriptParams[1]) && pVehicle->IsDoorClosed((eDoors)ScriptParams[1]));
		return 0;
	}
	*/
	case COMMAND_LOAD_AND_LAUNCH_MISSION:
		return 0;
	case COMMAND_LOAD_AND_LAUNCH_MISSION_INTERNAL:
	{
#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
		uint32 oldIp = m_nIp;
#endif
		CollectParameters(&m_nIp, 1);

		if (CTheScripts::NumberOfExclusiveMissionScripts > 0 && ScriptParams[0] <= UINT16_MAX - 2)
			return 0;
#ifdef MISSION_REPLAY
		missionRetryScriptIndex = ScriptParams[0];
#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
		if (!UsingMobileScript && CTheScripts::MissionSupportsMissionReplay(missionRetryScriptIndex)){
			if (!AlreadySavedGame) {
				m_nIp = oldIp - 2;
				SaveGameForPause(SAVE_TYPE_QUICKSAVE_FOR_SCRIPT);
				AlreadySavedGame = true;
				return 0;
			}
			else {
				AlreadySavedGame = false;
			}
		}
#endif
#endif
		CTimer::Suspend();
		int offset = CTheScripts::MultiScriptArray[ScriptParams[0]];
#ifdef USE_DEBUG_SCRIPT_LOADER
		int handle = CTheScripts::OpenScript();
#else
		CFileMgr::ChangeDir("\\");
		int handle = CFileMgr::OpenFile("data\\main.scm", "rb");
#endif
		CFileMgr::Seek(handle, offset, 0);
		CFileMgr::Read(handle, (const char*)&CTheScripts::ScriptSpace[SIZE_MAIN_SCRIPT], SIZE_MISSION_SCRIPT);
		CFileMgr::CloseFile(handle);
		CRunningScript* pMissionScript = CTheScripts::StartNewScript(SIZE_MAIN_SCRIPT);
		CTimer::Resume();
		pMissionScript->m_bIsMissionScript = true;
		pMissionScript->m_bMissionFlag = true;
		CTheScripts::bAlreadyRunningAMissionScript = true;
		CGameLogic::ClearShortCut();
		return 0;
	}
	case COMMAND_SET_OBJECT_DRAW_LAST:
	{
		CollectParameters(&m_nIp, 2);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		pObject->bDrawLast = !!ScriptParams[1];
		return 0;
	}
	case COMMAND_GET_AMMO_IN_PLAYER_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		ScriptParams[0] = 0;
		for (int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {
			if (pPed->GetWeapon(i).m_eWeaponType == (eWeaponType)ScriptParams[1])
				ScriptParams[0] = pPed->GetWeapon(i).m_nAmmoTotal;
		}
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	/*
	case COMMAND_GET_AMMO_IN_CHAR_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CWeapon* pWeaponSlot = &pPed->m_weapons[ScriptParams[1]];
		if (pWeaponSlot->m_eWeaponType == (eWeaponType)ScriptParams[1])
			ScriptParams[0] = pWeaponSlot->m_nAmmoTotal;
		else
			ScriptParams[0] = 0;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_REGISTER_KILL_FRENZY_PASSED:
		CStats::AnotherKillFrenzyPassed();
		return 0;
	case COMMAND_SET_CHAR_SAY:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		switch (ScriptParams[1]) {
		case SCRIPT_SOUND_CHUNKY_RUN_SHOUT:
			pPed->Say(SOUND_PED_FLEE_RUN);
			break;
		case SCRIPT_SOUND_SECURITY_GUARD_AWAY_SHOUT:
			pPed->Say(SOUND_PED_FLEE_RUN);
			break;
		case SCRIPT_SOUND_SWAT_PED_SHOUT:
			pPed->Say(SOUND_PED_PURSUIT_SWAT);
			break;
		case SCRIPT_SOUND_AMMUNATION_CHAT_1:
			pPed->Say(SOUND_AMMUNATION_WELCOME_1);
			break;
		case SCRIPT_SOUND_AMMUNATION_CHAT_2:
			pPed->Say(SOUND_AMMUNATION_WELCOME_2);
			break;
		case SCRIPT_SOUND_AMMUNATION_CHAT_3:
			pPed->Say(SOUND_AMMUNATION_WELCOME_3);
			break;
		default:
			break;
		}
		return 0;
	}
	*/
	case COMMAND_SET_NEAR_CLIP:
		CollectParameters(&m_nIp, 1);
		TheCamera.SetNearClipScript(*(float*)&ScriptParams[0]);
		return 0;
	case COMMAND_SET_RADIO_CHANNEL:
		CollectParameters(&m_nIp, 2);
		DMAudio.SetRadioChannel(ScriptParams[0], ScriptParams[1]);
		return 0;
	/*
	case COMMAND_OVERRIDE_HOSPITAL_LEVEL:
		CollectParameters(&m_nIp, 1);
		CRestart::OverrideHospitalLevel = ScriptParams[0];
		return 0;
	case COMMAND_OVERRIDE_POLICE_STATION_LEVEL:
		CollectParameters(&m_nIp, 1);
		CRestart::OverridePoliceStationLevel = ScriptParams[0];
		return 0;
	case COMMAND_FORCE_RAIN:
		CollectParameters(&m_nIp, 1);
		CWeather::bScriptsForceRain = !!ScriptParams[0];
		return 0;
	case COMMAND_DOES_GARAGE_CONTAIN_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		script_assert(pVehicle);
		UpdateCompareFlag(CGarages::IsThisCarWithinGarageArea(ScriptParams[0], pVehicle));
		return 0;
	}
	*/
	case COMMAND_SET_CAR_TRACTION:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		float fTraction = *(float*)&ScriptParams[1];
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR || pVehicle->m_vehType == VEHICLE_TYPE_BIKE);
		if (pVehicle->m_vehType == VEHICLE_TYPE_CAR)
			((CAutomobile*)pVehicle)->m_fTraction = fTraction;
		else
			((CBike*)pVehicle)->m_fTraction = fTraction;
		return 0;
	}
	case COMMAND_ARE_MEASUREMENTS_IN_METRES:
#ifdef USE_MEASUREMENTS_IN_METERS
		UpdateCompareFlag(true);
#else
		UpdateCompareFlag(false);
#endif
		return 0;
	case COMMAND_CONVERT_METRES_TO_FEET:
	{
		CollectParameters(&m_nIp, 1);
		float fMeterValue = *(float*)&ScriptParams[0];
		float fFeetValue = fMeterValue / METERS_IN_FOOT;
		*(float*)&ScriptParams[0] = fFeetValue;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	/*
	case COMMAND_MARK_ROADS_BETWEEN_LEVELS:
	{
		CollectParameters(&m_nIp, 6);
		float infX = *(float*)&ScriptParams[0];
		float infY = *(float*)&ScriptParams[1];
		float infZ = *(float*)&ScriptParams[2];
		float supX = *(float*)&ScriptParams[3];
		float supY = *(float*)&ScriptParams[4];
		float supZ = *(float*)&ScriptParams[5];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[0];
		}
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[1];
		}
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[5];
			supZ = *(float*)&ScriptParams[2];
		}
		ThePaths.MarkRoadsBetweenLevelsInArea(infX, supX, infY, supY, infZ, supZ);
		return 0;
	}
	case COMMAND_MARK_PED_ROADS_BETWEEN_LEVELS:
	{
		CollectParameters(&m_nIp, 6);
		float infX = *(float*)&ScriptParams[0];
		float infY = *(float*)&ScriptParams[1];
		float infZ = *(float*)&ScriptParams[2];
		float supX = *(float*)&ScriptParams[3];
		float supY = *(float*)&ScriptParams[4];
		float supZ = *(float*)&ScriptParams[5];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[0];
		}
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[1];
		}
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[5];
			supZ = *(float*)&ScriptParams[2];
		}
		ThePaths.PedMarkRoadsBetweenLevelsInArea(infX, supX, infY, supY, infZ, supZ);
		return 0;
	}
	*/
	case COMMAND_SET_CAR_AVOID_LEVEL_TRANSITIONS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->AutoPilot.m_bStayInCurrentLevel = !!ScriptParams[1];
		return 0;
	}
	/*
	case COMMAND_SET_CHAR_AVOID_LEVEL_TRANSITIONS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pPed);
		// not implemented
		return 0;
	}
	case COMMAND_IS_THREAT_FOR_PED_TYPE:
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(CPedType::IsThreat(ScriptParams[0], ScriptParams[1]));
		return 0;
	*/
	case COMMAND_CLEAR_AREA_OF_CHARS:
	{
		CollectParameters(&m_nIp, 6);
		float infX = *(float*)&ScriptParams[0];
		float infY = *(float*)&ScriptParams[1];
		float infZ = *(float*)&ScriptParams[2];
		float supX = *(float*)&ScriptParams[3];
		float supY = *(float*)&ScriptParams[4];
		float supZ = *(float*)&ScriptParams[5];
		if (infX > supX) {
			infX = *(float*)&ScriptParams[3];
			supX = *(float*)&ScriptParams[0];
		}
		if (infY > supY) {
			infY = *(float*)&ScriptParams[4];
			supY = *(float*)&ScriptParams[1];
		}
		if (infZ > supZ) {
			infZ = *(float*)&ScriptParams[5];
			supZ = *(float*)&ScriptParams[2];
		}
		CWorld::ClearPedsFromArea(infX, infY, infZ, supX, supY, supZ);
		return 0;
	}
	case COMMAND_SET_TOTAL_NUMBER_OF_MISSIONS:
		CollectParameters(&m_nIp, 1);
		CStats::SetTotalNumberMissions(CGame::germanGame ? ScriptParams[0] - 2 : ScriptParams[0]);
		return 0;
	case COMMAND_CONVERT_METRES_TO_FEET_INT:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] *= FEET_IN_METER;
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_REGISTER_FASTEST_TIME:
		CollectParameters(&m_nIp, 2);
		CStats::RegisterFastestTime(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_REGISTER_HIGHEST_SCORE:
		CollectParameters(&m_nIp, 2);
		CStats::RegisterHighestScore(ScriptParams[0], ScriptParams[1]);
		return 0;
	//case COMMAND_WARP_CHAR_INTO_CAR_AS_PASSENGER:
	case COMMAND_IS_CAR_PASSENGER_SEAT_FREE:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		UpdateCompareFlag(ScriptParams[1] < pVehicle->m_nNumMaxPassengers && pVehicle->pPassengers[ScriptParams[1]] == nil);
		return 0;
	}
	/*
	case COMMAND_GET_CHAR_IN_CAR_PASSENGER_SEAT:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(ScriptParams[1] >= 0 && ScriptParams[1] < ARRAY_SIZE(pVehicle->pPassengers));
		CPed* pPassenger = pVehicle->pPassengers[ScriptParams[1]];
		ScriptParams[0] = CPools::GetPedPool()->GetIndex(pPassenger);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	*/
	case COMMAND_SET_CHAR_IS_CHRIS_CRIMINAL:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bChrisCriminal = !!ScriptParams[1];
		return 0;
	}
	case COMMAND_START_CREDITS:
		CCredits::Start();
		return 0;
	case COMMAND_STOP_CREDITS:
		CCredits::Stop();
		return 0;
	case COMMAND_ARE_CREDITS_FINISHED:
		UpdateCompareFlag(CCredits::AreCreditsDone());
		return 0;
	case COMMAND_CREATE_SINGLE_PARTICLE:
		CollectParameters(&m_nIp, 8);
		CParticle::AddParticle((tParticleType)ScriptParams[0], *(CVector*)&ScriptParams[1],
			*(CVector*)&ScriptParams[4], nil, *(float*)&ScriptParams[7], 0, 0, 0, 0);
		return 0;
	/*
	case COMMAND_SET_CHAR_IGNORE_LEVEL_TRANSITIONS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		if (ScriptParams[1])
			pPed->m_nZoneLevel = LEVEL_IGNORE;
		else
			pPed->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pPed->GetPosition());
		return 0;
	}
	case COMMAND_GET_CHASE_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CRecordDataForChase::TurnChaseCarIntoScriptCar(ScriptParams[0]);
		ScriptParams[0] = CPools::GetVehiclePool()->GetIndex(pVehicle);
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			CTheScripts::MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CAR);
		return 0;
	}
	case COMMAND_START_BOAT_FOAM_ANIMATION:
		CSpecialParticleStuff::StartBoatFoamAnimation();
		return 0;
	case COMMAND_UPDATE_BOAT_FOAM_ANIMATION:
	{
		CollectParameters(&m_nIp, 1);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CSpecialParticleStuff::UpdateBoatFoamAnimation(&pObject->GetMatrix());
		return 0;
	}
	*/
	case COMMAND_SET_MUSIC_DOES_FADE:
		CollectParameters(&m_nIp, 1);
		TheCamera.m_bIgnoreFadingStuffForMusic = (ScriptParams[0] == 0);
		return 0;
	/*
	case COMMAND_SET_INTRO_IS_PLAYING:
		CollectParameters(&m_nIp, 1);
		if (ScriptParams[0]) {
			CGame::playingIntro = true;
			CStreaming::RemoveCurrentZonesModels();
		} else {
			CGame::playingIntro = false;
			DMAudio.ChangeMusicMode(MUSICMODE_GAME);
			int mi;
			CModelInfo::GetModelInfo("bridgefukb", &mi);
			CStreaming::RequestModel(mi, STREAMFLAGS_DEPENDENCY);
			CStreaming::LoadAllRequestedModels(false);
		}
		return 0;
	*/
	case COMMAND_SET_PLAYER_HOOKER:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerInfo* pPlayerInfo = &CWorld::Players[ScriptParams[0]];
		if (ScriptParams[1] < 0) {
			pPlayerInfo->m_pHooker = nil;
			pPlayerInfo->m_nNextSexFrequencyUpdateTime = 0;
			pPlayerInfo->m_nNextSexMoneyUpdateTime = 0;
		} else {
			CPed* pHooker = CPools::GetPedPool()->GetAt(ScriptParams[1]);
			script_assert(pHooker);
			pPlayerInfo->m_pHooker = (CCivilianPed*)pHooker;
			pPlayerInfo->m_nSexFrequency = 1000;
			pPlayerInfo->m_nNextSexFrequencyUpdateTime = CTimer::GetTimeInMilliseconds() + 1000;
			pPlayerInfo->m_nNextSexMoneyUpdateTime = CTimer::GetTimeInMilliseconds() + 3000;
		}
		return 0;
	}
	case COMMAND_PLAY_END_OF_GAME_TUNE:
		DMAudio.PlayPreloadedCutSceneMusic();
		return 0;
	case COMMAND_STOP_END_OF_GAME_TUNE:
		DMAudio.StopCutSceneMusic();
		DMAudio.ChangeMusicMode(MUSICMODE_GAME);
		return 0;
	case COMMAND_GET_CAR_MODEL:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		ScriptParams[0] = pVehicle->GetModelIndex();
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_PLAYER_SITTING_IN_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		UpdateCompareFlag(pPed->GetPedState() == PED_DRIVING && pPed->m_objective != OBJECTIVE_LEAVE_CAR && pPed->m_pMyVehicle == pVehicle);
		return 0;
	}
	case COMMAND_IS_PLAYER_SITTING_IN_ANY_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->GetPedState() == PED_DRIVING && pPed->m_objective != OBJECTIVE_LEAVE_CAR);
		return 0;
	}
	/*
	case COMMAND_SET_SCRIPT_FIRE_AUDIO:
		CollectParameters(&m_nIp, 2);
		gFireManager.SetScriptFireAudio(ScriptParams[0], !!ScriptParams[1]);
		return 0;
	*/
	case COMMAND_ARE_ANY_CAR_CHEATS_ACTIVATED:
		UpdateCompareFlag(CVehicle::bAllDodosCheat || CVehicle::bCheat3 || CVehicle::bHoverCheat || CVehicle::bCheat8 || CVehicle::bCheat9);
		return 0;
	case COMMAND_SET_CHAR_SUFFERS_CRITICAL_HITS:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->bNoCriticalHits = (ScriptParams[1] == 0);
		return 0;
	}
	/*
	case COMMAND_IS_PLAYER_LIFTING_A_PHONE:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->GetPedState() == PED_MAKE_CALL);
		return 0;
	}
	*/
	case COMMAND_IS_CHAR_SITTING_IN_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		script_assert(pVehicle);
		UpdateCompareFlag(pPed->GetPedState() == PED_DRIVING && pPed->m_objective != OBJECTIVE_LEAVE_CAR && pPed->m_pMyVehicle == pVehicle);
		return 0;
	}
	case COMMAND_IS_CHAR_SITTING_IN_ANY_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->GetPedState() == PED_DRIVING && pPed->m_objective != OBJECTIVE_LEAVE_CAR);
		return 0;
	}
	case COMMAND_IS_PLAYER_ON_FOOT:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(!pPed->bInVehicle && pPed->m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER &&
			pPed->m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER);
		return 0;
	}
	case COMMAND_IS_CHAR_ON_FOOT:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(!pPed->bInVehicle && pPed->m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER &&
			pPed->m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER);
		return 0;
	}
	default:
		script_assert(0);
	}
	return -1;
}

int8 CRunningScript::ProcessCommands1100To1199(int32 command)
{
	char tmp[48];
	switch (command) {
	/*
	case COMMAND_LOAD_COLLISION_WITH_SCREEN:
		CollectParameters(&m_nIp, 1);
		CTimer::Stop();
		CGame::currLevel = (eLevelName)ScriptParams[0];
		if (CGame::currLevel != CCollision::ms_collisionInMemory) {
			ISLAND_LOADING_IS(LOW)
			{
				DMAudio.SetEffectsFadeVol(0);
				CPad::StopPadsShaking();
				CCollision::LoadCollisionScreen(CGame::currLevel);
				DMAudio.Service();
			}
			CPopulation::DealWithZoneChange(CCollision::ms_collisionInMemory, CGame::currLevel, false);

			ISLAND_LOADING_IS(LOW)
			{
				CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
				CStreaming::RemoveUnusedBuildings(CGame::currLevel);
			}
			CCollision::SortOutCollisionAfterLoad();

			ISLAND_LOADING_ISNT(HIGH)
				CStreaming::RequestIslands(CGame::currLevel);

			ISLAND_LOADING_IS(LOW)
				CStreaming::RequestBigBuildings(CGame::currLevel);

			ISLAND_LOADING_ISNT(HIGH)
				CStreaming::LoadAllRequestedModels(true);

			ISLAND_LOADING_IS(LOW)
				DMAudio.SetEffectsFadeVol(127);
		}
		CTimer::Update();
		return 0;
	*/
	case COMMAND_LOAD_SPLASH_SCREEN:
		CTheScripts::ReadTextLabelFromScript(&m_nIp, tmp);
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			tmp[i] = tolower(tmp[i]);
		m_nIp += 8;
		LoadSplash(tmp);
		return 0;
	/*
	case COMMAND_SET_CAR_IGNORE_LEVEL_TRANSITIONS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		if (ScriptParams[1])
			pVehicle->m_nZoneLevel = LEVEL_IGNORE;
		else
			pVehicle->m_nZoneLevel = CTheZones::GetLevelFromPosition(&pVehicle->GetPosition());
		return 0;
	}
	case COMMAND_MAKE_CRAIGS_CAR_A_BIT_STRONGER:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		script_assert(pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		CAutomobile* pCar = (CAutomobile*)pVehicle;
		pCar->bMoreResistantToDamage = ScriptParams[1];
		return 0;
	}
	*/
	case COMMAND_SET_JAMES_CAR_ON_PATH_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CCarCtrl::JoinCarWithRoadSystemGotoCoors(pVehicle, FindPlayerCoors(), false);
		return 0;
	}
	case COMMAND_LOAD_END_OF_GAME_TUNE:
		DMAudio.ChangeMusicMode(MUSICMODE_CUTSCENE);
		printf("Start preload end of game audio\n");
		DMAudio.PreloadCutSceneMusic(STREAMED_SOUND_CUTSCENE_FINALE);
		printf("End preload end of game audio\n");
		return 0;
	/*
	case COMMAND_ENABLE_PLAYER_CONTROL_CAMERA:
		CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_CAMERA);
		return 0;
	*/
	case COMMAND_SET_OBJECT_ROTATION:
	{
		CollectParameters(&m_nIp, 4);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CWorld::Remove(pObject);
		pObject->SetOrientation(
			DEGTORAD(*(float*)&ScriptParams[1]),
			DEGTORAD(*(float*)&ScriptParams[2]),
			DEGTORAD(*(float*)&ScriptParams[3]));
		pObject->GetMatrix().UpdateRW();
		pObject->UpdateRwFrame();
		CWorld::Add(pObject);
		return 0;
	}
	case COMMAND_GET_DEBUG_CAMERA_COORDINATES:
		*(CVector*)&ScriptParams[0] = TheCamera.Cams[2].Source;
		StoreParameters(&m_nIp, 3);
		return 0;
	/*
	case COMMAND_GET_DEBUG_CAMERA_FRONT_VECTOR:
		*(CVector*)&ScriptParams[0] = TheCamera.Cams[2].Front;
		StoreParameters(&m_nIp, 3);
		return 0;
	case COMMAND_IS_PLAYER_TARGETTING_ANY_CHAR:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CEntity* pTarget = pPed->m_pPointGunAt;
		UpdateCompareFlag(pTarget && pTarget->IsPed());
		return 0;
	}
	*/
	case COMMAND_IS_PLAYER_TARGETTING_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CPed* pTestedPed = CPools::GetPedPool()->GetAt(ScriptParams[1]);
		script_assert(pTestedPed);
		CEntity* pTarget = pPed->m_pPointGunAt;
		bool bTargetting = pTarget && pTarget->IsPed() && pTarget == pTestedPed;
		// PC shit
		static int nCounter = 0;
		nCounter = Max(0, nCounter - 1);
		if (!pPed->GetWeapon()->IsTypeMelee() && !bTargetting) {
			if ((pTestedPed->GetPosition() - TheCamera.GetPosition()).Magnitude() < 10.0f) {
				CVector vTestedPos(pTestedPed->GetPosition().x, pTestedPed->GetPosition().y, pTestedPed->GetPosition().z + 0.4);
				CVector vScreenPos;
				float w, h;
				if (CSprite::CalcScreenCoors(vTestedPos, &vScreenPos, &w, &h, false)) {
					CVector2D vCrosshairPosition(CCamera::m_f3rdPersonCHairMultX * RsGlobal.maximumWidth, CCamera::m_f3rdPersonCHairMultY * RsGlobal.maximumHeight);
					float fScreenDistance = ((CVector2D)vScreenPos - vCrosshairPosition).Magnitude();
					if (SCREEN_STRETCH_X(0.45f) > fScreenDistance / w) {
						CColPoint point;
						CEntity* entity;
						if (!CWorld::ProcessLineOfSight(TheCamera.GetPosition() + 2.0f * TheCamera.GetForward(),
							vTestedPos, point, entity, true, true, true, true, true, false) ||
							entity == pTestedPed) {
							nCounter += 2;
							if (nCounter > 20) {
								bTargetting = true;
								nCounter = 20;
							}
						}
					}
				}
			}
		}
		UpdateCompareFlag(bTargetting);
		return 0;
	}
	/*
	case COMMAND_IS_PLAYER_TARGETTING_OBJECT:
	{
		CollectParameters(&m_nIp, 2);
		CPlayerPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		CObject* pTestedObject = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
		script_assert(pTestedObject);
		CEntity* pTarget = pPed->m_pPointGunAt;
		UpdateCompareFlag(pTarget && pTarget->IsObject() && pTarget == pTestedObject);
		return 0;
	}
	*/
	case COMMAND_TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME:
	{
		CTheScripts::ReadTextLabelFromScript(&m_nIp, tmp);
		for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++)
			tmp[i] = tolower(tmp[i]);
		m_nIp += 8;
		CRunningScript* pScript = CTheScripts::pActiveScripts;
		while (pScript) {
			CRunningScript* pNext = pScript->next;
			if (strcmp(pScript->m_abScriptName, tmp) == 0) {
				pScript->RemoveScriptFromList(&CTheScripts::pActiveScripts);
				pScript->AddScriptToList(&CTheScripts::pIdleScripts);
			}
			pScript = pNext;
		}
		return 0;
	}
	case COMMAND_DISPLAY_TEXT_WITH_NUMBER:
	{
		CollectParameters(&m_nIp, 2);
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fAtX = *(float*)&ScriptParams[0];
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fAtY = *(float*)&ScriptParams[1];
		CollectParameters(&m_nIp, 1);
		CMessages::InsertNumberInString(text, ScriptParams[0], -1, -1, -1, -1, -1,
			CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame++].m_Text);
		return 0;
	}
	case COMMAND_DISPLAY_TEXT_WITH_2_NUMBERS:
	{
		CollectParameters(&m_nIp, 2);
		wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fAtX = *(float*)&ScriptParams[0];
		CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame].m_fAtY = *(float*)&ScriptParams[1];
		CollectParameters(&m_nIp, 2);
		CMessages::InsertNumberInString(text, ScriptParams[0], ScriptParams[1], -1, -1, -1, -1,
			CTheScripts::IntroTextLines[CTheScripts::NumberOfIntroTextLinesThisFrame++].m_Text);
		return 0;
	}
	case COMMAND_FAIL_CURRENT_MISSION:
		CTheScripts::FailCurrentMission = 2;
#ifdef MISSION_REPLAY
		MissionSkipLevel = 0;
#endif
		return 0;
	case COMMAND_GET_CLOSEST_OBJECT_OF_TYPE:
	{
		return 0;
/*
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float range = *(float*)&ScriptParams[3];
		int mi = ScriptParams[4] < 0 ? CTheScripts::UsedObjectArray[-ScriptParams[4]].index : ScriptParams[4];
		int16 total;
		CEntity* apEntities[16];
		CWorld::FindObjectsOfTypeInRange(mi, pos, range, true, &total, 16, apEntities, false, false, false, true, true);
		CEntity* pClosestEntity = nil;
		float min_dist = 2.0f * range;
		for (int i = 0; i < total; i++) {
			float dist = (apEntities[i]->GetPosition() - pos).Magnitude();
			if (dist < min_dist) {
				min_dist = dist;
				pClosestEntity = apEntities[i];
			}
		}
		if (pClosestEntity && pClosestEntity->IsDummy()) {
			CPopulation::ConvertToRealObject((CDummyObject*)pClosestEntity);
			CWorld::FindObjectsOfTypeInRange(mi, pos, range, true, &total, 16, apEntities, false, false, false, true, true);
			pClosestEntity = nil;
			float min_dist = 2.0f * range;
			for (int i = 0; i < total; i++) {
				float dist = (apEntities[i]->GetPosition() - pos).Magnitude();
				if (dist < min_dist) {
					min_dist = dist;
					pClosestEntity = apEntities[i];
				}
			}
			if (pClosestEntity->IsDummy())
				pClosestEntity = nil;
		}
		if (pClosestEntity) {
			script_assert(pClosestEntity->IsObject());
			CObject* pObject = (CObject*)pClosestEntity;
			pObject->ObjectCreatedBy = MISSION_OBJECT;
			ScriptParams[0] = CPools::GetObjectPool()->GetIndex(pObject);
		} else {
			ScriptParams[0] = -1;
		}
		StoreParameters(&m_nIp, 1);
		return 0;
*/
	}
	/*
	case COMMAND_PLACE_OBJECT_RELATIVE_TO_OBJECT:
	{
		CollectParameters(&m_nIp, 5);
		CObject* pObject = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		script_assert(pObject);
		CObject* pTarget = CPools::GetObjectPool()->GetAt(ScriptParams[1]);
		script_assert(pTarget);
		CVector offset = *(CVector*)&ScriptParams[2];
		CPhysical::PlacePhysicalRelativeToOtherPhysical(pTarget, pObject, offset);
		return 0;
	}
	*/
	case COMMAND_SET_ALL_OCCUPANTS_OF_CAR_LEAVE_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CCarAI::TellOccupantsToLeaveCar(pVehicle);
		return 0;
	}
	case COMMAND_SET_INTERPOLATION_PARAMETERS:
		CollectParameters(&m_nIp, 2);
		TheCamera.SetParametersForScriptInterpolation(*(float*)&ScriptParams[0], 100.0f - *(float*)&ScriptParams[0], ScriptParams[1]);
		return 0;
	/*
	case COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING_TOWARDS_POINT:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float destX = *(float*)&ScriptParams[3];
		float destY = *(float*)&ScriptParams[4];
		int32 nid = ThePaths.FindNodeClosestToCoors(pos, 0, 999999.9f, true, true);
		CPathNode* pNode = &ThePaths.m_pathNodes[nid];
		*(CVector*)&ScriptParams[0] = pNode->GetPosition();
		*(float*)&ScriptParams[3] = ThePaths.FindNodeOrientationForCarPlacementFacingDestination(nid, destX, destY, true);
		StoreParameters(&m_nIp, 4);
		return 0;
	}
	case COMMAND_GET_CLOSEST_CAR_NODE_WITH_HEADING_AWAY_POINT:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		float destX = *(float*)&ScriptParams[3];
		float destY = *(float*)&ScriptParams[4];
		int32 nid = ThePaths.FindNodeClosestToCoors(pos, 0, 999999.9f, true, true);
		CPathNode* pNode = &ThePaths.m_pathNodes[nid];
		*(CVector*)&ScriptParams[0] = pNode->GetPosition();
		*(float*)&ScriptParams[3] = ThePaths.FindNodeOrientationForCarPlacementFacingDestination(nid, destX, destY, false);
		StoreParameters(&m_nIp, 4);
		return 0;
	}
	*/
	case COMMAND_GET_DEBUG_CAMERA_POINT_AT:
		*(CVector*)&ScriptParams[0] = TheCamera.Cams[2].Source + TheCamera.Cams[2].Front;
		StoreParameters(&m_nIp, 3);
		return 0;
	case COMMAND_ATTACH_CHAR_TO_CAR:
	{
		CollectParameters(&m_nIp, 8);
		CPed *pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CVehicle *pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		pPed->AttachPedToEntity(pVehicle, *(CVector*)&ScriptParams[2], ScriptParams[5], DEGTORAD(*(float*)&ScriptParams[6]), (eWeaponType)ScriptParams[7]);
		return 0;
	}
	case COMMAND_DETACH_CHAR_FROM_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed *pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		if (pPed && pPed->m_attachedTo)
			pPed->DettachPedFromEntity();
		return 0;
	}
	case COMMAND_SET_CAR_CHANGE_LANE: // for some reason changed in SA
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->AutoPilot.m_bStayInFastLane = !ScriptParams[1];
		return 0;
	}
	case COMMAND_CLEAR_CHAR_LAST_WEAPON_DAMAGE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		if (pPed)
			pPed->m_lastWepDam = -1;
		else
			debug("CLEAR_CHAR_LAST_WEAPON_DAMAGE - Character doesn't exist\n");
		return 0;
	}
	case COMMAND_CLEAR_CAR_LAST_WEAPON_DAMAGE:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		if (pVehicle)
			pVehicle->m_nLastWeaponDamage = -1;
		else
			debug("CLEAR_CAR_LAST_WEAPON_DAMAGE - Vehicle doesn't exist\n");
		return 0;
	}
	case COMMAND_GET_RANDOM_COP_IN_AREA:
	{
		CollectParameters(&m_nIp, 9);
		int ped_handle = -1;
		CVector pos = FindPlayerCoors();
		float x1 = *(float*)&ScriptParams[0];
		float y1 = *(float*)&ScriptParams[1];
		float x2 = *(float*)&ScriptParams[2];
		float y2 = *(float*)&ScriptParams[3];
		int i = CPools::GetPedPool()->GetSize();
		while (--i && ped_handle == -1) {
			CPed* pPed = CPools::GetPedPool()->GetSlot(i);
			if (!pPed)
				continue;
			if (CTheScripts::LastRandomPedId == CPools::GetPedPool()->GetIndex(pPed))
				continue;
			if (pPed->m_nPedType != PEDTYPE_COP)
				continue;
			if (!ThisIsAValidRandomCop(pPed->GetModelIndex(), ScriptParams[4], ScriptParams[5], ScriptParams[6], ScriptParams[7], ScriptParams[8]))
				continue;
			if (pPed->CharCreatedBy != RANDOM_CHAR)
				continue;
			if (!pPed->IsPedInControl() && pPed->GetPedState() != PED_DRIVING && pPed->GetPedState() != PED_ABSEIL)
				continue;
			if (pPed->bRemoveFromWorld)
				continue;
			if (pPed->bFadeOut)
				continue;
			if (pPed->bIsLeader || pPed->m_leader)
				continue;
			if (!pPed->IsWithinArea(x1, y1, x2, y2))
				continue;
			if (pos.z - COP_PED_FIND_Z_OFFSET > pPed->GetPosition().z)
				continue;
			if (pos.z + COP_PED_FIND_Z_OFFSET < pPed->GetPosition().z)
				continue;
			ped_handle = CPools::GetPedPool()->GetIndex(pPed);
			CTheScripts::LastRandomPedId = ped_handle;
			pPed->CharCreatedBy = MISSION_CHAR;
			pPed->bRespondsToThreats = false;
			++CPopulation::ms_nTotalMissionPeds;
			if (m_bIsMissionScript)
				CTheScripts::MissionCleanUp.AddEntityToList(ped_handle, CLEANUP_CHAR);
		}
		ScriptParams[0] = ped_handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	/*
	case COMMAND_GET_RANDOM_COP_IN_ZONE:
	{
		char zone[KEY_LENGTH_IN_SCRIPT];
		strncpy(zone, (const char*)&CTheScripts::ScriptSpace[m_nIp], KEY_LENGTH_IN_SCRIPT);
		int nZone = CTheZones::FindZoneByLabelAndReturnIndex(zone, ZONE_DEFAULT);
		if (nZone != -1)
			m_nIp += KEY_LENGTH_IN_SCRIPT;
		CZone* pZone = CTheZones::GetNavigationZone(nZone);
		int ped_handle = -1;
		CVector pos = FindPlayerCoors();
		int i = CPools::GetPedPool()->GetSize();
		while (--i && ped_handle == -1) {
			CPed* pPed = CPools::GetPedPool()->GetSlot(i);
			if (!pPed)
				continue;
			if (CTheScripts::LastRandomPedId == CPools::GetPedPool()->GetIndex(pPed))
				continue;
			if (pPed->m_nPedType != PEDTYPE_COP)
				continue;
			if (pPed->CharCreatedBy != RANDOM_CHAR)
				continue;
			if (!pPed->IsPedInControl() && pPed->GetPedState() != PED_DRIVING)
				continue;
			if (pPed->bRemoveFromWorld)
				continue;
			if (pPed->bFadeOut)
				continue;
			if (pPed->bIsLeader || pPed->m_leader)
				continue;
			if (!CTheZones::PointLiesWithinZone(&pPed->GetPosition(), pZone))
				continue;
			if (pos.z - COP_PED_FIND_Z_OFFSET > pPed->GetPosition().z)
				continue;
			if (pos.z + COP_PED_FIND_Z_OFFSET < pPed->GetPosition().z)
				continue;
			ped_handle = CPools::GetPedPool()->GetIndex(pPed);
			CTheScripts::LastRandomPedId = ped_handle;
			pPed->CharCreatedBy = MISSION_CHAR;
			pPed->bRespondsToThreats = false;
			++CPopulation::ms_nTotalMissionPeds;
			if (m_bIsMissionScript)
				CTheScripts::MissionCleanUp.AddEntityToList(ped_handle, CLEANUP_CHAR);
		}
		ScriptParams[0] = ped_handle;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	*/
	case COMMAND_SET_CHAR_OBJ_FLEE_CAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[1]);
		script_assert(pVehicle);
		pPed->bScriptObjectiveCompleted = false;
		pPed->SetObjective(OBJECTIVE_FLEE_CAR, pVehicle);
		return 0;
	}
	case COMMAND_GET_DRIVER_OF_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		CPed* pDriver = pVehicle->pDriver;
		if (pDriver)
			ScriptParams[0] = CPools::GetPedPool()->GetIndex(pDriver);
		else
			ScriptParams[0] = -1;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_NUMBER_OF_FOLLOWERS:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pLeader = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pLeader);
		int total = 0;
		int i = CPools::GetPedPool()->GetSize();
		while (--i) {
			CPed* pPed = CPools::GetPedPool()->GetSlot(i);
			if (!pPed)
				continue;
			if (pPed->m_leader == pLeader)
				total++;
		}
		ScriptParams[0] = total;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GIVE_REMOTE_CONTROLLED_MODEL_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 6);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y);
		CRemote::GivePlayerRemoteControlledCar(pos.x, pos.y, pos.z, DEGTORAD(*(float*)&ScriptParams[4]), ScriptParams[5]);
		return 0;
	}
	case COMMAND_GET_CURRENT_PLAYER_WEAPON:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		ScriptParams[0] = pPed->GetWeapon()->m_eWeaponType;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_CURRENT_CHAR_WEAPON:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		ScriptParams[0] = pPed->GetWeapon()->m_eWeaponType;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_2D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_2D:
	case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_2D:
	case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D:
		LocateCharObjectCommand(command, &m_nIp);
		return 0;
	case COMMAND_SET_CAR_TEMP_ACTION:
	{
		CollectParameters(&m_nIp, 3);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->AutoPilot.m_nTempAction = (uint8)ScriptParams[1];
		pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + ScriptParams[2];
		return 0;
	}
	/*
	case COMMAND_SET_CAR_HANDBRAKE_TURN_RIGHT:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->AutoPilot.m_nTempAction = TEMPACT_HANDBRAKETURNRIGHT;
		pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_CAR_HANDBRAKE_STOP:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->AutoPilot.m_nTempAction = TEMPACT_HANDBRAKESTRAIGHT;
		pVehicle->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + ScriptParams[1];
		return 0;
	}
	*/
	case COMMAND_IS_CHAR_ON_ANY_BIKE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle&& pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE);
		return 0;
	}
	/*
	case COMMAND_LOCATE_SNIPER_BULLET_2D:
	case COMMAND_LOCATE_SNIPER_BULLET_3D:
		LocateSniperBulletCommand(command, &m_nIp);
		return 0;
	*/
	case COMMAND_GET_NUMBER_OF_SEATS_IN_MODEL:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = CVehicleModelInfo::GetMaximumNumberOfPassengersFromNumberOfDoors(ScriptParams[0]) + 1;
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_IS_PLAYER_ON_ANY_BIKE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE);
		return 0;
	}
	/*
	case COMMAND_IS_CHAR_LYING_DOWN:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->bFallenDown);
		return 0;
	}
	*/
	case COMMAND_CAN_CHAR_SEE_DEAD_CHAR:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		int pedtype = ScriptParams[1];
		bool can = false;
		for (int i = 0; i < pPed->m_numNearPeds; i++) {
			CPed* pTestPed = pPed->m_nearPeds[i];
			if (pTestPed->m_fHealth <= 0.0f && pTestPed->m_nPedType == pedtype && pPed->OurPedCanSeeThisOne(pTestPed))
				can = true;
		}
		UpdateCompareFlag(can);
		return 0;
	}
	case COMMAND_SET_ENTER_CAR_RANGE_MULTIPLIER:
		CollectParameters(&m_nIp, 1);
		CPed::nEnterCarRangeMultiplier = *(float*)&ScriptParams[0];
		return 0;
	case COMMAND_SET_THREAT_REACTION_RANGE_MULTIPLIER:
		CollectParameters(&m_nIp, 1);
		CPed::nThreatReactionRangeMultiplier = *(float*)&ScriptParams[0];
		return 0;
	case COMMAND_SET_CHAR_CEASE_ATTACK_TIMER:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->m_ceaseAttackTimer = ScriptParams[1];
		return 0;
	}
	case COMMAND_GET_REMOTE_CONTROLLED_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CWorld::Players[ScriptParams[0]].m_pRemoteVehicle;
		if (pVehicle)
			ScriptParams[0] = CPools::GetVehiclePool()->GetIndex(pVehicle);
		else
			ScriptParams[0] = -1;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_PC_VERSION:
		UpdateCompareFlag(true);
		return 0;
	//case COMMAND_REPLAY:
	//case COMMAND_IS_REPLAY_PLAYING:
	case COMMAND_IS_MODEL_AVAILABLE:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(CModelInfo::GetModelInfo(ScriptParams[0]) != nil);
		return 0;
	case COMMAND_SHUT_CHAR_UP:
		CollectParameters(&m_nIp, 2);
		DMAudio.SetPedTalkingStatus(CPools::GetPedPool()->GetAt(ScriptParams[0]), ScriptParams[1] == 0);
		return 0;
	case COMMAND_SET_ENABLE_RC_DETONATE:
		CollectParameters(&m_nIp, 1);
		CVehicle::bDisableRemoteDetonation = !ScriptParams[0];
		return 0;
	case COMMAND_SET_CAR_RANDOM_ROUTE_SEED:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		pVehicle->m_nRouteSeed = ScriptParams[1];
		return 0;
	}
	case COMMAND_IS_ANY_PICKUP_AT_COORDS:
	{
		CollectParameters(&m_nIp, 3);
		CVector pos = *(CVector*)&ScriptParams[0];
		CRunningScript::UpdateCompareFlag(CPickups::TestForPickupsInBubble(pos, 0.5f));
		return 0;
	}
	case COMMAND_GET_FIRST_PICKUP_COORDS:
	case COMMAND_GET_NEXT_PICKUP_COORDS:
	case COMMAND_REMOVE_ALL_CHAR_WEAPONS:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		pPed->ClearWeapons();
		return 0;
	}
	case COMMAND_HAS_PLAYER_GOT_WEAPON:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		bool bFound = false;
		for (int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {
			if (pPed->GetWeapon(i).m_eWeaponType == ScriptParams[1]) {
				bFound = true;
				break;
			}
		}
		UpdateCompareFlag(bFound);
		return 0;
	}
	//case COMMAND_HAS_CHAR_GOT_WEAPON:
	//case COMMAND_IS_PLAYER_FACING_CHAR:
	case COMMAND_SET_TANK_DETONATE_CARS:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle && pVehicle->m_vehType == VEHICLE_TYPE_CAR);
		((CAutomobile*)pVehicle)->bTankDetonateCars = ScriptParams[1];
		return 0;
	}
	case COMMAND_GET_POSITION_OF_ANALOGUE_STICKS:
	{
		CollectParameters(&m_nIp, 1);
		CPad* pPad = CPad::GetPad(ScriptParams[0]);
		ScriptParams[0] = pPad->NewState.LeftStickX;
		ScriptParams[1] = pPad->NewState.LeftStickY;
		ScriptParams[2] = pPad->NewState.RightStickX;
		ScriptParams[3] = pPad->NewState.RightStickY;
		StoreParameters(&m_nIp, 4);
		return 0;
	}
	case COMMAND_IS_CAR_ON_FIRE:
	{
		CollectParameters(&m_nIp, 1);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		bool bOnFire = false;
		if (pVehicle->m_pCarFire)
			bOnFire = true;
		if (pVehicle->m_vehType == VEHICLE_TYPE_CAR && ((CAutomobile*)pVehicle)->Damage.GetEngineStatus() >= ENGINE_STATUS_ON_FIRE)
			bOnFire = true;
		if (pVehicle->m_fHealth < 250.0f)
			bOnFire = true;
		UpdateCompareFlag(bOnFire);
		return 0;
	}
	case COMMAND_IS_CAR_TYRE_BURST:
	{
		CollectParameters(&m_nIp, 2);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle);
		bool bIsBurst = false;
		CBike* pBike = (CBike*)pVehicle;
		if (pVehicle->IsBike()) {
			if (ScriptParams[1] == 4) {
				for (int i = 0; i < 2; i++) {
					if (pBike->m_wheelStatus[i] == WHEEL_STATUS_BURST)
						bIsBurst = true;
				}
			}
			else {
				if (ScriptParams[1] == 2)
					ScriptParams[1] = 0;
				if (ScriptParams[1] == 3)
					ScriptParams[1] = 1;
				bIsBurst = pBike->m_wheelStatus[ScriptParams[1]] == WHEEL_STATUS_BURST;
			}
		}
		else {
			CAutomobile* pCar = (CAutomobile*)pVehicle;
			if (ScriptParams[1] == 4) {
				for (int i = 0; i < 4; i++) {
					if (pCar->Damage.GetWheelStatus(i) == WHEEL_STATUS_BURST)
						bIsBurst = true;
				}
			}
			else
				bIsBurst = pCar->Damage.GetWheelStatus(ScriptParams[1] == WHEEL_STATUS_BURST);
		}
		UpdateCompareFlag(bIsBurst);
		return 0;
	}
	//case COMMAND_SET_CAR_DRIVE_STRAIGHT_AHEAD:
	//case COMMAND_SET_CAR_WAIT:
	//case COMMAND_IS_PLAYER_STANDING_ON_A_VEHICLE:
	//case COMMAND_IS_PLAYER_FOOT_DOWN:
	//case COMMAND_IS_CHAR_FOOT_DOWN:
	case COMMAND_INITIALISE_OBJECT_PATH: {
		CollectParameters(&m_nIp, 2);
		int32 counter = 0;
		while (counter < 3 && CScriptPaths::aArray[counter].m_state != SCRIPT_PATH_DISABLED) {
			counter++;
		}
		CScriptPaths::aArray[counter].InitialiseOne(ScriptParams[0], *(float*)&ScriptParams[1]);
		ScriptParams[0] = counter;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_START_OBJECT_ON_PATH:
	{
		CollectParameters(&m_nIp, 2);
		CObject *pObj = CPools::GetObjectPool()->GetAt(ScriptParams[0]);
		assert(pObj);
		CScriptPaths::aArray[ScriptParams[1]].SetObjectToControl(pObj);
		return 0;
	}
	case COMMAND_SET_OBJECT_PATH_SPEED:
	{
		CollectParameters(&m_nIp, 2);
		CScriptPaths::aArray[ScriptParams[0]].m_fSpeed = *(float*)&ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_OBJECT_PATH_POSITION:
	{
		CollectParameters(&m_nIp, 2);
		CScriptPaths::aArray[ScriptParams[0]].m_fPosition = *(float*)&ScriptParams[1];
		return 0;
	}
	//case COMMAND_GET_OBJECT_DISTANCE_ALONG_PATH:
	case COMMAND_CLEAR_OBJECT_PATH:
	{
		CollectParameters(&m_nIp, 1);
		CScriptPaths::aArray[ScriptParams[0]].Clear();
		return 0;
	}
	case COMMAND_HELI_GOTO_COORDS:
	{
		CollectParameters(&m_nIp, 5);
		CVehicle* pVehicle = CPools::GetVehiclePool()->GetAt(ScriptParams[0]);
		script_assert(pVehicle && pVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI);
		((CAutomobile*)pVehicle)->TellHeliToGoToCoors(*(float*)&ScriptParams[1], *(float*)&ScriptParams[2], *(float*)&ScriptParams[3], ScriptParams[4]);
		return 0;
	}
	case COMMAND_IS_INT_VAR_EQUAL_TO_CONSTANT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr == ScriptParams[0]);
		return 0;
	}
	case COMMAND_IS_INT_LVAR_EQUAL_TO_CONSTANT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(*ptr == ScriptParams[0]);
		return 0;
	}
	case COMMAND_GET_DEAD_CHAR_PICKUP_COORDS:
	{
		CollectParameters(&m_nIp, 1);
		CPed *pTarget = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		CVector pos;
		pTarget->CreateDeadPedPickupCoors(&pos.x, &pos.y, &pos.z);
		*(CVector*)&ScriptParams[0] = pos;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_CREATE_PROTECTION_PICKUP:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y) + PICKUP_PLACEMENT_OFFSET;
		CPickups::GetActualPickupIndex(CollectNextParameterWithoutIncreasingPC(m_nIp));
		ScriptParams[0] = CPickups::GenerateNewOne(pos, MI_PICKUP_REVENUE, PICKUP_ASSET_REVENUE, ScriptParams[3], ScriptParams[4]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_ANY_BOAT:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BOAT);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_ANY_BOAT:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_BOAT);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_ANY_HELI:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_ANY_HELI:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_ANY_PLANE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_ANY_PLANE:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CWorld::Players[ScriptParams[0]].m_pPed;
		script_assert(pPed);
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_WATER:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
		UpdateCompareFlag(pPed && pPed->bIsInWater);
		return 0;
	}
	case COMMAND_SET_VAR_INT_TO_CONSTANT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		CollectParameters(&m_nIp, 1);
		*ptr = ScriptParams[0];
		return 0;
	}
	case COMMAND_SET_LVAR_INT_TO_CONSTANT:
	{
		int32* ptr = GetPointerToScriptVariable(&m_nIp, VAR_LOCAL);
		CollectParameters(&m_nIp, 1);
		*ptr = ScriptParams[0];
		return 0;
	}
	default:
		script_assert(0);
	}
	return -1;
}
