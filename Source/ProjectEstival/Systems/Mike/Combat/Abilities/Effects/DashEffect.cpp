// Fill out your copyright notice in the Description page of Project Settings.


#include "DashEffect.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UDashEffect::Execute_Implementation(const FAbilityContext& Context) const
{
	if (const auto Character = Cast<ACharacter>(Context.Instigator))
	{
		const FVector Direction = Character->GetLastMovementInputVector().GetSafeNormal();
		if (Direction.IsNearlyZero()) return;
		
		Character->GetCharacterMovement()->Launch(Direction * DashDistance);
	}
}