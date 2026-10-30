#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "RepeaterEffect.generated.h"

UCLASS(meta = (DisplayName = "Repeat"))
class PROJECTESTIVAL_API URepeaterEffect : public UAbilityEffect
{
	GENERATED_BODY()
	
public:
	virtual void Execute_Implementation(const FAbilityContext& Context) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat")
	TArray<TObjectPtr<UAbilityEffect>> Effects;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat", meta = (ClampMin = 2))
	int32 TotalRepeats = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat", meta = (ClampMin = 0.01, Units = "s"))
	float Interval = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat")
	bool bFollowCaster = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repeat")
	bool bStopOnInterrupt = true;
	
private:
	void Run(const FAbilityContext& Context) const;
};