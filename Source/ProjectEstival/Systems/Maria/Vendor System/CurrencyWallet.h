#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CurrencyWallet.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UCurrencyWallet : public UInterface
{
	GENERATED_BODY()
};

/** Anything that holds spendable currencies, keyed by tag. Implementable on actors and components.
 * The vendor only talks to this, so the real currency system can replace UResourceWalletComponent by implementing it.
 * */
class PROJECTESTIVAL_API ICurrencyWallet
{
	GENERATED_BODY()

public:
#pragma region Interface Methods

	/** True if the wallet holds at least Amount of this currency. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanAfford(FGameplayTag Currency, int32 Amount) const;

	/** Removes Amount of the currency. Returns false and changes nothing if it can't be afforded. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TrySpend(FGameplayTag Currency, int32 Amount);
#pragma endregion
};