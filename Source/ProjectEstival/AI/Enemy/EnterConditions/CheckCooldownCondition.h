#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "GameplayTagContainer.h"
#include "CheckCooldownCondition.generated.h"

class AEnemyBaseCharacter;

USTRUCT()
struct FCheckCooldownConditionInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AEnemyBaseCharacter> Enemy;
	
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (Categories = "Ability"))
	FGameplayTag AbilityTag;
	
};

USTRUCT(meta = (DisplayName = "Check Cooldown"))
struct FCheckCooldownCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FCheckCooldownConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

protected:
	
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
};