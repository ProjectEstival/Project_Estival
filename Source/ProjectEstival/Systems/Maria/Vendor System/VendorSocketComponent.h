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

UCLASS(ClassGroup=(Vendor), meta=(BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UVendorSocketComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	EVendorSocketType Type = EVendorSocketType::Product;
};