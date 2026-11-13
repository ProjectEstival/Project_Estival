#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"

#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Components/Widget.h"

void ULocalPlayerSessionDirectory::RegisterPlayer(ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer)) return;
	const TWeakObjectPtr<ULocalPlayer> PlayerKey(LocalPlayer);
	FLocalPlayerSessionRecord& Record = Players.FindOrAdd(PlayerKey);
	Record.LocalPlayer = LocalPlayer;
	Record.PlatformUserId = LocalPlayer->GetPlatformUserId();
	Record.SlateUser = LocalPlayer->GetSlateUser();
	Record.PlayerController = LocalPlayer->GetPlayerController(GetWorld());
}
void ULocalPlayerSessionDirectory::UnregisterPlayer(ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer)) return;
	const TWeakObjectPtr<ULocalPlayer> PlayerKey(LocalPlayer);
	for (auto It = DeviceOwners.CreateIterator(); It; ++It)
	{
		if (It.Value().Get() == LocalPlayer) It.RemoveCurrent();
	}
	Players.Remove(PlayerKey);
}

void ULocalPlayerSessionDirectory::AssociateInputDevice(ULocalPlayer* LocalPlayer, const FInputDeviceId InputDeviceId)
{
	if (!IsValid(LocalPlayer) || !InputDeviceId.IsValid()) return;
	RegisterPlayer(LocalPlayer);
	if (const TWeakObjectPtr<ULocalPlayer>* PreviousOwner = DeviceOwners.Find(InputDeviceId))
	{
		if (PreviousOwner->IsValid() && PreviousOwner->Get() != LocalPlayer)
		{
			if (FLocalPlayerSessionRecord* PreviousRecord = Players.Find(*PreviousOwner)) PreviousRecord->InputDevices.Remove(InputDeviceId);
		}
	}
	FLocalPlayerSessionRecord& Record = Players.FindChecked(TWeakObjectPtr<ULocalPlayer>(LocalPlayer));
	if (!Record.InputDevices.Contains(InputDeviceId)) Record.InputDevices.Add(InputDeviceId);
	DeviceOwners.Add(InputDeviceId, LocalPlayer);
}
ULocalPlayer* ULocalPlayerSessionDirectory::RemoveInputDevice(const FInputDeviceId InputDeviceId)
{
	TWeakObjectPtr<ULocalPlayer> Owner;
	if (!DeviceOwners.RemoveAndCopyValue(InputDeviceId, Owner)) return nullptr;
	if (!Owner.IsValid()) return nullptr;
	if (FLocalPlayerSessionRecord* Record = Players.Find(Owner)) Record->InputDevices.Remove(InputDeviceId);
	return Owner.Get();
}
void ULocalPlayerSessionDirectory::SetProfile(ULocalPlayer* LocalPlayer, const FGuid& ProfileId)
{
	if (!IsValid(LocalPlayer)) return;
	RegisterPlayer(LocalPlayer);
	Players.FindChecked(TWeakObjectPtr<ULocalPlayer>(LocalPlayer)).ProfileId = ProfileId;
}

void ULocalPlayerSessionDirectory::SetFocusedWidget(ULocalPlayer* LocalPlayer, UWidget* Widget)
{
	if (!IsValid(LocalPlayer)) return;
	RegisterPlayer(LocalPlayer);
	Players.FindChecked(TWeakObjectPtr<ULocalPlayer>(LocalPlayer)).FocusedWidget = Widget;
}
void ULocalPlayerSessionDirectory::FocusWidget(ULocalPlayer* LocalPlayer, UWidget* Widget)
{
	if (!IsValid(LocalPlayer) || !IsValid(Widget)) return;
	RegisterPlayer(LocalPlayer);
	FLocalPlayerSessionRecord& Record = Players.FindChecked(TWeakObjectPtr<ULocalPlayer>(LocalPlayer));
	if (!IsValid(Record.PlayerController.Get())) Record.PlayerController = LocalPlayer->GetPlayerController(GetWorld());
	if (APlayerController* PC = Record.PlayerController.Get())
	{
		Widget->SetUserFocus(PC);
		Record.FocusedWidget = Widget;
	}
}

const FLocalPlayerSessionRecord* ULocalPlayerSessionDirectory::FindPlayer(const ULocalPlayer* LocalPlayer) const
{
	return IsValid(LocalPlayer) ? Players.Find(TWeakObjectPtr<ULocalPlayer>(const_cast<ULocalPlayer*>(LocalPlayer))) : nullptr;
}
const FLocalPlayerSessionRecord* ULocalPlayerSessionDirectory::FindPlayerForDevice(FInputDeviceId InputDeviceId) const
{
	const TWeakObjectPtr<ULocalPlayer>* Owner = DeviceOwners.Find(InputDeviceId);
	return Owner && Owner->IsValid() ? FindPlayer(Owner->Get()) : nullptr;
}

FGuid ULocalPlayerSessionDirectory::GetProfileId(const ULocalPlayer* LocalPlayer) const
{
	const FLocalPlayerSessionRecord* Record = FindPlayer(LocalPlayer);
	return Record ? Record->ProfileId : FGuid();
}
bool ULocalPlayerSessionDirectory::IsProfileAssigned(const FGuid& ProfileId, const ULocalPlayer* ExceptPlayer) const
{
	if (!ProfileId.IsValid()) return false;
	for (const TPair<TWeakObjectPtr<ULocalPlayer>, FLocalPlayerSessionRecord>& Pair : Players)
	{
		if (Pair.Key.IsValid() && Pair.Key.Get() != ExceptPlayer && Pair.Value.ProfileId == ProfileId) return true;
	}
	return false;
}
