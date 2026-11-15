// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BossStateTreeTasks.generated.h"

class APawn;
class AAIController;


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
	int32 HealthPhase = 1;

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
	int32 Phase = 1;
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