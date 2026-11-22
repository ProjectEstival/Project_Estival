#include "Systems/Thanos/PCG_Systems/Pcg Core/RoomProcessor.h"
#include "Engine/DataTable.h"

TArray<FRoomModule> FRoomProcessor::BuildModules(const UDataTable& RoomTable) {
	TArray<FRoomModule> Modules;

	RoomTable.ForeachRow<FRoomRow>(TEXT("DungeonRoomProcessor"), [&Modules](const FName& RowName, const FRoomRow& Row) {
		const FRoomModule Authored = MakeModule(RowName, Row, false);
		const FRoomModule Rotated = MakeModule(RowName, Row, true);

		Modules.Add(Authored);

		//Same doors after turning = duplicate, skip it
		if (Rotated.DoorMask != Authored.DoorMask) {
			Modules.Add(Rotated);
		}
	});

	LogMissingDoorLayouts(Modules);
	return Modules;
}

FRoomModule FRoomProcessor::MakeModule(FName RowName, const FRoomRow& Row, bool bRotated180) {
	FRoomModule Module;
	Module.RowName = RowName;
	Module.RoomType = Row.RoomType;
	Module.DoorMask = bRotated180 ? Direction::RotateDoorMask(Row.GetDoorMask()) : Row.GetDoorMask();
	Module.bRotated180 = bRotated180;
	//Never 0 :)
	Module.Weight = FMath::Max(Row.Weight, KINDA_SMALL_NUMBER);
	Module.ActorVariants = Row.ActorVariants;
	return Module;
}

void FRoomProcessor::LogMissingDoorLayouts(const TArray<FRoomModule>& Modules) {
	//4 sides with door or wall = 16 possible layouts mask 0 EMPTY to 15 OMNI
	for (uint8 DoorMask = 0; DoorMask < 16; DoorMask++) {
		const bool bCovered = Modules.ContainsByPredicate([DoorMask](const FRoomModule& Module) {
			return Module.RoomType == ERoomType::Normal && Module.DoorMask == DoorMask;
		});

		if (!bCovered) {
			UE_LOG(LogTemp, Warning, TEXT("Dungeon: no Normal room has the door layout %s, the solver can never place one"), *Direction::DoorMaskToString(DoorMask));
		}
	}
}