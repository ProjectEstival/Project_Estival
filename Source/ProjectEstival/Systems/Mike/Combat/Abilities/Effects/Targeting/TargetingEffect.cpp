// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetingEffect.h"

void UTargetingEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	TSet<AActor*> AlreadyHit;
	int SuccessCounter = 0;
 
	for (AActor* Target : GetTargets_Implementation(Context))
	{
		if (TargetLimit > 0 && SuccessCounter >= TargetLimit) break;
		
		if (!Target || AlreadyHit.Contains(Target)) continue;
 
		AlreadyHit.Add(Target);
		
		bool PayloadApplied = false;
		
		for (const auto& Payload : EffectPayloads)
		{
			if (Payload->TryApply(Context, Target)) PayloadApplied = true;
		}
		
		if (PayloadApplied) SuccessCounter++;
	}
}