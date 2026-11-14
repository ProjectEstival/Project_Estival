#include "Systems/Core/Input/PlayerJoinInputSubsystem.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/IInputProcessor.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
#include "UI/HUDBase.h"

class FPlayerJoinInputProcessor final : public IInputProcessor
{
public:
	explicit FPlayerJoinInputProcessor(UPlayerJoinInputSubsystem* InOwner) : Owner(InOwner) { }
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override
	{
		const UPlayerJoinInputSubsystem* PinnedOwner = Owner.Get();
		if (!IsValid(PinnedOwner)) return false;
		if (PinnedOwner->ShouldBlockInputForModal(Event.GetInputDeviceId()))
		{
			if (const bool bIsKeyboardDevice = Event.GetInputDeviceId() == IPlatformInputDeviceMapper::Get().GetDefaultInputDevice())
			{
				if (const UGameInstance* GI = PinnedOwner->GetGameInstance())
				{
					if (const UPlayerProfileSubsystem* Profiles = GI->GetSubsystem<UPlayerProfileSubsystem>())
					{
						if (Event.GetKey() == EKeys::Escape && Profiles->HandleForeignKeyboardNameCancel()) return true;
						// Let Slate handle arrows and other keys for the keyboard-focused editable field in this modal
						if (Profiles->IsForeignKeyboardNameEntryActive()) return false;
					}
				}
			}
			if (!Event.IsRepeat() && Event.GetKey() == EKeys::Gamepad_FaceButton_Bottom) PinnedOwner->HandleJoinKeyForBlockedModal(Event.GetInputDeviceId());
			return true;
		}
		if (Event.IsRepeat()) return false;
		if (const FKey Key = Event.GetKey(); Key != EKeys::Gamepad_FaceButton_Bottom && Key != EKeys::Enter && Key != EKeys::Gamepad_FaceButton_Right && Key != EKeys::Escape) return false;
		return IsValid(PinnedOwner) && PinnedOwner->HandleKeyDown(Event);
	}
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override
	{
		const UPlayerJoinInputSubsystem* PinnedOwner = Owner.Get();
		if (IsValid(PinnedOwner) && Event.GetInputDeviceId() == IPlatformInputDeviceMapper::Get().GetDefaultInputDevice())
		{
			if (const UGameInstance* GI = PinnedOwner->GetGameInstance())
			{
				if (const UPlayerProfileSubsystem* Profiles = GI->GetSubsystem<UPlayerProfileSubsystem>(); Profiles && Profiles->IsForeignKeyboardNameEntryActive()) return false;
			}
		}
		return IsValid(PinnedOwner) && PinnedOwner->ShouldBlockInputForModal(Event.GetInputDeviceId());
	}
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& Event) override
	{
		const UPlayerJoinInputSubsystem* PinnedOwner = Owner.Get();
		return IsValid(PinnedOwner) && PinnedOwner->ShouldBlockInputForModal(Event.GetInputDeviceId());
	}
private:
	TWeakObjectPtr<UPlayerJoinInputSubsystem> Owner;
};

void UPlayerJoinInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ConnectionChangeHandle = IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().AddUObject(this, &UPlayerJoinInputSubsystem::HandleDeviceConnectionChange);
	if (!FSlateApplication::IsInitialized()) return;
	InputProcessor = MakeShared<FPlayerJoinInputProcessor>(this);
	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
}
void UPlayerJoinInputSubsystem::Deinitialize()
{
	if (ConnectionChangeHandle.IsValid()) IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().Remove(ConnectionChangeHandle);
	ConnectionChangeHandle = FDelegateHandle();
	if (FSlateApplication::IsInitialized() && InputProcessor.IsValid()) FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	InputProcessor.Reset();
	bInputEnabled = false;
	
	Super::Deinitialize();
}

void UPlayerJoinInputSubsystem::SetInputEnabled(const bool bEnabled)
{
	bInputEnabled = bEnabled;
	if (bInputEnabled) AssociateConnectedDevices();
}
void UPlayerJoinInputSubsystem::AssociateConnectedDevices() const
{
	const UGameInstance* GI = GetGameInstance();
	ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	if (!GI || !Directory) return;
	
	for (ULocalPlayer* LocalPlayer : GI->GetLocalPlayers())
	{
		if (!IsValid(LocalPlayer)) continue;
		Directory->RegisterPlayer(LocalPlayer);
		TArray<FInputDeviceId> Devices;
		IPlatformInputDeviceMapper::Get().GetAllConnectedInputDevicesForUser(LocalPlayer->GetPlatformUserId(), Devices);
		for (const FInputDeviceId DeviceId : Devices) Directory->AssociateInputDevice(LocalPlayer, DeviceId);
	}
	
	// Keyboard and mouse share the platform's default input device and belong to player 0
	if (const FInputDeviceId DefaultInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(); DefaultInputDevice.IsValid())
	{
		if (ULocalPlayer* FirstPlayer = GI->GetLocalPlayerByIndex(0)) Directory->AssociateInputDevice(FirstPlayer, DefaultInputDevice);
	}
}
void UPlayerJoinInputSubsystem::HandleDeviceConnectionChange(const EInputDeviceConnectionState ConnectionState, const FPlatformUserId PlatformUserId, const FInputDeviceId InputDeviceId) const
{
	if (!bInputEnabled) return;
	const UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	if (ConnectionState == EInputDeviceConnectionState::Disconnected)
	{
		if (UPlayerProfileSubsystem* Profiles = GI->GetSubsystem<UPlayerProfileSubsystem>()) Profiles->HandleInputDeviceDisconnected(InputDeviceId);
		return;
	}
	if (ConnectionState == EInputDeviceConnectionState::Connected)
	{
		if (ULocalPlayer* Player = GI->FindLocalPlayerFromPlatformUserId(PlatformUserId))
		{
			if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>()) Directory->AssociateInputDevice(Player, InputDeviceId);
		}
	}
}
bool UPlayerJoinInputSubsystem::HandleKeyDown(const FKeyEvent& Event) const
{
	if (!bInputEnabled) return false;
	const UGameInstance* GI = GetGameInstance();
	if (!GI) return false;
	const UPlayerProfileSubsystem* Profiles = GI->GetSubsystem<UPlayerProfileSubsystem>();
	if (!IsValid(Profiles)) return false;
	const FKey Key = Event.GetKey();
	const bool bIsKeyboardDevice = Event.GetInputDeviceId() == IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
	if (bIsKeyboardDevice && Key == EKeys::Escape && Profiles->HandleForeignKeyboardNameCancel()) return true;
	
	// Let Slate's editable text process keyboard Enter and emit its normal OnTextCommitted event
	if (bIsKeyboardDevice && Key == EKeys::Enter && Profiles->IsForeignKeyboardNameEntryActive()) return false;
	if (Key == EKeys::Gamepad_FaceButton_Bottom && Profiles->TryCommitProfileNameFromDevice(Event.GetInputDeviceId())) return true;
	if (Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Escape)
	{
		const ULocalPlayer* InputPlayer = nullptr;
		if (const ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>())
		{
			if (const FLocalPlayerSessionRecord* Record = Directory->FindPlayerForDevice(Event.GetInputDeviceId())) InputPlayer = Record->LocalPlayer.Get();
		}
		if (!IsValid(InputPlayer)) InputPlayer = bIsKeyboardDevice ? GI->GetLocalPlayerByIndex(0) : GI->FindLocalPlayerFromDeviceId(Event.GetInputDeviceId());
		if (APlayerController* InputPlayerController = InputPlayer ? InputPlayer->GetPlayerController(GetWorld()) : nullptr; AHUDBase::HandleBackForPlayer(GetWorld(), InputPlayerController)) return true;
		
		// If player has no modal to close, release the profile they have selected
		return Profiles->HandleCancelInput(Event.GetInputDeviceId());
	}
	return Profiles->HandlePreprocessorJoinKeyDown(Event);
}
bool UPlayerJoinInputSubsystem::ShouldBlockInputForModal(const FInputDeviceId InputDeviceId) const
{
	const APlayerController* ModalOwner = AHUDBase::GetTopmostModalOwner(GetWorld());
	if (!IsValid(ModalOwner)) return false;
	const UGameInstance* GI = GetGameInstance();
	const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	const FLocalPlayerSessionRecord* Record = Directory ? Directory->FindPlayerForDevice(InputDeviceId) : nullptr;
	const ULocalPlayer* InputPlayer = Record ? Record->LocalPlayer.Get() : GI->FindLocalPlayerFromDeviceId(InputDeviceId);
	return InputPlayer != ModalOwner->GetLocalPlayer();
}
void UPlayerJoinInputSubsystem::HandleJoinKeyForBlockedModal(const FInputDeviceId InputDeviceId) const
{
	if (bInputEnabled) JoinDevice(InputDeviceId);
}
bool UPlayerJoinInputSubsystem::JoinDevice(const FInputDeviceId InputDeviceId) const
{
	UGameInstance* GI = GetGameInstance();
	if (!bInputEnabled || !GI || !InputDeviceId.IsValid()) return false;
	UPlayerProfileSubsystem* Profiles = GI->GetSubsystem<UPlayerProfileSubsystem>();
	if (!IsValid(Profiles)) return false;
	if (const ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>())
	{
		if (const FLocalPlayerSessionRecord* Record = Directory->FindPlayerForDevice(InputDeviceId))
		{
			Profiles->HandlePlayerJoined(Record->LocalPlayer.Get(), InputDeviceId, false);
			return true;
		}
	}
	
	if (DeviceJoinPolicy == ELocalDeviceJoinPolicy::AttachToFirstLocalPlayer)
	{
		ULocalPlayer* FirstPlayer = GI->GetLocalPlayerByIndex(0);
		if (!IsValid(FirstPlayer)) return false;
		IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
		const FPlatformUserId CurrentUserId = Mapper.GetUserForInputDevice(InputDeviceId);
		const FPlatformUserId TargetUserId = FirstPlayer->GetPlatformUserId();
		if (!TargetUserId.IsValid()) return false;
		if (CurrentUserId != TargetUserId && !Mapper.Internal_ChangeInputDeviceUserMapping(InputDeviceId, TargetUserId, CurrentUserId)) return false;
		Profiles->HandlePlayerJoined(FirstPlayer, InputDeviceId, false);
		return true;
	}
	
	if (ULocalPlayer* Player = GI->FindLocalPlayerFromDeviceId(InputDeviceId); IsValid(Player))
	{
		Profiles->HandlePlayerJoined(Player, InputDeviceId, false);
		return true;
	}
	
	IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
	FPlatformUserId PlatformUserId = Mapper.GetUserForInputDevice(InputDeviceId);
	const FPlatformUserId UnpairedUserId = Mapper.GetUserForUnpairedInputDevices();
	if (!PlatformUserId.IsValid() || PlatformUserId == UnpairedUserId)
	{
		const FPlatformUserId OldUserId = PlatformUserId;
		PlatformUserId = Mapper.GetFirstPlatformUserWithNoInputDevice();
		if (!PlatformUserId.IsValid()) PlatformUserId = Mapper.AllocateNewUserId();
		if (!PlatformUserId.IsValid() || !Mapper.Internal_ChangeInputDeviceUserMapping(InputDeviceId, PlatformUserId, OldUserId)) return false;
	}
	
	if (ULocalPlayer* ExistingUserPlayer = GI->FindLocalPlayerFromPlatformUserId(PlatformUserId))
	{
		Profiles->HandlePlayerJoined(ExistingUserPlayer, InputDeviceId, false);
		return true;
	}
	if (GI->GetNumLocalPlayers() >= MaxLocalPlayers)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Players] Maximum local player count reached"));
		return true;
	}
	
	FString Error;
	ULocalPlayer* NewPlayer = GI->CreateLocalPlayer(PlatformUserId, Error, true);
	if (!IsValid(NewPlayer))
	{
		UE_LOG(LogTemp, Error, TEXT("[Players] CreateLocalPlayer failed: %s"), *Error);
		return true;
	}
	Profiles->HandlePlayerJoined(NewPlayer, InputDeviceId, true);
	return true;
}
