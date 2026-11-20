#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/ConstraintInitializer.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Wave.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/Grid.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/RoomConstraint.h"

bool FConstraintInitializer::BuildStartingWave(FWave& OutWave, const FGrid& Grid, const TArray<FRoomModule>& Modules,
	const TArray<TObjectPtr<URoomConstraint>>& Constraints, FRandomStream& Random) {

	OutWave.Reset(Grid.NumCells(), Modules.Num());
	
	RemoveDoorsLeavingGrid(OutWave, Grid, Modules);
	
	for (const URoomConstraint* Constraint : Constraints) {
		if (Constraint && !Constraint->Apply(OutWave, Grid, Modules, Random)) {
			return false;
		}
	}

	return true;
}

void FConstraintInitializer::RemoveDoorsLeavingGrid(FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules) {
	for (int32 CellIndex = 0; CellIndex < Wave.NumCells(); CellIndex++) {
		//Keep only modules whose doors all lead to a real neighbour
		Wave.Restrict(CellIndex, [&Grid, &Modules, CellIndex](int32 ModuleIndex) {
			return Grid.DoorsStayInGrid(CellIndex, Modules[ModuleIndex].DoorMask);
		});
	}
}