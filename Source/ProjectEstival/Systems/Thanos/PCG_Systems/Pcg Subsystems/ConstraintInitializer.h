#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/RoomModule.h"

class FGrid;
class FWave;
class URoomConstraint;

class FConstraintInitializer {
public:
	static bool BuildStartingWave(FWave& OutWave, const FGrid& Grid, const TArray<FRoomModule>& Modules,
		const TArray<TObjectPtr<URoomConstraint>>& Constraints, FRandomStream& Random); //List of rules
private:
	static void RemoveDoorsLeavingGrid(FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules);
};
