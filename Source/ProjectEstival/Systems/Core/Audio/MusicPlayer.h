// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MusicPlayer.generated.h"

class UFMODEvent;
class UFMODAudioComponent;

UCLASS()
class PROJECTESTIVAL_API UMusicPlayer : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TObjectPtr<UFMODAudioComponent> MusicComponent;

	UPROPERTY(Transient)
	TObjectPtr<UFMODEvent> CurrentEvent;

public:
	bool Initialize(UWorld* World);
	bool Play(UFMODEvent* Event);
	void Stop(bool bAllowFadeOut = true);
	void Deinitialize();

	UFMODAudioComponent* GetComponent() const { return MusicComponent; }
	UFMODEvent* GetCurrentEvent() const { return CurrentEvent; }
	bool IsPlaying() const;
	void SetPaused(bool bPaused) const;
	bool IsPaused() const;
};
