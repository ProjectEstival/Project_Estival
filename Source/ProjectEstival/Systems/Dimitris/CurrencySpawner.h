
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CurrencyTypes.h"
#include "CurrencySpawner.generated.h"

class ACurrencyPickup;

UCLASS()
class PROJECTESTIVAL_API ACurrencySpawner : public AActor
{
	GENERATED_BODY()

public:
	ACurrencySpawner();

	UFUNCTION(BlueprintCallable, Category = "Currency")
	void SpawnCurrency(FVector Location,ECurrencyStorage Storage,int32 Amount);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	TSubclassOf<ACurrencyPickup> PickupClass;
};
