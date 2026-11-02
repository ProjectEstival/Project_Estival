#include "TargetingEffect.h"
#include "Algo/Sort.h"

void UTargetingEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	TArray<AActor*> Targets = GetTargets(Context);
	
	if (bNearestFirst)
	{
		const FVector Origin = Context.Origin;
		Algo::SortBy(Targets, [Origin](const AActor* Target)
		{
			return FVector::DistSquared(Origin, Target->GetActorLocation());
		});
	}
	
	ApplyOnTargets(Context, Targets);
}