#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CurrencySubsystem.generated.h"

UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
	Red,
	Blue,
	Green
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCurrencyChanged,
	ECurrencyType, CurrencyType,
	int32, Amount
);

UCLASS()
class UCurrencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "Currency")
	FOnCurrencyChanged OnCurrencyChanged;

	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddCurrency(ECurrencyType Type, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCurrency(ECurrencyType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool TryConsumeCurrency(ECurrencyType Type, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Currency")
	void ClearAllCurrency();
	
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void ClearCurrency(ECurrencyType Type);

private:

	UPROPERTY()
	TMap<ECurrencyType, int32> CurrencyAmounts;
};