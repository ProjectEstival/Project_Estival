// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTESTIVAL_API IInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanInteract(AActor* Interactor) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetHoldDuration() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(AActor* Interactor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoldStarted(AActor* Interactor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHoldCancelled(AActor* Interactor);
};
