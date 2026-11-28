// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Stratos/SpawnerBase.h"
#include "Systems/Core/Spawning/EnemySpawner.h"
#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"
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
    
    if (World->IsGameWorld())
    {
        if (UActorPoolSubsystem* PoolSubsystem = World->GetSubsystem<UActorPoolSubsystem>())
        {
            return PoolSubsystem->AcquireActor(ActorClass, SpawnTransform);
        }
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

void ASpawnerBase::ReleaseActor(AActor* ActorToRelease)
{
    if (!IsValid(ActorToRelease))
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (World && World->IsGameWorld())
    {
        if (UActorPoolSubsystem* PoolSubsystem = World->GetSubsystem<UActorPoolSubsystem>())
        {
            PoolSubsystem->ReleaseActor(ActorToRelease);
            return;
        }
    }
    ActorToRelease->Destroy();
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

