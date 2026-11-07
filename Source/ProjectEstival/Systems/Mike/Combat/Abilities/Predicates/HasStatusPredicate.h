#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "HasStatusPredicate.generated.h"

UCLASS(meta = (DisplayName = "Has Status"))
class PROJECTESTIVAL_API UHasStatusPredicate : public UAbilityPredicate
{
	GENERATED_BODY()

public:
	virtual bool IsSatisfied_Implementation(const FAbilityContext& Context) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
	ETargetOrCaster Check = ETargetOrCaster::Caster;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status", meta = (Categories = "Status."))
	FGameplayTag Status;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Status")
	bool bInvert = false;
};