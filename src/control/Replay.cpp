#include "common.h"
#ifdef GTA_REPLAY
#include "AnimBlendAssocGroup.h"
#include "AnimBlendAssociation.h"
#include "Bike.h"
#include "Boat.h"
#include "SpecialFX.h"
#include "CarCtrl.h"
#include "CivilianPed.h"
#include "CopPed.h"
#include "Wanted.h"
#include "Clock.h"
#include "DMAudio.h"
#include "Draw.h"
#include "Explosion.h"
#include "FileMgr.h"
#include "Fire.h"
#include "Frontend.h"
#include "Garages.h"
#include "Heli.h"
#include "main.h"
#include "Matrix.h"
#include "ModelIndices.h"
#include "ModelInfo.h"
#include "Object.h"
#include "Pad.h"
#include "Particle.h"
#include "PedAttractor.h"
#include "Phones.h"
#include "Pickups.h"
#include "Plane.h"
#include "Pools.h"
#include "Population.h"
#include "Projectile.h"
#include "ProjectileInfo.h"
#include "Replay.h"
#include "References.h"
#include "Pools.h"
#include "RpAnimBlend.h"
#include "RwHelper.h"
#include "CutsceneMgr.h"
#include "Skidmarks.h"
#include "Stinger.h"
#include "Streaming.h"
#include "Timer.h"
#include "Train.h"
#include "Weather.h"
#include "Zones.h"
#include "Font.h"
#include "Text.h"
#include "Camera.h"
#include "Radar.h"
#include "Fluff.h"
#include "WaterCreatures.h"

uint8 CReplay::Mode;
CAddressInReplayBuffer CReplay::Record;
CAddressInReplayBuffer CReplay::Playback;
uint8 *CReplay::pBuf0;
CAutomobile *CReplay::pBuf1;
uint8 *CReplay::pBuf2;
CPlayerPed *CReplay::pBuf3;
uint8 *CReplay::pBuf4;
CCutsceneObject *CReplay::pBuf5;
uint8 *CReplay::pBuf6;
CPtrNode *CReplay::pBuf7;
uint8 *CReplay::pBuf8;
CEntryInfoNode *CReplay::pBuf9;
uint8 *CReplay::pBuf10;
CDummyPed *CReplay::pBuf11;
uint8 *CReplay::pRadarBlips;
uint8 *CReplay::pStoredCam;
uint8 *CReplay::pWorld1;
CReference *CReplay::pEmptyReferences;
CStoredDetailedAnimationState *CReplay::pPedAnims;
uint8 *CReplay::pPickups;
uint8 *CReplay::pReferences;
uint8 CReplay::BufferStatus[NUM_REPLAYBUFFERS];
uint8 CReplay::Buffers[NUM_REPLAYBUFFERS][REPLAYBUFFERSIZE];
bool CReplay::bPlayingBackFromFile;
bool CReplay::bReplayEnabled = true;
uint32 CReplay::SlowMotion;
uint32 CReplay::FramesActiveLookAroundCam;
bool CReplay::bDoLoadSceneWhenDone;
CPtrNode* CReplay::WorldPtrList;
CPtrNode* CReplay::BigBuildingPtrList;
CWanted CReplay::PlayerWanted;
CPlayerInfo CReplay::PlayerInfo;
uint32 CReplay::Time1;
uint32 CReplay::Time2;
uint32 CReplay::Time3;
uint32 CReplay::Time4;
uint32 CReplay::Frame;
uint8 CReplay::ClockHours;
uint8 CReplay::ClockMinutes;
uint16 CReplay::OldWeatherType;
uint16 CReplay::NewWeatherType;
float CReplay::WeatherInterpolationValue;
float CReplay::TimeStepNonClipped;
float CReplay::TimeStep;
float CReplay::TimeScale;
float CReplay::CameraFixedX;
float CReplay::CameraFixedY;
float CReplay::CameraFixedZ;
int32 CReplay::OldRadioStation;
int8 CReplay::CameraMode;
bool CReplay::bAllowLookAroundCam;
float CReplay::LoadSceneX;
float CReplay::LoadSceneY;
float CReplay::LoadSceneZ;
float CReplay::CameraFocusX;
float CReplay::CameraFocusY;
float CReplay::CameraFocusZ;
bool CReplay::bPlayerInRCBuggy;
float CReplay::fDistanceLookAroundCam;
float CReplay::fBetaAngleLookAroundCam;
float CReplay::fAlphaAngleLookAroundCam;
int CReplay::ms_nNumCivMale_Stored;
int CReplay::ms_nNumCivFemale_Stored;
int CReplay::ms_nNumCop_Stored;
int CReplay::ms_nNumEmergency_Stored;
int CReplay::ms_nNumGang1_Stored;
int CReplay::ms_nNumGang2_Stored;
int CReplay::ms_nNumGang3_Stored;
int CReplay::ms_nNumGang4_Stored;
int CReplay::ms_nNumGang5_Stored;
int CReplay::ms_nNumGang6_Stored;
int CReplay::ms_nNumGang7_Stored;
int CReplay::ms_nNumGang8_Stored;
int CReplay::ms_nNumGang9_Stored;
int CReplay::ms_nNumDummy_Stored;
int CReplay::ms_nTotalCarPassengerPeds_Stored;
int CReplay::ms_nTotalCivPeds_Stored;
int CReplay::ms_nTotalGangPeds_Stored;
int CReplay::ms_nTotalPeds_Stored;
int CReplay::ms_nTotalMissionPeds_Stored;
uint8* CReplay::pGarages;
CFire* CReplay::FireArray;
uint32 CReplay::NumOfFires;
uint8* CReplay::paProjectileInfo;
uint8* CReplay::paProjectiles;
uint8 CReplay::CurrArea;
#ifdef FIX_BUGS
int CReplay::nHandleOfPlayerPed[NUMPLAYERS];
#endif

static void(*CBArray[])(CAnimBlendAssociation*, void*) =
{
	nil, &CPed::PedGetupCB, &CPed::PedStaggerCB, &CPed::PedEvadeCB, &CPed::FinishDieAnimCB,
	&CPed::FinishedWaitCB, &CPed::FinishLaunchCB, &CPed::FinishHitHeadCB, &CPed::PedAnimGetInCB, &CPed::PedAnimDoorOpenCB,
	&CPed::PedAnimPullPedOutCB, &CPed::PedAnimDoorCloseCB, &CPed::PedSetInCarCB, &CPed::PedSetOutCarCB, &CPed::PedAnimAlignCB,
	&CPed::PedSetDraggedOutCarCB, &CPed::PedAnimStepOutCarCB, &CPed::PedSetInTrainCB,
#ifdef GTA_TRAIN
	&CPed::PedSetOutTrainCB,
#endif
	&CPed::FinishedAttackCB,
	&CPed::FinishFightMoveCB, &PhonePutDownCB, &PhonePickUpCB, &CPed::PedAnimDoorCloseRollingCB, &CPed::FinishJumpCB,
	&CPed::PedLandCB, &CPed::RestoreHeadingRateCB, &CPed::PedSetQuickDraggedOutCarPositionCB, &CPed::PedSetDraggedOutCarPositionCB,
	&CPed::PedSetPreviousStateCB, &CPed::FinishedReloadCB, &CPed::PedSetGetInCarPositionCB,
	&CPed::PedAnimShuffleCB, &CPed::DeleteSunbatheIdleAnimCB, &StartTalkingOnMobileCB, &FinishTalkingOnMobileCB
};

static uint8 FindCBFunctionID(void(*f)(CAnimBlendAssociation*, void*))
{
	for (int i = 0; i < sizeof(CBArray) / sizeof(*CBArray); i++){
		if (CBArray[i] == f)
			return i;
	}

	return 0;
}

static void(*FindCBFunction(uint8 id))(CAnimBlendAssociation*, void*)
{
	return CBArray[id];
}

static void ApplyPanelDamageToCar(uint32 panels, CAutomobile* vehicle, bool flying)
{
	if(vehicle->Damage.GetPanelStatus(VEHPANEL_FRONT_LEFT) != CDamageManager::GetPanelStatus(panels, VEHPANEL_FRONT_LEFT)){
		vehicle->Damage.SetPanelStatus(VEHPANEL_FRONT_LEFT, CDamageManager::GetPanelStatus(panels, VEHPANEL_FRONT_LEFT));
		vehicle->SetPanelDamage(CAR_WING_LF, VEHPANEL_FRONT_LEFT, flying);
	}
	if(vehicle->Damage.GetPanelStatus(VEHPANEL_FRONT_RIGHT) != CDamageManager::GetPanelStatus(panels, VEHPANEL_FRONT_RIGHT)){
		vehicle->Damage.SetPanelStatus(VEHPANEL_FRONT_RIGHT, CDamageManager::GetPanelStatus(panels, VEHPANEL_FRONT_RIGHT));
		vehicle->SetPanelDamage(CAR_WING_RF, VEHPANEL_FRONT_RIGHT, flying);
	}
	if(vehicle->Damage.GetPanelStatus(VEHPANEL_REAR_LEFT) != CDamageManager::GetPanelStatus(panels, VEHPANEL_REAR_LEFT)){
		vehicle->Damage.SetPanelStatus(VEHPANEL_REAR_LEFT, CDamageManager::GetPanelStatus(panels, VEHPANEL_REAR_LEFT));
		vehicle->SetPanelDamage(CAR_WING_LR, VEHPANEL_REAR_LEFT, flying);
	}
	if(vehicle->Damage.GetPanelStatus(VEHPANEL_REAR_RIGHT) != CDamageManager::GetPanelStatus(panels, VEHPANEL_REAR_RIGHT)){
		vehicle->Damage.SetPanelStatus(VEHPANEL_REAR_RIGHT, CDamageManager::GetPanelStatus(panels, VEHPANEL_REAR_RIGHT));
		vehicle->SetPanelDamage(CAR_WING_RR, VEHPANEL_REAR_RIGHT, flying);
	}
	if(vehicle->Damage.GetPanelStatus(VEHPANEL_WINDSCREEN) != CDamageManager::GetPanelStatus(panels, VEHPANEL_WINDSCREEN)){
		vehicle->Damage.SetPanelStatus(VEHPANEL_WINDSCREEN, CDamageManager::GetPanelStatus(panels, VEHPANEL_WINDSCREEN));
		vehicle->SetPanelDamage(CAR_WINDSCREEN, VEHPANEL_WINDSCREEN, flying);
	}
	if(vehicle->Damage.GetPanelStatus(VEHBUMPER_FRONT) != CDamageManager::GetPanelStatus(panels, VEHBUMPER_FRONT)){
		vehicle->Damage.SetPanelStatus(VEHBUMPER_FRONT, CDamageManager::GetPanelStatus(panels, VEHBUMPER_FRONT));
		vehicle->SetPanelDamage(CAR_BUMP_FRONT, VEHBUMPER_FRONT, flying);
	}
	if(vehicle->Damage.GetPanelStatus(VEHBUMPER_REAR) != CDamageManager::GetPanelStatus(panels, VEHBUMPER_REAR)){
		vehicle->Damage.SetPanelStatus(VEHBUMPER_REAR, CDamageManager::GetPanelStatus(panels, VEHBUMPER_REAR));
		vehicle->SetPanelDamage(CAR_BUMP_REAR, VEHBUMPER_REAR, flying);
	}
}

void PrintElementsInPtrList(void) 
{
	for (CPtrNode* node = CWorld::GetBigBuildingList(LEVEL_GENERIC).first; node; node = node->next) {
		/* Most likely debug print was present here */
	}
}

void CReplay::Init(void)
{
	pBuf0 = nil;
	pBuf1 = nil;
	pBuf2 = nil;
	pBuf3 = nil;
	pBuf4 = nil;
	pBuf5 = nil;
	pBuf6 = nil;
	pBuf7 = nil;
	pBuf8 = nil;
	pBuf9 = nil;
	pBuf10 = nil;
	pBuf11 = nil;
	pRadarBlips = nil;
	pStoredCam = nil;
	pWorld1 = nil;
	pEmptyReferences = nil;
	pPedAnims = nil;
	pPickups = nil;
	pReferences = nil;
	Mode = MODE_RECORD;
	Playback.m_nOffset = 0;
	Playback.m_pBase = nil;
	Playback.m_bSlot = 0;
	Record.m_nOffset = 0;
	Record.m_pBase = nil;
	Record.m_bSlot = 0;
	for (int i = 0; i < NUM_REPLAYBUFFERS; i++)
		BufferStatus[i] = REPLAYBUFFER_UNUSED;
	Record.m_bSlot = 0;
	Record.m_pBase = Buffers[0];
	BufferStatus[0] = REPLAYBUFFER_RECORD;
	Buffers[0][Record.m_nOffset] = REPLAYPACKET_END;
	bPlayingBackFromFile = false;
	bReplayEnabled = true;
	SlowMotion = 1;
	FramesActiveLookAroundCam = 0;
	bDoLoadSceneWhenDone = false;
	MarkEverythingAsNew();
}

void CReplay::DisableReplays(void)
{
	bReplayEnabled = false;
}

void CReplay::EnableReplays(void)
{
	bReplayEnabled = true;
}

void PlayReplayFromHD(void);
void CReplay::Update(void)
{
	if (CCutsceneMgr::IsCutsceneProcessing() || CPad::GetPad(0)->ArePlayerControlsDisabled() || CScriptPaths::IsOneActive() || FrontEndMenuManager.GetIsMenuActive()) {
		Init();
		return;
	}
	switch (Mode){
	case MODE_RECORD:
		RecordThisFrame();
		break;
	case MODE_PLAYBACK:
		PlaybackThisFrame();
		break;
	}
	if (CDraw::FadeValue || !bReplayEnabled)
		return;
	if (Mode == MODE_PLAYBACK){
		if (CPad::GetPad(0)->GetFJustDown(0))
			FinishPlayback();
	}
	else if (Mode == MODE_RECORD){
		if (CPad::GetPad(0)->GetFJustDown(0))
			TriggerPlayback(REPLAYCAMMODE_ASSTORED, 0.0f, 0.0f, 0.0f, false);
		if (CPad::GetPad(0)->GetFJustDown(1))
			SaveReplayToHD();
		if (CPad::GetPad(0)->GetFJustDown(2))
			PlayReplayFromHD();
#ifdef USE_BETA_REPLAY_MODE
		if (CPad::GetPad(0)->GetFJustDown(3))
			TriggerPlaybackLastCoupleOfSeconds(5000, REPLAYCAMMODE_TOPDOWN, 0.0f, 0.0f, 0.0f, 4);
#endif
	}
}

void CReplay::RecordThisFrame(void)
{
	uint32 memory_required = sizeof(tGeneralPacket) + sizeof(tClockPacket) + sizeof(tWeatherPacket) + sizeof(tTimerPacket) + sizeof(tMiscPacket);
	CVehiclePool* vehiclesT = CPools::GetVehiclePool();
	for (int i = 0; i < vehiclesT->GetSize(); i++) {
		CVehicle* v = vehiclesT->GetSlot(i);
		if (v && v->m_rwObject && v->GetModelIndex() != MI_AIRTRAIN && v->GetModelIndex() != MI_TRAIN) {
			if (v->IsBike())
				memory_required += sizeof(tBikeUpdatePacket);
			else
				memory_required += sizeof(tVehicleUpdatePacket);
		}
	}
	CPedPool* pedsT = CPools::GetPedPool();
	for (int i = 0; i < pedsT->GetSize(); i++) {
		CPed* p = pedsT->GetSlot(i);
		if (!p || !p->m_rwObject)
			continue;
		if (!p->bHasAlreadyBeenRecorded) {
			memory_required += sizeof(tPedHeaderPacket);
		}
		memory_required += sizeof(tPedUpdatePacket);
	}
	for (uint8 i = 0; i < NUMBULLETTRACES; i++) {
		if (!CBulletTraces::aTraces[i].m_bInUse)
			continue;
		memory_required += sizeof(tBulletTracePacket);
	}
	memory_required += sizeof(tEndOfFramePacket) + 1; // 1 for Record.m_pBase[Record.m_nOffset] = REPLAYPACKET_END;
	if (Record.m_nOffset + memory_required > REPLAYBUFFERSIZE - 16)
		GoToNextBlock();
	tGeneralPacket* general = (tGeneralPacket*)&Record.m_pBase[Record.m_nOffset];
	general->type = REPLAYPACKET_GENERAL;
	general->camera_pos.CopyOnlyMatrix(TheCamera.GetMatrix());
	general->player_pos = FindPlayerCoors();
	general->in_rcvehicle = CWorld::Players[CWorld::PlayerInFocus].m_pRemoteVehicle ? true : false;
	Record.m_nOffset += sizeof(*general);
	tClockPacket* clock = (tClockPacket*)&Record.m_pBase[Record.m_nOffset];
	clock->type = REPLAYPACKET_CLOCK;
	clock->hours = CClock::GetHours();
	clock->minutes = CClock::GetMinutes();
	Record.m_nOffset += sizeof(*clock);
	tWeatherPacket* weather = (tWeatherPacket*)&Record.m_pBase[Record.m_nOffset];
	weather->type = REPLAYPACKET_WEATHER;
	weather->old_weather = CWeather::OldWeatherType;
	weather->new_weather = CWeather::NewWeatherType;
	weather->interpolation = CWeather::InterpolationValue;
	Record.m_nOffset += sizeof(*weather);
	tTimerPacket* timer = (tTimerPacket*)&Record.m_pBase[Record.m_nOffset];
	timer->type = REPLAYPACKET_TIMER;
	timer->timer = CTimer::GetTimeInMilliseconds();
	Record.m_nOffset += sizeof(*timer);
	CVehiclePool* vehicles = CPools::GetVehiclePool();
	for (int i = 0; i < vehicles->GetSize(); i++){
		CVehicle* v = vehicles->GetSlot(i);
		if (v && v->m_rwObject && v->GetModelIndex() != MI_AIRTRAIN && v->GetModelIndex() != MI_TRAIN) {
			if (v->IsBike())
				StoreBikeUpdate(v, i);
			else
				StoreCarUpdate(v, i);
		}
	}
	CPedPool* peds = CPools::GetPedPool();
	for (int i = 0; i < peds->GetSize(); i++) {
		CPed* p = peds->GetSlot(i);
		if (!p || !p->m_rwObject)
			continue;
		if (!p->bHasAlreadyBeenRecorded){
			tPedHeaderPacket* ph = (tPedHeaderPacket*)&Record.m_pBase[Record.m_nOffset];
			ph->type = REPLAYPACKET_PED_HEADER;
			ph->index = i;
			ph->mi = p->GetModelIndex();
			ph->pedtype = p->m_nPedType;
			Record.m_nOffset += sizeof(*ph);
			p->bHasAlreadyBeenRecorded = true;
		}
		StorePedUpdate(p, i);
	}
	for (uint8 i = 0; i < NUMBULLETTRACES; i++){
		if (!CBulletTraces::aTraces[i].m_bInUse)
			continue;
		tBulletTracePacket* bt = (tBulletTracePacket*)&Record.m_pBase[Record.m_nOffset];
		bt->type = REPLAYPACKET_BULLET_TRACES;
		bt->index = i;
		bt->inf = CBulletTraces::aTraces[i].m_vecStartPos;
		bt->sup = CBulletTraces::aTraces[i].m_vecEndPos;
		Record.m_nOffset += sizeof(*bt);
	}
	tMiscPacket* misc = (tMiscPacket*)&Record.m_pBase[Record.m_nOffset];
	misc->type = REPLAYPACKET_MISC;
	misc->cam_shake_start = TheCamera.m_uiCamShakeStart;
	misc->cam_shake_strength = TheCamera.m_fCamShakeForce;
	misc->cur_area = CGame::currArea;
	misc->video_cam = CSpecialFX::bVideoCam;
	misc->lift_cam = CSpecialFX::bLiftCam;
	Record.m_nOffset += sizeof(*misc);
	tEndOfFramePacket* eof = (tEndOfFramePacket*)&Record.m_pBase[Record.m_nOffset];
	eof->type = REPLAYPACKET_ENDOFFRAME;
	Record.m_nOffset += sizeof(*eof);
	Record.m_pBase[Record.m_nOffset] = REPLAYPACKET_END;
}

void CReplay::GoToNextBlock(void)
{
	Record.m_pBase[Record.m_nOffset] = REPLAYPACKET_END;
	BufferStatus[Record.m_bSlot] = REPLAYBUFFER_PLAYBACK;
	Record.m_bSlot = (Record.m_bSlot + 1) % NUM_REPLAYBUFFERS;
	BufferStatus[Record.m_bSlot] = REPLAYBUFFER_RECORD;
	Record.m_pBase = Buffers[Record.m_bSlot];
	Record.m_nOffset = 0;
	*Record.m_pBase = REPLAYPACKET_END;
	MarkEverythingAsNew();
}

void CReplay::RecordParticle(tParticleType type, const CVector& vecPos, const CVector& vecDir, float fSize, const RwRGBA& color)
{
	if (Record.m_nOffset > REPLAYBUFFERSIZE - 16 - sizeof(tParticlePacket))
		GoToNextBlock();
	tParticlePacket* pp = (tParticlePacket*)&Record.m_pBase[Record.m_nOffset];
	pp->type = REPLAYPACKET_PARTICLE;
	pp->particle_type = type;
	pp->pos_x = 4.0f * vecPos.x;
	pp->pos_y = 4.0f * vecPos.y;
	pp->pos_z = 4.0f * vecPos.z;
	pp->dir_x = 120.0f * Clamp(vecDir.x, -1.0f, 1.0f);
	pp->dir_y = 120.0f * Clamp(vecDir.y, -1.0f, 1.0f);
	pp->dir_z = 120.0f * Clamp(vecDir.z, -1.0f, 1.0f);
	pp->size = fSize;
	pp->r = color.red;
	pp->g = color.green;
	pp->b = color.blue;
	pp->a = color.alpha;
	Record.m_nOffset += sizeof(tParticlePacket);
	Record.m_pBase[Record.m_nOffset] = REPLAYPACKET_END;
}

void CReplay::StorePedUpdate(CPed *ped, int id)
{
	tPedUpdatePacket* pp = (tPedUpdatePacket*)&Record.m_pBase[Record.m_nOffset];
	pp->type = REPLAYPACKET_PED_UPDATE;
	pp->index = id;
	pp->heading = 128.0f / PI * ped->m_fRotationCur;
	pp->matrix.CompressFromFullMatrix(ped->GetMatrix());
	pp->assoc_group_id = ped->m_animGroup;
	pp->is_visible = ped->bIsVisible;
	/* 	Would be more sane to use GetJustIndex(ped->m_pMyVehicle) in following assignment */
	if (ped->InVehicle())
		pp->vehicle_index = (CPools::GetVehiclePool()->GetIndex(ped->m_pMyVehicle) >> 8) + 1;
	else
		pp->vehicle_index = 0;
	pp->weapon_model = ped->m_wepModelID;
	StorePedAnimation(ped, &pp->anim_state);
	Record.m_nOffset += sizeof(tPedUpdatePacket);
}

void CReplay::StorePedAnimation(CPed *ped, CStoredAnimationState *state)
{
	CAnimBlendAssociation* second;
	float blend_amount;
	CAnimBlendAssociation* main = RpAnimBlendClumpGetMainAssociation((RpClump*)ped->m_rwObject, &second, &blend_amount);
	if (main){
		state->animId = main->animId;
		state->time = 255.0f / 4.0f * Clamp(main->currentTime, 0.0f, 4.0f);
		state->speed = 255.0f / 3.0f * Clamp(main->speed, 0.0f, 3.0f);
		state->groupId = main->groupId;
	}else{
		state->animId = 3;
		state->time = 0;
		state->speed = 85;
		state->groupId = 0;
	}
	if (second) {
		state->secAnimId = second->animId;
		state->secTime = 255.0f / 4.0f * Clamp(second->currentTime, 0.0f, 4.0f);
		state->secSpeed = 255.0f / 3.0f * Clamp(second->speed, 0.0f, 3.0f);
		state->blendAmount = 255.0f / 2.0f * Clamp(blend_amount, 0.0f, 2.0f);
		state->secGroupId = second->groupId;
	}else{
		state->secAnimId = 0;
		state->secTime = 0;
		state->secSpeed = 0;
		state->blendAmount = 0;
		state->secGroupId = 0;
	}
	CAnimBlendAssociation* partial = RpAnimBlendClumpGetMainPartialAssociation((RpClump*)ped->m_rwObject);
	if (partial) {
		state->partAnimId = partial->animId;
		state->partAnimTime = 255.0f / 4.0f * Clamp(partial->currentTime, 0.0f, 4.0f);
		state->partAnimSpeed = 255.0f / 3.0f * Clamp(partial->speed, 0.0f, 3.0f);
		state->partBlendAmount = 255.0f / 2.0f * Clamp(partial->blendAmount, 0.0f, 2.0f);
		state->partGroupId = partial->groupId;
	}else{
		state->partAnimId = 0;
		state->partAnimTime = 0;
		state->partAnimSpeed = 0;
		state->partBlendAmount = 0;
		state->partGroupId = 0;
	}
}

void CReplay::StoreDetailedPedAnimation(CPed *ped, CStoredDetailedAnimationState *state)
{
	for (int i = 0; i < NUM_MAIN_ANIMS_IN_REPLAY; i++){
		CAnimBlendAssociation* assoc = RpAnimBlendClumpGetMainAssociation_N((RpClump*)ped->m_rwObject, i);
		if (assoc){
			state->aAnimId[i] = assoc->animId;
			state->aCurTime[i] = 255.0f / 4.0f * Clamp(assoc->currentTime, 0.0f, 4.0f);
			state->aSpeed[i] = 255.0f / 3.0f * Clamp(assoc->speed, 0.0f, 3.0f);
			state->aBlendAmount[i] = 255.0f / 2.0f * Clamp(assoc->blendAmount, 0.0f, 2.0f);
			state->aBlendDelta[i] = 127.0f / 32.0f * Clamp(assoc->blendDelta, -16.0f, 16.0f);
			state->aFlags[i] = assoc->flags;
			state->aGroupId[i] = assoc->groupId;
			if (assoc->callbackType == CAnimBlendAssociation::CB_FINISH || assoc->callbackType == CAnimBlendAssociation::CB_DELETE) {
				state->aFunctionCallbackID[i] = FindCBFunctionID(assoc->callback);
				if (assoc->callbackType == CAnimBlendAssociation::CB_FINISH)
					state->aFunctionCallbackID[i] |= 0x80;
			}else{
				state->aFunctionCallbackID[i] = 0;
			}
		}else{
			state->aAnimId[i] = ANIM_STD_NUM;
			state->aCurTime[i] = 0;
			state->aSpeed[i] = 85;
			state->aFunctionCallbackID[i] = 0;
			state->aFlags[i] = 0;
			state->aGroupId[i] = 0;
		}
	}
	for (int i = 0; i < NUM_PARTIAL_ANIMS_IN_REPLAY; i++) {
		CAnimBlendAssociation* assoc = RpAnimBlendClumpGetMainPartialAssociation_N((RpClump*)ped->m_rwObject, i);
		if (assoc) {
			state->aAnimId2[i] = assoc->animId;
			state->aCurTime2[i] = 255.0f / 4.0f * Clamp(assoc->currentTime, 0.0f, 4.0f);
			state->aSpeed2[i] = 255.0f / 3.0f * Clamp(assoc->speed, 0.0f, 3.0f);
			state->aBlendAmount2[i] = 255.0f / 2.0f * Clamp(assoc->blendAmount, 0.0f, 2.0f);
			state->aBlendDelta2[i] = 127.0f / 16.0f * Clamp(assoc->blendDelta, -16.0f, 16.0f);
			state->aFlags2[i] = assoc->flags;
			state->aGroupId2[i] = assoc->groupId;
			if (assoc->callbackType == CAnimBlendAssociation::CB_FINISH || assoc->callbackType == CAnimBlendAssociation::CB_DELETE) {
				state->aFunctionCallbackID2[i] = FindCBFunctionID(assoc->callback);
				if (assoc->callbackType == CAnimBlendAssociation::CB_FINISH)
					state->aFunctionCallbackID2[i] |= 0x80;
			}else{
				state->aFunctionCallbackID2[i] = 0;
			}
		}
		else {
			state->aAnimId2[i] = ANIM_STD_NUM;
			state->aCurTime2[i] = 0;
			state->aSpeed2[i] = 85;
			state->aFunctionCallbackID2[i] = 0;
			state->aFlags2[i] = 0;
			state->aGroupId2[i] = 0;
		}
	}
}

void CReplay::ProcessPedUpdate(CPed *ped, float interpolation, CAddressInReplayBuffer *buffer)
{
	tPedUpdatePacket *pp = (tPedUpdatePacket*)&buffer->m_pBase[buffer->m_nOffset];
	if (!ped){
		printf("Replay:Ped wasn't there\n");
		buffer->m_nOffset += sizeof(tPedUpdatePacket);
		return;
	}
	ped->m_fRotationCur = pp->heading * PI / 128.0f;
	ped->m_fRotationDest = pp->heading * PI / 128.0f;
	CMatrix ped_matrix;
	pp->matrix.DecompressIntoFullMatrix(ped_matrix);
	ped->GetMatrix() = ped->GetMatrix() * CMatrix(1.0f - interpolation);
	ped->GetMatrix().GetPosition() *= (1.0f - interpolation);
	ped->GetMatrix() += CMatrix(interpolation) * ped_matrix;
	if (pp->vehicle_index) {
		ped->m_pMyVehicle = CPools::GetVehiclePool()->GetSlot(pp->vehicle_index - 1);
		ped->bInVehicle = true;
	}
	else {
		ped->m_pMyVehicle = nil;
		ped->bInVehicle = false;
	}
	if (pp->assoc_group_id != ped->m_animGroup) {
		ped->m_animGroup = (AssocGroupId)pp->assoc_group_id;
		if (ped == FindPlayerPed())
			((CPlayerPed*)ped)->ReApplyMoveAnims();
	}
	ped->bIsVisible = pp->is_visible;
	if (FramesActiveLookAroundCam && ped->m_nPedType == PEDTYPE_PLAYER1)
		ped->bIsVisible = true;
	RetrievePedAnimation(ped, &pp->anim_state);
	ped->RemoveWeaponModel(-1);
	if (pp->weapon_model != (uint16)-1) {
		if (CStreaming::HasModelLoaded(pp->weapon_model))
			ped->AddWeaponModel(pp->weapon_model);
		else
			CStreaming::RequestModel(pp->weapon_model, 0);
	}
	CWorld::Remove(ped);
	CWorld::Add(ped);
	buffer->m_nOffset += sizeof(tPedUpdatePacket);
}

bool HasAnimGroupLoaded(uint8 group)
{
	CAnimBlendAssocGroup* pGroup = &CAnimManager::GetAnimAssocGroups()[group];
	return pGroup->animBlock && pGroup->animBlock->isLoaded;
}

void CReplay::RetrievePedAnimation(CPed *ped, CStoredAnimationState *state)
{
	CAnimBlendAssociation* anim1;
	if (state->animId <= ANIM_STD_IDLE)
		anim1 = CAnimManager::BlendAnimation(
			(RpClump*)ped->m_rwObject, ped->m_animGroup, (AnimationId)state->animId, 100.0f);
	else if (HasAnimGroupLoaded(state->groupId))
		anim1 = CAnimManager::BlendAnimation((RpClump*)ped->m_rwObject, (AssocGroupId)state->groupId, (AnimationId)state->animId, 100.0f);
	else
		anim1 = CAnimManager::BlendAnimation((RpClump*)ped->m_rwObject, ASSOCGRP_STD, ANIM_STD_WALK, 100.0f);

	anim1->SetCurrentTime(state->time * 4.0f / 255.0f);
	anim1->speed = state->speed * 3.0f / 255.0f;
	anim1->SetBlend(1.0f, 1.0f);
	anim1->callbackType = CAnimBlendAssociation::CB_NONE;
	if (state->blendAmount && state->secAnimId){
		float time = state->secTime * 4.0f / 255.0f;
		float speed = state->secSpeed * 3.0f / 255.0f;
		float blend = state->blendAmount * 2.0f / 255.0f;
		CAnimBlendAssociation* anim2 = CAnimManager::BlendAnimation(
			(RpClump*)ped->m_rwObject,
			(state->secAnimId > ANIM_STD_IDLE) ? (AssocGroupId)state->secGroupId : ped->m_animGroup,
			(AnimationId)state->secAnimId, 100.0f);
		anim2->SetCurrentTime(time);
		anim2->speed = speed;
		anim2->SetBlend(blend, 1.0f);
		anim2->callbackType = CAnimBlendAssociation::CB_NONE;
	}
	RpAnimBlendClumpRemoveAssociations((RpClump*)ped->m_rwObject, 0x10);
	if (state->partAnimId){
		float time = state->partAnimTime * 4.0f / 255.0f;
		float speed = state->partAnimSpeed * 3.0f / 255.0f;
		float blend = state->partBlendAmount * 2.0f / 255.0f;
		if (blend > 0.0f && state->partAnimId != ANIM_STD_IDLE && HasAnimGroupLoaded(state->partGroupId)){
			CAnimBlendAssociation* anim3 = CAnimManager::BlendAnimation(
				(RpClump*)ped->m_rwObject, (AssocGroupId)state->partGroupId, (AnimationId)state->partAnimId, 1000.0f);
			anim3->SetCurrentTime(time);
			anim3->speed = speed;
			anim3->SetBlend(blend, 0.0f);
		}
	}
}

void CReplay::RetrieveDetailedPedAnimation(CPed *ped, CStoredDetailedAnimationState *state)
{
	CAnimBlendAssociation* assoc;
	for (int i = 0; ((assoc = RpAnimBlendClumpGetMainAssociation_N(ped->GetClump(), i))); i++)
		assoc->SetBlend(0.0f, -1.0f);
	for (int i = 0; ((assoc = RpAnimBlendClumpGetMainPartialAssociation_N(ped->GetClump(), i))); i++)
		assoc->SetBlend(0.0f, -1.0f);
	for (int i = 0; i < NUM_MAIN_ANIMS_IN_REPLAY; i++) {
		if (state->aAnimId[i] == ANIM_STD_NUM)
			continue;
		CAnimBlendAssociation* anim = CAnimManager::AddAnimation(ped->GetClump(),
			state->aAnimId[i] > ANIM_STD_IDLE ? (AssocGroupId)state->aGroupId[i] : ped->m_animGroup,
			(AnimationId)state->aAnimId[i]);
		anim->SetCurrentTime(state->aCurTime[i] * 4.0f / 255.0f);
		anim->speed = state->aSpeed[i] * 3.0f / 255.0f;
		anim->SetBlend(state->aBlendAmount[i] * 2.0f / 255.0f, state->aBlendDelta[i] * 16.0f / 127.0f);
		anim->flags = state->aFlags[i];
		uint8 callback = state->aFunctionCallbackID[i];
		if (!callback)
			anim->callbackType = CAnimBlendAssociation::CB_NONE;
		else if (callback & 0x80)
			anim->SetFinishCallback(FindCBFunction(callback & 0x7F), ped);
		else
			anim->SetDeleteCallback(FindCBFunction(callback & 0x7F), ped);
	}
	for (int i = 0; i < NUM_PARTIAL_ANIMS_IN_REPLAY; i++) {
		if (state->aAnimId2[i] == ANIM_STD_NUM)
			continue;
		CAnimBlendAssociation* anim = CAnimManager::AddAnimation(ped->GetClump(),
			state->aAnimId2[i] > ANIM_STD_IDLE ? (AssocGroupId)state->aGroupId2[i] : ped->m_animGroup,
			(AnimationId)state->aAnimId2[i]);
		anim->SetCurrentTime(state->aCurTime2[i] * 4.0f / 255.0f);
		anim->speed = state->aSpeed2[i] * 3.0f / 255.0f;
		anim->SetBlend(state->aBlendAmount2[i] * 2.0f / 255.0f, state->aBlendDelta2[i] * 16.0f / 127.0f);
		anim->flags = state->aFlags2[i];
		uint8 callback = state->aFunctionCallbackID2[i];
		if (!callback)
			anim->callbackType = CAnimBlendAssociation::CB_NONE;
		else if (callback & 0x80)
			anim->SetFinishCallback(FindCBFunction(callback & 0x7F), ped);
		else
			anim->SetDeleteCallback(FindCBFunction(callback & 0x7F), ped);
	}
}

void CReplay::PlaybackThisFrame(void)
{
	static int FrameSloMo = 0;
	CAddressInReplayBuffer buf = Playback;
	if (PlayBackThisFrameInterpolation(&buf, 1.0f, nil)){
		DMAudio.SetEffectsFadeVol(127);
		DMAudio.SetMusicFadeVol(127);
		return;
	}
	if (FrameSloMo){
		CAddressInReplayBuffer buf_sm = buf;
		if (PlayBackThisFrameInterpolation(&buf_sm, FrameSloMo * 1.0f / SlowMotion, nil)){
			DMAudio.SetEffectsFadeVol(127);
			DMAudio.SetMusicFadeVol(127);
			return;
		}
	}
	FrameSloMo = (FrameSloMo + 1) % SlowMotion;
	if (FrameSloMo == 0)
		Playback = buf;
	ProcessLookAroundCam();
	DMAudio.SetEffectsFadeVol(0);
	DMAudio.SetMusicFadeVol(0);
}

// next two functions are only found in mobile version
// most likely they were optimized out for being unused
void CReplay::TriggerPlaybackLastCoupleOfSeconds(uint32 start, uint8 cam_mode, float cam_x, float cam_y, float cam_z, uint32 slomo)
{
	if (Mode != MODE_RECORD)
		return;
	TriggerPlayback(cam_mode, cam_x, cam_y, cam_z, true);
	SlowMotion = slomo;
	bAllowLookAroundCam = false;
	if (!FastForwardToTime(CTimer::GetTimeInMilliseconds() - start))
		Mode = MODE_RECORD;
}

bool CReplay::FastForwardToTime(uint32 start)
{
	uint32 timer = 0;
	while (start > timer)
		if (PlayBackThisFrameInterpolation(&Playback, 1.0f, &timer))
			return false;
	return true;
}

void CReplay::StoreCarUpdate(CVehicle *vehicle, int id)
{
	tVehicleUpdatePacket* vp = (tVehicleUpdatePacket*)&Record.m_pBase[Record.m_nOffset];
	vp->type = REPLAYPACKET_VEHICLE;
	vp->index = id;
	vp->matrix.CompressFromFullMatrix(vehicle->GetMatrix());
	vp->health = vehicle->m_fHealth / 4.0f; /* Not anticipated that health can be > 1000. */
	vp->acceleration = vehicle->m_fGasPedal * 100.0f;
	vp->panels = vehicle->IsCar() ? ((CAutomobile*)vehicle)->Damage.m_panelStatus : 0;
	vp->velocityX = 8000.0f * Max(-4.0f, Min(4.0f, vehicle->GetMoveSpeed().x)); /* 8000!? */
	vp->velocityY = 8000.0f * Max(-4.0f, Min(4.0f, vehicle->GetMoveSpeed().y));
	vp->velocityZ = 8000.0f * Max(-4.0f, Min(4.0f, vehicle->GetMoveSpeed().z));
	vp->mi = vehicle->GetModelIndex();
	vp->primary_color = vehicle->m_currentColour1;
	vp->secondary_color = vehicle->m_currentColour2;
	if (vehicle->GetModelIndex() == MI_RHINO)
		vp->car_gun = 128.0f / PI * ((CAutomobile*)vehicle)->m_fCarGunLR;
	else
		vp->wheel_state = 50.0f * vehicle->m_fSteerAngle;
	if (vehicle->IsCar()){
		CAutomobile* car = (CAutomobile*)vehicle;
		for (int i = 0; i < 4; i++){
			vp->wheel_susp_dist[i] = 50.0f * car->m_aSuspensionSpringRatio[i];
			vp->wheel_rotation[i] = 128.0f / PI * car->m_aWheelRotation[i];
		}
		vp->door_angles[0] = 127.0f / PI * car->Doors[2].m_fAngle;
		vp->door_angles[1] = 127.0f / PI * car->Doors[3].m_fAngle;
		vp->door_status = 0;
		for (int i = 0; i < 6; i++){
			if (car->Damage.GetDoorStatus(i) == DOOR_STATUS_MISSING)
				vp->door_status |= BIT(i);
		}
	}
	if (vehicle->GetModelIndex() == MI_SKIMMER)
		vp->skimmer_speed = 50.0f * ((CBoat*)vehicle)->m_fMovingSpeed;
	vp->render_scorched = vehicle->bRenderScorched;
	vp->vehicle_type = vehicle->m_vehType;
	Record.m_nOffset += sizeof(tVehicleUpdatePacket);
}

void CReplay::StoreBikeUpdate(CVehicle* vehicle, int id)
{
	CBike* bike = (CBike*)vehicle;
	tBikeUpdatePacket* vp = (tBikeUpdatePacket*)&Record.m_pBase[Record.m_nOffset];
	vp->type = REPLAYPACKET_BIKE;
	vp->index = id;
	vp->matrix.CompressFromFullMatrix(vehicle->GetMatrix());
	vp->health = vehicle->m_fHealth / 4.0f; /* Not anticipated that health can be > 1000. */
	vp->acceleration = vehicle->m_fGasPedal * 100.0f;
#ifdef FIX_BUGS // originally it's undefined behaviour - different fields are copied on PC and mobile
	for (int i = 0; i < 2; i++)
		vp->wheel_rotation[i] = 128.0f / PI * bike->m_aWheelRotation[i];
	for (int i = 0; i < 2; i++)
		vp->wheel_rotation[i + 2] = 128.0f / PI * bike->m_aWheelSpeed[i];
	for (int i = 0; i < 4; i++)
		vp->wheel_susp_dist[i] = 50.0f * bike->m_aSuspensionSpringRatio[i];
#else
	for (int i = 0; i < 4; i++) {
		vp->wheel_susp_dist[i] = 50.0f * bike->m_aSuspensionSpringRatio[i];
		vp->wheel_rotation[i] = 128.0f / PI * bike->m_aWheelRotation[i];
	}
#endif
	vp->velocityX = 8000.0f * Max(-4.0f, Min(4.0f, vehicle->GetMoveSpeed().x)); /* 8000!? */
	vp->velocityY = 8000.0f * Max(-4.0f, Min(4.0f, vehicle->GetMoveSpeed().y));
	vp->velocityZ = 8000.0f * Max(-4.0f, Min(4.0f, vehicle->GetMoveSpeed().z));
	vp->mi = vehicle->GetModelIndex();
	vp->primary_color = vehicle->m_currentColour1;
	vp->secondary_color = vehicle->m_currentColour2;
	vp->wheel_state = 50.0f * vehicle->m_fSteerAngle;
	vp->lean_angle = 50.0f * bike->m_fLeanLRAngle;
	vp->wheel_angle = 50.0f * bike->m_fWheelAngle;
	Record.m_nOffset += sizeof(tBikeUpdatePacket);
}

void CReplay::ProcessCarUpdate(CVehicle *vehicle, float interpolation, CAddressInReplayBuffer *buffer)
{
	tVehicleUpdatePacket* vp = (tVehicleUpdatePacket*)&buffer->m_pBase[buffer->m_nOffset];
	if (!vehicle){
		printf("Replay:Car wasn't there");
		return;
	}
	CMatrix vehicle_matrix;
	vp->matrix.DecompressIntoFullMatrix(vehicle_matrix);
	vehicle->GetMatrix() = vehicle->GetMatrix() * CMatrix(1.0f - interpolation);
	vehicle->GetMatrix().GetPosition() *= (1.0f - interpolation);
	vehicle->GetMatrix() += CMatrix(interpolation) * vehicle_matrix;
	vehicle->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
	vehicle->m_fHealth = 4 * vp->health;
	vehicle->m_fGasPedal = vp->acceleration / 100.0f;
	if (vehicle->IsCar())
		ApplyPanelDamageToCar(vp->panels, (CAutomobile*)vehicle, true);
	vehicle->m_vecMoveSpeed = CVector(vp->velocityX / 8000.0f, vp->velocityY / 8000.0f, vp->velocityZ / 8000.0f);
	if (vehicle->GetModelIndex() == MI_RHINO) {
		((CAutomobile*)vehicle)->m_fCarGunLR = vp->car_gun * PI / 128.0f;
		vehicle->m_fSteerAngle = 0.0f;
	}else{
		vehicle->m_fSteerAngle = vp->wheel_state / 50.0f;
	}
	if (vehicle->IsCar()) {
		CAutomobile* car = (CAutomobile*)vehicle;
		for (int i = 0; i < 4; i++) {
			car->m_aSuspensionSpringRatio[i] = vp->wheel_susp_dist[i] / 50.0f;
			car->m_aWheelRotation[i] = vp->wheel_rotation[i] * PI / 128.0f;
		}
		car->Doors[DOOR_FRONT_LEFT].m_fAngle = car->Doors[DOOR_FRONT_LEFT].m_fPrevAngle = vp->door_angles[0] * PI / 127.0f;
		car->Doors[DOOR_FRONT_RIGHT].m_fAngle = car->Doors[DOOR_FRONT_RIGHT].m_fPrevAngle = vp->door_angles[1] * PI / 127.0f;
		if (vp->door_angles[0])
			car->Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_SWINGING);
		if (vp->door_angles[1])
			car->Damage.SetDoorStatus(DOOR_FRONT_RIGHT, DOOR_STATUS_SWINGING);
		if (vp->door_status & 1 && car->Damage.GetDoorStatus(DOOR_BONNET) != DOOR_STATUS_MISSING) {
			car->Damage.SetDoorStatus(DOOR_BONNET, DOOR_STATUS_MISSING);
			car->SetDoorDamage(CAR_BONNET, DOOR_BONNET, true);
		}
		if (vp->door_status & 2 && car->Damage.GetDoorStatus(DOOR_BOOT) != DOOR_STATUS_MISSING) {
			car->Damage.SetDoorStatus(DOOR_BOOT, DOOR_STATUS_MISSING);
			car->SetDoorDamage(CAR_BOOT, DOOR_BOOT, true);
		}
		if (vp->door_status & 4 && car->Damage.GetDoorStatus(DOOR_FRONT_LEFT) != DOOR_STATUS_MISSING) {
			car->Damage.SetDoorStatus(DOOR_FRONT_LEFT, DOOR_STATUS_MISSING);
			car->SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT, true);
		}
		if (vp->door_status & 8 && car->Damage.GetDoorStatus(DOOR_FRONT_RIGHT) != DOOR_STATUS_MISSING) {
			car->Damage.SetDoorStatus(DOOR_FRONT_RIGHT, DOOR_STATUS_MISSING);
			car->SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT, true);
		}
		if (vp->door_status & 0x10 && car->Damage.GetDoorStatus(DOOR_REAR_LEFT) != DOOR_STATUS_MISSING) {
			car->Damage.SetDoorStatus(DOOR_REAR_LEFT, DOOR_STATUS_MISSING);
			car->SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT, true);
		}
		if (vp->door_status & 0x20 && car->Damage.GetDoorStatus(DOOR_REAR_RIGHT) != DOOR_STATUS_MISSING) {
			car->Damage.SetDoorStatus(DOOR_REAR_RIGHT, DOOR_STATUS_MISSING);
			car->SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT, true);
		}
	}
	vehicle->bEngineOn = true;
	if (vehicle->IsCar())
		((CAutomobile*)vehicle)->m_nDriveWheelsOnGround = 4;
	CWorld::Remove(vehicle);
	CWorld::Add(vehicle);
	if (vehicle->IsBoat())
		((CBoat*)vehicle)->m_bIsAnchored = false;
	vehicle->bRenderScorched = vp->render_scorched;
	if (vehicle->GetModelIndex() == MI_SKIMMER)
		((CBoat*)vehicle)->m_fMovingSpeed = vp->skimmer_speed / 50.0f;
}

void CReplay::ProcessBikeUpdate(CVehicle* vehicle, float interpolation, CAddressInReplayBuffer* buffer)
{
	CBike* bike = (CBike*)vehicle;
	tBikeUpdatePacket* vp = (tBikeUpdatePacket*)&buffer->m_pBase[buffer->m_nOffset];
	if (!vehicle) {
		printf("Replay:Car wasn't there");
		return;
	}
	CMatrix vehicle_matrix;
	vp->matrix.DecompressIntoFullMatrix(vehicle_matrix);
	vehicle->GetMatrix() = vehicle->GetMatrix() * CMatrix(1.0f - interpolation);
	vehicle->GetMatrix().GetPosition() *= (1.0f - interpolation);
	vehicle->GetMatrix() += CMatrix(interpolation) * vehicle_matrix;
	vehicle->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
	vehicle->m_fHealth = 4 * vp->health;
	vehicle->m_fGasPedal = vp->acceleration / 100.0f;
	vehicle->m_vecMoveSpeed = CVector(vp->velocityX / 8000.0f, vp->velocityY / 8000.0f, vp->velocityZ / 8000.0f);
	vehicle->m_fSteerAngle = vp->wheel_state / 50.0f;
	vehicle->bEngineOn = true;
#ifdef FIX_BUGS
	for (int i = 0; i < 2; i++)
		bike->m_aWheelRotation[i] = vp->wheel_rotation[i] / (128.0f / PI);
	for (int i = 0; i < 2; i++)
		bike->m_aWheelSpeed[i] = vp->wheel_rotation[i + 2] / (128.0f / PI);
	for (int i = 0; i < 4; i++)
		bike->m_aSuspensionSpringRatio[i] = vp->wheel_susp_dist[i] / 50.0f;
#else
	for (int i = 0; i < 4; i++) {
		bike->m_aSuspensionSpringRatio[i] = vp->wheel_susp_dist[i] / 50.0f;
		bike->m_aWheelRotation[i] = vp->wheel_rotation[i] / (128.0f / PI);
	}
#endif
	bike->m_fLeanLRAngle = vp->lean_angle / 50.0f;
	bike->m_fWheelAngle = vp->wheel_angle / 50.0f;
	bike->bLeanMatrixClean = false;
	bike->CalculateLeanMatrix();
	CWorld::Remove(vehicle);
	CWorld::Add(vehicle);
}

bool CReplay::PlayBackThisFrameInterpolation(CAddressInReplayBuffer *buffer, float interpolation, uint32 *pTimer)
{
	CBulletTraces::Init();
	float split = 1.0f - interpolation;
	int ped_min_index = 0; /* Optimization due to peds and vehicles placed in buffer sequentially. */
	int vehicle_min_index = 0; /* So next ped can't have pool index less than current. */
	for(;;){
		uint8* ptr = buffer->m_pBase;
		uint32 offset = buffer->m_nOffset;
		uint8 type = ptr[offset];
		if (type == REPLAYPACKET_ENDOFFRAME)
			break;
		switch (type) {
		case REPLAYPACKET_END:
		{
			int slot = buffer->m_bSlot;
			if (BufferStatus[slot] == REPLAYBUFFER_RECORD) {
				FinishPlayback();
				return true;
			}
			buffer->m_bSlot = (slot + 1) % NUM_REPLAYBUFFERS;
			buffer->m_nOffset = 0;
			buffer->m_pBase = Buffers[buffer->m_bSlot];
			ped_min_index = 0;
			vehicle_min_index = 0;
			break;
		}
		case REPLAYPACKET_VEHICLE:
		{
			tVehicleUpdatePacket* vp = (tVehicleUpdatePacket*)&ptr[offset];
			for (int i = vehicle_min_index; i < vp->index; i++) {
				CVehicle* v = CPools::GetVehiclePool()->GetSlot(i);
				if (!v)
					continue;
				/* Removing vehicles not present in this frame. */
				CWorld::Remove(v);
				delete v;
			}
			vehicle_min_index = vp->index + 1;
			CVehicle* v = CPools::GetVehiclePool()->GetSlot(vp->index);
			CVehicle* new_v;
			if (!v) {
				int mi = vp->mi;
				if (CStreaming::ms_aInfoForModel[mi].m_loadState != 1) {
					CStreaming::RequestModel(mi, 0);
				}
				else {
					switch (vp->vehicle_type) {
					case VEHICLE_TYPE_CAR:
						new_v = new(vp->index << 8) CAutomobile(mi, 2);	
						break;
					case VEHICLE_TYPE_BOAT:
						new_v = new(vp->index << 8) CBoat(mi, 2);
						break;
					case VEHICLE_TYPE_TRAIN:
						new_v = new(vp->index << 8) CTrain(mi, 2);
						break;
					case VEHICLE_TYPE_HELI:
						new_v = new(vp->index << 8) CHeli(mi, 2);
						break;
					case VEHICLE_TYPE_PLANE:
						new_v = new(vp->index << 8) CPlane(mi, 2);
						break;
					case VEHICLE_TYPE_BIKE: // not possible
						new_v = new(vp->index << 8) CBike(mi, 2);
						break;
					}
					new_v->SetStatus(STATUS_PLAYER_PLAYBACKFROMBUFFER);
					vp->matrix.DecompressIntoFullMatrix(new_v->GetMatrix());
					new_v->m_currentColour1 = vp->primary_color;
					new_v->m_currentColour2 = vp->secondary_color;
					CWorld::Add(new_v);
				}
			}
			ProcessCarUpdate(CPools::GetVehiclePool()->GetSlot(vp->index), interpolation, buffer);
			buffer->m_nOffset += sizeof(tVehicleUpdatePacket);
			break;
		}
		case REPLAYPACKET_BIKE:
		{
			tBikeUpdatePacket* vp = (tBikeUpdatePacket*)&ptr[offset];
			for (int i = vehicle_min_index; i < vp->index; i++) {
				CVehicle* v = CPools::GetVehiclePool()->GetSlot(i);
				if (!v)
					continue;
				/* Removing vehicles not present in this frame. */
				CWorld::Remove(v);
				delete v;
			}
			vehicle_min_index = vp->index + 1;
			CVehicle* v = CPools::GetVehiclePool()->GetSlot(vp->index);
			CVehicle* new_v;
			if (!v) {
				int mi = vp->mi;
				if (CStreaming::ms_aInfoForModel[mi].m_loadState != 1) {
					CStreaming::RequestModel(mi, 0);
				}
				else {
					new_v = new(vp->index << 8) CBike(mi, 2);
					new_v->SetStatus(STATUS_PLAYER_PLAYBACKFROMBUFFER);
					vp->matrix.DecompressIntoFullMatrix(new_v->GetMatrix());
					new_v->m_currentColour1 = vp->primary_color;
					new_v->m_currentColour2 = vp->secondary_color;
					CWorld::Add(new_v);
				}
			}
			ProcessBikeUpdate(CPools::GetVehiclePool()->GetSlot(vp->index), interpolation, buffer);
			buffer->m_nOffset += sizeof(tBikeUpdatePacket);
			break;
		}
		case REPLAYPACKET_PED_HEADER:
		{
			tPedHeaderPacket* ph = (tPedHeaderPacket*)&ptr[offset];
			if (!CPools::GetPedPool()->GetSlot(ph->index)) {
				if (!CStreaming::HasModelLoaded(ph->mi) || (ph->mi >= MI_SPECIAL01 && ph->mi < MI_LAST_PED)) {
					CStreaming::RequestModel(ph->mi, 0);
				}
				else {
					CPed* new_p;
					if (ph->pedtype != PEDTYPE_PLAYER1)
						new_p = new(ph->index << 8) CCivilianPed((ePedType)ph->pedtype, ph->mi);
					else
						new_p = new(ph->index << 8) CPlayerPed();
					new_p->SetStatus(STATUS_PLAYER_PLAYBACKFROMBUFFER);
					new_p->GetMatrix().SetUnity();
					CWorld::Add(new_p);
				}
			}
			buffer->m_nOffset += sizeof(tPedHeaderPacket);
			break;
		}
		case REPLAYPACKET_PED_UPDATE:
		{
			tPedUpdatePacket* pu = (tPedUpdatePacket*)&ptr[offset];
			for (int i = ped_min_index; i < pu->index; i++) {
				CPed* p = CPools::GetPedPool()->GetSlot(i);
				if (!p)
					continue;
				/* Removing peds not present in this frame. */
				CWorld::Remove(p);
				delete p;
			}
			ped_min_index = pu->index + 1;
			ProcessPedUpdate(CPools::GetPedPool()->GetSlot(pu->index), interpolation, buffer);
			break;
		}
		case REPLAYPACKET_GENERAL:
		{
			tGeneralPacket* pg = (tGeneralPacket*)&ptr[offset];
			TheCamera.GetMatrix() = TheCamera.GetMatrix() * CMatrix(split);
			TheCamera.GetMatrix().GetPosition() *= split;
			TheCamera.GetMatrix() += CMatrix(interpolation) * pg->camera_pos;
			RwMatrix* pm = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));
			pm->pos = TheCamera.GetPosition();
			pm->at = TheCamera.GetForward();
			pm->up = TheCamera.GetUp();
			pm->right = TheCamera.GetRight();
			CameraFocusX = split * CameraFocusX + interpolation * pg->player_pos.x;
			CameraFocusY = split * CameraFocusY + interpolation * pg->player_pos.y;
			CameraFocusZ = split * CameraFocusZ + interpolation * pg->player_pos.z;
			bPlayerInRCBuggy = pg->in_rcvehicle;
			buffer->m_nOffset += sizeof(tGeneralPacket);
			break;
		}
		case REPLAYPACKET_CLOCK:
		{
			tClockPacket* pc = (tClockPacket*)&ptr[offset];
			CClock::SetGameClock(pc->hours, pc->minutes);
			buffer->m_nOffset += sizeof(tClockPacket);
			break;
		}
		case REPLAYPACKET_WEATHER:
		{
			tWeatherPacket* pw = (tWeatherPacket*)&ptr[offset];
			CWeather::OldWeatherType = pw->old_weather;
			CWeather::NewWeatherType = pw->new_weather;
			CWeather::InterpolationValue = pw->interpolation;
			buffer->m_nOffset += sizeof(tWeatherPacket);
			break;
		}
		case REPLAYPACKET_ENDOFFRAME:
		{
			/* Not supposed to be here. */
			assert(false);
			buffer->m_nOffset++;
			break;
		}
		case REPLAYPACKET_TIMER:
		{
			tTimerPacket* pt = (tTimerPacket*)&ptr[offset];
			if (pTimer)
				*pTimer = pt->timer;
			CTimer::SetTimeInMilliseconds(pt->timer);
			buffer->m_nOffset += sizeof(tTimerPacket);
			break;
		}
		case REPLAYPACKET_BULLET_TRACES:
		{
			tBulletTracePacket* pb = (tBulletTracePacket*)&ptr[offset];
			CBulletTraces::aTraces[pb->index].m_bInUse = true;
			CBulletTraces::aTraces[pb->index].m_vecStartPos = pb->inf;
			CBulletTraces::aTraces[pb->index].m_vecEndPos = pb->sup;
			buffer->m_nOffset += sizeof(tBulletTracePacket);
			break;
		}
		case REPLAYPACKET_PARTICLE:
		{
			tParticlePacket* pp = (tParticlePacket*)&ptr[offset];
			CVector pos(pp->pos_x / 4.0f, pp->pos_y / 4.0f, pp->pos_z / 4.0f);
			CVector dir(pp->dir_x / 120.0f, pp->dir_y / 120.0f, pp->dir_z / 120.0f);
			RwRGBA color;
			color.red = pp->r;
			color.green = pp->g;
			color.blue = pp->b;
			color.alpha = pp->a;
			CParticle::AddParticle((tParticleType)pp->particle_type, pos, dir, nil, pp->size, color);
			buffer->m_nOffset += sizeof(tParticlePacket);
			break;
		}
		case REPLAYPACKET_MISC:
		{
			tMiscPacket* pm = (tMiscPacket*)&ptr[offset];
			TheCamera.m_uiCamShakeStart = pm->cam_shake_start;
			TheCamera.m_fCamShakeForce = pm->cam_shake_strength;
			CSpecialFX::bVideoCam = pm->video_cam;
			CSpecialFX::bLiftCam = pm->lift_cam;
			CGame::currArea = pm->cur_area;
			buffer->m_nOffset += sizeof(tMiscPacket);
			break;
		}
		default:
			break;
		}
	}
	buffer->m_nOffset += 4;
	for (int i = vehicle_min_index; i < CPools::GetVehiclePool()->GetSize(); i++) {
		CVehicle* v = CPools::GetVehiclePool()->GetSlot(i);
		if (!v)
			continue;
		/* Removing vehicles not present in this frame. */
		CWorld::Remove(v);
		delete v;
	}
	for (int i = ped_min_index; i < CPools::GetPedPool()->GetSize(); i++) {
		CPed* p = CPools::GetPedPool()->GetSlot(i);
		if (!p)
			continue;
		/* Removing peds not present in this frame. */
		CWorld::Remove(p);
		delete p;
	}
	ProcessReplayCamera();
	return false;
}

void CReplay::FinishPlayback(void)
{
	if (Mode != MODE_PLAYBACK)
		return;
	EmptyAllPools();
	RestoreStuffFromMem();
	Mode = MODE_RECORD;
	if (bDoLoadSceneWhenDone){
		CVector v_ls(LoadSceneX, LoadSceneY, LoadSceneZ);
		CGame::currLevel = CTheZones::GetLevelFromPosition(&v_ls);
		CCollision::SortOutCollisionAfterLoad();
		CStreaming::LoadScene(v_ls);
	}
	bDoLoadSceneWhenDone = false;
	if (bPlayingBackFromFile){
		Init();
		MarkEverythingAsNew();
	}
	DMAudio.SetEffectsFadeVol(127);
	DMAudio.SetMusicFadeVol(127);
}

void CReplay::EmptyReplayBuffer(void)
{
	if (Mode == MODE_PLAYBACK)
		return;
	Record.m_nOffset = 0;
	for (int i = 0; i < NUM_REPLAYBUFFERS; i++){
		BufferStatus[i] = REPLAYBUFFER_UNUSED;
	}
	Record.m_bSlot = 0;
	Record.m_pBase = Buffers[0];
	BufferStatus[0] = REPLAYBUFFER_RECORD;
	Record.m_pBase[Record.m_nOffset] = 0;
	MarkEverythingAsNew();
}

void CReplay::ProcessReplayCamera(void)
{
	switch (CameraMode) {
	case REPLAYCAMMODE_TOPDOWN:
	{
		TheCamera.SetPosition(CameraFocusX, CameraFocusY, CameraFocusZ + 15.0f);
		TheCamera.GetForward() = CVector(0.0f, 0.0f, -1.0f);
		TheCamera.GetUp() = CVector(0.0f, 1.0f, 0.0f);
		TheCamera.GetRight() = CVector(1.0f, 0.0f, 0.0f);
		RwMatrix* pm = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));
		pm->pos = TheCamera.GetPosition();
		pm->at = TheCamera.GetForward();
		pm->up = TheCamera.GetUp();
		pm->right = TheCamera.GetRight();
		break;
	}
	case REPLAYCAMMODE_FIXED:
	{
		TheCamera.GetMatrix().GetPosition() = CVector(CameraFixedX, CameraFixedY, CameraFixedZ);
		CVector forward(CameraFocusX - CameraFixedX, CameraFocusY - CameraFixedY, CameraFocusZ - CameraFixedZ);
		forward.Normalise();
		CVector right = CrossProduct(CVector(0.0f, 0.0f, 1.0f), forward);
		right.Normalise();
		CVector up = CrossProduct(forward, right);
		up.Normalise();
		TheCamera.GetMatrix().GetForward() = forward;
		TheCamera.GetMatrix().GetUp() = up;
		TheCamera.GetMatrix().GetRight() = right;
		RwMatrix* pm = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));
		pm->pos = TheCamera.GetMatrix().GetPosition();
		pm->at = TheCamera.GetMatrix().GetForward();
		pm->up = TheCamera.GetMatrix().GetUp();
		pm->right = TheCamera.GetMatrix().GetRight();
		break;
	}
	default:
		break;
	}
	TheCamera.m_vecGameCamPos = TheCamera.GetMatrix().GetPosition();
	TheCamera.CalculateDerivedValues();
	RwMatrixUpdate(RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera)));
	RwFrameUpdateObjects(RwCameraGetFrame(TheCamera.m_pRwCamera));
}

extern CWeaponEffects gCrossHair;

void CReplay::TriggerPlayback(uint8 cam_mode, float cam_x, float cam_y, float cam_z, bool load_scene)
{
	if (Mode != MODE_RECORD)
		return;
	CameraFixedX = cam_x;
	CameraFixedY = cam_y;
	CameraFixedZ = cam_z;
	Mode = MODE_PLAYBACK;
	FramesActiveLookAroundCam = 0;
	CameraMode = cam_mode;
	bAllowLookAroundCam = true;
	bPlayingBackFromFile = false;
	OldRadioStation = DMAudio.GetRadioInCar();
	DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
	DMAudio.SetEffectsFadeVol(0);
	DMAudio.SetMusicFadeVol(0);
	CEscalators::Shutdown();
	CWaterCreatures::RemoveAll();
	int current;
	for (current = 0; current < NUM_REPLAYBUFFERS; current++)
		if (BufferStatus[current] == REPLAYBUFFER_RECORD)
			break;
	int first;
	for (first = (current + 1) % NUM_REPLAYBUFFERS; ; first = (first + 1) % NUM_REPLAYBUFFERS)
		if (BufferStatus[first] == REPLAYBUFFER_RECORD || BufferStatus[first] == REPLAYBUFFER_PLAYBACK)
			break;
	Playback.m_bSlot = first;
	Playback.m_nOffset = 0;
	Playback.m_pBase = Buffers[first];
	CObject::DeleteAllTempObjectsInArea(CVector(0.0f, 0.0f, 0.0f), 1000000.0f);
	StoreStuffInMem();
	EmptyPedsAndVehiclePools();
	SlowMotion = 1;
	CSkidmarks::Clear();
	StreamAllNecessaryCarsAndPeds();
	if (load_scene)
		bDoLoadSceneWhenDone = false;
	else{
		bDoLoadSceneWhenDone = true;
		LoadSceneX = TheCamera.GetPosition().x;
		LoadSceneY = TheCamera.GetPosition().y;
		LoadSceneZ = TheCamera.GetPosition().z;
		CVector ff_coord;
		FindFirstFocusCoordinate(&ff_coord);
		CGame::currLevel = CTheZones::GetLevelFromPosition(&ff_coord);
		CCollision::SortOutCollisionAfterLoad();
		CStreaming::LoadScene(ff_coord);
	}
	if (cam_mode == REPLAYCAMMODE_ASSTORED)
		TheCamera.CarZoomIndicator = CAM_ZOOM_CINEMATIC;
	gCrossHair.m_bActive = false;
	CExplosion::ClearAllExplosions();
	CPlaneBanners::Init();
#ifndef FIX_BUGS // this doesn't do anything useful and accesses destroyed player ped
	TheCamera.Restore();
#endif
	CDraw::SetFOV(70.0f);
}

void CReplay::StoreStuffInMem(void)
{
#ifdef FIX_BUGS
	for (int i = 0; i < NUMPLAYERS; i++)
		nHandleOfPlayerPed[i] = CPools::GetPedPool()->GetIndex(CWorld::Players[i].m_pPed);
#endif
	int i = CPools::GetPedPool()->GetSize();
	while (--i >= 0) {
		CPed* ped = CPools::GetPedPool()->GetSlot(i);
		if (!ped)
			continue;
		if (ped->m_attractor)
			GetPedAttractorManager()->DeRegisterPed(ped, ped->m_attractor);
	}
	CPools::GetVehiclePool()->Store(pBuf0, pBuf1);
	CPools::GetPedPool()->Store(pBuf2, pBuf3);
	CPools::GetObjectPool()->Store(pBuf4, pBuf5);
	CPools::GetPtrNodePool()->Store(pBuf6, pBuf7);
	CPools::GetEntryInfoNodePool()->Store(pBuf8, pBuf9);
	CPools::GetDummyPool()->Store(pBuf10, pBuf11);
	pWorld1 = new uint8[sizeof(CSector) * NUMSECTORS_X * NUMSECTORS_Y];
	memcpy(pWorld1, CWorld::GetSector(0, 0), NUMSECTORS_X * NUMSECTORS_Y * sizeof(CSector));
	WorldPtrList = CWorld::GetMovingEntityList().first; // why
	BigBuildingPtrList = CWorld::GetBigBuildingList(LEVEL_GENERIC).first;
	pPickups = new uint8[sizeof(CPickup) * NUMPICKUPS];
	memcpy(pPickups, CPickups::aPickUps, NUMPICKUPS * sizeof(CPickup));
	pReferences = new uint8[(sizeof(CReference) * NUMREFERENCES)];
	memcpy(pReferences, CReferences::aRefs, NUMREFERENCES * sizeof(CReference));
	pEmptyReferences = CReferences::pEmptyList;
	pStoredCam = new uint8[sizeof(CCamera)];
	memcpy(pStoredCam, &TheCamera, sizeof(CCamera));
	pRadarBlips = new uint8[sizeof(sRadarTrace) * NUMRADARBLIPS];
	memcpy(pRadarBlips, CRadar::ms_RadarTrace, NUMRADARBLIPS * sizeof(sRadarTrace));
	PlayerWanted = *FindPlayerPed()->m_pWanted;
	PlayerInfo = CWorld::Players[0];
	Time1 = CTimer::GetTimeInMilliseconds();
	Time2 = CTimer::GetTimeInMillisecondsNonClipped();
	Time3 = CTimer::GetPreviousTimeInMilliseconds();
	Time4 = CTimer::GetTimeInMillisecondsPauseMode();
	Frame = CTimer::GetFrameCounter();
	ClockHours = CClock::GetHours();
	ClockMinutes = CClock::GetMinutes();
	OldWeatherType = CWeather::OldWeatherType;
	NewWeatherType = CWeather::NewWeatherType;
	WeatherInterpolationValue = CWeather::InterpolationValue;
	CurrArea = CGame::currArea;
	TimeStepNonClipped = CTimer::GetTimeStepNonClipped();
	TimeStep = CTimer::GetTimeStep();
	TimeScale = CTimer::GetTimeScale();
	ms_nNumCivMale_Stored = CPopulation::ms_nNumCivMale;
	ms_nNumCivFemale_Stored = CPopulation::ms_nNumCivFemale;
	ms_nNumCop_Stored = CPopulation::ms_nNumCop;
	ms_nNumEmergency_Stored = CPopulation::ms_nNumEmergency;
	ms_nNumGang1_Stored = CPopulation::ms_nNumGang1;
	ms_nNumGang2_Stored = CPopulation::ms_nNumGang2;
	ms_nNumGang3_Stored = CPopulation::ms_nNumGang3;
	ms_nNumGang4_Stored = CPopulation::ms_nNumGang4;
	ms_nNumGang5_Stored = CPopulation::ms_nNumGang5;
	ms_nNumGang6_Stored = CPopulation::ms_nNumGang6;
	ms_nNumGang7_Stored = CPopulation::ms_nNumGang7;
	ms_nNumGang8_Stored = CPopulation::ms_nNumGang8;
	ms_nNumGang9_Stored = CPopulation::ms_nNumGang9;
	ms_nNumDummy_Stored = CPopulation::ms_nNumDummy;
	ms_nTotalCivPeds_Stored = CPopulation::ms_nTotalCivPeds;
	ms_nTotalGangPeds_Stored = CPopulation::ms_nTotalGangPeds;
	ms_nTotalPeds_Stored = CPopulation::ms_nTotalPeds;
	ms_nTotalMissionPeds_Stored = CPopulation::ms_nTotalMissionPeds;
	int size = CPools::GetPedPool()->GetSize();
	pPedAnims = new CStoredDetailedAnimationState[size];
	for (int i = 0; i < size; i++) {
		CPed* ped = CPools::GetPedPool()->GetSlot(i);
		if (ped)
			StoreDetailedPedAnimation(ped, &pPedAnims[i]);
	}
	pGarages = new uint8[sizeof(CGarages::aGarages)];
	memcpy(pGarages, CGarages::aGarages, sizeof(CGarages::aGarages));
	FireArray = new CFire[NUM_FIRES];
	memcpy(FireArray, gFireManager.m_aFires, sizeof(gFireManager.m_aFires));
	NumOfFires = gFireManager.m_nTotalFires;
	paProjectileInfo = new uint8[sizeof(gaProjectileInfo)];
	memcpy(paProjectileInfo, gaProjectileInfo, sizeof(gaProjectileInfo));
	paProjectiles = new uint8[sizeof(CProjectileInfo::ms_apProjectile)];
	memcpy(paProjectiles, CProjectileInfo::ms_apProjectile, sizeof(CProjectileInfo::ms_apProjectile));
	CScriptPaths::Save_ForReplay();
}

void CReplay::RestoreStuffFromMem(void)
{
	CPools::GetVehiclePool()->CopyBack(pBuf0, pBuf1);
	CPools::GetPedPool()->CopyBack(pBuf2, pBuf3);
	CPools::GetObjectPool()->CopyBack(pBuf4, pBuf5);
	CPools::GetPtrNodePool()->CopyBack(pBuf6, pBuf7);
	CPools::GetEntryInfoNodePool()->CopyBack(pBuf8, pBuf9);
	CPools::GetDummyPool()->CopyBack(pBuf10, pBuf11);
	memcpy(CWorld::GetSector(0, 0), pWorld1, sizeof(CSector) * NUMSECTORS_X * NUMSECTORS_Y);
	delete[] pWorld1;
	pWorld1 = nil;
	CWorld::GetMovingEntityList().first = WorldPtrList;
	CWorld::GetBigBuildingList(LEVEL_GENERIC).first = BigBuildingPtrList;
	memcpy(CPickups::aPickUps, pPickups, sizeof(CPickup) * NUMPICKUPS);
	delete[] pPickups;
	pPickups = nil;
	memcpy(CReferences::aRefs, pReferences, sizeof(CReference) * NUMREFERENCES);
	delete[] pReferences;
	pReferences = nil;
	CReferences::pEmptyList = pEmptyReferences;
	pEmptyReferences = nil;
	memcpy(&TheCamera, pStoredCam, sizeof(CCamera));
	delete[] pStoredCam;
	pStoredCam = nil;
	memcpy(CRadar::ms_RadarTrace, pRadarBlips, sizeof(sRadarTrace) * NUMRADARBLIPS);
	delete[] pRadarBlips;
	pRadarBlips = nil;
#ifdef FIX_BUGS
	for (int i = 0; i < NUMPLAYERS; i++) {
		CPlayerPed* pPlayerPed = (CPlayerPed*)CPools::GetPedPool()->GetAt(nHandleOfPlayerPed[i]);
		assert(pPlayerPed);
		CWorld::Players[i].m_pPed = pPlayerPed;
		pPlayerPed->RegisterReference((CEntity**)&CWorld::Players[i].m_pPed);
	}
#endif
	FindPlayerPed()->m_pWanted = new CWanted(PlayerWanted);
	CWorld::Players[0] = PlayerInfo;
	int i = CPools::GetPedPool()->GetSize();
	while (--i >= 0) {
		CPed* ped = CPools::GetPedPool()->GetSlot(i);
		if (!ped)
			continue;
		int mi = ped->GetModelIndex();
		CStreaming::RequestModel(mi, 0);
		CStreaming::LoadAllRequestedModels(false);
		ped->m_rwObject = nil;
		ped->m_modelIndex = -1;
		ped->SetModelIndex(mi);
		ped->m_pVehicleAnim = nil;
		ped->m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, ped);
		DMAudio.SetEntityStatus(ped->m_audioEntityId, TRUE);
		CPopulation::UpdatePedCount((ePedType)ped->m_nPedType, false);
		for (int j = 0; j < TOTAL_WEAPON_SLOTS; j++) {
			int mi1 = CWeaponInfo::GetWeaponInfo(ped->m_weapons[j].m_eWeaponType)->m_nModelId;
			if (mi1 != -1)
				CStreaming::RequestModel(mi1, STREAMFLAGS_DEPENDENCY);
			int mi2 = CWeaponInfo::GetWeaponInfo(ped->m_weapons[j].m_eWeaponType)->m_nModel2Id;
			if (mi2 != -1)
				CStreaming::RequestModel(mi2, STREAMFLAGS_DEPENDENCY);
			CStreaming::LoadAllRequestedModels(false);
			ped->m_weapons[j].Initialise(ped->m_weapons[j].m_eWeaponType, ped->m_weapons[j].m_nAmmoTotal);
		}
		if (ped->m_wepModelID >= 0) {
			ped->m_pWeaponModel = nil;
			if (ped->IsPlayer())
				((CPlayerPed*)ped)->m_pMinigunTopAtomic = nil;
			ped->AddWeaponModel(ped->m_wepModelID);
		}
		if (ped->m_nPedType == PEDTYPE_COP)
			((CCopPed*)ped)->m_pStinger = new CStinger;
	}
	i = CPools::GetVehiclePool()->GetSize();
	while (--i >= 0) {
		CVehicle* vehicle = CPools::GetVehiclePool()->GetSlot(i);
		if (!vehicle)
			continue;
		int mi = vehicle->GetModelIndex();
		CStreaming::RequestModel(mi, 0);
		CStreaming::LoadAllRequestedModels(false);
		vehicle->m_rwObject = nil;
		vehicle->m_modelIndex = -1;
		vehicle->SetModelIndex(mi);
		if (vehicle->IsCar()) {
			CAutomobile* car = (CAutomobile*)vehicle;
			if (mi == MI_DODO) {
				RpAtomicSetFlags((RpAtomic*)GetFirstObject(car->m_aCarNodes[CAR_WHEEL_LF]), 0);
				CMatrix tmp1;
				tmp1.Attach(RwFrameGetMatrix(car->m_aCarNodes[CAR_WHEEL_RF]), false);
				CMatrix tmp2(RwFrameGetMatrix(car->m_aCarNodes[CAR_WHEEL_LF]), false);
				tmp1.GetPosition() += CVector(tmp2.GetPosition().x + 0.1f, 0.0f, tmp2.GetPosition().z);
				tmp1.UpdateRW();
			}
			else if (mi == MI_HUNTER) {
				RpAtomicSetFlags((RpAtomic*)GetFirstObject(car->m_aCarNodes[CAR_WHEEL_LB]), 0);
				RpAtomicSetFlags((RpAtomic*)GetFirstObject(car->m_aCarNodes[CAR_WHEEL_RB]), 0);
			}
			else if (vehicle->IsRealHeli()) {
				RpAtomicSetFlags((RpAtomic*)GetFirstObject(car->m_aCarNodes[CAR_WHEEL_LF]), 0);
				RpAtomicSetFlags((RpAtomic*)GetFirstObject(car->m_aCarNodes[CAR_WHEEL_RF]), 0);
				RpAtomicSetFlags((RpAtomic*)GetFirstObject(car->m_aCarNodes[CAR_WHEEL_LB]), 0);
				RpAtomicSetFlags((RpAtomic*)GetFirstObject(car->m_aCarNodes[CAR_WHEEL_RB]), 0);
			}
		}
		if (vehicle->IsCar()){
			CAutomobile* car = (CAutomobile*)vehicle;
			int32 panels = car->Damage.m_panelStatus;
			car->Damage.m_panelStatus = 0;
			ApplyPanelDamageToCar(panels, car, true);
			car->SetDoorDamage(CAR_BONNET, DOOR_BONNET, true);
			car->SetDoorDamage(CAR_BOOT, DOOR_BOOT, true);
			car->SetDoorDamage(CAR_DOOR_LF, DOOR_FRONT_LEFT, true);
			car->SetDoorDamage(CAR_DOOR_RF, DOOR_FRONT_RIGHT, true);
			car->SetDoorDamage(CAR_DOOR_LR, DOOR_REAR_LEFT, true);
			car->SetDoorDamage(CAR_DOOR_RR, DOOR_REAR_RIGHT, true);
		}
		vehicle->m_audioEntityId = DMAudio.CreateEntity(AUDIOTYPE_PHYSICAL, vehicle);
		DMAudio.SetEntityStatus(vehicle->m_audioEntityId, TRUE);
		CCarCtrl::UpdateCarCount(vehicle, false);
		if ((mi == MI_AIRTRAIN || mi == MI_DEADDODO) && vehicle->m_rwObject){
			CVehicleModelInfo* info = (CVehicleModelInfo*)CModelInfo::GetModelInfo(mi);
			if (RwObjectGetType(vehicle->m_rwObject) == rpATOMIC){
				vehicle->GetMatrix().Detach();
				if (vehicle->m_rwObject){
					if (RwObjectGetType(vehicle->m_rwObject) == rpATOMIC){
						RwFrame* frame = RpAtomicGetFrame((RpAtomic*)vehicle->m_rwObject);
						RpAtomicDestroy((RpAtomic*)vehicle->m_rwObject);
						RwFrameDestroy(frame);
					}
					vehicle->m_rwObject = nil;
				}
			}else{
				vehicle->DeleteRwObject();
				int model_id = info->m_wheelId;
				if (model_id != -1){
					if ((vehicle->m_rwObject = CModelInfo::GetModelInfo(model_id)->CreateInstance())){
						vehicle->GetMatrix().AttachRW(RwFrameGetMatrix(RpClumpGetFrame((RpClump*)vehicle->m_rwObject)), false);
					}
				}
			}
		}
	}
	PrintElementsInPtrList();
	i = CPools::GetObjectPool()->GetSize();
	while (--i >= 0) {
		CObject* object = CPools::GetObjectPool()->GetSlot(i);
		if (!object)
			continue;
		int mi = object->GetModelIndex();
		object->m_rwObject = nil;
		object->m_modelIndex = -1;
		object->SetModelIndexNoCreate(mi);
		object->GetMatrix().m_attachment = nil;
	}
	i = CPools::GetDummyPool()->GetSize();
	while (--i >= 0) {
		CDummy* dummy = CPools::GetDummyPool()->GetSlot(i);
		if (!dummy)
			continue;
		int mi = dummy->GetModelIndex();
		dummy->m_rwObject = nil;
		dummy->m_modelIndex = -1;
		dummy->SetModelIndexNoCreate(mi);
		dummy->GetMatrix().m_attachment = nil;
	}
	++ClockMinutes;
	CTimer::SetTimeInMilliseconds(Time1);
	CTimer::SetTimeInMillisecondsNonClipped(Time2);
	CTimer::SetPreviousTimeInMilliseconds(Time3);
	CTimer::SetTimeInMillisecondsPauseMode(Time4);
	CTimer::SetTimeScale(TimeScale);
	CTimer::SetFrameCounter(Frame);
	CTimer::SetTimeStep(TimeStep);
	CTimer::SetTimeStepNonClipped(TimeStepNonClipped);
	CClock::SetGameClock(ClockHours, ClockMinutes);
	CWeather::OldWeatherType = OldWeatherType;
	CWeather::NewWeatherType = NewWeatherType;
	CWeather::InterpolationValue = WeatherInterpolationValue;
	CGame::currArea = CurrArea;
	CPopulation::ms_nNumCivMale = ms_nNumCivMale_Stored;
	CPopulation::ms_nNumCivFemale = ms_nNumCivFemale_Stored;
	CPopulation::ms_nNumCop = ms_nNumCop_Stored;
	CPopulation::ms_nNumEmergency = ms_nNumEmergency_Stored;
	CPopulation::ms_nNumGang1 = ms_nNumGang1_Stored;
	CPopulation::ms_nNumGang2 = ms_nNumGang2_Stored;
	CPopulation::ms_nNumGang3 = ms_nNumGang3_Stored;
	CPopulation::ms_nNumGang4 = ms_nNumGang4_Stored;
	CPopulation::ms_nNumGang5 = ms_nNumGang5_Stored;
	CPopulation::ms_nNumGang6 = ms_nNumGang6_Stored;
	CPopulation::ms_nNumGang7 = ms_nNumGang7_Stored;
	CPopulation::ms_nNumGang8 = ms_nNumGang8_Stored;
	CPopulation::ms_nNumGang9 = ms_nNumGang9_Stored;
	CPopulation::ms_nNumDummy = ms_nNumDummy_Stored;
	CPopulation::ms_nTotalCivPeds = ms_nTotalCivPeds_Stored;
	CPopulation::ms_nTotalGangPeds = ms_nTotalGangPeds_Stored;
	CPopulation::ms_nTotalPeds = ms_nTotalPeds_Stored;
	CPopulation::ms_nTotalMissionPeds = ms_nTotalMissionPeds_Stored;
	for (int i = 0; i < CPools::GetPedPool()->GetSize(); i++) {
		CPed* ped = CPools::GetPedPool()->GetSlot(i);
		if (!ped)
			continue;
		RetrieveDetailedPedAnimation(ped, &pPedAnims[i]);
	}
	delete[] pPedAnims;
	pPedAnims = nil;
	memcpy(CGarages::aGarages, pGarages, sizeof(CGarages::aGarages));
	delete[] pGarages;
	pGarages = nil;
	memcpy(gFireManager.m_aFires, FireArray, sizeof(gFireManager.m_aFires));
	delete[] FireArray;
	FireArray = nil;
	gFireManager.m_nTotalFires = NumOfFires;
	memcpy(gaProjectileInfo, paProjectileInfo, sizeof(gaProjectileInfo));
	delete[] paProjectileInfo;
	paProjectileInfo = nil;
	memcpy(CProjectileInfo::ms_apProjectile, paProjectiles, sizeof(CProjectileInfo::ms_apProjectile));
	delete[] paProjectiles;
	paProjectiles = nil;
	CScriptPaths::Load_ForReplay();
	CExplosion::ClearAllExplosions();
	DMAudio.ChangeMusicMode(MUSICMODE_FRONTEND);
	DMAudio.SetRadioInCar(OldRadioStation);
	DMAudio.ChangeMusicMode(MUSICMODE_GAME);
}

void CReplay::EmptyPedsAndVehiclePools(void)
{
	int i = CPools::GetVehiclePool()->GetSize();
	while (--i >= 0) {
		CVehicle* v = CPools::GetVehiclePool()->GetSlot(i);
		if (!v)
			continue;
		CWorld::Remove(v);
		delete v;
	}
	i = CPools::GetPedPool()->GetSize();
	while (--i >= 0) {
		CPed* p = CPools::GetPedPool()->GetSlot(i);
		if (!p)
			continue;
		CWorld::Remove(p);
		delete p;
	}
}

void CReplay::EmptyAllPools(void)
{
	EmptyPedsAndVehiclePools();
	int i = CPools::GetObjectPool()->GetSize();
	while (--i >= 0) {
		CObject* o = CPools::GetObjectPool()->GetSlot(i);
		if (!o)
			continue;
		CWorld::Remove(o);
		delete o;
	}
	i = CPools::GetDummyPool()->GetSize();
	while (--i >= 0) {
		CDummy* d = CPools::GetDummyPool()->GetSlot(i);
		if (!d)
			continue;
		CWorld::Remove(d);
		delete d;
	}
}

void CReplay::MarkEverythingAsNew(void)
{
	int i = CPools::GetVehiclePool()->GetSize();
	while (--i >= 0) {
		CVehicle* v = CPools::GetVehiclePool()->GetSlot(i);
		if (!v)
			continue;
		v->bHasAlreadyBeenRecorded = false;
	}
	i = CPools::GetPedPool()->GetSize();
	while (--i >= 0) {
		CPed* p = CPools::GetPedPool()->GetSlot(i);
		if (!p)
			continue;
		p->bHasAlreadyBeenRecorded = false;
	}
}

void CReplay::SaveReplayToHD(void)
{
	CFileMgr::SetDirMyDocuments();
	int fw = CFileMgr::OpenFileForWriting("replay.rep");
#ifdef FIX_REPLAY_BUGS
	if (fw == 0) {
#else
	if (fw < 0){ // BUG?
#endif
		printf("Couldn't open replay.rep for writing");
		CFileMgr::SetDir("");
		return;
	}
	CFileMgr::Write(fw, "gta3_7f", sizeof("gta3_7f"));
	int current;
	for (current = 0; current < NUM_REPLAYBUFFERS; current++)
		if (BufferStatus[current] == REPLAYBUFFER_RECORD)
			break;
	int first;
	for (first = (current + 1) % NUM_REPLAYBUFFERS; ; first = (first + 1) % NUM_REPLAYBUFFERS)
		if (BufferStatus[first] == REPLAYBUFFER_RECORD || BufferStatus[first] == REPLAYBUFFER_PLAYBACK)
			break;
	for(int i = first; ; i = (i + 1) % NUM_REPLAYBUFFERS){
		CFileMgr::Write(fw, (char*)Buffers[i], sizeof(Buffers[i]));
		if (BufferStatus[i] == REPLAYBUFFER_RECORD)
			break;
	}
	CFileMgr::CloseFile(fw);
	CFileMgr::SetDir("");
}

void CReplay::PlayReplayFromHD(void)
{
	CFileMgr::SetDirMyDocuments();
	int fr = CFileMgr::OpenFile("replay.rep", "rb");
	if (fr == 0) {
		printf("Couldn't open replay.rep for reading");
#ifdef FIX_REPLAY_BUGS
	CFileMgr::SetDir("");
#endif
		return;
	}
	CFileMgr::Read(fr, gString, 8);
	if (strncmp(gString, "gta3_7f", sizeof("gta3_7f"))){
		CFileMgr::CloseFile(fr);
		printf("Wrong file type for replay");
		CFileMgr::SetDir("");
		return;
	}
	int slot;
	for (slot = 0; CFileMgr::Read(fr, (char*)Buffers[slot], sizeof(Buffers[slot])); slot++)
		BufferStatus[slot] = REPLAYBUFFER_PLAYBACK;
	BufferStatus[slot - 1] = REPLAYBUFFER_RECORD;
	while (slot < NUM_REPLAYBUFFERS)
		BufferStatus[slot++] = REPLAYBUFFER_UNUSED;
	CFileMgr::CloseFile(fr);
	CFileMgr::SetDir("");
	TriggerPlayback(REPLAYCAMMODE_ASSTORED, 0.0f, 0.0f, 0.0f, false);
	bPlayingBackFromFile = true;
	bAllowLookAroundCam = true;
	StreamAllNecessaryCarsAndPeds();
}

void CReplay::StreamAllNecessaryCarsAndPeds(void)
{
	for (int slot = 0; slot < NUM_REPLAYBUFFERS; slot++) {
		if (BufferStatus[slot] == REPLAYBUFFER_UNUSED)
			continue;
		for (size_t offset = 0; Buffers[slot][offset] != REPLAYPACKET_END; offset += FindSizeOfPacket(Buffers[slot][offset])) {
			switch (Buffers[slot][offset]) {
			case REPLAYPACKET_VEHICLE:
				CStreaming::RequestModel(((tVehicleUpdatePacket*)&Buffers[slot][offset])->mi, 0);
				break;
			case REPLAYPACKET_BIKE:
				CStreaming::RequestModel(((tBikeUpdatePacket*)&Buffers[slot][offset])->mi, 0);
				break;
			case REPLAYPACKET_PED_HEADER:
				CStreaming::RequestModel(((tPedHeaderPacket*)&Buffers[slot][offset])->mi, 0);
				break;
			default:
				break;
			}
		}
	}
	CStreaming::LoadAllRequestedModels(false);
}

void CReplay::FindFirstFocusCoordinate(CVector *coord)
{
	*coord = CVector(0.0f, 0.0f, 0.0f);
	for (int slot = 0; slot < NUM_REPLAYBUFFERS; slot++) {
		if (BufferStatus[slot] == REPLAYBUFFER_UNUSED)
			continue;
		for (size_t offset = 0; Buffers[slot][offset] != REPLAYPACKET_END; offset += FindSizeOfPacket(Buffers[slot][offset])) {
			if (Buffers[slot][offset] == REPLAYPACKET_GENERAL) {
				*coord = ((tGeneralPacket*)&Buffers[slot][offset])->player_pos;
				return;
			}
		}
	}
}

bool CReplay::ShouldStandardCameraBeProcessed(void)
{
	if (Mode != MODE_PLAYBACK)
		return true;
	if (FramesActiveLookAroundCam || bPlayerInRCBuggy)
		return false;
	return FindPlayerVehicle() != nil;
}

void CReplay::ProcessLookAroundCam(void)
{
	if (!bAllowLookAroundCam)
		return;
	float x_moved = CPad::NewMouseControllerState.x / 200.0f;
	float y_moved = CPad::NewMouseControllerState.y / 200.0f;
	if (x_moved > 0.01f || y_moved > 0.01f) {
		if (FramesActiveLookAroundCam == 0)
			fDistanceLookAroundCam = 9.0f;
		FramesActiveLookAroundCam = 60;
	}
	if (bPlayerInRCBuggy)
		FramesActiveLookAroundCam = 0;
	if (!FramesActiveLookAroundCam)
		return;
	--FramesActiveLookAroundCam;
	fBetaAngleLookAroundCam += x_moved;
	if (CPad::NewMouseControllerState.LMB && CPad::NewMouseControllerState.RMB)
		fDistanceLookAroundCam = Max(3.0f, Min(15.0f, fDistanceLookAroundCam + 2.0f * y_moved));
	else
		fAlphaAngleLookAroundCam = Max(0.1f, Min(1.5f, fAlphaAngleLookAroundCam + y_moved));
	CVector camera_pt(
		fDistanceLookAroundCam * Sin(fBetaAngleLookAroundCam) * Cos(fAlphaAngleLookAroundCam),
		fDistanceLookAroundCam * Cos(fBetaAngleLookAroundCam) * Cos(fAlphaAngleLookAroundCam),
		fDistanceLookAroundCam * Sin(fAlphaAngleLookAroundCam)
	);
	CVector focus = CVector(CameraFocusX, CameraFocusY, CameraFocusZ);
	camera_pt += focus;
	CColPoint cp;
	CEntity* pe = nil;
	if (CWorld::ProcessLineOfSight(focus, camera_pt, cp, pe, true, false, false, false, false, true, true)){
		camera_pt = cp.point;
		CVector direction = focus - cp.point;
		direction.Normalise();
		camera_pt += direction / 4.0f;
	}
	CVector forward = focus - camera_pt;
	forward.Normalise();
	CVector right = CrossProduct(CVector(0.0f, 0.0f, 1.0f), forward);
	right.Normalise();
	CVector up = CrossProduct(forward, right);
	up.Normalise();
	TheCamera.GetForward() = forward;
	TheCamera.GetUp() = up;
	TheCamera.GetRight() = right;
	TheCamera.SetPosition(camera_pt);
	RwMatrix* pm = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));
	pm->pos = TheCamera.GetPosition();
	pm->at = TheCamera.GetForward();
	pm->up = TheCamera.GetUp();
	pm->right = TheCamera.GetRight();
	TheCamera.CalculateDerivedValues();
	RwMatrixUpdate(RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera)));
	RwFrameUpdateObjects(RwCameraGetFrame(TheCamera.m_pRwCamera));
}

size_t CReplay::FindSizeOfPacket(uint8 type)
{
	switch (type) {
	case REPLAYPACKET_END:			return 4;
	case REPLAYPACKET_VEHICLE:		return sizeof(tVehicleUpdatePacket);
	case REPLAYPACKET_BIKE:			return sizeof(tBikeUpdatePacket);
	case REPLAYPACKET_PED_HEADER:	return sizeof(tPedHeaderPacket);
	case REPLAYPACKET_PED_UPDATE:	return sizeof(tPedUpdatePacket);
	case REPLAYPACKET_GENERAL:		return sizeof(tGeneralPacket);
	case REPLAYPACKET_CLOCK:		return sizeof(tClockPacket);
	case REPLAYPACKET_WEATHER:		return sizeof(tWeatherPacket);
	case REPLAYPACKET_ENDOFFRAME:	return 4;
	case REPLAYPACKET_TIMER:		return sizeof(tTimerPacket);
	case REPLAYPACKET_BULLET_TRACES:return sizeof(tBulletTracePacket);
	case REPLAYPACKET_PARTICLE:		return sizeof(tParticlePacket);
	case REPLAYPACKET_MISC:			return sizeof(tMiscPacket);
	default: assert(false); break;
	}
	return 0;
}

void CReplay::Display()
{
	static int TimeCount = 0;
	if (Mode == MODE_RECORD)
		return;
	TimeCount = (TimeCount + 1) % UINT16_MAX;
	if ((TimeCount & 0x20) == 0)
		return;
	
	CFont::SetScale(SCREEN_SCALE_X(1.5f), SCREEN_SCALE_Y(1.5f));
	CFont::SetJustifyOff();
	CFont::SetBackgroundOff();
#ifdef FIX_BUGS
	CFont::SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-20));
#else
	CFont::SetCentreSize(SCREEN_WIDTH-20);
#endif
	CFont::SetCentreOff();
	CFont::SetPropOn();
	CFont::SetColor(CRGBA(255, 255, 200, 200));
	CFont::SetFontStyle(FONT_STANDARD);
	if (Mode == MODE_PLAYBACK)
		CFont::PrintString(SCREEN_WIDTH/15, SCREEN_HEIGHT/10, TheText.Get("REPLAY"));
}
#endif
