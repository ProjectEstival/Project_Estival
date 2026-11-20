#pragma once

#include "CoreMinimal.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/RoomConstraint.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/RoomRow.h"
#include "UniqueRoomConstraint.generated.h"

UENUM()
enum class EPlacement : uint8 {
	Random,
	Fixed
};

//Places exactly one room of RoomType (START, BOSS) and forbids that type in every other cell
UCLASS(meta = (DisplayName = "Unique Room"))
class PROJECTESTIVAL_API UUniqueRoomConstraint : public URoomConstraint {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Unique Room")
	ERoomType RoomType = ERoomType::Start;

	UPROPERTY(EditAnywhere, Category = "Unique Room")
	EPlacement Placement = EPlacement::Random;

	//X along world X (Up), Y along world Y (Right)
	UPROPERTY(EditAnywhere, Category = "Unique Room", meta = (EditCondition = "Placement == EPlacement::Fixed", EditConditionHides))
	FIntPoint FixedCell = FIntPoint::ZeroValue;

	virtual bool Apply(FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, FRandomStream& Random) const override;

private:
	//INDEX_NONE when no cell can hold the room
	int32 ChooseCell(const FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, FRandomStream& Random) const;
	
	bool CanHoldRoomType(const FWave& Wave, int32 CellIndex, const TArray<FRoomModule>& Modules) const;
};