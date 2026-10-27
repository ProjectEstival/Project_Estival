#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MikeUtils.generated.h"

UCLASS()
class PROJECTESTIVAL_API UMikeUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static float RandomizeFloat(const float Val, float Percentage01);
};