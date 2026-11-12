// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorPoolSubsystem.generated.h"

//Per-class pool configuration. Embed this in a data asset so each use case sets its own values.
USTRUCT(BlueprintType)
struct FActorPoolSettings
{
	GENERATED_BODY()
	
	// Maximum number of idle actors kept for this class. Actors released beyond this are destroyed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pooling", meta = (ClampMin = "0"))
	int32 MaxPoolSize = 10;
	
	// Seconds an acquired actor stays active before returning to the pool. 0 keeps it until released manually.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pooling", meta = (ClampMin = "0", Units = "s"))
	float Lifetime = 0.f;
};

USTRUCT()
struct FPooledActorList
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
	
	int32 MaxIdle = 0;
};

// Component state captured on release so reuse restores it instead of forcing everything on.
struct FSuspendedComponentState
{
	TWeakObjectPtr<UActorComponent> Component;
	bool bWasActive = false;
	bool bTickWasEnabled = false;
};

/** World-scoped pool for arbitrary actors. Pooled actors keep their own class; nothing needs to inherit from a special base.
 * Each class has its own list, capacity and lifetime, taken from FActorPoolSettings.
 * The pool is torn down together with the world, no map changes need no extra handling.
 */
UCLASS()
class PROJECTESTIVAL_API UActorPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Deinitialize() override;
	
	/**
	 * Returns an actor placed at Transform, reusing an idle pooled one if available and spawning otherwise.
	 If Settings.Lifetime is above 0 the actor is released automatically once it elapses.
	 A reused actor does not get BeginPlay again, so the caller resets any gameplay state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	AActor* AcquireActorFromSettings(TSubclassOf<AActor> ActorClass, FActorPoolSettings Settings, const FTransform& Transform);
	
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& Transform);
	
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	AActor* AcquireActorAtLocation(TSubclassOf<AActor> ActorClass, FVector Location, FRotator Rotation = FRotator::ZeroRotator);
	
	/** Same as AcquireActor, but takes a location and optional rotation instead of a full transform. Scale stays at 1. */
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	AActor* AcquireActorAtLocationFromSettings(TSubclassOf<AActor> ActorClass, const FActorPoolSettings& Settings, FVector Location, FRotator Rotation =
		                                           FRotator::ZeroRotator);

	/**
	 * Deactivates the actor and returns it to the pool of its class. If that pool already holds its
	 * maximum number of idle actors, the actor is destroyed instead.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void ReleaseActor(AActor* Actor);
	
	/**
	 * Deactivates the actor after Delay seconds. Calling it again for the same actor restarts the countdown,
	 * and releasing the actor manually cancels it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void ReleaseActorAfterDelay(AActor* Actor, float Delay);
	
	/**Spawns up to Count inactive actors (never more than Settings.MaxPoolSize) to avoid spawn hitches later. */
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void Prewarm(TSubclassOf<AActor> ActorClass, const FActorPoolSettings& Settings, int32 Count);
	
private:
	FPooledActorList& GetPool(TSubclassOf<AActor> ActorClass, const FActorPoolSettings& Settings);
	AActor* SpawnPooledActor(UClass* ActorClass, const FTransform& Transform) const;
	void ActivateActor(AActor* Actor, const FTransform& Transform);
	void DeactivateActor(AActor* Actor);
	
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FPooledActorList> Pools;
	
	TMap<TWeakObjectPtr<AActor>, FTimerHandle> PendingReleases;

	TMap<TWeakObjectPtr<AActor>, TArray<FSuspendedComponentState>> SuspendedComponents;
};