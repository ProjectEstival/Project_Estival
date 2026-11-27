// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "GameplayTagContainer.h"
#include "BossAIController.generated.h"

class UFMODEvent;
class UStateTreeAIComponent;
class UBossRotation;
class UAbilityDefinition;

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
	
	// the order of the boss's abilities. If empty the tree decides alone
	UPROPERTY(EditAnywhere, Category="Boss")
	TObjectPtr<UBossRotation> Rotation;
	
	UPROPERTY(EditAnywhere, Category="Music")
	TObjectPtr<UFMODEvent> Phase2Event;
	
	// what the rotation remembers between abilities
	FGameplayTag NextAbility;
	FGameplayTag LastAbility;
	int32 OpeningStep = 0;
	int32 BasicsSinceAbility = 0;
	double LastAbilityTime = 0.0;
	double NextPickedTime = 0.0;

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
	
	// the ability that the rotation wants next. Empty means no rotation
	UFUNCTION(BlueprintPure, Category="Boss|Rotation")
	FGameplayTag GetNextAbility() const { return NextAbility; }

	// true when enough basic attacks or enough time  passed since the last rotation ability
	UFUNCTION(BlueprintPure, Category="Boss|Rotation")
	bool IsAbilityTurn() const;

	// how far the target must be for the next ability
	void GetNextAbilityRange(float& OutMin, float& OutMax) const;

	// true when the next ability waited longer than the rotation's Patience
	bool IsNextAbilityOverdue() const;

	// gives up on the next ability and picks another one
	UFUNCTION(BlueprintCallable, Category="Boss|Rotation")
	void SkipNextAbility();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
private:
	void PickNextAbility();
	
	// can this slot be used before the boss gives up on it?
	bool IsReadyInTime(FGameplayTag Slot) const;

	// the loadout calls this every time the body uses an ability
	UFUNCTION()
	void HandleAbilityCommitted(FGameplayTag Slot, UAbilityDefinition* Ability);
	
};
