// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"
#include "GameplayTagContainer.h"
#include "BossStateTreeTasks.generated.h"

class APawn;
class AAIController;
class AActor;


// ------------ Wartch Phase ---------------
USTRUCT()
struct FBossWatchPhaseInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;

	// health (0-1) where each next phase starts. 0.75, 0.5, 0.25 (4 phases)
	UPROPERTY(EditAnywhere, Category="Parameter")
	TArray<float> PhaseThresholds = {0.75f, 0.5f, 0.25f};

	UPROPERTY(VisibleAnywhere, Category="Output")
	float HealthPercent = 1.f;

	// the phase the health says we should be in. it only goes up
	UPROPERTY(VisibleAnywhere, Category="Output")
	int32 HealthPhase = 0;

	// true until the tree has announced the new phase. The Howl state does that
	UPROPERTY(VisibleAnywhere, Category="Output")
	bool bPhaseChangePending = false;
	
};

// global task: turns the boss's health into a phase number
USTRUCT(meta=(DisplayName="Boss Watch Phase", Category="Boss"))
struct FBossWatchPhaseTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBossWatchPhaseInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	static void Update(FInstanceDataType& InstanceData);
};


// ----------- Announce Phase ------------
USTRUCT()
struct FBossAnnouncePhaseInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;

	// bind to Boss Watch Phase's HealthPhase
	UPROPERTY(EditAnywhere, Category="Input")
	int32 Phase = 0;
};

// tells the controller the new phase, once, when its state is entered
USTRUCT(meta=(DisplayName="Boss Announce Phase", Category="Boss"))
struct FBossAnnouncePhaseTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBossAnnouncePhaseInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

// ----------- Find Targets ------------
USTRUCT()
struct FBossFindTargetsInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	// draws a line to the nearest target and a ball on every target
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bDrawDebug = false;

	// every living hero nearest first. 3 nearest = the first 3
	UPROPERTY(VisibleAnywhere, Category="Output")
	TArray<TObjectPtr<AActor>> Targets;

	// Scratch, Leap and every other single or area attack aims at this hero
	UPROPERTY(VisibleAnywhere, Category="Output")
	TObjectPtr<AActor> NearestTarget;

	UPROPERTY(VisibleAnywhere, Category="Output")
	FVector NearestLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category="Output")
	float DistanceToNearest = 0.f;

	UPROPERTY(VisibleAnywhere, Category="Output")
	bool bHasTarget = false;
};

// global task: finds who the boss can attack at every tick
USTRUCT(meta=(DisplayName="Boss Find Targets", Category="Boss"))
struct FBossFindTargetsTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBossFindTargetsInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	static void Update(FInstanceDataType& InstanceData);
};


// ------------ Ability Ready -----------
USTRUCT()
struct FBossAbilityReadyConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	UPROPERTY(EditAnywhere, Category="Parameter", meta=(Categories="Ability."))
	FGameplayTag Slot;
};

// true when the boss could use this slot right now: it has an ability and it is off cooldown
USTRUCT(DisplayName="Boss Ability Ready", Category="Boss")
struct FBossAbilityReadyCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBossAbilityReadyConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// -------- Use Ability -----------
USTRUCT()
struct FBossUseAbilityInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	// which loadout slot to use
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(Categories="Ability."))
	FGameplayTag Slot;

	// the ability goes where the boss faces
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bAimAtLocation = true;

	// where the ability aims
	UPROPERTY(EditAnywhere, Category="Parameter")
	FVector AimLocation = FVector::ZeroVector;
	
	// keep aiming at AimLocation while the ability runs (binds to NearestLocation).
	// effects that run later (the 2nd Charge dash for example) should go where the target is now
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(EditCondition="bAimAtLocation"))
	bool bKeepAiming = false;

	// stands still this long after the ability ends (the players' window to hit back?)
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, Units="s"))
	float RecoveryTime = 0.5f;

	// fail-safe: give up if the whole attack takes longer than this. 0 means no limit
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0, Units="s"))
	float MaxTime = 10.f;
	
	float Timer = 0.f;
	float RecoveryTimer = 0.f;
	bool bStarted = false;
};

// Aims uses one ability waits until it is over and then rests for RecoveryTime.
// Fails on an empty slot or after MaxTime
USTRUCT(meta=(DisplayName="Boss Use Ability", Category="Boss"))
struct FBossUseAbilityTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBossUseAbilityInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};


// ------------- Boss Rotation --------------
USTRUCT()
struct FBossRotationInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;
	
	// the slot the rotation wants next
	UPROPERTY(VisibleAnywhere, Category="Output")
	FGameplayTag NextSlot;
	
	// the distance the next ability needs on the floor
	UPROPERTY(VisibleAnywhere, Category="Output")
	float MinRange = 0.f;
	
	UPROPERTY(VisibleAnywhere, Category="Output")
	float MaxRange = 100000.f;
	
	// false while the boss still owes basic attacks
	UPROPERTY(VisibleAnywhere, Category="Output")
	bool bAbilityTurn = true;
};

// global task: shows the controller's rotation on the state tree and gives up  
// on abilities that wiat too long
USTRUCT(meta=(DisplayName="Boss Rotation", Category="Boss"))
struct FBossRotationTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBossRotationInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

private:
	static void Update(FInstanceDataType& InstanceData);
};