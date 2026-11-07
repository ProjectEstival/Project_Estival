#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "Systems/Mike/Stats/StatTypes.h"
#include "ApplyModifiersPayload.generated.h"

UCLASS(meta = (DisplayName = "Apply Modifiers"))
class PROJECTESTIVAL_API UApplyModifiersPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers", meta = (Categories = "Stats."))
	TMap<FGameplayTag, FStatModifier> Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers", meta = (ClampMin = 0.01, Units = "s"))
	float Duration = 5.f;

	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};