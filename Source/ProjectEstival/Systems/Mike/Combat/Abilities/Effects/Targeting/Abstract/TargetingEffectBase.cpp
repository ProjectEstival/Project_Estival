#include "TargetingEffectBase.h"

void UTargetingEffectBase::ApplyOnTargets(const FAbilityContext& Context, TSet<AActor*> Targets) const
{
	TMap<TEnumAsByte<ETeamAttitude::Type>, int32> HitCounts;
	TSet<AActor*> AlreadyHit;

	for (AActor* Target : Targets)
	{
		if (!IsValid(Target) || AlreadyHit.Contains(Target)) continue;

		const ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(Context.Instigator, Target);

		const int32 Limit = bAllowsTargetLimits ? TargetLimits.FindRef(Attitude) : 0;
		if (Limit > 0 && HitCounts.FindRef(Attitude) >= Limit) continue;

		AlreadyHit.Add(Target);
		bool bPayloadApplied = false;

		for (UAbilityEffectPayload* Payload : EffectPayloads)
		{
			if (IsValid(Payload) && Payload->TryApply(Context, Target)) bPayloadApplied = true;
		}

		if (bPayloadApplied) HitCounts.FindOrAdd(Attitude)++;
	}
}