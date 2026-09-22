// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CameraPresetData.generated.h"

USTRUCT(BlueprintType)
struct FCameraPresetData : public FTableRowBase
{
	GENERATED_BODY()

	//Optional label shown in logs to help designers tell presets apart
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FString PresetName;

	//Distance from the character to the camera along the boom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset", meta = (ClampMin = "0.0", Units = "cm"))
	float ArmLength = 800.0f;

	//Fixed (absolute) rotation of the boom - pitch is the "top down" angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FRotator ArmRotation = FRotator(-60.0f, 0.0f, 0.0f);

	//Ortho width if the camera is orthographic, or FOV (degrees) if perspective
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset", meta = (ClampMin = "1.0"))
	float OrthoWidthOrFOV = 2000.0f;

	//Data Table row name for a slot - the switcher (reads) and the authoring tool (writes) must agree on this
	static FName RowNameForSlot(int32 Slot) { return FName(*FString::FromInt(Slot)); }
};
