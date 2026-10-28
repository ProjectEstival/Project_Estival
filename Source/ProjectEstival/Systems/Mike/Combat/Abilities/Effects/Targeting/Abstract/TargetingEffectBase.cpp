#include "TargetingEffectBase.h"

bool UTargetingEffectBase::TryApplyOnTarget(const FAbilityContext& Context, AActor* Target, FTargetHitTracker& Tracker) const
{
	if (!IsValid(Target) || Tracker.AlreadyHit.Contains(Target)) return false;

	const ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(Context.Instigator, Target);

	const int32 Limit = bAllowsTargetLimits ? TargetLimits.FindRef(Attitude) : 0;
	if (Limit > 0 && Tracker.HitCounts.FindRef(Attitude) >= Limit) return false;

	Tracker.AlreadyHit.Add(Target);
	bool bPayloadApplied = false;

	for (UAbilityEffectPayload* Payload : EffectPayloads)
	{
		if (IsValid(Payload) && Payload->TryApply(Context, Target)) bPayloadApplied = true;
	}

	if (!bPayloadApplied) return false;
	
	Tracker.HitCounts.FindOrAdd(Attitude)++;
	Tracker.TotalHits++;
	return true;
}

void UTargetingEffectBase::ApplyOnTargets(const FAbilityContext& Context, const TArray<AActor*>& Targets) const
{
	FTargetHitTracker Tracker;

	for (AActor* Target : Targets)
	{
		TryApplyOnTarget(Context, Target, Tracker);
	}
}