// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VendorSocketComponent.generated.h"

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
};