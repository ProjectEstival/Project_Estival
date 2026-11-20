// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VendorData.h"
#include "VendorProfileGrant.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Profile Upgrade"))
class PROJECTESTIVAL_API UVendorProfileGrant : public UVendorGrant
{
	GENERATED_BODY()
	
public:
	
	virtual bool CanGrant_Implementation(AActor* Buyer) const override;
	virtual void Grant_Implementation(AActor* Buyer) const override;
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	FName ClassId = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor")
	FName UpgradeId = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor", meta = (ClampMin = "0"))
	int32 MaxLevel = 1;
};
