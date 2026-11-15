// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Stratos/SpawnerBase.h"
#include "EnemySpawner.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"


ASpawnerBase::ASpawnerBase()
{
    PrimaryActorTick.bCanEverTick = false;

    EnemySpawnerComponent =
        CreateDefaultSubobject<UEnemySpawner>(
            TEXT("EnemySpawnerComponent")
        );
}

AActor* ASpawnerBase::SpawnActor(
    TSubclassOf<AActor> ActorClass,
    const FTransform& SpawnTransform
)
{
    if (!ActorClass)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SpawnerBase: ActorClass is not valid."));
        return nullptr;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParameters;

    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

    return World->SpawnActor<AActor>(
        ActorClass,
        SpawnTransform,
        SpawnParameters
    );
}

void ASpawnerBase::TestSpawnEnemies()
{
    if (EnemySpawnerComponent)
    {
        EnemySpawnerComponent->SpawnEnemies();
    }
}

void ASpawnerBase::ResetSpawnedEnemies()
{
    if (EnemySpawnerComponent)
    {
        EnemySpawnerComponent->ClearSpawnedEnemies();
    }
}

