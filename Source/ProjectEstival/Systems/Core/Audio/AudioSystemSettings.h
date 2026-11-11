// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AudioSystemSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Audio Settings"))
class PROJECTESTIVAL_API UAudioSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="FMOD|Buses")
	FString MasterBusPath = TEXT("bus:/");

	UPROPERTY(Config, EditAnywhere, Category="FMOD|Buses")
	FString MusicBusPath = TEXT("bus:/Music");

	UPROPERTY(Config, EditAnywhere, Category="FMOD|Buses")
	FString SFXBusPath = TEXT("bus:/SFX");

	UPROPERTY(Config, EditAnywhere, Category="FMOD|Buses")
	FString AmbienceBusPath = TEXT("bus:/Ambience");
	
	UPROPERTY(Config, EditAnywhere, Category="FMOD|Buses")
	FString UIBusPath = TEXT("bus:/UI");
	
	UPROPERTY(Config, EditAnywhere, Category="FMOD|Buses")
	FString DialogueBusPath = TEXT("bus:/Dialogue");
};