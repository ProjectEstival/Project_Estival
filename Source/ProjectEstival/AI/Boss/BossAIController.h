// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "BossAIController.generated.h"

class UStateTreeAIComponent;

// create one log category for all boss AI code. 
DECLARE_LOG_CATEGORY_EXTERN(LogBossAI, Log, All);

// fires once, when the fight begins. External systems can bind to it 
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossFightStarted);

// this fires every time the boss enters a new phase
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossPhaseChanged, int32, NewPhase, int32, OldPhase);

UCLASS()
class PROJECTESTIVAL_API ABossAIController : public AAIController
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UStateTreeAIComponent>StateTreeAI;
	
	// true: the fight starts as soon as the Boss has a body
	// turn it of once a level triggers calls StartFight
	UPROPERTY(EditAnywhere, Category="Boss")
	bool bAutoStartFight = true;
	
	bool bFightStarted = false;
	int32 Phase = 0;

public:
	ABossAIController();
	
	// starts the Boss's State Tree
	// a room trigger, a cutscene or a debug commands calls it 
	UFUNCTION(BlueprintCallable, Category="Boss")
	void StartFight();
	
	UFUNCTION(BlueprintPure, Category="Boss"	)
	bool IsFightStarted() const { return bFightStarted; }
	
	// first phase = 0 and it only goes up
	UFUNCTION(BlueprintPure, Category="Boss")
	int32 GetPhase() const { return Phase; }
	
	// the StateTree calls this when the Boss announces a new phase.
	void SetPhase(int32 NewPhase);
	
	UPROPERTY(BlueprintAssignable, Category="Boss")
	FOnBossFightStarted OnFightStarted;
	
	UPROPERTY(BlueprintAssignable, Category="Boss")
	FOnBossPhaseChanged OnPhaseChanged;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	
};
