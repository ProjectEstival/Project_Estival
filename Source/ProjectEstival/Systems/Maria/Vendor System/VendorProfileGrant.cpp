// Fill out your copyright notice in the Description page of Project Settings.

#include "VendorProfileGrant.h"
#include "VendorProfileAccess.h"
#include "Systems/Core/Profiles/PlayerProfileSaveGame.h"

#pragma region Public Methods
bool UVendorProfileGrant::CanGrant_Implementation(AActor* Buyer) const
{
	if (ClassId.IsNone() || UpgradeId.IsNone()) return false;

	const FVendorProfileAccess Profile = FVendorProfileAccess::Find(Buyer);
	if (!Profile.IsValid()) return false;

	const FClassProgressionData* Progression = Profile.Save->Profile.ClassProgressions.FindByPredicate([this](const FClassProgressionData& Entry)
	{
		return Entry.ClassId == ClassId;
	});
	const FProfileUpgradeData* Upgrade = Progression ? Progression->Upgrades.FindByPredicate([this](const FProfileUpgradeData& Entry)
	{
		return Entry.UpgradeId == UpgradeId;
	}) : nullptr;

	return MaxLevel == 0 || !Upgrade || Upgrade->UpgradeLevel < MaxLevel;
}

void UVendorProfileGrant::Grant_Implementation(AActor* Buyer) const
{
	const FVendorProfileAccess Profile = FVendorProfileAccess::Find(Buyer);
	if (!Profile.IsValid()) return;

	TArray<FClassProgressionData>& Classes = Profile.Save->Profile.ClassProgressions;
	FClassProgressionData* Progression = Classes.FindByPredicate([this](const FClassProgressionData& Entry)
	{
		return Entry.ClassId == ClassId;
	});
	if (!Progression)
	{
		Progression = &Classes.AddDefaulted_GetRef();
		Progression->ClassId = ClassId;
	}

	FProfileUpgradeData* Upgrade = Progression->Upgrades.FindByPredicate([this](const FProfileUpgradeData& Entry)
	{
		return Entry.UpgradeId == UpgradeId;
	});
	if (!Upgrade)
	{
		Upgrade = &Progression->Upgrades.AddDefaulted_GetRef();
		Upgrade->UpgradeId = UpgradeId;
	}

	Upgrade->bUnlocked = true;
	Upgrade->UpgradeLevel++;
	Profile.Commit();
}
#pragma endregion