#include "RepeaterEffect.h"
#include "TimerManager.h"
#include "Engine/World.h"

URepeaterEffect::URepeaterEffect()
{
	Lifetime = EAbilityTaskLifetime::KeepAbilityActive;
}

void URepeaterEffect::OnStart_Implementation()
{
	if (InitialDelay > 0.f) GetWorld()->GetTimerManager().SetTimer(Timer, this, &URepeaterEffect::Run, InitialDelay, false);
	else Run();
}

void URepeaterEffect::Run()
{
	ExecuteAll(Effects, GetContext());

	// The effects may have cancelled the ability and with it this task
	if (!IsRunning()) return;

	if (++RunCount >= TotalRepeats)
	{
		Finish();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(Timer, this, &URepeaterEffect::Run, Interval, false);
}

void URepeaterEffect::OnInputReleased_Implementation()
{
	if (bStopOnRelease) Finish();
}

float URepeaterEffect::GetProgress_Implementation() const
{
	const float TotalTime = InitialDelay + Interval * (TotalRepeats - 1);
	return TotalTime > 0.f ? GetElapsedTime() / TotalTime : -1.f;
}
