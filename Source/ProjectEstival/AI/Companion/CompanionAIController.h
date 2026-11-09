// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/Companion/CompanionTactics.h"
#include "CompanionAIController.generated.h"

class UStateTreeAIComponent;
class UAIPerceptionComponent;  

UCLASS(Abstract)
class PROJECTESTIVAL_API ACompanionAIController : public AAIController
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UStateTreeAIComponent> StateTreeAI;
	
	// let's the companions see the enemies  
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UAIPerceptionComponent> Perception;
	
	// transient: so it starts as nullptr every time it loads
	UPROPERTY(Transient)
	TObjectPtr<APawn> FollowTarget;
	
	// the enemy we follow and fight
	UPROPERTY(Transient)
	TObjectPtr<AActor> AttackTarget;
	
	// every enemy in the fight. Find54 Threat sets it, the target query reads it
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> Threats;
	
	// this bot's style: abilities, target rules and formation. 
	// Set it in BP_CompanionAIController
	UPROPERTY(EditAnywhere, Category="Companion|Tactics")
	TObjectPtr<UCompanionTactics> Tactics;

public:
	ACompanionAIController(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintPure, Category="Companion")
	APawn* GetFollowTarget() const {return FollowTarget;}
	
	void SetFollowTarget(APawn* NewTarget) { FollowTarget = NewTarget; }
	
	UFUNCTION(BlueprintPure, Category="Companion")
	AActor* GetAttackTarget() const { return AttackTarget; }

	void SetAttackTarget(AActor* NewTarget) { AttackTarget = NewTarget; }
	
	const TArray<TObjectPtr<AActor>>& GetThreats() const { return Threats; }
	void SetThreats(const TArray<TObjectPtr<AActor>>& NewThreats) { Threats = NewThreats; }
	
	UFUNCTION(BlueprintPure, Category="Companion")
	UCompanionTactics* GetTactics() const { return Tactics; }

	// swaps the style while playing. A menu or whoever can call it from a Blueprint later
	UFUNCTION(BlueprintCallable, Category="Companion")
	void SetTactics(UCompanionTactics* NewTactics) { Tactics = NewTactics; }
	
	// how fast the pawn turns to face when it walks. this is fix for the snapping issue on rotation. 
	UPROPERTY(EditAnywhere, Category = "Companion", meta=(ClampMin=0))
	float TurnSpeed = 10.f;
	float PawnTurnSpeed = 0.f;
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;


};
