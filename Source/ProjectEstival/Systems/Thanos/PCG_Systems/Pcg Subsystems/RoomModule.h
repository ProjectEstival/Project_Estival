#pragma once

#include "CoreMinimal.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Direction.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/RoomRow.h"

/**The Rooms that the solver actually places.
* The solver only ever talks about modules, never about rows.
*/
struct FRoomModule
{
	//The DataTable row this came from
	FName RowName;

	ERoomType RoomType = ERoomType::Normal;

	//Doors after rotation, one bit per EDirection
	uint8 DoorMask = 0;

	//true = spawn the actor turned 180
	bool bRotated180 = false;

	float Weight = 1.0f;

	TArray<FActorVariant> ActorVariants;

	bool HasDoor(EDirection Direction) const
	{
		return Direction::HasDoor(DoorMask, Direction);
	}

	//No doors = EMPTY
	bool IsEmpty() const
	{
		return DoorMask == 0;
	}
};