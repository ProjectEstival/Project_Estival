// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"

#include "EnemyBaseCharacter.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Systems/Core/Characters/CharacterBase.h"

AEnemyAIController::AEnemyAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
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

// Checks Dist to see what player to chase first
void AEnemyAIController::DistCheck()
{
	// forget a target that died
	if (PlayerRef && !PlayerRef->IsAlive()) PlayerRef = nullptr; 
	
	if (Players.IsEmpty()) return;

	APawn* ControlledPawn = GetPawn();
	
	if (!ControlledPawn) return;
	
	float MinDist = FLT_MAX;
	AEstivalBaseCharacter* PlayerToTarget = nullptr;
	
	for (AEstivalBaseCharacter* Player : Players)
	{
		if (!Player || !Player->IsAlive()) continue;
		
		float Dist = FVector::Distance(Player->GetActorLocation(), ControlledPawn->GetActorLocation());
		
		if (Dist < MinDist)
		{
			MinDist = Dist;
			PlayerToTarget = Player;
		}
	}
	
	if (PlayerToTarget != PlayerRef)
	{
		PlayerRef = PlayerToTarget;
		
		if (PlayerRef && StateTree && ReEvaluateChaseTargetTag.IsValid())
		{
			StopMovement();
			StateTree->SendStateTreeEvent(ReEvaluateChaseTargetTag);
		}
	}
}

// Assigns Correct state tree to controller.
void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (!InPawn) return;
	
	if (AEnemyBaseCharacter* EnemyChar = Cast<AEnemyBaseCharacter>(InPawn))
	{
		EnemyChar->OnTakeAnyDamage.AddDynamic(this, &AEnemyAIController::Damaged);
		
		if (StateTree && EnemyChar->StateTreeAsset)
		{
			StateTree->StopLogic(TEXT("Possessed enemy changed"));
			StateTree->SetStateTree(EnemyChar->StateTreeAsset);
			StateTree->StartLogic();
		}
	}
}

void AEnemyAIController::OnUnPossess()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (AEnemyBaseCharacter* EnemyChar = Cast<AEnemyBaseCharacter>(ControlledPawn))
		{
			EnemyChar->OnTakeAnyDamage.RemoveDynamic(this, &AEnemyAIController::Damaged);
		}
	}
	
	if (StateTree) StateTree->StopLogic(TEXT("Dead Pawn"));
	
	// pawn is gone: stop the looping DistCheck timer we started in BeginPlay
	GetWorldTimerManager().ClearTimer(CheckDistanceTimerHandle);
	
	Super::OnUnPossess();
	
	// enemies never get a second pawn, so remove this controller too
	SetLifeSpan(0.1f);
}

// Changes State if damaged
void AEnemyAIController::Damaged(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!StateTree) return;
	if (!PlayerRef) PlayerRef = Cast<AEstivalBaseCharacter>(DamageCauser);
	
	if (ChangeToDefence.IsValid())
	{
		StopMovement();
		StateTree->SendStateTreeEvent(ChangeToDefence);
	}
}