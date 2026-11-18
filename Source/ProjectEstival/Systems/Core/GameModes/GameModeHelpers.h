#pragma once

#include "GameModeHelpers.generated.h"

UENUM(BlueprintType)
enum class EDungeonPhase : uint8
{
	LoadingFloor,
	Playing,
	FloorCleared,
	RunOver
};

UENUM(BlueprintType)
enum class ERunResult : uint8
{
	Won,
	Wiped,
	Left
};

USTRUCT(BlueprintType)
struct FDungeonFloor
{
	GENERATED_BODY()

	/** Beating this floor's boss gets every downed hero back up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floor")
	bool bRevivesParty = false;
};