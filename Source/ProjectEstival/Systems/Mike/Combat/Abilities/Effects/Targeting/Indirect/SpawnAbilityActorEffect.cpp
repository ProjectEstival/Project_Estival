#include "SpawnAbilityActorEffect.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
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
	if (!Context.World || !ActorClass) return nullptr;

	AAbilityActor* Actor = Context.World->SpawnActorDeferred<AAbilityActor>(
		ActorClass,
		Transform,
		Context.Instigator,
		Cast<APawn>(Context.Instigator),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (Actor) Actor->InitializeAbilityActor(Context, this);
	return Actor;
}

void USpawnAbilityActorEffect::FinishSpawn(const FAbilityContext& Context, AAbilityActor* Actor, const FTransform& Transform) const
{
	if (!Actor) return;
	Actor->FinishSpawning(Transform);

	UAbilityActivation* Activation = Context.Activation.Get();
	if (bCancelOnInterrupt && Activation && Activation->IsActive()) Activation->OnInterrupted.AddUObject(Actor, &AAbilityActor::Cancel);
}
