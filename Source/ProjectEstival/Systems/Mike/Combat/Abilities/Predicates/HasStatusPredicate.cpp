#include "HasStatusPredicate.h"

#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"

bool UHasStatusPredicate::IsSatisfied_Implementation(const FAbilityContext& Context) const
{
	const AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(Context.Instigator);

	const bool bHasStatus = Character && Character->GetStatusEffects()->HasEffect(Status);
	return bHasStatus != bInvert;
}
