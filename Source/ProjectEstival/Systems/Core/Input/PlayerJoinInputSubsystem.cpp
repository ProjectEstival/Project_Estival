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

class FPlayerJoinInputProcessor final : public IInputProcessor
{
public:
	explicit FPlayerJoinInputProcessor(UPlayerJoinInputSubsystem* InOwner) : Owner(InOwner) { }
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override
	{
		if (Event.IsRepeat()) return false;
		const FKey Key = Event.GetKey();
		if (Key != EKeys::Gamepad_FaceButton_Bottom && Key != EKeys::Enter && Key != EKeys::Gamepad_FaceButton_Right && Key != EKeys::Escape) return false;
		UPlayerJoinInputSubsystem* PinnedOwner = Owner.Get();
		return IsValid(PinnedOwner) && PinnedOwner->HandleKeyDown(Event);
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

void UPlayerJoinInputSubsystem::HandleDeviceConnectionChange(EInputDeviceConnectionState ConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId) const
{
	if (!bInputEnabled) return;
	UGameInstance* GI = GetGameInstance();
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
	UGameInstance* GI = GetGameInstance();
	if (!GI) return false;
	UPlayerProfileSubsystem* Profiles = GI->GetSubsystem<UPlayerProfileSubsystem>();
	if (!IsValid(Profiles)) return false;
	const FKey Key = Event.GetKey();
	const bool bIsKeyboardDevice = Event.GetInputDeviceId() == IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
	if (bIsKeyboardDevice && Key == EKeys::Escape && Profiles->HandleForeignKeyboardNameCancel()) return true;
	
	// Let Slate's editable text process keyboard Enter and emit its normal OnTextCommitted event
	if (bIsKeyboardDevice && Key == EKeys::Enter && Profiles->IsForeignKeyboardNameEntryActive()) return false;
	if (Key == EKeys::Gamepad_FaceButton_Bottom && Profiles->TryCommitProfileNameFromDevice(Event.GetInputDeviceId())) return true;
	if (Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Escape) return Profiles->HandleCancelInput(Event.GetInputDeviceId());
	return Profiles->HandlePreprocessorJoinKeyDown(Event);
}
bool UPlayerJoinInputSubsystem::JoinDevice(FInputDeviceId InputDeviceId) const
{
	UGameInstance* GI = GetGameInstance();
	if (!bInputEnabled || !GI || !InputDeviceId.IsValid()) return false;
	UPlayerProfileSubsystem* Profiles = GI->GetSubsystem<UPlayerProfileSubsystem>();
	if (!IsValid(Profiles)) return false;
	if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>())
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
