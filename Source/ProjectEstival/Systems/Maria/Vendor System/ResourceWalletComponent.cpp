// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceWalletComponent.h"
#include "Systems/Mike/Combat/ResourceComponent.h"

void UResourceWalletComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Resources = GetOwner() ? GetOwner()->FindComponentByClass<UResourceComponent>() : nullptr;
	if (!Resources)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: ResourceWalletComponent needs a ResourceComponent on the same actor."),
			*GetNameSafe(GetOwner()));
	}
}

bool UResourceWalletComponent::CanAfford_Implementation(FGameplayTag Currency, const int32 Amount) const
{
	return Resources && Resources->CanAfford(Currency, Amount);
}

bool UResourceWalletComponent::TrySpend_Implementation(const FGameplayTag Currency, const int32 Amount)
{
	return Resources && Resources->TrySpend(Currency, Amount);
}