#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "StatusFilterPayload.generated.h"

/** Continues to its Success Chain only if the receiver has the status (or doesn't, inverted). */
UCLASS(meta = (DisplayName = "Status Filter"))
class PROJECTESTIVAL_API UStatusFilterPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status", meta = (Categories = "Status."))
	FGameplayTag Status;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
	bool bInvert = false;

	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};