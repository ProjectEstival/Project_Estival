#include "Systems/Thanos/PCG_Systems/Pcg Core/Validator.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/Grid.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Wave.h"
 
bool FValidator::IsPlayable(const FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, int32 MinRoomCount, TArray<int32>& OutReachable) {
	//Player Start
	const int32 StartCell = FindStartCell(Wave, Modules);
	if (StartCell == INDEX_NONE) {
		return false;
	}
 
	//Where players can walk from start
	OutReachable = FindReachableCells(StartCell, Wave, Grid, Modules);
 
	//Have we exceeded the minimum room requirement
	return OutReachable.Num() >= MinRoomCount;
}
 
int32 FValidator::FindStartCell(const FWave& Wave, const TArray<FRoomModule>& Modules) {
	for (int32 CellIndex = 0; CellIndex < Wave.NumCells(); ++CellIndex) {
		const int32 ModuleIndex = Wave.GetCollapsedModule(CellIndex);
		if (Modules[ModuleIndex].RoomType == ERoomType::Start) {
			return CellIndex;
		}
	}
	return INDEX_NONE;
}
 
TArray<int32> FValidator::FindReachableCells(int32 StartCell, const FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules) {
	//Checked ones
	TArray<int32> Visited = {StartCell};
	//Unchecked ones
	TArray<int32> ToVisit = {StartCell};
	
	while (!ToVisit.IsEmpty()) {
		const int32 CellIndex = ToVisit.Pop();
		const FRoomModule& Room = Modules[Wave.GetCollapsedModule(CellIndex)];
		
		for (EDirection Direction : Direction::All) {
			//No door on this side
			if (!Room.HasDoor(Direction)) {
				continue;
			}
			int32 NeighbourIndex = INDEX_NONE;
			if (!Grid.TryGetNeighbor(CellIndex,Direction, NeighbourIndex)) {
				continue;
			}
			if (Visited.Contains (NeighbourIndex)) {
				continue;
			}
			Visited.Add(NeighbourIndex);
			ToVisit.Add(NeighbourIndex);
		}
	}
	return Visited;
}
