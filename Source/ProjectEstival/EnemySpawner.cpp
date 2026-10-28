// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"


// Sets default values for this component's properties
UEnemySpawner::UEnemySpawner()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UEnemySpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UEnemySpawner::SpawnEnemies()
{
    if (EnemyClasses.IsEmpty())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("EnemySpawner: EnemyClasses is empty."));
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    for (const FEnemySpawnPoint& SpawnPoint : SpawnPoints)
    {
        if (SpawnPoint.Amount <= 0)
        {
            continue;
        }

        for (int32 i = 0; i < SpawnPoint.Amount; i++)
        {
            const int32 RandomIndex =
                FMath::RandRange(0, EnemyClasses.Num() - 1);

            TSubclassOf<AActor> SelectedEnemyClass =
                EnemyClasses[RandomIndex];

            if (!SelectedEnemyClass)
            {
                continue;
            }

            const FVector SpawnLocation =
                SpawnPoint.SpawnTransform.GetLocation();

            const FRotator SpawnRotation =
                SpawnPoint.SpawnTransform.GetRotation().Rotator();

            World->SpawnActor<AActor>(
                SelectedEnemyClass,
                SpawnLocation,
                SpawnRotation
            );
        }
    }
}

