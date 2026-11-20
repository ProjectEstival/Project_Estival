// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CurrencyWallet.h"
#include "GameplayTagContainer.h"
#include "VendorProfileAccess.h"
#include "ProfileWalletComponent.generated.h"


UCLASS(ClassGroup=(Vendor), meta=(BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UProfileWalletComponent : public UActorComponent, public ICurrencyWallet
{
	GENERATED_BODY()

public:
	virtual bool CanAfford_Implementation(FGameplayTag Currency, int32 Amount) const override;
	virtual bool TrySpend_Implementation(FGameplayTag Currency, int32 Amount) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	FGameplayTag FragmentsCurrency;

private:
	FVendorProfileAccess FindProfile(FGameplayTag Currecny) const;
};
