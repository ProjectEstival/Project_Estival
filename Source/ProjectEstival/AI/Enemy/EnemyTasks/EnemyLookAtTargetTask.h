#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "EnemyLookAtTargetTask.generated.h"

class AAIController;

USTRUCT()
struct FEnemyLookAtTargetTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APawn> Enemy;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<APawn> Player;
	
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float TurnSpeed = 20.f;
	
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxThreshold = 1.f;
	
};

USTRUCT()
struct FEnemyLookAtTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FEnemyLookAtTargetTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
public:
	
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
};
