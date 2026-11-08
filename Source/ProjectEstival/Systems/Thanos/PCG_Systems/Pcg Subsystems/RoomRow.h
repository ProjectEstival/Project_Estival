#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Templates/SubclassOf.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Direction.h"
#include "RoomRow.generated.h"

class AActor;

/**
 * Type of room either normal or player start room
 */
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Normal,
	Start
};

/**
 * The room blueprint that can be picked by weight
 */
USTRUCT(BlueprintType)
struct FActorVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Variant")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, Category = "Variant", meta = (ClampMin = 0.01))
	float Weight = 1.0f;
};

/**
 * The DT row of rooms, rooms get rotated by Direction.h
 */
USTRUCT(BlueprintType)
struct FRoomRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Room")
	ERoomType RoomType = ERoomType::Normal;

	//U = X
	UPROPERTY(EditAnywhere, Category = "Room|Doors")
	bool bDoorUp = false;

	//R = Y
	UPROPERTY(EditAnywhere, Category = "Room|Doors")
	bool bDoorRight = false;

	//D = -X
	UPROPERTY(EditAnywhere, Category = "Room|Doors")
	bool bDoorDown = false;

	//L = -Y
	UPROPERTY(EditAnywhere, Category = "Room|Doors")
	bool bDoorLeft = false;

	/**
	 * Weight determines how often the room is picked by the algo
	 */
	UPROPERTY(EditAnywhere, Category = "Room", meta = (ClampMin = 0.01))
	float Weight = 1.0f;

	/**
	 * If left empty it means we pick the EMPTY room
	 */
	UPROPERTY(EditAnywhere, Category = "Room")
	TArray<FActorVariant> ActorVariants;

	/**
	 *Puts the four bools into the mask, one bit per EDungeonDirection
	 */
	uint8 GetDoorMask() const
	{
		uint8 DoorMask = 0;
		if (bDoorUp)    DoorMask |= Direction::ToBit(EDirection::Up);
		if (bDoorRight) DoorMask |= Direction::ToBit(EDirection::Right);
		if (bDoorDown)  DoorMask |= Direction::ToBit(EDirection::Down);
		if (bDoorLeft)  DoorMask |= Direction::ToBit(EDirection::Left);
		return DoorMask;
	}
};

