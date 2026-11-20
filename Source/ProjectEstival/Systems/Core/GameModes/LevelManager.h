#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LevelManager.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class ULevelManager : public UInterface
{
	GENERATED_BODY()
};

class PROJECTESTIVAL_API ILevelManager
{
	GENERATED_BODY()

public:
	/** Build this floor in place of the last one, then call FloorReady on the dungeon game mode. */
	UFUNCTION(BlueprintNativeEvent, Category = "Run")
	void BuildFloor(int32 Floor);
};
