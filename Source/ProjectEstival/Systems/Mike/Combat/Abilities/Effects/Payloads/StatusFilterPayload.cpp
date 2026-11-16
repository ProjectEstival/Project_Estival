#include "StatusFilterPayload.h"

#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"

bool UStatusFilterPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!IsValid(Target)) return false;
	
	const auto Character = Cast<AEstivalBaseCharacter>(Target);
	if (!Character) return false;
	
	const bool bHasStatus = Character->GetStatusEffects()->HasEffect(Status);
	return bHasStatus != bInvert;
}