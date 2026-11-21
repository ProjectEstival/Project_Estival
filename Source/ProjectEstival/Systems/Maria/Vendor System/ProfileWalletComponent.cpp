// Fill out your copyright notice in the Description page of Project Settings.

#include "ProfileWalletComponent.h"
#include "Systems/Core/TagDefinitions.h"
#include "Systems/Core/Profiles/PlayerProfileSaveGame.h"

#pragma region Public Methods
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
#pragma endregion

#pragma region Private Methods
FVendorProfileAccess UProfileWalletComponent::FindProfile(const FGameplayTag Currency) const
{
	// resolved here and not in the constructor, because native tags may not be registered yet when components are constructed
	const FGameplayTag Fragments = FragmentsCurrency.IsValid() ? FragmentsCurrency : TagDefinitions::Currency_Perma_Fragments.GetTag();
	if (!Fragments.IsValid() || Currency != Fragments) return {};

	return FVendorProfileAccess::Find(GetOwner());
}
#pragma endregion