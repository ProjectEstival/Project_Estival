// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Thanos/PCG_Systems/Pcg Core/Solver.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/Grid.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Wave.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Random.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Direction.h"

Solver::Solver(FWave& InWave, const FGrid& InGrid, const TArray<FRoomModule>& InModules, FRandomStream& InRandom): Wave(InWave), Grid(InGrid), Modules(InModules), Random(InRandom) {
}

#pragma region METHODS

const bool Solver::Solve() {
	
	TArray<int32> CellsToVisit;
	for (int32 CellIndex = 0; CellIndex < Wave.NumCells(); ++CellIndex) {
		if (Wave.GetOptions(CellIndex).Num()==0) {
			return false;
		}
		CellsToVisit.Add(CellIndex);
	}
	
	if (!Propagate(CellsToVisit)) {
		return false;
	}
 
	while (true) {
		const int32 CellIndex = FindLowestEntropyCell();
		if (CellIndex == INDEX_NONE) {
			return true;
		}
 
		Collapse(CellIndex);
 
		CellsToVisit = { CellIndex };
		if (!Propagate(CellsToVisit)) {
			return false;
		}
	}
	
}

//Which cell to decide next
const int32 Solver::FindLowestEntropyCell() {
	int32 BestCell = INDEX_NONE;
	int32 BestCount = MAX_int32;

	for (int32 CellIndex = 0; CellIndex < Wave.NumCells(); ++CellIndex) { //If we dont skip on 1 here, the editor freezes in an infinite loop.
		//If cell already decided, skip
		if (Wave.IsCollapsed(CellIndex)) {
			continue;
		}
		//How many options it has?
		const int32 Count = Wave.GetOptions(CellIndex).Num();
		//If fewer than BestCount, remember cell and its count
		if (Count < BestCount) {
			BestCell = CellIndex;
			BestCount = Count;
		}
	}

	return BestCell;
}

//Decide this cell
void Solver::Collapse(int32 CellIndex) {
	//Modules this cell can still become
	const TArray<int32>& Options = Wave.GetOptions(CellIndex);
 
	//One weight per option
	TArray<float> Weights;
	Weights.Reserve(Options.Num());
	for (int32 ModuleIndex : Options) {
		Weights.Add(Modules[ModuleIndex].Weight);
	}
 
	//Position in Options
	const int32 PickedPosition = Random::PickWeightedIndex(Weights, Random);
	const int32 ChosenModule = Options[PickedPosition];
 
	Wave.CollapseTo(CellIndex, ChosenModule);
}

//Tell the neighbouring cells
bool Solver::Propagate(TArray<int32>& CellsToVisit) {
	/*return true; */
	while (!CellsToVisit.IsEmpty()) {
		//Take one cell off the list
		const int32 CellIndex = CellsToVisit.Pop();
		const TArray<int32>& CellOptions = Wave.GetOptions(CellIndex);
 
		for (EDirection Direction : Direction::All) {
			//Edge of the grid, nobody to tell
			int32 NeighborIndex = INDEX_NONE;
			if (!Grid.TryGetNeighbor(CellIndex, Direction, NeighborIndex)) {
				continue;
			}
 
			//Neighbour keeps an option only if at least one of our options fits it on the shared side
			const bool bChanged = Wave.Restrict(NeighborIndex, [&](int32 NeighborModule) {
				return CellOptions.ContainsByPredicate([&](int32 CellModule) {
					return FGrid::SidesMatch(Modules[CellModule].DoorMask, Modules[NeighborModule].DoorMask, Direction);
				});
			});
 
			//Nothing removed, nothing to spread
			if (!bChanged) {
				continue;
			}
 
			//Neighbour has no room left that fits: contradiction
			if (Wave.GetOptions(NeighborIndex).IsEmpty()) {
				return false;
			}
 
			//It changed, so its own neighbours must hear about it
			CellsToVisit.AddUnique(NeighborIndex);
		}
	}
	return true;
}

#pragma endregion METHODS