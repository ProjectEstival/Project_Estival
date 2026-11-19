#include "EyeballAttackTask.h"
#include "StateTreeExecutionContext.h"
#include "AI/Enemy/EnemyBaseCharacter.h"


EStateTreeRunStatus FEyeballAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FEyeballAttackTaskInstanceData& InstanceData = Context.GetInstanceData<FEyeballAttackTaskInstanceData>(*this);
	if (!IsValid(InstanceData.Enemy))
	{
		return EStateTreeRunStatus::Failed;
	}
	
	AEnemyBaseCharacter* EnemyChar = Cast<AEnemyBaseCharacter>(InstanceData.Enemy);
	if (!EnemyChar || !InstanceData.AbilityTag.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}
	
	const bool bActivated = EnemyChar->TryActivateAbility(InstanceData.AbilityTag);
	
	return bActivated ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

