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
		AIPerceptionComp->OnTargetPerceptionForgotten.AddDynamic(this, &AEnemyAIController::OnTargetSensorForgot);
	}
	
	GetWorld()->GetTimerManager().SetTimer(CheckDistanceTimerHandle, this, &AEnemyAIController::DistCheck, CheckDistTimer, true);
}


void AEnemyAIController::OnTargetSensorUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;
	
	if (AEstivalBaseCharacter* PlayerCandidate = Cast<AEstivalBaseCharacter>(Actor))
	{
		if (PlayerCandidate->GetController<AEnemyAIController>()) return;
		
		if (Stimulus.WasSuccessfullySensed())
		{
			Players.AddUnique(PlayerCandidate);
			
			if (!PlayerRef)
			{
				PlayerRef = PlayerCandidate;
			}
		}
		else
		{
			Players.Remove(PlayerCandidate);
		}
	}
}

void AEnemyAIController::OnTargetSensorForgot(AActor* Actor)
{
	if (!Actor) return;

	if (AEstivalBaseCharacter* PlayerCandidate = Cast<AEstivalBaseCharacter>(Actor))
	{
		if (PlayerRef == PlayerCandidate)
		{
			PlayerRef = nullptr;
		}
		
		Players.Remove(PlayerCandidate);
	}
}

void AEnemyAIController::DistCheck()
{
	float MinDist = FLT_MAX;
	AEstivalBaseCharacter* PlayerToTarget = nullptr;
	
	for (AEstivalBaseCharacter* Player : Players)
	{
		if (!Player) continue;
		
		float Dist = FVector::Distance(Player->GetActorLocation(), GetPawn()->GetActorLocation());
		
		if (Dist < MinDist)
		{
			MinDist = Dist;
			PlayerToTarget = Player;
		}
	}
	
	PlayerRef = PlayerToTarget;
}
