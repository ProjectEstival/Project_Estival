// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorPoolSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/MovementComponent.h"
#include "TimerManager.h"
#include "Systems/Core/EstivalGameInstance.h"

void UActorPoolSubsystem::Deinitialize()
{
	// The world destroys the actors itself; only the references need dropping.
	Pools.Empty();
	PendingReleases.Empty();
	SuspendedComponents.Empty();
	Super::Deinitialize();
}

AActor* UActorPoolSubsystem::AcquireActorFromSettings(TSubclassOf<AActor> ActorClass, FActorPoolSettings Settings, const FTransform& Transform)
{
	if (!ActorClass) return nullptr;
	
	FPooledActorList& Pool = GetPool(ActorClass, Settings);
	
	AActor* Actor = nullptr;
	while (!Actor && Pool.Actors.Num() > 0)
	{
		AActor* Candidate = Pool.Actors.Pop(EAllowShrinking::No);
		if (IsValid(Candidate))
		{
			ActivateActor(Candidate, Transform);
			Actor = Candidate;
		}
	}
	
	if (!Actor) Actor = SpawnPooledActor(ActorClass, Transform);
	
	if (Actor && Settings.Lifetime > 0.f) ReleaseActorAfterDelay(Actor, Settings.Lifetime);
	
	return Actor;
}

AActor* UActorPoolSubsystem::AcquireActor(const TSubclassOf<AActor> ActorClass, const FTransform& Transform)
{
	if (const auto GI = GetWorld()->GetGameInstance<UEstivalGameInstance>())
	{
		return AcquireActorFromSettings(ActorClass, GI->GetActorPoolSettings(ActorClass), Transform);
	}
	
	return SpawnPooledActor(ActorClass, Transform);
}

AActor* UActorPoolSubsystem::AcquireActorAtLocation(TSubclassOf<AActor> ActorClass, FVector Location, FRotator Rotation)
{
	if (const auto GI = GetWorld()->GetGameInstance<UEstivalGameInstance>())
	{
		return AcquireActorAtLocationFromSettings(ActorClass, GI->GetActorPoolSettings(ActorClass), Location, Rotation);
	}
	
	return SpawnPooledActor(ActorClass, FTransform(Rotation, Location));
}

AActor* UActorPoolSubsystem::AcquireActorAtLocationFromSettings(TSubclassOf<AActor> ActorClass, const FActorPoolSettings& Settings, FVector Location, FRotator Rotation)
{
	return AcquireActorFromSettings(ActorClass, Settings, FTransform(Rotation, Location));
}

void UActorPoolSubsystem::ReleaseActor(AActor* Actor)
{
	if (!IsValid(Actor)) return;
	
	if (FTimerHandle* Pending = PendingReleases.Find(Actor))
	{
		if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(*Pending);
		PendingReleases.Remove(Actor);
	}
	
	FPooledActorList* Pool = Pools.Find(Actor->GetClass());
	if (Pool && Pool->Actors.Contains(Actor)) return;
	
	// A class that was never acquired through settings has no pool and therefore no capacity.
	if (!Pool || Pool->Actors.Num() >= Pool->MaxIdle)
	{
		Actor->Destroy();
		return;
	}
	
	DeactivateActor(Actor);
	Pool->Actors.Add(Actor);
}

void UActorPoolSubsystem::ReleaseActorAfterDelay(AActor* Actor, float Delay)
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(Actor)) return;
	
	if (Delay <= 0.f)
	{
		ReleaseActor(Actor);
		return;
	}
	
	FTimerHandle& Handle = PendingReleases.FindOrAdd(Actor);
	World->GetTimerManager().ClearTimer(Handle);
	
	TWeakObjectPtr<AActor> WeakActor = Actor;
	World->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateWeakLambda(this,[this, WeakActor]()
		{
			PendingReleases.Remove(WeakActor);
			ReleaseActor(WeakActor.Get());
		}),
		Delay,
		false);
}

void UActorPoolSubsystem::Prewarm(const TSubclassOf<AActor> ActorClass, const FActorPoolSettings& Settings, int32 Count)
{
	if (!ActorClass) return;
	
	FPooledActorList& Pool = GetPool(ActorClass, Settings);
	const int32 ToSpawn = FMath::Min(Count, FMath::Min(Settings.MaxPoolSize, Pool.MaxIdle - Pool.Actors.Num()));
	for (int32 i = 0; i< ToSpawn; ++i)
	{
		if (AActor* Actor = SpawnPooledActor(ActorClass, FTransform::Identity))
		{
			DeactivateActor(Actor);
			Pool.Actors.Add(Actor);
		}
	}
}

FPooledActorList& UActorPoolSubsystem::GetPool(TSubclassOf<AActor> ActorClass, const FActorPoolSettings& Settings)
{
	FPooledActorList& Pool = Pools.FindOrAdd(ActorClass);
	
	// Data assets may list the same class with different caps; the largest wins.
	Pool.MaxIdle = FMath::Max(Pool.MaxIdle, Settings.MaxPoolSize);
	return Pool;
}

AActor* UActorPoolSubsystem::SpawnPooledActor(UClass* ActorClass, const FTransform& Transform) const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// Sublevel unloads would destroy pooled actors while the pool survives, so keep them in the persistent level.
	Params.OverrideLevel = World->PersistentLevel;
	
	return World->SpawnActor<AActor>(ActorClass, Transform, Params);
}

void UActorPoolSubsystem::ActivateActor(AActor* Actor, const FTransform& Transform)
{
	Actor->SetActorTransform(Transform, false, nullptr, ETeleportType::ResetPhysics);

	TArray<FSuspendedComponentState> Saved;
	if (SuspendedComponents.RemoveAndCopyValue(Actor, Saved))
	{
		for (const FSuspendedComponentState& State : Saved)
		{
			UActorComponent* Component = State.Component.Get();
			if (!Component) continue;

			if (State.bWasActive) Component->Activate();

			// Activate() force-enables the tick, so the original tick state is applied afterwards.
			Component->SetComponentTickEnabled(State.bTickWasEnabled);
		}
	}

	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);
}

void UActorPoolSubsystem::DeactivateActor(AActor* Actor)
{
	// Before collision is disabled, because a body with no collision may stop reporting as simulating.
	TArray<UPrimitiveComponent*> Primitives;
	Actor->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		if (Primitive->IsSimulatingPhysics())
		{
			Primitive->SetPhysicsLinearVelocity(FVector::ZeroVector);
			Primitive->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}
	}

	TArray<UActorComponent*> Components;
	Actor->GetComponents<UActorComponent>(Components);

	TArray<FSuspendedComponentState>& Saved = SuspendedComponents.FindOrAdd(Actor);
	Saved.Reset();
	for (UActorComponent* Component : Components)
	{
		FSuspendedComponentState& State = Saved.AddDefaulted_GetRef();
		State.Component = Component;
		State.bWasActive = Component->IsActive();
		State.bTickWasEnabled = Component->IsComponentTickEnabled();

		if (State.bWasActive)
		{
			if (UMovementComponent* Movement = Cast<UMovementComponent>(Component)) Movement->StopMovementImmediately();
			Component->Deactivate();
		}
		Component->SetComponentTickEnabled(false);
	}

	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);
}