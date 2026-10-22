#include "KnockbackPayload.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UKnockbackPayload::TryApply_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	const ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (!TargetCharacter) return false;

	const FVector Dir = Context.Origin - TargetCharacter->GetActorLocation();
	TargetCharacter->GetCharacterMovement()->Launch(Dir.GetSafeNormal() * KnockbackDistance);
	return true;
}