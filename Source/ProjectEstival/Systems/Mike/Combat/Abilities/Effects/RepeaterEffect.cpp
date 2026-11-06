#include "RepeaterEffect.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"

void URepeaterEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	if (!Context.World || !Context.Instigator) return;

	const TWeakObjectPtr WeakThis(this);

	for (int32 Index = 0; Index < TotalRepeats; ++Index)
	{
		const float Delay = InitialDelay + Interval * Index;

		if (Delay <= 0.f)
		{
			Run(Context);
			continue;
		}

		FTimerHandle Handle;
		
		Context.World->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateWeakLambda(Context.Instigator, [WeakThis, Context]()
			{
				if (const URepeaterEffect* Self = WeakThis.Get()) Self->Run(Context);
			}),
			Delay,
			false);

		if (bStopOnInterrupt && Context.Loadout) Context.Loadout->AddInterruptibleTimer(Context, Handle);
	}
}

void URepeaterEffect::Run(const FAbilityContext& Context) const
{
	for (const TObjectPtr<UAbilityEffect>& Effect : Effects)
	{
		if (Effect) Effect->Execute(Context);
	}
}