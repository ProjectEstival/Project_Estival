#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BLK_PCG.generated.h"

class UTextRenderComponent;

//Prefabs
enum class EWFCDirection : uint8
{
	/**TO BE RENAMED TO U, D, L, R (U is East) */ 
	North,
	East,
	South,
	West
};

//Select one of the variants based on the weight of each prefab
USTRUCT(BlueprintType)
struct FWFCActorVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Variant")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, Category = "Variant", meta = (ClampMin = 0.01))
	float Weight = 1.0f;
};

//DataTable based architecture
USTRUCT(BlueprintType)
struct FWFCTile : public FTableRowBase
{
	GENERATED_BODY()

	//Variant for this tile - random based on weight.
	UPROPERTY(EditAnywhere, Category = "Tile")
	TArray<FWFCActorVariant> ActorVariants;

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

//Force tiles to place before the generation
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

	/** Must match a row name in TileSet */
	UPROPERTY(EditAnywhere, Category = "Forced Tile")
	FName TileName = NAME_None;
};

//Fills a rectangular grid with prefabs picked by WFC, matching tile sockets so near prefabs can connect.
UCLASS()
class PROJECTESTIVAL_API ABLK_PCG : public AActor
{
	GENERATED_BODY()

public:
	//Sets default values
	ABLK_PCG();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	//Data Table of FWFCTile rows - the row name is each tile's identifier, used by ForcedTiles
	UPROPERTY(EditAnywhere, Category = "WFC")
	TObjectPtr<UDataTable> TileSet;

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

	/** LEGACY SET DIMENSIONS */
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

	//Debug magic (draw the name on the grid / or draw an error so we can see what went right or wrong during gen and where)
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

	//Cached copy of TileSet's rows, rebuilt every InitializeCells() call
	TArray<FWFCTile> Tiles;

	//Row name for each entry in Tiles
	TArray<FName> TileRowNames;

	/** Change to DT option when we change the dimensions to be easier to import to other iterations */
	//float CellSizeX(const TArray<float>& Options) const; <- Create Option for those.
	//float CellSizeY(const TArray<float>& Options) const;

	//Rebuilds Tiles/TileRowNames from TileSet. Returns false if TileSet is unset or has no valid rows.
	bool BuildTileCache();

	bool InitializeCells();

	bool OnPath();

	//Removes any tile from border cells that their outward socket does not match BoundaryClosedSocket
	void RestrictBoundaryCells();
	
	//Determine which cells are concidered "OnPath"
	void DeterminePath();

	//Remove incompatible cells
	void PropagateFrom(int32 CellIndex);
	
	bool AreCompatible(const FWFCTile& CellTile, const FWFCTile& NeighborTile, EWFCDirection DirectionToNeighbor) const;
	
	int32 PickWeightedTile(const TArray<int32>& Options) const;

	//Picks a variant from ActorVariants based on weight
	TSubclassOf<AActor> PickActorVariant(const FWFCTile& Tile) const;
	
	bool GetNeighborIndex(int32 CellIndex, EWFCDirection Direction, int32& OutNeighborIndex) const;
	
	void SpawnTiles();

};