// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UStateTreeAIComponent;
class UAIPerceptionComponent;
class AEstivalBaseCharacter;

UCLASS()
class PROJECTESTIVAL_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:

	AEnemyAIController();
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AI Senses")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AI State Tree")
	TObjectPtr<UStateTreeAIComponent> StateTree;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Targets")
	TObjectPtr<AEstivalBaseCharacter> PlayerRef;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Timers")
	float CheckDistTimer = 2.0f;
	
	UPROPERTY()
	TArray<AEstivalBaseCharacter*> Players;
	
protected:
	
	virtual void BeginPlay() override;
	
	FTimerHandle CheckDistanceTimerHandle;
	
private:
	
	UFUNCTION()
	void OnTargetSensorUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	UFUNCTION()
	void OnTargetSensorForgot(AActor* Actor);
	
	UFUNCTION()
	void DistCheck();
	
};
