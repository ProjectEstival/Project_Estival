// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "TargetingEffectBase.generated.h"

USTRUCT()
struct FTargetHitTracker
{
	GENERATED_BODY()
	
	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHit;
	
	UPROPERTY()
	TMap<TEnumAsByte<ETeamAttitude::Type>, int32> HitCounts;
};

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UTargetingEffectBase : public UAbilityEffect
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	bool bAllowsTargetLimits = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (ClampMin = 0, EditCondition = "bAllowsTargetLimits", EditConditionHides, HideEditConditionToggle))
	TMap<TEnumAsByte<ETeamAttitude::Type>, int32> TargetLimits;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TArray<TObjectPtr<UAbilityEffectPayload>> EffectPayloads;
	
public:
	bool TryApplyOnTarget(const FAbilityContext& Context, AActor* Target, FTargetHitTracker& Tracker) const;
	bool IsExhausted(const FTargetHitTracker& Tracker) const;
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ApplyOnTargets(const FAbilityContext& Context, const TArray<AActor*>& Targets) const;
};