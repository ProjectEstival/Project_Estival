// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VendorSocketComponent.generated.h"

class UVendorCatalog;

UENUM(BlueprintType)
enum class EVendorSocketType : uint8
{
	Product,
	Reroll
};

/** Marks where the vendor spawns an item. Add as many as the vendor should sell, move them freely in the Blueprint. */
UCLASS(ClassGroup = (Vendor), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UVendorSocketComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	/** Product sockets hold stock. Reroll sockets hold the reroll button and are ignored when the vendor has no reroll. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	EVendorSocketType Type = EVendorSocketType::Product;

	/** Product sockets only. Stock for this slot alone, for a slot that sells something other than the vendor's main catalog (the elixir slot). Empty = the vendor's catalog. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	TObjectPtr<UVendorCatalog> CatalogOverride;

	/** Product sockets only. Whether a reroll replaces this slot's item. Turn off for a slot that keeps its stock. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	bool bRerollable = true;
};