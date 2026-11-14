#include "OverTimePayload.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"

bool UOverTimePayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!IsValid(Target) || Payloads.Num() == 0) return false;

	if (Status.IsValid())
	{

		const auto Character = Cast<AEstivalBaseCharacter>(Target);
		UAC_StatusEffectComponent* StatusEffects = Character ? Character->GetStatusEffects() : nullptr;
		if (!StatusEffects) return false;

		StatusEffects->AddEffect(Status, Duration + 0.01f);

		for (TInlineComponentArray<UOverTimeComponent*> OverTimeComps(Target); const UOverTimeComponent* Existing : OverTimeComps)
		{
			if (Existing->IsTicking(Status)) return true;
		}
	}

	UOverTimeComponent* OverTime = NewObject<UOverTimeComponent>(Target);
	OverTime->RegisterComponent();
	OverTime->Start(this, Context);

	return true;
}

void UOverTimeComponent::Start(const UOverTimePayload* InSource, const FAbilityContext& InContext)
{
	Source = InSource;
	Context = InContext;
	PulsesLeft = FMath::Max(1, FMath::FloorToInt(Source->Duration / Source->Interval + UE_KINDA_SMALL_NUMBER));

	GetWorld()->GetTimerManager().SetTimer(Timer, this, &UOverTimeComponent::Pulse, Source->Interval, true);
}

bool UOverTimeComponent::IsTicking(const FGameplayTag InStatus) const
{
	return Source && Source->Status == InStatus && IsValid(Context.Instigator);
}

void UOverTimeComponent::Pulse()
{
	const auto Character = Cast<AEstivalBaseCharacter>(GetOwner());
	const UAC_StatusEffectComponent* StatusEffects = Character ? Character->GetStatusEffects() : nullptr;
	const bool bStatusGone = Source->Status.IsValid() && !(StatusEffects && StatusEffects->HasEffect(Source->Status));

	if (bStatusGone || !IsValid(Context.Instigator))
	{
		Stop();
		return;
	}

	UAbilityEffectPayload::ApplyAll(Source->Payloads, Context, GetOwner());

	if (!Source->Status.IsValid() && --PulsesLeft <= 0) Stop();
}

void UOverTimeComponent::Stop()
{
	GetWorld()->GetTimerManager().ClearTimer(Timer);
	DestroyComponent();
}
