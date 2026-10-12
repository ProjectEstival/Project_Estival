// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "CompanionStateTreeTasks.generated.h"

class AActor;
class APawn;

USTRUCT()
struct FCompanionFindFollowTargetInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;
	
	UPROPERTY(VisibleAnywhere, Category="Output")
	TObjectPtr<APawn> FollowTarget;
	
	UPROPERTY(VisibleAnywhere, Category="Output")
	float DistanceToTarget = 0.f;
	

};

/**
 *  This is a global task: it finds the nearest player-controlled pawn at every tick.
 *  Runs for the whole tree so every state can read its outputs.
 */
USTRUCT(meta=(DisplayName="Find Follow Target", Category="Companion"))
struct FCompanionFindFollowTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCompanionFindFollowTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:

	static void UpdateTarget(FInstanceDataType& InstanceData);
};

/** Instance data for Find Wander Point */
USTRUCT()
struct FCompanionFindWanderPointInstanceData
{
	GENERATED_BODY()

	/** Actor to wander around. Bind to Find Follow Target's FollowTarget */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AActor> Anchor;

	/** How far from the anchor that point may be */
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, Units="cm"))
	float Radius = 500.f;

	/** The chosen point */
	UPROPERTY(VisibleAnywhere, Category="Output")
	FVector WanderLocation = FVector::ZeroVector;
};

/**
 *  Picks a random reachable navmesh point near the anchor when its state starts.
 *  Keeps running so child states can bind to WanderLocation.
 */
USTRUCT(meta=(DisplayName="Find Wander Point", Category="Companion"))
struct FCompanionFindWanderPointTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FCompanionFindWanderPointTask()
	{
		// --- all the work happens in EnterState
		bShouldCallTick = false;
	}

	using FInstanceDataType = FCompanionFindWanderPointInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
