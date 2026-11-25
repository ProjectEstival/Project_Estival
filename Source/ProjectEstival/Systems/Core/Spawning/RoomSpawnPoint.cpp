#include "Systems/Core/Spawning/RoomSpawnPoint.h"
#include "Systems/Core/Spawning/EnemySpawner.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Components/ArrowComponent.h"

ARoomSpawnPoint::ARoomSpawnPoint() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(RootComponent);
	Arrow->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
}

void ARoomSpawnPoint::SpawnInRoom() {
	if (!EnemySpawnerComponent) return;

	for (FEnemySpawnPoint& Point : EnemySpawnerComponent->SpawnPoints) {
		if (!Point.SpawnMarker) {
			Point.SpawnMarker = this;
		}
	}
	EnemySpawnerComponent->SpawnEnemies();
}

AEstivalBaseCharacter* ARoomSpawnPoint::FindSpawnedBoss() const {
	if (!bIsBoss || !EnemySpawnerComponent) return nullptr;

	for (AActor* Spawned : EnemySpawnerComponent->GetSpawnedEnemies()) {
		if (AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(Spawned)) return Character;
	}
	return nullptr;
}

AEstivalBaseCharacter* ARoomSpawnPoint::SpawnAllIn(const TArray<TObjectPtr<AActor>>& Rooms) {
	AEstivalBaseCharacter* Boss = nullptr;

	for (AActor* Room : Rooms) {
		if (!IsValid(Room)) continue;

		TArray<AActor*> Children;
		Room->GetAllChildActors(Children);
		for (AActor* Child : Children) {
			ARoomSpawnPoint* Spawner = Cast<ARoomSpawnPoint>(Child);
			if (!Spawner) continue;

			Spawner->SpawnInRoom();
			if (!Boss) Boss = Spawner->FindSpawnedBoss();
		}
	}
	return Boss;
}

void ARoomSpawnPoint::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (EndPlayReason == EEndPlayReason::Destroyed) {
		ResetSpawnedEnemies();
	}
	Super::EndPlay(EndPlayReason);
}