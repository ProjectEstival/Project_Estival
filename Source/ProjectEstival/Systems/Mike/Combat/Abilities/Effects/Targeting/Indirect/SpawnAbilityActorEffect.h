#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetingEffectBase.h"
#include "SpawnAbilityActorEffect.generated.h"

class AAbilityActor;

/** Base for effects that spawn ability actors. The actors hit with this effect's payloads and run its expire effects when they end. */
UCLASS(Abstract)
class PROJECTESTIVAL_API USpawnAbilityActorEffect : public UTargetingEffectBase
{
	GENERATED_BODY()

public:
	void RunExpireEffects(const FAbilityContext& Context) const;
	FGameplayTag GetExpireOnEvent() const { return ExpireOnEvent; }

protected:
	/** Spawn position relative to the origin and direction (X forward, Y right, Z up). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FVector SpawnOffset = FVector::ZeroVector;

	/** Spawned actors expire (running their expire effects) when the caster sends this ability event. Use it to detonate traps with a follow-up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (Categories = "AbilityEvent"))
	FGameplayTag ExpireOnEvent;

	/** Spawned actors are cancelled (no expire effects) if the ability is interrupted while still active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	bool bCancelOnInterrupt = false;

	/** Run where a spawned actor ends (projectile impact or max range, zone end). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Expire")
	TArray<TObjectPtr<UAbilityEffect>> ExpireEffects;

	FTransform GetSpawnTransform(const FAbilityContext& Context) const;

	/** Takes an actor from the pool, new or reused, and gives it the context. Configure it, then call FinishSpawn. */
	AAbilityActor* BeginSpawn(const FAbilityContext& Context, TSubclassOf<AAbilityActor> ActorClass, const FTransform& Transform) const;

	/** Starts the actor. */
	void FinishSpawn(const FAbilityContext& Context, AAbilityActor* Actor) const;
};
