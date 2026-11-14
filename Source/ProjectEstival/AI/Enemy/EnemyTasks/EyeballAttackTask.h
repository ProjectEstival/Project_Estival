#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "EyeballAttackTask.generated.h"

USTRUCT()
struct FEyeballAttackTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APawn> Pawn;
	
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(Categories="Ability."))
	FGameplayTag AbilityTag;

};

USTRUCT()
struct FEyeballAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FEyeballAttackTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
public:
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
};
