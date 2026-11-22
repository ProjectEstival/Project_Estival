#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "ReduceCooldownsPayload.generated.h"

/** Takes time off the receiver's ability cooldowns. Succeeds only if one of the slots was on cooldown. */
UCLASS(meta = (DisplayName = "Reduce Cooldowns"))
class PROJECTESTIVAL_API UReduceCooldownsPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()

protected:
	/** Seconds taken off each slot's cooldown. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cooldowns", meta = (Categories = "Ability.", ClampMin = 0))
	TMap<FGameplayTag, float> ReduceSeconds;

	/** Part of each slot's cooldown taken off. 50 takes away half of the STARTING cooldown amount, 100 makes it ready. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cooldowns", meta = (Categories = "Ability.", ClampMin = 0, ClampMax = 100))
	TMap<FGameplayTag, float> ReducePercent;

	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};
