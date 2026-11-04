#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "Systems/Mike/Combat/Abilities/AbilityContext.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetingEffectBase.h"
#include "AbilityProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UArrowComponent;

UCLASS(Abstract, Blueprintable)
class PROJECTESTIVAL_API AAbilityProjectile : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	AAbilityProjectile();
	void Initialize(const FAbilityContext& InContext, const UTargetingEffectBase* InEffect, float Speed, float Range);
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void PostInitializeComponents() override;
	virtual void LifeSpanExpired() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> Movement;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UArrowComponent> Arrow;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnHitActor(AActor* Target);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile")
	void OnExpired(bool bHitWorld);
	
	virtual void Expire(bool bHitWorld);
	
private:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void HandleStop(const FHitResult& ImpactResult);
	
	bool CanHit(AActor* Other) const;
	
	UPROPERTY(Transient)
	FAbilityContext Context;
	
	UPROPERTY(Transient)
	FTargetHitTracker Hits;
	
	TWeakObjectPtr<const UTargetingEffectBase> Effect;
	bool bExpired = false;
};