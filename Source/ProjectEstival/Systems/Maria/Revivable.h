#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Revivable.generated.h"

UINTERFACE(BlueprintType)
class URevivable : public UInterface
{
	GENERATED_BODY()
};

class IRevivable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool TryRevive(float HealthPercent);
	virtual bool TryRevive_Implementation(float HealthPercent) { return true; }
};