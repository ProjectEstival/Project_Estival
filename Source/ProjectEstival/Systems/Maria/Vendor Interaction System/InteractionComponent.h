// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionEvent);

/** Interactor side, lives on the player.
 * Finds the nearest IInteractable in range and runs instant or hold interactions on it.
 * */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	UInteractionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Interacts with the nearest valid target: instantly if its hold duration is 0,
	 * otherwise begins a hold. Returns false if nothing happened.
	 * */
	UFUNCTION(BlueprintCallable)
	bool StartInteract();
	
	/** Cancels the current hold, if any. Safe to call when no hold is active. */
	UFUNCTION(BlueprintCallable)
	void StopInteract();
	
	/** True while a hold is in progress. */
	UFUNCTION(BlueprintPure)
	bool IsHolding() const { return HoldDuration > 0.f; }
	
	/** Progress of the current hold from 0 to 1, or 0 when not holding. */
	UFUNCTION(BlueprintPure)
	float GetHoldProgress() const;
#pragma endregion
	
#pragma region Events
	
	/** A hold began. */
	UPROPERTY(BlueprintAssignable) FOnInteractionEvent OnHoldStarted;
	
	/** A hold ended early: released, out of range, target invalid or destroyed. */
	UPROPERTY(BlueprintAssignable) FOnInteractionEvent OnHoldCancelled;
	
	/** An interaction succeeded, instant or completed hold. */
	UPROPERTY(BlueprintAssignable) FOnInteractionEvent OnInteracted;
#pragma endregion

protected:
#pragma region Serialized Fields
	
	/** Radius around the owner searched for interactables. Also the range a hold must stay within. */
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0"))
	float InteractRange = 200.f;
	
	/** Collision channel used to find interactables. Targets must respond to it. */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractChannel = ECC_Pawn;
#pragma endregion
	
private:
#pragma region Private Fields
	TWeakObjectPtr<UObject> HoldTarget;
	float HoldElapsed = 0.f;
	float HoldDuration = 0.f;
#pragma endregion
	
#pragma region Private Methods
	UObject* FindNearestInteractable() const;
	AActor* GetTargetActor(const UObject* Target) const;
	bool IsHoldValid() const;
	void ResetHold();
#pragma endregion
};