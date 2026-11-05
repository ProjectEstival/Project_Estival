#include "AreaTargetingEffect.h"
#include "Engine/World.h"

TArray<AActor*> UAreaTargetingEffect::GetTargets_Implementation(const FAbilityContext& Context) const
{
	TArray<AActor*> Targets;
	if (!Shape || !Context.World) return Targets;
	
	const FQuat Rotation = Context.Direction.ToOrientationQuat();
	
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityAreaTargeting), false);
	if (!bIncludeInstigator) Params.AddIgnoredActor(Context.Instigator.Get());
	
	Shape->CollectActors(Context.World, Context.Origin, Rotation, CollisionChannel, Params, Targets);
	
	if (bDrawDebug) Shape->DrawDebug(Context.World, Context.Origin, Rotation, 0.5f);
	return Targets;
}