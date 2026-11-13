// Fill out your copyright notice in the Description page of Project Settings.

#include "MusicPlayer.h"
#include "FMODAudioComponent.h"
#include "FMODEvent.h"
#include "Engine/World.h"

bool UMusicPlayer::Initialize(UWorld* World)
{
	if (IsValid(MusicComponent)) return true;
	if (!IsValid(World)) return false;

	MusicComponent = NewObject<UFMODAudioComponent>(this);
	if (!IsValid(MusicComponent)) return false;

	MusicComponent->bAutoActivate = false;
	MusicComponent->bAutoDestroy = false;
	MusicComponent->RegisterComponentWithWorld(World);

	return MusicComponent->IsRegistered();
}

bool UMusicPlayer::Play(UFMODEvent* Event)
{
	if (!IsValid(Event) || !IsValid(MusicComponent)) return false;

	// Prevent restarting the same track.
	if (CurrentEvent == Event && (MusicComponent->IsPlaying() || MusicComponent->GetPaused()))
	{
		MusicComponent->SetPaused(false);
		return true;
	}

	Stop(true);

	CurrentEvent = Event;
	MusicComponent->SetEvent(Event);
	MusicComponent->Play();

	return true;
}

void UMusicPlayer::Stop(bool bAllowFadeOut)
{
	if (IsValid(MusicComponent))
	{
		FMOD::Studio::EventInstance* Instance = MusicComponent->StudioInstance;

		if (Instance && Instance->isValid())
		{
			Instance->stop(bAllowFadeOut ? FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE);
		}

		MusicComponent->Release();
	}

	CurrentEvent = nullptr;
}

bool UMusicPlayer::IsPlaying() const
{
	return IsValid(MusicComponent) && MusicComponent->IsPlaying();
}

void UMusicPlayer::SetPaused(bool bPaused) const
{
	if (!IsValid(MusicComponent)) return;
	if (!MusicComponent->IsPlaying() && !MusicComponent->GetPaused()) return;

	MusicComponent->SetPaused(bPaused);
}

bool UMusicPlayer::IsPaused() const
{
	return IsValid(MusicComponent) && MusicComponent->GetPaused();
}

void UMusicPlayer::Deinitialize()
{
	Stop();

	if (IsValid(MusicComponent))
	{
		MusicComponent->DestroyComponent();
		MusicComponent = nullptr;
	}
}
