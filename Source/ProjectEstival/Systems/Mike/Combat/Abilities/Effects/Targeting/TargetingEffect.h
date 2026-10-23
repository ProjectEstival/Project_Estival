// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "TargetingEffect.generated.h"

UCLASS(Abstract, NotBlueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UTargetingEffect : public UAbilityEffect
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	bool bAllowsTargetLimits = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, EditCondition = "bAllowsTargetLimits", EditConditionHides, HideEditConditionToggle))
	TMap<TEnumAsByte<ETeamAttitude::Type>, int32> TargetLimits;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TArray<TObjectPtr<UAbilityEffectPayload>> EffectPayloads;
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	TSet<AActor*> GetTargets(const FAbilityContext& Context) const;
	virtual TSet<AActor*> GetTargets_Implementation(const FAbilityContext& Context) const { return TSet<AActor*>(); }
	virtual void Execute_Implementation(const FAbilityContext& Context) const override final;
};