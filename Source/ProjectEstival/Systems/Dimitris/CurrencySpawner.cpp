
#include "CurrencySpawner.h"
#include "CurrencyPickup.h"
#include "Engine/World.h"
#include "Systems/Maria/Pooling System/ActorPoolSubsystem.h"

ACurrencySpawner::ACurrencySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACurrencySpawner::SpawnCurrency(FVector Location,ECurrencyStorage Storage,int32 Amount)
{
	if (!PickupClass || !GetWorld() || Amount <= 0)
	{
		return;
	}

	const auto Pooling = UWorld::GetSubsystem<UActorPoolSubsystem>(GetWorld());
	ACurrencyPickup* Pickup = nullptr;
	if (Pooling) Pickup = Cast<ACurrencyPickup>(Pooling->AcquireActorAtLocation(PickupClass, Location, FRotator::ZeroRotator));

	if (Pickup)
	{
		Pickup->SetCurrency(Storage, Amount);

		UE_LOG(LogTemp, Log,
			TEXT("Spawned %d currency (Storage: %d)"),
			Amount,
			static_cast<int32>(Storage));
	}
}
