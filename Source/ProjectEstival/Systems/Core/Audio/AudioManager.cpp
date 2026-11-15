// Fill out your copyright notice in the Description page of Project Settings.

#include "Systems/Core/Audio/AudioManager.h"
#include "AudioSystemSettings.h"
#include "FMODStudioModule.h"
#include "fmod_studio.hpp"
#include "FMODBlueprintStatics.h"
#include "FMODAudioComponent.h"
#include "FMODEvent.h"
#include "FMODSnapshot.h"
#include "MusicPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"


void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, "[Audio Manager] Initialized");
	MusicPlayer = NewObject<UMusicPlayer>(this);
}

void UAudioManager::Deinitialize()
{
	ParameterTweener.StopAll();
	StopAllSnapshots(true);

	if (IsValid(MusicPlayer))
	{
		MusicPlayer->Deinitialize();
		MusicPlayer = nullptr;
	}

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("[AudioManager] Deinitialized"));

	Super::Deinitialize();
}

void UAudioManager::PlayOneShot(UFMODEvent* Event)
{
	if (!IsValid(Event)) return;
	UFMODBlueprintStatics::PlayEvent2D(GetGameInstance(), Event, true);
}

void UAudioManager::PlayOneShotAtLocation(UFMODEvent* Event, const FVector Location)
{
	if (!IsValid(Event)) return;
	UFMODBlueprintStatics::PlayEventAtLocation(GetGameInstance(), Event, FTransform(Location), true);
}

void UAudioManager::PlayOneShotAttached(UFMODEvent* Event, USceneComponent* AttachToComponent)
{
	if (!IsValid(Event) || !IsValid(AttachToComponent)) return;
	UFMODBlueprintStatics::PlayEventAttached(Event, AttachToComponent, NAME_None, FVector::ZeroVector,
	                                         EAttachLocation::KeepRelativeOffset, true, true, true);
}

UFMODAudioComponent* UAudioManager::CreateAttachedAudio(UFMODEvent* Event, USceneComponent* AttachToComponent)
{
	if (!IsValid(Event) || !IsValid(AttachToComponent)) return nullptr;

	AActor* Owner = AttachToComponent->GetOwner();
	if (!IsValid(Owner)) return nullptr;

	UFMODAudioComponent* Component = NewObject<UFMODAudioComponent>(Owner);
	if (!Component) return nullptr;

	Component->bAutoActivate = false;
	Component->bAutoDestroy = false;
	Component->bStopWhenOwnerDestroyed = true;

	Component->SetupAttachment(AttachToComponent);
	Component->SetEvent(Event);
	Component->RegisterComponent();

	return Component;
}

UFMODAudioComponent* UAudioManager::Create2DAudio(UFMODEvent* Event, AActor* Owner) const
{
	if (!IsValid(Event) || !IsValid(Owner)) return nullptr;

	UFMODAudioComponent* Component = NewObject<UFMODAudioComponent>(Owner);
	if (!IsValid(Component)) return nullptr;

	Component->bAutoActivate = false;
	Component->bAutoDestroy = false;
	Component->bStopWhenOwnerDestroyed = true;

	Component->SetEvent(Event);

	Owner->AddInstanceComponent(Component);
	Component->RegisterComponent();

	if (!Component->IsRegistered())
	{
		Component->DestroyComponent();
		return nullptr;
	}

	return Component;
}

bool UAudioManager::SetGlobalParameter(FName ParameterName, float Value, float Duration)
{
	if (Duration > 0.f) return TweenGlobalParameter(ParameterName, Value, Duration);

	ParameterTweener.Stop(nullptr, ParameterName);
	return ApplyGlobalParameter(ParameterName, Value);
}

bool UAudioManager::GetGlobalParameter(const FName ParameterName, float& OutValue)
{
	OutValue = 0.0f;

	if (ParameterName.IsNone() || !IFMODStudioModule::IsAvailable()) return false;

	FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
	if (!System) return false;

	const FString Name = ParameterName.ToString();
	FMOD_RESULT Result = System->getParameterByName(TCHAR_TO_UTF8(*Name), &OutValue);

	if (Result != FMOD_OK)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("[AudioManager] Failed to get global parameter."));
		return false;
	}

	return true;
}

bool UAudioManager::ApplyGlobalParameter(const FName ParameterName, const float Value)
{
	if (ParameterName.IsNone() || !IFMODStudioModule::IsAvailable()) return false;

	FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
	if (!System) return false;

	const FString Name = ParameterName.ToString();
	const FMOD_RESULT Result = System->setParameterByName(TCHAR_TO_UTF8(*Name), Value);

	if (Result != FMOD_OK)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("[AudioManager] Failed to set global parameter: %s (FMOD: %d)"), *Name, static_cast<int32>(Result)));
		return false;
	}

	return true;
}

bool UAudioManager::SetInstanceParameter(UFMODAudioComponent* Component, const FName ParameterName, const float Value, const float Duration)
{
	if (!IsValid(Component) || ParameterName.IsNone()) return false;

	if (Duration > 0.f) return TweenInstanceParameter(Component, ParameterName, Value, Duration);

	ParameterTweener.Stop(Component, ParameterName);

	Component->SetParameter(ParameterName, Value);
	return true;
}

bool UAudioManager::GetInstanceParameter(UFMODAudioComponent* Component, const FName ParameterName, float& OutValue)
{
	OutValue = 0.f;

	if (!IsValid(Component) || ParameterName.IsNone()) return false;

	OutValue = Component->GetParameter(ParameterName);
	return true;
}

bool UAudioManager::PlayMusic(UFMODEvent* Event)
{
	if (!IsValid(Event) || !IsValid(MusicPlayer)) return false;

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!MusicPlayer->Initialize(World)) return false;

	if (MusicPlayer->GetCurrentEvent() != Event)
	{
		ParameterTweener.StopAllForObject(MusicPlayer->GetComponent());
	}

	return MusicPlayer->Play(Event);
}

void UAudioManager::StopMusic(bool bAllowFadeOut)
{
	if (!IsValid(MusicPlayer)) return;

	ParameterTweener.StopAllForObject(MusicPlayer->GetComponent());
	MusicPlayer->Stop(bAllowFadeOut);
}

UFMODAudioComponent* UAudioManager::GetMusicComponent() const
{
	return IsValid(MusicPlayer) ? MusicPlayer->GetComponent() : nullptr;
}

void UAudioManager::SetMusicPaused(const bool bPaused) const
{
	if (IsValid(MusicPlayer)) MusicPlayer->SetPaused(bPaused);
}

bool UAudioManager::IsMusicPlaying() const
{
	return IsValid(MusicPlayer) && MusicPlayer->IsPlaying();
}

bool UAudioManager::IsMusicPaused() const
{
	return IsValid(MusicPlayer) && MusicPlayer->IsPaused();
}

void UAudioManager::DestroyAudio(UFMODAudioComponent* Component)
{
	if (!IsValid(Component)) return;
	if (IsValid(MusicPlayer) && Component == MusicPlayer->GetComponent()) return;

	ParameterTweener.StopAllForObject(Component);

	Component->Stop();
	Component->DestroyComponent();
}

bool UAudioManager::TweenGlobalParameter(FName ParameterName, const float TargetValue, const float Duration)
{
	ParameterTweener.Stop(nullptr, ParameterName);

	float StartValue = 0.f;
	if (!GetGlobalParameter(ParameterName, StartValue)) return false;

	if (Duration <= 0.f) return ApplyGlobalParameter(ParameterName, TargetValue);

	ParameterTweener.Tween(nullptr, ParameterName, StartValue, TargetValue, Duration, [this, ParameterName](float Value)
	{
		return ApplyGlobalParameter(ParameterName, Value);
	});

	return true;
}

bool UAudioManager::TweenInstanceParameter(UFMODAudioComponent* Component, FName ParameterName, const float TargetValue, const float Duration)
{
	if (!IsValid(Component) || ParameterName.IsNone()) return false;

	ParameterTweener.Stop(Component, ParameterName);

	if (Duration <= 0.f)
	{
		Component->SetParameter(ParameterName, TargetValue);
		return true;
	}

	const float StartValue = Component->GetParameter(ParameterName);
	const TWeakObjectPtr WeakComponent(Component);

	ParameterTweener.Tween(Component, ParameterName, StartValue, TargetValue, Duration, [WeakComponent, ParameterName](float Value)
	{
		UFMODAudioComponent* AudioComponent = WeakComponent.Get();
		if (!IsValid(AudioComponent)) return false;

		AudioComponent->SetParameter(ParameterName, Value);
		return true;
	});

	return true;
}

#pragma region Volume Control
bool UAudioManager::SetBusVolume(const FString& BusPath, float Volume)
{
	if (BusPath.IsEmpty() || !IFMODStudioModule::IsAvailable()) return false;

	FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
	if (!System) return false;

	FMOD::Studio::Bus* Bus = nullptr;
	FMOD_RESULT Result = System->getBus(TCHAR_TO_UTF8(*BusPath), &Bus);

	if (Result != FMOD_OK || !Bus)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
													  FString::Printf(
														  TEXT("[AudioManager] Failed to find bus: %s (FMOD: %d)"),
														  *BusPath, static_cast<int32>(Result)));
		return false;
	}

	Result = Bus->setVolume(FMath::Clamp(Volume, 0.0f, 1.0f));
	return Result == FMOD_OK;
}

float UAudioManager::GetBusVolume(const FString& BusPath)
{
	if (BusPath.IsEmpty() || !IFMODStudioModule::IsAvailable()) return -1.0f;

	FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
	if (!System) return -1.0f;

	FMOD::Studio::Bus* Bus = nullptr;
	if (System->getBus(TCHAR_TO_UTF8(*BusPath), &Bus) != FMOD_OK || !Bus) return -1.0f;

	float Volume = 0.0f;
	if (Bus->getVolume(&Volume) != FMOD_OK) return -1.0f;

	return Volume;
}

bool UAudioManager::SetMasterVolume(float Volume) const
{
	return SetBusVolume(GetDefault<UAudioSystemSettings>()->MasterBusPath, Volume);
}

bool UAudioManager::SetMusicVolume(float Volume) const
{
	return SetBusVolume(GetDefault<UAudioSystemSettings>()->MusicBusPath, Volume);
}

bool UAudioManager::SetSFXVolume(float Volume) const
{
	return SetBusVolume(GetDefault<UAudioSystemSettings>()->SFXBusPath, Volume);
}

bool UAudioManager::SetAmbienceVolume(float Volume) const
{
	return SetBusVolume(GetDefault<UAudioSystemSettings>()->AmbienceBusPath, Volume);
}

bool UAudioManager::SetUIVolume(float Volume) const
{
	return SetBusVolume(GetDefault<UAudioSystemSettings>()->UIBusPath, Volume);
}

bool UAudioManager::SetDialogueVolume(float Volume) const
{
	return SetBusVolume(GetDefault<UAudioSystemSettings>()->DialogueBusPath, Volume);
}

float UAudioManager::GetMasterVolume() const
{
	return GetBusVolume(GetDefault<UAudioSystemSettings>()->MasterBusPath);
}

float UAudioManager::GetMusicVolume() const
{
	return GetBusVolume(GetDefault<UAudioSystemSettings>()->MusicBusPath);
}

float UAudioManager::GetSFXVolume() const
{
	return GetBusVolume(GetDefault<UAudioSystemSettings>()->SFXBusPath);
}

float UAudioManager::GetAmbienceVolume() const
{
	return GetBusVolume(GetDefault<UAudioSystemSettings>()->AmbienceBusPath);
}

float UAudioManager::GetUIVolume() const
{
	return GetBusVolume(GetDefault<UAudioSystemSettings>()->UIBusPath);
}

float UAudioManager::GetDialogueVolume() const
{
	return GetBusVolume(GetDefault<UAudioSystemSettings>()->DialogueBusPath);
}
#pragma endregion

bool UAudioManager::SetAudioListener(APlayerController* PlayerController, USceneComponent* ListenerComponent)
{
	if (!IsValid(PlayerController) || !IsValid(ListenerComponent)) return false;
	if (!PlayerController->IsLocalController()) return false;

	PlayerController->SetAudioListenerOverride(ListenerComponent, FVector::ZeroVector, FRotator::ZeroRotator);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("[AudioManager] Audio listener configured."));

	return true;
}

void UAudioManager::ResetAudioListener(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;

	PlayerController->ClearAudioListenerOverride();
}

bool UAudioManager::PlaySnapshot(UFMODSnapshot* Snapshot)
{
	if (!IsValid(Snapshot) || !Snapshot->AssetGuid.IsValid()) return false;
	if (!IFMODStudioModule::IsAvailable()) return false;

	const FGuid Key = Snapshot->AssetGuid;

	if (FMOD::Studio::EventInstance** Existing = ActiveSnapshots.Find(Key))
	{
		if (*Existing && (*Existing)->isValid())
		{
			FMOD_STUDIO_PLAYBACK_STATE State = FMOD_STUDIO_PLAYBACK_STOPPED;

			if ((*Existing)->getPlaybackState(&State) == FMOD_OK && State != FMOD_STUDIO_PLAYBACK_STOPPED)
				return true;

			(*Existing)->release();
		}

		ActiveSnapshots.Remove(Key);
	}

	FMOD::Studio::EventDescription* Description = IFMODStudioModule::Get().GetEventDescription(Snapshot, EFMODSystemContext::Runtime);
	if (!Description) return false;

	bool bIsSnapshot = false;
	if (Description->isSnapshot(&bIsSnapshot) != FMOD_OK || !bIsSnapshot) return false;

	FMOD::Studio::EventInstance* Instance = nullptr;
	FMOD_RESULT Result = Description->createInstance(&Instance);

	if (Result != FMOD_OK || !Instance) return false;

	Result = Instance->start();

	if (Result != FMOD_OK)
	{
		Instance->release();

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
			FString::Printf(TEXT("[AudioManager] Failed to play snapshot: %s (FMOD: %d)"), *Snapshot->GetName(), static_cast<int32>(Result)));

		return false;
	}

	ActiveSnapshots.Add(Key, Instance);
	return true;
}

bool UAudioManager::StopSnapshot(UFMODSnapshot* Snapshot, bool bImmediate)
{
	if (!IsValid(Snapshot)) return false;

	FMOD::Studio::EventInstance** Found = ActiveSnapshots.Find(Snapshot->AssetGuid);
	if (!Found) return false;

	FMOD::Studio::EventInstance* Instance = *Found;
	ActiveSnapshots.Remove(Snapshot->AssetGuid);

	if (!Instance || !Instance->isValid()) return false;

	const FMOD_STUDIO_STOP_MODE Mode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;

	const FMOD_RESULT StopResult = Instance->stop(Mode);
	const FMOD_RESULT ReleaseResult = Instance->release();

	return StopResult == FMOD_OK && ReleaseResult == FMOD_OK;
}

bool UAudioManager::IsSnapshotActive(UFMODSnapshot* Snapshot) const
{
	if (!IsValid(Snapshot)) return false;

	FMOD::Studio::EventInstance* const* Found = ActiveSnapshots.Find(Snapshot->AssetGuid);
	if (!Found || !*Found || !(*Found)->isValid()) return false;

	FMOD_STUDIO_PLAYBACK_STATE State = FMOD_STUDIO_PLAYBACK_STOPPED;

	return (*Found)->getPlaybackState(&State) == FMOD_OK && State != FMOD_STUDIO_PLAYBACK_STOPPED;
}

void UAudioManager::StopAllSnapshots(bool bImmediate)
{
	if (IFMODStudioModule::IsAvailable() && IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime))
	{
		const FMOD_STUDIO_STOP_MODE Mode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;

		for (const auto& Pair : ActiveSnapshots)
		{
			FMOD::Studio::EventInstance* Instance = Pair.Value;

			if (!Instance || !Instance->isValid()) continue;

			Instance->stop(Mode);
			Instance->release();
		}
	}

	ActiveSnapshots.Empty();
}