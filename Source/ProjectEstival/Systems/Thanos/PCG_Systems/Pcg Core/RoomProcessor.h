#pragma once

#include "CoreMinimal.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/RoomModule.h"

class UDataTable;

class FRoomProcessor {
public:
	static TArray<FRoomModule> BuildModules(const UDataTable& RoomTable);
private:
	static FRoomModule MakeModule(FName RowName, const FRoomRow& Row, bool bRotated180);
	static void LogMissingDoorLayouts(const TArray<FRoomModule>& Modules);
};