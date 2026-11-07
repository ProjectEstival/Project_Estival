#include "HasStatusPredicate.h"

#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"

bool UHasStatusPredicate::IsSatisfied_Implementation(const FAbilityContext& Context) const
{
	const AActor* Actor = Check == ETargetOrCaster::Caster ? Context.Instigator.Get() : Context.TargetActor.Get();
	const AEstivalBaseCharacter* Character = IsValid(Actor) ? Actor->FindComponentByClass<AEstivalBaseCharacter>() : nullptr;

	const bool bHasStatus = Character && Character->GetStatusEffects()->HasEffect(Status);
	return bHasStatus != bInvert;
}
