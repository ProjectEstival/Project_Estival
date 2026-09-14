// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraPresetSwitcherComponent.h"
#include "CameraPresetTarget.h"
#include "Components/InputComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"

UCameraPresetSwitcherComponent::UCameraPresetSwitcherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCameraPresetSwitcherComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableDebugKeyBindings)
	{
		BindDebugKeys();
	}
}

void UCameraPresetSwitcherComponent::BindDebugKeys()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());

	if (!PC || !PC->InputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPresetSwitcherComponent must be added to a PlayerController with an InputComponent to bind debug keys."));
		return;
	}

	PC->InputComponent->BindKey(EKeys::One, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey1);
	PC->InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey2);
	PC->InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey3);
	PC->InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey4);
	PC->InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey5);
	PC->InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey6);
	PC->InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey7);
	PC->InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey8);
	PC->InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnPresetKey9);
	PC->InputComponent->BindKey(EKeys::Zero, IE_Pressed, this, &UCameraPresetSwitcherComponent::OnReturnKey);
}

void UCameraPresetSwitcherComponent::ActivatePreset(int32 Slot)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	if (!Pawn || !Pawn->GetClass()->ImplementsInterface(UCameraPresetTarget::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPresetSwitcherComponent: possessed pawn does not implement ICameraPresetTarget."));
		return;
	}

	if (!PresetTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPresetSwitcherComponent: no Preset Table assigned."));
		return;
	}

	const FName RowName(*FString::FromInt(Slot));
	const FCameraPresetData* Row = PresetTable->FindRow<FCameraPresetData>(RowName, TEXT("CameraPresetSwitcherComponent::ActivatePreset"));

	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("No camera preset row named '%d' found in the Preset Table."), Slot);
		return;
	}

	// capture the rig's starting configuration the first time it's touched, so 0 can restore it
	if (!bHasCapturedDefault)
	{
		DefaultPreset = ICameraPresetTarget::Execute_CaptureCameraPreset(Pawn);
		bHasCapturedDefault = true;
	}

	ICameraPresetTarget::Execute_ApplyCameraPreset(Pawn, *Row);
}

void UCameraPresetSwitcherComponent::ReturnToDefault()
{
	if (!bHasCapturedDefault)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	if (Pawn && Pawn->GetClass()->ImplementsInterface(UCameraPresetTarget::StaticClass()))
	{
		ICameraPresetTarget::Execute_ApplyCameraPreset(Pawn, DefaultPreset);
	}
}
