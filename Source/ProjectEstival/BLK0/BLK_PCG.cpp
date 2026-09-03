#include "BLK0/BLK_PCG.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/TextRenderComponent.h"

// Sets default values
ABLK_PCG::ABLK_PCG()
{
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABLK_PCG::BeginPlay()
{
	Super::BeginPlay();

	if (bGenerateOnBeginPlay)
	{
		GenerateGrid();
	}
}

void ABLK_PCG::GenerateGrid()
{
	ClearGrid();

	if (!InitializeCells())
	{
		return;
	}

	RandomStream.Initialize(bUseRandomSeed ? RandomSeed : FMath::Rand());

	const int32 CellCount = GridWidth * GridHeight;

	//collapse one cell per iteration, until the whole grid is resolved
	for (int32 Iteration = 0; Iteration < CellCount; ++Iteration)
	{
		int32 LowestEntropyIndex = INDEX_NONE;
		int32 LowestEntropyCount = MAX_int32;

		for (int32 CellIndex = 0; CellIndex < CellCount; ++CellIndex)
		{
			const int32 OptionCount = CellPossibilities[CellIndex].Num();

			if (OptionCount == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("BLK_PCG: contradiction reached generating the grid, aborting"));
				return;
			}

			if (OptionCount > 1 && OptionCount < LowestEntropyCount)
			{
				LowestEntropyCount = OptionCount;
				LowestEntropyIndex = CellIndex;
			}
		}

		//every cell is already collapsed to a single tile
		if (LowestEntropyIndex == INDEX_NONE)
		{
			break;
		}

		const int32 ChosenTile = PickWeightedTile(CellPossibilities[LowestEntropyIndex]);
		CellPossibilities[LowestEntropyIndex] = { ChosenTile };

		PropagateFrom(LowestEntropyIndex);
	}

	SpawnTiles();

	if (bShowDebugLabels)
	{
		ShowDebugGrid();
	}
}

void ABLK_PCG::ClearGrid()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}

	SpawnedActors.Reset();
	CellPossibilities.Reset();
	ClearDebugGrid();
}

void ABLK_PCG::ShowDebugGrid()
{
	if (CellPossibilities.Num() != GridWidth * GridHeight)
	{
		UE_LOG(LogTemp, Warning, TEXT("BLK_PCG: no grid data to show, generate the grid first"));
		return;
	}

	ClearDebugGrid();

	const int32 CellCount = GridWidth * GridHeight;

	for (int32 CellIndex = 0; CellIndex < CellCount; ++CellIndex)
	{
		const int32 X = CellIndex % GridWidth;
		const int32 Y = CellIndex / GridWidth;
		const FVector CellOrigin = GetActorLocation() + FVector(X * CellSizeX, Y * CellSizeY, 0.0f);
		const FVector CellCenter = CellOrigin + FVector(CellSizeX * 0.5f, CellSizeY * 0.5f, 0.0f);

		const TArray<int32>& Options = CellPossibilities[CellIndex];

		FString Label;
		FColor DebugColor;

		if (Options.Num() == 0)
		{
			Label = TEXT("CONTRADICTION");
			DebugColor = FColor::Red;
		}
		else if (Options.Num() > 1)
		{
			Label = FString::Printf(TEXT("%d options"), Options.Num());
			DebugColor = FColor::Yellow;
		}
		else
		{
			const FWFCTile& Tile = Tiles[Options[0]];
			if (Tile.ActorClass)
			{
				Label = Tile.TileName != NAME_None ? Tile.TileName.ToString() : Tile.ActorClass->GetName();
				DebugColor = FColor::Green;
			}
			else
			{
				Label = TEXT("NO ACTOR CLASS");
				DebugColor = FColor::Orange;
			}
		}

		DrawDebugBox(GetWorld(), CellCenter, FVector(CellSizeX * 0.5f, CellSizeY * 0.5f, 5.0f), DebugColor, true, -1.0f, 0, 5.0f);

		//Debug magic, i have no clue but i need it
		UTextRenderComponent* TextComp = NewObject<UTextRenderComponent>(this, NAME_None, RF_Transient);
		TextComp->RegisterComponentWithWorld(GetWorld());
		TextComp->SetWorldLocation(CellCenter + FVector(0.0f, 0.0f, 50.0f));
		TextComp->SetWorldRotation(FRotator(90.0f, 0.0f, 0.0f));
		TextComp->SetText(FText::FromString(Label));
		TextComp->SetTextRenderColor(DebugColor);
		TextComp->SetWorldSize(40.0f);
		TextComp->SetHorizontalAlignment(EHTA_Center);
		TextComp->SetVerticalAlignment(EVRTA_TextCenter);

		DebugTextComponents.Add(TextComp);
	}
}

void ABLK_PCG::ClearDebugGrid()
{
	FlushPersistentDebugLines(GetWorld());
	FlushDebugStrings(GetWorld());

	for (UTextRenderComponent* TextComp : DebugTextComponents)
	{
		if (IsValid(TextComp))
		{
			TextComp->DestroyComponent();
		}
	}
	DebugTextComponents.Reset();
}

bool ABLK_PCG::InitializeCells()
{
	if (Tiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BLK_PCG: no tiles assigned, nothing to generate"));
		return false;
	}

	TArray<int32> AllOptions;
	AllOptions.Reserve(Tiles.Num());
	for (int32 TileIndex = 0; TileIndex < Tiles.Num(); ++TileIndex)
	{
		AllOptions.Add(TileIndex);
	}

	CellPossibilities.Init(AllOptions, GridWidth * GridHeight);

	RestrictBoundaryCells();

	//pin forced cells to their tile first
	for (const FWFCForcedTile& Forced : ForcedTiles)
	{
		if (Forced.X < 0 || Forced.X >= GridWidth || Forced.Y < 0 || Forced.Y >= GridHeight)
		{
			UE_LOG(LogTemp, Warning, TEXT("BLK_PCG: forced tile (%d, %d) is outside the grid, skipping"), Forced.X, Forced.Y);
			continue;
		}

		const int32 TileIndex = Tiles.IndexOfByPredicate([&Forced](const FWFCTile& Tile) { return Tile.TileName == Forced.TileName; });
		if (TileIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("BLK_PCG: forced tile name '%s' not found in Tiles, skipping"), *Forced.TileName.ToString());
			continue;
		}

		CellPossibilities[Forced.Y * GridWidth + Forced.X] = { TileIndex };
	}
	
	for (const FWFCForcedTile& Forced : ForcedTiles)
	{
		if (Forced.X >= 0 && Forced.X < GridWidth && Forced.Y >= 0 && Forced.Y < GridHeight)
		{
			PropagateFrom(Forced.Y * GridWidth + Forced.X);
		}
	}

	return true;
}

void ABLK_PCG::RestrictBoundaryCells()
{
	const int32 CellCount = GridWidth * GridHeight;

	for (int32 CellIndex = 0; CellIndex < CellCount; ++CellIndex)
	{
		const int32 X = CellIndex % GridWidth;
		const int32 Y = CellIndex / GridWidth;

		TArray<EWFCDirection> OutwardDirections;
		if (Y == 0) OutwardDirections.Add(EWFCDirection::North);
		if (Y == GridHeight - 1) OutwardDirections.Add(EWFCDirection::South);
		if (X == 0) OutwardDirections.Add(EWFCDirection::West);
		if (X == GridWidth - 1) OutwardDirections.Add(EWFCDirection::East);

		if (OutwardDirections.Num() == 0)
		{
			continue;
		}

		TArray<int32>& Options = CellPossibilities[CellIndex];

		for (int32 i = Options.Num() - 1; i >= 0; --i)
		{
			const FWFCTile& Tile = Tiles[Options[i]];

			bool bAllOutwardSidesClosed = true;
			for (EWFCDirection Direction : OutwardDirections)
			{
				FName Socket;
				switch (Direction)
				{
				case EWFCDirection::North: Socket = Tile.NorthSocket; break;
				case EWFCDirection::East:  Socket = Tile.EastSocket;  break;
				case EWFCDirection::South: Socket = Tile.SouthSocket; break;
				case EWFCDirection::West:  Socket = Tile.WestSocket;  break;
				}

				if (Socket != BoundaryClosedSocket)
				{
					bAllOutwardSidesClosed = false;
					break;
				}
			}

			if (!bAllOutwardSidesClosed)
			{
				Options.RemoveAt(i);
			}
		}
	}
}

void ABLK_PCG::PropagateFrom(int32 StartCellIndex)
{
	TArray<int32> Stack;
	Stack.Push(StartCellIndex);

	while (Stack.Num() > 0)
	{
		const int32 CurrentIndex = Stack.Pop();

		for (EWFCDirection Direction : { EWFCDirection::North, EWFCDirection::East, EWFCDirection::South, EWFCDirection::West })
		{
			int32 NeighborIndex = INDEX_NONE;
			if (!GetNeighborIndex(CurrentIndex, Direction, NeighborIndex))
			{
				continue;
			}

			TArray<int32>& NeighborOptions = CellPossibilities[NeighborIndex];
			if (NeighborOptions.Num() <= 1)
			{
				continue;
			}

			bool bChanged = false;
			for (int32 i = NeighborOptions.Num() - 1; i >= 0; --i)
			{
				const FWFCTile& NeighborTile = Tiles[NeighborOptions[i]];

				bool bCompatibleWithAny = false;
				for (int32 CurrentOption : CellPossibilities[CurrentIndex])
				{
					if (AreCompatible(Tiles[CurrentOption], NeighborTile, Direction))
					{
						bCompatibleWithAny = true;
						break;
					}
				}

				if (!bCompatibleWithAny)
				{
					NeighborOptions.RemoveAt(i);
					bChanged = true;
				}
			}

			if (bChanged)
			{
				Stack.Push(NeighborIndex);
			}
		}
	}
}

bool ABLK_PCG::AreCompatible(const FWFCTile& CellTile, const FWFCTile& NeighborTile, EWFCDirection DirectionToNeighbor) const
{
	switch (DirectionToNeighbor)
	{
	case EWFCDirection::North:
		return CellTile.NorthSocket == NeighborTile.SouthSocket;
	case EWFCDirection::East:
		return CellTile.EastSocket == NeighborTile.WestSocket;
	case EWFCDirection::South:
		return CellTile.SouthSocket == NeighborTile.NorthSocket;
	case EWFCDirection::West:
		return CellTile.WestSocket == NeighborTile.EastSocket;
	}

	return false;
}

int32 ABLK_PCG::PickWeightedTile(const TArray<int32>& Options) const
{
	float TotalWeight = 0.0f;
	for (int32 Option : Options)
	{
		TotalWeight += FMath::Max(Tiles[Option].Weight, KINDA_SMALL_NUMBER);
	}

	float Roll = RandomStream.FRandRange(0.0f, TotalWeight);
	for (int32 Option : Options)
	{
		const float Weight = FMath::Max(Tiles[Option].Weight, KINDA_SMALL_NUMBER);
		if (Roll <= Weight)
		{
			return Option;
		}
		Roll -= Weight;
	}

	return Options.Last();
}

bool ABLK_PCG::GetNeighborIndex(int32 CellIndex, EWFCDirection Direction, int32& OutNeighborIndex) const
{
	const int32 X = CellIndex % GridWidth;
	const int32 Y = CellIndex / GridWidth;

	int32 NeighborX = X;
	int32 NeighborY = Y;

	switch (Direction)
	{
	case EWFCDirection::North:
		NeighborY -= 1;
		break;
	case EWFCDirection::East:
		NeighborX += 1;
		break;
	case EWFCDirection::South:
		NeighborY += 1;
		break;
	case EWFCDirection::West:
		NeighborX -= 1;
		break;
	}

	if (NeighborX < 0 || NeighborX >= GridWidth || NeighborY < 0 || NeighborY >= GridHeight)
	{
		return false;
	}

	OutNeighborIndex = NeighborY * GridWidth + NeighborX;
	return true;
}

void ABLK_PCG::SpawnTiles()
{
	const int32 CellCount = GridWidth * GridHeight;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 CellIndex = 0; CellIndex < CellCount; ++CellIndex)
	{
		if (CellPossibilities[CellIndex].Num() != 1)
		{
			continue;
		}

		const FWFCTile& Tile = Tiles[CellPossibilities[CellIndex][0]];
		if (!Tile.ActorClass)
		{
			continue;
		}

		const int32 X = CellIndex % GridWidth;
		const int32 Y = CellIndex / GridWidth;

		const FVector Location = GetActorLocation() + FVector(X * CellSizeX, Y * CellSizeY, 0.0f);
		const FTransform SpawnTransform(GetActorRotation(), Location);

		if (AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Tile.ActorClass, SpawnTransform, SpawnParams))
		{
			SpawnedActors.Add(SpawnedActor);
		}
	}
}
