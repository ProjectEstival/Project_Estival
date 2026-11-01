#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Stats/StatDataAssets.h"
#include "BotUpgradeDefinition.generated.h"

class UTexture2D;

// One upgrade a bot can own.
UCLASS(BlueprintType)
class PROJECTESTIVAL_API UBotUpgradeDefinition : public UStatModifierSet
{
	GENERATED_BODY()

public:
	// Shown by the upgrade screen (UI task).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	TObjectPtr<UTexture2D> Icon;

	// Rune cost of a manual pick. 0 means free. Not used yet.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade", meta = (ClampMin = "0"))
	int32 Cost = 0;
};