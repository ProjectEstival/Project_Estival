// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/AddGameModes/PvE/EndlessSpawnManager.h"

// Sets default values
AEndlessSpawnManager::AEndlessSpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AEndlessSpawnManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (SpawnInterval <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Endless PvE: Invalid SpawnInterval! Using 1.0 second instead."));

		SpawnInterval = 1.0f;
	}
	
	if (MaxAliveEnemies <= 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Endless PvE: Invalid MaxAliveEnemies! Using 10 instead."));

		MaxAliveEnemies = 10;
	}

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AEndlessSpawnManager::SpawnEnemy,
		SpawnInterval,
		true
	);

	UE_LOG(LogTemp, Warning, TEXT("Endless Spawn Manager started! Interval: %.1f seconds"), SpawnInterval);
}

void AEndlessSpawnManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void AEndlessSpawnManager::SpawnEnemy()
{
	
	if (AliveEnemies >= MaxAliveEnemies)
	{
		return;
	}

	if (!EnemyClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Endless PvE: No EnemyClass assigned!"));
		return;
	}
	
	if (SpawnPoints.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Endless PvE: No SpawnPoints assigned!"));
		return;
	}
	
	TArray<int32> AvailableIndices;

	for (int32 i = 0; i < SpawnPoints.Num(); i++)
	{
		AvailableIndices.Add(i);
	}
	
	while (!AvailableIndices.IsEmpty())
	{
		const int32 RandomArrayIndex = FMath::RandRange(0, AvailableIndices.Num() - 1);

		const int32 SpawnPointIndex = AvailableIndices[RandomArrayIndex];

		AActor* SelectedSpawnPoint = SpawnPoints[SpawnPointIndex];
	
		AvailableIndices.RemoveAt(RandomArrayIndex);
		
		if (!IsValid(SelectedSpawnPoint))
		{
			UE_LOG(LogTemp, Warning, TEXT("Endless PvE: Invalid SpawnPoint selected!"));
			continue;
		}
		
		const FVector SpawnLocation = SelectedSpawnPoint->GetActorLocation();
		const FRotator SpawnRotation = SelectedSpawnPoint->GetActorRotation();
		
		FActorSpawnParameters SpawnParams;

		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

		AActor* SpawnedEnemy = GetWorld()->SpawnActor<AActor>(
			EnemyClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);
		
		if (SpawnedEnemy)
		{
		
			AliveEnemies++;
		
			SpawnedEnemy->OnDestroyed.AddDynamic(
				this,
				&AEndlessSpawnManager::OnEnemyDestroyed
			);
		
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Endless PvE: Enemy spawned at X=%.1f Y=%.1f Z=%.1f"),
				SpawnLocation.X,
				SpawnLocation.Y,
				SpawnLocation.Z
			);
			
			return;
		}
		else
		{
			UE_LOG(
				LogTemp,
				Verbose,
				TEXT("Endless PvE: Spawn blocked at point %s. Trying another point..."),
				*SelectedSpawnPoint->GetName()
				);
		}
	}
	
	UE_LOG(LogTemp, Verbose,
		TEXT("Endless PvE: All spawn points blocked. No enemy spawned."));
	
}

void AEndlessSpawnManager::OnEnemyDestroyed(AActor* DestroyedActor)
{
	AliveEnemies = FMath::Max(0, AliveEnemies - 1);

	UE_LOG(LogTemp, Warning,
		TEXT("Endless PvE: Enemy destroyed! Alive enemies: %d"),
		AliveEnemies);
}




