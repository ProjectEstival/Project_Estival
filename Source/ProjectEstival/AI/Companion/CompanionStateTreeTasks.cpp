// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Companion/CompanionStateTreeTasks.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "AI/Companion/CompanionAIController.h"
#include "Systems/Thanos/Components/AC_LookAtComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "StateTreeExecutionContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogCompanionAI, Log, All);

// ---------- Find Follow Target ----------

EStateTreeRunStatus FCompanionFindFollowTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// update on enter too, so the outputs are valid before the first tick
	UpdateTarget(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FCompanionFindFollowTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	UpdateTarget(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FCompanionFindFollowTargetTask::UpdateTarget(FInstanceDataType& InstanceData)
{
	InstanceData.FollowTarget = nullptr;
	InstanceData.DistanceToTarget = 0.f;

	if (!InstanceData.Pawn)
	{
		return;
	}

	const FVector MyLocation = InstanceData.Pawn->GetActorLocation();
	double NearestDistSq = TNumericLimits<double>::Max();

	// a player is any pawn possessed by a PlayerController
	for (FConstPlayerControllerIterator It = InstanceData.Pawn->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
		if (!PlayerPawn)
		{
			continue;
		}

		const double DistSq = FVector::DistSquared(MyLocation, PlayerPawn->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			InstanceData.FollowTarget = PlayerPawn;
		}
	}

	if (InstanceData.FollowTarget)
	{
		InstanceData.DistanceToTarget = static_cast<float>(FMath::Sqrt(NearestDistSq));
	}
	
	// EQS can't read StateTree data, so sharing the target through the controller
	if (ACompanionAIController* Companion = Cast<ACompanionAIController>(InstanceData.Pawn->GetController()))
	{
		Companion->SetFollowTarget(InstanceData.FollowTarget);
	}
}

// ---------- Find Wander Point ----------

EStateTreeRunStatus FCompanionFindWanderPointTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.Anchor)
	{
		return EStateTreeRunStatus::Failed;
	}

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InstanceData.Anchor->GetWorld());
	FNavLocation Result;
	if (!NavSystem || !NavSystem->GetRandomReachablePointInRadius(InstanceData.Anchor->GetActorLocation(), InstanceData.Radius, Result))
	{
		UE_LOG(LogCompanionAI, Warning, TEXT("No reachable wander point near %s. Is there a navmesh?"), *InstanceData.Anchor->GetName());
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.WanderLocation = Result.Location;

	// keep running: child states bind to WanderLocation while this state is active
	return EStateTreeRunStatus::Running;
}

// ---------- Look At ----------

EStateTreeRunStatus FCompanionLookAtTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	UpdateLook(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FCompanionLookAtTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// targets move so update this every tick
	UpdateLook(Context.GetInstanceData(*this));
	return EStateTreeRunStatus::Running;
}

void FCompanionLookAtTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UAC_LookAtComponent* LookAt = InstanceData.Pawn ? InstanceData.Pawn->FindComponentByClass<UAC_LookAtComponent>() : nullptr;
	if (LookAt)
	{
		// back to facing the movement direction
		LookAt->ClearLookTarget();
	}
}

void FCompanionLookAtTask::UpdateLook(const FInstanceDataType& InstanceData)
{
	UAC_LookAtComponent* LookAt = InstanceData.Pawn ? InstanceData.Pawn->FindComponentByClass<UAC_LookAtComponent>() : nullptr;
	if (!LookAt)
	{
		return;
	}

	const AActor* LookTarget = IsValid(InstanceData.PriorityTarget) ? InstanceData.PriorityTarget.Get() : InstanceData.Target.Get();
	if (LookTarget)
	{
		LookAt->SetLookAtLocation(LookTarget->GetActorLocation());
	}
}

// ---------- Find Threat ----------
EStateTreeRunStatus FCompanionFindThreatTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.SeenEnemy = nullptr;
	InstanceData.AttackTarget = nullptr;

	const AEstivalBaseCharacter* Me = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	const UAIPerceptionComponent* Perception = InstanceData.AIController ? InstanceData.AIController->GetAIPerceptionComponent() : nullptr;
	if (!Me || !Perception)
	{
		return EStateTreeRunStatus::Running;
	}

	TArray<AActor*> SeenActors;
	Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);

	const double EngageRadiusSq = FMath::Square(InstanceData.EngageRadius);
	double NearestSeenSq = TNumericLimits<double>::Max();
	double NearestAttackSq = TNumericLimits<double>::Max();

	for (AActor* Actor : SeenActors)
	{
		// only living characters on a hostile team (never players or other bots!!)
		const AEstivalBaseCharacter* Enemy = Cast<AEstivalBaseCharacter>(Actor);
		if (!Enemy || !Enemy->IsAlive() || Me->GetTeamAttitudeTowards(*Enemy) != ETeamAttitude::Hostile)
		{
			continue;
		}

		const double DistSq = FVector::DistSquared(Me->GetActorLocation(), Enemy->GetActorLocation());
		if (DistSq < NearestSeenSq)
		{
			NearestSeenSq = DistSq;
			InstanceData.SeenEnemy = Actor;
		}

		// "in the fight" = close to the player or close to me
		const bool bNearMe = DistSq < EngageRadiusSq;
		const bool bNearPlayer = InstanceData.FollowTarget
			&& FVector::DistSquared(InstanceData.FollowTarget->GetActorLocation(), Enemy->GetActorLocation()) < EngageRadiusSq;

		if ((bNearMe || bNearPlayer) && DistSq < NearestAttackSq)
		{
			NearestAttackSq = DistSq;
			InstanceData.AttackTarget = Actor;
		}
	}

	// global task: keeps running for the whole tree
	return EStateTreeRunStatus::Running;
}

// ---------- Use Ability ----------
EStateTreeRunStatus FCompanionUseAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(InstanceData.Pawn);
	if (!Character || !InstanceData.Slot.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}
	
	if (Character->TryActivateAbility(InstanceData.Slot))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// not ready yet: try again next tick
	return EStateTreeRunStatus::Running;
}