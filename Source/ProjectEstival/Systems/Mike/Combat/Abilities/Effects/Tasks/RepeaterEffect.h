#pragma once

#include "CoreMinimal.h"
#include "AbilityTask.h"
#include "RepeaterEffect.generated.h"

/** Runs its effects a number of times with a delay before and between runs. With one repeat it's a plain delay. */
UCLASS(meta = (DisplayName = "Repeat / Delay"))
class PROJECTESTIVAL_API URepeaterEffect : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	URepeaterEffect();
	virtual float GetProgress_Implementation() const override;

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
	bool bStopOnRelease = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat")
	TArray<TObjectPtr<UAbilityEffect>> Effects;

	virtual void OnStart_Implementation() override;
	virtual void OnInputReleased_Implementation() override;

private:
	void Run();

	FTimerHandle Timer;
	int32 RunCount = 0;
};
