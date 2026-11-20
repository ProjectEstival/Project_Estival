// Fill out your copyright notice in the Description page of Project Settings.

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
 *  The Vendor only talks to this, so the real currency system can replace UResourceWalletComponent by implementing it.*/
class PROJECTESTIVAL_API ICurrencyWallet
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanAfford(FGameplayTag Currency, int32 Amount) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TrySpend(FGameplayTag Currency, int32 Amount);
};