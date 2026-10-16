#pragma once
 
#include "CoreMinimal.h"
#include "Systems/Mike/Combat/ResourceComponent.h"
#include "AbilityContext.generated.h"
 
class UStatisticsComponent;
class ULoadoutComponent;
class UAbilityDefinition;
 
USTRUCT(BlueprintType)
struct FAbilityContext
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TObjectPtr<AActor> Instigator;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UStatisticsComponent> Stats;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UResourceComponent> Resources;
 
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TObjectPtr<ULoadoutComponent> Loadout;
 
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UAbilityDefinition> Ability;
};