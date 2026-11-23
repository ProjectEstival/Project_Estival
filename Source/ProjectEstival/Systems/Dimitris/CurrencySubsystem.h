
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CurrencyTypes.h"
#include "CurrencySubsystem.generated.h"

class ULocalPlayer;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnCurrencyChanged,
	ULocalPlayer*, LocalPlayer,
	ECurrencyStorage, Storage,
	int32, Amount
);

UCLASS()
class PROJECTESTIVAL_API UCurrencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Currency")
	FOnCurrencyChanged OnCurrencyChanged;

	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddCurrency(
		ULocalPlayer* LocalPlayer,
		ECurrencyStorage Storage,
		int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetCurrency(
		ULocalPlayer* LocalPlayer,
		ECurrencyStorage Storage) const;

	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool TryConsumeCurrency(
		ULocalPlayer* LocalPlayer,
		ECurrencyStorage Storage,
		int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Currency")
	void ClearRunCurrency(ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintPure, Category = "Currency")
	static ULocalPlayer* GetLocalPlayerFromController(
		APlayerController* PlayerController);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// Temporary currency, keyed by profile ID.
	UPROPERTY(Transient)
	TMap<FGuid, int32> RunCurrencies;
	
	UFUNCTION()
	void HandleProfileAssigned(ULocalPlayer* LocalPlayer, FGuid ProfileId);
	
	UFUNCTION()
	void HandleProfileReleased(ULocalPlayer* LocalPlayer);

	UPROPERTY(Transient)
	TMap<ULocalPlayer*, FGuid> ActiveProfileIds;
};
