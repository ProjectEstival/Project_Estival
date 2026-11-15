#include "RunEffectsPayload.h"
#include "GameFramework/Actor.h"

bool URunEffectsPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!IsValid(Target)) return false;

	FAbilityContext TargetContext = Context;
	TargetContext.Origin = Target->GetActorLocation();
	TargetContext.bFollowCaster = false;

	const FVector ToTarget = (TargetContext.Origin - Context.Origin).GetSafeNormal2D();
	if (!ToTarget.IsNearlyZero()) TargetContext.Direction = ToTarget;

	UAbilityEffect::ExecuteAll(Effects, TargetContext);
	return true;
}
