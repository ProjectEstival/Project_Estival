#include "CheckCooldownCondition.h"
#include "StateTreeExecutionContext.h"
#include "AI/Enemy/EnemyBaseCharacter.h"

bool FCheckCooldownCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!IsValid(InstanceData.Enemy)) return false;
	
	return !InstanceData.Enemy->GetLoadout()->IsOnCooldown(InstanceData.AbilityTag);
}
