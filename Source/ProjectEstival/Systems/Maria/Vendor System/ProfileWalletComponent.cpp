// Fill out your copyright notice in the Description page of Project Settings.


#include "ProfileWalletComponent.h"
#include "Systems/Core/Profiles/PlayerProfileSaveGame.h"

bool UProfileWalletComponent::CanAfford_Implementation(const FGameplayTag Currency, const int32 Amount) const
{
	const FVendorProfileAccess Profile = FindProfile(Currency);
	return Profile.IsValid() && Profile.Save->Profile.UnspentFragments >= Amount;
}

bool UProfileWalletComponent::TrySpend_Implementation(const FGameplayTag Currency, const int32 Amount)
{
	const FVendorProfileAccess Profile = FindProfile(Currency);
	if (!Profile.IsValid() || Profile.Save->Profile.UnspentFragments < Amount) return false;
	
	Profile.Save->Profile.UnspentFragments -= Amount;
	Profile.Commit();
	return true;
}

FVendorProfileAccess UProfileWalletComponent::FindProfile(const FGameplayTag Currency) const
{
	if (!FragmentsCurrency.IsValid() || Currency != FragmentsCurrency) return {};
	
	return FVendorProfileAccess::Find(GetOwner());
}