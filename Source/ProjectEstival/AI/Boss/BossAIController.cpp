// Fill out your copyright notice in the Description page of Project Settings.


#include "BossAIController.h"
#include "Components/StateTreeAIComponent.h"

DEFINE_LOG_CATEGORY(LogBossAI);

// Sets default values
ABossAIController::ABossAIController()
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	
	// don't start it on BeginPlay because StateTree can start twice, on BeginPlay and OnPossess. 
	StateTreeAI->SetStartLogicAutomatically(false);
	bStartAILogicOnPossess = false;
	
	// this keeps the controller where the pawn is, so location checks and EQS work as excpected.
	bAttachToPawn = true;
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (bAutoStartFight)
	{
		StartFight();
	}
}

void ABossAIController::OnUnPossess()
{
	// stop the tree while it still has its pawn otherwise StopLogic fails
	StateTreeAI->StopLogic(TEXT("Unpossessed"));
	Super::OnUnPossess();
}

void ABossAIController::StartFight()
{
	if (bFightStarted || !GetPawn())
	{
		return;
	}
	
	bFightStarted = true;
	StateTreeAI->StartLogic();
	OnFightStarted.Broadcast();
	
	UE_LOG(LogBossAI, Log, TEXT("%s: the fight started"), *GetNameSafe(GetPawn()));
}


void ABossAIController::SetPhase(const int32 NewPhase)
{
	// phase only increase
	if (NewPhase <= Phase)
	{
		return;
	}
	
	const int32 OldPhase = Phase;
	Phase = NewPhase;
	OnPhaseChanged.Broadcast(NewPhase, OldPhase);
	
	UE_LOG(LogBossAI, Log, TEXT("%s: phase %d -> %d"), *GetNameSafe(GetPawn()), OldPhase, NewPhase);
}


