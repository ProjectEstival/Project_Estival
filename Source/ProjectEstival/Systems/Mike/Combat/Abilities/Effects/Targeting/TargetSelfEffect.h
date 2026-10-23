// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetingEffect.h"
#include "TargetSelfEffect.generated.h"

UCLASS()
class PROJECTESTIVAL_API UTargetSelfEffect : public UTargetingEffect
{
	GENERATED_BODY()
	
public:
	UTargetSelfEffect() { bAllowsTargetLimits = false; }
	virtual TSet<AActor*> GetTargets_Implementation(const FAbilityContext& Context) const override;
};