// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "KnockbackPayload.generated.h"

UCLASS()
class PROJECTESTIVAL_API UKnockbackPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (Units = "cm"))
	float KnockbackDistance = 1000;
	
public:
	virtual bool TryApply_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};