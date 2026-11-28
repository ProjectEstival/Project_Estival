#include "Generator.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Profile.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Wave.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/Random.h"
#include "Systems/Thanos/PCG_Systems/Pcg Subsystems/ConstraintInitializer.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/Grid.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/RoomProcessor.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/Solver.h"
#include "Systems/Thanos/PCG_Systems/Pcg Core/Validator.h"
#include "Systems/Core/GameModes/DungeonGameMode.h"
#include "Systems/Core/Spawning/RoomSpawnPoint.h"
#include "Systems/Core/Characters/CharacterBase.h"

AGenerator::AGenerator() {
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AGenerator::Generate() {
	TryGenerate();
}

bool AGenerator::TryGenerate(){
	Clear();

	if (!Profile || !Profile->RoomTable) {
		UE_LOG(LogTemp, Error, TEXT("DungeonGenerator: no Profile or RoomTable set"));
		return false;
	}

	//Rows and modules
	const TArray<FRoomModule> Modules = FRoomProcessor::BuildModules(*Profile->RoomTable);
	if (Modules.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("DungeonGenerator: RoomTable gave no modules"));
		return false;
	}

	//Board
	const FGrid Grid(Profile->GridSize, Profile->CellSize);
	
	Random.Initialize(bUseFixedSeed ? FixedSeed : FMath::Rand());

	//Solve until playable
	FWave Wave;
	TArray<int32> Reachable;
	if (!TrySolve(Wave, Grid, Modules, Reachable)) {
		UE_LOG(LogTemp, Error, TEXT("DungeonGenerator: no playable dungeon after %d attempts"), Profile->MaxAttempts);
		return false;
	}

	//Spawn :)
	SpawnRooms(Wave, Grid, Modules, Reachable);
	return true;
}

bool AGenerator::TrySolve(FWave& OutWave, const FGrid& Grid, const TArray<FRoomModule>& Modules, TArray<int32>& OutReachable) {
	for (int32 Attempt = 0; Attempt < Profile->MaxAttempts; ++Attempt) {
		if (!FConstraintInitializer::BuildStartingWave(OutWave, Grid, Modules, Profile->Constraints, Random)) {
			continue;
		}

		Solver WaveSolver(OutWave, Grid, Modules, Random);
		if (!WaveSolver.Solve()) {
			continue;
		}

		if (FValidator::IsPlayable(OutWave, Grid, Modules, Profile->MinRoomCount, OutReachable)) {
			return true;
		}
	}
	return false;
}

void AGenerator::SpawnRooms(const FWave& Wave, const FGrid& Grid, const TArray<FRoomModule>& Modules, const TArray<int32>& Reachable) {
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.OverrideLevel = GetLevel();
	SpawnParams.ObjectFlags |= RF_Transient;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform& GeneratorTransform = GetActorTransform();

	for (int32 CellIndex = 0; CellIndex < Wave.NumCells(); ++CellIndex) {
		const FRoomModule& Module = Modules[Wave.GetCollapsedModule(CellIndex)];
		
		if (!Module.IsEmpty() && !Reachable.Contains(CellIndex)) {
			continue;
		}

		//EMPTY rows nothing to spawn
		const TSubclassOf<AActor> RoomClass = PickVariant(Module);
		if (!RoomClass) {
			continue;
		}

		const FVector Location = GeneratorTransform.TransformPosition(Grid.GetCellCenter(CellIndex));
		FRotator Rotation = GetActorRotation();
		if (Module.bRotated180) {
			Rotation.Yaw += 180.0f;
		}

		AActor* Room = World->SpawnActor<AActor>(RoomClass, Location, Rotation, SpawnParams);
		if (!Room) {
			continue;
		}

		SpawnedRooms.Add(Room);
		if (Module.RoomType == ERoomType::Start) {
			StartRoom = Room;
		}
	}
}

TSubclassOf<AActor> AGenerator::PickVariant(const FRoomModule& Module) {
	if (Module.ActorVariants.IsEmpty()) {
		return nullptr;
	}

	TArray<float> Weights;
	Weights.Reserve(Module.ActorVariants.Num());
	for (const FActorVariant& Variant : Module.ActorVariants) {
		Weights.Add(Variant.Weight);
	}

	return Module.ActorVariants[Random::PickWeightedIndex(Weights, Random)].ActorClass;
}

void AGenerator::BuildFloor_Implementation(int32 Floor) {
	const bool bGenerated = TryGenerate();
	if (!bGenerated) {
		UE_LOG(LogTemp, Error, TEXT("DungeonGenerator: floor %d failed to generate"), Floor);
	}

	AEstivalBaseCharacter* Boss = bGenerated ? ARoomSpawnPoint::SpawnAllIn(SpawnedRooms) : nullptr;
	const FTransform PartyStart = StartRoom ? StartRoom->GetActorTransform() : GetActorTransform();

	if (ADungeonGameMode* GameMode = GetWorld()->GetAuthGameMode<ADungeonGameMode>()) {
		GameMode->FloorReady(PartyStart, Boss);
	}
}

void AGenerator::Clear() {
	for (AActor* Room : SpawnedRooms) {
		if (IsValid(Room)) {
			Room->Destroy();
		}
	}
	SpawnedRooms.Reset();
	StartRoom = nullptr;
}

void AGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	if (EndPlayReason == EEndPlayReason::Destroyed) {
		Clear();
	}
	Super::EndPlay(EndPlayReason);
}