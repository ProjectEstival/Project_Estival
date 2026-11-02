#include "PlayerProfileSubsystem.h"

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/IInputProcessor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"

class FPlayerJoinInputProcessor final : public IInputProcessor
{
public:
    explicit FPlayerJoinInputProcessor(UPlayerProfileSubsystem* InSubsystem) : Subsystem(InSubsystem) { }
    virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }
    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp,
    const FKeyEvent& InEvent) override
    {
        if (InEvent.GetKey() != EKeys::Gamepad_FaceButton_Bottom) return false;
        
        // Don't process key-repeat events as new join requests.
        if (InEvent.IsRepeat()) return true;
        
        if (UPlayerProfileSubsystem* PinnedSubsystem = Subsystem.Get()) return PinnedSubsystem->HandlePreprocessorKeyDown(InEvent);
        return false;
    }

private:
    TWeakObjectPtr<UPlayerProfileSubsystem> Subsystem;
};


void UPlayerProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    if (!FSlateApplication::IsInitialized()) return;
    InputProcessor = MakeShared<FPlayerJoinInputProcessor>(this);
    FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
}
void UPlayerProfileSubsystem::Deinitialize()
{
    if (FSlateApplication::IsInitialized() && InputProcessor.IsValid()) FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
    InputProcessor.Reset();
    
    Super::Deinitialize();
}
bool UPlayerProfileSubsystem::HandlePreprocessorKeyDown(const FKeyEvent& KeyEvent)
{
    const FInputDeviceId DeviceId = KeyEvent.GetInputDeviceId();
    if (!DeviceId.IsValid()) return false;
    
    UGameInstance* GI = GetGameInstance();
    if (!GI) return false;
    
    // Let the active player's controller input operate its popup.
    if (IsValid(CurrentOnboardingPlayer))
    {
        ULocalPlayer* InputPlayer = GEngine ? GEngine->GetLocalPlayerFromInputDevice(GetWorld(), DeviceId) : nullptr;
        if (InputPlayer == CurrentOnboardingPlayer) return false;
    }
    
    // This device is not controlling the active onboarding popup. Handle it as a join request.
    HandleJoinInput(DeviceId);
    return true;
}

void UPlayerProfileSubsystem::HandleJoinInput(FInputDeviceId InputDeviceId)
{
    UGameInstance* GI = GetGameInstance();
    if (!GI || !InputDeviceId.IsValid()) return;
    
    // Case 1: This device already belongs to a local player.
    // If that player hasn't selected a profile, start onboarding.
    ULocalPlayer* ExistingPlayer = GEngine ? GEngine->GetLocalPlayerFromInputDevice(GetWorld(), InputDeviceId) : nullptr;
    if (IsValid(ExistingPlayer))
    {
        if (!SelectedProfiles.Contains(ExistingPlayer)) EnqueuePlayer(ExistingPlayer);
        return;
    }
    
    // Case 2: No local player is associated with this device.
    IPlatformInputDeviceMapper& Mapper = IPlatformInputDeviceMapper::Get();
    FPlatformUserId PlatformUserId = Mapper.GetUserForInputDevice(InputDeviceId);
    
    const FPlatformUserId UnpairedUserId = Mapper.GetUserForUnpairedInputDevices();
    
    if (!PlatformUserId.IsValid() || PlatformUserId == UnpairedUserId)
    {
        const FPlatformUserId OldUserId = PlatformUserId;
        PlatformUserId = Mapper.GetFirstPlatformUserWithNoInputDevice();
        
        if (!PlatformUserId.IsValid()) PlatformUserId = Mapper.AllocateNewUserId();
        if (!PlatformUserId.IsValid()) return;
        
        if (!Mapper.Internal_ChangeInputDeviceUserMapping(InputDeviceId, PlatformUserId, OldUserId)) return;
    }

    // Another device may already have created a local player for this platform user.
    if (ULocalPlayer* UserPlayer = GI->FindLocalPlayerFromPlatformUserId(PlatformUserId))
    {
        if (!SelectedProfiles.Contains(UserPlayer)) EnqueuePlayer(UserPlayer);
        return;
    }

    if (GI->GetNumLocalPlayers() >= MaxPlayers)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Profiles] Maximum local player count reached"));
        return;
    }
    
    FString Error;
    ULocalPlayer* NewPlayer = GI->CreateLocalPlayer(PlatformUserId, Error, true);
    if (!IsValid(NewPlayer))
    {
        UE_LOG(LogTemp, Error,TEXT("[Profiles] CreateLocalPlayer failed: %s"), *Error);
        return;
    }
    
    JoinCreatedPlayers.Add(NewPlayer);
    EnqueuePlayer(NewPlayer);
}
void UPlayerProfileSubsystem::RequestPlayerJoin(FPlatformUserId PlatformUserId)
{
    UGameInstance* GI = GetGameInstance();
    if (!GI || !PlatformUserId.IsValid()) return;
    
    // This platform user already has a local player.
    if (GI->FindLocalPlayerFromPlatformUserId(PlatformUserId)) return;
    if (GI->GetNumLocalPlayers() >= MaxPlayers) return;
    
    FString Error;
    ULocalPlayer* NewPlayer = GI->CreateLocalPlayer(PlatformUserId, Error, true);
    
    if (!IsValid(NewPlayer))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create local player: %s"), *Error);
        return;
    }
    
    JoinCreatedPlayers.Add(NewPlayer);
    EnqueuePlayer(NewPlayer);
}
void UPlayerProfileSubsystem::EnqueuePlayer(ULocalPlayer* LocalPlayer)
{
    if (!IsValid(LocalPlayer)) return;
    if (!IsPlayerRegistered(LocalPlayer)) return;
    if (SelectedProfiles.Contains(LocalPlayer)) return;
    if (PendingPlayers.Contains(LocalPlayer)) return;
    if (CurrentOnboardingPlayer == LocalPlayer) return;
    PendingPlayers.Add(LocalPlayer);
    ShowNextOnboardingPlayer();
}
void UPlayerProfileSubsystem::ShowNextOnboardingPlayer()
{
    if (IsValid(CurrentOnboardingPlayer)) return;
    while (!PendingPlayers.IsEmpty())
    {
        ULocalPlayer* NextPlayer = PendingPlayers[0];
        PendingPlayers.RemoveAt(0);
        if (!IsValid(NextPlayer)) continue;
        
        CurrentOnboardingPlayer = NextPlayer;
        OnOnboardingPlayerChanged.Broadcast(NextPlayer);
        return;
    }
}

void UPlayerProfileSubsystem::ConfirmProfileSelection(ULocalPlayer* LocalPlayer, int32 ProfileIndex)
{
    if (!IsValid(LocalPlayer)) return;
    if (CurrentOnboardingPlayer != LocalPlayer) return;
    if (ProfileIndex < 0) return;
    
    // TODO: Check that profile isn't already assigned
    
    SelectedProfiles.Add(LocalPlayer, ProfileIndex);
    CurrentOnboardingPlayer = nullptr;
    
    ShowNextOnboardingPlayer();
}
void UPlayerProfileSubsystem::CancelProfileSelection(ULocalPlayer* LocalPlayer)
{
    if (!IsValid(LocalPlayer)) return;
    if (CurrentOnboardingPlayer != LocalPlayer) return; // Only the active player can cancel this onboarding session.
    
    UGameInstance* GI = GetGameInstance();
    if (!GI) return;
    
    // Remove any duplicate queue entries.
    PendingPlayers.RemoveAll([LocalPlayer](const ULocalPlayer* Player) { return Player == LocalPlayer; });
    SelectedProfiles.Remove(LocalPlayer);
    
    // Clear the active player before removing the local player.
    CurrentOnboardingPlayer = nullptr;
    
    // Only remove players created provisionally by our join flow.
    const bool bWasJoinCreated = JoinCreatedPlayers.Remove(LocalPlayer) > 0;
    if (bWasJoinCreated && IsPlayerRegistered(LocalPlayer)) GI->RemoveLocalPlayer(LocalPlayer);
    ShowNextOnboardingPlayer();
}

bool UPlayerProfileSubsystem::IsPlayerRegistered(ULocalPlayer* LocalPlayer) const
{
    return IsValid(LocalPlayer) && GetGameInstance()->GetLocalPlayers().Contains(LocalPlayer);
}