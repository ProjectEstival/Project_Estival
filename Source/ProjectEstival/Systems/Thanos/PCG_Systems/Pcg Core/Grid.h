#pragma once

#include "CoreMinimal.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Direction.h"

class FGrid {
	
public:
	
	FGrid(const FIntPoint& InSize, const FVector2D& InCellSize);
	
	int32 NumCells() const { return Size.X * Size.Y; }
	
	const FVector2D& GetCellSize() const { return CellSize; }
	
	bool IsInBounds(const FIntPoint&  Cell) const;
	
	int32 ToIndex(const FIntPoint& Cell) const { return Cell.Y * Size.X + Cell.X; }
	
	FIntPoint ToCell(int32 CellIndex) const { return FIntPoint(CellIndex % Size.X, CellIndex / Size.X); }
	
	bool TryGetNeighbor(int32 CellIndex, EDirection Direction, int32& OutNeighborIndex) const;
	
	bool DoorsStayInGrid(int32 CellIndex, uint8 DoorMask) const;
	
	static bool SidesMatch(uint8 DoorMask, uint8 NeighborDoorMask, EDirection DirectionToNeighbor);
	
	FVector GetCellCenter(int32 CellIndex) const;
	
private:
	FIntPoint Size;
	FVector2D CellSize;
	
};
