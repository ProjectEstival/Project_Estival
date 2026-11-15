#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "RunEffectsPayload.generated.h"

/** Runs effects at the receiver: Origin moves to it and Direction points from the previous origin toward it (explosion on hit). An Area inside it also finds the receiver. */
UCLASS(meta = (DisplayName = "Run Effects"))
class PROJECTESTIVAL_API URunEffectsPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run Effects")
	TArray<TObjectPtr<UAbilityEffect>> Effects;

	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};
