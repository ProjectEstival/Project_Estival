// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraPresetAuthoringComponent.generated.h"

class UDataTable;

UCLASS(ClassGroup = (DevTools), meta = (BlueprintSpawnableComponent), DisplayName = "Camera Preset Authoring")
class UCameraPresetAuthoringComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Category = "Camera Preset Authoring")
	TObjectPtr<UDataTable> TargetPresetTable;
	
	UPROPERTY(EditAnywhere, Category = "Camera Preset Authoring", meta = (ClampMin = "1", ClampMax = "9"))
	int32 PresetSlotToSave = 1;
	
	UPROPERTY(EditAnywhere, Category = "Camera Preset Authoring")
	FString PresetNameToSave;

#if WITH_EDITOR
	
	UFUNCTION(CallInEditor, Category = "Camera Preset Authoring")
	void SaveCurrentCameraToPresetRow();
#endif
};
