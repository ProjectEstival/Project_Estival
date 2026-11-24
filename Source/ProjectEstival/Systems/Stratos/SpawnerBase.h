// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnerBase.generated.h"


class UEnemySpawner;

UCLASS()
class PROJECTESTIVAL_API ASpawnerBase : public AActor
{
	GENERATED_BODY()

public:
	ASpawnerBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	TObjectPtr<UEnemySpawner> EnemySpawnerComponent;
	
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	AActor* SpawnActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

	UFUNCTION(CallInEditor, Category = "Spawning")
	void TestSpawnEnemies();

	UFUNCTION(CallInEditor, Category = "Spawning")
	void ResetSpawnedEnemies();
};