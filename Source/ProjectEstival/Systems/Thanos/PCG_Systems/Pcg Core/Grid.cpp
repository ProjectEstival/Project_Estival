#include "Systems/Thanos/PCG_Systems/Pcg Core/Grid.h"

FGrid::FGrid(const FIntPoint& InSize, const FVector2D& InCellSize)
	: Size(FMath::Max(InSize.X, 1), FMath::Max(InSize.Y, 1))
	, CellSize(InCellSize)
{
}

bool FGrid::IsInBounds(const FIntPoint& Cell) const
{
	return Cell.X >= 0 && Cell.X < Size.X && Cell.Y >= 0 && Cell.Y < Size.Y;
}

bool FGrid::TryGetNeighbor(int32 CellIndex, EDirection Direction, int32& OutNeighborIndex) const
{
	const FIntPoint NeighborCell = ToCell(CellIndex) + Direction::ToGridOffset(Direction);
	if (!IsInBounds(NeighborCell))
	{
		return false;
	}

	OutNeighborIndex = ToIndex(NeighborCell);
	return true;
}

bool FGrid::DoorsStayInGrid(int32 CellIndex, uint8 DoorMask) const
{
	for (EDirection Direction : Direction::All)
	{
		int32 NeighborIndex = INDEX_NONE;
		if (Direction::HasDoor(DoorMask, Direction) && !TryGetNeighbor(CellIndex, Direction, NeighborIndex))
		{
			return false;
		}
	}
	return true;
}

bool FGrid::SidesMatch(uint8 DoorMask, uint8 NeighborDoorMask, EDirection DirectionToNeighbor)
{
	//My side facing the neighbour, and the neighbour's side facing me
	const bool bDoorHere = Direction::HasDoor(DoorMask, DirectionToNeighbor);
	const bool bDoorThere = Direction::HasDoor(NeighborDoorMask, Direction::Opposite(DirectionToNeighbor));
	return bDoorHere == bDoorThere;
}

FVector FGrid::GetCellCenter(int32 CellIndex) const
{
	const FIntPoint Cell = ToCell(CellIndex);
	return FVector((Cell.X + 0.5f) * CellSize.X, (Cell.Y + 0.5f) * CellSize.Y, 0.0f);
}