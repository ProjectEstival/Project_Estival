// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MusicPlayer.generated.h"

class UFMODEvent;
namespace FMOD::Studio { class EventInstance; }

UCLASS()
class PROJECTESTIVAL_API UMusicPlayer : public UObject
{
	GENERATED_BODY()

private:
	// Owned until Stop/Deinitialize, independent of any level world.
	FMOD::Studio::EventInstance* MusicInstance = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UFMODEvent> CurrentEvent;

public:
	bool Play(UFMODEvent* Event);
	void Stop(bool bAllowFadeOut = true);
	void Deinitialize();

	bool SetParameter(FName Name, float Value);
	bool GetParameter(FName Name, float& OutValue) const;
	UFMODEvent* GetCurrentEvent() const { return CurrentEvent; }
	bool IsPlaying() const;
	void SetPaused(bool bPaused) const;
	bool IsPaused() const;
};
