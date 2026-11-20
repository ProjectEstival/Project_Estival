#include "MusicPlayer.h"
#include "FMODEvent.h"
#include "FMODStudioModule.h"
#include "fmod_studio.hpp"

bool UMusicPlayer::Play(UFMODEvent* Event)
{
	if (!IsValid(Event) || !IFMODStudioModule::IsAvailable()) return false;

	if (CurrentEvent == Event && (IsPlaying() || IsPaused()))
	{
		SetPaused(false);
		return true;
	}

	FMOD::Studio::EventDescription* Description =
		IFMODStudioModule::Get().GetEventDescription(Event, EFMODSystemContext::Runtime);
	if (!Description) return false;

	FMOD::Studio::EventInstance* NewInstance = nullptr;
	if (Description->createInstance(&NewInstance) != FMOD_OK || !NewInstance) return false;
	if (NewInstance->start() != FMOD_OK)
	{
		NewInstance->release();
		return false;
	}

	Stop(true);
	MusicInstance = NewInstance;
	CurrentEvent = Event;
	return true;
}

void UMusicPlayer::Stop(bool bAllowFadeOut)
{
	if (IFMODStudioModule::IsAvailable() &&
		IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime) &&
		MusicInstance && MusicInstance->isValid())
	{
		MusicInstance->stop(bAllowFadeOut ? FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE);
		MusicInstance->release();
	}
	MusicInstance = nullptr;
	CurrentEvent = nullptr;
}

bool UMusicPlayer::IsPlaying() const
{
	if (!MusicInstance || !MusicInstance->isValid()) return false;
	FMOD_STUDIO_PLAYBACK_STATE State = FMOD_STUDIO_PLAYBACK_STOPPED;
	return MusicInstance->getPlaybackState(&State) == FMOD_OK && State != FMOD_STUDIO_PLAYBACK_STOPPED;
}

void UMusicPlayer::SetPaused(bool bPaused) const
{
	if (MusicInstance && MusicInstance->isValid()) MusicInstance->setPaused(bPaused);
}

bool UMusicPlayer::IsPaused() const
{
	bool bPaused = false;
	return MusicInstance && MusicInstance->isValid() &&
		MusicInstance->getPaused(&bPaused) == FMOD_OK && bPaused;
}

bool UMusicPlayer::SetParameter(FName Name, float Value)
{
	return !Name.IsNone() && MusicInstance && MusicInstance->isValid() &&
		MusicInstance->setParameterByName(TCHAR_TO_UTF8(*Name.ToString()), Value) == FMOD_OK;
}

bool UMusicPlayer::GetParameter(FName Name, float& OutValue) const
{
	OutValue = 0.f;
	return !Name.IsNone() && MusicInstance && MusicInstance->isValid() &&
		MusicInstance->getParameterByName(TCHAR_TO_UTF8(*Name.ToString()), &OutValue) == FMOD_OK;
}

void UMusicPlayer::Deinitialize()
{
	Stop(false);
}
