#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "ConditionalEffect.generated.h"

/** Runs Then when every condition passes, otherwise Else. */
UCLASS(meta = (DisplayName = "If"))
class PROJECTESTIVAL_API UConditionalEffect : public UAbilityEffect
{
	GENERATED_BODY()

public:
	virtual void Execute_Implementation(const FAbilityContext& Context) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "If")
	TArray<TObjectPtr<UAbilityPredicate>> Conditions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "If")
	TArray<TObjectPtr<UAbilityEffect>> Then;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "If")
	TArray<TObjectPtr<UAbilityEffect>> Else;
};
