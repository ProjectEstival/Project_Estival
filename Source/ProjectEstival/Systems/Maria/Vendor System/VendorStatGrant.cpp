// Fill out your copyright notice in the Description page of Project Settings.

#include "VendorStatGrant.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

#pragma region Public Methods
bool UVendorStatGrant::CanGrant_Implementation(AActor* Buyer) const { return Modifiers && Buyer && Buyer->FindComponentByClass<UStatisticsComponent>(); }

void UVendorStatGrant::Grant_Implementation(AActor* Buyer) const
{
	UStatisticsComponent* Stats = Buyer ? Buyer->FindComponentByClass<UStatisticsComponent>() : nullptr;
	if (!Stats) return;

	// this grant is the source, so RemoveAllFromSource(grant) takes back everything this item gave, and the game mode's floor modifiers are untouched
	Stats->ApplyModifierAsset(Modifiers, this);
}
#pragma endregion