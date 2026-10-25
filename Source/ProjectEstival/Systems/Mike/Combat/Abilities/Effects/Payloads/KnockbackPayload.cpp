#include "KnockbackPayload.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UKnockbackPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	const ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (!TargetCharacter) return false;

	const auto Movement = TargetCharacter->GetCharacterMovement();
	FVector Dir = Context.Origin - TargetCharacter->GetActorLocation();    
	Dir.Z = 0.f;
	
	Movement->StopMovementImmediately();
	Movement->Launch(-Dir.GetSafeNormal() * KnockbackDistance);
	return true;
}