// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abstract/TargetingEffect.h"
#include "TargetSelfEffect.generated.h"

/** Applies its payloads to the caster. */
UCLASS(meta = (DisplayName = "Target Self"))
class PROJECTESTIVAL_API UTargetSelfEffect : public UTargetingEffect
{
	GENERATED_BODY()
	
public:
	UTargetSelfEffect() { bAllowsTargetLimits = false; }
	virtual TArray<AActor*> GetTargets_Implementation(const FAbilityContext& Context) const override;
};