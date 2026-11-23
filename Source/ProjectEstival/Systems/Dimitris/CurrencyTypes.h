
#pragma once

#include "CoreMinimal.h"
#include "CurrencyTypes.generated.h"

UENUM(BlueprintType)
enum class ECurrencyStorage : uint8
{
	Run,
	Permanent
};

