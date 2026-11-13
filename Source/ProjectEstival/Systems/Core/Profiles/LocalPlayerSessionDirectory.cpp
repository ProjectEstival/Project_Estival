#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"

#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Components/Widget.h"

ULocalPlayerSessionDirectory::ULocalPlayerSessionDirectory()
{
	PlayerColorSlotOrder = { 0, 1, 2, 3 };
	PlayerSlotColors.Add(0, FLinearColor(0.05f, 0.25f, 1.0f));
	PlayerSlotColors.Add(1, FLinearColor(1.0f, 0.05f, 0.05f));
	PlayerSlotColors.Add(2, FLinearColor(0.05f, 0.8f, 0.12f));
	PlayerSlotColors.Add(3, FLinearColor(1.0f, 0.78f, 0.02f));
}

void ULocalPlayerSessionDirectory::RegisterPlayer(ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer)) return;
	const TWeakObjectPtr<ULocalPlayer> PlayerKey(LocalPlayer);
	FLocalPlayerSessionRecord& Record = Players.FindOrAdd(PlayerKey);
	Record.LocalPlayer = LocalPlayer;
	Record.PlatformUserId = LocalPlayer->GetPlatformUserId();
	Record.SlateUser = LocalPlayer->GetSlateUser();
	Record.PlayerController = LocalPlayer->GetPlayerController(GetWorld());
	if (Record.PlayerColorSlot == INDEX_NONE) Record.PlayerColorSlot = FindAvailableColorSlot();
	ApplyPlayerLightColor(LocalPlayer);
}
void ULocalPlayerSessionDirectory::UnregisterPlayer(ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer)) return;
	if (const FLocalPlayerSessionRecord* Record = FindPlayer(LocalPlayer))
	{
		if (APlayerController* PC = Record->PlayerController.Get()) PC->ResetControllerLightColor();
	}
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
const FLocalPlayerSessionRecord* ULocalPlayerSessionDirectory::FindPlayerForDevice(const FInputDeviceId InputDeviceId) const
{
	const TWeakObjectPtr<ULocalPlayer>* Owner = DeviceOwners.Find(InputDeviceId);
	return Owner && Owner->IsValid() ? FindPlayer(Owner->Get()) : nullptr;
}

FGuid ULocalPlayerSessionDirectory::GetProfileId(const ULocalPlayer* LocalPlayer) const
{
	const FLocalPlayerSessionRecord* Record = FindPlayer(LocalPlayer);
	return Record ? Record->ProfileId : FGuid();
}
int32 ULocalPlayerSessionDirectory::GetPlayerColorSlot(const ULocalPlayer* LocalPlayer) const
{
	const FLocalPlayerSessionRecord* Record = FindPlayer(LocalPlayer);
	return Record ? Record->PlayerColorSlot : INDEX_NONE;
}

FLinearColor ULocalPlayerSessionDirectory::GetPlayerColor(const ULocalPlayer* LocalPlayer) const
{
	const int32 Slot = GetPlayerColorSlot(LocalPlayer);
	if (const FLinearColor* Color = PlayerSlotColors.Find(Slot)) return *Color;
	return FLinearColor::White;
}
void ULocalPlayerSessionDirectory::SetPlayerSlotColor(const int32 Slot, const FLinearColor Color)
{
	if (Slot == INDEX_NONE) return;
	PlayerSlotColors.Add(Slot, Color);
	if (!PlayerColorSlotOrder.Contains(Slot)) PlayerColorSlotOrder.Add(Slot);
	SaveConfig();
	for (const TPair<TWeakObjectPtr<ULocalPlayer>, FLocalPlayerSessionRecord>& Entry : Players)
	{
		if (Entry.Key.IsValid() && Entry.Value.PlayerColorSlot == Slot) ApplyPlayerLightColor(Entry.Key.Get());
	}
}
void ULocalPlayerSessionDirectory::ApplyPlayerLightColor(const ULocalPlayer* LocalPlayer) const
{
	// I can't get Unreal to see my DualShock controllers to test this, but if we ever set up the dlls from Sony, this should work(?)
	const FLocalPlayerSessionRecord* Record = FindPlayer(LocalPlayer);
	APlayerController* PC = Record ? Record->PlayerController.Get() : nullptr;
	if (!IsValid(PC)) PC = LocalPlayer ? LocalPlayer->GetPlayerController(GetWorld()) : nullptr;
	if (IsValid(PC)) PC->SetControllerLightColor(GetPlayerColor(LocalPlayer).ToFColor(true));
}

int32 ULocalPlayerSessionDirectory::FindAvailableColorSlot() const
{
	auto IsSlotInUse = [this](const int32 Slot)
	{
		for (const TPair<TWeakObjectPtr<ULocalPlayer>, FLocalPlayerSessionRecord>& Entry : Players)
		{
			if (Entry.Key.IsValid() && Entry.Value.PlayerColorSlot == Slot) return true;
		}
		return false;
	};
	
	for (const int32 CandidateSlot : PlayerColorSlotOrder)
	{
		if (!IsSlotInUse(CandidateSlot)) return CandidateSlot;
	}
	
	int32 OverflowSlot = 0;
	for (const int32 CandidateSlot : PlayerColorSlotOrder) OverflowSlot = FMath::Max(OverflowSlot, CandidateSlot + 1);
	while (IsSlotInUse(OverflowSlot)) ++OverflowSlot;
	return OverflowSlot;
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
