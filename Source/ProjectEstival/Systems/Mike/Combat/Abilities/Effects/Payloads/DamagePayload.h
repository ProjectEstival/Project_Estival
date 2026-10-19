// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "DamagePayload.generated.h"
UCLASS()
class PROJECTESTIVAL_API UDamagePayload : public UAbilityEffectPayload
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
	virtual bool TryApply_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};
