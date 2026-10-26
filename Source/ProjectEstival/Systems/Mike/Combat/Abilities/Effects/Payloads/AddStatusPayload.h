// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "AddStatusPayload.generated.h"

UCLASS()
class PROJECTESTIVAL_API UAddStatusPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status", meta = (Categories = "Status", ClampMin = 0))
	TMap<FGameplayTag, float> StatusDurations;
	
	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};