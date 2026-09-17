// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraPresetAuthoringComponent.h"
#include "CameraPresetData.h"
#include "CameraPresetTarget.h"
#include "Engine/DataTable.h"

#if WITH_EDITOR
void UCameraPresetAuthoringComponent::SaveCurrentCameraToPresetRow()
{
	AActor* Owner = GetOwner();

	if (!Owner || !Owner->GetClass()->ImplementsInterface(UCameraPresetTarget::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPresetAuthoringComponent: owning actor does not implement ICameraPresetTarget."));
		return;
	}

	if (!TargetPresetTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraPresetAuthoringComponent: no Target Preset Table assigned."));
		return;
	}

	FCameraPresetData Preset = ICameraPresetTarget::Execute_CaptureCameraPreset(Owner);
	Preset.PresetName = PresetNameToSave.IsEmpty() ? FString::FromInt(PresetSlotToSave) : PresetNameToSave;

	const FName RowName(*FString::FromInt(PresetSlotToSave));

	TargetPresetTable->Modify();
	TargetPresetTable->AddRow(RowName, Preset);
	TargetPresetTable->MarkPackageDirty();
	TargetPresetTable->OnDataTableChanged().Broadcast();

	UE_LOG(LogTemp, Log, TEXT("Saved camera preset to row '%d' (Arm Length %.1f, Rotation %s, Ortho/FOV %.1f). Remember to save the Data Table asset."),
		PresetSlotToSave, Preset.ArmLength, *Preset.ArmRotation.ToString(), Preset.OrthoWidthOrFOV);
}
#endif
