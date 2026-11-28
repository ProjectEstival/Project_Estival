#pragma once

#include "CoreMinimal.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/RoomModule.h"

class FWave;
class FGrid;

/**
 * Checks a solved wave is a playable dungeon
 */
class FValidator {
public:
	static bool IsPlayable(const FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, int32 MinRoomCount, TArray<int32>& OutReachable);

private:
	//Cell holding the START room, INDEX_NONE if there is none
	static int32 FindStartCell(const FWave& Wave, const TArray<FRoomModule>& Modules);

	//Every cell you can walk to from StartCell through doors
	static TArray<int32> FindReachableCells(int32 StartCell, const FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules);
};
