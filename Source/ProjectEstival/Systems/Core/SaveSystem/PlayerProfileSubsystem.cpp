#include "PlayerProfileSubsystem.h"

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/IInputProcessor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SaveGame.h"
#include "PlayerProfileSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"
#include "InputCoreTypes.h"

// Input handling
class FPlayerJoinInputProcessor final : public IInputProcessor
{
public:
    explicit FPlayerJoinInputProcessor(UPlayerProfileSubsystem* InSubsystem) : Subsystem(InSubsystem) { }
    virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }
    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InEvent) override
    {
        const FKey Key = InEvent.GetKey();
        const bool bIsJoinButton = Key == EKeys::Gamepad_FaceButton_Bottom;
        const bool bIsCancelButton = Key == EKeys::Gamepad_FaceButton_Right;
        if (!bIsJoinButton && !bIsCancelButton) return true;
        
        // Ignore key-repeat events
        if (InEvent.IsRepeat()) return true;
        
        if (UPlayerProfileSubsystem* PinnedSubsystem = Subsystem.Get())
        {
            if (bIsCancelButton) return PinnedSubsystem->HandleCancelInput(InEvent.GetInputDeviceId());
            return PinnedSubsystem->HandlePreprocessorKeyDown(InEvent);
        }
        return false;
    }
private:
    TWeakObjectPtr<UPlayerProfileSubsystem> Subsystem;
};
bool UPlayerProfileSubsystem::HandlePreprocessorKeyDown(const FKeyEvent& KeyEvent)
{
    const FInputDeviceId DeviceId = KeyEvent.GetInputDeviceId();
    if (!DeviceId.IsValid()) return false;
    
    UGameInstance* GI = GetGameInstance();
    if (!GI) return false;
    
    // Let the active player's controller input operate its popup
    if (IsValid(CurrentOnboardingPlayer))
    {
        ULocalPlayer* InputPlayer = GEngine ? GEngine->GetLocalPlayerFromInputDevice(GetWorld(), DeviceId) : nullptr;
        if (InputPlayer == CurrentOnboardingPlayer) return false;
    }
    
    // Device not controlling the active popup; handle it as a join request
    HandleJoinInput(DeviceId);
    return true;
}
void UPlayerProfileSubsystem::HandleJoinInput(FInputDeviceId InputDeviceId)
{
    UGameInstance* GI = GetGameInstance();
    if (!GI || !InputDeviceId.IsValid()) return;
    
    // Case 1: This device already belongs to a local player
    // If that player hasn't selected a profile, start onboarding
    ULocalPlayer* ExistingPlayer = GEngine ? GEngine->GetLocalPlayerFromInputDevice(GetWorld(), InputDeviceId) : nullptr;
    if (IsValid(ExistingPlayer))
    {
        if (!SelectedProfiles.Contains(ExistingPlayer)) EnqueuePlayer(ExistingPlayer);
        return;
    }
    
    // Case 2: No local player is associated with this device
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

    // Another device may already have created a local player for this platform user
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
    
    TemporaryPlayers.Add(NewPlayer);
    EnqueuePlayer(NewPlayer);
}
bool UPlayerProfileSubsystem::HandleCancelInput(FInputDeviceId InputDeviceId) const
{
    if (!InputDeviceId.IsValid()) return false;
    if (!IsValid(CurrentOnboardingPlayer)) return false;
    
    // Only the player who owns the active popup can cancel it
    ULocalPlayer* InputPlayer = GEngine ? GEngine->GetLocalPlayerFromInputDevice(GetWorld(), InputDeviceId) : nullptr;
    if (InputPlayer != CurrentOnboardingPlayer) return true;
    
    // Ask the HUD to close the popup, canceling the onboarding for this player
    OnProfileSelectionCancelRequested.Broadcast(CurrentOnboardingPlayer);
    return true;
}

// Init / cleanup
void UPlayerProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    RefreshSavedProfiles();
    
    if (!FSlateApplication::IsInitialized()) return;
    InputProcessor = MakeShared<FPlayerJoinInputProcessor>(this);
    FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
}
void UPlayerProfileSubsystem::Deinitialize()
{
    if (FSlateApplication::IsInitialized() && InputProcessor.IsValid()) FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
    InputProcessor.Reset();
    
    PendingPlayers.Reset();
    TemporaryPlayers.Reset();
    SelectedProfiles.Reset();
    LoadedProfiles.Reset();
    SavedProfiles.Reset();
    CurrentOnboardingPlayer = nullptr;
    
    Super::Deinitialize();
}

// Selection queue
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
void UPlayerProfileSubsystem::ContinueOnboarding()
{
    ShowNextOnboardingPlayer();
}

// Selection logic
bool UPlayerProfileSubsystem::ConfirmProfileSelection(ULocalPlayer* LocalPlayer, const FGuid& ProfileId)
{
    if (!IsValid(LocalPlayer) || CurrentOnboardingPlayer != LocalPlayer || !IsPlayerRegistered(LocalPlayer)) return false;
    if (!ProfileId.IsValid()) return false;
    
    // Reject a profile already assigned to another local player
    if (IsProfileTaken(ProfileId))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Profiles] Profile %s is already assigned"), *ProfileId.ToString());
        return false;
    }
    
    // Fetch loaded profile
    if (const UPlayerProfileSaveGame* ExistingLoadedSave = GetLoadedProfile(ProfileId); !ExistingLoadedSave || ExistingLoadedSave->Profile.ProfileId != ProfileId)
    {
        if (!LoadProfile(ProfileId))
        {
            UE_LOG(LogTemp, Warning, TEXT("[Profiles] Could not load profile %s"), *ProfileId.ToString());
            return false;
        }
    }
    
    // Broadcast assignment event
    if (!AssignProfile(LocalPlayer, ProfileId))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Profiles] Could not assign profile %s"), *ProfileId.ToString());
        return false;
    }
    OnProfileAssigned.Broadcast(LocalPlayer, ProfileId);
    
    CurrentOnboardingPlayer = nullptr;
    ShowNextOnboardingPlayer();
    return true;
}
void UPlayerProfileSubsystem::CancelProfileSelection(ULocalPlayer* LocalPlayer)
{
    if (!IsValid(LocalPlayer) || CurrentOnboardingPlayer != LocalPlayer) return;
    
    UGameInstance* GI = GetGameInstance();
    if (!GI) return;
    
    // Remove duplicate queue entries for this player
    PendingPlayers.RemoveAll([LocalPlayer](const TObjectPtr<ULocalPlayer>& Player) { return Player.Get() == LocalPlayer; });
    ReleaseProfile(LocalPlayer);
    CurrentOnboardingPlayer = nullptr;
    
    // Only remove local players created provisionally by this subsystem's join flow
    if (TemporaryPlayers.Remove(LocalPlayer) > 0 && IsPlayerRegistered(LocalPlayer)) GI->RemoveLocalPlayer(LocalPlayer);
    ShowNextOnboardingPlayer();
}

// Profile claiming
void UPlayerProfileSubsystem::RefreshSavedProfiles()
{
    SavedProfiles.Reset();
    
    ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
    if (!SaveSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("[Profiles] No save game system available"));
        return;
    }
    
    TArray<FString> SlotNames;
    SaveSystem->GetSaveGameNames(SlotNames, 0);
    
    TSet<FGuid> SeenProfileIds;
    
    for (const FString& SlotName : SlotNames)
    {
        if (!SlotName.StartsWith(TEXT("Profile_"))) continue;
        
        UPlayerProfileSaveGame* Save = Cast<UPlayerProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
        if (!Save)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Profiles] Ignoring invalid save slot: %s"), *SlotName);
            continue;
        }
        
        const FGuid ProfileId = Save->Profile.ProfileId;
        if (!ProfileId.IsValid()) continue;
        
        // The filename must match the profile ID stored inside it
        if (GetProfileSlotName(ProfileId) != SlotName)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Profiles] Slot/profile ID mismatch: %s"), *SlotName);
            continue;
        }
        
        // Avoid duplicate profile entries if the backend returns multiple slots containing the same ID
        if (SeenProfileIds.Contains(ProfileId)) continue;
        
        SeenProfileIds.Add(ProfileId);
        
        FProfileSummary Summary;
        Summary.ProfileId = ProfileId;
        Summary.ProfileName = Save->Profile.ProfileName;
        
        SavedProfiles.Add(MoveTemp(Summary));
    }
    
    SavedProfiles.Sort([](const FProfileSummary& A, const FProfileSummary& B) { return A.ProfileName.Compare(B.ProfileName, ESearchCase::IgnoreCase) < 0; });
}
TArray<FProfileSummary> UPlayerProfileSubsystem::GetUnassignedProfiles() const
{
    TArray<FProfileSummary> UnassignedProfiles = SavedProfiles;
    UnassignedProfiles.RemoveAll([this](const FProfileSummary& Profile) { return IsProfileTaken(Profile.ProfileId); });
    return UnassignedProfiles;
}
bool UPlayerProfileSubsystem::IsProfileTaken(const FGuid& ProfileId) const
{
    if (!ProfileId.IsValid()) return false;
    
    for (const auto& Pair : SelectedProfiles)
    {
        if (Pair.Value == ProfileId && IsValid(Pair.Key)) return true;
    }
    return false;
}
bool UPlayerProfileSubsystem::IsPlayerRegistered(ULocalPlayer* LocalPlayer) const
{
    const UGameInstance* GI = GetGameInstance();
    return IsValid(LocalPlayer) && GI && GI->GetLocalPlayers().Contains(LocalPlayer);
}

// Profile management
FGuid UPlayerProfileSubsystem::CreateProfile(const FString& ProfileName)
{
    const FString CleanName = ProfileName.TrimStartAndEnd();
    if (CleanName.IsEmpty() || !ProfileSaveGameClass) return FGuid();
    
    RefreshSavedProfiles();
    
    // Name already exists
    if (SavedProfiles.ContainsByPredicate([&CleanName](const FProfileSummary& Profile) { return Profile.ProfileName.Equals(CleanName, ESearchCase::IgnoreCase); })) return FGuid();
    
    const FGuid NewId = FGuid::NewGuid();
    UPlayerProfileSaveGame* NewSave = Cast<UPlayerProfileSaveGame>(UGameplayStatics::CreateSaveGameObject(ProfileSaveGameClass));
    if (!NewSave) return FGuid();
    
    NewSave->Profile.ProfileId = NewId;
    NewSave->Profile.ProfileName = CleanName;
    
    const FString SlotName = GetProfileSlotName(NewId);
    if (!UGameplayStatics::SaveGameToSlot(NewSave, SlotName, 0)) return FGuid();
    
    LoadedProfiles.Add(NewId, NewSave);
    RefreshSavedProfiles();
    return NewId;
}
bool UPlayerProfileSubsystem::AssignProfile(ULocalPlayer* LocalPlayer, const FGuid& ProfileId)
{
    if (!IsValid(LocalPlayer) || !IsPlayerRegistered(LocalPlayer) || !ProfileId.IsValid()) return false;
    
    const bool bProfileExists = SavedProfiles.ContainsByPredicate([&ProfileId](const FProfileSummary& Summary) { return Summary.ProfileId == ProfileId; });
    if (!bProfileExists) return false;
    
    // A profile can belong to only one local player in this session
    for (const auto& Pair : SelectedProfiles)
    {
        if (Pair.Key != LocalPlayer && IsValid(Pair.Key) && Pair.Value == ProfileId) return false;
    }
    
    // Assignment requires the profile's save-data to be in memory
    if (!GetLoadedProfile(ProfileId)) return false;
    
    SelectedProfiles.Add(LocalPlayer, ProfileId);
    return true;
}
void UPlayerProfileSubsystem::ReleaseProfile( ULocalPlayer* LocalPlayer)
{
    if (!IsValid(LocalPlayer)) return;
    SelectedProfiles.Remove(LocalPlayer);
}

// Saving / loading
bool UPlayerProfileSubsystem::LoadProfile(const FGuid& ProfileId)
{
    if (!ProfileId.IsValid()) return false;
    
    RefreshSavedProfiles();
    
    const bool bProfileExists = SavedProfiles.ContainsByPredicate([&ProfileId](const FProfileSummary& Summary) { return Summary.ProfileId == ProfileId; });
    if (!bProfileExists) return false;
    
    const FString SlotName = GetProfileSlotName(ProfileId);
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0)) return false;
    
    UPlayerProfileSaveGame* Save = Cast<UPlayerProfileSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (!Save || Save->Profile.ProfileId != ProfileId)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Profiles] Failed to load profile %s"), *ProfileId.ToString());
        return false;
    }
    
    LoadedProfiles.Add(ProfileId, Save);
    return true;
}
bool UPlayerProfileSubsystem::SaveProfile(const FGuid& ProfileId) const
{
    if (!ProfileId.IsValid()) return false;
    
    UPlayerProfileSaveGame* Save = GetLoadedProfile(ProfileId);
    if (!Save || Save->Profile.ProfileId != ProfileId)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Profiles] Cannot save unloaded or mismatched profile %s"), *ProfileId.ToString());
        return false;
    }
    
    const FString SlotName = GetProfileSlotName(ProfileId);
    if (!UGameplayStatics::SaveGameToSlot(Save, SlotName, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("[Profiles] Failed to save profile %s"), *ProfileId.ToString());
        return false;
    }
    
    return true;
}
UPlayerProfileSaveGame* UPlayerProfileSubsystem::GetLoadedProfile(const FGuid& ProfileId) const
{
    const TObjectPtr<UPlayerProfileSaveGame>* Found = LoadedProfiles.Find(ProfileId);
    return Found ? Found->Get() : nullptr;
}
FString UPlayerProfileSubsystem::GetProfileSlotName(const FGuid& ProfileId) const
{
    return FString::Printf(TEXT("%s%s"), *SavePrefix, *ProfileId.ToString(EGuidFormats::Digits));
}
