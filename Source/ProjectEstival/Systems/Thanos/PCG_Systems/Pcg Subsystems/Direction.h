#pragma once

#include "CoreMinimal.h"

/**
* This enum counts the directions of a room clockwise so the opposite side is always 2 rotations away
* U is world X
*/
enum class EDirection : uint8 {
	Up,
	Right,
	Down,
	Left
};

namespace Direction {
	/**
	 * Inline helps with naming conflicts when this header is used in a script and share methods of the same name
	 * Constexpr bakes the value on runtime and is much lighter at runtime
	 */
	inline constexpr EDirection All[] = {EDirection::Up, EDirection::Right, EDirection::Down, EDirection::Left};
	
	inline EDirection Opposite(EDirection Direction) {
		return static_cast<EDirection>((static_cast<uint8>(Direction)+2) % 4);
	}
	
	inline uint8 ToBit(EDirection Direction) {
		return static_cast<uint8>(1 << static_cast<uint8>(Direction));
	}
	
	inline bool HasDoor(uint8 DoorMask, EDirection Direction) {
		return (DoorMask & ToBit(Direction)) != 0;
	}
	
	inline FIntPoint ToGridOffset(EDirection Direction) {
		switch (Direction) {
			case EDirection::Up: return FIntPoint(1, 0);
			case EDirection::Right: return FIntPoint(0, 1);
			case EDirection::Down: return FIntPoint(-1, 0);
			case EDirection::Left: return FIntPoint(0, -1);
		}
		return FIntPoint::ZeroValue;
	}
	
	inline uint8 RotateDoorMask(uint8 DoorMask) {
		uint8 Rotated = 0;
		for (EDirection Direction : All) {
			if (HasDoor(DoorMask, Direction)) {
				Rotated |= ToBit(Direction);
			}
		}
		return Rotated;
	}
	
	inline FString DoorMaskToString(uint8 DoorMask) {
		if (DoorMask == 0) {
			return TEXT("EMPTY");
		}
		if (DoorMask == 0b1111) {
			return TEXT("OMNI");
		}

		FString Result;
		if (HasDoor(DoorMask, EDirection::Up))    Result += TEXT("U");
		if (HasDoor(DoorMask, EDirection::Right)) Result += TEXT("R");
		if (HasDoor(DoorMask, EDirection::Down))  Result += TEXT("D");
		if (HasDoor(DoorMask, EDirection::Left))  Result += TEXT("L");
		return Result;
	}
}