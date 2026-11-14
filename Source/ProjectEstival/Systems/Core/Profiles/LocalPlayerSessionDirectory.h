#pragma once

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InputCoreTypes.h"
#include "CoreMinimal.h"

#include "LocalPlayerSessionDirectory.generated.h"

class APlayerController;
class ULocalPlayer;
class UWidget;
class FSlateUser;

// Runtime links for one local player. Input device IDs and platform user IDs are deliberately distinct
struct FLocalPlayerSessionRecord
{
	TWeakObjectPtr<ULocalPlayer> LocalPlayer;
	TWeakObjectPtr<APlayerController> PlayerController;
	FPlatformUserId PlatformUserId;
	TArray<FInputDeviceId> InputDevices;
	FGuid ProfileId;
	int32 PlayerColorSlot = INDEX_NONE;
	TWeakPtr<FSlateUser> SlateUser;
	TWeakObjectPtr<UWidget> FocusedWidget;
};

// Session-scoped lookup for the engine objects and identities associated with each local player
UCLASS(Config = Game)
class PROJECTESTIVAL_API ULocalPlayerSessionDirectory : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	ULocalPlayerSessionDirectory();
	
	void RegisterPlayer(ULocalPlayer* LocalPlayer);
	void UnregisterPlayer(ULocalPlayer* LocalPlayer);
	void AssociateInputDevice(ULocalPlayer* LocalPlayer, FInputDeviceId InputDeviceId);
	ULocalPlayer* RemoveInputDevice(FInputDeviceId InputDeviceId);
	void SetProfile(ULocalPlayer* LocalPlayer, const FGuid& ProfileId);
	void SetFocusedWidget(ULocalPlayer* LocalPlayer, UWidget* Widget);
	void FocusWidget(ULocalPlayer* LocalPlayer, UWidget* Widget);
	
	const FLocalPlayerSessionRecord* FindPlayer(const ULocalPlayer* LocalPlayer) const;
	const FLocalPlayerSessionRecord* FindPlayerForDevice(FInputDeviceId InputDeviceId) const;
	FGuid GetProfileId(const ULocalPlayer* LocalPlayer) const;
	UFUNCTION(BlueprintPure, Category = "Local Players")
	int32 GetPlayerColorSlot(const ULocalPlayer* LocalPlayer) const;
	UFUNCTION(BlueprintPure, Category = "Local Players")
	FLinearColor GetPlayerColor(const ULocalPlayer* LocalPlayer) const;
	UFUNCTION(BlueprintCallable, Category = "Local Players")
	void SetPlayerSlotColor(int32 Slot, FLinearColor Color);
	bool IsProfileAssigned(const FGuid& ProfileId, const ULocalPlayer* ExceptPlayer = nullptr) const;
	
private:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Player Colors", meta = (AllowPrivateAccess = "true"))
	TArray<int32> PlayerColorSlotOrder;
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Player Colors", meta = (AllowPrivateAccess = "true"))
	TMap<int32, FLinearColor> PlayerSlotColors;
	int32 FindAvailableColorSlot() const;
	void ApplyPlayerLightColor(const ULocalPlayer* LocalPlayer) const;
	TMap<TWeakObjectPtr<ULocalPlayer>, FLocalPlayerSessionRecord> Players;
	TMap<FInputDeviceId, TWeakObjectPtr<ULocalPlayer>> DeviceOwners;
};
