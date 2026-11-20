#include "Systems/Thanos/PCG_Systems/Pcg Core/UniqueRoomConstraint.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Wave.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/Grid.h"

bool UUniqueRoomConstraint::Apply(FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, FRandomStream& Random) const {
	const int32 ChosenCell = ChooseCell(Wave, Grid, Modules, Random);
	if (ChosenCell == INDEX_NONE) {
		UE_LOG(LogTemp, Error, TEXT("PCG URC: no cell can hold the %s room. Is there a row of that type"), *UEnum::GetValueAsString(RoomType));
		return false;
	}

	const auto IsRoomType = [&Modules, this](int32 ModuleIndex) {
		return Modules[ModuleIndex].RoomType == RoomType;
	};

	for (int32 CellIndex = 0; CellIndex < Wave.NumCells(); ++CellIndex) {
		if (CellIndex == ChosenCell) {
			//Only this room type here
			Wave.Restrict(CellIndex, IsRoomType);
		}
		else {
			//This room type nowhere else
			Wave.Restrict(CellIndex, [&IsRoomType](int32 ModuleIndex) { return !IsRoomType(ModuleIndex); });
		}
	}
	return true;
}

int32 UUniqueRoomConstraint::ChooseCell(const FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, FRandomStream& Random) const {
	if (Placement == EPlacement::Fixed) {
		if (!Grid.IsInBounds(FixedCell)) {
			return INDEX_NONE;
		}
		const int32 CellIndex = Grid.ToIndex(FixedCell);
		return CanHoldRoomType(Wave, CellIndex, Modules) ? CellIndex : INDEX_NONE;
	}

	TArray<int32> Candidates;
	for (int32 CellIndex = 0; CellIndex < Wave.NumCells(); ++CellIndex) {
		if (CanHoldRoomType(Wave, CellIndex, Modules)) {
			Candidates.Add(CellIndex);
		}
	}
	return Candidates.IsEmpty() ? INDEX_NONE : Candidates[Random.RandRange(0, Candidates.Num() - 1)];
}

bool UUniqueRoomConstraint::CanHoldRoomType(const FWave& Wave, int32 CellIndex, const TArray<FRoomModule>& Modules) const {
	return Wave.GetOptions(CellIndex).ContainsByPredicate([&Modules, this](int32 ModuleIndex) {
		return Modules[ModuleIndex].RoomType == RoomType;
	});
}