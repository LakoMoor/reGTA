#include "common.h"

#include "DMAudio.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "AudioScriptObject.h"
#include "sampman.h"

cDMAudio DMAudio;

void
cDMAudio::Initialise(void)
{
	AudioManager.Initialise();
}

void
cDMAudio::Terminate(void)
{
	AudioManager.Terminate();
}

void
cDMAudio::Service(void)
{
	AudioManager.Service();
}

int32
cDMAudio::CreateEntity(eAudioType type, void *UID)
{
	return AudioManager.CreateEntity(type, (CPhysical *)UID);
}

void
cDMAudio::DestroyEntity(int32 audioEntity)
{
	AudioManager.DestroyEntity(audioEntity);
}

bool8
cDMAudio::GetEntityStatus(int32 audioEntity)
{
	return AudioManager.GetEntityStatus(audioEntity);
}

void
cDMAudio::SetEntityStatus(int32 audioEntity, bool8 status)
{
	AudioManager.SetEntityStatus(audioEntity, status);
}

void
cDMAudio::PlayOneShot(int32 audioEntity, uint16 oneShot, float volume)
{
	AudioManager.PlayOneShot(audioEntity, oneShot, volume);
}

void
cDMAudio::DestroyAllGameCreatedEntities(void)
{
	AudioManager.DestroyAllGameCreatedEntities();
}

void
cDMAudio::SetOutputMode(bool8 surround)
{
	AudioManager.SetOutputMode(surround);
}

void
cDMAudio::SetMP3BoostVolume(uint8 volume)
{
	uint8 vol = volume;
	if (vol > MAX_VOLUME) vol = MAX_VOLUME;

	AudioManager.SetMP3BoostVolume(vol);
}

void
cDMAudio::SetEffectsMasterVolume(uint8 volume)
{
	uint8 vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	AudioManager.SetEffectsMasterVolume(vol);
}

void
cDMAudio::SetMusicMasterVolume(uint8 volume)
{
	uint8 vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	AudioManager.SetMusicMasterVolume(vol);
}

void
cDMAudio::SetEffectsFadeVol(uint8 volume)
{
	uint8 vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	AudioManager.SetEffectsFadeVol(vol);
}

void
cDMAudio::SetMusicFadeVol(uint8 volume)
{
	uint8 vol = volume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	AudioManager.SetMusicFadeVol(vol);
}

uint8
cDMAudio::GetNum3DProvidersAvailable(void)
{
	return AudioManager.GetNum3DProvidersAvailable();
}

char *
cDMAudio::Get3DProviderName(uint8 id)
{
	return AudioManager.Get3DProviderName(id);
}

int8 cDMAudio::AutoDetect3DProviders(void)
{
	return AudioManager.AutoDetect3DProviders();
}

int8
cDMAudio::GetCurrent3DProviderIndex(void)
{
	return AudioManager.GetCurrent3DProviderIndex();
}

int8
cDMAudio::SetCurrent3DProvider(uint8 which)
{
	return AudioManager.SetCurrent3DProvider(which);
}

void
cDMAudio::SetSpeakerConfig(int32 config)
{
	AudioManager.SetSpeakerConfig(config);
}

bool8
cDMAudio::IsMP3RadioChannelAvailable(void)
{
	return AudioManager.IsMP3RadioChannelAvailable();
}

void
cDMAudio::ReleaseDigitalHandle(void)
{
	AudioManager.ReleaseDigitalHandle();
}

void
cDMAudio::ReacquireDigitalHandle(void)
{
	AudioManager.ReacquireDigitalHandle();
}

void
cDMAudio::SetDynamicAcousticModelingStatus(bool8 status)
{
#ifdef AUDIO_REFLECTIONS
	AudioManager.SetDynamicAcousticModelingStatus(status);
#endif
}

bool8
cDMAudio::CheckForAnAudioFileOnCD(void)
{
	return AudioManager.CheckForAnAudioFileOnCD();
}

char
cDMAudio::GetCDAudioDriveLetter(void)
{
	return AudioManager.GetCDAudioDriveLetter();
}

bool8
cDMAudio::IsAudioInitialised(void)
{
	return AudioManager.IsAudioInitialised();
}

void
cDMAudio::ResetPoliceRadio()
{
	AudioManager.ResetPoliceRadio();
}

void
cDMAudio::ReportCrime(eCrimeType crime, const CVector &pos)
{
	AudioManager.ReportCrime(crime, pos);
}

int32
cDMAudio::CreateLoopingScriptObject(cAudioScriptObject *scriptObject)
{
	int32 audioEntity = AudioManager.CreateEntity(AUDIOTYPE_SCRIPTOBJECT, scriptObject);

	if ( AEHANDLE_IS_OK(audioEntity) )
		AudioManager.SetEntityStatus(audioEntity, TRUE);
	
	return audioEntity;
}

void
cDMAudio::DestroyLoopingScriptObject(int32 audioEntity)
{
	AudioManager.DestroyEntity(audioEntity);
}

void
cDMAudio::CreateOneShotScriptObject(cAudioScriptObject *scriptObject)
{
	int32 audioEntity = AudioManager.CreateEntity(AUDIOTYPE_SCRIPTOBJECT, scriptObject);

	if ( AEHANDLE_IS_OK(audioEntity) )
	{
		AudioManager.SetEntityStatus(audioEntity, TRUE);
		AudioManager.PlayOneShot(audioEntity, scriptObject->AudioId, 0.0f);
	}
}

void
cDMAudio::PlaySuspectLastSeen(float x, float y, float z)
{
	AudioManager.PlaySuspectLastSeen(x, y, z);
}

void
cDMAudio::ReportCollision(CEntity *entityA, CEntity *entityB, uint8 surfaceTypeA, uint8 surfaceTypeB, float collisionPower, float velocity)
{
	AudioManager.ReportCollision(entityA, entityB, surfaceTypeA, surfaceTypeB, collisionPower, velocity);
}

void
cDMAudio::PlayFrontEndSound(uint16 frontend, uint32 volume)
{
	AudioManager.PlayOneShot(AudioManager.m_nFrontEndEntity, frontend, (float)volume);
}

void
cDMAudio::PlayRadioAnnouncement(uint32 announcement)
{
	MusicManager.PlayAnnouncement(announcement);
}

void
cDMAudio::PlayFrontEndTrack(uint32 track, bool8 frontendFlag)
{
	MusicManager.PlayFrontEndTrack(track, frontendFlag);
}

void
cDMAudio::StopFrontEndTrack(void)
{
	MusicManager.StopFrontEndTrack();
}

void
cDMAudio::ResetTimers(uint32 time)
{
	AudioManager.ResetTimers(time);
}

void
cDMAudio::ChangeMusicMode(uint8 mode)
{
	MusicManager.ChangeMusicMode(mode);
}

void
cDMAudio::PreloadCutSceneMusic(uint32 track)
{
	MusicManager.PreloadCutSceneMusic(track);
}

void
cDMAudio::PlayPreloadedCutSceneMusic(void)
{
	MusicManager.PlayPreloadedCutSceneMusic();
}

void
cDMAudio::StopCutSceneMusic(void)
{
	MusicManager.StopCutSceneMusic();
}

void
cDMAudio::PreloadMissionAudio(uint8 slot, Const char *missionAudio)
{
	AudioManager.PreloadMissionAudio(slot, missionAudio);
}

uint8
cDMAudio::GetMissionAudioLoadingStatus(uint8 slot)
{
	return AudioManager.GetMissionAudioLoadingStatus(slot);
}

void
cDMAudio::SetMissionAudioLocation(uint8 slot, float x, float y, float z)
{
	AudioManager.SetMissionAudioLocation(slot, x, y, z);
}

void
cDMAudio::PlayLoadedMissionAudio(uint8 slot)
{
	AudioManager.PlayLoadedMissionAudio(slot);
}

bool8
cDMAudio::IsMissionAudioSamplePlaying(uint8 slot)
{
	return AudioManager.IsMissionAudioSamplePlaying(slot);
}

bool8
cDMAudio::IsMissionAudioSampleFinished(uint8 slot)
{
	return AudioManager.IsMissionAudioSampleFinished(slot);
}

void
cDMAudio::ClearMissionAudio(uint8 slot)
{
	AudioManager.ClearMissionAudio(slot);
}

const char *
cDMAudio::GetMissionAudioLoadedLabel(uint8 slot)
{
	return AudioManager.GetMissionAudioLoadedLabel(slot);
}

uint8
cDMAudio::GetRadioInCar(void)
{
	return MusicManager.GetRadioInCar();
}

void
cDMAudio::SetRadioInCar(uint32 radio)
{
	MusicManager.SetRadioInCar(radio);
}

void
cDMAudio::SetRadioChannel(uint32 radio, int32 pos)
{
	MusicManager.SetRadioChannelByScript(radio, pos);
}

void
cDMAudio::SetStartingTrackPositions(bool8 isStartGame)
{
	MusicManager.SetStartingTrackPositions(isStartGame);
}

float *
cDMAudio::GetListenTimeArray()
{
	return MusicManager.GetListenTimeArray();
}

uint32
cDMAudio::GetFavouriteRadioStation()
{
	return MusicManager.GetFavouriteRadioStation();
}

int32
cDMAudio::GetRadioPosition(uint32 station)
{
	return MusicManager.GetRadioPosition(station);
}

void
cDMAudio::SetPedTalkingStatus(CPed *ped, bool8 status)
{
	return AudioManager.SetPedTalkingStatus(ped, status);
}

void
cDMAudio::SetPlayersMood(uint8 mood, uint32 time)
{
	return AudioManager.SetPlayersMood(mood, time);
}

void
cDMAudio::ShutUpPlayerTalking(bool8 state)
{
	AudioManager.m_bIsPlayerShutUp = state;
}