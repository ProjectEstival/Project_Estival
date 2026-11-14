#include "SpawnAbilityActorEffect.h"
#include "Engine/World.h"
#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"
#include "Systems/Mike/Combat/Abilities/Actors/AbilityActor.h"
#include "Systems/Mike/Combat/Loadouts/RuntimeAbilities.h"

void USpawnAbilityActorEffect::RunExpireEffects(const FAbilityContext& Context) const
{
	ExecuteAll(ExpireEffects, Context);
}

FTransform USpawnAbilityActorEffect::GetSpawnTransform(const FAbilityContext& Context) const
{
	const FVector Forward = Context.Direction.IsNearlyZero() ? FVector::ForwardVector : Context.Direction;
	return FTransform(Forward.Rotation(), Context.Origin + Forward.ToOrientationQuat().RotateVector(SpawnOffset));
}

AAbilityActor* USpawnAbilityActorEffect::BeginSpawn(const FAbilityContext& Context, const TSubclassOf<AAbilityActor> ActorClass, const FTransform& Transform) const
{
	UActorPoolSubsystem* Pool = UWorld::GetSubsystem<UActorPoolSubsystem>(Context.World);
	if (!Pool || !ActorClass) return nullptr;

	AAbilityActor* Actor = Cast<AAbilityActor>(Pool->AcquireActor(ActorClass, Transform));
	if (Actor) Actor->InitializeAbilityActor(Context, this);
	return Actor;
}

void USpawnAbilityActorEffect::FinishSpawn(const FAbilityContext& Context, AAbilityActor* Actor) const
{
	if (!Actor) return;
	Actor->StartAbilityActor();

	UAbilityActivation* Activation = Context.Activation.Get();
	if (bCancelOnInterrupt && !Actor->IsExpired() && Activation && Activation->IsActive()) Activation->OnInterrupted.AddUObject(Actor, &AAbilityActor::Cancel);
}
