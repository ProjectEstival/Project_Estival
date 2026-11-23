// Fill out your copyright notice in the Description page of Project Settings.


#include "Systems/Thanos/PCG_Systems/Pcg Core/Solver.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/Grid.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Wave.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Random.h"

Solver::Solver(FWave& InWave, const FGrid& InGrid, const TArray<FRoomModule>& InModules, FRandomStream& InRandom): Wave(InWave), Grid(InGrid), Modules(InModules), Random(InRandom) {
}

const bool Solver::Solve(FWave& Wave) {
	
	TArray<int32> CellsToVisit;
	for (int32 CellIndex = 0; CellIndex < Wave.NumCells(); ++CellIndex) {
		if (Wave.GetOptions(CellIndex).Num()==0) {
			return false;
		}
		CellsToVisit.Add(CellIndex);
	}
	
/*	if(!Propagate(Wave, CellsToVisit)) {
		return false;
	}
	return true;*/ /** TODO: Placeholder for compilation, remember to change later*/
}
/**TODO: Add entropy*/
/**TODO: Add Weight*/
/**TODO: Add Propagation*/