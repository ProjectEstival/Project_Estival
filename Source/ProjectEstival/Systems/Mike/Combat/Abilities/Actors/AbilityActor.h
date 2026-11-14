#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "Systems/Mike/Combat/Abilities/AbilityContext.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetingEffectBase.h"
#include "Systems/Maria/Pooling System/Poolable.h"
#include "AbilityActor.generated.h"

class USpawnAbilityActorEffect;

/**
 * Actor spawned by an ability (projectile, zone). Carries the cast's context, hits with the payloads of the effect that spawned it and runs that effect's expire effects when it ends.
 * It comes from the actor pool and goes back to it, so one actor is reused many times: per-use setup goes in StartAbilityActor or On Started, not BeginPlay.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTESTIVAL_API AAbilityActor : public AActor, public IGenericTeamAgentInterface, public IPoolable
{
	GENERATED_BODY()

public:
	/** Gives the actor this use's context. Configure it next, then call StartAbilityActor. */
	void InitializeAbilityActor(const FAbilityContext& InContext, const USpawnAbilityActorEffect* InSource);

	/** Starts this use, for a new actor and a reused one alike. An override must call Super. */
	virtual void StartAbilityActor();

	void SetLifetime(const float Seconds) { SetLifeSpan(Seconds); }

	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void LifeSpanExpired() override final;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnReleased_Implementation() override final;

	/** Ends the actor and runs the expire effects. */
	UFUNCTION(BlueprintCallable, Category = "Ability Actor")
	void Expire();

	/** Ends the actor without running the expire effects. */
	UFUNCTION(BlueprintCallable, Category = "Ability Actor")
	void Cancel();

	UFUNCTION(BlueprintPure, Category = "Ability Actor")
	const FAbilityContext& GetAbilityContext() const { return Context; }

	UFUNCTION(BlueprintPure, Category = "Ability Actor")
	bool IsExpired() const { return bExpired; }

protected:
	/** Runs every time the actor starts, new or reused from the pool. Use it instead of BeginPlay, which only runs the first time. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability Actor")
	void OnStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability Actor")
	void OnHitActor(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability Actor")
	void OnExpired(bool bCancelled);

	/** The caster sent an ability event (Send Event effect, animation notify). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability Actor")
	void OnAbilityEvent(FGameplayTag Event);

	/** The cast's context, moved to this actor's location and facing. */
	virtual FAbilityContext MakeActorContext() const;

	bool TryHit(AActor* Target);
	const USpawnAbilityActorEffect* GetSource() const { return Source.Get(); }

	UPROPERTY(Transient)
	FTargetHitTracker Hits;

private:
	UFUNCTION()
	void HandleAbilityEvent(FGameplayTag Event);

	void Finish(bool bCancelled);
	void CleanUp();

	UPROPERTY(Transient)
	FAbilityContext Context;

	TWeakObjectPtr<const USpawnAbilityActorEffect> Source;
	bool bExpired = false;
};
