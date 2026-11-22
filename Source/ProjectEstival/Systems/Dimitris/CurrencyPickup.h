
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CurrencySubsystem.h"
#include "CurrencyPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPrimitiveComponent;

UCLASS()
class PROJECTESTIVAL_API ACurrencyPickup : public AActor
{
	GENERATED_BODY()

public:
	ACurrencyPickup();

	UFUNCTION(BlueprintCallable, Category = "Currency")
	void SetCurrency(ECurrencyStorage Storage, int32 NewAmount);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	ECurrencyStorage CurrencyStorage = ECurrencyStorage::Run;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency",meta = (ClampMin = "1"))
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	bool bRandomizeAmount = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency",meta = (EditCondition = "bRandomizeAmount", ClampMin = "1"))
	int32 MinAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency",meta = (EditCondition = "bRandomizeAmount", ClampMin = "1"))
	int32 MaxAmount = 10;

	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,
		UPrimitiveComponent* OtherComp,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

private:
	bool bCollected = false;
};
