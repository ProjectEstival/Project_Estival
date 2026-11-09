// Fill out your copyright notice in the Description page of Project Settings.

#include "Systems/Core/Audio/AudioManager.h"

#include "FMODBlueprintStatics.h"
#include "FMODEvent.h"


void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, "[Audio Manager] Initialized");
}

void UAudioManager::Deinitialize()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, "[Audio Manager] Deinitialized");
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
	if (!IsValid(Event)	|| !IsValid(AttachToComponent)) return;
	UFMODBlueprintStatics::PlayEventAttached(Event, AttachToComponent, NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, true, true);
}
