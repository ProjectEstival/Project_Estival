// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Characters/Components/AC_StatusEffectComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UAC_StatusEffectComponent::AddEffect(FGameplayTag Tag, float Duration)
{
	if (!Tag.IsValid())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	// (re)start or clear the expiration timer
	FTimerHandle& Handle = Timers.FindOrAdd(Tag);
	TimerManager.ClearTimer(Handle);
	if (Duration > 0.0f)
	{
		TimerManager.SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this, Tag]() { RemoveEffect(Tag); }), Duration, false);;
	}

	if (!ActiveEffects.HasTagExact(Tag))
	{
		ActiveEffects.AddTag(Tag);
		OnEffectAdded.Broadcast(Tag);
	}
}

void UAC_StatusEffectComponent::RemoveEffect(FGameplayTag Tag)
{
	if (FTimerHandle* Handle = Timers.Find(Tag))
	{
		GetWorld()->GetTimerManager().ClearTimer(*Handle);
		Timers.Remove(Tag);
	}

	if (ActiveEffects.RemoveTag(Tag))
	{
		OnEffectRemoved.Broadcast(Tag);
	}
}

void UAC_StatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (TPair<FGameplayTag, FTimerHandle>& Pair : Timers)
	{
		GetWorld()->GetTimerManager().ClearTimer(Pair.Value);
	}
	Timers.Empty();

	Super::EndPlay(EndPlayReason);
}