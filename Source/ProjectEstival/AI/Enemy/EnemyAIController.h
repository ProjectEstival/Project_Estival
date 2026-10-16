// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
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
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "AI Senses")
	UAIPerceptionComponent* AIPerceptionComp;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "AI State Tree")
	UStateTreeAIComponent* StateTree;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Targets")
	AEstivalBaseCharacter* PlayerRef;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Targets")
	int PatrolIndex = 0;
	
protected:
	
	virtual void BeginPlay() override;
	
private:

	UFUNCTION()
	void OnTargetSensotUpdated(AActor* Actor, FAIStimulus Stimulus);
	
};
