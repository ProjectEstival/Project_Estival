#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CurrencyWallet.h"
#include "ResourceWalletComponent.generated.h"

class UResourceComponent;

/** Stopgap wallet: exposes the owner's UResourceComponent as a currency wallet.
 * Delete it once the real currency component implements ICurrencyWallet.
 * */
UCLASS(ClassGroup = (Vendor), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UResourceWalletComponent : public UActorComponent, public ICurrencyWallet
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	virtual bool CanAfford_Implementation(FGameplayTag Currency, int32 Amount) const override;
	virtual bool TrySpend_Implementation(FGameplayTag Currency, int32 Amount) override;
#pragma endregion

protected:
	virtual void BeginPlay() override;

private:
#pragma region Private Fields
	UPROPERTY(Transient)
	TObjectPtr<UResourceComponent> Resources;
#pragma endregion
};