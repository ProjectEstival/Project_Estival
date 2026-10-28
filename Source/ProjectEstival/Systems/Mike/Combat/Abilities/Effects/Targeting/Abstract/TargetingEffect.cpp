#include "TargetingEffect.h"

void UTargetingEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	ApplyOnTargets(Context, GetTargets(Context));
}