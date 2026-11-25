#pragma once

#include "CoreMinimal.h"
#include "Systems/Stratos/SpawnerBase.h"
#include "RoomSpawnPoint.generated.h"

class AEstivalBaseCharacter;
class UArrowComponent;

UCLASS()
class PROJECTESTIVAL_API ARoomSpawnPoint : public ASpawnerBase {
	GENERATED_BODY()

public:
	ARoomSpawnPoint();

	UPROPERTY(EditAnywhere, Category = "Spawning")
	bool bIsBoss = false;

	static AEstivalBaseCharacter* SpawnAllIn(const TArray<TObjectPtr<AActor>>& Rooms);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	TObjectPtr<UArrowComponent> Arrow;

	void SpawnInRoom();
	AEstivalBaseCharacter* FindSpawnedBoss() const;
};