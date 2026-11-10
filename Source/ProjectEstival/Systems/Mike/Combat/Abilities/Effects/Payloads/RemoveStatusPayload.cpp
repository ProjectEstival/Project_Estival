#include "RemoveStatusPayload.h"

#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"

bool URemoveStatusPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!IsValid(Target)) return false;
	
	const auto Character = Cast<AEstivalBaseCharacter>(Target);
	if (!Character) return false;
	
	const auto StatusEffects = Character->GetStatusEffects();
	bool bRemovedAny = false;

	for (const FGameplayTag& Status : Statuses)
	{
		if (!StatusEffects->HasEffect(Status)) continue;

		StatusEffects->RemoveEffect(Status);
		bRemovedAny = true;
	}

	return bRemovedAny;
}