// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Systems/Core/Characters/CharacterBase.h"

AEnemyAIController::AEnemyAIController()
{
	StateTree = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTree Component"));
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetSensotUpdated);
	}
}

void AEnemyAIController::OnTargetSensotUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	AEstivalBaseCharacter* SensePlayer = Cast<AEstivalBaseCharacter>(Actor);
	if (!SensePlayer) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		PlayerRef = SensePlayer;
	}
	else if (PlayerRef == SensePlayer)
	{
		PlayerRef = nullptr;
	}
}