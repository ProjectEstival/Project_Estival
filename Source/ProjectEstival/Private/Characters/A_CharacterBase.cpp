// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Characters/A_CharacterBase.h"
#include "Public/Characters/Components/AC_LookAtComponent.h"
#include "Public/Characters/Components/AC_StatusEffectComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"

AA_EstivalBaseCharacter::AA_EstivalBaseCharacter()
{
	// components do their own ticking
	PrimaryActorTick.bCanEverTick = false;
	
	LookAt = CreateDefaultSubobject<UAC_LookAtComponent>(TEXT("LookAt"));
	StatusEffects = CreateDefaultSubobject<UAC_StatusEffectComponent>(TEXT("StatusEffects"));

	// rotation is driven by the LookAt component
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AA_EstivalBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		return;
	}

	if (MoveAction)
	{
		Input->BindActionValueLambda(MoveAction, ETriggerEvent::Triggered, [this](const FInputActionValue& Value)
		{
			const FVector2D Axis = Value.Get<FVector2D>();
			DoMove(Axis.X, Axis.Y);
		});
	}
}

void AA_EstivalBaseCharacter::DoMove(float AxisX, float AxisY)
{
	FRotator FlatRot = GetControlRotation();
	FlatRot.Pitch = 0.0f;

	AddMovementInput(FlatRot.RotateVector(FVector::ForwardVector), AxisX);
	AddMovementInput(FlatRot.RotateVector(FVector::RightVector), AxisY);
}