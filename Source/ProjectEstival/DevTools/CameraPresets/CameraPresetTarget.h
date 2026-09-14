// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CameraPresetData.h"
#include "CameraPresetTarget.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCameraPresetTarget : public UInterface
{
	GENERATED_BODY()
};
class ICameraPresetTarget
{
	GENERATED_BODY()

public:

	//Reconfigures the camera rig to match the given preset
	UFUNCTION(BlueprintNativeEvent, Category = "Camera Preset")
	void ApplyCameraPreset(const FCameraPresetData& Preset);

	//Reads the rig's current configuration back out, e.g. to restore it later
	UFUNCTION(BlueprintNativeEvent, Category = "Camera Preset")
	FCameraPresetData CaptureCameraPreset() const;
};
