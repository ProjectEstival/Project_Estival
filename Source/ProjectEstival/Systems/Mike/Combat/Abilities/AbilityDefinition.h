#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilityBody.h"
#include "Systems/Mike/Stats/StatTypes.h"
#include "AbilityDefinition.generated.h"

class UStatisticsComponent;
class UStatModifierSet;

UCLASS(BlueprintType)
class PROJECTESTIVAL_API UAbilityDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText DisplayName;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info", meta = (MultiLine = true))
	FText Description;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	TObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0", Units = "s"))
	float Cooldown = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (Categories = "Stats.Resources", ClampMin = "0"))
	TMap<FGameplayTag, float> ResourceCosts;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<UAnimMontage> Montage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Casting", meta = (ClampMin = "0", Units = "s"))
	float CastTime = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Casting", meta = (Categories = "Stats."))
	TMap<FGameplayTag, FStatModifier> CastModifiers;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TArray<TObjectPtr<UAbilityPredicate>> Predicates;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TArray<TObjectPtr<UAbilityEffect>> Effects;
};