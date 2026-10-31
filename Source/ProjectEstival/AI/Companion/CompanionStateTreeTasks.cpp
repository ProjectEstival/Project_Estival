// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Companion/CompanionStateTreeTasks.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "AI/Companion/CompanionAIController.h"
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

