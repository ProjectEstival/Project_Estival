#pragma once

#include "CoreMinimal.h"
#include "AbilityActor.h"
#include "AbilityProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UArrowComponent;

/** Ability actor that flies forward and hits what it overlaps. Expires on world hit or at max range. */
UCLASS(Abstract, Blueprintable)
class PROJECTESTIVAL_API AAbilityProjectile : public AAbilityActor
{
	GENERATED_BODY()
	
public:
	AAbilityProjectile();

	/** Call before StartAbilityActor. */
	void SetupProjectile(float Speed, float Range);

	virtual void StartAbilityActor() override;
	virtual void PostInitializeComponents() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> Movement;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UArrowComponent> Arrow;

	virtual FAbilityContext MakeActorContext() const override;

private:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void HandleStop(const FHitResult& ImpactResult);

	bool CanHit(const AActor* Other) const;
};
