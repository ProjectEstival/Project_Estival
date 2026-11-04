#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetingEffectBase.h"
#include "SpawnProjectilesEffect.generated.h"

class AAbilityProjectile;

UCLASS()
class PROJECTESTIVAL_API USpawnProjectilesEffect : public UTargetingEffectBase
{
	GENERATED_BODY()
	
public:
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	FVector SpawnOffset = FVector::ForwardVector * 50.f;
	
private:
	void SpawnProjectile(const FAbilityContext& Context, const FVector& Location, const FVector& Direction) const;
};