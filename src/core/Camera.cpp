#include "common.h"

#include "main.h"
#include "Draw.h"
#include "World.h"
#include "Vehicle.h"
#include "Train.h"
#include "Automobile.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "Pad.h"
#include "ControllerConfig.h"
#include "General.h"
#include "ZoneCull.h"
#include "SurfaceTable.h"
#include "Particle.h"
#include "WaterLevel.h"
#include "World.h"
#include "Garages.h"
#include "Replay.h"
#include "CutsceneMgr.h"
#include "Renderer.h"
#include "Timecycle.h"
#include "MBlur.h"
#include "Text.h"
#include "Hud.h"
#include "DMAudio.h"
#include "FileMgr.h"
#include "Frontend.h"
#include "SceneEdit.h"
#include "Pools.h"
#include "Debug.h"
#include "GenericGameStorage.h"
#include "Camera.h"

enum
{
	// car
	OBBE_WHEEL,
	OBBE_1,
	OBBE_2,
	OBBE_3,
	OBBE_1STPERSON,	// unused
	OBBE_5,
	OBBE_ONSTRING,
	OBBE_COPCAR,
	OBBE_COPCAR_WHEEL,
	// ped
	OBBE_9,
	OBBE_10,
	OBBE_11,
	OBBE_12,
	OBBE_13,
	// heli
	OBBE_14,
	OBBE_15,
	OBBE_16,
	OBBE_17,
	OBBE_18,
	OBBE_19,
	OBBE_ONSTRING_HELI,

	OBBE_INVALID
};

// abbreviate a few things
#define PLAYER (CWorld::Players[CWorld::PlayerInFocus].m_pPed)
// NB: removed explicit TheCamera from all functions

CCamera TheCamera;
#ifdef PC_PLAYER_CONTROLS
bool CCamera::m_bUseMouse3rdPerson = true;
#else
bool CCamera::m_bUseMouse3rdPerson = false;
#endif
bool bDidWeProcessAnyCinemaCam;
static bool bSwitchedToObbeCam;
float CCamera::m_fMouseAccelHorzntl;
float CCamera::m_fMouseAccelVertical;
float CCamera::m_f3rdPersonCHairMultX;
float CCamera::m_f3rdPersonCHairMultY;

#ifdef IMPROVED_CAMERA
#define KEYJUSTDOWN(k) ControlsManager.GetIsKeyboardKeyJustDown((RsKeyCodes)k)
#define KEYDOWN(k) ControlsManager.GetIsKeyboardKeyDown((RsKeyCodes)k)
#define CTRLJUSTDOWN(key) \
	       ((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYJUSTDOWN((RsKeyCodes)key) || \
	        (KEYJUSTDOWN(rsLCTRL) || KEYJUSTDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))
#define CTRLDOWN(key) ((KEYDOWN(rsLCTRL) || KEYDOWN(rsRCTRL)) && KEYDOWN((RsKeyCodes)key))
#endif

const float ZOOM_ONE_DISTANCE[] = { -0.6f, 0.05f, -3.2f, 0.05f, -2.41f };
const float ZOOM_TWO_DISTANCE[] = { 1.9f, 1.4f, 0.65f, 1.9f, 6.49f };
const float ZOOM_THREE_DISTANCE[] = { 15.9f, 15.9f, 15.9f, 15.9f, 25.25f };

#ifdef FREE_CAM
const float LCS_ZOOM_ONE_DISTANCE[] = { -1.0f, -0.2f, -3.2f, 0.05f, -2.41f };
const float LCS_ZOOM_TWO_DISTANCE[] = { 2.0f, 2.2f, 1.65f, 2.9f, 6.49f };
const float LCS_ZOOM_THREE_DISTANCE[] = { 6.0f, 6.0f, 15.9f, 15.9f, 15.0f };
#endif

CCamera::CCamera(void)
{
	Init();
}

void
CCamera::Init(void)
{
	memset(this, 0, sizeof(CCamera));	// this is fine, no vtable
	m_pRwCamera = nil;
	m_bPlayerWasOnBike = false;
	m_1rstPersonRunCloseToAWall = false;
	m_fPositionAlongSpline = 0.0f;
	m_bCameraJustRestored = false;
	Cams[0].Init();
	Cams[1].Init();
	Cams[2].Init();
	Cams[0].Mode = CCam::MODE_FOLLOWPED;
	Cams[1].Mode = CCam::MODE_FOLLOWPED;
	m_bEnable1rstPersonCamCntrlsScript = false;
	m_bAllow1rstPersonWeaponsCamera = false;
	m_bVehicleSuspenHigh = false;
	Cams[0].m_fMinRealGroundDist = 1.85f;
	// TODO: what weird value is this?
	Cams[0].m_fTargetCloseInDist = 2.0837801f - Cams[0].m_fMinRealGroundDist;
	Cams[0].m_fTargetZoomGroundOne = 0.25f;
	Cams[0].m_fTargetZoomGroundTwo = 1.5f;
	Cams[0].m_fTargetZoomGroundThree = 4.0f;
	Cams[0].m_fTargetZoomOneZExtra = -0.14f;
	Cams[0].m_fTargetZoomTwoZExtra = 0.16f;
	Cams[0].m_fTargetZoomThreeZExtra = 0.25f;
	// TODO: another weird value
	Cams[0].m_fTargetZoomZCloseIn = 0.90040702f;
	m_bMoveCamToAvoidGeom = false;
	ClearPlayerWeaponMode();
	m_bInATunnelAndABigVehicle = false;
	m_iModeObbeCamIsInForCar = OBBE_INVALID;
	Cams[0].CamTargetEntity = nil;
	Cams[1].CamTargetEntity = nil;
	Cams[2].CamTargetEntity = nil;
	Cams[0].m_fCamBufferedHeight = 0.0f;
	Cams[0].m_fCamBufferedHeightSpeed = 0.0f;
	Cams[1].m_fCamBufferedHeight = 0.0f;
	Cams[1].m_fCamBufferedHeightSpeed = 0.0f;
	Cams[0].m_bCamLookingAtVector = false;
	Cams[1].m_bCamLookingAtVector = false;
	Cams[2].m_bCamLookingAtVector = false;
	Cams[0].m_fPlayerVelocity = 0.0f;
	Cams[1].m_fPlayerVelocity = 0.0f;
	Cams[2].m_fPlayerVelocity = 0.0f;
	m_bHeadBob = false;
	m_fFractionInterToStopMoving = 0.25f;
	m_fFractionInterToStopCatchUp = 0.75f;
	m_fGaitSwayBuffer = 0.85f;
	m_bScriptParametersSetForInterPol = false;
	m_uiCamShakeStart = 0;
	m_fCamShakeForce = 0.0f;
	m_iModeObbeCamIsInForCar = OBBE_INVALID;
	m_bIgnoreFadingStuffForMusic = false;
	m_bWaitForInterpolToFinish = false;
	pToGarageWeAreIn = nil;
	pToGarageWeAreInForHackAvoidFirstPerson = nil;
	m_bPlayerIsInGarage = false;
	m_bJustCameOutOfGarage = false;
	m_fNearClipScript = DEFAULT_NEAR;
	m_bUseNearClipScript = false;
	m_vecDoingSpecialInterPolation = false;
	m_bAboveGroundTrainNodesLoaded = false;
	m_bBelowGroundTrainNodesLoaded = false;
	m_WideScreenOn = false;
	m_fFOV_Wide_Screen = 0.0f;
	m_bRestoreByJumpCut = false;
	CarZoomIndicator = CAM_ZOOM_2;
	PedZoomIndicator = CAM_ZOOM_2;
	CarZoomValueSmooth = 0.0f;
	m_fPedZoomValueSmooth = 0.0f;
	pTargetEntity = nil;
	if(FindPlayerVehicle())
		pTargetEntity = FindPlayerVehicle();
	else
		pTargetEntity = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
	m_bInitialNodeFound = false;
	m_ScreenReductionPercentage = 0.0f;
	m_ScreenReductionSpeed = 0.0f;
	m_WideScreenOn = false;
	m_bWantsToSwitchWidescreenOff = false;
	WorldViewerBeingUsed = false;
	PlayerExhaustion = 1.0f;
	DebugCamMode = CCam::MODE_NONE;
	m_PedOrientForBehindOrInFront = 0.0f;
	if(!FrontEndMenuManager.m_bWantToRestart){
		m_bFading = false;
		CDraw::FadeValue = 0;
		m_fFLOATingFade = 0.0f;
		m_bMusicFading = false;
		m_fTimeToFadeMusic = 0.0f;
		m_fFLOATingFadeMusic = 0.0f;
		m_fMouseAccelVertical = 0.003f;
		m_fMouseAccelHorzntl = 0.0025f;
	}
	if(FrontEndMenuManager.m_bWantToRestart)
		m_fTimeToFadeMusic = 0.0f;
	m_bStartingSpline = false;
	m_iTypeOfSwitch = INTERPOLATION;
	m_bUseScriptZoomValuePed = false;
	m_bUseScriptZoomValueCar = false;
	m_fPedZoomValueScript = 0.0f;
	m_fCarZoomValueScript = 0.0f;
	m_bUseSpecialFovTrain = false;
	m_fFovForTrain = 70.0f;	// or DefaultFOV from Cam.cpp
	m_iModeToGoTo = CCam::MODE_FOLLOWPED;
	m_bJust_Switched = false;
	m_bUseTransitionBeta = false;
	GetMatrix().SetScale(1.0f);
	m_bTargetJustBeenOnTrain = false;
	m_bInitialNoNodeStaticsSet = false;
	m_uiLongestTimeInMill = 5000;
	m_uiTimeLastChange = 0;
	m_uiTimeWeEnteredIdle = 0;
	m_bIdleOn = false;
	m_uiTimeWeLeftIdle_StillNoInput = 0;
	m_uiTimeWeEnteredIdle = 0;
	LODDistMultiplier = 1.0f;
	m_bCamDirectlyBehind = false;
	m_bCamDirectlyInFront = false;
	m_motionBlur = 0;
	m_bGarageFixedCamPositionSet = false;
	SetMotionBlur(255, 255, 255, 0, 0);
	m_bCullZoneChecksOn = false;
	m_bFailedCullZoneTestPreviously = false;
	m_iCheckCullZoneThisNumFrames = 6;
	m_iZoneCullFrameNumWereAt = 0;
	m_CameraAverageSpeed = 0.0f;
	m_CameraSpeedSoFar = 0.0f;
	m_PreviousCameraPosition = CVector(0.0f, 0.0f, 0.0f);
	m_iWorkOutSpeedThisNumFrames = 4;
	m_iNumFramesSoFar = 0;
	m_bJustInitalised = true;
	m_uiTransitionState = 0;
	m_uiTimeTransitionStart = 0;
	m_bLookingAtPlayer = true;
	m_f3rdPersonCHairMultX = 0.53f;
	m_f3rdPersonCHairMultY = 0.4f;
	m_fAvoidTheGeometryProbsTimer = 0.0f;
	m_nAvoidTheGeometryProbsDirn = 0;
}

void
CCamera::Process(void)
{
	// static bool InterpolatorNotInitialised = true;	// unused
	static float PlayerMinDist = 1.3f;
	static bool WasPreviouslyInterSyhonFollowPed = false;	// only written
	float FOV = 0.0f;
	float oldBeta, newBeta;
	float deltaBeta = 0.0f;
	bool lookLRBVehicle = false;
	CVector CamFront, CamUp, CamRight, CamSource, Target;

	m_bJust_Switched = false;
	m_RealPreviousCameraPosition = GetPosition();

	// Update target entity
	if(m_bLookingAtPlayer || m_bTargetJustBeenOnTrain || WhoIsInControlOfTheCamera == CAMCONTROL_OBBE)
		UpdateTargetEntity();
	if(pTargetEntity == nil)
		pTargetEntity = FindPlayerPed();
	if(Cams[ActiveCam].CamTargetEntity == nil)
		Cams[ActiveCam].CamTargetEntity = pTargetEntity;
	if(Cams[(ActiveCam+1)%2].CamTargetEntity == nil)
		Cams[(ActiveCam+1)%2].CamTargetEntity = pTargetEntity;

	CamControl();
	if(m_bFading)
		ProcessFade();
	if(m_bMusicFading)
		ProcessMusicFade();
	if(m_WideScreenOn)
		ProcessWideScreenOn();

#ifndef MASTER
#ifdef IMPROVED_CAMERA
	if(CPad::GetPad(1)->GetCircleJustDown() || CTRLJUSTDOWN('B')){
#else
	if(CPad::GetPad(1)->GetCircleJustDown()){
#endif
		WorldViewerBeingUsed = !WorldViewerBeingUsed;
		if(WorldViewerBeingUsed)
			InitialiseCameraForDebugMode();
		else
			CPad::m_bMapPadOneToPadTwo = false;
	}
#endif

	RwCameraSetNearClipPlane(Scene.camera, DEFAULT_NEAR);

	if(Cams[ActiveCam].Front.x == 0.0f && Cams[ActiveCam].Front.y == 0.0f)
		oldBeta = 0.0f;
	else
		oldBeta = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y);

	Cams[ActiveCam].Process();
	Cams[ActiveCam].ProcessSpecialHeightRoutines();

	if(Cams[ActiveCam].Front.x == 0.0f && Cams[ActiveCam].Front.y == 0.0f)
		newBeta = 0.0f;
	else
		newBeta = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y);


	// Stop transition when it's done
	if(m_uiTransitionState != 0){
		if(CTimer::GetTimeInMilliseconds() > m_uiTransitionDuration+m_uiTimeTransitionStart){
			m_uiTransitionState = 0;
			m_vecDoingSpecialInterPolation = false;
			m_bWaitForInterpolToFinish = false;
		}
	}

	if(m_bUseNearClipScript)
		RwCameraSetNearClipPlane(Scene.camera, m_fNearClipScript);

	deltaBeta = newBeta - oldBeta;
	while(deltaBeta >= PI) deltaBeta -= 2*PI;
	while(deltaBeta < -PI) deltaBeta += 2*PI;
	if(Abs(deltaBeta) > 0.3f)
		m_bJust_Switched = true;

#ifndef MASTER
	// Debug stuff
	if(!gbModelViewer)
		Cams[ActiveCam].PrintMode();	// actually missing in VC
	if(WorldViewerBeingUsed)
		Cams[2].Process();
#endif

	if(Cams[ActiveCam].DirectionWasLooking != LOOKING_FORWARD && pTargetEntity->IsVehicle())
		lookLRBVehicle = true;

	if(m_uiTransitionState != 0 && !lookLRBVehicle){
		// Process transition

		uint32 currentTime = CTimer::GetTimeInMilliseconds() - m_uiTimeTransitionStart;
		if(currentTime >= m_uiTransitionDuration)
			currentTime = m_uiTransitionDuration;
		float fractionInter = (float) currentTime / m_uiTransitionDuration;
		float fractionInterTarget = (float) currentTime / m_uiTransitionDurationTargetCoors;
		fractionInterTarget = Clamp(fractionInterTarget, 0.0f, 1.0f);

		// Interpolate target separately
		if(fractionInterTarget <= m_fFractionInterToStopMovingTarget){
			float inter;
			if(m_fFractionInterToStopMovingTarget == 0.0f)
				inter = 0.0f;
			else
				inter = (m_fFractionInterToStopMovingTarget - fractionInterTarget)/m_fFractionInterToStopMovingTarget;
			inter = 0.5f - 0.5*Cos(inter*PI);	// smooth it

			m_vecTargetWhenInterPol = m_cvecStartingTargetForInterPol + inter*m_cvecTargetSpeedAtStartInter;
			Target = m_vecTargetWhenInterPol;
		}else if(fractionInterTarget > m_fFractionInterToStopMovingTarget){
			float inter;
			if(m_fFractionInterToStopCatchUpTarget == 0.0f)
				inter = 0.0f;
			else
				inter = (fractionInterTarget - m_fFractionInterToStopMovingTarget)/m_fFractionInterToStopCatchUpTarget;
			inter = 0.5f - 0.5*Cos(inter*PI);	// smooth it

			if(m_fFractionInterToStopMovingTarget == 0.0f)
				m_vecTargetWhenInterPol = m_cvecStartingTargetForInterPol;
			Target = m_vecTargetWhenInterPol + inter*(Cams[ActiveCam].m_cvecTargetCoorsForFudgeInter - m_vecTargetWhenInterPol);
		}

		if(fractionInter <= m_fFractionInterToStopMoving){
			float inter;
			if(m_fFractionInterToStopMoving == 0.0f)
				inter = 0.0f;
			else
				inter = (m_fFractionInterToStopMoving - fractionInter)/m_fFractionInterToStopMoving;
			inter = 0.5f - 0.5*Cos(inter*PI);	// smooth it

			m_vecSourceWhenInterPol = m_cvecStartingSourceForInterPol + inter*m_cvecSourceSpeedAtStartInter;

			if(m_bLookingAtPlayer){
				CVector ToCam = m_vecSourceWhenInterPol - Target;
				if(ToCam.Magnitude2D() < PlayerMinDist){
					float beta = CGeneral::GetATanOfXY(ToCam.x, ToCam.y);
					CamSource.x = Target.x + PlayerMinDist*Cos(beta);
					CamSource.y = Target.y + PlayerMinDist*Sin(beta);
				}
			}

			m_vecUpWhenInterPol = m_cvecStartingUpForInterPol + inter*m_cvecUpSpeedAtStartInter;
			m_fFOVWhenInterPol = m_fStartingFOVForInterPol + inter*m_fFOVSpeedAtStartInter;

			CamSource = m_vecSourceWhenInterPol;
			CamFront = Target - CamSource;
			StoreValuesDuringInterPol(CamSource, m_vecTargetWhenInterPol, m_vecUpWhenInterPol, m_fFOVWhenInterPol);
			CamFront.Normalise();
			if(m_bLookingAtPlayer)
				CamUp = CVector(0.0f, 0.0f, 1.0f);
			else
				CamUp = m_vecUpWhenInterPol;
			CamUp.Normalise();

			if(Cams[ActiveCam].Mode == CCam::MODE_TOPDOWN || Cams[ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED){
				CamFront.Normalise();
				CamRight = CVector(-1.0f, 0.0f, 0.0f);
				CamUp = CrossProduct(CamFront, CamRight);
				CamUp.Normalise();
			}else{
				CamFront.Normalise();
				CamUp.Normalise();
				CamRight = CrossProduct(CamFront, CamUp);
				CamRight.Normalise();
				CamUp = CrossProduct(CamRight, CamFront);
				CamUp.Normalise();
			}
			FOV = m_fFOVWhenInterPol;
		}else if(fractionInter > m_fFractionInterToStopMoving && fractionInter <= 1.0f){
			float inter;
			if(m_fFractionInterToStopCatchUp == 0.0f)
				inter = 0.0f;
			else
				inter = (fractionInter - m_fFractionInterToStopMoving)/m_fFractionInterToStopCatchUp;
			inter = 0.5f - 0.5*Cos(inter*PI);	// smooth it

			CamSource = m_vecSourceWhenInterPol + inter*(Cams[ActiveCam].Source - m_vecSourceWhenInterPol);

			if(m_bLookingAtPlayer){
				CVector ToCam = m_vecSourceWhenInterPol - Target;
				if(ToCam.Magnitude2D() < PlayerMinDist){
					float beta = CGeneral::GetATanOfXY(ToCam.x, ToCam.y);
					CamSource.x = Target.x + PlayerMinDist*Cos(beta);
					CamSource.y = Target.y + PlayerMinDist*Sin(beta);
				}
			}

			FOV = m_fFOVWhenInterPol + inter*(Cams[ActiveCam].FOV - m_fFOVWhenInterPol);
			CamUp = m_vecUpWhenInterPol + inter*(Cams[ActiveCam].Up - m_vecUpWhenInterPol);
			deltaBeta = Cams[ActiveCam].m_fTrueBeta - m_fBetaWhenInterPol;
			MakeAngleLessThan180(deltaBeta);

			CamFront = Target - CamSource;
			StoreValuesDuringInterPol(CamSource, Target, CamUp, FOV);
			CamFront.Normalise();
			if(m_bLookingAtPlayer)
				CamUp = CVector(0.0f, 0.0f, 1.0f);

			if(Cams[ActiveCam].Mode == CCam::MODE_TOPDOWN || Cams[ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED){
				CamFront.Normalise();
				CamRight = CVector(-1.0f, 0.0f, 0.0f);
				CamUp = CrossProduct(CamFront, CamRight);
				CamUp.Normalise();
			}else{
				CamFront.Normalise();
				CamUp.Normalise();
				CamRight = CrossProduct(CamFront, CamUp);
				CamRight.Normalise();
				CamUp = CrossProduct(CamRight, CamFront);
				CamUp.Normalise();
			}
#ifndef FIX_BUGS
			// BUG: FOV was already interpolated but m_fFOVWhenInterPol was not
			FOV = m_fFOVWhenInterPol;
#endif
		}

		CVector Dist = CamSource - Target;
		float DistOnGround = Dist.Magnitude2D();
		float Alpha = CGeneral::GetATanOfXY(DistOnGround, Dist.z);
		float Beta = CGeneral::GetATanOfXY(Dist.x, Dist.y);
		Cams[ActiveCam].KeepTrackOfTheSpeed(CamSource, Target, CamUp, Alpha, Beta, FOV);
	}else{
		// No transition, take Cam values directly
#ifndef MASTER
		if(WorldViewerBeingUsed){
			CamSource = Cams[2].Source;
			CamFront = Cams[2].Front;
			CamUp = Cams[2].Up;
			FOV = Cams[2].FOV;
		}else
#endif
		{
			CamSource = Cams[ActiveCam].Source;
			CamUp = Cams[ActiveCam].Up;
			if(m_bMoveCamToAvoidGeom){
				CamSource += m_vecClearGeometryVec;
				CamFront = Cams[ActiveCam].m_cvecTargetCoorsForFudgeInter - CamSource;
				CamFront.Normalise();
				CVector Right = CrossProduct(CamFront, CamUp);
				Right.Normalise();
				CamUp = CrossProduct(Right, CamFront);
				CamUp.Normalise();
			}else{
				CamFront = Cams[ActiveCam].Front;
				CamUp = Cams[ActiveCam].Up;
			}
			FOV = Cams[ActiveCam].FOV;
		}
		WasPreviouslyInterSyhonFollowPed = false;	// unused
	}

	if(m_uiTransitionState != 0)
		if(!m_bLookingAtVector && m_bLookingAtPlayer && !CCullZones::CamStairsForPlayer() && !m_bPlayerIsInGarage){
			CEntity *entity = nil;
			CColPoint colPoint;
			if(CWorld::ProcessLineOfSight(pTargetEntity->GetPosition(), CamSource, colPoint, entity, true, false, false, true, false, true, true)){
				CamSource = colPoint.point;
				RwCameraSetNearClipPlane(Scene.camera, 0.05f);
			}
		}

	if(CMBlur::Drunkness > 0.0f){
		static float DrunkAngle;

		int tableIndex = (int)(DEGTORAD(DrunkAngle)/TWOPI * CParticle::SIN_COS_TABLE_SIZE) & CParticle::SIN_COS_TABLE_SIZE-1;
		DrunkAngle += 5.0f;
#ifndef FIX_BUGS
		// This just messes up interpolation, probably not what they intended
		// and multiplying the interpolated FOV is also a bit extreme
		// so let's not do any of this nonsense
		Cams[ActiveCam].FOV *= (1.0f + CMBlur::Drunkness);
#endif

		CamSource.x += -0.02f*CMBlur::Drunkness * CParticle::m_CosTable[tableIndex];
		CamSource.y += -0.02f*CMBlur::Drunkness * CParticle::m_SinTable[tableIndex];

		CamUp.Normalise();
		CamUp.x += 0.05f*CMBlur::Drunkness * CParticle::m_CosTable[tableIndex];
		CamUp.y += 0.05f*CMBlur::Drunkness * CParticle::m_SinTable[tableIndex];
		CamUp.Normalise();

		CamFront.Normalise();
		CamFront.x += -0.1f*CMBlur::Drunkness * CParticle::m_CosTable[tableIndex];
		CamFront.y += -0.1f*CMBlur::Drunkness * CParticle::m_SinTable[tableIndex];
		CamFront.Normalise();

		CamRight = CrossProduct(CamFront, CamUp);
		CamRight.Normalise();
		CamUp = CrossProduct(CamRight, CamFront);
		CamUp.Normalise();
	}

	GetMatrix().GetRight() = CrossProduct(CamUp, CamFront);	// actually Left
	GetMatrix().GetForward() = CamFront;
	GetMatrix().GetUp() = CamUp;
	GetMatrix().GetPosition() = CamSource;

	// Process Shake
	float shakeStrength = m_fCamShakeForce - 0.28f*(CTimer::GetTimeInMilliseconds()-m_uiCamShakeStart)/1000.0f;
	shakeStrength = Clamp(shakeStrength, 0.0f, 2.0f);
	int shakeRand = CGeneral::GetRandomNumber();
	float shakeOffset = shakeStrength*0.1f;
	GetMatrix().GetPosition().x += shakeOffset * ((shakeRand & 0xF) - 7);
	GetMatrix().GetPosition().y += shakeOffset * (((shakeRand & 0xF0) >> 4) - 7);
	GetMatrix().GetPosition().z += shakeOffset * (((shakeRand & 0xF00) >> 8) - 7);

	if(shakeOffset > 0.0f && m_BlurType != MOTION_BLUR_SNIPER)
		SetMotionBlurAlpha(Min((int)(shakeStrength*255.0f) + 25, 150));

	static bool bExtra1stPrsBlur = false;
	if(Cams[ActiveCam].Mode == CCam::MODE_1STPERSON && FindPlayerVehicle() && FindPlayerVehicle()->GetUp().z < 0.2f){
		SetMotionBlur(230, 230, 230, 215, MOTION_BLUR_LIGHT_SCENE);
		bExtra1stPrsBlur = true;
	}else if(bExtra1stPrsBlur){
		SetMotionBlur(CTimeCycle::GetBlurRed(), CTimeCycle::GetBlurGreen(), CTimeCycle::GetBlurBlue(), m_motionBlur, MOTION_BLUR_LIGHT_SCENE);
		bExtra1stPrsBlur = false;
	}

	CalculateDerivedValues();
	CDraw::SetFOV(FOV);

	// Set RW camera
#ifndef MASTER
	if(WorldViewerBeingUsed){
		RwFrame *frame = RwCameraGetFrame(m_pRwCamera);
		CVector Source = Cams[2].Source;
		CVector Front = Cams[2].Front;
		CVector Up = Cams[2].Up;

		GetMatrix().GetRight() = CrossProduct(Up, Front);
		GetMatrix().GetForward() = Front;
		GetMatrix().GetUp() = Up;
		GetMatrix().GetPosition() = Source;

		CDraw::SetFOV(Cams[2].FOV);
		m_vecGameCamPos = Cams[ActiveCam].Source;

		*RwMatrixGetPos(RwFrameGetMatrix(frame)) = GetPosition();
		*RwMatrixGetAt(RwFrameGetMatrix(frame)) = GetForward();
		*RwMatrixGetUp(RwFrameGetMatrix(frame)) = GetUp();
		*RwMatrixGetRight(RwFrameGetMatrix(frame)) = GetRight();
		RwMatrixUpdate(RwFrameGetMatrix(frame));
		RwFrameUpdateObjects(frame);
	}else
#endif
	{
		RwFrame *frame = RwCameraGetFrame(m_pRwCamera);
		m_vecGameCamPos = GetPosition();
		*RwMatrixGetPos(RwFrameGetMatrix(frame)) = GetPosition();
		*RwMatrixGetAt(RwFrameGetMatrix(frame)) = GetForward();
		*RwMatrixGetUp(RwFrameGetMatrix(frame)) = GetUp();
		*RwMatrixGetRight(RwFrameGetMatrix(frame)) = GetRight();
		RwMatrixUpdate(RwFrameGetMatrix(frame));
		RwFrameUpdateObjects(frame);
		RwFrameOrthoNormalize(frame);
	}

	UpdateSoundDistances();

	if((CTimer::GetFrameCounter()&0xF) == 3)
		DistanceToWater = CWaterLevel::CalcDistanceToWater(GetPosition().x, GetPosition().y);

	// LOD dist
	if(!CCutsceneMgr::IsRunning() || CCutsceneMgr::UseLodMultiplier()){
		LODDistMultiplier = 70.0f/CDraw::GetFOV();

		if(GetPosition().z > 55.0f && FindPlayerVehicle() && FindPlayerVehicle()->pHandling->Flags & (HANDLING_IS_HELI|HANDLING_IS_PLANE) ||
		   FindPlayerPed()->m_attachedTo){
			LODDistMultiplier *= 1.0f + Max((GetPosition().z - 55.0f)/60.0f, 0.0f);
			float NewNear = DEFAULT_NEAR * (1.0f + Max((GetPosition().z - 55.0f)/60.0f, 0.0f));
			if(RwCameraGetNearClipPlane(Scene.camera) >= DEFAULT_NEAR)
				RwCameraSetNearClipPlane(Scene.camera, NewNear);
		}
		if(LODDistMultiplier > 2.2f) LODDistMultiplier = 2.2f;
	}else
		LODDistMultiplier = 1.0f;
	GenerationDistMultiplier = LODDistMultiplier;
	LODDistMultiplier *= CRenderer::ms_lodDistScale;

	CDraw::SetNearClipZ(RwCameraGetNearClipPlane(m_pRwCamera));
	CDraw::SetFarClipZ(RwCameraGetFarClipPlane(m_pRwCamera));

	// Keep track of speed
	if(m_bJustInitalised || m_bJust_Switched){
		m_PreviousCameraPosition = GetPosition();
		m_bJustInitalised = false;
	}
	m_CameraSpeedSoFar += (GetPosition() - m_PreviousCameraPosition).Magnitude();
	m_iNumFramesSoFar++;
	if(m_iNumFramesSoFar == m_iWorkOutSpeedThisNumFrames){
		m_CameraAverageSpeed = m_CameraSpeedSoFar / m_iWorkOutSpeedThisNumFrames;
		m_CameraSpeedSoFar = 0.0f;
		m_iNumFramesSoFar = 0;
	}
	m_PreviousCameraPosition = GetPosition();

	if(Cams[ActiveCam].DirectionWasLooking != LOOKING_FORWARD && Cams[ActiveCam].Mode != CCam::MODE_TOP_DOWN_PED){
		Cams[ActiveCam].Source = Cams[ActiveCam].SourceBeforeLookBehind;
		Orientation += PI;
	}

	if(m_uiTransitionState != 0){
		int OtherCam = (ActiveCam+1)%2;
		if(Cams[OtherCam].CamTargetEntity &&
		   pTargetEntity && pTargetEntity->IsPed() &&
		   !Cams[OtherCam].CamTargetEntity->IsVehicle() &&
		   Cams[ActiveCam].Mode != CCam::MODE_TOP_DOWN_PED && Cams[ActiveCam].DirectionWasLooking != LOOKING_FORWARD){
			Cams[OtherCam].Source = Cams[ActiveCam%2].SourceBeforeLookBehind;
			Orientation += PI;
		}
	}

	m_bCameraJustRestored = false;
	m_bMoveCamToAvoidGeom = false;
}

void
CCamera::CamControl(void)
{
	static bool PlaceForFixedWhenSniperFound = false;
	static int16 ReqMode;
	bool switchByJumpCut = false;
	bool stairs = false;
	bool boatTarget = false;
	int PrevMode = Cams[ActiveCam].Mode;
	CVector targetPos;
	CVector garageCenter, garageDoorPos1, garageDoorPos2;
	CVector garageCenterToDoor, garageCamPos;
	int whichDoor;

	m_bObbeCinematicPedCamOn = false;
	m_bObbeCinematicCarCamOn = false;
	m_bUseTransitionBeta = false;
	m_bUseSpecialFovTrain = false;
	m_bJustCameOutOfGarage = false;
	m_bTargetJustCameOffTrain = false;
	m_bInATunnelAndABigVehicle = false;
	m_bJustJumpedOutOf1stPersonBecauseOfTarget = false;
	bSwitchedToObbeCam = false;

	if(Cams[ActiveCam].CamTargetEntity == nil && pTargetEntity == nil)
		pTargetEntity = PLAYER;

	m_iZoneCullFrameNumWereAt++;
	if(m_iZoneCullFrameNumWereAt > m_iCheckCullZoneThisNumFrames)
		m_iZoneCullFrameNumWereAt = 1;
	m_bCullZoneChecksOn = m_iZoneCullFrameNumWereAt == m_iCheckCullZoneThisNumFrames;
	if(m_bCullZoneChecksOn)
		m_bFailedCullZoneTestPreviously = CCullZones::CamCloseInForPlayer();

	if(m_bLookingAtPlayer){
		CPad::GetPad(0)->DisablePlayerControls &= ~PLAYERCONTROL_CAMERA;
		FindPlayerPed()->bIsVisible = true;
	}

	if(!CTimer::GetIsPaused() && !m_bIdleOn){
		float CloseInCarHeightTarget = 0.0f;
		float CloseInPedHeightTarget = 0.0f;

		if(m_bTargetJustBeenOnTrain){
			// Getting off train
			if(!pTargetEntity->IsVehicle() || !((CVehicle*)pTargetEntity)->IsTrain()){
				Restore();
				m_bTargetJustCameOffTrain = true;
				m_bTargetJustBeenOnTrain = false;
				SetWideScreenOff();
			}
		}

		// Vehicle target
		if(pTargetEntity->IsVehicle()){
#ifdef GTA_TRAIN
			if(((CVehicle*)pTargetEntity)->IsTrain()){
				if(!m_bTargetJustBeenOnTrain){
					m_bInitialNodeFound = false;
					m_bInitialNoNodeStaticsSet = false;
				}
				Process_Train_Camera_Control();
			}else
#endif
			{
				if(((CVehicle*)pTargetEntity)->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
					boatTarget = true;

				// Change user selected mode
				if(CPad::GetPad(0)->CycleCameraModeUpJustDown() && !CReplay::IsPlayingBack() &&
				   (m_bLookingAtPlayer || WhoIsInControlOfTheCamera == CAMCONTROL_OBBE) &&
				   !m_WideScreenOn){
					CarZoomIndicator--;
					// disable topdown here
					if(CarZoomIndicator == CAM_ZOOM_TOPDOWN)
						CarZoomIndicator--;
				}
				if(CPad::GetPad(0)->CycleCameraModeDownJustDown() && !CReplay::IsPlayingBack() &&
				   (m_bLookingAtPlayer || WhoIsInControlOfTheCamera == CAMCONTROL_OBBE) &&
				   !m_WideScreenOn){
					CarZoomIndicator++;
					if(CarZoomIndicator == CAM_ZOOM_TOPDOWN)
						CarZoomIndicator++;
				}
				if(!m_bFailedCullZoneTestPreviously){
					if(CarZoomIndicator < CAM_ZOOM_1STPRS) CarZoomIndicator = CAM_ZOOM_CINEMATIC;
					else if(CarZoomIndicator > CAM_ZOOM_CINEMATIC) CarZoomIndicator = CAM_ZOOM_1STPRS;
				}

				if(m_bFailedCullZoneTestPreviously)
					if(CarZoomIndicator != CAM_ZOOM_1STPRS && CarZoomIndicator != CAM_ZOOM_TOPDOWN)
						ReqMode = CCam::MODE_CAM_ON_A_STRING;

				int vehType = ((CVehicle*)pTargetEntity)->m_vehType;
				if(((CVehicle*)pTargetEntity)->IsBoat() && pTargetEntity->GetModelIndex() == MI_SKIMMER)
					vehType = VEHICLE_TYPE_CAR;

				switch(vehType){
				case VEHICLE_TYPE_CAR:
				case VEHICLE_TYPE_BIKE:{
					CAttributeZone *stairsZone = nil;
					if(vehType == VEHICLE_TYPE_BIKE && CCullZones::CamStairsForPlayer()){
						stairsZone = CCullZones::FindZoneWithStairsAttributeForPlayer();
						if(stairsZone)
							stairs = true;
					}
					if(CGarages::IsPointInAGarageCameraZone(pTargetEntity->GetPosition()) || stairs){
						if(!m_bGarageFixedCamPositionSet && m_bLookingAtPlayer ||
						   WhoIsInControlOfTheCamera == CAMCONTROL_OBBE){
							if(pToGarageWeAreIn || stairsZone){
								float ground;
								bool foundGround;

								if(pToGarageWeAreIn){
									// This is all very strange....
									// targetPos = pTargetEntity->GetPosition();	// unused
									if(pToGarageWeAreIn->m_pDoor1){
										whichDoor = 1;
										garageDoorPos1.x = pToGarageWeAreIn->m_fDoor1X;
										garageDoorPos1.y = pToGarageWeAreIn->m_fDoor1Y;
										garageDoorPos1.z = 0.0f;
										// targetPos.z = 0.0f;	// unused
										// (targetPos - doorPos1).Magnitude();	// unused
									}else if(pToGarageWeAreIn->m_pDoor2){
										whichDoor = 2;
#ifdef FIX_BUGS
										garageDoorPos2.x = pToGarageWeAreIn->m_fDoor2X;
										garageDoorPos2.y = pToGarageWeAreIn->m_fDoor2Y;
										garageDoorPos2.z = 0.0f;
#endif
									}else{
										whichDoor = 1;
										garageDoorPos1.x = pTargetEntity->GetPosition().x;
										garageDoorPos1.y = pTargetEntity->GetPosition().y;
#ifdef FIX_BUGS
										garageDoorPos1.z = 0.0f;
#else
										garageDoorPos2.z = 0.0f;
#endif
									}
								}else{
									assert(stairsZone);
									whichDoor = 1;
									garageDoorPos1 = Cams[ActiveCam].Source;
									garageCenter = CVector((stairsZone->minx+stairsZone->maxx)/2.0f, (stairsZone->miny+stairsZone->maxy)/2.0f, 0.0f);
									if((garageCenter-garageDoorPos1).Magnitude() > 15.0f){
										bool bClearViewOutside = true;
										CVector dirOutside = pTargetEntity->GetPosition() - garageCenter;
										dirOutside.z = 0.0f;
										dirOutside.Normalise();
										float zoneDim = stairsZone->maxx - stairsZone->minx;
										if(zoneDim < stairsZone->maxy - stairsZone->miny)
											zoneDim = stairsZone->maxy - stairsZone->miny;
										zoneDim *= 2.0f;
										CVector posOutside = pTargetEntity->GetPosition() + zoneDim*dirOutside;
										if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), posOutside, true, false, false, false, false, false, true)){
											posOutside = pTargetEntity->GetPosition() - zoneDim*dirOutside;
											if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), posOutside, true, false, false, false, false, false, true))
												bClearViewOutside = false;
										}
										if(bClearViewOutside)
											garageDoorPos1 = posOutside;
									}
								}

								if(pToGarageWeAreIn){
									garageCenter.x = pToGarageWeAreIn->GetGarageCenterX();
									garageCenter.y = pToGarageWeAreIn->GetGarageCenterY();
									garageCenter.z = 0.0f;
								}else{
									garageDoorPos1.z = 0.0f;
									if(stairsZone == nil)	// how can this be true?
										garageCenter = CVector(pTargetEntity->GetPosition().x, pTargetEntity->GetPosition().y, 0.0f);
								}

								if(whichDoor == 1)
									garageCenterToDoor = garageDoorPos1 - garageCenter;
								else
									garageCenterToDoor = garageDoorPos2 - garageCenter;
								targetPos = pTargetEntity->GetPosition();
								ground = CWorld::FindGroundZFor3DCoord(targetPos.x, targetPos.y, targetPos.z, &foundGround);
								if(!foundGround)
									ground = targetPos.z - 0.2f;
								garageCenterToDoor.z = 0.0f;
								garageCenterToDoor.Normalise();
								if(whichDoor == 1){
									if(pToGarageWeAreIn == nil && stairsZone){
										float zoneDim = stairsZone->maxx - stairsZone->minx;
										if(zoneDim < stairsZone->maxy - stairsZone->miny)
											zoneDim = stairsZone->maxy - stairsZone->miny;
										garageCamPos = garageCenter + (0.7f*zoneDim + 3.75f)*garageCenterToDoor;
									}else
										garageCamPos = garageDoorPos1 + 13.0f*garageCenterToDoor;
								}else
									garageCamPos = garageDoorPos2 + 13.0f*garageCenterToDoor;
								garageCamPos.z = ground + 3.1f;
								SetCamPositionForFixedMode(garageCamPos, CVector(0.0f, 0.0f, 0.0f));
								m_bGarageFixedCamPositionSet = true;
							}
						}

						if(CGarages::CameraShouldBeOutside() && m_bGarageFixedCamPositionSet &&
						   (m_bLookingAtPlayer || WhoIsInControlOfTheCamera == CAMCONTROL_OBBE)){
							if(pToGarageWeAreIn){
								ReqMode = CCam::MODE_FIXED;
								m_bPlayerIsInGarage = true;
							}
						}else{
							if(m_bPlayerIsInGarage){
								m_bJustCameOutOfGarage = true;
								m_bPlayerIsInGarage = false;
							}
							ReqMode = CCam::MODE_CAM_ON_A_STRING;
						}
					}else{
						if(m_bPlayerIsInGarage){
							m_bJustCameOutOfGarage = true;
							m_bPlayerIsInGarage = false;
						}
						m_bGarageFixedCamPositionSet = false;
						ReqMode = CCam::MODE_CAM_ON_A_STRING;
					}
					break;
				}
				case VEHICLE_TYPE_BOAT:
					ReqMode = CCam::MODE_BEHINDBOAT;
					break;
				default: break;
				}

				int vehApp = ((CVehicle*)pTargetEntity)->GetVehicleAppearance();
				int vehArrPos = 0;
				GetArrPosForVehicleType(vehApp, vehArrPos);

				// Car zoom value
				if (CarZoomIndicator == CAM_ZOOM_1STPRS && !m_bPlayerIsInGarage) {
					CarZoomValue = 0.0f;
					ReqMode = CCam::MODE_1STPERSON;
				}
#ifdef FREE_CAM
				else if (bFreeCam) {
					if (CarZoomIndicator == CAM_ZOOM_1)
						CarZoomValue = LCS_ZOOM_ONE_DISTANCE[vehArrPos];
					else if (CarZoomIndicator == CAM_ZOOM_2)
						CarZoomValue = LCS_ZOOM_TWO_DISTANCE[vehArrPos];
					else if (CarZoomIndicator == CAM_ZOOM_3)
						CarZoomValue = LCS_ZOOM_THREE_DISTANCE[vehArrPos];
				}
#endif
				else if (CarZoomIndicator == CAM_ZOOM_1)
					CarZoomValue = ZOOM_ONE_DISTANCE[vehArrPos];
				else if(CarZoomIndicator == CAM_ZOOM_2)
					CarZoomValue = ZOOM_TWO_DISTANCE[vehArrPos];
				else if(CarZoomIndicator == CAM_ZOOM_3)
					CarZoomValue = ZOOM_THREE_DISTANCE[vehArrPos];

				if(CarZoomIndicator == CAM_ZOOM_TOPDOWN && !m_bPlayerIsInGarage){
					CarZoomValue = 1.0f;
					ReqMode = CCam::MODE_TOPDOWN;
				}

				// Check if we have to go into first person
				if(vehType == VEHICLE_TYPE_CAR && !m_bPlayerIsInGarage){
					if(CCullZones::Cam1stPersonForPlayer() && 
					   pTargetEntity->GetColModel()->boundingBox.GetSize().z >= 3.026f &&
					   pToGarageWeAreInForHackAvoidFirstPerson == nil){
						ReqMode = CCam::MODE_1STPERSON;
						m_bInATunnelAndABigVehicle = true;
					}
				}
				if(ReqMode == CCam::MODE_TOPDOWN &&
				   (CCullZones::Cam1stPersonForPlayer() || CCullZones::CamNoRain() || CCullZones::PlayerNoRain()))
					ReqMode = CCam::MODE_1STPERSON;

				// Smooth zoom value - ugly code
				if(m_bUseScriptZoomValueCar){
					if(CarZoomValueSmooth < m_fCarZoomValueScript){
						CarZoomValueSmooth += 0.12f * CTimer::GetTimeStep();
						CarZoomValueSmooth = Min(CarZoomValueSmooth, m_fCarZoomValueScript);
					}else{
						CarZoomValueSmooth -= 0.12f * CTimer::GetTimeStep();
						CarZoomValueSmooth = Max(CarZoomValueSmooth, m_fCarZoomValueScript);
					}
				}else if(m_bFailedCullZoneTestPreviously){
					CloseInCarHeightTarget = 0.65f;
					if(CarZoomValueSmooth < -0.65f){
						CarZoomValueSmooth += 0.12f * CTimer::GetTimeStep();
						CarZoomValueSmooth = Min(CarZoomValueSmooth, -0.65f);
					}else{
						CarZoomValueSmooth -= 0.12f * CTimer::GetTimeStep();
						CarZoomValueSmooth = Max(CarZoomValueSmooth, -0.65f);
					}
				}else{
					if(CarZoomValueSmooth < CarZoomValue){
						CarZoomValueSmooth += 0.12f * CTimer::GetTimeStep();
						CarZoomValueSmooth = Min(CarZoomValueSmooth, CarZoomValue);
					}else{
						CarZoomValueSmooth -= 0.12f * CTimer::GetTimeStep();
						CarZoomValueSmooth = Max(CarZoomValueSmooth, CarZoomValue);
					}
				}

				WellBufferMe(CloseInCarHeightTarget, &Cams[ActiveCam].m_fCloseInCarHeightOffset, &Cams[ActiveCam].m_fCloseInCarHeightOffsetSpeed, 0.1f, 0.25f, false);

				// Fallen into water
				if(Cams[ActiveCam].IsTargetInWater(Cams[ActiveCam].Source) && !boatTarget &&
				   !Cams[ActiveCam].CamTargetEntity->IsPed() &&
				   pTargetEntity->GetModelIndex() != MI_SKIMMER && pTargetEntity->GetModelIndex() != MI_SEASPAR)
					ReqMode = CCam::MODE_PLAYER_FALLEN_WATER;
			}
		}

		// Ped target
		else if(pTargetEntity->IsPed()){
			// Change user selected mode
			if(CPad::GetPad(0)->CycleCameraModeUpJustDown() && !CReplay::IsPlayingBack() &&
			   (m_bLookingAtPlayer || WhoIsInControlOfTheCamera == CAMCONTROL_OBBE) &&
			   !m_WideScreenOn && !m_bFailedCullZoneTestPreviously && !m_bFirstPersonBeingUsed){
				if(FrontEndMenuManager.m_ControlMethod == CONTROL_STANDARD){
					if(PedZoomIndicator == CAM_ZOOM_3)
						PedZoomIndicator = CAM_ZOOM_1;
					else
						PedZoomIndicator = CAM_ZOOM_3;
				}else
					PedZoomIndicator--;
			}
			if(CPad::GetPad(0)->CycleCameraModeDownJustDown() && !CReplay::IsPlayingBack() &&
			   (m_bLookingAtPlayer || WhoIsInControlOfTheCamera == CAMCONTROL_OBBE) &&
			   !m_WideScreenOn && !m_bFailedCullZoneTestPreviously && !m_bFirstPersonBeingUsed){
				if(FrontEndMenuManager.m_ControlMethod == CONTROL_STANDARD){
					if(PedZoomIndicator == CAM_ZOOM_3)
						PedZoomIndicator = CAM_ZOOM_1;
					else
						PedZoomIndicator = CAM_ZOOM_3;
				}else
					PedZoomIndicator++;
			}
			// disabled top down and obbe's cam here
			if(PedZoomIndicator < CAM_ZOOM_1) PedZoomIndicator = CAM_ZOOM_3;
			else if(PedZoomIndicator > CAM_ZOOM_3) PedZoomIndicator = CAM_ZOOM_1;

			ReqMode = CCam::MODE_FOLLOWPED;

			// Check 1st person mode
			if((m_bLookingAtPlayer || m_bEnable1rstPersonCamCntrlsScript) && pTargetEntity->IsPed() &&
			   (!m_WideScreenOn || m_bEnable1rstPersonCamCntrlsScript) && !Cams[0].Using3rdPersonMouseCam()
#ifdef FREE_CAM
			   && (!CCamera::bFreeCam || m_bEnable1rstPersonCamCntrlsScript)
#endif
			   ){
				// See if we want to enter first person mode
				if(CPad::GetPad(0)->LookAroundLeftRight() || CPad::GetPad(0)->LookAroundUpDown()){
					m_uiFirstPersonCamLastInputTime = CTimer::GetTimeInMilliseconds();
					m_bFirstPersonBeingUsed = true;
				}
				if(m_bFirstPersonBeingUsed){
					// Or if we want to go back to 3rd person
					if(CPad::GetPad(0)->GetPedWalkLeftRight() || CPad::GetPad(0)->GetPedWalkUpDown() ||
					   CPad::GetPad(0)->GetSquare() || CPad::GetPad(0)->GetTriangle() ||
					   CPad::GetPad(0)->GetCross() || CPad::GetPad(0)->GetCircle() ||
					   CTimer::GetTimeInMilliseconds() - m_uiFirstPersonCamLastInputTime > 2850.0f){
						m_bFirstPersonBeingUsed = false;
					}else if(CPad::GetPad(0)->TargetJustDown()){
						m_bFirstPersonBeingUsed = false;
						m_bJustJumpedOutOf1stPersonBecauseOfTarget = true;
					}
				}
			}else
				m_bFirstPersonBeingUsed = false;

			if(!FindPlayerPed()->IsPedInControl() || FindPlayerPed()->m_fMoveSpeed > 0.0f)
				m_bFirstPersonBeingUsed = false;
			if(m_bFirstPersonBeingUsed){
				ReqMode = CCam::MODE_1STPERSON;
				CPad::GetPad(0)->DisablePlayerControls |= PLAYERCONTROL_CAMERA;
			}

			// Zoom value
			if(PedZoomIndicator == CAM_ZOOM_1)
				m_fPedZoomValue = 0.25f;
			else if(PedZoomIndicator == CAM_ZOOM_2)
				m_fPedZoomValue = 1.5f;
			else if(PedZoomIndicator == CAM_ZOOM_3)
				m_fPedZoomValue = 2.9f;

			// Smooth zoom value - ugly code
			if(m_bUseScriptZoomValuePed){
				if(m_fPedZoomValueSmooth < m_fPedZoomValueScript){
					m_fPedZoomValueSmooth += 0.12f * CTimer::GetTimeStep();
					m_fPedZoomValueSmooth = Min(m_fPedZoomValueSmooth, m_fPedZoomValueScript);
				}else{
					m_fPedZoomValueSmooth -= 0.12f * CTimer::GetTimeStep();
					m_fPedZoomValueSmooth = Max(m_fPedZoomValueSmooth, m_fPedZoomValueScript);
				}
			}else if(m_bFailedCullZoneTestPreviously){
				static float PedZoomedInVal = 0.5f;
				CloseInPedHeightTarget = 0.7f;
				if(m_fPedZoomValueSmooth < PedZoomedInVal){
					m_fPedZoomValueSmooth += 0.12f * CTimer::GetTimeStep();
					m_fPedZoomValueSmooth = Min(m_fPedZoomValueSmooth, PedZoomedInVal);
				}else{
					m_fPedZoomValueSmooth -= 0.12f * CTimer::GetTimeStep();
					m_fPedZoomValueSmooth = Max(m_fPedZoomValueSmooth, PedZoomedInVal);
				}
			}else{
				if(m_fPedZoomValueSmooth < m_fPedZoomValue){
					m_fPedZoomValueSmooth += 0.12f * CTimer::GetTimeStep();
					m_fPedZoomValueSmooth = Min(m_fPedZoomValueSmooth, m_fPedZoomValue);
				}else{
					m_fPedZoomValueSmooth -= 0.12f * CTimer::GetTimeStep();
					m_fPedZoomValueSmooth = Max(m_fPedZoomValueSmooth, m_fPedZoomValue);
				}
				if(PedZoomIndicator == CAM_ZOOM_3 && m_fPedZoomValue == 0.0f)
					m_fPedZoomValueSmooth = m_fPedZoomValue;
			}

			WellBufferMe(CloseInPedHeightTarget, &Cams[ActiveCam].m_fCloseInPedHeightOffset, &Cams[ActiveCam].m_fCloseInPedHeightOffsetSpeed, 0.1f, 0.025f, false);

			// Check if entering fight cam
			if(!m_bFirstPersonBeingUsed){
				if(FindPlayerPed()->GetPedState() == PED_FIGHT && !m_bUseMouse3rdPerson)
					ReqMode = CCam::MODE_FIGHT_CAM;
				if(((CPed*)pTargetEntity)->GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT &&
				   FindPlayerPed()->GetPedState() == PED_ATTACK && !m_bUseMouse3rdPerson)
					ReqMode = CCam::MODE_FIGHT_CAM;
			}

			// Garage cam
			CAttributeZone *stairsZone = nil;
			if(CCullZones::CamStairsForPlayer()){
				stairsZone = CCullZones::FindZoneWithStairsAttributeForPlayer();
				if(stairsZone)
					stairs = true;
			}
			if(CGarages::IsPointInAGarageCameraZone(pTargetEntity->GetPosition()) && !m_bUseMouse3rdPerson || stairs){
				if(!m_bGarageFixedCamPositionSet && m_bLookingAtPlayer){
					if(pToGarageWeAreIn || stairs){
						float ground;
						bool foundGround;

						if(pToGarageWeAreIn){
							// targetPos = pTargetEntity->GetPosition();	// unused
							if(pToGarageWeAreIn->m_pDoor1){
								whichDoor = 1;
								garageDoorPos1.x = pToGarageWeAreIn->m_fDoor1X;
								garageDoorPos1.y = pToGarageWeAreIn->m_fDoor1Y;
								garageDoorPos1.z = 0.0f;
								// targetPos.z = 0.0f;	// unused
								// (targetPos - doorPos1).Magnitude();	// unused
							}else if(pToGarageWeAreIn->m_pDoor2){
								whichDoor = 2;
#ifdef FIX_BUGS
								garageDoorPos2.x = pToGarageWeAreIn->m_fDoor2X;
								garageDoorPos2.y = pToGarageWeAreIn->m_fDoor2Y;
								garageDoorPos2.z = 0.0f;
#endif
							}else{
								whichDoor = 1;
								garageDoorPos1.x = pTargetEntity->GetPosition().x;
								garageDoorPos1.y = pTargetEntity->GetPosition().y;
#ifdef FIX_BUGS
								garageDoorPos1.z = 0.0f;
#else
								garageDoorPos2.z = 0.0f;
#endif
							}
						}else{
							whichDoor = 1;
							garageDoorPos1 = Cams[ActiveCam].Source;

							if(stairsZone){	// always true
								garageCenter = CVector((stairsZone->minx+stairsZone->maxx)/2, (stairsZone->miny+stairsZone->maxy)/2, 0.0f);
								if(pTargetEntity->GetPosition().x > 376.0f && pTargetEntity->GetPosition().x < 383.0f &&
								   pTargetEntity->GetPosition().y > -496.0f && pTargetEntity->GetPosition().y < -489.0f &&
								   pTargetEntity->GetPosition().z > 11.6f && pTargetEntity->GetPosition().z < 13.6f){
									garageDoorPos1 = CVector(382.6f, -489.6f, 13.1f);
								}else{
									bool bClearViewOutside = true;
									CVector dirOutside = pTargetEntity->GetPosition() - garageCenter;
									dirOutside.z = 0.0f;
									dirOutside.Normalise();
									float zoneDim = stairsZone->maxx - stairsZone->minx;
									if(zoneDim < stairsZone->maxy - stairsZone->miny)
										zoneDim = stairsZone->maxy - stairsZone->miny;
									zoneDim *= 2.0f;
									CVector posOutside = pTargetEntity->GetPosition() + zoneDim*dirOutside;
									if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), posOutside, true, false, false, false, false, false, true)){
										posOutside = pTargetEntity->GetPosition() - zoneDim*dirOutside;
										if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), posOutside, true, false, false, false, false, false, true))
											bClearViewOutside = false;
									}
									if(bClearViewOutside)
										garageDoorPos1 = posOutside;
								}
							}
						}

						if(pToGarageWeAreIn){
							garageCenter.x = pToGarageWeAreIn->GetGarageCenterX();
							garageCenter.y = pToGarageWeAreIn->GetGarageCenterY();
							garageCenter.z = 0.0f;
						}else{
							garageDoorPos1.z = 0.0f;
							if(!stairs)	// how can this be true?
								garageCenter = CVector(pTargetEntity->GetPosition().x, pTargetEntity->GetPosition().y, 0.0f);
						}
						if(whichDoor == 1)
							garageCenterToDoor = garageDoorPos1 - garageCenter;
						else
							garageCenterToDoor = garageDoorPos2 - garageCenter;
						targetPos = pTargetEntity->GetPosition();
						ground = CWorld::FindGroundZFor3DCoord(targetPos.x, targetPos.y, targetPos.z, &foundGround);
						if(!foundGround)
							ground = targetPos.z - 0.2f;
						garageCenterToDoor.z = 0.0f;
						garageCenterToDoor.Normalise();
						if(whichDoor == 1){
							if(pToGarageWeAreIn == nil && stairs){
								if(stairsZone){
									float zoneDim = stairsZone->maxx - stairsZone->minx;
									if(zoneDim < stairsZone->maxy - stairsZone->miny)
										zoneDim = stairsZone->maxy - stairsZone->miny;
									garageCamPos = garageCenter + (0.7f*zoneDim + 3.75f)*garageCenterToDoor;
								}else	// how can this be true?
									garageCamPos = garageDoorPos1 + 3.75f*garageCenterToDoor;
							}else
								garageCamPos = garageDoorPos1 + 13.0f*garageCenterToDoor;
						}else{
							garageCamPos = garageDoorPos2 + 13.0f*garageCenterToDoor;
						}
						if(PedZoomIndicator == CAM_ZOOM_TOPDOWN && !stairs){
							garageCamPos = garageCenter;
							garageCamPos.z += FindPlayerPed()->GetPosition().z + 2.1f;
							if(pToGarageWeAreIn && garageCamPos.z > pToGarageWeAreIn->m_fSupX)	// What?
								garageCamPos.z = pToGarageWeAreIn->m_fSupX;
						}else
							garageCamPos.z = ground + 3.1f;
						SetCamPositionForFixedMode(garageCamPos, CVector(0.0f, 0.0f, 0.0f));
						m_bGarageFixedCamPositionSet = true;
					}
				}

				if((CGarages::CameraShouldBeOutside() || stairs) && m_bLookingAtPlayer && m_bGarageFixedCamPositionSet){
					if(pToGarageWeAreIn || stairs){
						ReqMode = CCam::MODE_FIXED;
						m_bPlayerIsInGarage = true;
					}
				}else{
					if(m_bPlayerIsInGarage){
						m_bJustCameOutOfGarage = true;
						m_bPlayerIsInGarage = false;
					}
					ReqMode = CCam::MODE_FOLLOWPED;
				}
			}else{
				if(m_bPlayerIsInGarage){
					m_bJustCameOutOfGarage = true;
					m_bPlayerIsInGarage = false;
				}
				m_bGarageFixedCamPositionSet = false;
			}

			// Lighthouse
			if(!m_bFirstPersonBeingUsed && (pTargetEntity->GetPosition() - CVector(474.3f, -1717.6f, 0.0f)).Magnitude2D() < 6.0f)
				if((pTargetEntity->GetPosition() - CVector(474.3f, -1717.6f, 0.0f)).Magnitude2D() < 3.8f ||
				   pTargetEntity->GetPosition().z > 50.0f)
					if(!Cams[ActiveCam].Using3rdPersonMouseCam())
						ReqMode = CCam::MODE_LIGHTHOUSE;

			// Fallen into water
			if(Cams[ActiveCam].IsTargetInWater(Cams[ActiveCam].Source) &&
			   Cams[ActiveCam].CamTargetEntity->IsPed())
				ReqMode = CCam::MODE_PLAYER_FALLEN_WATER;

			// Set top down
			if(PedZoomIndicator == CAM_ZOOM_TOPDOWN &&
			   !CCullZones::Cam1stPersonForPlayer() &&
			   !CCullZones::CamNoRain() &&
			   !CCullZones::PlayerNoRain() &&
			   !m_bFirstPersonBeingUsed &&
			   !m_bPlayerIsInGarage)
				ReqMode = CCam::MODE_TOP_DOWN_PED;

			// Weapon mode
			if(!CPad::GetPad(0)->GetTarget() && PlayerWeaponMode.Mode != CCam::MODE_HELICANNON_1STPERSON)
				ClearPlayerWeaponMode();
			if(m_PlayerMode.Mode != CCam::MODE_NONE)
				ReqMode = m_PlayerMode.Mode;
			if(PlayerWeaponMode.Mode != CCam::MODE_NONE && !stairs){
				if(PlayerWeaponMode.Mode == CCam::MODE_SNIPER ||
				   PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER ||
					// game also checks MODE_MODELVIEW here but that does make any sense...
				   PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON ||
				   PlayerWeaponMode.Mode == CCam::MODE_HELICANNON_1STPERSON ||
				   PlayerWeaponMode.Mode == CCam::MODE_CAMERA ||
				   Cams[ActiveCam].GetWeaponFirstPersonOn()){
					// First person weapon mode
					if(PLAYER->GetPedState() == PED_SEEK_CAR){
						if(ReqMode == CCam::MODE_TOP_DOWN_PED || Cams[ActiveCam].GetWeaponFirstPersonOn())
							ReqMode = PlayerWeaponMode.Mode;
						else
							ReqMode = CCam::MODE_FOLLOWPED;
					}else
						ReqMode = PlayerWeaponMode.Mode;
				}else if(ReqMode != CCam::MODE_TOP_DOWN_PED && PedZoomIndicator != CAM_ZOOM_3){
					// Syphon mode
					float playerTargetDist;
					float deadPedDist = 4.0f;
					static float alivePedDist = 2.0f;	// original name lost
					float pedDist;		// actually only used on dead target
					bool targetDead = false;
					float camAngle, targetAngle;
					CVector playerToTarget = m_cvecAimingTargetCoors - pTargetEntity->GetPosition();
					CVector playerToCam = Cams[ActiveCam].Source - pTargetEntity->GetPosition();

					if(PedZoomIndicator == CAM_ZOOM_1)
						deadPedDist = 2.25f;
					if(FindPlayerPed()->m_pPointGunAt){
						// BUG: this need not be a ped!
						if(((CPed*)FindPlayerPed()->m_pPointGunAt)->DyingOrDead()){
							targetDead = true;
							pedDist = deadPedDist;
						}else
							pedDist = alivePedDist;
						playerTargetDist = playerToTarget.Magnitude2D();
						camAngle = CGeneral::GetATanOfXY(playerToCam.x, playerToCam.y);
						targetAngle = CGeneral::GetATanOfXY(playerToTarget.x, playerToTarget.y);
						ReqMode = PlayerWeaponMode.Mode;

						// Check whether to start aiming in crim-in-front mode
						if(Cams[ActiveCam].Mode != CCam::MODE_SYPHON){
							float angleDiff = camAngle - targetAngle;
							while(angleDiff >= PI) angleDiff -= 2*PI;
							while(angleDiff < -PI) angleDiff += 2*PI;
							if(Abs(angleDiff) < HALFPI && playerTargetDist < 3.5f && playerToTarget.z > -1.0f)
								ReqMode = CCam::MODE_SYPHON_CRIM_IN_FRONT;
						}

						// Check whether to go to special fixed mode
						float fixedModeDist = 0.0f;
						if((ReqMode == CCam::MODE_SYPHON_CRIM_IN_FRONT || ReqMode == CCam::MODE_SYPHON) &&
						   (m_uiTransitionState == 0 || Cams[ActiveCam].Mode == CCam::MODE_SPECIAL_FIXED_FOR_SYPHON) &&
						   playerTargetDist < pedDist && targetDead){
							if(ReqMode == CCam::MODE_SYPHON_CRIM_IN_FRONT)
								fixedModeDist = 5.0f;
							else
								fixedModeDist = 5.6f;
							ReqMode = CCam::MODE_SPECIAL_FIXED_FOR_SYPHON;
						}
						if(ReqMode == CCam::MODE_SPECIAL_FIXED_FOR_SYPHON){
							if(!PlaceForFixedWhenSniperFound){
								// Find position
								CEntity *entity;
								CColPoint colPoint;
								CVector fixedPos = pTargetEntity->GetPosition();
								fixedPos.x += fixedModeDist*Cos(camAngle);
								fixedPos.y += fixedModeDist*Sin(camAngle);
								fixedPos.z += 1.15f;
								if(CWorld::ProcessLineOfSight(pTargetEntity->GetPosition(), fixedPos, colPoint, entity, true, false, false, true, false, true, true))
									SetCamPositionForFixedMode(colPoint.point, CVector(0.0f, 0.0f, 0.0f));
								else
									SetCamPositionForFixedMode(fixedPos, CVector(0.0f, 0.0f, 0.0f));
								PlaceForFixedWhenSniperFound = true;
							}
						}else
							PlaceForFixedWhenSniperFound = false;
					}
				}
			}
		}
	}

	if(DebugCamMode)
		ReqMode = DebugCamMode;


	// Process arrested player
	static int ThePickedArrestMode;
	static int LastPedState;
	bool startArrestCam = false;
	static bool beingArrested = false;
	bool stopArrestCam = false;

	if(PLAYER->GetPedState() == PED_ARRESTED)
		beingArrested = true;
	else if(beingArrested){
		stopArrestCam = true;
		beingArrested = false;
	}
	if(LastPedState != PED_ARRESTED && PLAYER->GetPedState() == PED_ARRESTED){
		if(CarZoomIndicator != CAM_ZOOM_1STPRS || !pTargetEntity->IsVehicle())
			startArrestCam = true;
	}else
		startArrestCam = false;
	LastPedState = PLAYER->GetPedState();

	if(startArrestCam){
		ThePickedArrestMode = CCam::MODE_ARRESTCAM_ONE;
		ReqMode = CCam::MODE_ARRESTCAM_ONE;
		Cams[ActiveCam].ResetStatics = true;
	}else if(PLAYER->GetPedState() == PED_ARRESTED)
		ReqMode = ThePickedArrestMode;

	// Process dead player
	if(PLAYER->GetPedState() == PED_DEAD){
		if(Cams[ActiveCam].Mode == CCam::MODE_PED_DEAD_BABY)
			ReqMode = CCam::MODE_PED_DEAD_BABY;
		else{
			bool useArrestCam = false;
			if(pTargetEntity->IsPed()){
				for(int i = 0; i < ((CPed*)pTargetEntity)->m_numNearPeds; i++){
					CPed *ped = ((CPed*)pTargetEntity)->m_nearPeds[i];
					if(ped && ped->GetPedState() == PED_ARREST_PLAYER)
						if((ped->GetPosition() - pTargetEntity->GetPosition()).Magnitude() < 4.0f){
							ReqMode = CCam::MODE_ARRESTCAM_ONE;
							Cams[ActiveCam].ResetStatics = true;
							useArrestCam = true;
							break;
						}
				}
			}
			if(!useArrestCam){
				ReqMode = CCam::MODE_PED_DEAD_BABY;
				Cams[ActiveCam].ResetStatics = true;
			}
		}
	}

	// Restore with a jump cut
	if(m_bRestoreByJumpCut){
		if(ReqMode != CCam::MODE_FOLLOWPED &&
		   ReqMode != CCam::MODE_BEHINDCAR &&
		   ReqMode != CCam::MODE_CAM_ON_A_STRING &&
		   ReqMode != CCam::MODE_M16_1STPERSON &&
		   ReqMode != CCam::MODE_SYPHON &&
		   ReqMode != CCam::MODE_SYPHON_CRIM_IN_FRONT &&
		   ReqMode != CCam::MODE_SPECIAL_FIXED_FOR_SYPHON &&
		   ReqMode != CCam::MODE_SNIPER &&
		   ReqMode != CCam::MODE_ROCKETLAUNCHER &&
		   ReqMode != CCam::MODE_CAMERA &&
		   !m_bUseMouse3rdPerson)
			SetCameraDirectlyBehindForFollowPed_CamOnAString();

		ReqMode = m_iModeToGoTo;
		Cams[ActiveCam].Mode = ReqMode;
		m_bJust_Switched = true;
		Cams[ActiveCam].ResetStatics = true;
		Cams[ActiveCam].m_cvecCamFixedModeVector = m_vecFixedModeVector;
		Cams[ActiveCam].CamTargetEntity = pTargetEntity;
		Cams[ActiveCam].m_cvecCamFixedModeSource = m_vecFixedModeSource;
		Cams[ActiveCam].m_cvecCamFixedModeUpOffSet = m_vecFixedModeUpOffSet;
		Cams[ActiveCam].m_bCamLookingAtVector = false;
		Cams[ActiveCam].m_vecLastAboveWaterCamPosition = Cams[(ActiveCam+1)%2].m_vecLastAboveWaterCamPosition;
		m_bRestoreByJumpCut = false;
		Cams[ActiveCam].ResetStatics = true;
		pTargetEntity->RegisterReference(&pTargetEntity);
		Cams[ActiveCam].CamTargetEntity->RegisterReference(&Cams[ActiveCam].CamTargetEntity);
		CarZoomValueSmooth = CarZoomValue;
		m_fPedZoomValueSmooth = m_fPedZoomValue;
		m_uiTransitionState = 0;
		m_vecDoingSpecialInterPolation = false;
	}

	if(gbModelViewer)
		ReqMode = CCam::MODE_MODELVIEW;

	// Turn on Obbe's cam
	bool canUseObbeCam = true;
	if(pTargetEntity){
		if(pTargetEntity->IsVehicle()){
			if(CarZoomIndicator == CAM_ZOOM_CINEMATIC)
				m_bObbeCinematicCarCamOn = true;
		}else{
			if(PedZoomIndicator == CAM_ZOOM_CINEMATIC)
				m_bObbeCinematicPedCamOn = true;
		}
	}
	if(m_bTargetJustBeenOnTrain ||
	   ReqMode == CCam::MODE_SYPHON || ReqMode == CCam::MODE_SYPHON_CRIM_IN_FRONT || ReqMode == CCam::MODE_SPECIAL_FIXED_FOR_SYPHON ||
	   ReqMode == CCam::MODE_PED_DEAD_BABY || ReqMode == CCam::MODE_ARRESTCAM_ONE || ReqMode == CCam::MODE_ARRESTCAM_TWO ||
	   ReqMode == CCam::MODE_FIGHT_CAM || ReqMode == CCam::MODE_PLAYER_FALLEN_WATER ||
	   ReqMode == CCam::MODE_SNIPER || ReqMode == CCam::MODE_ROCKETLAUNCHER || ReqMode == CCam::MODE_M16_1STPERSON ||
	   ReqMode == CCam::MODE_SNIPER_RUNABOUT || ReqMode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT ||
	   ReqMode == CCam::MODE_1STPERSON_RUNABOUT || ReqMode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
	   ReqMode == CCam::MODE_FIGHT_CAM_RUNABOUT || ReqMode == CCam::MODE_HELICANNON_1STPERSON || ReqMode == CCam::MODE_CAMERA ||
	   WhoIsInControlOfTheCamera == CAMCONTROL_SCRIPT ||
	   m_bJustCameOutOfGarage || m_bPlayerIsInGarage)
		canUseObbeCam = false;

	if(m_bObbeCinematicPedCamOn && canUseObbeCam)
		ProcessObbeCinemaCameraPed();
	else if(m_bObbeCinematicCarCamOn && canUseObbeCam){
		if(pTargetEntity->IsVehicle() && ((CVehicle*)pTargetEntity)->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI ||
		   ((CVehicle*)pTargetEntity)->IsBoat())
			ProcessObbeCinemaCameraHeli();
		else
			ProcessObbeCinemaCameraCar();
	}else{
		if(m_bPlayerIsInGarage && m_bObbeCinematicCarCamOn)
			switchByJumpCut = true;
		canUseObbeCam = false;
		DontProcessObbeCinemaCamera();
	}

	// Start the transition or do a jump cut
	if(m_bLookingAtPlayer){
		// Going into top down modes normally needs a jump cut (but see below)
		if(ReqMode == CCam::MODE_TOPDOWN || ReqMode == CCam::MODE_1STPERSON || ReqMode == CCam::MODE_TOP_DOWN_PED){
			switchByJumpCut = true;
		}
		// Going from top down to vehicle
		else if(ReqMode == CCam::MODE_CAM_ON_A_STRING || ReqMode == CCam::MODE_BEHINDBOAT){
			if(Cams[ActiveCam].Mode == CCam::MODE_TOPDOWN ||
			   Cams[ActiveCam].Mode == CCam::MODE_1STPERSON ||
			   Cams[ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED)
				switchByJumpCut = true;
		}else if(ReqMode == CCam::MODE_FIXED){
			if(Cams[ActiveCam].Mode == CCam::MODE_TOPDOWN)
				switchByJumpCut = true;
		}

		// Going into Syphon mode
		if(ReqMode == CCam::MODE_SYPHON || ReqMode == CCam::MODE_SYPHON_CRIM_IN_FRONT)
			switchByJumpCut = true;

		// Top down modes can interpolate between each other
		if(ReqMode == CCam::MODE_TOPDOWN){
			if(Cams[ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED || Cams[ActiveCam].Mode == CCam::MODE_PED_DEAD_BABY)
				switchByJumpCut = false;
		}else if(ReqMode == CCam::MODE_TOP_DOWN_PED){
			if(Cams[ActiveCam].Mode == CCam::MODE_TOPDOWN || Cams[ActiveCam].Mode == CCam::MODE_PED_DEAD_BABY)
				switchByJumpCut = false;
		}

		if(ReqMode == CCam::MODE_1STPERSON || ReqMode == CCam::MODE_M16_1STPERSON ||
		   ReqMode == CCam::MODE_SNIPER || ReqMode == CCam::MODE_ROCKETLAUNCHER ||
		   ReqMode == CCam::MODE_SNIPER_RUNABOUT || ReqMode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT ||
		   ReqMode == CCam::MODE_1STPERSON_RUNABOUT || ReqMode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
		   ReqMode == CCam::MODE_FIGHT_CAM_RUNABOUT ||
		   ReqMode == CCam::MODE_HELICANNON_1STPERSON || ReqMode == CCam::MODE_CAMERA ||
		   ReqMode == CCam::MODE_ARRESTCAM_ONE || ReqMode == CCam::MODE_ARRESTCAM_TWO){
			// Going into any 1st person mode is a jump cut
			if(pTargetEntity->IsPed())
				switchByJumpCut = true;
		}else if(ReqMode == CCam::MODE_FIXED && m_bPlayerIsInGarage){
			// Going from 1st peron mode into garage
			if(Cams[ActiveCam].Mode == CCam::MODE_SNIPER ||
			   Cams[ActiveCam].Mode == CCam::MODE_HELICANNON_1STPERSON ||
			   Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER ||
			   Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON ||
			   Cams[ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED ||
			   stairs ||
			   Cams[ActiveCam].Mode == CCam::MODE_1STPERSON ||
			   Cams[ActiveCam].Mode == CCam::MODE_SNIPER_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_FIGHT_CAM_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_1STPERSON_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_CAMERA){
				if(pTargetEntity && pTargetEntity->IsVehicle())
					switchByJumpCut = true;
			}
		}else if(ReqMode == CCam::MODE_FOLLOWPED){
			bool syphonJumpCut = false;
			if(Cams[ActiveCam].Mode == CCam::MODE_SYPHON || Cams[ActiveCam].Mode == CCam::MODE_SYPHON_CRIM_IN_FRONT)
				if(!((CPed*)pTargetEntity)->CanWeRunAndFireWithWeapon())
					syphonJumpCut = true;
			if(Cams[ActiveCam].Mode == CCam::MODE_1STPERSON ||
			   Cams[ActiveCam].Mode == CCam::MODE_SNIPER ||
			   Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER ||
			   Cams[ActiveCam].Mode == CCam::MODE_ARRESTCAM_ONE ||
			   Cams[ActiveCam].Mode == CCam::MODE_ARRESTCAM_TWO ||
			   Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON ||
			   Cams[ActiveCam].Mode == CCam::MODE_PED_DEAD_BABY ||
			   Cams[ActiveCam].Mode == CCam::MODE_PILLOWS_PAPS ||
			   Cams[ActiveCam].Mode == CCam::MODE_SNIPER_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_1STPERSON_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_FIGHT_CAM_RUNABOUT ||
			   Cams[ActiveCam].Mode == CCam::MODE_HELICANNON_1STPERSON ||
			   Cams[ActiveCam].Mode == CCam::MODE_CAMERA ||
			   Cams[ActiveCam].Mode == CCam::MODE_TOPDOWN ||
			   Cams[ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED ||
			   syphonJumpCut || stopArrestCam){
				if(!m_bJustCameOutOfGarage){
					if(Cams[ActiveCam].Mode == CCam::MODE_1STPERSON ||
					   Cams[ActiveCam].Mode == CCam::MODE_SNIPER ||
					   Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER ||
					   Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON ||
					   Cams[ActiveCam].Mode == CCam::MODE_SNIPER_RUNABOUT ||
					   Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT ||
					   Cams[ActiveCam].Mode == CCam::MODE_1STPERSON_RUNABOUT ||
					   Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
					   Cams[ActiveCam].Mode == CCam::MODE_FIGHT_CAM_RUNABOUT ||
					   Cams[ActiveCam].Mode == CCam::MODE_HELICANNON_1STPERSON ||
					   Cams[ActiveCam].Mode == CCam::MODE_CAMERA){
						float angle = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y) - HALFPI;
						((CPed*)pTargetEntity)->m_fRotationCur = angle;
						((CPed*)pTargetEntity)->m_fRotationDest = angle;
					}
					m_bUseTransitionBeta = true;
					switchByJumpCut = true;
					if(Cams[ActiveCam].Mode == CCam::MODE_TOP_DOWN_PED){
						CVector front = Cams[ActiveCam].Source - FindPlayerPed()->GetPosition();
						front.z = 0.0f;
						front.Normalise();
#ifdef FIX_BUGS
						// this is almost as bad as the bugged code
						if(front.x == 0.001f && front.y == 0.001f)
							front.y = 1.0f;
#else
						// someone used = instead of == in the above check by accident
						front.x = 0.001f;
						front.y = 1.0f;
#endif
						Cams[ActiveCam].m_fTransitionBeta = CGeneral::GetATanOfXY(front.x, front.y);
					}else
						Cams[ActiveCam].m_fTransitionBeta = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y) + PI;
				}
			}
		}else if(ReqMode == CCam::MODE_FIGHT_CAM){
			if(Cams[ActiveCam].Mode == CCam::MODE_1STPERSON)
				switchByJumpCut = true;
		}else if(ReqMode == CCam::MODE_LIGHTHOUSE ||
		         ReqMode == CCam::MODE_ARRESTCAM_ONE || ReqMode == CCam::MODE_ARRESTCAM_TWO ||
		         ReqMode == CCam::MODE_PED_DEAD_BABY)
			switchByJumpCut = true;
		else if(Cams[ActiveCam].Mode == CCam::MODE_PED_DEAD_BABY && ReqMode != CCam::MODE_PED_DEAD_BABY)
			switchByJumpCut = true;

		if(ReqMode != Cams[ActiveCam].Mode && Cams[ActiveCam].CamTargetEntity == nil)
			switchByJumpCut = true;
		if(m_bPlayerIsInGarage && pToGarageWeAreIn){
			if(pToGarageWeAreIn->m_eGarageType == GARAGE_BOMBSHOP1 ||
			   pToGarageWeAreIn->m_eGarageType == GARAGE_BOMBSHOP2 ||
			   pToGarageWeAreIn->m_eGarageType == GARAGE_BOMBSHOP3){
				if(pTargetEntity->IsVehicle() && pTargetEntity->GetModelIndex() == MI_MRWHOOP &&
				   ReqMode != Cams[ActiveCam].Mode)
					switchByJumpCut = true;
			}
		}
#ifdef GTA_SCENE_EDIT
		if(CSceneEdit::m_bEditOn)
			ReqMode = CCam::MODE_EDITOR;
#endif

		if((m_uiTransitionState == 0 || switchByJumpCut) && ReqMode != Cams[ActiveCam].Mode){
			if(switchByJumpCut){
				if(!m_bPlayerIsInGarage || m_bJustCameOutOfGarage){
					if(ReqMode != CCam::MODE_FOLLOWPED &&
					   ReqMode != CCam::MODE_M16_1STPERSON &&
					   ReqMode != CCam::MODE_SNIPER &&
					   ReqMode != CCam::MODE_ROCKETLAUNCHER &&
					   !m_bUseMouse3rdPerson)
						SetCameraDirectlyBehindForFollowPed_CamOnAString();
				}
				Cams[ActiveCam].Mode = ReqMode;
				m_bJust_Switched = true;
				Cams[ActiveCam].m_cvecCamFixedModeVector = m_vecFixedModeVector;
				Cams[ActiveCam].CamTargetEntity = pTargetEntity;
				Cams[ActiveCam].m_cvecCamFixedModeSource = m_vecFixedModeSource;
				Cams[ActiveCam].m_cvecCamFixedModeUpOffSet = m_vecFixedModeUpOffSet;
				Cams[ActiveCam].m_bCamLookingAtVector = m_bLookingAtVector;
				Cams[ActiveCam].m_vecLastAboveWaterCamPosition = Cams[(ActiveCam+1)%2].m_vecLastAboveWaterCamPosition;
				CarZoomValueSmooth = CarZoomValue;
				m_fPedZoomValueSmooth = m_fPedZoomValue;
				m_uiTransitionState = 0;
				m_vecDoingSpecialInterPolation = false;
				m_bStartInterScript = false;
				Cams[ActiveCam].ResetStatics = true;

				pTargetEntity->RegisterReference(&pTargetEntity);
				Cams[ActiveCam].CamTargetEntity->RegisterReference(&Cams[ActiveCam].CamTargetEntity);
			}else if(!m_bWaitForInterpolToFinish){
				StartTransition(ReqMode);
				pTargetEntity->RegisterReference(&pTargetEntity);
				Cams[ActiveCam].CamTargetEntity->RegisterReference(&Cams[ActiveCam].CamTargetEntity);
			}
		}else if(m_uiTransitionState != 0 && ReqMode != Cams[ActiveCam].Mode){
			bool startTransition = true;

			if(ReqMode == CCam::MODE_FIGHT_CAM || Cams[ActiveCam].Mode == CCam::MODE_FIGHT_CAM)
				startTransition = false;
			if(ReqMode == CCam::MODE_FOLLOWPED && Cams[ActiveCam].Mode == CCam::MODE_FIGHT_CAM)
				startTransition = false;

			if(!m_bWaitForInterpolToFinish && m_bLookingAtPlayer && m_uiTransitionState != 0){
				CVector playerDist;
				playerDist.x = FindPlayerPed()->GetPosition().x - GetPosition().x;
				playerDist.y = FindPlayerPed()->GetPosition().y - GetPosition().y;
				playerDist.z = FindPlayerPed()->GetPosition().z - GetPosition().z;
				// if player is too far away, keep interpolating and don't transition
				if(pTargetEntity && pTargetEntity->IsPed()){
					if(playerDist.Magnitude() > 17.5f &&
					   (ReqMode == CCam::MODE_SYPHON || ReqMode == CCam::MODE_SYPHON_CRIM_IN_FRONT))
						m_bWaitForInterpolToFinish = true;
				}
			}
			if(m_bWaitForInterpolToFinish)
				startTransition = false;

			if(startTransition){
				StartTransitionWhenNotFinishedInter(ReqMode);
				pTargetEntity->RegisterReference(&pTargetEntity);
				Cams[ActiveCam].CamTargetEntity->RegisterReference(&Cams[ActiveCam].CamTargetEntity);
			}
		}else if(ReqMode == CCam::MODE_FIXED && pTargetEntity != Cams[ActiveCam].CamTargetEntity && m_bPlayerIsInGarage){
			if(m_uiTransitionState != 0)
				StartTransitionWhenNotFinishedInter(ReqMode);
			else
				StartTransition(ReqMode);
			pTargetEntity->RegisterReference(&pTargetEntity);
			Cams[ActiveCam].CamTargetEntity->RegisterReference(&Cams[ActiveCam].CamTargetEntity);
		}
	}else{
		// not following player
		bool useWeaponMode = false;
		bool jumpCutTo1stPrs = false;
		if(m_bEnable1rstPersonCamCntrlsScript || m_bAllow1rstPersonWeaponsCamera){
			if(ReqMode == CCam::MODE_1STPERSON){
				if(Cams[ActiveCam].Mode != ReqMode)
					jumpCutTo1stPrs = true;
			}else if((PlayerWeaponMode.Mode == CCam::MODE_SNIPER || PlayerWeaponMode.Mode == CCam::MODE_1STPERSON || PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER) &&
			         CPad::GetPad(0)->GetTarget() && m_bAllow1rstPersonWeaponsCamera){
				useWeaponMode = true;
				jumpCutTo1stPrs = true;
			}else if(Cams[ActiveCam].Mode != m_iModeToGoTo){
				m_bStartInterScript = true;
				m_iTypeOfSwitch = JUMP_CUT;
				CPad::GetPad(0)->DisablePlayerControls &= ~PLAYERCONTROL_CAMERA;
			}
		}

		if(m_uiTransitionState == 0 && m_bStartInterScript && m_iTypeOfSwitch == INTERPOLATION){
			ReqMode = m_iModeToGoTo;
			StartTransition(ReqMode);
			pTargetEntity->RegisterReference(&pTargetEntity);
			Cams[ActiveCam].CamTargetEntity->RegisterReference(&Cams[ActiveCam].CamTargetEntity);
		}else if(m_uiTransitionState != 0 && m_bStartInterScript && m_iTypeOfSwitch == INTERPOLATION){
			ReqMode = m_iModeToGoTo;
			StartTransitionWhenNotFinishedInter(ReqMode);
			pTargetEntity->RegisterReference(&pTargetEntity);
			Cams[ActiveCam].CamTargetEntity->RegisterReference(&Cams[ActiveCam].CamTargetEntity);
		}else if(m_bStartInterScript && m_iTypeOfSwitch == JUMP_CUT || jumpCutTo1stPrs){
			m_uiTransitionState = 0;
			m_vecDoingSpecialInterPolation = false;
			if(m_bEnable1rstPersonCamCntrlsScript && ReqMode == CCam::MODE_1STPERSON)
				Cams[ActiveCam].Mode = ReqMode;
			else if(useWeaponMode)
				Cams[ActiveCam].Mode = PlayerWeaponMode.Mode;
			else
				Cams[ActiveCam].Mode = m_iModeToGoTo;
			m_bJust_Switched = true;
			Cams[ActiveCam].ResetStatics = true;
			Cams[ActiveCam].m_cvecCamFixedModeVector = m_vecFixedModeVector;
			Cams[ActiveCam].CamTargetEntity = pTargetEntity;
			Cams[ActiveCam].m_cvecCamFixedModeSource = m_vecFixedModeSource;
			Cams[ActiveCam].m_cvecCamFixedModeUpOffSet = m_vecFixedModeUpOffSet;
			Cams[ActiveCam].m_bCamLookingAtVector = m_bLookingAtVector;
			Cams[ActiveCam].m_vecLastAboveWaterCamPosition = Cams[(ActiveCam+1)%2].m_vecLastAboveWaterCamPosition;
			m_bJust_Switched = true;
			pTargetEntity->RegisterReference(&pTargetEntity);
			Cams[ActiveCam].CamTargetEntity->RegisterReference(&Cams[ActiveCam].CamTargetEntity);
			CarZoomValueSmooth = CarZoomValue;
			m_fPedZoomValueSmooth = m_fPedZoomValue;
		}
	}

	m_bStartInterScript = false;

	if(Cams[ActiveCam].CamTargetEntity == nil)
		Cams[ActiveCam].CamTargetEntity = pTargetEntity;

	// Ped visibility
	if((Cams[ActiveCam].Mode == CCam::MODE_1STPERSON ||
	    Cams[ActiveCam].Mode == CCam::MODE_SNIPER ||
	    Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON ||
	    Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER ||
	    Cams[ActiveCam].Mode == CCam::MODE_HELICANNON_1STPERSON ||
	    Cams[ActiveCam].Mode == CCam::MODE_CAMERA) && pTargetEntity->IsPed() ||
	   Cams[ActiveCam].Mode == CCam::MODE_FLYBY)
		FindPlayerPed()->bIsVisible = false;
	else
		FindPlayerPed()->bIsVisible = true;

	bool switchedFromObbe = false;
	if(!canUseObbeCam && WhoIsInControlOfTheCamera == CAMCONTROL_OBBE){
		RestoreWithJumpCut();
		switchedFromObbe = true;
		SetCameraDirectlyBehindForFollowPed_CamOnAString();
	}

	if(PrevMode != Cams[ActiveCam].Mode || switchedFromObbe ||
	   Cams[ActiveCam].Mode == CCam::MODE_FOLLOWPED || Cams[ActiveCam].Mode == CCam::MODE_CAM_ON_A_STRING)
		if(CPad::GetPad(0)->CycleCameraModeJustDown() &&
		   !CReplay::IsPlayingBack() &&
		   (m_bLookingAtPlayer || WhoIsInControlOfTheCamera == CAMCONTROL_OBBE) &&
		   !m_WideScreenOn &&
		   (WhoIsInControlOfTheCamera != CAMCONTROL_OBBE || bSwitchedToObbeCam))
			DMAudio.PlayFrontEndSound(SOUND_HUD, 0);
}

// What a mess!
void
CCamera::UpdateTargetEntity(void)
{
	bool enteringCar = false;
	bool obbeCam = false;

	m_bPlayerWasOnBike = false;
	if(pTargetEntity && pTargetEntity->IsVehicle() && ((CVehicle*)pTargetEntity)->IsBike())
		m_bPlayerWasOnBike = true;

	if(WhoIsInControlOfTheCamera == CAMCONTROL_OBBE){
		obbeCam = true;
		if(m_iModeObbeCamIsInForCar == OBBE_COPCAR_WHEEL || m_iModeObbeCamIsInForCar == OBBE_COPCAR){
			if(FindPlayerPed()->GetPedState() != PED_ARRESTED)
				obbeCam = false;
			if(FindPlayerVehicle() == nil)
				pTargetEntity = FindPlayerPed();
		}
	}

	if((m_bLookingAtPlayer || obbeCam) && m_uiTransitionState == 0 ||
	   pTargetEntity == nil ||
	   m_bTargetJustBeenOnTrain){
		if(FindPlayerVehicle())
			pTargetEntity = FindPlayerVehicle();
		else{
			pTargetEntity = FindPlayerPed();
			// this keeps the camera on the player while entering cars
			if(PLAYER->GetPedState() == PED_ENTER_CAR ||
			   PLAYER->GetPedState() == PED_CARJACK ||
			   PLAYER->GetPedState() == PED_OPEN_DOOR)
				enteringCar = true;

			if(!enteringCar)
				if(Cams[ActiveCam].CamTargetEntity != pTargetEntity)
					Cams[ActiveCam].CamTargetEntity = pTargetEntity;
		}

		bool cantOpen = true;
		if(PLAYER){
			if(PLAYER->m_pMyVehicle){
				if(FindPlayerPed()->m_pMyVehicle->CanPedOpenLocks(PLAYER))
					cantOpen = false;
			}else if(FindPlayerPed()->m_carInObjective &&
			         (FindPlayerPed()->GetPedState() == PED_ENTER_CAR ||
			          FindPlayerPed()->GetPedState() == PED_CARJACK ||
			          FindPlayerPed()->GetPedState() == PED_OPEN_DOOR)){
				if(FindPlayerPed()->m_carInObjective->CanPedOpenLocks(FindPlayerPed()))
					cantOpen = false;
			}
		}

		if(PLAYER->GetPedState() == PED_ENTER_CAR && !cantOpen){
			if(!enteringCar && CarZoomIndicator != CAM_ZOOM_1STPRS){
				pTargetEntity = PLAYER->m_pMyVehicle;
				if(PLAYER->m_pMyVehicle == nil)
					pTargetEntity = PLAYER;
			}
		}

		if((PLAYER->GetPedState() == PED_CARJACK || PLAYER->GetPedState() == PED_OPEN_DOOR) && !cantOpen){
			if(!enteringCar && CarZoomIndicator != CAM_ZOOM_1STPRS)
				pTargetEntity = PLAYER->m_pMyVehicle;
			if(PLAYER->m_pMyVehicle == nil)
				pTargetEntity = PLAYER;
		}

		if(PLAYER->GetPedState() == PED_EXIT_CAR)
			pTargetEntity = FindPlayerPed();
		if(PLAYER->GetPedState() == PED_DRAG_FROM_CAR)
			pTargetEntity = FindPlayerPed();
		if(pTargetEntity->IsVehicle() && CarZoomIndicator == CAM_ZOOM_1STPRS && FindPlayerPed()->GetPedState() == PED_ARRESTED)
			pTargetEntity = FindPlayerPed();
	}
}

const float SOUND_DIST = 20.0f;

void
CCamera::UpdateSoundDistances(void)
{
	CVector center, end;
	CEntity *entity;
	CColPoint colPoint;
	float f;
	int n;

	if((Cams[ActiveCam].Mode == CCam::MODE_1STPERSON ||
	    Cams[ActiveCam].Mode == CCam::MODE_SNIPER ||
	    Cams[ActiveCam].Mode == CCam::MODE_SNIPER_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_1STPERSON_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_FIGHT_CAM_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_HELICANNON_1STPERSON ||
	    Cams[ActiveCam].Mode == CCam::MODE_CAMERA ||
	    Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON ||
	    Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER) &&
	   pTargetEntity->IsPed())
		center = GetPosition() + 0.5f*GetForward();
	else
		center = GetPosition() + 5.0f*GetForward();

	// check up
	n = CTimer::GetFrameCounter() % 12;
	if(n == 0){
		SoundDistUpAsReadOld = SoundDistUpAsRead;
		if(CWorld::ProcessVerticalLine(center, center.z+SOUND_DIST, colPoint, entity, true, false, false, false, true, false, nil))
			SoundDistUpAsRead = colPoint.point.z - center.z;
		else
			SoundDistUpAsRead = SOUND_DIST;
	}
	f = (n + 1) / 6.0f;
	SoundDistUp = (1.0f-f)*SoundDistUpAsReadOld + f*SoundDistUpAsRead;
}

void
CCamera::InitialiseCameraForDebugMode(void)
{
	if(FindPlayerVehicle())
		Cams[2].Source = FindPlayerVehicle()->GetPosition();
	else if(FindPlayerPed())
		Cams[2].Source = FindPlayerPed()->GetPosition();
	Cams[2].Alpha = 0.0f;
	Cams[2].Beta = 0.0f;
	Cams[2].Mode = CCam::MODE_DEBUG;
}

void
CCamera::CamShake(float strength, float x, float y, float z)
{
	CVector Dist = Cams[ActiveCam].Source - CVector(x, y, z);
	// a bit complicated...
	float dist2d = Sqrt(SQR(Dist.x) + SQR(Dist.y));
	float dist3d = Sqrt(SQR(dist2d) + SQR(Dist.z));
	if(dist3d > 100.0f) dist3d = 100.0f;
	if(dist3d < 0.0f) dist3d = 0.0f;
	float mult = 1.0f - dist3d/100.0f;

	float curForce = mult*(m_fCamShakeForce - (CTimer::GetTimeInMilliseconds() - m_uiCamShakeStart)/1000.0f);
	strength = mult*strength;
	if(Clamp(curForce, 0.0f, 2.0f) < strength){
		m_fCamShakeForce = strength;
		m_uiCamShakeStart = CTimer::GetTimeInMilliseconds();
	}
}

// This seems to be CCamera::CamShake(float) on PS2
void
CamShakeNoPos(CCamera *cam, float strength)
{
	float curForce = cam->m_fCamShakeForce - (CTimer::GetTimeInMilliseconds() - cam->m_uiCamShakeStart)/1000.0f;
	if(Clamp(curForce, 0.0f, 2.0f) < strength){
		cam->m_fCamShakeForce = strength;
		cam->m_uiCamShakeStart = CTimer::GetTimeInMilliseconds();
	}
}

bool bAvoidTest1 = false;
bool bAvoidTest2 = false;	// unused
bool bAvoidTest3 = false;	// unused
float fRangePlayerRadius = 0.5f;
float fCloseNearClipLimit = 0.15f;
float fAvoidTweakFOV = 1.15f;
float fAvoidProbTimerDamp = 0.9f;

void
CCamera::AvoidTheGeometry(const CVector &Source, const CVector &TargetPos, CVector &NewSource, float FOV)
{
	float Beta = 0.0f;
	float Alpha = 0.0f;

	CVector vDist = TargetPos - Source;
	m_vecClearGeometryVec = CVector(0.0f, 0.0f, 0.0f);
	float fDist = vDist.Magnitude();
	float fDistOnGround = vDist.Magnitude2D();
	if(vDist.x == 0.0f && vDist.y == 0.0f)
		Beta = CGeneral::GetATanOfXY(GetForward().x, GetForward().y);
	else
		Beta = CGeneral::GetATanOfXY(vDist.x, vDist.y);
	if(fDistOnGround != 0.0f || vDist.z != 0.0f)
		Alpha = CGeneral::GetATanOfXY(fDistOnGround, vDist.z);
	CVector Front(Cos(Alpha)*Cos(Beta), Cos(Alpha)*Sin(Beta), Sin(Alpha));
	NewSource = TargetPos - Front*fDist;
	Front.Normalise();

	// Clip camera source
	CColPoint point;
	CEntity *entity = nil;
	CWorld::pIgnoreEntity = pTargetEntity;
	if(CWorld::ProcessLineOfSight(TargetPos, NewSource, point, entity, true, false, false, true, false, false, true)){
		CVector ClipPoint1 = point.point;
		NewSource = point.point;
		if(!bAvoidTest1){
			if(CWorld::ProcessLineOfSight(NewSource, TargetPos, point, entity, false, true, true, true, false, false, true)){
				if((NewSource - point.point).Magnitude() < RwCameraGetNearClipPlane(Scene.camera))
					NewSource = point.point;
				else if((NewSource - ClipPoint1).Magnitude() < RwCameraGetNearClipPlane(Scene.camera))
					NewSource = ClipPoint1;
			}
		}
	}
	CWorld::pIgnoreEntity = nil;


	vDist = TargetPos - NewSource;
	fDist = vDist.Magnitude();
	if(FindPlayerPed())
		if(fDist - fRangePlayerRadius < RwCameraGetNearClipPlane(Scene.camera))
			RwCameraSetNearClipPlane(Scene.camera, Max(fDist - fRangePlayerRadius, fCloseNearClipLimit));


	static float fClearGeomAmount;
	static float fClearGeomAmountSpeed;
	float Near = RwCameraGetNearClipPlane(Scene.camera);
	float ViewPlaneHeight = Tan(DEGTORAD(FOV) / 2.0f);
	float ViewPlaneWidth = ViewPlaneHeight * CDraw::CalculateAspectRatio() * fAvoidTweakFOV;
	CVector Center = NewSource + Front*Near;
	float fClearGeomTarget = 0.0f;
	if(CWorld::TestSphereAgainstWorld(Center, ViewPlaneWidth, nil, true, false, false, true, false, true)){
		CVector CamToCol = gaTempSphereColPoints[0].point - NewSource;
		float FrontDist = DotProduct(CamToCol, Front);
		CVector CenterToCol = gaTempSphereColPoints[0].point - Center;
		if(FrontDist < DEFAULT_NEAR && FrontDist > fCloseNearClipLimit){
			if(FrontDist < RwCameraGetNearClipPlane(Scene.camera))
				RwCameraSetNearClipPlane(Scene.camera, FrontDist);
		}else if(FrontDist < fCloseNearClipLimit)
			RwCameraSetNearClipPlane(Scene.camera, fCloseNearClipLimit);

		float ColDepth = ViewPlaneWidth - CenterToCol.Magnitude();	// amount of radius in collision
		CenterToCol.Normalise();
		CVector Normal = gaTempSphereColPoints[0].normal;
		Normal.Normalise();
		if(-DotProduct(CenterToCol, Normal) < 0.0f)
			Normal = -Normal;	// always push away from col surface
		float DistToMove = DotProduct(-ColDepth*CenterToCol, Normal);
		m_vecClearGeometryVec = DistToMove*Normal;	// move source so this point is out of collision

		if(pTargetEntity && pTargetEntity->IsPed() && RwCameraGetNearClipPlane(Scene.camera) < 2.0f*fCloseNearClipLimit){
			float TargetNormalDir = DotProduct(Normal, pTargetEntity->GetForward());
			if(TargetNormalDir < 0.0f){
				// target looking towards collision
				if(m_fAvoidTheGeometryProbsTimer < 0.0f)
					m_fAvoidTheGeometryProbsTimer = 0.0f;
				m_fAvoidTheGeometryProbsTimer += CTimer::GetTimeStep();
			}else if(TargetNormalDir > 0.5f){
				// target looking away from collision
				if(m_fAvoidTheGeometryProbsTimer > 0.0f)
					m_fAvoidTheGeometryProbsTimer = 0.0f;
				m_fAvoidTheGeometryProbsTimer -= CTimer::GetTimeStep();
			}

			if(m_nAvoidTheGeometryProbsDirn == 0){
				if(CrossProduct(pTargetEntity->GetPosition() - NewSource, Normal).z > 0.0f)
					m_nAvoidTheGeometryProbsDirn = -1;
				else
					m_nAvoidTheGeometryProbsDirn = 1;
			}
		}

		fClearGeomTarget = 1.0f;
	}

	m_fAvoidTheGeometryProbsTimer *= Pow(fAvoidProbTimerDamp, CTimer::GetTimeStep());
	WellBufferMe(fClearGeomTarget, &fClearGeomAmount, &fClearGeomAmountSpeed, 0.2f, 0.05f, false);
	m_vecClearGeometryVec *= fClearGeomAmount;
	m_bMoveCamToAvoidGeom = true;
}

void
CCamera::GetArrPosForVehicleType(int apperance, int &index)
{
	switch(apperance){
	case VEHICLE_APPEARANCE_CAR: index = 0; break;
	case VEHICLE_APPEARANCE_BIKE: index = 1; break;
	case VEHICLE_APPEARANCE_HELI: index = 2; break;
	case VEHICLE_APPEARANCE_PLANE: index = 3; break;
	case VEHICLE_APPEARANCE_BOAT: index = 4; break;
	}
}

void
CCamera::GetScreenRect(CRect &rect)
{
	rect.left = 0.0f;
	rect.right = SCREEN_WIDTH;
	if(m_WideScreenOn
#ifdef CUTSCENE_BORDERS_SWITCH
		&& CMenuManager::m_PrefsCutsceneBorders
#endif
		){
		float borderSize = (SCREEN_HEIGHT / 2) * (m_ScreenReductionPercentage / 100.f);
		rect.top = borderSize - SCREEN_SCALE_Y(22.f);
		rect.bottom = SCREEN_HEIGHT - borderSize - SCREEN_SCALE_Y(14.f);
	}else{
		rect.top = 0.0f;
		rect.bottom = SCREEN_HEIGHT;
	}
}


void
CCamera::TakeControl(CEntity *target, int16 mode, int16 typeOfSwitch, int32 controller)
{
	bool doSwitch = true;
	if(controller == CAMCONTROL_OBBE && WhoIsInControlOfTheCamera == CAMCONTROL_SCRIPT)
		doSwitch = false;
	if(doSwitch){
		WhoIsInControlOfTheCamera = controller;
		if(target){
			if(mode == CCam::MODE_NONE){
				// Why are we checking the old entity?
				if(pTargetEntity->IsPed())
					mode = CCam::MODE_FOLLOWPED;
				else if(pTargetEntity->IsVehicle())
					mode = CCam::MODE_CAM_ON_A_STRING;
			}
		}else if(FindPlayerVehicle())
			target = FindPlayerVehicle();
		else
			target = PLAYER;

		m_bLookingAtVector = false;
		pTargetEntity = target;
		m_iModeToGoTo = mode;
		m_iTypeOfSwitch = typeOfSwitch;
		m_bLookingAtPlayer = false;
		m_bStartInterScript = true;
	}
}

void
CCamera::TakeControlNoEntity(const CVector &position, int16 typeOfSwitch, int32 controller)
{
	bool doSwitch = true;
	if(controller == CAMCONTROL_OBBE && WhoIsInControlOfTheCamera == CAMCONTROL_SCRIPT)
		doSwitch = false;
	if(doSwitch){
		WhoIsInControlOfTheCamera = controller;
		m_bLookingAtVector = true;
		m_bLookingAtPlayer = false;
		m_iModeToGoTo = CCam::MODE_FIXED;
		m_vecFixedModeVector = position;
		m_iTypeOfSwitch = typeOfSwitch;
		m_bStartInterScript = true;
	}
}

void
CCamera::TakeControlWithSpline(int16 typeOfSwitch)
{
	m_iModeToGoTo = CCam::MODE_FLYBY;
	m_bLookingAtPlayer = false;
	m_bLookingAtVector = false;
	m_bcutsceneFinished = false;
	m_iTypeOfSwitch = typeOfSwitch;
	m_bStartInterScript = true;
};

void
CCamera::Restore(void)
{
	m_bLookingAtPlayer = true;
	m_bLookingAtVector = false;
	m_iTypeOfSwitch = INTERPOLATION;
	m_bUseNearClipScript = false;
	m_iModeObbeCamIsInForCar = OBBE_INVALID;
	m_fPositionAlongSpline = 0.0;
	m_bStartingSpline = false;
	m_bScriptParametersSetForInterPol = false;
	WhoIsInControlOfTheCamera = CAMCONTROL_GAME;

	if(FindPlayerVehicle()){
		m_iModeToGoTo = CCam::MODE_CAM_ON_A_STRING;
		pTargetEntity = FindPlayerVehicle();
	}else{
		m_iModeToGoTo = CCam::MODE_FOLLOWPED;
		pTargetEntity = PLAYER;
	}

	if(PLAYER->GetPedState() == PED_ENTER_CAR ||
	   PLAYER->GetPedState() == PED_CARJACK ||
	   PLAYER->GetPedState() == PED_OPEN_DOOR){
		m_iModeToGoTo = CCam::MODE_CAM_ON_A_STRING;
		pTargetEntity = PLAYER->m_pSeekTarget;
	}
	if(PLAYER->GetPedState() == PED_EXIT_CAR){
		m_iModeToGoTo = CCam::MODE_FOLLOWPED;
		pTargetEntity = PLAYER;
	}

	m_bEnable1rstPersonCamCntrlsScript = false;
	m_bAllow1rstPersonWeaponsCamera = false;
	m_bUseScriptZoomValuePed = false;
	m_bUseScriptZoomValueCar = false;
	m_bStartInterScript = true;
	m_bCameraJustRestored = true;
	m_fAvoidTheGeometryProbsTimer = 0.0f;
}

void
CCamera::RestoreWithJumpCut(void)
{
	m_bRestoreByJumpCut = true;
	m_bLookingAtPlayer = true;
	m_bLookingAtVector = false;
	m_iTypeOfSwitch = JUMP_CUT;
	m_bUseNearClipScript = false;
	m_iModeObbeCamIsInForCar = OBBE_INVALID;
	m_fPositionAlongSpline = 0.0;
	m_bStartingSpline = false;
	m_bScriptParametersSetForInterPol = false;
	WhoIsInControlOfTheCamera = CAMCONTROL_GAME;
	m_bCameraJustRestored = true;
	m_bEnable1rstPersonCamCntrlsScript = false;
	m_bAllow1rstPersonWeaponsCamera = false;

	if(FindPlayerVehicle()){
		m_iModeToGoTo = CCam::MODE_CAM_ON_A_STRING;
		pTargetEntity = FindPlayerVehicle();
	}else{
		m_iModeToGoTo = CCam::MODE_FOLLOWPED;
		pTargetEntity = PLAYER;
	}

	if(PLAYER->GetPedState() == PED_ENTER_CAR ||
	   PLAYER->GetPedState() == PED_CARJACK ||
	   PLAYER->GetPedState() == PED_OPEN_DOOR){
		m_iModeToGoTo = CCam::MODE_CAM_ON_A_STRING;
		pTargetEntity = PLAYER->m_pSeekTarget;
	}
	if(PLAYER->GetPedState() == PED_EXIT_CAR){
		m_iModeToGoTo = CCam::MODE_FOLLOWPED;
		pTargetEntity = PLAYER;
	}

	m_bUseScriptZoomValuePed = false;
	m_bUseScriptZoomValueCar = false;
}

void
CCamera::SetCamPositionForFixedMode(const CVector &Source, const CVector &UpOffSet)
{
	m_vecFixedModeSource = Source;
	m_vecFixedModeUpOffSet = UpOffSet;
	m_bGarageFixedCamPositionSet = false;
}


void
CCamera::StartTransition(int16 newMode)
{
	bool switchFromFixedSyphon = false;
	bool switchSyphonMode = false;
	bool switchPedMode = false;
	bool switchPedToCar = false;
	bool switchFromFight = false;
	bool switchBikeToPed = false;
	bool switchFromFixed = false;
	bool switch1stPersonToVehicle = false;
	float betaOffset, targetBeta, camBeta, deltaBeta;
	int door;
	bool vehicleVertical;

	m_bItsOkToLookJustAtThePlayer = false;
	m_fFractionInterToStopMoving = 0.25f;
	m_fFractionInterToStopCatchUp = 0.75f;

	if(Cams[ActiveCam].Mode == CCam::MODE_SYPHON_CRIM_IN_FRONT ||
	   Cams[ActiveCam].Mode == CCam::MODE_FOLLOWPED ||
	   Cams[ActiveCam].Mode == CCam::MODE_SYPHON ||
	   Cams[ActiveCam].Mode == CCam::MODE_SPECIAL_FIXED_FOR_SYPHON){
		if(newMode == CCam::MODE_SYPHON_CRIM_IN_FRONT ||
		   newMode == CCam::MODE_FOLLOWPED ||
		   newMode == CCam::MODE_SYPHON ||
		   newMode == CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)
			switchPedMode = true;
		if(newMode == CCam::MODE_CAM_ON_A_STRING)
			switchPedToCar = true;
	}

	if(Cams[ActiveCam].Mode == CCam::MODE_SPECIAL_FIXED_FOR_SYPHON)
		switchFromFixedSyphon = true;
	if(Cams[ActiveCam].Mode == CCam::MODE_CAM_ON_A_STRING && newMode == CCam::MODE_FOLLOWPED && m_bPlayerWasOnBike)
		switchBikeToPed = true;
	if(Cams[ActiveCam].Mode == CCam::MODE_SYPHON_CRIM_IN_FRONT && newMode == CCam::MODE_SYPHON)
		switchSyphonMode = true;
	if(Cams[ActiveCam].Mode == CCam::MODE_FIGHT_CAM && newMode == CCam::MODE_FOLLOWPED)
		switchFromFight = true;
	if(Cams[ActiveCam].Mode == CCam::MODE_FIXED)
		switchFromFixed = true;

	m_bUseTransitionBeta = false;

	if((Cams[ActiveCam].Mode == CCam::MODE_SNIPER ||
	    Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER ||
	    Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON ||
	    Cams[ActiveCam].Mode == CCam::MODE_SNIPER_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_FIGHT_CAM_RUNABOUT ||
	    Cams[ActiveCam].Mode == CCam::MODE_HELICANNON_1STPERSON ||
	    Cams[ActiveCam].Mode == CCam::MODE_CAMERA ||
	    Cams[ActiveCam].Mode == CCam::MODE_1STPERSON_RUNABOUT) &&
	   pTargetEntity->IsPed()){
		float angle = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y) - HALFPI;
		((CPed*)pTargetEntity)->m_fRotationCur = angle;
		((CPed*)pTargetEntity)->m_fRotationDest = angle;
	}

	Cams[ActiveCam].m_cvecCamFixedModeVector = m_vecFixedModeVector;
	Cams[ActiveCam].CamTargetEntity = pTargetEntity;
	Cams[ActiveCam].m_cvecCamFixedModeSource = m_vecFixedModeSource;
	Cams[ActiveCam].m_cvecCamFixedModeUpOffSet = m_vecFixedModeUpOffSet;
	Cams[ActiveCam].m_bCamLookingAtVector = m_bLookingAtVector;

	if(newMode == CCam::MODE_SNIPER ||
	   newMode == CCam::MODE_ROCKETLAUNCHER ||
	   newMode == CCam::MODE_M16_1STPERSON ||
	   newMode == CCam::MODE_SNIPER_RUNABOUT ||
	   newMode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT ||
	   newMode == CCam::MODE_1STPERSON_RUNABOUT ||
	   newMode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
	   newMode == CCam::MODE_FIGHT_CAM_RUNABOUT ||
	   newMode == CCam::MODE_HELICANNON_1STPERSON ||
	   newMode == CCam::MODE_CAMERA)
		Cams[ActiveCam].Alpha = 0.0f;

	switch(Cams[ActiveCam].Mode)
	case CCam::MODE_SNIPER_RUNABOUT:
	case CCam::MODE_ROCKETLAUNCHER_RUNABOUT:
	case CCam::MODE_1STPERSON_RUNABOUT:
	case CCam::MODE_M16_1STPERSON_RUNABOUT:
	case CCam::MODE_FIGHT_CAM_RUNABOUT:
	case CCam::MODE_CAMERA:
		if(newMode == CCam::MODE_CAM_ON_A_STRING || newMode == CCam::MODE_BEHINDBOAT)
			switch1stPersonToVehicle = true;

	switch(newMode){
	case CCam::MODE_BEHINDCAR:
		Cams[ActiveCam].BetaSpeed = 0.0f;
		break;

	case CCam::MODE_BEHINDBOAT:
		Cams[ActiveCam].BetaSpeed = 0.0f;
		break;

	case CCam::MODE_FOLLOWPED:
		// Getting out of vehicle normally
		betaOffset = DEGTORAD(55.0f);
		if(m_bJustCameOutOfGarage){
			m_bUseTransitionBeta = true;
			if(Cams[ActiveCam].Front.x != 0.0f || Cams[ActiveCam].Front.y != 0.0f)
				Cams[ActiveCam].m_fTransitionBeta = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y) + PI;
			else
				Cams[ActiveCam].m_fTransitionBeta = 0.0f;
		}
		if(m_bTargetJustCameOffTrain)
			m_bCamDirectlyInFront = true;
		if(Cams[ActiveCam].Mode != CCam::MODE_CAM_ON_A_STRING)
			break;
		m_bUseTransitionBeta = true;
		vehicleVertical = false;
		if(((CPed*)pTargetEntity)->m_carInObjective &&
		   ((CPed*)pTargetEntity)->m_carInObjective->GetForward().x == 0.0f &&
		   ((CPed*)pTargetEntity)->m_carInObjective->GetForward().y == 0.0f)
			vehicleVertical = true;
		if(vehicleVertical){
			Cams[ActiveCam].m_fTransitionBeta = 0.0f;
			break;
		}
		camBeta = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y);
		if(((CPed*)pTargetEntity)->m_carInObjective)
			targetBeta = CGeneral::GetATanOfXY(((CPed*)pTargetEntity)->m_carInObjective->GetForward().x, ((CPed*)pTargetEntity)->m_carInObjective->GetForward().y);
		else
			targetBeta = camBeta;
		deltaBeta = targetBeta - camBeta;
		while(deltaBeta >= PI) deltaBeta -= 2*PI;
		while(deltaBeta < -PI) deltaBeta += 2*PI;
		deltaBeta = Abs(deltaBeta);

		door = FindPlayerPed()->m_vehDoor;
		if(deltaBeta > HALFPI){
			if(((CPed*)pTargetEntity)->m_carInObjective){
				if(((CPed*)pTargetEntity)->m_carInObjective->IsUpsideDown()){
					if(door == CAR_DOOR_LF || door == CAR_DOOR_LR)
						betaOffset = -DEGTORAD(95.0f);
				}else{
					if(door == CAR_DOOR_RF || door == CAR_DOOR_RR)
						betaOffset = -DEGTORAD(95.0f);
				}
			}
			Cams[ActiveCam].m_fTransitionBeta = targetBeta + betaOffset;
		}else{
			if(((CPed*)pTargetEntity)->m_carInObjective){
				if(((CPed*)pTargetEntity)->m_carInObjective->IsUpsideDown()){
					if(door == CAR_DOOR_RF || door == CAR_DOOR_RR)
						betaOffset = -DEGTORAD(55.0f);
					else if(door == CAR_DOOR_LF || door == CAR_DOOR_LR)
						betaOffset = DEGTORAD(95.0f);
				}else{
					if(door == CAR_DOOR_LF || door == CAR_DOOR_LR)
						betaOffset = -DEGTORAD(55.0f);
					else if(door == CAR_DOOR_RF || door == CAR_DOOR_RR)
						betaOffset = DEGTORAD(95.0f);
				}
			}
			Cams[ActiveCam].m_fTransitionBeta = targetBeta + betaOffset + PI;
		}
		break;

	case CCam::MODE_SNIPER:
	case CCam::MODE_ROCKETLAUNCHER:
	case CCam::MODE_M16_1STPERSON:
	case CCam::MODE_SNIPER_RUNABOUT:
	case CCam::MODE_ROCKETLAUNCHER_RUNABOUT:
	case CCam::MODE_1STPERSON_RUNABOUT:
	case CCam::MODE_M16_1STPERSON_RUNABOUT:
	case CCam::MODE_FIGHT_CAM_RUNABOUT:
	case CCam::MODE_HELICANNON_1STPERSON:
	case CCam::MODE_CAMERA:
		if(FindPlayerVehicle())
			Cams[ActiveCam].Beta = Atan2(FindPlayerVehicle()->GetForward().x, FindPlayerVehicle()->GetForward().y);
		else
			Cams[ActiveCam].Beta = Atan2(PLAYER->GetForward().x, PLAYER->GetForward().y);
		break;

	case CCam::MODE_SYPHON:
		Cams[ActiveCam].Alpha = 0.0f;
		Cams[ActiveCam].AlphaSpeed = 0.0f;
		break;

	case CCam::MODE_CAM_ON_A_STRING:
		// Get into vehicle
		betaOffset = DEGTORAD(57.0f);
		if(!m_bLookingAtPlayer || m_bJustCameOutOfGarage)
			break;
		m_bUseTransitionBeta = true;
		Cams[ActiveCam].m_fTransitionBeta = CGeneral::GetATanOfXY(Cams[ActiveCam].Front.x, Cams[ActiveCam].Front.y);
		break;

	case CCam::MODE_PED_DEAD_BABY:
		Cams[ActiveCam].Alpha = DEGTORAD(15.0f);
		break;

	case CCam::MODE_FIGHT_CAM:
		Cams[ActiveCam].Beta = 0.0f;
		Cams[ActiveCam].BetaSpeed = 0.0f;
		Cams[ActiveCam].Alpha = 0.0f;
		Cams[ActiveCam].AlphaSpeed = 0.0f;
		break;
	}

	Cams[ActiveCam].Init();
	Cams[ActiveCam].Mode = newMode;

	m_uiTransitionDuration = 1350;
	if(switchSyphonMode)
		m_uiTransitionDuration = 1800;
	else if(switchFromFight)
		m_uiTransitionDuration = 750;
	else if(switchPedToCar){
		m_fFractionInterToStopMoving = 0.1f;
		m_fFractionInterToStopCatchUp = 0.9f;
		m_uiTransitionDuration = 750;
	}else if(switchFromFixedSyphon){
		m_fFractionInterToStopMoving = 0.0f;
		m_fFractionInterToStopCatchUp = 1.0f;
		m_uiTransitionDuration = 600;
	}else if(switchFromFixed){
		m_fFractionInterToStopMoving = 0.05f;
		m_fFractionInterToStopCatchUp = 0.95f;
	}else if(switchBikeToPed){
		m_uiTransitionDuration = 800;
	}else if(switch1stPersonToVehicle){
		m_fFractionInterToStopMoving = 0.0f;
		m_fFractionInterToStopCatchUp = 1.0f;
		m_uiTransitionDuration = 1;
	}else if(switchPedMode){
		m_fFractionInterToStopMoving = 0.5f;
		m_fFractionInterToStopCatchUp = 0.5f;
		m_uiTransitionDuration = 350;
	}else
		m_uiTransitionDuration = 1350;	// already set above
	m_uiTransitionState = 1;
	m_uiTimeTransitionStart = CTimer::GetTimeInMilliseconds();
	m_uiTransitionJUSTStarted = 1;
	if(m_vecDoingSpecialInterPolation){
		m_cvecStartingSourceForInterPol = SourceDuringInter;
		m_cvecStartingTargetForInterPol = TargetDuringInter;
		m_cvecStartingUpForInterPol = UpDuringInter;
		m_fStartingAlphaForInterPol = m_fAlphaDuringInterPol;
		m_fStartingBetaForInterPol = m_fBetaDuringInterPol;
	}else{
		m_cvecStartingSourceForInterPol = Cams[ActiveCam].Source;
		m_cvecStartingTargetForInterPol = Cams[ActiveCam].m_cvecTargetCoorsForFudgeInter;
		m_cvecStartingUpForInterPol = Cams[ActiveCam].Up;
		m_fStartingAlphaForInterPol = Cams[ActiveCam].m_fTrueAlpha;
		m_fStartingBetaForInterPol = Cams[ActiveCam].m_fTrueBeta;
	}
	Cams[ActiveCam].m_bCamLookingAtVector = m_bLookingAtVector;
	Cams[ActiveCam].m_cvecCamFixedModeVector = m_vecFixedModeVector;
	Cams[ActiveCam].m_cvecCamFixedModeSource = m_vecFixedModeSource;
	Cams[ActiveCam].m_cvecCamFixedModeUpOffSet = m_vecFixedModeUpOffSet;
	Cams[ActiveCam].Mode = newMode;	// already done above
	Cams[ActiveCam].CamTargetEntity = pTargetEntity;
	m_uiTransitionState = 1;	// these three already done above
	m_uiTimeTransitionStart = CTimer::GetTimeInMilliseconds();
	m_uiTransitionJUSTStarted = 1;
	m_fStartingFOVForInterPol = Cams[ActiveCam].FOV;
	m_cvecSourceSpeedAtStartInter = Cams[ActiveCam].m_cvecSourceSpeedOverOneFrame;
	m_cvecTargetSpeedAtStartInter = Cams[ActiveCam].m_cvecTargetSpeedOverOneFrame;
	m_cvecUpSpeedAtStartInter = Cams[ActiveCam].m_cvecUpOverOneFrame;
	m_fAlphaSpeedAtStartInter = Cams[ActiveCam].m_fAlphaSpeedOverOneFrame;
	m_fBetaSpeedAtStartInter = Cams[ActiveCam].m_fBetaSpeedOverOneFrame;
	m_fFOVSpeedAtStartInter = Cams[ActiveCam].m_fFovSpeedOverOneFrame;
	Cams[ActiveCam].ResetStatics = true;
	if(m_bLookingAtPlayer){
		if(switchPedMode)
			m_uiTransitionDurationTargetCoors = 350;
		else
			m_uiTransitionDurationTargetCoors = 600;
		m_fFractionInterToStopMovingTarget = 0.0f;
		m_fFractionInterToStopCatchUpTarget = 1.0f;
	}else{
		if(m_bScriptParametersSetForInterPol){
			m_fFractionInterToStopMoving = m_fScriptPercentageInterToStopMoving;
			m_fFractionInterToStopCatchUp = m_fScriptPercentageInterToCatchUp;
			m_uiTransitionDuration = m_fScriptTimeForInterPolation;
		}
		m_uiTransitionDurationTargetCoors = m_uiTransitionDuration;
		m_fFractionInterToStopMovingTarget = m_fFractionInterToStopMoving;
		m_fFractionInterToStopCatchUpTarget = m_fFractionInterToStopCatchUp;
	}
}

void
CCamera::StartTransitionWhenNotFinishedInter(int16 mode)
{
	m_vecDoingSpecialInterPolation = true;
	StartTransition(mode);
}

void
CCamera::StoreValuesDuringInterPol(CVector &source, CVector &target, CVector &up, float &FOV)
{
	SourceDuringInter = source;
	TargetDuringInter = target;
	UpDuringInter = up;
	FOVDuringInter = FOV;
	CVector Dist = source - TargetDuringInter;
	float DistOnGround = Dist.Magnitude2D();
	m_fBetaDuringInterPol = CGeneral::GetATanOfXY(Dist.x, Dist.y);
	m_fAlphaDuringInterPol = CGeneral::GetATanOfXY(DistOnGround, Dist.z);
}



void
CCamera::SetWideScreenOn(void)
{
	m_WideScreenOn = true;
}

void
CCamera::SetWideScreenOff(void)
{
	m_bWantsToSwitchWidescreenOff = m_WideScreenOn;
}

void
CCamera::ProcessWideScreenOn(void)
{
	if(m_bWantsToSwitchWidescreenOff){
		m_bWantsToSwitchWidescreenOff = false;
		m_WideScreenOn = false;
		m_ScreenReductionPercentage = 0.0f;
		m_fFOV_Wide_Screen = 0.0f;
		m_fWideScreenReductionAmount = 0.0f;
	}else{
		m_fFOV_Wide_Screen = 0.3f*Cams[ActiveCam].FOV;
		m_fWideScreenReductionAmount = 1.0f;
		m_ScreenReductionPercentage = 30.0f;
	}
}

void
CCamera::DrawBordersForWideScreen(void)
{
	float bottom, top;
	if (m_WideScreenOn) {
		float borderSize = (SCREEN_HEIGHT / 2) * (m_ScreenReductionPercentage / 100.f);
		top = borderSize - SCREEN_SCALE_Y(22.f);
		bottom = SCREEN_HEIGHT - borderSize - SCREEN_SCALE_Y(14.f);
	} else {
		top = 0.f;
		bottom = SCREEN_HEIGHT;
	}

	if(m_BlurType == MOTION_BLUR_NONE || m_BlurType == MOTION_BLUR_LIGHT_SCENE)
		SetMotionBlurAlpha(80);

	// top border
	CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, top), CRGBA(0, 0, 0, 255));

	// bottom border
	CSprite2d::DrawRect(CRect(0.0f, bottom, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(0, 0, 0, 255));
}



bool
CCamera::IsItTimeForNewcam(int32 obbeMode, int32 time)
{
	CVehicle *veh;
	uint32 t = time;	// no annoying compiler warnings
	CVector fwd;

	if(obbeMode < 0)
		return true;
	switch(obbeMode){
	case OBBE_WHEEL:
		veh = FindPlayerVehicle();
		if(veh){
			if(veh->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
				return true;
			if(veh->GetModelIndex() == MI_RHINO)
				return true;
			if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), Cams[ActiveCam].Source, true, false, false, false, false, false, false))
				return true;
		}
		if(CTimer::GetTimeInMilliseconds() > t+5000)
			return true;
		SetNearClipScript(0.6f);
		return false;
	case OBBE_1:
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
			return true;
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
			return true;

		fwd = FindPlayerCoors() - m_vecFixedModeSource;
		fwd.z = 0.0f;

		// too far and driving away from cam
		if(fwd.Magnitude() > 40.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return true;
		// too close
		if(fwd.Magnitude() < 1.6f)
			return true;
		return false;
	case OBBE_2:
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
			return true;
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
			return true;

		fwd = FindPlayerCoors() - m_vecFixedModeSource;
		fwd.z = 0.0f;

		if(fwd.Magnitude() < 2.0f)
			// very close, fix near clip
			SetNearClipScript(Max(fwd.Magnitude()*0.5f, 0.05f));
		// too far and driving away from cam
		if(fwd.Magnitude() > 29.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return true;
		// too close
		if(fwd.Magnitude() < 2.0f)
			return true;
		return false;
	case OBBE_3:
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
			return true;

		fwd = FindPlayerCoors() - m_vecFixedModeSource;
		fwd.z = 0.0f;

		// too far and driving away from cam
		if(fwd.Magnitude() > 48.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return true;
		return false;
	case OBBE_1STPERSON:
		return CTimer::GetTimeInMilliseconds() > t+3000;
	case OBBE_5:
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
			return true;
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
			return true;

		fwd = FindPlayerCoors() - m_vecFixedModeSource;
		fwd.z = 0.0f;

		// too far and driving away from cam
		if(fwd.Magnitude() > 38.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return true;
		return false;
	case OBBE_ONSTRING:
		return CTimer::GetTimeInMilliseconds() > t+3000;
	case OBBE_COPCAR:
		return CTimer::GetTimeInMilliseconds() > t+2000 && !FindPlayerVehicle()->GetIsOnScreen();
	case OBBE_COPCAR_WHEEL:
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat())
			return true;
		if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), Cams[ActiveCam].Source, true, false, false, false, false, false, false))
			return true;
		if(CTimer::GetTimeInMilliseconds() > t+1000)
			return true;
		SetNearClipScript(0.6f);
		return false;

	// Ped modes
	case OBBE_9:
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
			return true;

		fwd = FindPlayerCoors() - m_vecFixedModeSource;
		fwd.z = 0.0f;

		// too far and driving away from cam
		if(fwd.Magnitude() > 20.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return true;
		return false;
	case OBBE_10:
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
			return true;

		fwd = FindPlayerCoors() - m_vecFixedModeSource;
		fwd.z = 0.0f;

		// too far and driving away from cam
		if(fwd.Magnitude() > 8.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return true;
		return false;
	case OBBE_11:
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
			return true;

		fwd = FindPlayerCoors() - m_vecFixedModeSource;
		fwd.z = 0.0f;

		// too far and driving away from cam
		if(fwd.Magnitude() > 25.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return true;
		return false;
	case OBBE_12:
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
			return true;

		fwd = FindPlayerCoors() - m_vecFixedModeSource;
		fwd.z = 0.0f;

		// too far and driving away from cam
		if(fwd.Magnitude() > 8.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return true;
		return false;
	case OBBE_13:
		return CTimer::GetTimeInMilliseconds() > t+5000;

	// Heli modes
	case OBBE_14:
		if(FindPlayerVehicle())
			if(!CWorld::GetIsLineOfSightClear(pTargetEntity->GetPosition(), Cams[ActiveCam].Source, true, false, false, false, false, false, false))
				return true;
		return CTimer::GetTimeInMilliseconds() > t+8000;
	case OBBE_15:
		if(FindPlayerVehicle()){
			if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
				return true;
			fwd = FindPlayerCoors() - m_vecFixedModeSource;
			fwd.z = 0.0f;

			// too far and driving away from cam
			if(fwd.Magnitude() > 44.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
				return true;
			// too close
			if(fwd.Magnitude() < 2.0f)
				return true;
		}
		return false;
	case OBBE_16:
		if(FindPlayerVehicle()){
			if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
				return true;
			fwd = FindPlayerCoors() - m_vecFixedModeSource;
			fwd.z = 0.0f;

			// too far and driving away from cam
			if(fwd.Magnitude() > 50.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
				return true;
			// too close
			if(fwd.Magnitude() < 3.0f)
				return true;
		}
		return false;
	case OBBE_17:
		if(FindPlayerVehicle()){
			if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
				return true;
			fwd = FindPlayerCoors() - m_vecFixedModeSource;
			fwd.z = 0.0f;

			// too far
			if(fwd.Magnitude() > 50.0f)
				return true;
			// too close
			if(fwd.Magnitude() < 2.0f)
				return true;
		}
		return false;
	case OBBE_18:
		if(FindPlayerVehicle()){
			if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
				return true;
			fwd = FindPlayerCoors() - m_vecFixedModeSource;

			// too far
			if(fwd.Magnitude() > 57.0f)
				return true;
			// too close
			if(fwd.Magnitude() < 1.0f)
				return true;
		}
		return false;
	case OBBE_19:
		if(FindPlayerVehicle()){
			if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), m_vecFixedModeSource, true, false, false, false, false, false, false))
				return true;
			fwd = FindPlayerCoors() - m_vecFixedModeSource;
			fwd.z = 0.0f;

			// too far
			if(fwd.Magnitude() > 36.0f)
				return true;
			// too close
			if(fwd.Magnitude() < 2.0f)
				return true;
		}
		return false;
	case OBBE_ONSTRING_HELI:
		return CTimer::GetTimeInMilliseconds() > t+5000;

	default:
		return false;
	}
}

bool
CCamera::TryToStartNewCamMode(int obbeMode)
{
	CVehicle *veh;
	CVector target, camPos, playerSpeed, fwd, fwd2;
	float angle;
	float ground;
	bool foundGround;
	int i;

	if(obbeMode < 0)
		return true;
	switch(obbeMode){
	case OBBE_WHEEL:
		veh = FindPlayerVehicle();
		if(veh == nil || (veh->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER) || veh->GetModelIndex() == MI_RHINO)
			return false;
		target = Multiply3x3(FindPlayerVehicle()->GetMatrix(), CVector(-1.4f, -2.3f, 0.3f));
		target += FindPlayerVehicle()->GetPosition();
		if(!CWorld::GetIsLineOfSightClear(veh->GetPosition(), target, true, false, false, false, false, false, false))
			return false;
		TakeControl(veh, CCam::MODE_WHEELCAM, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_1:
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 20.0f*playerSpeed;
		camPos += 3.0f*CVector(playerSpeed.y, -playerSpeed.x, 0.0f);
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
			return false;

		ground = CWorld::FindGroundZFor3DCoord(camPos.x, camPos.y, camPos.z+5.0f, &foundGround);
		if(foundGround)
			camPos.z = ground + 1.5f;
		else{
			ground = CWorld::FindRoofZFor3DCoord(camPos.x, camPos.y, camPos.z-5.0f, &foundGround);
			if(foundGround)
				camPos.z = ground + 1.5f;
		}
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		fwd = FindPlayerCoors() - camPos;
		fwd.z = 0.0f;
		// too far and driving away from cam
		if(fwd.Magnitude() > 40.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return false;
		// too close
		if(fwd.Magnitude() < 2.5f)
			return true;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_2:
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
			return false;
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 16.0f*playerSpeed;
		camPos += 2.5f*CVector(playerSpeed.y, -playerSpeed.x, 0.0f);

		ground = CWorld::FindGroundZFor3DCoord(camPos.x, camPos.y, camPos.z+5.0f, &foundGround);
		if(foundGround)
			camPos.z = ground + 0.5f;
		else{
			ground = CWorld::FindRoofZFor3DCoord(camPos.x, camPos.y, camPos.z-5.0f, &foundGround);
			if(foundGround)
				camPos.z = ground + 0.5f;
		}
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		fwd = FindPlayerCoors() - camPos;
		fwd.z = 0.0f;
		// too far and driving away from cam
		if(fwd.Magnitude() > 29.0f && DotProduct(FindPlayerSpeed(), fwd) > 0.0f)
			return false;
		// too close
		if(fwd.Magnitude() < 2.0f)
			return true;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_3:
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 30.0f*playerSpeed;
		camPos += 8.0f*CVector(playerSpeed.y, -playerSpeed.x, 0.0f);

		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_1STPERSON:
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_5:
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 30.0f*playerSpeed;
		camPos += 6.0f*CVector(playerSpeed.y, -playerSpeed.x, 0.0f);

		ground = CWorld::FindGroundZFor3DCoord(camPos.x, camPos.y, camPos.z+5.0f, &foundGround);
		if(foundGround)
			camPos.z = ground + 3.5f;
		else{
			ground = CWorld::FindRoofZFor3DCoord(camPos.x, camPos.y, camPos.z-5.0f, &foundGround);
			if(foundGround)
				camPos.z = ground + 3.5f;
		}
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_ONSTRING:
		TakeControl(FindPlayerEntity(), CCam::MODE_CAM_ON_A_STRING, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_COPCAR:
#ifdef FIX_BUGS
		if (CReplay::IsPlayingBack())
			return false;
#endif
		if(FindPlayerPed()->m_pWanted->GetWantedLevel() < 1)
			return false;
		if(FindPlayerVehicle() == nil)
			return false;
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
			return false;
		i = CPools::GetVehiclePool()->GetSize();
		while(--i >= 0){
			veh = CPools::GetVehiclePool()->GetSlot(i);
			if(veh && veh->IsCar() && veh != FindPlayerVehicle() && veh->bIsLawEnforcer){
				float dx = veh->GetPosition().x - FindPlayerCoors().x;
				float dy = veh->GetPosition().y - FindPlayerCoors().y;
				float dist = (veh->GetPosition() - FindPlayerCoors()).Magnitude();
				if(dist < 30.0f){
					if(dx*FindPlayerVehicle()->GetForward().x + dy*FindPlayerVehicle()->GetForward().y < 0.0f &&
					   veh->GetForward().x*FindPlayerVehicle()->GetForward().x + veh->GetForward().y*FindPlayerVehicle()->GetForward().y > 0.8f){
						TakeControl(veh, CCam::MODE_CAM_ON_A_STRING, JUMP_CUT, CAMCONTROL_OBBE);
						return true;
					}
				}
			}
		}
		return false;
	case OBBE_COPCAR_WHEEL:
#ifdef FIX_BUGS
		if (CReplay::IsPlayingBack())
			return false;
#endif
		if(FindPlayerPed()->m_pWanted->GetWantedLevel() < 1)
			return false;
		if(FindPlayerVehicle() == nil)
			return false;
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat() && pTargetEntity->GetModelIndex() != MI_SKIMMER)
			return false;
		i = CPools::GetVehiclePool()->GetSize();
		while(--i >= 0){
			veh = CPools::GetVehiclePool()->GetSlot(i);
			if(veh && veh->IsCar() && veh != FindPlayerVehicle() && veh->bIsLawEnforcer){
				float dx = veh->GetPosition().x - FindPlayerCoors().x;
				float dy = veh->GetPosition().y - FindPlayerCoors().y;
				float dist = (veh->GetPosition() - FindPlayerCoors()).Magnitude();
				if(dist < 30.0f){
					if(dx*FindPlayerVehicle()->GetForward().x + dy*FindPlayerVehicle()->GetForward().y < 0.0f &&
					   veh->GetForward().x*FindPlayerVehicle()->GetForward().x + veh->GetForward().y*FindPlayerVehicle()->GetForward().y > 0.8f){
						target = Multiply3x3(veh->GetMatrix(), CVector(-1.4f, -2.3f, 0.3f));
						target += veh->GetPosition();
						if(!CWorld::GetIsLineOfSightClear(veh->GetPosition(), target, true, false, false, false, false, false, false))
							return false;
						TakeControl(veh, CCam::MODE_WHEELCAM, JUMP_CUT, CAMCONTROL_OBBE);
						return true;
					}
				}
			}
		}
		return false;

	case OBBE_9:
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 15.0f*playerSpeed;
		camPos += CVector(2.0f, 1.0f, 0.0f);

		ground = CWorld::FindGroundZFor3DCoord(camPos.x, camPos.y, camPos.z+5.0f, &foundGround);
		if(foundGround)
			camPos.z = ground + 0.5f;
		else{
			ground = CWorld::FindRoofZFor3DCoord(camPos.x, camPos.y, camPos.z-5.0f, &foundGround);
			if(foundGround)
				camPos.z = ground + 0.5f;
		}
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_10:
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 5.0f*playerSpeed;
		camPos += CVector(2.0f, 1.0f, 0.5f);

		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_11:
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 20.0f*playerSpeed;
		camPos += CVector(2.0f, 1.0f, 20.0f);

		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_12:
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 5.0f*playerSpeed;
		camPos += CVector(2.0f, 1.0f, 10.5f);

		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_13:
#ifdef FIX_BUGS
		TakeControl(FindPlayerEntity(), CCam::MODE_TOP_DOWN_PED, JUMP_CUT, CAMCONTROL_OBBE);
#else
		TakeControl(FindPlayerEntity(), CCam::MODE_TOPDOWN, JUMP_CUT, CAMCONTROL_OBBE);
#endif
		return true;

	// Heli modes
	case OBBE_14:
		veh = FindPlayerVehicle();
		if(veh == nil)
			return false;
		target = Multiply3x3(FindPlayerVehicle()->GetMatrix(), CVector(-1.4f, -2.3f, 0.3f));
		target += FindPlayerVehicle()->GetPosition();
		if(!veh->IsBoat() && !CWorld::GetIsLineOfSightClear(veh->GetPosition(), target, true, false, false, false, false, false, false))
			return false;
		TakeControl(veh, CCam::MODE_WHEELCAM, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_15:
		if(FindPlayerVehicle() == nil)
			return false;
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		camPos += 34.0f*playerSpeed;
		camPos.z = FindPlayerCoors().z + 0.5f;
		if(FindPlayerVehicle()->IsBoat())
			camPos.z += 1.0f;

		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		fwd = FindPlayerCoors() - camPos;
		fwd2 = FindPlayerCoors() - camPos;
		fwd2.z = 0.0f;
		// too far and driving away from cam
		if(fwd.Magnitude() > 44.0f && DotProduct(FindPlayerSpeed(), fwd2) > 0.0f)
			return false;
		// too close
		if(fwd.Magnitude() < 3.0f)
			return true;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_16:
		if(FindPlayerVehicle() == nil)
			return false;
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		angle = CGeneral::GetATanOfXY(playerSpeed.x, playerSpeed.y) + DEGTORAD(60.0f);
		playerSpeed += CVector(Cos(angle), Sin(angle), 0.0f);
		playerSpeed.Normalise();
		camPos += 30.0f*playerSpeed;
		camPos.z = FindPlayerCoors().z - 5.5f;

		foundGround = false;
		ground = CWorld::FindRoofZFor3DCoord(camPos.x, camPos.y, camPos.z+5.0f, &foundGround);
		if(foundGround)
			camPos.z = ground + 0.5f;
		else if(CWaterLevel::GetWaterLevelNoWaves(camPos.x, camPos.y, camPos.z, &ground)){
			float waterOffset = 1.0f;
			if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat())
				waterOffset = -2.0f;
			if(camPos.z < ground + waterOffset)
				camPos.z = ground + waterOffset;
		}
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		fwd = FindPlayerCoors() - camPos;
		// too far
		if(fwd.Magnitude() > 50.0f)
			return false;
		// too close
		if(fwd.Magnitude() < 3.0f)
			return true;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_17:
		if(FindPlayerVehicle() == nil)
			return false;
		camPos = FindPlayerCoors();
		playerSpeed = FindPlayerSpeed();
		playerSpeed.z = 0.0f;
		playerSpeed.Normalise();
		angle = CGeneral::GetATanOfXY(playerSpeed.x, playerSpeed.y) + DEGTORAD(190.0f);
		playerSpeed += CVector(Cos(angle), Sin(angle), 0.0f);
		playerSpeed.Normalise();
		camPos += 25.0f*playerSpeed;
		camPos.z = FindPlayerCoors().z - 1.0f;

		foundGround = false;
		ground = CWorld::FindRoofZFor3DCoord(camPos.x, camPos.y, camPos.z+5.0f, &foundGround);
		if(foundGround)
			camPos.z = ground + 0.5f;
		else if(CWaterLevel::GetWaterLevelNoWaves(camPos.x, camPos.y, camPos.z, &ground)){
			float waterOffset = 1.0f;
			if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat())
				waterOffset = -2.0f;
			if(camPos.z < ground + waterOffset)
				camPos.z = ground + waterOffset;
		}
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		fwd = FindPlayerCoors() - camPos;
		fwd2 = FindPlayerCoors() - camPos;
		fwd2.z = 0.0f;
		// too far and driving away from cam
		if(fwd.Magnitude() > 50.0f && DotProduct(FindPlayerSpeed(), fwd2) > 0.0f)
			return false;
		// too close
		if(fwd.Magnitude() < 2.0f)
			return true;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_18:
		camPos = FindPlayerCoors();
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat())
			camPos.z += 23.0f;
		else
			camPos.z -= 23.0f;
		playerSpeed = FindPlayerSpeed();
		angle = CGeneral::GetATanOfXY(playerSpeed.x, playerSpeed.y) + DEGTORAD(145.0f);
		playerSpeed += CVector(Cos(angle), Sin(angle), 0.0f);
		playerSpeed.Normalise();
		camPos += 15.0f*playerSpeed;

		foundGround = false;
		ground = CWorld::FindGroundZFor3DCoord(camPos.x, camPos.y, camPos.z+5.0f, &foundGround);
#ifdef FIX_BUGS
		if(foundGround)
#else
		if(ground == true)
#endif
		{
			if(camPos.z < ground)
				camPos.z = ground + 0.5f;
		}else if(CWaterLevel::GetWaterLevelNoWaves(camPos.x, camPos.y, camPos.z, &ground)){
			float waterOffset = 1.0f;
			if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat())
				waterOffset = -2.0f;
			if(camPos.z < ground + waterOffset)
				camPos.z = ground + waterOffset;
		}
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		fwd = FindPlayerCoors() - camPos;
		// too far
		if(fwd.Magnitude() > 57.0f)
			return false;
		// too close
		if(fwd.Magnitude() < 1.0f)
			return true;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_19:
		camPos = FindPlayerCoors();
		if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat())
			camPos.z += 4.0f;
		else
			camPos.z -= 1.0f;
		playerSpeed = FindPlayerSpeed();
		angle = CGeneral::GetATanOfXY(playerSpeed.x, playerSpeed.y) + DEGTORAD(28.0f);
		playerSpeed += CVector(Cos(angle), Sin(angle), 0.0f);
		playerSpeed.Normalise();
		camPos += 12.5f*playerSpeed;

		foundGround = false;
		ground = CWorld::FindGroundZFor3DCoord(camPos.x, camPos.y, camPos.z+5.0f, &foundGround);
#ifdef FIX_BUGS
		if(foundGround)
#else
		if(ground == true)
#endif
		{
			if(camPos.z < ground)
				camPos.z = ground + 0.5f;
		}else if(CWaterLevel::GetWaterLevelNoWaves(camPos.x, camPos.y, camPos.z, &ground)){
			float waterOffset = 1.0f;
			if(FindPlayerVehicle() && FindPlayerVehicle()->IsBoat())
				waterOffset = -2.0f;
			if(camPos.z < ground + waterOffset)
				camPos.z = ground + waterOffset;
		}
		if(!CWorld::GetIsLineOfSightClear(FindPlayerCoors(), camPos, true, false, false, false, false, false, false))
			return false;

		fwd = FindPlayerCoors() - camPos;
		// too far
		if(fwd.Magnitude() > 36.0f)
			return false;
		// too close
		if(fwd.Magnitude() < 2.0f)
			return true;

		SetCamPositionForFixedMode(camPos, CVector(0.0f, 0.0f, 0.0f));
		TakeControl(FindPlayerEntity(), CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	case OBBE_ONSTRING_HELI:
		TakeControl(FindPlayerEntity(), CCam::MODE_CAM_ON_A_STRING, JUMP_CUT, CAMCONTROL_OBBE);
		return true;
	default:
		return false;
	}
}

int32 SequenceOfCarCams[16] = {
	OBBE_WHEEL, OBBE_COPCAR, OBBE_3, OBBE_1, OBBE_3, OBBE_COPCAR_WHEEL,
	OBBE_2, OBBE_3, OBBE_COPCAR_WHEEL, OBBE_COPCAR, OBBE_2, OBBE_3,
	OBBE_5, OBBE_3,
	OBBE_ONSTRING	// actually unused...
};

void
CCamera::ProcessObbeCinemaCameraCar(void)
{
	static int OldMode = -1;
	static int32 TimeForNext = 0;
	int i = 0;

	if(!bDidWeProcessAnyCinemaCam){
		OldMode = -1;
		bSwitchedToObbeCam = true;
	}

	if(!bDidWeProcessAnyCinemaCam || IsItTimeForNewcam(SequenceOfCarCams[OldMode], TimeForNext)){
		// This is very strange code...
		for(OldMode = (OldMode+1) % 14;
		    !TryToStartNewCamMode(SequenceOfCarCams[OldMode]) && i <= 14;
		    OldMode = (OldMode+1) % 14)
			i++;
		TimeForNext = CTimer::GetTimeInMilliseconds();
		if(i >= 14){
			OldMode = 14;
			TryToStartNewCamMode(SequenceOfCarCams[14]);
		}
	}

	m_iModeObbeCamIsInForCar = OldMode;
	bDidWeProcessAnyCinemaCam = true;
}

int32 SequenceOfHeliCams[6] = { OBBE_14, OBBE_15, OBBE_16, OBBE_17, OBBE_18, OBBE_19 };

void
CCamera::ProcessObbeCinemaCameraHeli(void)
{
	static int OldMode = -1;
	static int32 TimeForNext = 0;
	int i = 0;

	if(!bDidWeProcessAnyCinemaCam){
		OldMode = -1;
		bSwitchedToObbeCam = true;
	}

	if(!bDidWeProcessAnyCinemaCam || IsItTimeForNewcam(SequenceOfHeliCams[OldMode], TimeForNext)){
		// This is very strange code...
		for(OldMode = (OldMode+1) % 6;
		    !TryToStartNewCamMode(SequenceOfCarCams[OldMode]) && i <= 6;
		    OldMode = (OldMode+1) % 6)
			i++;
		if(i >= 6){
			OldMode = 6;
			if(Cams[ActiveCam].Mode != CCam::MODE_CAM_ON_A_STRING){
				TryToStartNewCamMode(OBBE_ONSTRING_HELI);
				TimeForNext = CTimer::GetTimeInMilliseconds();
			}
		}else
			TimeForNext = CTimer::GetTimeInMilliseconds();
	}

	m_iModeObbeCamIsInForCar = OldMode;
	bDidWeProcessAnyCinemaCam = true;
}

int32 SequenceOfPedCams[5] = { OBBE_9, OBBE_10, OBBE_11, OBBE_12, OBBE_13 };

void
CCamera::ProcessObbeCinemaCameraPed(void)
{
	// static bool bObbePedProcessed = false;	// unused
	static int PedOldMode = -1;
	static int32 PedTimeForNext = 0;

	if(!bDidWeProcessAnyCinemaCam)
		PedOldMode = -1;

	if(!bDidWeProcessAnyCinemaCam || IsItTimeForNewcam(SequenceOfPedCams[PedOldMode], PedTimeForNext)){
		for(PedOldMode = (PedOldMode+1) % 5;
		    !TryToStartNewCamMode(SequenceOfPedCams[PedOldMode]);
		    PedOldMode = (PedOldMode+1) % 5);
		PedTimeForNext = CTimer::GetTimeInMilliseconds();
	}
	bDidWeProcessAnyCinemaCam = true;
}

void
CCamera::DontProcessObbeCinemaCamera(void)
{
	bDidWeProcessAnyCinemaCam = false;
}

#ifdef GTA_TRAIN
void
CCamera::LoadTrainCamNodes(char const *name)
{
	CFileMgr::SetDir("data");

	char token[16] = { 0 };
	char filename[16] = { 0 };
	uint8 *buf;
	ssize_t bufpos = 0;
	int field = 0;
	int tokpos = 0;
	char c;
	int i;
	ssize_t len;

	strcpy(filename, name);
	len = (int)strlen(filename);
	filename[len] = '.';
	filename[len+1] = 'd';
	filename[len+2] = 'a';
	filename[len+3] = 't';

	m_uiNumberOfTrainCamNodes = 0;

	buf = new uint8[20000];
	len = CFileMgr::LoadFile(filename, buf, 20000, "r");

	for(i = 0; i < MAX_NUM_OF_NODES; i++){
		m_arrTrainCamNode[i].m_cvecPointToLookAt = CVector(0.0f, 0.0f, 0.0f);
		m_arrTrainCamNode[i].m_cvecMinPointInRange = CVector(0.0f, 0.0f, 0.0f);
		m_arrTrainCamNode[i].m_cvecMaxPointInRange = CVector(0.0f, 0.0f, 0.0f);
		m_arrTrainCamNode[i].m_fDesiredFOV = 0.0f;
		m_arrTrainCamNode[i].m_fNearClip = 0.0f;
	}

	while(bufpos <= len){
		c = buf[bufpos];
		switch(c){
		case '-':
		case '.':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
//		case '10': case '11': case '12': case '13':	// ahem...
			token[tokpos++] = c;
			bufpos++;
			break;

		case ',':
		case ';':	// game has the code for this duplicated but we handle both under the same case
			switch((field+14)%14){
			case 0:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.x = atof(token);
				break;
			case 1:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.y = atof(token);
				break;
			case 2:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecCamPosition.z = atof(token);
				break;
			case 3:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.x = atof(token);
				break;
			case 4:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.y = atof(token);
				break;
			case 5:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecPointToLookAt.z = atof(token);
				break;
			case 6:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.x = atof(token);
				break;
			case 7:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.y = atof(token);
				break;
			case 8:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMinPointInRange.z = atof(token);
				break;
			case 9:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.x = atof(token);
				break;
			case 10:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.y = atof(token);
				break;
			case 11:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_cvecMaxPointInRange.z = atof(token);
				break;
			case 12:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_fDesiredFOV = atof(token);
				break;
			case 13:
				m_arrTrainCamNode[m_uiNumberOfTrainCamNodes].m_fNearClip = atof(token);
				m_uiNumberOfTrainCamNodes++;
				break;
			}
			field++;
			bufpos++;
			memset(token, 0, sizeof(token));
			tokpos = 0;
			break;

		default:
			bufpos++;
			break;
		}
	}

	delete[] buf;
	CFileMgr::SetDir("");
}

void
CCamera::Process_Train_Camera_Control(void)
{
	bool found = false;
	CTrain *target = (CTrain*)pTargetEntity;
	m_bUseSpecialFovTrain = true;
	static bool OKtoGoBackToNodeCam = true;	// only ever set to true
	uint32 i;

	if(target->m_nTrackId == TRACK_ELTRAIN && !m_bAboveGroundTrainNodesLoaded){
		m_bAboveGroundTrainNodesLoaded = true;
		m_bBelowGroundTrainNodesLoaded = false;
		LoadTrainCamNodes("Train");
		m_uiTimeLastChange = CTimer::GetTimeInMilliseconds();
		OKtoGoBackToNodeCam = true;
		m_iCurrentTrainCamNode = 0;
	}
	if(target->m_nTrackId == TRACK_SUBWAY && !m_bBelowGroundTrainNodesLoaded){
		m_bBelowGroundTrainNodesLoaded = true;
		m_bAboveGroundTrainNodesLoaded = false;
		LoadTrainCamNodes("Train2");
		m_uiTimeLastChange = CTimer::GetTimeInMilliseconds();
		OKtoGoBackToNodeCam = true;
		m_iCurrentTrainCamNode = 0;
	}

	m_bTargetJustBeenOnTrain = true;
	uint32 node = m_iCurrentTrainCamNode;
	for(i = 0; i < m_uiNumberOfTrainCamNodes && !found; i++){
		if(target->IsWithinArea(m_arrTrainCamNode[node].m_cvecMinPointInRange.x,
		                        m_arrTrainCamNode[node].m_cvecMinPointInRange.y,
		                        m_arrTrainCamNode[node].m_cvecMinPointInRange.z,
		                        m_arrTrainCamNode[node].m_cvecMaxPointInRange.x,
		                        m_arrTrainCamNode[node].m_cvecMaxPointInRange.y,
		                        m_arrTrainCamNode[node].m_cvecMaxPointInRange.z)){
			m_iCurrentTrainCamNode = node;
			found = true;
		}
		node++;
		if(node >= m_uiNumberOfTrainCamNodes)
			node = 0;
	}
#ifdef FIX_BUGS
	// Not really a bug but be nice and respect the debug mode
	if(DebugCamMode){
		TakeControl(target, DebugCamMode, JUMP_CUT, CAMCONTROL_SCRIPT);
		return;
	}
#endif

	if(found){
		SetWideScreenOn();
		if(DotProduct(((CTrain*)pTargetEntity)->GetMoveSpeed(), pTargetEntity->GetForward()) < 0.001f){
			TakeControl(FindPlayerPed(), CCam::MODE_FOLLOWPED, JUMP_CUT, CAMCONTROL_SCRIPT);
			if(target->Doors[0].IsFullyOpen())
				SetWideScreenOff();
		}else{
			SetCamPositionForFixedMode(m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecCamPosition, CVector(0.0f, 0.0f, 0.0f));
			if(m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecPointToLookAt.x == 999.0f &&
			   m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecPointToLookAt.y == 999.0f &&
			   m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecPointToLookAt.z == 999.0f)
				TakeControl(target, CCam::MODE_FIXED, JUMP_CUT, CAMCONTROL_SCRIPT);
			else
				TakeControlNoEntity(m_arrTrainCamNode[m_iCurrentTrainCamNode].m_cvecPointToLookAt, JUMP_CUT, CAMCONTROL_SCRIPT);
			RwCameraSetNearClipPlane(Scene.camera, m_arrTrainCamNode[m_iCurrentTrainCamNode].m_fNearClip);
		}
	}else{
		if(DotProduct(((CTrain*)pTargetEntity)->GetMoveSpeed(), pTargetEntity->GetForward()) < 0.001f){
			TakeControl(FindPlayerPed(), CCam::MODE_FOLLOWPED, JUMP_CUT, CAMCONTROL_SCRIPT);
			if(target->Doors[0].IsFullyOpen())
				SetWideScreenOff();
		}
	}
}
#endif


void
CCamera::LoadPathSplines(int file)
{
	bool reading = true;
	char c, token[32] = { 0 };
	int i, j, n;

	n = 0;

	DeleteCutSceneCamDataMemory();
	for(i = 0; i < MAX_NUM_OF_SPLINETYPES; i++)
		m_arrPathArray[i].m_arr_PathData = new float[CCamPathSplines::MAXPATHLENGTH];

//	Why is this gone?
//	for(i = 0; i < MAX_NUM_OF_SPLINETYPES; i++)
//		for(j = 0; j < CCamPathSplines::MAXPATHLENGTH; j++)
//			m_arrPathArray[i].m_arr_PathData[j] = 0.0f;

	m_bStartingSpline = false;

	i = 0;
	j = 0;
	while(reading){
		CFileMgr::Read(file, &c, 1);
		switch(c){
		case '\0':
			reading = false;
			break;

		case '+': case '-': case '.':
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case 'e': case 'E':
			token[n++] = c;
			break;

		case ',':
#ifdef FIX_BUGS
			if(i < MAX_NUM_OF_SPLINETYPES && j < CCamPathSplines::MAXPATHLENGTH)
#endif
			m_arrPathArray[i].m_arr_PathData[j] = atof(token);
			j++;
			memset(token, 0, 32);
			n = 0;
			break;

		case ';':
#ifdef FIX_BUGS
			if(i < MAX_NUM_OF_SPLINETYPES && j < CCamPathSplines::MAXPATHLENGTH)
#endif
			m_arrPathArray[i].m_arr_PathData[j] = atof(token);
			i++;
			j = 0;
			if (i == MAX_NUM_OF_SPLINETYPES)
				reading = false;
			memset(token, 0, 32);
			n = 0;
		}
	}
}

void
CCamera::DeleteCutSceneCamDataMemory(void)
{
	int i;
	for(i = 0; i < MAX_NUM_OF_SPLINETYPES; i++)
		if(m_arrPathArray[i].m_arr_PathData){
			delete[] m_arrPathArray[i].m_arr_PathData;
			m_arrPathArray[i].m_arr_PathData = nil;
		}
}

void
CCamera::FinishCutscene(void)
{
	SetPercentAlongCutScene(100.0f);
	m_fPositionAlongSpline = 1.0f;
	m_bcutsceneFinished = true;
}

uint32
CCamera::GetCutSceneFinishTime(void)
{
	int cam = ActiveCam;
	if (Cams[cam].Mode == CCam::MODE_FLYBY)
		return Cams[cam].m_uiFinishTime;
	cam = (cam + 1) % 2;
	if (Cams[cam].Mode == CCam::MODE_FLYBY)
		return Cams[cam].m_uiFinishTime;

	return 0;
}

void
CCamera::SetCamCutSceneOffSet(const CVector &pos)
{
	m_vecCutSceneOffset = pos;
};

void
CCamera::SetPercentAlongCutScene(float percent)
{
	if(Cams[ActiveCam].Mode == CCam::MODE_FLYBY)
		Cams[ActiveCam].m_fTimeElapsedFloat = percent/100.0f * Cams[ActiveCam].m_uiFinishTime;
	else if(Cams[(ActiveCam+1)%2].Mode == CCam::MODE_FLYBY)
		Cams[(ActiveCam+1)%2].m_fTimeElapsedFloat = percent/100.0f * Cams[(ActiveCam+1)%2].m_uiFinishTime;
}

void
CCamera::SetParametersForScriptInterpolation(float stopMoving, float catchUp, int32 time)
{
	m_fScriptPercentageInterToStopMoving = stopMoving * 0.01f;
	m_fScriptPercentageInterToCatchUp = catchUp * 0.01f;
	m_fScriptTimeForInterPolation = time;
	m_bScriptParametersSetForInterPol = true;
}

void
CCamera::SetZoomValueFollowPedScript(int16 dist)
{
	switch (dist) {
	case 0: m_fPedZoomValueScript = 0.25f; break;
	case 1: m_fPedZoomValueScript = 1.5f; break;
	case 2: m_fPedZoomValueScript = 2.9f; break;
	default: break;
	}

	m_bUseScriptZoomValuePed = true;
}

void
CCamera::SetZoomValueCamStringScript(int16 dist)
{
	if (Cams[ActiveCam].CamTargetEntity->IsVehicle()) {
		int vehApp = ((CVehicle*)Cams[ActiveCam].CamTargetEntity)->GetVehicleAppearance();
		int vehArrPos = 0;
		GetArrPosForVehicleType(vehApp, vehArrPos);

#ifdef FREE_CAM
		if (bFreeCam) {
			switch (dist) {
			case 0: m_fCarZoomValueScript = LCS_ZOOM_ONE_DISTANCE[vehArrPos]; break;
			case 1: m_fCarZoomValueScript = LCS_ZOOM_TWO_DISTANCE[vehArrPos]; break;
			case 2: m_fCarZoomValueScript = LCS_ZOOM_THREE_DISTANCE[vehArrPos]; break;
			default: break;
			}
		}
		else
#endif
		{
			switch (dist) {
			case 0: m_fCarZoomValueScript = ZOOM_ONE_DISTANCE[vehArrPos]; break;
			case 1: m_fCarZoomValueScript = ZOOM_TWO_DISTANCE[vehArrPos]; break;
			case 2: m_fCarZoomValueScript = ZOOM_THREE_DISTANCE[vehArrPos]; break;
			default: break;
			}
		}

		m_bUseScriptZoomValueCar = true;
	} else {
		switch (dist) {
		case 0: m_fPedZoomValueScript = 0.25f; break;
		case 1: m_fPedZoomValueScript = 1.5f; break;
		case 2: m_fPedZoomValueScript = 2.9f; break;
		default: break;
		}

		m_bUseScriptZoomValuePed = true;
	}
}

void
CCamera::SetNearClipScript(float clip)
{
	m_fNearClipScript = clip;
	m_bUseNearClipScript = true;
}



void
CCamera::ProcessFade(void)
{
	if(m_bFading){
		if(m_iFadingDirection == FADE_IN){
			if(m_fTimeToFadeOut != 0.0f)
				m_fFLOATingFade -= CTimer::GetTimeStepInSeconds() * 255.0f / m_fTimeToFadeOut;
			else
				m_fFLOATingFade = 0.0f;
			if (m_fFLOATingFade <= 0.0f) {
				m_bFading = false;
				m_fFLOATingFade = 0.0f;
			}
		}else if(m_iFadingDirection == FADE_OUT){
			if(m_fTimeToFadeOut != 0.0f)
				m_fFLOATingFade += CTimer::GetTimeStepInSeconds() * 255.0f / m_fTimeToFadeOut;
			else
				m_fFLOATingFade = 255.0f;
			if (m_fFLOATingFade >= 255.0f) {
				m_bFading = false;
				m_fFLOATingFade = 255.0f;
			}
		}
		CDraw::FadeValue = m_fFLOATingFade;
	}
}

void
CCamera::ProcessMusicFade(void)
{
	if(m_bMusicFading){
		if(m_iMusicFadingDirection == FADE_IN){
			if(m_fTimeToFadeMusic == 0.0f)
				m_fFLOATingFadeMusic = 0.0f;
			else
				m_fFLOATingFadeMusic -= 255.0f*CTimer::GetTimeStepInSeconds()/m_fTimeToFadeMusic;
			if(m_fFLOATingFadeMusic <= 0.0f){
				m_bMusicFading = false;
				m_fFLOATingFadeMusic = 0.0f;
			}
		}else if(m_iMusicFadingDirection == FADE_OUT){
			if(m_fTimeToFadeMusic == 0.0f)
				m_fFLOATingFadeMusic = 255.0f;
			else
				m_fFLOATingFadeMusic += 255.0f*CTimer::GetTimeStepInSeconds()/m_fTimeToFadeMusic;
			if(m_fFLOATingFadeMusic >= 255.0f){
				m_bMusicFading = false;
				m_fFLOATingFadeMusic = 255.0f;
			}
		}
		DMAudio.SetEffectsFadeVol(127 - m_fFLOATingFadeMusic/255.0f * 127);
		DMAudio.SetMusicFadeVol(127 - m_fFLOATingFadeMusic/255.0f * 127);
	}
}

void
CCamera::Fade(float timeout, int16 direction)
{
	m_bFading = true;
	m_iFadingDirection = direction;
	m_fTimeToFadeOut = timeout;
	m_uiFadeTimeStarted = CTimer::GetTimeInMilliseconds();
	if(!m_bIgnoreFadingStuffForMusic){
		m_bMusicFading = true;
		m_iMusicFadingDirection = direction;
		m_fTimeToFadeMusic = timeout;
		m_uiFadeTimeStartedMusic = CTimer::GetTimeInMilliseconds();
	}
}

void
CCamera::SetFadeColour(uint8 r, uint8 g, uint8 b)
{
	m_FadeTargetIsSplashScreen = r == 2 && g == 2 && b == 2;
	CDraw::FadeRed = r;
	CDraw::FadeGreen = g;
	CDraw::FadeBlue = b;
}

bool
CCamera::GetFading(void)
{
	return m_bFading;
}

int
CCamera::GetFadingDirection(void)
{
	if(m_bFading)
		return m_iFadingDirection == FADE_IN ? FADE_IN : FADE_OUT;
	else
		return FADE_NONE;
}

int
CCamera::GetScreenFadeStatus(void)
{
	if(m_fFLOATingFade == 0.0f)
		return FADE_0;
	if(m_fFLOATingFade == 255.0f)
		return FADE_2;
	return FADE_1;
}



void
CCamera::RenderMotionBlur(void)
{
	if(m_BlurType == 0)
		return;

	CMBlur::MotionBlurRender(m_pRwCamera,
		m_BlurRed, m_BlurGreen, m_BlurBlue,
		m_motionBlur, m_BlurType, m_imotionBlurAddAlpha);
}

void
CCamera::SetMotionBlur(int r, int g, int b, int a, int type)
{
	m_BlurRed = r;
	m_BlurGreen = g;
	m_BlurBlue = b;
	m_motionBlur = a;
	m_BlurType = type;
}

void
CCamera::SetMotionBlurAlpha(int a)
{
	m_imotionBlurAddAlpha = a;
}



int
CCamera::GetLookDirection(void)
{
	if(Cams[ActiveCam].Mode == CCam::MODE_CAM_ON_A_STRING ||
	   Cams[ActiveCam].Mode == CCam::MODE_1STPERSON ||
	   Cams[ActiveCam].Mode == CCam::MODE_BEHINDBOAT ||
	   Cams[ActiveCam].Mode == CCam::MODE_FOLLOWPED)
		return Cams[ActiveCam].DirectionWasLooking;
	return LOOKING_FORWARD;
}

bool
CCamera::GetLookingForwardFirstPerson(void)
{
	return Cams[ActiveCam].Mode == CCam::MODE_1STPERSON &&
		Cams[ActiveCam].DirectionWasLooking == LOOKING_FORWARD;
}

bool
CCamera::GetLookingLRBFirstPerson(void)
{
	return Cams[ActiveCam].Mode == CCam::MODE_1STPERSON && Cams[ActiveCam].DirectionWasLooking != LOOKING_FORWARD;
}

void
CCamera::SetCameraDirectlyBehindForFollowPed_CamOnAString(void)
{
	m_bCamDirectlyBehind = true;
	CPlayerPed *player = FindPlayerPed();
	if (player)
		m_PedOrientForBehindOrInFront = CGeneral::GetATanOfXY(player->GetForward().x, player->GetForward().y);
}

void
CCamera::SetCameraDirectlyInFrontForFollowPed_CamOnAString(void)
{
	m_bCamDirectlyInFront = true;
	CPlayerPed *player = FindPlayerPed();
	if (player)
		m_PedOrientForBehindOrInFront = CGeneral::GetATanOfXY(player->GetForward().x, player->GetForward().y);
}

void
CCamera::SetNewPlayerWeaponMode(int16 mode, int16 minZoom, int16 maxZoom)
{
	SetMotionBlur(CTimeCycle::GetBlurRed(), CTimeCycle::GetBlurGreen(), CTimeCycle::GetBlurBlue(), m_motionBlur, MOTION_BLUR_LIGHT_SCENE);
	PlayerWeaponMode.Mode = mode;
	PlayerWeaponMode.MaxZoom = maxZoom;
	PlayerWeaponMode.MinZoom = minZoom;
	PlayerWeaponMode.Duration = 0.0f;
}

void
CCamera::ClearPlayerWeaponMode(void)
{
	SetMotionBlur(CTimeCycle::GetBlurRed(), CTimeCycle::GetBlurGreen(), CTimeCycle::GetBlurBlue(), m_motionBlur, MOTION_BLUR_LIGHT_SCENE);
	PlayerWeaponMode.Mode = 0;
	PlayerWeaponMode.MaxZoom = 1;
	PlayerWeaponMode.MinZoom = -1;
	PlayerWeaponMode.Duration = 0.0f;
}

void
CCamera::UpdateAimingCoors(CVector const &coors)
{
	m_cvecAimingTargetCoors = coors;
}

bool
CCamera::Find3rdPersonCamTargetVector(float dist, CVector pos, CVector &source, CVector &target)
{
	if(CPad::GetPad(0)->GetLookBehindForPed()){
		source = pos;
		target = dist*Cams[ActiveCam].CamTargetEntity->GetForward() + source;
		return false;
	}else{
		float angleX = DEGTORAD((m_f3rdPersonCHairMultX-0.5f) * 1.8f * 0.5f * Cams[ActiveCam].FOV * CDraw::GetAspectRatio());
		float angleY = DEGTORAD((0.5f-m_f3rdPersonCHairMultY) * 1.8f * 0.5f * Cams[ActiveCam].FOV);
		source = Cams[ActiveCam].Source;
		target = Cams[ActiveCam].Front;
		target += Cams[ActiveCam].Up * Tan(angleY);
		target += CrossProduct(Cams[ActiveCam].Front, Cams[ActiveCam].Up) * Tan(angleX);
		target.Normalise();
		source += DotProduct(pos - source, target)*target;
		target = dist*target + source;
		return true;
	}
}

float
CCamera::Find3rdPersonQuickAimPitch(void)
{
	float clampedFrontZ = Clamp(Cams[ActiveCam].Front.z, -1.0f, 1.0f);

	float rot = Asin(clampedFrontZ);

	return -(DEGTORAD(((0.5f - m_f3rdPersonCHairMultY) * 1.8f * 0.5f * Cams[ActiveCam].FOV)) + rot);
}

bool
CCamera::Using1stPersonWeaponMode(void)
{
	switch(PlayerWeaponMode.Mode)
	case CCam::MODE_SNIPER:
	case CCam::MODE_M16_1STPERSON:
	case CCam::MODE_ROCKETLAUNCHER:
	case CCam::MODE_HELICANNON_1STPERSON:
	case CCam::MODE_CAMERA:
		return true;
	return false;
}


void
CCamera::SetRwCamera(RwCamera *cam)
{
	m_pRwCamera = cam;
	m_viewMatrix.Attach(RwCameraGetViewMatrix(m_pRwCamera), false);
	CMBlur::MotionBlurOpen(m_pRwCamera);
}

void
CCamera::CalculateDerivedValues(void)
{
	m_cameraMatrix = Invert(GetMatrix());

	float hfov = DEGTORAD(CDraw::GetScaledFOV()/2.0f);
	float c = Cos(hfov);
	float s = Sin(hfov);

	// right plane
	m_vecFrustumNormals[0] = CVector(c, -s, 0.0f);
	// left plane
	m_vecFrustumNormals[1] = CVector(-c, -s, 0.0f);

	CDraw::CalculateAspectRatio();
	c /= SCREEN_ASPECT_RATIO;
	s /= SCREEN_ASPECT_RATIO;
	// bottom plane
	m_vecFrustumNormals[2] = CVector(0.0f, -s, -c);
	// top plane
	m_vecFrustumNormals[3] = CVector(0.0f, -s, c);

	if(GetForward().x == 0.0f && GetForward().y == 0.0f)
		GetForward().x = 0.0001f;
	else
		Orientation = Atan2(GetForward().x, GetForward().y);

	CamFrontXNorm = GetForward().x;
	CamFrontYNorm = GetForward().y;
	float l = Sqrt(SQR(CamFrontXNorm) + SQR(CamFrontYNorm));
	if(l == 0.0f)
		CamFrontXNorm = 1.0f;
	else{
		CamFrontXNorm /= l;
		CamFrontYNorm /= l;
	}
}

bool
CCamera::IsPointVisible(const CVector &center, const CMatrix *mat)
{
#ifdef GTA_PS2
	CVuVector c;
	TransformPoint(c, *mat, center);
#else
	CVector c = center;
	#ifdef FIX_BUGS
		c = *mat * center;
	#else
		RwV3dTransformPoints(&c, &c, 1, (RwMatrix*)mat);
	#endif
#endif
	if(c.y < CDraw::GetNearClipZ()) return false;
	if(c.y > CDraw::GetFarClipZ()) return false;
	if(c.x*m_vecFrustumNormals[0].x + c.y*m_vecFrustumNormals[0].y > 0.0f) return false;
	if(c.x*m_vecFrustumNormals[1].x + c.y*m_vecFrustumNormals[1].y > 0.0f) return false;
	if(c.y*m_vecFrustumNormals[2].y + c.z*m_vecFrustumNormals[2].z > 0.0f) return false;
	if(c.y*m_vecFrustumNormals[3].y + c.z*m_vecFrustumNormals[3].z > 0.0f) return false;
	return true;
}

bool
CCamera::IsSphereVisible(const CVector &center, float radius, const CMatrix *mat)
{
#ifdef GTA_PS2
	CVuVector c;
	TransformPoint(c, *mat, center);
#else
	CVector c = center;
	#ifdef FIX_BUGS
		c = *mat * center;
	#else
		RwV3dTransformPoints(&c, &c, 1, (RwMatrix*)mat);
	#endif
#endif
	if(c.y + radius < CDraw::GetNearClipZ()) return false;
	if(c.y - radius > CDraw::GetFarClipZ()) return false;
	if(c.x*m_vecFrustumNormals[0].x + c.y*m_vecFrustumNormals[0].y > radius) return false;
	if(c.x*m_vecFrustumNormals[1].x + c.y*m_vecFrustumNormals[1].y > radius) return false;
	if(c.y*m_vecFrustumNormals[2].y + c.z*m_vecFrustumNormals[2].z > radius) return false;
	if(c.y*m_vecFrustumNormals[3].y + c.z*m_vecFrustumNormals[3].z > radius) return false;
	return true;
}

bool
CCamera::IsSphereVisible(const CVector &center, float radius)
{
	return IsSphereVisible(center, radius, &GetCameraMatrix());
}

bool
CCamera::IsBoxVisible(CVUVECTOR *box, const CMatrix *mat)
{
	int i;
	int frustumTests[6] = { 0 };
#ifdef GTA_PS2
	TransformPoints(box, 8, *mat, box);
#else
	#ifdef FIX_BUGS
		for (i = 0; i < 8; i++)
			box[i] = *mat * box[i];
	#else
		RwV3dTransformPoints(box, box, 8, (RwMatrix*)mat);
	#endif
#endif

	for(i = 0; i < 8; i++){
		if(box[i].y < CDraw::GetNearClipZ()) frustumTests[0]++;
		if(box[i].y > CDraw::GetFarClipZ()) frustumTests[1]++;
		if(box[i].x*m_vecFrustumNormals[0].x + box[i].y*m_vecFrustumNormals[0].y > 0.0f) frustumTests[2]++;
		if(box[i].x*m_vecFrustumNormals[1].x + box[i].y*m_vecFrustumNormals[1].y > 0.0f) frustumTests[3]++;
//	Why not test z?
//		if(box[i].y*m_vecFrustumNormals[2].y + box[i].z*m_vecFrustumNormals[2].z > 0.0f) frustumTests[4]++;
//		if(box[i].y*m_vecFrustumNormals[3].y + box[i].z*m_vecFrustumNormals[3].z > 0.0f) frustumTests[5]++;
	}
	for(i = 0; i < 6; i++)
		if(frustumTests[i] == 8)
			return false;		// Box is completely outside of one plane
	return true;
}



CCamPathSplines::CCamPathSplines(void)
{
	m_arr_PathData = nil;
}
