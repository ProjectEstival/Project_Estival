// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ParameterTweener.h"
#include "AudioManager.generated.h"

namespace FMOD::Studio
{
	class EventInstance;
}

class UFMODSnapshot;
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
	 * @return UFMODAudioComponent: Pointer to the created FMOD Audio Component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Playback")
	UFMODAudioComponent* CreateAttachedAudio(UFMODEvent* Event, USceneComponent* AttachToComponent);
	/**
	 * Used for looping 2D sounds, mostly ambience, do NOT use for music.
	 * @param Event UFMODEvent: Pointer to the audio event.
	 * @param Owner AActor: Pointer to the owning Actor.
	 * @return UFMODAudioComponent: Pointer to the created FMOD Audio Component.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Playback")
	UFMODAudioComponent* Create2DAudio(UFMODEvent* Event, AActor* Owner) const;
	/**
	 * Used to change the value of a global FMOD parameter.
	 * @param ParameterName FName: The name of the parameter.
	 * @param Value float: The new value to set.
	 * @param Duration float: Smoothing time to change. Leave blank or 0 for instant.
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Parameters")
	bool SetGlobalParameter(FName ParameterName, float Value, float Duration = 0.f);
	/**
	 * Tries to retrieve the value of the specified global parameter.
	 * @param ParameterName FName: The name of the parameter.
	 * @param OutValue float: The RETURNED value of the parameter.
	 * @return bool: Whether the method found the specified parameter.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Parameters")
	static bool GetGlobalParameter(FName ParameterName, float& OutValue);
	
	UFUNCTION(BlueprintCallable, Category="Audio|Parameters")
	bool SetInstanceParameter(UFMODAudioComponent* Component, FName ParameterName, float Value, float Duration = 0.f);
	/**
	 * Tries to retrieve the value of the specified instance parameter.
	 * @param ParameterName FName: The name of the parameter.
	 * @param OutValue float: The RETURNED value of the parameter.
	 * @return bool: Whether the method found the specified parameter.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Parameters")
	static bool GetInstanceParameter(UFMODAudioComponent* Component, FName ParameterName, float& OutValue);
	/**
	 * Used to start or resume the current music event, or switch to a new music track.
	 * @param Event UFMODEvent: Pointer to the FMOD Event.
	 * @return bool: Whether FMOD's Play function was successful.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Music")
	bool PlayMusic(UFMODEvent* Event);
	/**
	 * Used for stopping the currently active music track.
	 * @param bAllowFadeOut bool: Whether the music should stop with fade out.
	 */
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void StopMusic(bool bAllowFadeOut = true);
	/**
	 * Retrieves the Music Player's active music component.
	 * @return UFMODAudioComponent: If found, the requested music component.
	 */
	UFUNCTION(BlueprintPure, Category="Audio|Music")
	UFMODAudioComponent* GetMusicComponent() const;
	/**
	 * Pause/Unpause the current music track.
	 * @param bPaused bool: The pause state of the music track.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Music")
	void SetMusicPaused(bool bPaused) const;
	/**
	 * Checks if the music player has an active music component.
	 * @return bool: Whether the music component is active.
	 */
	UFUNCTION(BlueprintPure, Category="Audio|Music")
	bool IsMusicPlaying() const;
	/**
	 * Checks the current music component's pause state.
	 * @return bool: Whether the component is paused.
	 */
	UFUNCTION(BlueprintPure, Category="Audio|Music")
	bool IsMusicPaused() const;
	/**
	 * Called to destroy an FMOD Audio Component. Use with care!
	 * @param Component UFMODAudioComponent: Pointer to the component to be destroyed.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Playback")
	void DestroyAudio(UFMODAudioComponent* Component);
	
	// These are all self-explanatory, if you need a summary... reconsider your discipline pls.
#pragma region VolumeControl
	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetMasterVolume(float Volume) const;

	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetMusicVolume(float Volume) const;

	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetSFXVolume(float Volume) const;

	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetAmbienceVolume(float Volume) const;

	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
	bool SetUIVolume(float Volume) const;
	
	UFUNCTION(BlueprintCallable, Category="Audio|Volume")
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
	// Okay self-explanatory methods are over you are safe now.
	
	/**
	 * Used to change the audio listener's position.
	 * @param PlayerController APlayerController: Pointer to the player controller who owns the listener.
	 * @param ListenerComponent USceneComponent: Pointer to the scene component to attach the listener to.
	 * @return bool: Whether the listener was overriden successfully.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Listener")
	bool SetAudioListener(APlayerController* PlayerController, USceneComponent* ListenerComponent);
	/**
	 * Resets the specified audio listener's position to its default.
	 * @param PlayerController APLayerController: Pointer to the player controller who owns the listener.
	 */
	UFUNCTION(BlueprintCallable, Category="Audio|Listener")
	void ResetAudioListener(APlayerController* PlayerController);
	
#pragma region Snapshots

	UFUNCTION(BlueprintCallable, Category="Audio|Snapshots")
	bool PlaySnapshot(UFMODSnapshot* Snapshot);

	UFUNCTION(BlueprintCallable, Category="Audio|Snapshots")
	bool StopSnapshot(UFMODSnapshot* Snapshot, bool bImmediate = false);

	UFUNCTION(BlueprintPure, Category="Audio|Snapshots")
	bool IsSnapshotActive(UFMODSnapshot* Snapshot) const;

	UFUNCTION(BlueprintCallable, Category="Audio|Snapshots")
	void StopAllSnapshots(bool bImmediate = false);

#pragma endregion
	
private:
#pragma region Helper Methods
	static bool SetBusVolume(const FString& BusPath, float Volume);
	static float GetBusVolume(const FString& BusPath);
	/**
	 * Instant set of the value for a global parameter.
	 */
	static bool ApplyGlobalParameter(FName ParameterName, float Value);
	/**
	 * Smoothly interpolates to the target value requested.
	 */
	bool TweenGlobalParameter(FName ParameterName, float TargetValue, float Duration);
	/**
	 * Smoothly interpolates to the target value requested.
	 */
	bool TweenInstanceParameter(UFMODAudioComponent* Component, FName ParameterName, float TargetValue, float Duration);
	TMap<FGuid, FMOD::Studio::EventInstance*> ActiveSnapshots;
#pragma endregion
};
