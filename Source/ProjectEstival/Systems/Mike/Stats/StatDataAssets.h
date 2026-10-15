#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StatTypes.h"
#include "Engine/DataAsset.h"
#include "StatDataAssets.generated.h"

UCLASS(BlueprintType)
class PROJECTESTIVAL_API UStatDefinitionSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (Categories = "Stats."))
	TMap<FGameplayTag, FStatDefinition> Stats;
};

UCLASS(BlueprintType)
class PROJECTESTIVAL_API UStatModifierSet : public UDataAsset, public IStatModifierProvider
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info", meta = (MultiLine = true))
	FText Description;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifiers",  meta = (Categories = "Stats."))
	TMap<FGameplayTag, FStatModifier> Modifiers;
	
	virtual TMap<FGameplayTag, FStatModifier> GetModifiers_Implementation() const override
	{
		return Modifiers;
	}
};