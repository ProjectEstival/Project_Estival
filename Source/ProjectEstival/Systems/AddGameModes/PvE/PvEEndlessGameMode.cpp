// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/AddGameModes/PvE/PvEEndlessGameMode.h"

APvEEndlessGameMode::APvEEndlessGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}


void APvEEndlessGameMode::StartSurvival()
{
    if (CurrentSurvivalState == ESurvivalState::Running)
    {
        return;
    }

    SurvivalTime = 0.0f;
    DifficultyLevel = 1;
    DebugLogTimer = 0.0f;
    
    CurrentSurvivalState = ESurvivalState::Running;

    UE_LOG(LogTemp, Log, TEXT("PvE Endless: Survival Started"));
}

void APvEEndlessGameMode::EndSurvival()
{
    if (CurrentSurvivalState != ESurvivalState::Running)
    {
        return;
    }

    CurrentSurvivalState = ESurvivalState::GameOver;

    UE_LOG(LogTemp, Log, TEXT("PvE Endless: Survival Ended. Time: %.2f seconds"), SurvivalTime);
}


void APvEEndlessGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentSurvivalState == ESurvivalState::Running)
    {
        SurvivalTime += DeltaTime;
        DebugLogTimer += DeltaTime;
        
        if (DifficultyIncreaseInterval > 0.0f)
        {
            const int32 NewDifficultyLevel =
                1 + FMath::FloorToInt(SurvivalTime / DifficultyIncreaseInterval);

            if (NewDifficultyLevel != DifficultyLevel)
            {
                DifficultyLevel = NewDifficultyLevel;

                UE_LOG(LogTemp, Log,
                    TEXT("PvE Endless: Difficulty Level = %d"),
                    DifficultyLevel);
            }
        }


        if (DebugLogTimer >= 5.0f)
        {
            UE_LOG(LogTemp, Log,
                TEXT("PvE Endless: Survival Time = %.2f seconds"),
                SurvivalTime);

            DebugLogTimer = 0.0f;
        }
    }
}
