#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "ModifyResource.generated.h"

USTRUCT(BlueprintType)
struct FModifyOption
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Stats.Resources."))
	FGameplayTag ModifyResource;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bFromStat = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Stats.", EditCondition = "bFromStat", EditConditionHides))
	FGameplayTag AmountStat;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.01, EditCondition = "bFromStat", EditConditionHides))
	float Multiplier = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.01, EditCondition = "bFromStat == false", EditConditionHides))
	float Amount = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool Deplete = false;
};

UCLASS()
class PROJECTESTIVAL_API UModifyResource : public UAbilityEffectPayload
{
	GENERATED_BODY()
	
	protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FModifyOption> ResourceChanges;
	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};