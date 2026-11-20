// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CurrencyWallet.h"
#include "Components/ActorComponent.h"
#include "ResourceWalletComponent.generated.h"

class UResourceComponent;
UCLASS(ClassGroup=(Vendor), meta=(BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UResourceWalletComponent : public UActorComponent, public ICurrencyWallet
{
	GENERATED_BODY()

public:
	virtual bool CanAfford_Implementation(FGameplayTag Currency, int32 Amount) const override;
	virtual bool TrySpend_Implementation(FGameplayTag Currency, int32 Amount) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UResourceComponent> Resources;
};