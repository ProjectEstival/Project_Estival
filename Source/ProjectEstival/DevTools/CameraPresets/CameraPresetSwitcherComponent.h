// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraPresetData.h"
#include "CameraPresetSwitcherComponent.generated.h"

class UDataTable;

UCLASS(ClassGroup = (DevTools), meta = (BlueprintSpawnableComponent), DisplayName = "Camera Preset Switcher")
class UCameraPresetSwitcherComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCameraPresetSwitcherComponent();
	
	UPROPERTY(EditAnywhere, Category = "Camera Preset Switcher", meta = (RequiredAssetDataTags = "RowStructure=CameraPresetData"))
	TObjectPtr<UDataTable> PresetTable;

	//If false, number keys are not bound - use this to disable the tool in non-dev builds
	UPROPERTY(EditAnywhere, Category = "Camera Preset Switcher")
	bool bEnableDebugKeyBindings = true;

	//Applies the preset registered for the given slot (1-9), if one exists
	UFUNCTION(BlueprintCallable, Category = "Camera Preset Switcher")
	void ActivatePreset(int32 Slot);

	/** Restores the camera rig to whatever it was configured to when play started. */
	UFUNCTION(BlueprintCallable, Category = "Camera Preset Switcher")
	void ReturnToDefault();

protected:

	virtual void BeginPlay() override;

private:

	void BindDebugKeys();

	// One thin wrapper per number key: UInputComponent::BindKey needs a concrete
	//no-argument member function pointer, so these simply forward to ActivatePreset/ReturnToDefault.
	void OnPresetKey1() { ActivatePreset(1); }
	void OnPresetKey2() { ActivatePreset(2); }
	void OnPresetKey3() { ActivatePreset(3); }
	void OnPresetKey4() { ActivatePreset(4); }
	void OnPresetKey5() { ActivatePreset(5); }
	void OnPresetKey6() { ActivatePreset(6); }
	void OnPresetKey7() { ActivatePreset(7); }
	void OnPresetKey8() { ActivatePreset(8); }
	void OnPresetKey9() { ActivatePreset(9); }
	void OnReturnKey() { ReturnToDefault(); }

	/** The rig's own configuration, captured from the pawn the first time a preset is applied. */
	FCameraPresetData DefaultPreset;
	bool bHasCapturedDefault = false;
};
