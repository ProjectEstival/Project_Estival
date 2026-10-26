#include "MainMenuHUD.h"

#include "GameFramework/PlayerController.h"
#include "PlayerProfileSubsystem.h"
#include "ProfileSelectionPopup.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

void AMainMenuHUD::BeginPlay()
{
    Super::BeginPlay();
    
    const UGameInstance* GI = GetGameInstance();
    const APlayerController* OwningPC = GetOwningPlayerController();
    if (!GI || !OwningPC) return;
    
    // Only local player 0's HUD manages global onboarding UI.
    if (OwningPC->GetLocalPlayer() != GI->GetLocalPlayerByIndex(0)) return;
    
    PlayerSubsystem = GI->GetSubsystem<UPlayerProfileSubsystem>();
    if (!PlayerSubsystem) return;
    
    PlayerSubsystem->OnOnboardingPlayerChanged.AddDynamic(this, &AMainMenuHUD::HandleOnboardingPlayerChanged);
}
void AMainMenuHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (PlayerSubsystem) PlayerSubsystem->OnOnboardingPlayerChanged.RemoveDynamic(this, &AMainMenuHUD::HandleOnboardingPlayerChanged);
    
    Super::EndPlay(EndPlayReason);
}

void AMainMenuHUD::HandleOnboardingPlayerChanged(ULocalPlayer* LocalPlayer)
{
    if (!IsValid(LocalPlayer) || !IsValid(PlayerSubsystem) || !ProfilePopupClass) return;
    APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
    if (!IsValid(PC) || IsValid(ActivePopup)) return; // Don't replace an active popup just because another onboarding event was received.
    
    ActivePopup = Cast<UProfileSelectionPopup>(PushWidget(ProfilePopupClass, 0, EManagedUIInputMode::UIOnly, true, PC));
    if (!IsValid(ActivePopup)) return;
    
    ActivePopup->SetTargetLocalPlayer(LocalPlayer);
    ActivePopup->OnProfileSelected.AddDynamic(this, &AMainMenuHUD::HandleProfileSelected);
    ActivePopup->OnProfileSelectionCancelled.AddDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
}

void AMainMenuHUD::HandleProfileSelected(ULocalPlayer* LocalPlayer, int32 ProfileIndex)
{
    if (!IsValid(PlayerSubsystem)) return;
    if (IsValid(ActivePopup) && ActivePopup->GetTargetLocalPlayer() == LocalPlayer)
    {
        ActivePopup->OnProfileSelected.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelected);
        ActivePopup->OnProfileSelectionCancelled.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
        
        PopWidget();
        ActivePopup = nullptr;
    }
    PlayerSubsystem->ConfirmProfileSelection(LocalPlayer, ProfileIndex);
}
void AMainMenuHUD::HandleProfileSelectionCancelled(ULocalPlayer* LocalPlayer)
{
    if (!IsValid(PlayerSubsystem)) return;
    
    // Close the current popup before advancing the queue.
    if (IsValid(ActivePopup) && ActivePopup->GetTargetLocalPlayer() == LocalPlayer)
    {
        ActivePopup->OnProfileSelected.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelected);
        ActivePopup->OnProfileSelectionCancelled.RemoveDynamic(this, &AMainMenuHUD::HandleProfileSelectionCancelled);
        
        PopWidget();
        ActivePopup = nullptr;
    }
    PlayerSubsystem->CancelProfileSelection(LocalPlayer);
}
