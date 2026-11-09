// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetingEffectBase.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "TargetingEffect.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories, HideFunctions = (Execute))
class PROJECTESTIVAL_API UTargetingEffect : public UTargetingEffectBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	TArray<AActor*> GetTargets(const FAbilityContext& Context) const;
	virtual TArray<AActor*> GetTargets_Implementation(const FAbilityContext& Context) const { return TArray<AActor*>(); }
	virtual void Execute_Implementation(const FAbilityContext& Context) const override final;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting")
	bool bNearestFirst = true;
};