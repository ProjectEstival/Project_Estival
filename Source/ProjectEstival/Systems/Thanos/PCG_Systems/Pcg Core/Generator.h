#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/Core/GameModes/LevelManager.h"
#include "Generator.generated.h"

class UProfile;
class FWave;
class FGrid;
struct FRoomModule;

/** Builds a floor with WFC: solve, validate, spawn the rooms, then tell the GameMode the floor is ready. */
UCLASS()
class PROJECTESTIVAL_API AGenerator : public AActor, public ILevelManager
{
	GENERATED_BODY()

public:
	AGenerator();

	//Exposing to GM
	virtual void BuildFloor_Implementation(int32 Floor) override;

	//The dataAsset that holds all the pcg customizable data
	UPROPERTY(EditAnywhere, Category = "PCG")
	TObjectPtr<UProfile> Profile;
	
	//There was a bug with player spawning so i spawn them with some height 
	UPROPERTY(EditAnywhere, Category = "PCG", meta = (Units = "cm"))
	float PartySpawnHeight = 150.0f;

	//For testing, it will spawn the same thing every time
	UPROPERTY(EditAnywhere, Category = "PCG|Debug")
	bool bUseFixedSeed = false;

	UPROPERTY(EditAnywhere, Category = "PCG|Debug", meta = (EditCondition = "bUseFixedSeed"))
	int32 FixedSeed = 0;

	//For testing, Solve and spawn only
	UFUNCTION(CallInEditor, Category = "PCG")
	void Generate();

	UFUNCTION(CallInEditor, Category = "PCG")
	void Clear();
	
	bool TryGenerate();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	//Attempt loop
	bool TrySolve(FWave& OutWave, const FGrid& Grid, const TArray<FRoomModule>& Modules, TArray<int32>& OutCells);
	
	void SpawnRooms(const FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, const TArray<int32>& Reachable);

	//Weighted pick
	TSubclassOf<AActor> PickVariant(const FRoomModule& Module);

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedRooms;

	//Start
	UPROPERTY(Transient)
	TObjectPtr<AActor> StartRoom;

	FRandomStream Random;
};
