#include "common.h"

#include "Pools.h"
#include "ModelIndices.h"
#include "Timer.h"
#include "World.h"
#include "ZoneCull.h"
#include "Darkel.h"
#include "DMAudio.h"
#include "CopPed.h"
#include "Wanted.h"
#include "General.h"
#include "Stats.h"

int32 CWanted::MaximumWantedLevel = 6;
int32 CWanted::nMaximumWantedLevel = 9600;

void
CWanted::Initialise()
{
	m_nChaos = 0;
	m_nMinChaos = 0;
	m_nLastUpdateTime = 0;
	m_nLastWantedLevelChange = 0;
	m_nLastTimeSuspended = 0;
	m_CurrentCops = 0;
	m_MaxCops = 0;
	m_MaximumLawEnforcerVehicles = 0;
	m_RoadblockDensity = 0;
	m_bIgnoredByCops = false;
	m_bIgnoredByEveryone = false;
	m_bSwatRequired = false;
	m_bFbiRequired = false;
	m_bArmyRequired = false;
	m_fCrimeSensitivity = 1.0f;
	m_nWantedLevel = 0;
	m_nMinWantedLevel = 0;
	m_CopsBeatingSuspect = 0;

	for (int i = 0; i < ARRAY_SIZE(m_pCops); i++)
		m_pCops[i] = nil;

	ClearQdCrimes();
}

bool
CWanted::AreMiamiViceRequired()
{
	return m_nWantedLevel >= 3;
}

bool
CWanted::AreSwatRequired()
{
	return m_nWantedLevel == 4 || m_bSwatRequired;
}

bool
CWanted::AreFbiRequired()
{
	return m_nWantedLevel == 5 || m_bFbiRequired;
}

bool
CWanted::AreArmyRequired()
{
	return m_nWantedLevel == 6 || m_bArmyRequired;
}

int32
CWanted::NumOfHelisRequired()
{
	if (m_bIgnoredByCops || m_bIgnoredByEveryone)
		return 0;

	switch (m_nWantedLevel) {
	case 3:
	case 4:
		return 1;
	case 5:
	case 6:
		return 1;
	default:
		return 0;
	}
}

void
CWanted::SetWantedLevel(int32 level)
{
	if (level > MaximumWantedLevel)
		level = MaximumWantedLevel;

	ClearQdCrimes();
	switch (level) {
	case 0:
		m_nChaos = 0;
		break;
	case 1:
		m_nChaos = 70;
		break;
	case 2:
		m_nChaos = 200;
		break;
	case 3:
		m_nChaos = 570;
		break;
	case 4:
		m_nChaos = 1220;
		break;
	case 5:
		m_nChaos = 2420;
		break;
	case 6:
		m_nChaos = 4820;
		break;
	default:
		break;
	}
	UpdateWantedLevel();
}

void
CWanted::SetWantedLevelNoDrop(int32 level)
{
	if (m_nWantedLevel < m_nMinWantedLevel)
		SetWantedLevel(m_nMinWantedLevel);

	if (level > m_nWantedLevel)
		SetWantedLevel(level);
}

void
CWanted::CheatWantedLevel(int32 level)
{
	SetWantedLevel(level);
	UpdateWantedLevel();
}

void
CWanted::SetMaximumWantedLevel(int32 level)
{
	switch(level){
	case 0:
		nMaximumWantedLevel = 0;
		MaximumWantedLevel = 0;
		break;
	case 1:
		nMaximumWantedLevel = 115;
		MaximumWantedLevel = 1;
		break;
	case 2:
		nMaximumWantedLevel = 365;
		MaximumWantedLevel = 2;
		break;
	case 3:
		nMaximumWantedLevel = 875;
		MaximumWantedLevel = 3;
		break;
	case 4:
		nMaximumWantedLevel = 1800;
		MaximumWantedLevel = 4;
		break;
	case 5:
		nMaximumWantedLevel = 3600;
		MaximumWantedLevel = 5;
		break;
	case 6:
		nMaximumWantedLevel = 7200;
		MaximumWantedLevel = 6;
		break;
	}
}

void
CWanted::RegisterCrime(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare)
{
	AddCrimeToQ(type, id, coors, false, policeDoesntCare);
}

void
CWanted::RegisterCrime_Immediately(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare)
{
#ifdef FIX_SIGNIFICANT_BUGS
	if(!AddCrimeToQ(type, id, coors, true, policeDoesntCare))
#else
	if(!AddCrimeToQ(type, id, coors, false, policeDoesntCare))
#endif
		ReportCrimeNow(type, coors, policeDoesntCare);
}

void
CWanted::ClearQdCrimes()
{
	for (int i = 0; i < 16; i++)
		m_aCrimes[i].m_nType = CRIME_NONE;
}

// returns whether the crime had been reported already
bool
CWanted::AddCrimeToQ(eCrimeType type, int32 id, const CVector &coors, bool reported, bool policeDoesntCare)
{
	int i;

	for(i = 0; i < 16; i++)
		if(m_aCrimes[i].m_nType == type && m_aCrimes[i].m_nId == id){
			if(m_aCrimes[i].m_bReported)
				return true;
			if(reported)
				m_aCrimes[i].m_bReported = reported;
			return false;
		}

	for(i = 0; i < 16; i++)
		if(m_aCrimes[i].m_nType == CRIME_NONE)
			break;
	if(i < 16){
		m_aCrimes[i].m_nType = type;
		m_aCrimes[i].m_nId = id;
		m_aCrimes[i].m_vecPosn = coors;
		m_aCrimes[i].m_nTime = CTimer::GetTimeInMilliseconds();
		m_aCrimes[i].m_bReported = reported;
		m_aCrimes[i].m_bPoliceDoesntCare = policeDoesntCare;
	}
	return false;
}

void
CWanted::ReportCrimeNow(eCrimeType type, const CVector &coors, bool policeDoesntCare)
{
	float sensitivity, chaos;
	int wantedLevelDrop;

	if(CDarkel::FrenzyOnGoing())
		sensitivity = m_fCrimeSensitivity*0.3f;
	else
		sensitivity = m_fCrimeSensitivity;

	wantedLevelDrop = Min(CCullZones::GetWantedLevelDrop(), 100);

	chaos = (1.0f - wantedLevelDrop/100.0f) * sensitivity;
	if (policeDoesntCare)
		chaos *= 0.333f;
	switch(type){
	case CRIME_POSSESSION_GUN:
		break;
	case CRIME_HIT_PED:
		m_nChaos += 5.0f*chaos;
		break;
	case CRIME_HIT_COP:
		m_nChaos += 45.0f*chaos;
		break;
	case CRIME_SHOOT_PED:
		m_nChaos += 30.0f*chaos;
		break;
	case CRIME_SHOOT_COP:
		m_nChaos += 80.0f*chaos;
		break;
	case CRIME_STEAL_CAR:
		m_nChaos += 15.0f*chaos;
		break;
	case CRIME_RUN_REDLIGHT:
		m_nChaos += 10.0f*chaos;
		break;
	case CRIME_RECKLESS_DRIVING:
		m_nChaos += 5.0f*chaos;
		break;
	case CRIME_SPEEDING:
		m_nChaos += 5.0f*chaos;
		break;
	case CRIME_RUNOVER_PED:
		m_nChaos += 18.0f*chaos;
		break;
	case CRIME_RUNOVER_COP:
		m_nChaos += 80.0f*chaos;
		break;
	case CRIME_SHOOT_HELI:
		m_nChaos += 400.0f*chaos;
		break;
	case CRIME_PED_BURNED:
		m_nChaos += 20.0f*chaos;
		break;
	case CRIME_COP_BURNED:
		m_nChaos += 80.0f*chaos;
		break;
	case CRIME_VEHICLE_BURNED:
		m_nChaos += 20.0f*chaos;
		break;
	case CRIME_DESTROYED_CESSNA:
		m_nChaos += 500.0f*chaos;
		break;
	case CRIME_EXPLOSION:
		m_nChaos += 25.0f * chaos;
		break;
	case CRIME_HIT_PED_NASTYWEAPON:
		m_nChaos += 35.0f * chaos;
		break;
	case CRIME_HIT_COP_NASTYWEAPON:
		m_nChaos += 100.0f * chaos;
		break;
	default:
	//	Error("Undefined crime type, RegisterCrime, Crime.cpp");	// different file for some reason
		Error("Undefined crime type, RegisterCrime, Wanted.cpp");
	}
	m_nChaos = Max(m_nChaos, m_nMinChaos);
	DMAudio.ReportCrime(type, coors);
	UpdateWantedLevel();
}

void
CWanted::UpdateWantedLevel()
{
	int32 CurrWantedLevel = m_nWantedLevel;

	if (m_nChaos > nMaximumWantedLevel)
		m_nChaos = nMaximumWantedLevel;

	if (m_nChaos >= 0 && m_nChaos < 50) {
		if (m_nWantedLevel == 1)
			++CStats::WantedStarsEvaded;
		m_nWantedLevel = 0;
		m_MaximumLawEnforcerVehicles = 0;
		m_MaxCops = 0;
		m_RoadblockDensity = 0;
	} else if (m_nChaos >= 50 && m_nChaos < 180) {
		CStats::WantedStarsAttained += 1 - m_nWantedLevel;
		m_nWantedLevel = 1;
		m_MaximumLawEnforcerVehicles = 1;
		m_MaxCops = 1;
		m_RoadblockDensity = 0;
	} else if (m_nChaos >= 180 && m_nChaos < 550) {
		CStats::WantedStarsAttained += 2 - m_nWantedLevel;
		m_nWantedLevel = 2;
		m_MaximumLawEnforcerVehicles = 2;
		m_MaxCops = 3;
		m_RoadblockDensity = 0;
	} else if (m_nChaos >= 550 && m_nChaos < 1200) {
		CStats::WantedStarsAttained += 3 - m_nWantedLevel;
		m_nWantedLevel = 3;
		m_MaximumLawEnforcerVehicles = 2;
		m_MaxCops = 4;
		m_RoadblockDensity = 12;
	} else if (m_nChaos >= 1200 && m_nChaos < 2400) {
		CStats::WantedStarsAttained += 4 - m_nWantedLevel;
		m_nWantedLevel = 4;
		m_MaximumLawEnforcerVehicles = 2;
		m_MaxCops = 6;
		m_RoadblockDensity = 18;
	} else if (m_nChaos >= 2400 && m_nChaos < 4800) {
		CStats::WantedStarsAttained += 5 - m_nWantedLevel;
		m_nWantedLevel = 5;
		m_MaximumLawEnforcerVehicles = 3;
		m_MaxCops = 8;
		m_RoadblockDensity = 24;
	} else if (m_nChaos >= 4800) {
		CStats::WantedStarsAttained += 6 - m_nWantedLevel;
		m_nWantedLevel = 6;
		m_MaximumLawEnforcerVehicles = 3;
		m_MaxCops = 10;
		m_RoadblockDensity = 30;
	}

	if (CurrWantedLevel != m_nWantedLevel)
		m_nLastWantedLevelChange = CTimer::GetTimeInMilliseconds();
}

int32
CWanted::WorkOutPolicePresence(CVector posn, float radius)
{
	int i;
	CPed *ped;
	CVehicle *vehicle;
	int numPolice = 0;

	i = CPools::GetPedPool()->GetSize();
	while(--i >= 0){
		ped = CPools::GetPedPool()->GetSlot(i);
		if(ped &&
		   IsPolicePedModel(ped->GetModelIndex()) &&
		   (posn - ped->GetPosition()).Magnitude() < radius)
			numPolice++;
	}

	i = CPools::GetVehiclePool()->GetSize();
	while(--i >= 0){
		vehicle = CPools::GetVehiclePool()->GetSlot(i);
		if(vehicle &&
		   vehicle->bIsLawEnforcer &&
		   IsPoliceVehicleModel(vehicle->GetModelIndex()) &&
		   vehicle != FindPlayerVehicle() &&
		   vehicle->GetStatus() != STATUS_ABANDONED && vehicle->GetStatus() != STATUS_WRECKED &&
		   (posn - vehicle->GetPosition()).Magnitude() < radius)
			numPolice++;
	}

	return numPolice;
}

void
CWanted::Update(void)
{
	if (CTimer::GetTimeInMilliseconds() > m_nLastTimeSuspended + 20000) {
		m_nMinChaos = 0;
		m_nMinWantedLevel = 0;
	}
	if (CTimer::GetTimeInMilliseconds() - m_nLastUpdateTime > 1000) {
		if (m_nWantedLevel > 1) {
			m_nLastUpdateTime = CTimer::GetTimeInMilliseconds();
		} else {
			float radius = 18.0f;
			CVector playerPos = FindPlayerCoors();
			if (WorkOutPolicePresence(playerPos, radius) == 0) {
				m_nLastUpdateTime = CTimer::GetTimeInMilliseconds();
				m_nChaos = Max(0, m_nChaos - 1);
				UpdateWantedLevel();
			}
		}
		UpdateCrimesQ();
		bool orderMessedUp = false;
		int currCopNum = 0;
		bool foundEmptySlot = false;
		for (int i = 0; i < ARRAY_SIZE(m_pCops); i++) {
			if (m_pCops[i]) {
				++currCopNum;
				if (foundEmptySlot)
					orderMessedUp = true;
			} else {
				foundEmptySlot = true;
			}
		}
		if (currCopNum != m_CurrentCops) {
			printf("CopPursuit total messed up: re-setting\n");
			m_CurrentCops = currCopNum;
		}
		if (orderMessedUp) {
			printf("CopPursuit pointer list messed up: re-sorting\n");
			bool fixed = true;
			for (int i = 0; i < ARRAY_SIZE(m_pCops); i++) {
				if (!m_pCops[i]) {
					for (int j = i; j < ARRAY_SIZE(m_pCops); j++) {
						if (m_pCops[j]) {
							m_pCops[i] = m_pCops[j];
							m_pCops[j] = nil;
							fixed = false;
							break;
						}
					}
					if (fixed)
						break;
				}
			}
		}
	}
}

void
CWanted::ResetPolicePursuit(void)
{
	for(int i = 0; i < ARRAY_SIZE(m_pCops); i++) {
		CCopPed *cop = m_pCops[i];
		if (!cop)
			continue;

		cop->m_bIsInPursuit = false;
		cop->m_objective = OBJECTIVE_NONE;
		cop->m_prevObjective = OBJECTIVE_NONE;
		cop->m_nLastPedState = PED_NONE;
		if (!cop->DyingOrDead()) {
			cop->SetWanderPath(CGeneral::GetRandomNumberInRange(0.0f, 8.0f));
		}
		m_pCops[i] = nil;
	}
	m_CurrentCops = 0;
}

void
CWanted::Reset(void)
{
	ResetPolicePursuit();
	Initialise();
}

void
CWanted::UpdateCrimesQ(void)
{
	for(int i = 0; i < ARRAY_SIZE(m_aCrimes); i++) {

		CCrimeBeingQd &crime = m_aCrimes[i];
		if (crime.m_nType != CRIME_NONE) {
			if (CTimer::GetTimeInMilliseconds() > crime.m_nTime + 500 && !crime.m_bReported) {
				ReportCrimeNow(crime.m_nType, crime.m_vecPosn, crime.m_bPoliceDoesntCare);
				crime.m_bReported = true;
			}
			if (CTimer::GetTimeInMilliseconds() > crime.m_nTime + 10000)
				crime.m_nType = CRIME_NONE;
		}
	}
}

void
CWanted::Suspend(void)
{
	CStats::WantedStarsEvaded += m_nWantedLevel;
	m_nMinChaos = m_nChaos;
	m_nMinWantedLevel = m_nWantedLevel;
	m_nLastTimeSuspended = CTimer::GetTimeInMilliseconds();
	m_nChaos = 0;
	m_nWantedLevel = 0;
	ResetPolicePursuit();
}
