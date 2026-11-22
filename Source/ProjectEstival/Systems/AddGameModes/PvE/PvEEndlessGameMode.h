// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PvEEndlessGameMode.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ESurvivalState : uint8
{
	Waiting UMETA(DisplayName = "Waiting"),
	Running UMETA(DisplayName = "Running"),
	GameOver UMETA(DisplayName = "Game Over")
};

UCLASS()
class PROJECTESTIVAL_API APvEEndlessGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	APvEEndlessGameMode();
	
	UPROPERTY(BlueprintReadOnly, Category = "PvE Endless | Survival")
	ESurvivalState CurrentSurvivalState = ESurvivalState::Waiting;
	
	UPROPERTY(BlueprintReadOnly, Category = "PvE Endless | Survival")
	float SurvivalTime = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "PvE Endless | Difficulty")
	int32 DifficultyLevel = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PvE Endless | Difficulty")
	float DifficultyIncreaseInterval = 60.0f;

	float DebugLogTimer = 0.0f;
	
	UFUNCTION(BlueprintCallable, Category = "PvE Endless | Survival")
	void StartSurvival();
	
	UFUNCTION(BlueprintCallable, Category = "PvE Endless | Survival")
	void EndSurvival();
	
	virtual void Tick(float DeltaTime) override;
	
	
};
