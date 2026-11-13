// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Poolable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPoolable : public UInterface
{
	GENERATED_BODY()
};

class PROJECTESTIVAL_API IPoolable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooling")
	void OnAcquired();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pooling")
	void OnReleased();
};
