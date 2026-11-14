#include "PlayerProfileSubsystem.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Systems/Core/Profiles/PlayerProfileRepository.h"
#include "Systems/Core/Profiles/PlayerOnboardingQueue.h"
#include "Systems/Core/Input/PlayerJoinInputSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Components/EditableText.h"
#include "Components/Widget.h"
#include "PlayerProfileSaveGame.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"
#include "UI/HUDBase.h"

// Init / cleanup
void UPlayerProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
	
	OnboardingQueue = NewObject<UPlayerOnboardingQueue>(this);
	OnboardingQueue->OnCurrentPlayerChanged.AddDynamic(this, &UPlayerProfileSubsystem::HandleOnboardingQueuePlayerChanged);
    ProfileRepository = NewObject<UPlayerProfileRepository>(this);
    ProfileRepository->Initialize();
}
void UPlayerProfileSubsystem::Deinitialize()
{
    TemporaryPlayers.Reset();
    ProfileRepository = nullptr;
    if (OnboardingQueue) OnboardingQueue->OnCurrentPlayerChanged.RemoveDynamic(this, &UPlayerProfileSubsystem::HandleOnboardingQueuePlayerChanged);
    OnboardingQueue = nullptr;
	
    Super::Deinitialize();
}

bool UPlayerProfileSubsystem::HandlePreprocessorJoinKeyDown(const FKeyEvent& KeyEvent) const
{
    if (!bJoinInputEnabled) return false;
    const FInputDeviceId DeviceId = KeyEvent.GetInputDeviceId();
    if (!DeviceId.IsValid()) return false;
	
    UGameInstance* GI = GetGameInstance();
    if (!GI) return false;
	
    ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>();
    const FLocalPlayerSessionRecord* DeviceRecord = Directory ? Directory->FindPlayerForDevice(DeviceId) : nullptr;
    ULocalPlayer* InputPlayer = DeviceRecord ? DeviceRecord->LocalPlayer.Get() : (GEngine ? GEngine->GetLocalPlayerFromInputDevice(GetWorld(), DeviceId) : nullptr);

    // Existing players keep using A/Enter for focused UI after onboarding. The join service only consumes the key for devices that still haven't joined yet
    if (const ULocalPlayer* CurrentPlayer = OnboardingQueue ? OnboardingQueue->GetCurrentPlayer() : nullptr; IsValid(CurrentPlayer))
    {
        if (InputPlayer == CurrentPlayer)
        {
            if (Directory) Directory->AssociateInputDevice(InputPlayer, DeviceId);
            return false;
        }

        // A different, already-onboarded player's A press must not interact with either the onboarding popup or UI beneath it
        if (IsValid(InputPlayer) && Directory && Directory->GetProfileId(InputPlayer).IsValid()) return true;
    }
    else if (IsValid(InputPlayer) && Directory && Directory->GetProfileId(InputPlayer).IsValid()) return false;
	
    // A non-owner's join press is routed to the dedicated local-player join service
    if (const UPlayerJoinInputSubsystem* Input = GI->GetSubsystem<UPlayerJoinInputSubsystem>()) return Input->JoinDevice(DeviceId);
    return false;
}
void UPlayerProfileSubsystem::HandlePlayerJoined(ULocalPlayer* LocalPlayer, const FInputDeviceId InputDeviceId, const bool bCreatedForJoin)
{
    if (!IsValid(LocalPlayer)) return;
    if (bCreatedForJoin && !TemporaryPlayers.Contains(LocalPlayer)) TemporaryPlayers.Add(LocalPlayer);
    if (const UGameInstance* GI = GetGameInstance())
    {
        if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>()) Directory->AssociateInputDevice(LocalPlayer, InputDeviceId);
    }
    EnqueuePlayer(LocalPlayer);
}
void UPlayerProfileSubsystem::HandleInputDeviceDisconnected(const FInputDeviceId InputDeviceId)
{
	if (!bJoinInputEnabled || !InputDeviceId.IsValid()) return;
	UGameInstance* GI = GetGameInstance();
	ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	const FLocalPlayerSessionRecord* Record = Directory ? Directory->FindPlayerForDevice(InputDeviceId) : nullptr;
	ULocalPlayer* LocalPlayer = Record ? Record->LocalPlayer.Get() : nullptr;
	if (!IsValid(LocalPlayer)) return;
	const bool bHasOtherInputDevices = Record->InputDevices.Num() > 1;
	const bool bIsFirstLocalPlayer = GI->GetLocalPlayerByIndex(0) == LocalPlayer;
	Directory->RemoveInputDevice(InputDeviceId);
	
	if (!bHasOtherInputDevices)
	{
		// No device can control this local player now, so remove it rather than opening another popup
		OnPlayerDisconnected.Broadcast(LocalPlayer);
		// Clear orphaned modal widgets before destroying the player's controller and HUD
		AHUDBase::ClearPlayerModalWidgets(GI->GetWorld(), LocalPlayer);
		ReleaseProfile(LocalPlayer);
		Directory->UnregisterPlayer(LocalPlayer);
		if (OnboardingQueue) OnboardingQueue->RemovePlayer(LocalPlayer);
		TemporaryPlayers.Remove(LocalPlayer);
		// Keep player 0's HUD alive because it owns the shared base menu widget
		if (!bIsFirstLocalPlayer) GI->RemoveLocalPlayer(LocalPlayer);
	}
}
bool UPlayerProfileSubsystem::HandleCancelInput(const FInputDeviceId InputDeviceId) const
{
    if (!bJoinInputEnabled) return false;
    if (!InputDeviceId.IsValid()) return false;
    ULocalPlayer* CurrentPlayer = OnboardingQueue ? OnboardingQueue->GetCurrentPlayer() : nullptr;
    const UGameInstance* GI = GetGameInstance();
	const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	const FLocalPlayerSessionRecord* DeviceRecord = Directory ? Directory->FindPlayerForDevice(InputDeviceId) : nullptr;
	ULocalPlayer* InputPlayer = DeviceRecord ? DeviceRecord->LocalPlayer.Get() : (GEngine ? GEngine->GetLocalPlayerFromInputDevice(GetWorld(), InputDeviceId) : nullptr);
	if (!IsValid(InputPlayer)) return false;
	
	// Only the player who owns the active popup can cancel or change its state
	if (IsValid(CurrentPlayer) && InputPlayer == CurrentPlayer)
	{
		if (ULocalPlayerSessionDirectory* MutableDirectory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr) MutableDirectory->AssociateInputDevice(InputPlayer, InputDeviceId);
		OnProfileSelectionCancelRequested.Broadcast(CurrentPlayer);
		return true;
	}
	if (IsValid(CurrentPlayer)) return true;
	
	// A back press outside onboarding releases this device's current profile
	if (!Directory || !Directory->GetProfileId(InputPlayer).IsValid()) return false;
	ReleaseProfile(InputPlayer);
	if (IsMandatoryProfilePlayer(InputPlayer)) RequireProfileForFirstPlayer(InputPlayer);
	else EnqueuePlayer(InputPlayer);
	if (ULocalPlayerSessionDirectory* MutableDirectory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr) MutableDirectory->AssociateInputDevice(InputPlayer, InputDeviceId);
	return true;
}
bool UPlayerProfileSubsystem::IsForeignKeyboardNameEntryActive() const
{
    if (!bJoinInputEnabled || !OnboardingQueue) return false;
    const ULocalPlayer* TargetPlayer = OnboardingQueue->GetCurrentPlayer();
    if (const ULocalPlayer* KeyboardPlayer = GetGameInstance() ? GetGameInstance()->GetLocalPlayerByIndex(0) : nullptr; !IsValid(TargetPlayer) || !IsValid(KeyboardPlayer) || TargetPlayer == KeyboardPlayer) return false;
    const ULocalPlayerSessionDirectory* Directory = GetGameInstance()->GetSubsystem<ULocalPlayerSessionDirectory>();
    const FLocalPlayerSessionRecord* Record = Directory ? Directory->FindPlayer(TargetPlayer) : nullptr;
    const UEditableText* TextField = Record ? Cast<UEditableText>(Record->FocusedWidget.Get()) : nullptr;
    return IsValid(TextField) && !TextField->GetIsReadOnly();
}
bool UPlayerProfileSubsystem::HandleForeignKeyboardNameCancel() const
{
    if (!IsForeignKeyboardNameEntryActive()) return false;
    ULocalPlayer* TargetPlayer = OnboardingQueue->GetCurrentPlayer();
    OnProfileSelectionCancelRequested.Broadcast(TargetPlayer);
    return true;
}
bool UPlayerProfileSubsystem::TryCommitProfileNameFromDevice(const FInputDeviceId InputDeviceId) const
{
    if (!IsForeignKeyboardNameEntryActive() || !InputDeviceId.IsValid()) return false;
    const UGameInstance* GI = GetGameInstance();
    const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
    const FLocalPlayerSessionRecord* Record = Directory ? Directory->FindPlayerForDevice(InputDeviceId) : nullptr;
    ULocalPlayer* InputPlayer = Record ? Record->LocalPlayer.Get() : (GEngine ? GEngine->GetLocalPlayerFromInputDevice(GetWorld(), InputDeviceId) : nullptr);
    if (InputPlayer != OnboardingQueue->GetCurrentPlayer()) return false;
    OnProfileNameCommitRequested.Broadcast(InputPlayer);
    return true;
}
void UPlayerProfileSubsystem::SetJoinInputEnabled(const bool bEnabled)
{
    bJoinInputEnabled = bEnabled;
    if (const UGameInstance* GI = GetGameInstance())
    {
        if (UPlayerJoinInputSubsystem* Input = GI->GetSubsystem<UPlayerJoinInputSubsystem>()) Input->SetInputEnabled(bEnabled);
    }
}

// Mandatory profile
void UPlayerProfileSubsystem::RequireProfileForFirstPlayer(ULocalPlayer* LocalPlayer) const
{
    if (!IsPlayerRegistered(LocalPlayer)) return;
    if (ULocalPlayerSessionDirectory* Directory = GetGameInstance()->GetSubsystem<ULocalPlayerSessionDirectory>())
	{
		Directory->RegisterPlayer(LocalPlayer);
		if (const FInputDeviceId KeyboardDeviceId = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(); KeyboardDeviceId.IsValid()) Directory->AssociateInputDevice(LocalPlayer, KeyboardDeviceId);
	}
	// Keep the retained first local player mandatory after it rejoins
    if (OnboardingQueue && GetGameInstance() && GetGameInstance()->GetLocalPlayerByIndex(0) == LocalPlayer) OnboardingQueue->SetMandatoryPlayer(LocalPlayer);
    EnqueuePlayer(LocalPlayer);
}
bool UPlayerProfileSubsystem::IsMandatoryProfilePlayer(const ULocalPlayer* LocalPlayer) const
{
    return OnboardingQueue && OnboardingQueue->IsMandatoryPlayer(LocalPlayer);
}

// Selection queue
void UPlayerProfileSubsystem::EnqueuePlayer(ULocalPlayer* LocalPlayer) const
{
    if (!IsValid(LocalPlayer)) return;
    if (!IsPlayerRegistered(LocalPlayer)) return;
    if (const UGameInstance* GI = GetGameInstance())
    {
        if (const ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>())
        {
            if (Directory->GetProfileId(LocalPlayer).IsValid()) return;
        }
    }
    if (OnboardingQueue) OnboardingQueue->Enqueue(LocalPlayer);
}
void UPlayerProfileSubsystem::HandleOnboardingQueuePlayerChanged(ULocalPlayer* LocalPlayer) const
{
    if (IsValid(LocalPlayer)) OnOnboardingPlayerChanged.Broadcast(LocalPlayer);
}
void UPlayerProfileSubsystem::ContinueOnboarding() const
{
    if (OnboardingQueue) OnboardingQueue->Continue();
}

// Selection logic
bool UPlayerProfileSubsystem::ConfirmProfileSelection(ULocalPlayer* LocalPlayer, const FGuid& ProfileId) const
{
    if (!IsValid(LocalPlayer) || !OnboardingQueue || OnboardingQueue->GetCurrentPlayer() != LocalPlayer || !IsPlayerRegistered(LocalPlayer)) return false;
    if (!ProfileId.IsValid()) return false;

    // Reject a profile already assigned to another local player
    if (IsProfileTaken(ProfileId))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Profiles] Profile %s is already assigned"), *ProfileId.ToString());
        return false;
    }

    // Fetch loaded profile
    if (const UPlayerProfileSaveGame* ExistingLoadedSave = ProfileRepository ? ProfileRepository->GetLoadedProfile(ProfileId) : nullptr; !ExistingLoadedSave || ExistingLoadedSave->Profile.ProfileId != ProfileId)
    {
        if (!ProfileRepository || !ProfileRepository->LoadProfile(ProfileId))
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

    OnboardingQueue->CompleteCurrent(LocalPlayer);
    return true;
}
void UPlayerProfileSubsystem::CancelProfileSelection(ULocalPlayer* LocalPlayer)
{
    if (!IsValid(LocalPlayer) || !OnboardingQueue || OnboardingQueue->GetCurrentPlayer() != LocalPlayer) return;

    // Mandatory players must select or create a profile
    if (IsMandatoryProfilePlayer(LocalPlayer)) return;

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    // Remove duplicate queue entries for this player
    ReleaseProfile(LocalPlayer);
    OnboardingQueue->CancelCurrent(LocalPlayer);

    // Only remove local players created provisionally by this subsystem's join flow
    if (TemporaryPlayers.Remove(LocalPlayer) > 0 && IsPlayerRegistered(LocalPlayer))
    {
        if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>()) Directory->UnregisterPlayer(LocalPlayer);
        GI->RemoveLocalPlayer(LocalPlayer);
    }
}

// Profile lookup and assignment
TArray<FProfileSummary> UPlayerProfileSubsystem::GetUnassignedProfiles() const
{
    TArray<FProfileSummary> Result = ProfileRepository ? ProfileRepository->GetSavedProfiles() : TArray<FProfileSummary>();
    Result.RemoveAll([this](const FProfileSummary& Profile) { return IsProfileTaken(Profile.ProfileId); });
    return Result;
}
bool UPlayerProfileSubsystem::IsProfileTaken(const FGuid& ProfileId) const
{
    const UGameInstance* GI = GetGameInstance();
    const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
    return Directory && Directory->IsProfileAssigned(ProfileId);
}
bool UPlayerProfileSubsystem::IsPlayerRegistered(ULocalPlayer* LocalPlayer) const
{
    const UGameInstance* GI = GetGameInstance();
    return IsValid(LocalPlayer) && GI && GI->GetLocalPlayers().Contains(LocalPlayer);
}

// Profile operations
FGuid UPlayerProfileSubsystem::CreateProfile(const FString& ProfileName) const
{
    return ProfileRepository ? ProfileRepository->CreateProfile(ProfileName) : FGuid();
}
bool UPlayerProfileSubsystem::EditProfileName(const FGuid& ProfileId, const FString& NewName) const
{
    return ProfileRepository && ProfileRepository->EditProfileName(ProfileId, NewName);
}
bool UPlayerProfileSubsystem::DeleteProfile(const FGuid& ProfileId) const
{
    if (IsProfileTaken(ProfileId))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Profiles] Cannot delete assigned profile %s"), *ProfileId.ToString());
        return false;
    }
    return ProfileRepository && ProfileRepository->DeleteProfile(ProfileId);
}
bool UPlayerProfileSubsystem::AssignProfile(ULocalPlayer* LocalPlayer, const FGuid& ProfileId) const
{
    if (!IsValid(LocalPlayer) || !IsPlayerRegistered(LocalPlayer) || !ProfileId.IsValid() || !ProfileRepository) return false;
    if (const bool bExists = ProfileRepository->GetSavedProfiles().ContainsByPredicate([&ProfileId](const FProfileSummary& Summary) { return Summary.ProfileId == ProfileId; }); !bExists || !ProfileRepository->GetLoadedProfile(ProfileId)) return false;
	const UGameInstance* GI = GetGameInstance();
	ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	if (!Directory || Directory->IsProfileAssigned(ProfileId, LocalPlayer)) return false;
	Directory->SetProfile(LocalPlayer, ProfileId);
    return true;
}
void UPlayerProfileSubsystem::ReleaseProfile(ULocalPlayer* LocalPlayer) const
{
    if (!IsValid(LocalPlayer)) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>())
		{
			if (!Directory->GetProfileId(LocalPlayer).IsValid()) return;
			Directory->SetProfile(LocalPlayer, FGuid());
			OnProfileReleased.Broadcast(LocalPlayer);
		}
	}
}

// Player-bound save access
UPlayerProfileSaveGame* UPlayerProfileSubsystem::GetProfile(ULocalPlayer* LocalPlayer) const
{
    if (!IsPlayerRegistered(LocalPlayer) || !ProfileRepository) return nullptr;
	const UGameInstance* GI = GetGameInstance();
	const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	const FGuid ProfileId = Directory ? Directory->GetProfileId(LocalPlayer) : FGuid();
	if (!ProfileId.IsValid()) return nullptr;
	UPlayerProfileSaveGame* Save = ProfileRepository->GetLoadedProfile(ProfileId);
	if (!Save && ProfileRepository->LoadProfile(ProfileId)) Save = ProfileRepository->GetLoadedProfile(ProfileId);
	return Save && Save->Profile.ProfileId == ProfileId ? Save : nullptr;
}
bool UPlayerProfileSubsystem::SaveProfile(ULocalPlayer* LocalPlayer) const
{
    if (!IsPlayerRegistered(LocalPlayer) || !ProfileRepository) return false;
	const UGameInstance* GI = GetGameInstance();
	const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	const FGuid ProfileId = Directory ? Directory->GetProfileId(LocalPlayer) : FGuid();
	return ProfileId.IsValid() && ProfileRepository->SaveProfile(ProfileId);
}
