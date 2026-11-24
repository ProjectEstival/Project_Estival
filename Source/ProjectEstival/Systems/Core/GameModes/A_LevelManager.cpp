#include "Systems/Core/GameModes/A_LevelManager.h"

#include "Systems/Core/GameModes/DungeonGameMode.h"

void AA_LevelManager::BuildFloor_Implementation(int32 Floor)
{
	//ClearCurrentFloor();
}

void AA_LevelManager::FinishFloor(FTransform StartTransform, AEstivalBaseCharacter *Boss) {
	if (ADungeonGameMode *GM = GetWorld()->GetAuthGameMode<ADungeonGameMode>()) {
		GM->FloorReady(StartTransform, Boss);
	}
}