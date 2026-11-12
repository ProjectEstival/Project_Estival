#pragma once

#include "CoreMinimal.h"
#include "AbilityActor.h"
#include "AbilityZone.generated.h"

class UArrowComponent;
class USpawnZoneEffect;

/** Ability actor that stays (or follows someone) and hits what is inside the spawning effect's shape. */
UCLASS(Abstract, Blueprintable)
class PROJECTESTIVAL_API AAbilityZone : public AAbilityActor
{
	GENERATED_BODY()

public:
	AAbilityZone();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UArrowComponent> Arrow;

	/** How often the zone checks what is inside it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zone", meta = (ClampMin = 0.02, Units = "s"))
	float OccupantUpdateInterval = 0.1f;

	virtual void BeginPlay() override;

private:
	const USpawnZoneEffect* GetZoneEffect() const;
	void UpdateOccupants();
	void RunTick() const;

	TMap<TWeakObjectPtr<AActor>, float> LastHitTimes;
	FTimerHandle OccupantTimer;
	FTimerHandle TickTimer;
};
