// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "DamageEffect.generated.h"

UCLASS(Abstract, NotBlueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UDamageEffect : public UAbilityEffect
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Stats."))
	FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Stats.Damage"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 1, UIMin = 0, UIMax = 1))
	float DamageRandomization = 0.25f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.01))
	float DamageMultiplier = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass();
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	TSet<AActor*> GetTargets(const FAbilityContext& Context) const;
	virtual TSet<AActor*> GetTargets_Implementation(const FAbilityContext& Context) const { return TSet<AActor*>(); }
	virtual void Execute_Implementation(const FAbilityContext& Context) const override final;
};