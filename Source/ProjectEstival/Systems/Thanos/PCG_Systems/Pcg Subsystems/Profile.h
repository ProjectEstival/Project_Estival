#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Profile.generated.h"

class UDataTable;
class URoomConstraint;

UCLASS(BlueprintType)
class PROJECTESTIVAL_API UProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//DataTables made with RoomRow show here
	UPROPERTY(EditAnywhere, Category = "Rooms", meta = (RequiredAssetDataTags = "RowStructure=/Script/ProjectEstival.RoomRow"))
	TObjectPtr<UDataTable> RoomTable;

	//Cells on world X/Y
	UPROPERTY(EditAnywhere, Category = "Grid", meta = (ClampMin = 1))
	FIntPoint GridSize = FIntPoint(5, 5);

	//Size of room
	UPROPERTY(EditAnywhere, Category = "Grid", meta = (ClampMin = 1, Units = "cm"))
	FVector2D CellSize = FVector2D(400.0f, 400.0f);

	//Validation fails when fewer rooms than this are reachable from START
	UPROPERTY(EditAnywhere, Category = "Rules", meta = (ClampMin = 1))
	int32 MinRoomCount = 5;

	//Rules applied before solving
	UPROPERTY(EditAnywhere, Instanced, Category = "Rules")
	TArray<TObjectPtr<URoomConstraint>> Constraints;

	//Max Tries
	UPROPERTY(EditAnywhere, Category = "Rules", meta = (ClampMin = 1))
	int32 MaxAttempts = 500;
};