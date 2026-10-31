// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CompanionAIController.generated.h"

class UStateTreeAIComponent;
class UAIPerceptionComponent;  

UCLASS(Abstract)
class PROJECTESTIVAL_API ACompanionAIController : public AAIController
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;
	
	// let's the companions see the enemies  
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UAIPerceptionComponent> Perception;
	
	// transient: so it starts as nullptr every time it loads
	UPROPERTY(Transient)
	TObjectPtr<APawn> FollowTarget;
	

public:
	ACompanionAIController(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintPure, Category="Companion")
	APawn* GetFollowTarget() const {return FollowTarget;}
	
	void SetFollowTarget(APawn* NewTarget) { FollowTarget = NewTarget; }


};
