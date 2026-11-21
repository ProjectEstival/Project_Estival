// Fill out your copyright notice in the Description page of Project Settings.

#include "RunWalletComponent.h"

#pragma region Public Methods
bool URunWalletComponent::CanAfford_Implementation(const FGameplayTag Currency, const int32 Amount) const
{
	return GetBalance(Currency) >= Amount;
}

bool URunWalletComponent::TrySpend_Implementation(const FGameplayTag Currency, const int32 Amount)
{
	if (Amount < 0 || !CanAfford_Implementation(Currency, Amount)) return false;

	SetBalance(Currency, GetBalance(Currency) - Amount);
	return true;
}

void URunWalletComponent::AddCurrency(const FGameplayTag Currency, const int32 Amount)
{
	if (!Currency.IsValid() || Amount <= 0) return;

	SetBalance(Currency, GetBalance(Currency) + Amount);
}

int32 URunWalletComponent::GetBalance(const FGameplayTag Currency) const
{
	return Balances.FindRef(Currency);
}

void URunWalletComponent::ResetAll()
{
	TArray<FGameplayTag> Held;
	Balances.GetKeys(Held);
	for (const FGameplayTag& Currency : Held) SetBalance(Currency, 0);

	Balances.Reset();
}
#pragma endregion

#pragma region Private Methods
void URunWalletComponent::SetBalance(const FGameplayTag Currency, const int32 NewAmount)
{
	const int32 OldAmount = GetBalance(Currency);
	if (OldAmount == NewAmount) return;

	Balances.Add(Currency, NewAmount);
	OnCurrencyChanged.Broadcast(Currency, NewAmount, OldAmount);
}
#pragma endregion