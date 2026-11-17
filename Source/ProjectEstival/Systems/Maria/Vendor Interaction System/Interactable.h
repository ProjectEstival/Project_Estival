// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/** Anything an interactor (player) can use: vendors, downed teammates, doors. Implementable on actors and components. */
class PROJECTESTIVAL_API IInteractable
{
	GENERATED_BODY()

public:
#pragma region Interface Methods
	
	/** Checked when an interaction starts and on every tick of a hold. Returning false prevents or cancels it. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanInteract(AActor* Interactor) const;

	/** Seconds the interactor must hold the input. 0 means instant. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetHoldDuration() const;

	/** Performs the interaction. Returns whether it succeeded. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TryInteract(AActor* Interactor);

	/** Feedback hook: a hold on this target has begun. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoldStarted(AActor* Interactor);

	/** Feedback hook: a hold on this target was cancelled before completing. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoldCancelled(AActor* Interactor);
#pragma endregion
};