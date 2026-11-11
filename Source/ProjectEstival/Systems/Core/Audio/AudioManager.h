// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ParameterTweener.h"
#include "AudioManager.generated.h"

class UFMODEvent;
class USceneComponent;
class UFMODAudioComponent;
class UMusicPlayer;
class APlayerController;

/**
 * UAudioManager
 * 
 * Game Instance Subsystem for accessing audio methods and managing FMOD.
 */
UCLASS()
class PROJECTESTIVAL_API UAudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	UPROPERTY(Transient)
	TObjectPtr<UMusicPlayer> MusicPlayer;
	
	FParameterTweener ParameterTweener;
	
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
	 * @param Event UFMODEvent: Pointer to the audio event.
	 * @param AttachToComponent USceneComponent: Pointer to the component to attach to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Playback")
	void PlayOneShotAttached(UFMODEvent* Event, USceneComponent* AttachToComponent);
	/**
	 * Creates an FMOD Audio Component attached to the specified component, which can
	 * be controlled at runtime.
	 * @param Event UFMODEvent: Pointer to the audio event.
	 * @param AttachToComponent USceneComponent: Pointer to the component to attach to.
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Playback")
	UFMODAudioComponent* CreateAttachedAudio(UFMODEvent* Event, USceneComponent* AttachToComponent);
	
	UFUNCTION(BlueprintCallable, Category="Audio|Playback")
	UFMODAudioComponent* Create2DAudio(UFMODEvent* Event, AActor* Owner) const;
	
	UFUNCTION(BlueprintCallable, Category="Audio|Parameters")
	bool SetGlobalParameter(FName ParameterName, float Value, float Duration = 0.f);

	UFUNCTION(BlueprintCallable, Category="Audio|Parameters")
	static bool GetGlobalParameter(FName ParameterName, float& OutValue);
	
	UFUNCTION(BlueprintCallable, Category="Audio|Parameters")
	bool SetInstanceParameter(UFMODAudioComponent* Component, FName ParameterName, float Value, float Duration = 0.f);
	
	UFUNCTION(BlueprintCallable, Category="Audio|Parameters")
	static bool GetInstanceParameter(UFMODAudioComponent* Component, FName ParameterName, float& OutValue);
	
	UFUNCTION(BlueprintCallable, Category="Audio|Music")
	bool PlayMusic(UFMODEvent* Event);

	UFUNCTION(BlueprintCallable, Category="Audio|Music")
	void StopMusic();

	UFUNCTION(BlueprintPure, Category="Audio|Music")
	UFMODAudioComponent* GetMusicComponent() const;
	
	UFUNCTION(BlueprintCallable, Category="Audio|Music")
	void SetMusicPaused(bool bPaused) const;

	UFUNCTION(BlueprintPure, Category="Audio|Music")
	bool IsMusicPlaying() const;

	UFUNCTION(BlueprintPure, Category="Audio|Music")
	bool IsMusicPaused() const;
	
	UFUNCTION(BlueprintCallable, Category="Audio|Playback")
	void DestroyAudio(UFMODAudioComponent* Component);
	
#pragma region VolumeControl
	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetMasterVolume(float Volume) const;

	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetMusicVolume(float Volume) const;

	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetSFXVolume(float Volume) const;

	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetAmbienceVolume(float Volume) const;

	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	bool SetUIVolume(float Volume) const;
	
	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	bool SetDialogueVolume(float Volume) const;
	
	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	float GetMasterVolume() const;

	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	float GetMusicVolume() const;

	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	float GetSFXVolume() const;

	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	float GetAmbienceVolume() const;
	
	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	float GetUIVolume() const;
	
	UFUNCTION(BlueprintPure, Category="Audio|Volume")
	float GetDialogueVolume() const;
#pragma endregion
	
	UFUNCTION(BlueprintCallable, Category="Audio|Listener")
	bool SetAudioListener(APlayerController* PlayerController, USceneComponent* ListenerComponent);

	UFUNCTION(BlueprintCallable, Category="Audio|Listener")
	void ResetAudioListener(APlayerController* PlayerController);
	
private:
#pragma region Helper Methods
	static bool SetBusVolume(const FString& BusPath, float Volume);
	static float GetBusVolume(const FString& BusPath);
	static bool ApplyGlobalParameter(FName ParameterName, float Value);
	bool TweenGlobalParameter(FName ParameterName, float TargetValue, float Duration);
	bool TweenInstanceParameter(UFMODAudioComponent* Component, FName ParameterName, float TargetValue, float Duration);
#pragma endregion
};
