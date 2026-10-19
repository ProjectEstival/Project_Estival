// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetingEffect.h"
#include "TargetSelf.generated.h"

UCLASS()
class PROJECTESTIVAL_API UTargetSelf : public UTargetingEffect
{
	GENERATED_BODY()
	
public:
	UTargetSelf() { bAllowTargetLimit = false; }
	virtual TSet<AActor*> GetTargets_Implementation(const FAbilityContext& Context) const override;
};