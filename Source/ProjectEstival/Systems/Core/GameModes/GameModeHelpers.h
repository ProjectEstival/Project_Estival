#pragma once

#include "CoreMinimal.h"
#include "GameModeHelpers.generated.h"

class UStatModifierSet;

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

UENUM(BlueprintType)
enum class EFloorModifierTarget : uint8
{
	Party,
	Enemies,
	Everyone
};

USTRUCT(BlueprintType)
struct FFloorModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floor")
	TObjectPtr<UStatModifierSet> Set;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floor")
	EFloorModifierTarget Target = EFloorModifierTarget::Party;
};

USTRUCT(BlueprintType)
struct FDungeonFloor
{
	GENERATED_BODY()

	/** Beating this floor's boss gets every downed hero back up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floor")
	bool bRevivesParty = false;

	/** Boss Fragments every player gets when this floor's boss dies. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floor", meta = (ClampMin = "0"))
	int32 BossFragments = 1;

	/** The pool this floor's modifiers are picked from. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floor")
	TArray<FFloorModifier> Modifiers;

	/** How many modifiers are picked from the pool, never the same one twice. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Floor", meta = (ClampMin = "0"))
	int32 ModifierCount = 1;
};