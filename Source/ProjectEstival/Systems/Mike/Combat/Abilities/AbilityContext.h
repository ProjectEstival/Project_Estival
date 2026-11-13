#pragma once
 
#include "CoreMinimal.h"
#include "Systems/Mike/Combat/ResourceComponent.h"
#include "AbilityContext.generated.h"
 
class UStatisticsComponent;
class ULoadoutComponent;
class UAbilityDefinition;
class UAbilityActivation;

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
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UWorld> World;
	
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FVector Origin = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	FVector Direction = FVector::ForwardVector;

	/** True while Origin and Direction are the caster's own. Tasks that run effects later then use where the caster is and aims at that moment. Set it to false when you move Origin somewhere else. */
	UPROPERTY(BlueprintReadWrite, Category = "Ability")
	bool bFollowCaster = true;

	/** The use of the ability this context came from. Lets tasks and spawned actors belong to it. Empty once that use is over. */
	UPROPERTY()
	TWeakObjectPtr<UAbilityActivation> Activation;
};
