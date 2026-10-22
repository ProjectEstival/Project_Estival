#include "KnockbackPayload.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UKnockbackPayload::TryApply_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	const ACharacter* Character = Cast<ACharacter>(Target);
	if (!Character) return false;

	const FVector Dir = Context.Origin - Character->GetActorLocation();
	Character->GetCharacterMovement()->Launch(Dir * KnockbackDistance);
	return true;
}