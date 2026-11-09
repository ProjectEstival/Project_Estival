#include "WaitEventTask.h"
#include "Systems/Mike/Combat/Loadouts/RuntimeAbilities.h"

UWaitEventTask::UWaitEventTask()
{
	Lifetime = EAbilityTaskLifetime::EndWithAbility;
	bLifetimeEditable = false;
}

void UWaitEventTask::OnStart_Implementation()
{
	// Events only reach the running ability's tasks, so without one this would wait forever
	const UAbilityActivation* Activation = GetActivation();
	if (!Activation || !Activation->IsActive()) Finish();
}

void UWaitEventTask::OnAbilityEvent_Implementation(const FGameplayTag ReceivedEvent)
{
	if (!ReceivedEvent.MatchesTagExact(Event)) return;

	ExecuteAll(Effects, GetContext());
	if (bOnce) Finish();
}
