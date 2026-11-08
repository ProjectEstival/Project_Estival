// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Systems/Stratos/SpawnerBase.h"


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

 
    ASpawnerBase* Spawner = Cast<ASpawnerBase>(GetOwner());

    if (!Spawner)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("EnemySpawner: Owner is not a SpawnerBase."));
        return;
    }

    for (const FEnemySpawnPoint& SpawnPoint : SpawnPoints)
    {
        if (SpawnPoint.Amount <= 0)
        {
            continue;
        }

        const float AngleStep =
            (2.0f * PI) / SpawnPoint.Amount;

        const float RandomAngleOffset =
            FMath::FRandRange(0.0f, 2.0f * PI);

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

            FVector SpawnLocation =
                SpawnPoint.SpawnTransform.GetLocation();

           
            if (SpawnPoint.Amount > 1 &&
                SpawnPoint.SpreadRadius > 0.0f)
            {
                const float Angle =
                    RandomAngleOffset + (AngleStep * i);

                SpawnLocation.X +=
                    FMath::Cos(Angle) * SpawnPoint.SpreadRadius;

                SpawnLocation.Y +=
                    FMath::Sin(Angle) * SpawnPoint.SpreadRadius;
            }

            const FRotator SpawnRotation =
                SpawnPoint.SpawnTransform.GetRotation().Rotator();

            const FTransform SpawnTransform(
                SpawnRotation,
                SpawnLocation,
                SpawnPoint.SpawnTransform.GetScale3D()
            );

            // Ask SpawnerBase to create the enemy
            AActor* SpawnedEnemy =
                Spawner->SpawnActor(SelectedEnemyClass, SpawnTransform);

            if (IsValid(SpawnedEnemy))
            {
                SpawnedEnemies.Add(SpawnedEnemy);
            }
        }
    }
}

void UEnemySpawner::ClearSpawnedEnemies()
{
    for (AActor* SpawnedEnemy : SpawnedEnemies)
    {
        if (IsValid(SpawnedEnemy))
        {
            SpawnedEnemy->Destroy();
        }
    }

    SpawnedEnemies.Empty();
}


/*
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
        
        const float AngleStep =
            (2.0f * PI) / SpawnPoint.Amount;
        const float RandomAngleOffset =
            FMath::FRandRange(0.0f, 2.0f * PI);
        

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

            FVector SpawnLocation =
                           SpawnPoint.SpawnTransform.GetLocation();

            // Distribute enemies around the center
            if (SpawnPoint.Amount > 1 &&
                SpawnPoint.SpreadRadius > 0.0f)
            {
                const float Angle =
                    RandomAngleOffset + (AngleStep * i);

                SpawnLocation.X +=
                    FMath::Cos(Angle) * SpawnPoint.SpreadRadius;

                SpawnLocation.Y +=
                    FMath::Sin(Angle) * SpawnPoint.SpreadRadius;
            }

            const FRotator SpawnRotation =
                SpawnPoint.SpawnTransform.GetRotation().Rotator();

            AActor* SpawnedEnemy = World->SpawnActor<AActor>(
               SelectedEnemyClass,
               SpawnLocation,
               SpawnRotation
           );
            
            if (IsValid(SpawnedEnemy))
            {
                SpawnedEnemies.Add(SpawnedEnemy);
            }
        }
    }
}

void UEnemySpawner::ClearSpawnedEnemies()
{
    for (AActor* SpawnedEnemy : SpawnedEnemies)
    {
        if (IsValid(SpawnedEnemy))
        {
            SpawnedEnemy->Destroy();
        }
    }

    SpawnedEnemies.Empty();
}
*/

