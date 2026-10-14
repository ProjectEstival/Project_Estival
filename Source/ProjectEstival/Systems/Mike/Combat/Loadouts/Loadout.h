// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
 
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Loadout.generated.h"
 
class UStatModifierSet;
class UAbilityDefinition;
class UTexture2D;
 
UCLASS(BlueprintType)
class PROJECTESTIVAL_API ULoadout : public UDataAsset
{
	GENERATED_BODY()
 
public:
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText DisplayName;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info", meta = (MultiLine = true))
	FText Description;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	TObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<UStatModifierSet> StatModifiers;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout", meta = (Categories = "Ability."))
	TMap<FGameplayTag, TObjectPtr<UAbilityDefinition>> Abilities;
};