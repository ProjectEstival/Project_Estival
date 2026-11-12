#include "EyeballAttackTask.h"
#include "StateTreeExecutionContext.h"
#include "AI/Enemy/EnemyBaseCharacter.h"


EStateTreeRunStatus FEyeballAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FEyeballAttackTaskInstanceData& InstanceData = Context.GetInstanceData<FEyeballAttackTaskInstanceData>(*this);
	if (!IsValid(InstanceData.Pawn))
	{
		return EStateTreeRunStatus::Failed;
	}
	
	AEnemyBaseCharacter* EnemyChar = Cast<AEnemyBaseCharacter>(InstanceData.Pawn);
	if (!EnemyChar || !InstanceData.AbilityTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}
	

	
	return EStateTreeRunStatus::Succeeded;
}

