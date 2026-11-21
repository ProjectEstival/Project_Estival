// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CurrencyWallet.h"
#include "GameplayTagContainer.h"
#include "VendorProfileAccess.h"
#include "ProfileWalletComponent.generated.h"

/** Wallet for the currency kept in the player's saved profile (boss fragments). Spending is written to the profile on disk.
 * Can sit next to UResourceWalletComponent on the same hero: the vendor pays each price from the wallet that holds its currency.
 * */
UCLASS(ClassGroup = (Vendor), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UProfileWalletComponent : public UActorComponent, public ICurrencyWallet
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	virtual bool CanAfford_Implementation(FGameplayTag Currency, int32 Amount) const override;
	virtual bool TrySpend_Implementation(FGameplayTag Currency, int32 Amount) override;
#pragma endregion

protected:
#pragma region Serialized Fields
	/** The currency paid from the profile's unspent fragments. Leave empty to use Currency.Perma.Fragments, and set it only to override that. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	FGameplayTag FragmentsCurrency;
#pragma endregion

private:
#pragma region Private Methods
	/** The owner's profile when Currency is the fragments currency, otherwise an invalid access. */
	FVendorProfileAccess FindProfile(FGameplayTag Currency) const;
#pragma endregion
};