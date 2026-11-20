#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Wave.h"

void FWave::Reset(int32 CellCount, int32 ModuleCount)
{
	TArray<int32> AllModules;
	AllModules.Reserve(ModuleCount);
	for (int32 ModuleIndex = 0; ModuleIndex < ModuleCount; ++ModuleIndex)
	{
		AllModules.Add(ModuleIndex);
	}

	CellOptions.Init(AllModules, CellCount);
}

void FWave::CollapseTo(int32 CellIndex, int32 ModuleIndex)
{
	CellOptions[CellIndex] = { ModuleIndex };
}

bool FWave::Restrict(int32 CellIndex, TFunctionRef<bool(int32 ModuleIndex)> ShouldKeep)
{
	const int32 RemovedCount = CellOptions[CellIndex].RemoveAll([&ShouldKeep](int32 ModuleIndex)
	{
		return !ShouldKeep(ModuleIndex);
	});

	return RemovedCount > 0;
}

