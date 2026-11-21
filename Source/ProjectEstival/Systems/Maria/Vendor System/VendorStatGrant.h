// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VendorData.h"
#include "VendorStatGrant.generated.h"

class UStatModifierSet;

/** Applies a stat modifier set directly to the buyer's stats, for the in-run vendor's temporary boosts.
 * Nothing is saved: the modifiers live on the hero's StatisticsComponent and disappear with it when the run ends.
 * */
UCLASS(meta = (DisplayName = "Stat Boost"))
class PROJECTESTIVAL_API UVendorStatGrant : public UVendorGrant
{
	GENERATED_BODY()

public:
#pragma region Public Methods
	/** Refuses, without charging, when no modifiers are set or the buyer has no stats. */
	virtual bool CanGrant_Implementation(AActor* Buyer) const override;
	virtual void Grant_Implementation(AActor* Buyer) const override;
#pragma endregion

protected:
#pragma region Serialized Fields
	/** The same asset type the floor modifiers use. Buying the item again stacks another copy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	TObjectPtr<UStatModifierSet> Modifiers;
#pragma endregion
};