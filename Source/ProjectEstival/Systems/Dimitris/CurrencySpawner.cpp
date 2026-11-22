
#include "CurrencySpawner.h"
#include "CurrencyPickup.h"
#include "Engine/World.h"

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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACurrencyPickup* Pickup = GetWorld()->SpawnActor<ACurrencyPickup>(PickupClass,Location,FRotator::ZeroRotator,SpawnParams);

	if (Pickup)
	{
		Pickup->SetCurrency(Storage, Amount);

		UE_LOG(LogTemp, Log,
			TEXT("Spawned %d currency (Storage: %d)"),
			Amount,
			static_cast<int32>(Storage));
	}
}
