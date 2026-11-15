#pragma once

#include "CoreMinimal.h"
#include "SpawnAbilityActorEffect.h"
#include "SpawnProjectilesEffect.generated.h"

class AAbilityProjectile;

/** Fires projectiles along the direction. They hit with this effect's payloads and run its expire effects where they end. */
UCLASS(meta = (DisplayName = "Spawn Projectiles"))
class PROJECTESTIVAL_API USpawnProjectilesEffect : public USpawnAbilityActorEffect
{
	GENERATED_BODY()
	
public:
	USpawnProjectilesEffect();
	virtual void Execute_Implementation(const FAbilityContext& Context) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AAbilityProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = 1, Units = "CentimetersPerSecond"))
	float Speed = 2000;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = 1, Units = "cm"))
	float Range = 1500;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = 1))
	int32 ProjectileCount = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = 1, ClampMax = 360, Units = "deg", EditCondition = "ProjectileCount > 1", EditConditionHides))
	float SpreadAngle = 90;
	
private:
	void SpawnProjectile(const FAbilityContext& Context, const FVector& Location, const FVector& Direction) const;
};