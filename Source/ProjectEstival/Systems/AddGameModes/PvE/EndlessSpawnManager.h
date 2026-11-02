// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EndlessSpawnManager.generated.h"

UCLASS()
class PROJECTESTIVAL_API AEndlessSpawnManager : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, Category = "Endless PvE")
	TSubclassOf<AActor> EnemyClass;
	
	UPROPERTY(EditAnywhere, Category = "Endless PvE")
	TArray<AActor*> SpawnPoints;
	
	UPROPERTY(EditAnywhere, Category = "Endless PvE", meta = (ClampMin = "0.1"))
	float SpawnInterval = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Endless PvE", meta = (ClampMin = "1"))
	int32 MaxAliveEnemies = 10;
	
	UPROPERTY(VisibleAnywhere, Category = "Endless PvE")
	int32 AliveEnemies = 0;



	FTimerHandle SpawnTimerHandle;
	
	void SpawnEnemy();
	
	UFUNCTION()
	void OnEnemyDestroyed(AActor* DestroyedActor);

	
public:	
	// Sets default values for this actor's properties
	AEndlessSpawnManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

};
