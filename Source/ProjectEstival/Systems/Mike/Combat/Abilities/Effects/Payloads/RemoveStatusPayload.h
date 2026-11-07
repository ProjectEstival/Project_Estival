#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "RemoveStatusPayload.generated.h"

/** Removes statuses from the receiver. Succeeds if at least one was present. */
UCLASS(meta = (DisplayName = "Remove Status"))
class PROJECTESTIVAL_API URemoveStatusPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status", meta = (Categories = "Status"))
	FGameplayTagContainer Statuses;

	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};
