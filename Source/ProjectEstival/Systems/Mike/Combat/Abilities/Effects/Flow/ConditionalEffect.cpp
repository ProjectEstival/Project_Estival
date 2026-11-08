#include "ConditionalEffect.h"

void UConditionalEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	bool bPassed = true;

	for (const TObjectPtr<UAbilityPredicate>& Condition : Conditions)
	{
		if (Condition && !Condition->IsSatisfied(Context))
		{
			bPassed = false;
			break;
		}
	}

	ExecuteAll(bPassed ? Then : Else, Context);
}
