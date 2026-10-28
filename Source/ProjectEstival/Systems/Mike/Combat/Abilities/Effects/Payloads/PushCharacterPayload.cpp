#include "PushCharacterPayload.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UPushCharacterPayload::TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const
{
	const ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (!TargetCharacter) return false;

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
