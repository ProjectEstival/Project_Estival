#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BLK_PCG.generated.h"

class UTextRenderComponent;

//Prefabs
enum class EWFCDirection : uint8
{
	North,
	East,
	South,
	West
};

//Edge sockets must match between neighboring tiles for them to be considered compatible. */
USTRUCT(BlueprintType)
struct FWFCTile
{
	GENERATED_BODY()

	//Identifier used by ForcedTiles to pin this tile to a specific cell */
	UPROPERTY(EditAnywhere, Category = "Tile")
	FName TileName = NAME_None;

	//Prefab spawned into the level for this tile
	UPROPERTY(EditAnywhere, Category = "Tile")
	TSubclassOf<AActor> ActorClass;
	
	UPROPERTY(EditAnywhere, Category = "Tile")
	FName NorthSocket = NAME_None;
	
	UPROPERTY(EditAnywhere, Category = "Tile")
	FName EastSocket = NAME_None;
	
	UPROPERTY(EditAnywhere, Category = "Tile")
	FName SouthSocket = NAME_None;
	
	UPROPERTY(EditAnywhere, Category = "Tile")
	FName WestSocket = NAME_None;
	
	UPROPERTY(EditAnywhere, Category = "Tile", meta = (ClampMin = 0.01))
	float Weight = 1.0f;
};

//Locks a specific grid cell to a specific tile before generation runs, e.g. to force corner pieces into the corners of the grid
USTRUCT(BlueprintType)
struct FWFCForcedTile
{
	GENERATED_BODY()

	/** Grid column, 0-based from the west edge */
	UPROPERTY(EditAnywhere, Category = "Forced Tile")
	int32 X = 0;

	/** Grid row, 0-based from the north edge */
	UPROPERTY(EditAnywhere, Category = "Forced Tile")
	int32 Y = 0;

	/** Must match a Tiles entry's TileName */
	UPROPERTY(EditAnywhere, Category = "Forced Tile")
	FName TileName = NAME_None;
};

//Fills a rectangular grid with prefabs picked by WFC, matching tile sockets so near prefabs can connect.
UCLASS()
class PROJECTESTIVAL_API ABLK_PCG : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABLK_PCG();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(EditAnywhere, Category = "WFC")
	TArray<FWFCTile> Tiles;

	//Cells locked to a specific tile before generation runs
	UPROPERTY(EditAnywhere, Category = "WFC")
	TArray<FWFCForcedTile> ForcedTiles;

	//Any socket facing outside the grid must equal this value, so no door/opening ever faces the void
	UPROPERTY(EditAnywhere, Category = "WFC")
	FName BoundaryClosedSocket = "Wall";

	//grid cells along X
	UPROPERTY(EditAnywhere, Category = "WFC", meta = (ClampMin = 1))
	int32 GridWidth = 10;

	//grid cells along Y
	UPROPERTY(EditAnywhere, Category = "WFC", meta = (ClampMin = 1))
	int32 GridHeight = 10;
	
	//UPROPERTY(EditAnywhere, Category = "WFC", meta = (ClampMin = 1, Units = "cm"))
	//float CellSize = 400.0f;
	
	UPROPERTY(EditAnywhere, Category = "WFC", meta = (ClampMin = 1, Units = "cm"))
	float CellSizeX = 400.0f;
	
	UPROPERTY(EditAnywhere, Category = "WFC", meta = (ClampMin = 1, Units = "cm"))
	float CellSizeY = 400.0f;

	//TESTING using RandomSeed instead of a new randomseed every time we generate
	UPROPERTY(EditAnywhere, Category = "WFC")
	bool bUseRandomSeed = false;
	
	UPROPERTY(EditAnywhere, Category = "WFC", meta = (EditCondition = "bUseRandomSeed"))
	int32 RandomSeed = 0;

	//Autorun on start
	UPROPERTY(EditAnywhere, Category = "WFC")
	bool bGenerateOnBeginPlay = true;

	UFUNCTION(CallInEditor, Category = "WFC")
	void GenerateGrid();

	UFUNCTION(CallInEditor, Category = "WFC")
	void ClearGrid();

	//Draw the resolved tile name (or an error) above every cell so the grid can be inspected visually
	UPROPERTY(EditAnywhere, Category = "WFC|Debug")
	bool bShowDebugLabels = true;

	UFUNCTION(CallInEditor, Category = "WFC|Debug")
	void ShowDebugGrid();

	UFUNCTION(CallInEditor, Category = "WFC|Debug")
	void ClearDebugGrid();

private:

	TArray<TArray<int32>> CellPossibilities;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedActors;

	//Text labels drawn per cell by ShowDebugGrid; TextRenderComponent is used instead of DrawDebugString because
	//DrawDebugString only renders through a HUD/Canvas, which doesn't exist outside of Play-In-Editor
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextRenderComponent>> DebugTextComponents;

	//Random stream for tile selection
	FRandomStream RandomStream;
	
	bool InitializeCells();

	//Removes any tile from border cells whose outward-facing socket(s) don't match BoundaryClosedSocket
	void RestrictBoundaryCells();

	//Remove incompatible cells
	void PropagateFrom(int32 CellIndex);
	
	bool AreCompatible(const FWFCTile& CellTile, const FWFCTile& NeighborTile, EWFCDirection DirectionToNeighbor) const;
	
	int32 PickWeightedTile(const TArray<int32>& Options) const;
	
	bool GetNeighborIndex(int32 CellIndex, EWFCDirection Direction, int32& OutNeighborIndex) const;
	
	void SpawnTiles();

};