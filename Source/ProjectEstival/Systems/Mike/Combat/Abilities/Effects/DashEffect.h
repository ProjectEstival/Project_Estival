// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "DashEffect.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTESTIVAL_API UDashEffect : public UAbilityEffect
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (Units = "cm"))
	float DashDistance = 2000;
	
public:
	virtual void Execute_Implementation(const FAbilityContext& Context) const override;
};