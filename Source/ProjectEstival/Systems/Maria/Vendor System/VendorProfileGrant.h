// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VendorData.h"
#include "VendorProfileGrant.generated.h"

/** Records a purchase in the buyer's saved profile, in the progression of one hero class: the upgrade is marked unlocked and its level goes up by one.
 * It only stores the purchase: whatever applies a class's owned upgrades at the start of a run reads them from the profile.
 * */
UCLASS(meta = (DisplayName = "Profile Upgrade"))
class PROJECTESTIVAL_API UVendorProfileGrant : public UVendorGrant
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	/** Refuses, without charging, when the buyer has no profile or the upgrade is already at MaxLevel. */
	virtual bool CanGrant_Implementation(AActor* Buyer) const override;
	virtual void Grant_Implementation(AActor* Buyer) const override;
#pragma endregion

protected:
#pragma region Serialized Fields
	/** The hero class whose progression gets the upgrade (FClassProgressionData::ClassId). Must match the id the rest of the profile code uses for that class. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	FName ClassId = NAME_None;

	/** The upgrade's key in that class's progression (FProfileUpgradeData::UpgradeId). Keep it stable once players have saves, since renaming it orphans their purchases. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	FName UpgradeId = NAME_None;

	/** Level one profile can reach, each purchase adding one: 1 for unlocks like abilities, 0 for no limit like stacking potions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor", meta = (ClampMin = "0"))
	int32 MaxLevel = 1;
#pragma endregion
};