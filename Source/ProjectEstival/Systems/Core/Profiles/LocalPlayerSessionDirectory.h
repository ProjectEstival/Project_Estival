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
	TWeakPtr<FSlateUser> SlateUser;
	TWeakObjectPtr<UWidget> FocusedWidget;
};

// Session-scoped lookup for the engine objects and identities associated with each local player
UCLASS()
class PROJECTESTIVAL_API ULocalPlayerSessionDirectory : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
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
	bool IsProfileAssigned(const FGuid& ProfileId, const ULocalPlayer* ExceptPlayer = nullptr) const;
	
private:
	TMap<TWeakObjectPtr<ULocalPlayer>, FLocalPlayerSessionRecord> Players;
	TMap<FInputDeviceId, TWeakObjectPtr<ULocalPlayer>> DeviceOwners;
};
