
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CurrencySubsystem.h"
#include "CurrencySpawner.generated.h"

class ACurrencyPickup;

UCLASS()
class ACurrencySpawner : public AActor
{
	GENERATED_BODY()

public:
	ACurrencySpawner();

	UFUNCTION(BlueprintCallable, Category = "Currency")
	void SpawnCurrency(FVector Location, ECurrencyType Type, int32 Amount);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	TSubclassOf<ACurrencyPickup> PickupClass;
};
