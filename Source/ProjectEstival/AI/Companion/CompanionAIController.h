// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "AIController.h"
#include "CompanionAIController.generated.h"

class UStateTreeAIComponent;

UCLASS(Abstract)
class PROJECTESTIVAL_API ACompanionAIController : public AAIController
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;

public:
	// Sets default values for this actor's properties
	ACompanionAIController();


};
