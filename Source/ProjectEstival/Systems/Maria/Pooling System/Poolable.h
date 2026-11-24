// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Poolable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPoolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Optional hooks for actors managed by UActorPoolSubsystem. Implement it on an actor that needs to
 * initialize itself when it is handed out and clean up when it goes back. Actors that don't implement
 * it still pool correctly and only get the generic reset.
 */
class PROJECTESTIVAL_API IPoolable
{
	GENERATED_BODY()

public:
	/**
	 * Called every time the pool hands the actor out, for a fresh spawn and for a reuse alike. The
	 * actor is alread as its requested transform and active. BeginPlay runs only once per actor, so 
	 * initialize per-use state here.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooling")
	void OnAcquired();

	/**
	 * Called when the actor is released, before the pool deactivates or destroys it. Stop effects,
	 * clear timers and drops references here.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooling")
	void OnReleased();
};
