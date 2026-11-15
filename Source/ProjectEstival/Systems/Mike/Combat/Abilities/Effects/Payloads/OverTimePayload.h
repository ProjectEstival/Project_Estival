#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "OverTimePayload.generated.h"

/** Applies its payloads to the receiver every interval for a duration (poison, burn, regen). Stops if the caster is gone. */
UCLASS(meta = (DisplayName = "Over Time"))
class PROJECTESTIVAL_API UOverTimePayload : public UAbilityEffectPayload
{
	GENERATED_BODY()

	friend class UOverTimeComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Over Time", meta = (ClampMin = 0.05, Units = "s"))
	float Duration = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Over Time", meta = (ClampMin = 0.05, Units = "s"))
	float Interval = 1.f;

	/** Optional. Added to the receiver for the duration and ticks run while it is present. Re-applying refreshes instead of stacking, removing it stops the ticks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Over Time", meta = (Categories = "Status"))
	FGameplayTag Status;

	/** Applied to the receiver on every tick. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Over Time")
	TArray<TObjectPtr<UAbilityEffectPayload>> Payloads;

	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};

/** Sits on the receiver while an Over Time payload ticks, then removes itself. */
UCLASS()
class UOverTimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void Start(const UOverTimePayload* InSource, const FAbilityContext& InContext);
	bool IsTicking(FGameplayTag InStatus) const;

private:
	void Pulse();
	void Stop();

	UPROPERTY()
	TObjectPtr<const UOverTimePayload> Source;

	UPROPERTY()
	FAbilityContext Context;

	FTimerHandle Timer;
	int32 PulsesLeft = 0;
};
