// Fill out your copyright notice in the Description page of Project Settings.


#include "CompanionAIController.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Systems/Thanos/Components/AC_LookAtComponent.h"
#include "Components/StateTreeAIComponent.h"


// Sets default values
ACompanionAIController::ACompanionAIController(const FObjectInitializer& ObjectInitializer):
Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	
	// don't start it on BeginPlay because StateTree can start twice, on BeginPlay and OnPossess. 
	StateTreeAI->SetStartLogicAutomatically(false);
	
	bStartAILogicOnPossess = true;
	bAttachToPawn = true;
	
	Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 1800.f;
	
	// because of top-down camera
	SightConfig->PeripheralVisionAngleDegrees = 180.f;
	
	// see everyone
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	// registering sight with the perception component because otherwise the bot can't see anything
	Perception->ConfigureSense(*SightConfig);
	
	// when several senses report the same actor, use sight's location (only sight for now). 
	// Matters once hearing or damage are added
	Perception->SetDominantSense(SightConfig->GetSenseImplementation());
	
	// puts this component into Controllers perception slot
	SetPerceptionComponent(*Perception);
}

void ACompanionAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// LookAt faces the walk direction instantly by default (0), and AI paths
	// change direction a lot, so bots snap. Turn smoothly while we drive.
	if (UAC_LookAtComponent* LookAt = InPawn ? InPawn->FindComponentByClass<UAC_LookAtComponent>() : nullptr)
	{
		PawnTurnSpeed = LookAt->InterpSpeed;
		LookAt->InterpSpeed = TurnSpeed;
	}
}

void ACompanionAIController::OnUnPossess()
{
	// give the "body" back as we found it for example when a human player drops in
	if (UAC_LookAtComponent* LookAt = GetPawn() ? GetPawn()->FindComponentByClass<UAC_LookAtComponent>() : nullptr)
	{
		LookAt->InterpSpeed = PawnTurnSpeed;
	}

	Super::OnUnPossess();
}



