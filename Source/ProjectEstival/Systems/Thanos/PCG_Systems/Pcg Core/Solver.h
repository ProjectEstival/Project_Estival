// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Direction.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/RoomModule.h"

/**
 *  Find the most certain cell and pick a one room for it
 *  tell the neighbouring cells to remove what cannot connect to it
 *  tell all neighbouring cells what choices are left for them to choose from (propagation)
 */
class FWave;
class FGrid;
class PROJECTESTIVAL_API Solver {
	
public:
	Solver(FWave& InWave, const FGrid& InGrid, const TArray<FRoomModule>& InModules, FRandomStream& InRandom);
	const bool Solve(FWave& Wave); /** TODO: Same here */
	
private:
	bool Propagate(int32 StartCell, TArray<int32>& CellsToVisit);
	const int32 FindLowestEntropyCell(); /** TODO: Change this to not change the solver directly the solver */
	void Collapce(int32 CellIndex);
	
	FWave& Wave;
	const FGrid& Grid;
	const TArray<FRoomModule>& Modules;
	FRandomStream& Random;
};