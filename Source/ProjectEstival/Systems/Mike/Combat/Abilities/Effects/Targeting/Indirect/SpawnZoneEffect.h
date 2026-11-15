#pragma once

#include "CoreMinimal.h"
#include "SpawnAbilityActorEffect.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetShape.h"
#include "SpawnZoneEffect.generated.h"

class AAbilityZone;

/** Spawns a zone that hits what is inside its shape with this effect's payloads, runs tick effects while alive and expire effects when it ends. */
UCLASS(meta = (DisplayName = "Spawn Zone"))
class PROJECTESTIVAL_API USpawnZoneEffect : public USpawnAbilityActorEffect
{
	GENERATED_BODY()

public:
	virtual void Execute_Implementation(const FAbilityContext& Context) const override;

	bool TracksOccupants() const { return Shape && EffectPayloads.Num() > 0; }
	float GetRehitInterval() const { return RehitInterval; }
	bool HasTickEffects() const { return TickEffects.Num() > 0; }
	float GetTickInterval() const { return TickInterval; }

	void CollectOccupants(const FAbilityContext& ZoneContext, TArray<AActor*>& OutActors) const;
	void RunTickEffects(const FAbilityContext& ZoneContext) const;
	void DrawDebug(const FAbilityContext& ZoneContext, float ForDuration) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	TSubclassOf<AAbilityZone> ZoneClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone", meta = (ClampMin = 0.1, Units = "s"))
	float Duration = 3.f;

	/** Follows the caster while alive (auras). Off = stays where it was spawned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	bool bAttachToCaster = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Hits")
	TObjectPtr<UTargetShape> Shape;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Hits")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Hits")
	bool bIncludeInstigator = false;

	/** Seconds before the same actor can be hit again while inside. 0 = each actor is hit once. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Hits", meta = (ClampMin = 0, Units = "s"))
	float RehitInterval = 0.f;

	/** Run at the zone every tick interval while it is alive. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Tick")
	TArray<TObjectPtr<UAbilityEffect>> TickEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Tick", meta = (ClampMin = 0.05, Units = "s"))
	float TickInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDrawDebug = false;
};
