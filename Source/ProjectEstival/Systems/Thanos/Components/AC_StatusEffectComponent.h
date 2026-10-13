// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AC_StatusEffectComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectChanged, FGameplayTag, Tag);

UCLASS(ClassGroup=(Estival), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UAC_StatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category="Status")
	FOnStatusEffectChanged OnEffectAdded;

	UPROPERTY(BlueprintAssignable, Category="Status")
	FOnStatusEffectChanged OnEffectRemoved;

	/** Adds an effect.*/
	UFUNCTION(BlueprintCallable, Category="Status")
	void AddEffect(FGameplayTag Tag, float Duration = 0.0f); //If duration is <=0 it lasts until removed or refreshed.

	UFUNCTION(BlueprintCallable, Category="Status")
	void RemoveEffect(FGameplayTag Tag);

	UFUNCTION(BlueprintPure, Category="Status")
	bool HasEffect(FGameplayTag Tag) const { return ActiveEffects.HasTag(Tag); }

	UFUNCTION(BlueprintPure, Category="Status")
	bool HasAnyEffect(const FGameplayTagContainer& Tags) const { return ActiveEffects.HasAny(Tags); }

	UFUNCTION(BlueprintPure, Category="Status")
	const FGameplayTagContainer& GetActiveEffects() const { return ActiveEffects; }

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	FGameplayTagContainer ActiveEffects;
	
	TMap<FGameplayTag, FTimerHandle> Timers;
};