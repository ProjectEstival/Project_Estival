#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "VendorData.generated.h"

class UStaticMesh;
class UTexture2D;

/** One currency amount. The currency is any tag the buyer's wallet understands, so run and permanent currencies take the same path. */
USTRUCT(BlueprintType)
struct FVendorPrice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor")
	FGameplayTag Currency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor", meta = (ClampMin = "0"))
	int32 Amount = 0;
};

/** What a purchase gives the buyer: a temporary stat boost, an ability unlock, a potion...
 * Stateless, because one instance lives in the item asset and is shared by every vendor selling it.
 * */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UVendorGrant : public UObject
{
	GENERATED_BODY()

public:
	/** Checked before the buyer is charged, so a grant that can't apply (maxed, already unlocked) refuses the purchase for free. */
	UFUNCTION(BlueprintNativeEvent, Category = "Vendor")
	bool CanGrant(AActor* Buyer) const;
	virtual bool CanGrant_Implementation(AActor* Buyer) const { return true; }

	UFUNCTION(BlueprintNativeEvent, Category = "Vendor")
	void Grant(AActor* Buyer) const;
	virtual void Grant_Implementation(AActor* Buyer) const {}
};

/** Something a vendor can sell. Run vendors and permanent vendors use the same asset, they only differ in the currencies and grants inside. */
UCLASS(BlueprintType)
class PROJECTESTIVAL_API UVendorItemDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	TObjectPtr<UTexture2D> Icon;

	/** Shown on the vendor's socket. Leave empty to drive the visuals from the item Blueprint instead. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<UStaticMesh> DisplayMesh;

	/** Everything the buyer pays. One entry per currency. Empty = free. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	TArray<FVendorPrice> Prices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execution")
	TArray<TObjectPtr<UVendorGrant>> Grants;
};

USTRUCT(BlueprintType)
struct FVendorCatalogEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor")
	TObjectPtr<UVendorItemDefinition> Item;

	/** Relative chance of being offered. 0 = never. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor", meta = (ClampMin = "0"))
	float Weight = 1.f;
};

/** Pool a vendor draws its stock from. Different catalogs give run, permanent, or per-room vendors. */
UCLASS(BlueprintType)
class PROJECTESTIVAL_API UVendorCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	TArray<FVendorCatalogEntry> Entries;
};