#include "RepeaterEffect.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Systems/Mike/Combat/Loadouts/LoadoutComponent.h"

void URepeaterEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	Run(Context);
	
	if (TotalRepeats <= 1 || !Context.World) return;
	if (!Context.Instigator) return;
	
	const TWeakObjectPtr WeakThis(this);
	
	for (int32 Index = 1; Index < TotalRepeats; ++Index)
	{
		FTimerHandle Handle;
		
		Context.World->GetTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateWeakLambda(Context.Instigator, [WeakThis, Context]()
			{
				if (const URepeaterEffect* Self = WeakThis.Get()) Self->Run(Context);
			}),
			Interval * Index,
			false);
		
		if (bStopOnInterrupt && Context.Loadout) Context.Loadout->AddInterruptibleTimer(Handle);
	}
}

void URepeaterEffect::Run(const FAbilityContext& Context) const
{
	FAbilityContext RunContext = Context;
	
	if (bFollowCaster && IsValid(Context.Instigator))
	{
		RunContext.Origin = Context.Instigator->GetActorLocation();
		RunContext.Direction = Context.Instigator->GetActorForwardVector().GetSafeNormal2D();
	}
	
	for (const TObjectPtr<UAbilityEffect>& Effect : Effects)
	{
		if (Effect) Effect->Execute(RunContext);
	}
}