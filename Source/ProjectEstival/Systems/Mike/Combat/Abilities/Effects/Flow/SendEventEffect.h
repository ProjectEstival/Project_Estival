#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "SendEventEffect.generated.h"

/** Sends an ability event from the caster: wakes Wait Event tasks and reaches the caster's ability actors. */
UCLASS(meta = (DisplayName = "Send Event"))
class PROJECTESTIVAL_API USendEventEffect : public UAbilityEffect
{
	GENERATED_BODY()

public:
	virtual void Execute_Implementation(const FAbilityContext& Context) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event", meta = (Categories = "AbilityEvent"))
	FGameplayTag Event;
};
