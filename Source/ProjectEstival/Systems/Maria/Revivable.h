// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Revivable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class URevivable : public UInterface
{
	GENERATED_BODY()
};

class PROJECTESTIVAL_API IRevivable
{
	GENERATED_BODY()

	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resurrection")
	bool CanBeRevived() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Resurrection")
	bool ReviveCharacter(float HealthPercent);
};
