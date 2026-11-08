#pragma once

#include "CoreMinimal.h"

/** The wave for every grid cell, the modules it can still become
*1 option left = collapsed, 0 options left = contradiction.
*/
class FWave
{
public:
	//Superposition
	void Reset(int32 CellCount, int32 ModuleCount);

	int32 NumCells() const { return CellOptions.Num(); }

	const TArray<int32>& GetOptions(int32 CellIndex) const { return CellOptions[CellIndex]; }

	bool IsCollapsed(int32 CellIndex) const { return CellOptions[CellIndex].Num() == 1; }

	//Only call on collapsed cells
	int32 GetCollapsedModule(int32 CellIndex) const { return CellOptions[CellIndex][0]; }

	void CollapseTo(int32 CellIndex, int32 ModuleIndex);

	//Removes every option ShouldKeep denys. Returns true if anything was removed
	bool Restrict(int32 CellIndex, TFunctionRef<bool(int32 ModuleIndex)> ShouldKeep);

private:
	TArray<TArray<int32>> CellOptions;
};
