// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Boss/BossStateTreeTasks.h"
#include "AI/Boss/BossAIController.h"
#include "StateTreeExecutionContext.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Core/TagDefinitions.h"
#include "EngineUtils.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Core/TagDefinitions.h"

// ---------- Watch Phase ------------

EStateTreeRunStatus FBossWatchPhaseTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// update on enter too, so the outputs are valid before the first tick
	Update(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBossWatchPhaseTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	Update(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FBossWatchPhaseTask::Update(FInstanceDataType& InstanceData)
{
	const AEstivalBaseCharacter* Me = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	const UResourceComponent* Resources = Me ? Me->GetResources() : nullptr;
	const FGameplayTag HealthTag = TagDefinitions::Stats_Resources_Health;
	
	if (!Resources || !Resources->HasResource(HealthTag))
	{
		return;
	}

	InstanceData.HealthPercent = Resources->GetPercentage01(HealthTag);

	// phase 1, plus one for every threshold the health has dropped to
	int32 NewPhase = 1;
	for (const float Threshold : InstanceData.PhaseThresholds)
	{
		if (InstanceData.HealthPercent <= Threshold)
		{
			++NewPhase;
		}
	}

	// healing can never bring an old phase back
	InstanceData.HealthPhase = FMath::Max(InstanceData.HealthPhase, NewPhase);

	// pending until the controller has been told. The Howl state does that
	const ABossAIController* Boss = Cast<ABossAIController>(InstanceData.AIController.Get());
	InstanceData.bPhaseChangePending = Boss && InstanceData.HealthPhase > Boss->GetPhase();
}

// ----------- Announce Phase ------------
EStateTreeRunStatus FBossAnnouncePhaseTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (ABossAIController* Boss = Cast<ABossAIController>(InstanceData.AIController.Get()))
	{
		Boss->SetPhase(InstanceData.Phase);
	}

	// keeps running: the other task in the state decides when the state is over
	return EStateTreeRunStatus::Running;
}

// ----------- DEBUG ------------
#if !UE_BUILD_SHIPPING
// Estival.Boss.SetHealth 50: sets every boss to 50% health, to test phases quickly
static void BossSetHealth(const TArray<FString>& Args, UWorld* World)
{
	if (!World || Args.IsEmpty())
	{
		return;
	}

	const float Percent = FMath::Clamp(FCString::Atof(*Args[0]), 0.f, 100.f) / 100.f;
	const FGameplayTag HealthTag = TagDefinitions::Stats_Resources_Health;

	for (TActorIterator<ABossAIController> It(World); It; ++It)
	{
		const AEstivalBaseCharacter* Boss = Cast<AEstivalBaseCharacter>(It->GetPawn());
		if (UResourceComponent* Resources = Boss ? Boss->GetResources() : nullptr)
		{
			Resources->SetCurrent(HealthTag, Resources->GetMax(HealthTag) * Percent);
		}
	}
}

// registers the command when the game module loads
static FAutoConsoleCommandWithWorldAndArgs BossSetHealthCommand(
	TEXT("Estival.Boss.SetHealth"),
	TEXT("Sets every boss's health to a percent, 0-100. Example: Estival.Boss.SetHealth 50"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&BossSetHealth));
#endif