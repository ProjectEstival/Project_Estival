#include "WaitEventTask.h"

UWaitEventTask::UWaitEventTask()
{
	Lifetime = EAbilityTaskLifetime::EndWithAbility;
	bLifetimeEditable = false;
}

void UWaitEventTask::OnAbilityEvent_Implementation(const FGameplayTag ReceivedEvent)
{
	if (!ReceivedEvent.MatchesTagExact(Event)) return;

	ExecuteAll(Effects, GetContext());
	if (bOnce) Finish();
}
