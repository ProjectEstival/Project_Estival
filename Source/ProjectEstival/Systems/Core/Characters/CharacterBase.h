// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "Systems/Mike/Combat/ResourceComponent.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"
#include "Systems/Core/TagDefinitions.h"
#include "CharacterBase.generated.h"

class UInputAction;
class UAC_LookAtComponent;
class UAC_StatusEffectComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevived);

/**Base class for every Estival character*/
UCLASS(Abstract)
class PROJECTESTIVAL_API AEstivalBaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAC_LookAtComponent> LookAt;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAC_StatusEffectComponent> StatusEffects;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStatisticsComponent> Statistics;
	
	UPROPERTY(EditDefaultsOnly, Category= "Status", meta = (Categories = "Status."))
	FGameplayTagContainer MovementBlockingTags;

	UPROPERTY(EditDefaultsOnly, Category= "Status", meta = (Categories = "Status."))
	FGameplayTagContainer AbilityBlockingTags;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Resources")
	TObjectPtr<UResourceComponent> Resources;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Resources")
	TObjectPtr<ULoadoutComponent> Loadout;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditDefaultsOnly, Category= "Input")
	TMap<TObjectPtr<UInputAction>, FGameplayTag> AbilityInputMap;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	FGenericTeamId DefaultTeam = FGenericTeamId(0);

public:
	
	UPROPERTY(BlueprintAssignable, Category= "Events")
	FOnDeath OnDeath;
	
	UPROPERTY(BlueprintAssignable, Category= "Events")
	FOnRevived OnRevived;

	AEstivalBaseCharacter();
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category="Movement")
	void DoMove(float AxisX, float AxisY);
	
	UFUNCTION(BlueprintCallable, Category= "Combat", meta = (Categories = "Ability."))
	bool TryActivateAbility(FGameplayTag Slot);

	UFUNCTION(BlueprintPure, Category= "Character")
	bool CanMove() const;

	UFUNCTION(BlueprintPure, Category= "Character")
	bool CanUseAbilities() const;
	
	UFUNCTION(BlueprintCallable, Category= "Character")
	UStatisticsComponent* GetStatistics() const
	{
		return Statistics.Get();
	}
	
	UFUNCTION(BlueprintCallable, Category= "Character")
	UAC_StatusEffectComponent* GetStatusEffects() const
	{
		return StatusEffects.Get();
	}
	
	UFUNCTION(BlueprintCallable, Category= "Character")
	UResourceComponent* GetResources() const
	{
		return Resources.Get();
	}
	
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override
	{
		this->TeamId = TeamID;
	}
	virtual FGenericTeamId GetGenericTeamId() const override
	{
		return this->TeamId;
	}
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool TryRevive(float HealthPercent);
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsAlive() const;
	
private:
	UFUNCTION()
	void OnResourceChanged(FGameplayTag Stat, float NewValue, float OldValue, float MaxValue);
	UFUNCTION()
	void OnStatusEffectChanged(FGameplayTag Tag);
	UFUNCTION()
	void BindAbilityInput(UEnhancedInputComponent* Input, const UInputAction* Action, FGameplayTag AbilityTag);
	void Die();
	bool bIsDead = false;
	FGenericTeamId TeamId = FGenericTeamId::NoTeam;
};