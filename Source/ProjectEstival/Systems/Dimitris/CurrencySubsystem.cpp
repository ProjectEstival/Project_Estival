#include "CurrencySubsystem.h"

void UCurrencySubsystem::AddCurrency(ECurrencyType Type, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	int32& CurrentAmount = CurrencyAmounts.FindOrAdd(Type);
	CurrentAmount += Amount;

	OnCurrencyChanged.Broadcast(Type, CurrentAmount);
}

int32 UCurrencySubsystem::GetCurrency(ECurrencyType Type) const
{
	return CurrencyAmounts.FindRef(Type);
}

bool UCurrencySubsystem::TryConsumeCurrency(ECurrencyType Type, int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	int32& CurrentAmount = CurrencyAmounts.FindOrAdd(Type);

	if (CurrentAmount < Amount)
	{
		return false;
	}

	CurrentAmount -= Amount;

	OnCurrencyChanged.Broadcast(Type, CurrentAmount);

	return true;
}

void UCurrencySubsystem::ClearCurrency()
{
	for (const auto& Entry : CurrencyAmounts)
	{
		OnCurrencyChanged.Broadcast(Entry.Key, 0);
	}

	CurrencyAmounts.Empty();
}