// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "CompanionStateTreeTasks.generated.h"

class AActor;
class APawn;
class AAIController;

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

// instance data for the LookAt task
USTRUCT()
struct FCompanionLookAtInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	// who to face (enemy or companion)
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AActor> Target;

	/** Faced instead of Target while it is valid. Bind to SeenEnemy, or leave it unbound */
	UPROPERTY(EditAnywhere, Category="Input", meta=(Optional))
	TObjectPtr<AActor> PriorityTarget;
};

// turns the pawn to face a target while the state is active.
// uses the character's LookAt component because SetFocus does not rotate Estival characters
USTRUCT()
struct FCompanionLookAtTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCompanionLookAtInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

private:
	static void UpdateLook(const FInstanceDataType& InstanceData);
};

USTRUCT()
struct FCompanionFindThreatInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;
	
	// bind to Find Follow Target's FollowTarget
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<APawn> FollowTarget;
	
	// an enemy this close to the player or to me is "in the fight" (= close to the player or close to me)
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, Units="cm"))
	float EngageRadius = 600.f;
	
	// debug AttackTarget 
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bDrawDebug = false;
	
	// nearest enemy I can see. Used for looking
	UPROPERTY(VisibleAnywhere, Category="Output")
	TObjectPtr<AActor> SeenEnemy;
	
	// Nearest enemy that is fighting the player or me. Used for attacking
	UPROPERTY(VisibleAnywhere, Category="Output")
	TObjectPtr<AActor> AttackTarget;
};

// global task: reads what the companion sees and picks enemies to look at and to attack.
USTRUCT()
struct FCompanionFindThreatTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCompanionFindThreatInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};

// instance data for Use Ability
USTRUCT()
struct FCompanionUseAbilityInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	// Which loadout slot to use. Ability.Base is the basic attack 
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(Categories="Ability."))
	FGameplayTag Slot;
	
	// Target faced before the ability fires. Leave this unbound for abilities without a target
	UPROPERTY(EditAnywhere, Category="Input", meta=(Optional))
	TObjectPtr<AActor> Target;
	
	// the target must be inside this cone before we fire the ability 
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, ClampMax=180, Units="Degrees"))
	float AimAngle = 20.f;
	
	// give up after waiting this long, so the tree can pick again
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, Units="s"))
	float MaxWaitTime = 2.f;
	
	// code only
	float WaitTimer = 0.f;
};


 //  Uses an ability through the character, the same function player input calls.
 // Waits while the ability is busy or on cooldown, then succeeds once it fires.
// Fails on an empty slot or after waiting MaxWaitTime
USTRUCT()
struct FCompanionUseAbilityTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCompanionUseAbilityInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
private:
	static bool IsFacing(const AActor* Me, const AActor* Target, float AimAngle);
};


// instance data for CatchUp Teleport
USTRUCT()
struct FCompanionCatchUpTeleportInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;
	
	// bind this to Find Follow Target's Follow target in StateTree 
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AActor> FollowTarget;
	
	// never teleport inside this distance
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, Units="cm"))
	float LeashRadius = 900.f;
	
	// farther than this means we need to teleport
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, Units="cm"))
	float TeleportDistance = 2500.f;
	
	// if not getting closer for this long then we need to teleport, we're stuck
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, Units="s"))
	float StuckTime = 3.f;
	
	
	// not shown in Editor
	float StuckTimer =  0.f;
	float ClosestDistance = 0.f;
	
};


 // Global safety net: teleports the companion just off screen when it is
 // off screen AND (far behind OR stuck). It then walks back into view. 
USTRUCT()
struct FCompanionCatchUpTeleportTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCompanionCatchUpTeleportInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	static bool IsOnScreen(const UWorld* World, const FVector& Location);
	static bool TryTeleport(const FInstanceDataType& InstanceData);
};
