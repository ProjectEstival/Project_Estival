#include "MainMenuHUD.h"

#include "GameFramework/PlayerController.h"
#include "ProfileSelectionPopup.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

void AMainMenuHUD::BeginPlay()
{
    Super::BeginPlay();
    
    const UGameInstance* GI = GetGameInstance();
    const APlayerController* OwningPC = GetOwningPlayerController();
    if (!GI || !OwningPC) return;
    
    // Only local player 0's HUD manages global onboarding UI
    if (OwningPC->GetLocalPlayer() != GI->GetLocalPlayerByIndex(0)) return;
    
    // Player profile setup
    PlayerProfileSubsystem = GI->GetSubsystem<UPlayerProfileSubsystem>();
    if (!PlayerProfileSubsystem) return;
    PlayerProfileSubsystem->OnOnboardingPlayerChanged.AddDynamic(this, &AMainMenuHUD::HandleOnboardingPlayerChanged);
    PlayerProfileSubsystem->OnProfileSelectionCancelRequested.AddDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
}
void AMainMenuHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Player profile cleanup
    if (PlayerProfileSubsystem)
    {
        PlayerProfileSubsystem->OnOnboardingPlayerChanged.RemoveDynamic(this, &AMainMenuHUD::HandleOnboardingPlayerChanged);
        PlayerProfileSubsystem->OnProfileSelectionCancelRequested.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
    }
    if (IsValid(ActivePopup))
    {
        ActivePopup->OnProfileSelected.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelected);
        ActivePopup->OnProfileSelectionCancelled.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
    }
    ActivePopup = nullptr;
    PlayerProfileSubsystem = nullptr;
    
    Super::EndPlay(EndPlayReason);
}

// Player profiles
void AMainMenuHUD::HandleOnboardingPlayerChanged(ULocalPlayer* LocalPlayer)
{
    if (!IsValid(LocalPlayer) || !IsValid(PlayerProfileSubsystem) || !ProfilePopupClass) return;
    APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
    if (!IsValid(PC) || IsValid(ActivePopup)) return; // Never replace an active popup
    
    ActivePopup = Cast<UProfileSelectionPopup>(PushWidget(ProfilePopupClass, 0, EManagedUIInputMode::UIOnly, true, PC));
    if (!IsValid(ActivePopup))
    {
        UE_LOG(LogTemp, Error, TEXT("[Profiles] Failed to create profile-selection popup"));
        return;
    }
    
    ActivePopup->SetTargetLocalPlayer(LocalPlayer);
    ActivePopup->OnProfileSelected.AddDynamic(this, &AMainMenuHUD::HandleProfileSelected);
    ActivePopup->OnProfileSelectionCancelled.AddDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
}
void AMainMenuHUD::HandleProfileSelected(ULocalPlayer* LocalPlayer, FGuid ProfileId)
{
    if (!IsValid(PlayerProfileSubsystem) || !IsValid(ActivePopup) || ActivePopup->GetTargetLocalPlayer() != LocalPlayer) return;
    
    // Keep the popup open if loading or assignment fails
    if (!PlayerProfileSubsystem->ConfirmProfileSelection(LocalPlayer, ProfileId)) return;
    
    ActivePopup->CompleteSelection();
    ActivePopup->OnProfileSelected.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelected);
    ActivePopup->OnProfileSelectionCancelled.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
    PopWidget();
    ActivePopup = nullptr;
    
    PlayerProfileSubsystem->ContinueOnboarding();
}
void AMainMenuHUD::HandleProfileSelectionCancelled(ULocalPlayer* LocalPlayer)
{
    if (!IsValid(PlayerProfileSubsystem)) return;
    
    // Close the current popup before advancing the queue
    if (IsValid(ActivePopup) && ActivePopup->GetTargetLocalPlayer() == LocalPlayer)
    {
        ActivePopup->OnProfileSelected.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelected);
        ActivePopup->OnProfileSelectionCancelled.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
        
        PopWidget();
        ActivePopup = nullptr;
    }
    PlayerProfileSubsystem->CancelProfileSelection(LocalPlayer);
}
