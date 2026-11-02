// Fill out your copyright notice in the Description page of Project Settings.


#include "AddStatusPayload.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"

bool UAddStatusPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (const auto Character = Cast<AEstivalBaseCharacter>(Target))
	{
		for (const auto Item : StatusDurations) Character->GetStatusEffects()->AddEffect(Item.Key, Item.Value);
		return true;
	}
	return false;
}