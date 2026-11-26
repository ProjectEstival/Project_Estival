#include "PushCharacterPayload.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Systems/Thanos/Components/AC_StatusEffectComponent.h"

bool UPushCharacterPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	const AEstivalBaseCharacter* TargetCharacter = Cast<AEstivalBaseCharacter>(Target);
	if (!TargetCharacter || TargetCharacter->GetStatusEffects()->HasEffect(TagDefinitions::Status_Grounded)) return false;

	const FVector TargetLocation = TargetCharacter->GetActorLocation();
	FVector Dir;
	
	switch (PushDirection)
	{
		case EPushDirection::AlongDirection: Dir = Context.Direction; break;
		case EPushDirection::TowardOrigin: Dir = Context.Origin - TargetLocation; break;
		default: Dir = TargetLocation - Context.Origin; break;
	}
	
	const auto Movement = TargetCharacter->GetCharacterMovement();
	Movement->StopMovementImmediately();
	Movement->Launch(Dir.GetSafeNormal() * KnockbackDistance);
	return true;
}
