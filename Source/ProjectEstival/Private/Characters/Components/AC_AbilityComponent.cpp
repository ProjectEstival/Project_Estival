// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Characters/Components/AC_AbilityComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

bool UAC_AbilityComponent::CanActivate() const
{
	return SpawnClass && Charges != 0 && Super::CanActivate();
}

void UAC_AbilityComponent::Execute()
{
	AActor* Owner = GetOwner();

	FTransform SpawnTransform = Owner->GetActorTransform();
	SpawnTransform.AddToTranslation(Owner->GetActorForwardVector() * SpawnOffset);

	FActorSpawnParameters Params;
	Params.Owner = Owner;
	Params.Instigator = Cast<APawn>(Owner);
	GetWorld()->SpawnActor<AActor>(SpawnClass, SpawnTransform, Params);

	if (Charges > 0)
	{
		--Charges;
	}
}

