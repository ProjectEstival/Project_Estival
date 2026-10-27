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
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetSensorUpdated);
	}
}

void AEnemyAIController::OnTargetSensorUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	AEstivalBaseCharacter* SensePlayer = Cast<AEstivalBaseCharacter>(Actor);
	
	if (!SensePlayer || !SensePlayer->IsPlayerControlled()) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		SensedPlayerRef.AddUnique(SensePlayer);
		UpdatePlayerRef();
	}
	else
	{
		SensedPlayerRef.Remove(SensePlayer);
		UpdatePlayerRef();
	}
}

void AEnemyAIController::UpdatePlayerRef()
{
	SensedPlayerRef.RemoveAll([](const TObjectPtr<AEstivalBaseCharacter>& Player)
	{
		return !IsValid(Player);
	});
	
	APawn* ControlledPawn = GetPawn();
	
	if (!ControlledPawn || SensedPlayerRef.IsEmpty())
	{
		PlayerRef = nullptr;
		return;
	}
	
	AEstivalBaseCharacter* ClosestPlayer = nullptr;
	float MinDistance = FLT_MAX;
	FVector SelfLocation = ControlledPawn->GetActorLocation();

	for (AEstivalBaseCharacter* Player : SensedPlayerRef)
	{
		if (!IsValid(Player)) continue;

		float Dist = FVector::DistSquared(SelfLocation, Player->GetActorLocation());
		if (Dist < MinDistance)
		{
			MinDistance = Dist;
			ClosestPlayer = Player;
		}
	}

	PlayerRef = ClosestPlayer;
}
