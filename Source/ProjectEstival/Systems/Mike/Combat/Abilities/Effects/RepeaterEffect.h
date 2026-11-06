#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "RepeaterEffect.generated.h"

UCLASS(meta = (DisplayName = "Repeat/Delay Effect"))
class PROJECTESTIVAL_API URepeaterEffect : public UAbilityEffect
{
	GENERATED_BODY()
	
public:
	virtual void Execute_Implementation(const FAbilityContext& Context) const override;
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat", meta = (ClampMin = 1))
	int32 TotalRepeats = 3;

	/** Wait before the first run. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat", meta = (ClampMin = 0, Units = "s"))
	float InitialDelay = 0.f;

	/** Time between runs. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat", meta = (ClampMin = 0.01, Units = "s", EditCondition = "TotalRepeats > 1", EditConditionHides))
	float Interval = 0.2f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat")
	bool bStopOnInterrupt = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat")
	TArray<TObjectPtr<UAbilityEffect>> Effects;
	
private:
	void Run(const FAbilityContext& Context) const;
};