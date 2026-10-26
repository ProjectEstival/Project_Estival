// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "HitStopPayload.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTESTIVAL_API UHitStopPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", meta = (ClampMin = 0.01, ClampMax = 1))
	float Duration = .1f;
	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
	
public:
	static bool TryApplyHitStop(AActor* TargetActor, const float Duration);
};