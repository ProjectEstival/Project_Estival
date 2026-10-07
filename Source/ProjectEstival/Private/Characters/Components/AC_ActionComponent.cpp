// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Characters/Components/AC_ActionComponent.h"
#include "Public/Characters/Components/AC_StatusEffectComponent.h"
#include "Engine/World.h"

bool UAC_ActionComponent::TryActivate()
{
	if (!CanActivate())
	{
		return false;
	}

	LastActivationTime = GetWorld()->GetTimeSeconds();
	Execute();
	OnActivated.Broadcast();
	return true;
}

bool UAC_ActionComponent::CanActivate() const
{
	if (GetRemainingCooldown() > 0.0f)
	{
		return false;
	}

	// blocked by a status effect?
	const UAC_StatusEffectComponent* Status = GetOwner()->FindComponentByClass<UAC_StatusEffectComponent>();
	return !Status || !Status->HasAnyEffect(BlockedByTags);
}

float UAC_ActionComponent::GetRemainingCooldown() const
{
	return FMath::Max(0.0f, static_cast<float>(LastActivationTime + Cooldown - GetWorld()->GetTimeSeconds()));
}