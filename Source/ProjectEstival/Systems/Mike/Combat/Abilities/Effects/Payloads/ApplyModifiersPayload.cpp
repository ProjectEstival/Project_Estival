#include "ApplyModifiersPayload.h"

#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Mike/Stats/StatisticsComponent.h"

bool UApplyModifiersPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	if (!IsValid(Target) || Modifiers.Num() == 0) return false;

	const auto Character = Cast<AEstivalBaseCharacter>(Target);
	if (!Character) return false;

	Character->GetStatistics()->AddTimedModifierSet(Modifiers, this, Duration);
	return true;
}