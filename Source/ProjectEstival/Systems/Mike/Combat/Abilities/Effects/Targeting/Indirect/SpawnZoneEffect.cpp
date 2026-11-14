#include "SpawnZoneEffect.h"
#include "Engine/World.h"
#include "Systems/Mike/Combat/Abilities/Actors/AbilityZone.h"

void USpawnZoneEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	if (!Context.World || !ZoneClass) return;

	const FTransform SpawnTransform = GetSpawnTransform(Context);

	AAbilityActor* Zone = BeginSpawn(Context, ZoneClass, SpawnTransform);
	if (!Zone) return;

	Zone->SetLifetime(Duration);
	FinishSpawn(Context, Zone);

	if (bAttachToCaster && IsValid(Context.Instigator) && !Zone->IsExpired()) Zone->AttachToActor(Context.Instigator, FAttachmentTransformRules::KeepWorldTransform);
}

void USpawnZoneEffect::CollectOccupants(const FAbilityContext& ZoneContext, TArray<AActor*>& OutActors) const
{
	if (!Shape || !ZoneContext.World) return;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityZone), false);
	if (!bIncludeInstigator) Params.AddIgnoredActor(ZoneContext.Instigator.Get());

	Shape->CollectActors(ZoneContext.World, ZoneContext.Origin, ZoneContext.Direction.ToOrientationQuat(), CollisionChannel, Params, OutActors);
}

void USpawnZoneEffect::RunTickEffects(const FAbilityContext& ZoneContext) const
{
	ExecuteAll(TickEffects, ZoneContext);
}

void USpawnZoneEffect::DrawDebug(const FAbilityContext& ZoneContext, const float ForDuration) const
{
	if (bDrawDebug && Shape) Shape->DrawDebug(ZoneContext.World, ZoneContext.Origin, ZoneContext.Direction.ToOrientationQuat(), ForDuration);
}
