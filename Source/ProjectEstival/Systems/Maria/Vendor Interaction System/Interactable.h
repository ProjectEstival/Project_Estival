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

class PROJECTESTIVAL_API IInteractable
{
	GENERATED_BODY()

public:
	// Checked on start and every tick of a hold; returning false cancels it. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanInteract(AActor* Interactor) const;

	// 0 = instant, anything above = hold for that many seconds. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetHoldDuration() const;

	// Returns whether the interaction succeeded. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TryInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoldStarted(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoldCancelled(AActor* Interactor);
};
