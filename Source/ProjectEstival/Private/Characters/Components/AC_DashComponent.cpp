// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Characters/Components/AC_DashComponent.h"
#include "GameFramework/Character.h"

void UAC_DashComponent::Execute()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	//dash toward the last move input
	FVector Direction = Character->GetLastMovementInputVector().GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = Character->GetActorForwardVector();
	}

	Character->LaunchCharacter(Direction * DashImpulse, true, true);
}