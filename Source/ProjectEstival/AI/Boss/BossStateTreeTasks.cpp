// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Boss/BossStateTreeTasks.h"
#include "AI/Boss/BossAIController.h"
#include "StateTreeExecutionContext.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Core/TagDefinitions.h"
#include "EngineUtils.h"
#include "Algo/Sort.h"
#include "DrawDebugHelpers.h"
#include "Systems/Mike/Combat/Loadouts/AimComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	int32 NewPhase = 0;
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


// ------------ Find Targets ----------
EStateTreeRunStatus FBossFindTargetsTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	Update(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBossFindTargetsTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	Update(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FBossFindTargetsTask::Update(FInstanceDataType& InstanceData)
{
	// start fresh every time so a dead hero never stays as a target
	InstanceData.Targets.Reset();
	InstanceData.NearestTarget = nullptr;
	InstanceData.DistanceToNearest = 0.f;
	InstanceData.bHasTarget = false;

	const AEstivalBaseCharacter* Me = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	if (!Me)
	{
		return;
	}

	const FVector MyLocation = Me->GetActorLocation();

	// a hero = any living character on a team that is hostile to me. Companions count too
	for (TActorIterator<AEstivalBaseCharacter> It(Me->GetWorld()); It; ++It)
	{
		AEstivalBaseCharacter* Other = *It;
		if (Other == Me || !Other->IsAlive() || Me->GetTeamAttitudeTowards(*Other) != ETeamAttitude::Hostile)
		{
			continue;
		}

		InstanceData.Targets.Add(Other);
	}

	if (InstanceData.Targets.IsEmpty())
	{
		return;
	}

	// nearest first
	Algo::SortBy(InstanceData.Targets, [&MyLocation](const AActor* Target)
	{
		return FVector::DistSquared2D(MyLocation, Target->GetActorLocation());
	});

	InstanceData.NearestTarget = InstanceData.Targets[0];
	InstanceData.NearestLocation = InstanceData.NearestTarget->GetActorLocation();
	InstanceData.DistanceToNearest = static_cast<float>(FVector::Dist2D(MyLocation, InstanceData.NearestLocation));
	InstanceData.bHasTarget = true;

	if (InstanceData.bDrawDebug)
	{
		// red line: the nearest hero. Yellow balls: everyone the boss can attack
		DrawDebugLine(Me->GetWorld(), MyLocation, InstanceData.NearestLocation, FColor::Red, false, -1.f, 0, 4.f);
		for (const AActor* Target : InstanceData.Targets)
		{
			DrawDebugSphere(Me->GetWorld(), Target->GetActorLocation(), 60.f, 12, FColor::Yellow, false, -1.f, 0, 3.f);
		}
	}
}

// --------- Ability Ready-----------
bool FBossAbilityReadyCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ULoadoutComponent* Loadout = InstanceData.Pawn ? InstanceData.Pawn->FindComponentByClass<ULoadoutComponent>() : nullptr;
	
	return Loadout && Loadout->CanActivate(InstanceData.Slot);
}

// ---------- Use Ability ----------
EStateTreeRunStatus FBossUseAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// a fresh start every time the state is entered
	InstanceData.Timer = 0.f;
	InstanceData.RecoveryTimer = 0.f;
	InstanceData.bStarted = false;

	// fail now and not after MaxTime
	const AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	if (!Character || !Character->GetLoadout()->GetAbility(InstanceData.Slot))
	{
		UE_LOG(LogBossAI, Warning, TEXT("%s has nothing in slot %s"), *GetNameSafe(InstanceData.Pawn), *InstanceData.Slot.ToString());
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBossUseAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	if (!Character)
	{
		return EStateTreeRunStatus::Failed;
	}

	// safeguard: never get stuck in one attack
	InstanceData.Timer += DeltaTime;
	if (InstanceData.MaxTime > 0.f && InstanceData.Timer > InstanceData.MaxTime)
	{
		UE_LOG(LogBossAI, Warning, TEXT("%s gave up on slot %s"), *GetNameSafe(Character), *InstanceData.Slot.ToString());
		return EStateTreeRunStatus::Failed;
	}

	// not fired yet: aim then press the button
	if (!InstanceData.bStarted)
	{
		if (InstanceData.bAimAtLocation)
		{
			Character->GetAim()->SetAimLocation(InstanceData.AimLocation);
		}
		else
		{
			Character->GetAim()->ClearAim();
		}

		InstanceData.bStarted = Character->TryActivateAbility(InstanceData.Slot);
		return EStateTreeRunStatus::Running;
	}

	// fired: wait until the ability is over
	if (Character->GetLoadout()->IsBusy())
	{
		// abilities with late effects (Charge) should follow the target
		if (InstanceData.bAimAtLocation && InstanceData.bKeepAiming)
		{
			// while a dash moves the body aim the way it goes.
			// between dashes: aim at the target again
			const FVector Velocity = Character->GetVelocity();
			if (Character->GetCharacterMovement()->HasRootMotionSources() && !Velocity.IsNearlyZero())
			{
				Character->GetAim()->SetAimDirection(Velocity);
			}
			else
			{
				Character->GetAim()->SetAimLocation(InstanceData.AimLocation);
			}
		}
		return EStateTreeRunStatus::Running;
	}

	//recovery: stand still so the players can hit back
	InstanceData.RecoveryTimer += DeltaTime;
	return InstanceData.RecoveryTimer >= InstanceData.RecoveryTime ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

void FBossUseAbilityTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	if (!Character)
	{
		return;
	}

	// the tree left early so don't leave an ability running
	if (InstanceData.bStarted && Character->GetLoadout()->IsBusy())
	{
		Character->GetLoadout()->Interrupt();
	}

	// give the aim back: abilities go where the boss faces again
	Character->GetAim()->ClearAim();
}


// ------------- Boss Rotation --------------

EStateTreeRunStatus FBossRotationTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	Update(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBossRotationTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	Update(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FBossRotationTask::Update(FInstanceDataType& InstanceData)
{
	ABossAIController* Boss = Cast<ABossAIController>(InstanceData.AIController.Get());
	if (!Boss)
	{
		return;
	}
	
	// if the next ability waited too long
	if (Boss->IsNextAbilityOverdue())
	{
		Boss->SkipNextAbility();
	}
	
	InstanceData.NextSlot = Boss->GetNextAbility();
	InstanceData.bAbilityTurn = Boss->IsAbilityTurn();
	Boss->GetNextAbilityRange(InstanceData.MinRange, InstanceData.MaxRange);
}