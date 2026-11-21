// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CurrencyWallet.h"
#include "GameplayTagContainer.h"
#include "RunWalletComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRunCurrencyChanged, FGameplayTag, Currency, int32, NewAmount, int32, OldAmount);

/** One player's temporary currency for the current run (runes, coins...). It lives on the hero, so every player has their own pool, and it is never saved:
 * a new run loads a new hero and starts at 0. Only prices in these currencies are paid from it, so the permanent vendor never touches it.
 * */
UCLASS(ClassGroup = (Vendor), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API URunWalletComponent : public UActorComponent, public ICurrencyWallet
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	virtual bool CanAfford_Implementation(FGameplayTag Currency, int32 Amount) const override;
	virtual bool TrySpend_Implementation(FGameplayTag Currency, int32 Amount) override;

	/** Where earned currency comes in, for example when an enemy dies or a pickup is collected. Ignores amounts of 0 or less. */
	UFUNCTION(BlueprintCallable, Category = "Run Wallet")
	void AddCurrency(FGameplayTag Currency, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Run Wallet")
	int32 GetBalance(FGameplayTag Currency) const;

	/** Sets every balance back to 0. Only needed when a run restarts without loading a new hero. */
	UFUNCTION(BlueprintCallable, Category = "Run Wallet")
	void ResetAll();
#pragma endregion

#pragma region Events
	/** Fires on every change, for HUD counters. */
	UPROPERTY(BlueprintAssignable, Category = "Run Wallet")
	FOnRunCurrencyChanged OnCurrencyChanged;
#pragma endregion

private:
#pragma region Private Fields
	UPROPERTY(VisibleInstanceOnly, Category = "Run Wallet|Debug")
	TMap<FGameplayTag, int32> Balances;
#pragma endregion

#pragma region Private Methods
	void SetBalance(FGameplayTag Currency, int32 NewAmount);
#pragma endregion
};