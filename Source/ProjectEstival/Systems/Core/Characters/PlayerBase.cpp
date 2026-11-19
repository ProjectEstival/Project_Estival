// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"

APlayerBase::APlayerBase()
{
	ResurrectionComponent = CreateDefaultSubobject<UReviveComponent>(TEXT("Resurrection"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
}

void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input) return;
	
	if (InteractAction)
	{
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerBase::StartInteract);
		Input->BindAction(InteractAction, ETriggerEvent::Completed, this, &APlayerBase::StopInteract);
		Input->BindAction(InteractAction,ETriggerEvent::Canceled,this, &APlayerBase::StopInteract);
	}
}

void APlayerBase::StartInteract()
{
	InteractionComponent->StartInteract();
}

void APlayerBase::StopInteract()
{
	InteractionComponent->StopInteract();
}