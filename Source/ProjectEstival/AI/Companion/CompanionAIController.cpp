// Fill out your copyright notice in the Description page of Project Settings.


#include "CompanionAIController.h"
#include "AI/Companion/CompanionAIController.h"
#include "Components/StateTreeAIComponent.h"


// Sets default values
ACompanionAIController::ACompanionAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	
	bStartAILogicOnPossess = true;
	bAttachToPawn = true;
}



