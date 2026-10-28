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
	// Sets default values for this actor's properties
	ASpawnerBase();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	TObjectPtr<UEnemySpawner> EnemySpawnerComponent;

public:	
	UFUNCTION(CallInEditor, Category = "Spawning")
	void TestSpawnEnemies();

};
