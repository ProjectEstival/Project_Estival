#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/RoomModule.h"
#include "RoomConstraint.generated.h"

class FGrid;
class FWave;

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API URoomConstraint : public UObject {
	GENERATED_BODY()

public:
	virtual bool Apply(FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, FRandomStream& Random) const
		PURE_VIRTUAL(URoomConstraint::Apply, return false;);
};