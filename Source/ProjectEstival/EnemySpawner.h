// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemySpawner.generated.h"


USTRUCT(BlueprintType)
struct FEnemySpawnPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FTransform SpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (ClampMin = "0"))
	int32 Amount = 1;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTESTIVAL_API UEnemySpawner : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemySpawner();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<TSubclassOf<AActor>> EnemyClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TArray<FEnemySpawnPoint> SpawnPoints;
	
	UFUNCTION(BlueprintCallable, Category = "Spawning")
        void SpawnEnemies();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
