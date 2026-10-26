// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"

APlayerBase::APlayerBase()
{
	ResurrectionComponent = CreateDefaultSubobject<UReviveComponent>(TEXT("Resurrection"));
}

void APlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input || !ReviveAction) return;
	
	Input->BindAction(ReviveAction, ETriggerEvent::Started, this, &APlayerBase::StartRevive);
	Input->BindAction(ReviveAction, ETriggerEvent::Completed, this, &APlayerBase::StopRevive);
	Input->BindAction(ReviveAction,ETriggerEvent::Canceled,this, &APlayerBase::StopRevive);
}

void APlayerBase::StartRevive()
{
	ResurrectionComponent->StartReviveNearby();
}

void APlayerBase::StopRevive()
{
	ResurrectionComponent->StopReviveNearby();
}