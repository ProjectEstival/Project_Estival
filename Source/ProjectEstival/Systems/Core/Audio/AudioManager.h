// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioManager.generated.h"

class UFMODEvent;
class USceneComponent;

/**
 * UAudioManager
 * 
 * Game Instance Subsystem for accessing audio methods and managing FMOD.
 */
UCLASS()
class PROJECTESTIVAL_API UAudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Plays a one-shot (play and forget) Sound Effect.
	 * @param Event UFMODEvent: Pointer to the audio event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Playback")
	void PlayOneShot(UFMODEvent* Event);
	/**
	 * Plays a one-shot (play and forget) Sound Effect at the specified location.
	 * @param Event UFMODEvent: Pointer to the audio event.
	 * @param Location FVector: The source location of the sound.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Playback")
	void PlayOneShotAtLocation(UFMODEvent* Event, FVector Location);
	/**
	 * Plays a one-shot (fire and forget) Sound Effect, attached to the specified component.
	 * @param Event 
	 * @param AttachToComponent 
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Playback")
	void PlayOneShotAttached(UFMODEvent* Event, USceneComponent* AttachToComponent);
};
