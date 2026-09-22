// Copyright Epic Games, Inc. All Rights Reserved.
//jf test
#include "CameraPresetAuthoringComponent.h"
#include "CameraPresetData.h"
#include "CameraPresetTarget.h"

#if WITH_EDITOR

#include "CameraPresetTableWriter.h"
#include "Engine/DataTable.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace
{
	void Report(bool bSuccess, const FString& Message)
	{
		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("CameraPresetAuthoring: %s"), *Message);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CameraPresetAuthoring: %s"), *Message);
		}

		FNotificationInfo Info(FText::FromString(Message));
		Info.ExpireDuration = bSuccess ? 4.0f : 8.0f;

		if (TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Notification->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		}
	}
}

void UCameraPresetAuthoringComponent::SaveCurrentCameraToPresetRow()
{
	AActor* Owner = GetOwner();

	if (!Owner || !Owner->GetClass()->ImplementsInterface(UCameraPresetTarget::StaticClass()))
	{
		Report(false, TEXT("The owning actor does not implement Camera Preset Target."));
		return;
	}

	if (!TargetPresetTable)
	{
		Report(false, TEXT("No Target Preset Table assigned."));
		return;
	}

	FCameraPresetData Preset = ICameraPresetTarget::Execute_CaptureCameraPreset(Owner);
	Preset.PresetName = PresetNameToSave.IsEmpty() ? FString::FromInt(PresetSlotToSave) : PresetNameToSave;

	if (!FCameraPresetTableWriter::WriteRow(*TargetPresetTable, FCameraPresetData::RowNameForSlot(PresetSlotToSave), Preset))
	{
		Report(false, FString::Printf(TEXT("%s does not use the Camera Preset Data row type, nothing was written."), *TargetPresetTable->GetName()));
		return;
	}

	Report(true, FString::Printf(TEXT("Saved slot %d to %s (arm %.0f, rotation %s, zoom %.0f). Save the Data Table asset to keep it."),
		PresetSlotToSave, *TargetPresetTable->GetName(), Preset.ArmLength, *Preset.ArmRotation.ToString(), Preset.OrthoWidthOrFOV));
}

#endif
