#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "EnemyUseAbility.generated.h"

class AEnemyBaseCharacter;
class UAnimMontage;

USTRUCT()
struct FEyeballAttackTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AEnemyBaseCharacter> Enemy;
	
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(Categories="Ability."))
	FGameplayTag AbilityTag;
	
};

USTRUCT()
struct FEnemyUseAbility : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FEyeballAttackTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
public:
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
};
