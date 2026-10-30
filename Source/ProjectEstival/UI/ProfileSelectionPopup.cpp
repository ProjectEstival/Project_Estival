
#include "UI/ProfileSelectionPopup.h"

#include "Engine/LocalPlayer.h"

void UProfileSelectionPopup::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	SetIsFocusable(true);
}

void UProfileSelectionPopup::SetTargetLocalPlayer(ULocalPlayer* LocalPlayer)
{
	OnTargetPlayerAssigned(IsValid(LocalPlayer) ? LocalPlayer->GetLocalPlayerIndex() : INDEX_NONE);
	TargetLocalPlayer = LocalPlayer;
	bSelectionCompleted = false;
}

void UProfileSelectionPopup::ConfirmSelection(int32 ProfileIndex)
{
	if (bSelectionCompleted || ProfileIndex < 0) return;
	if (!IsValid(TargetLocalPlayer)) return;
	
	bSelectionCompleted = true;
	
	OnSelectionConfirmed(ProfileIndex);
	OnProfileSelected.Broadcast(TargetLocalPlayer, ProfileIndex);
}
void UProfileSelectionPopup::CancelSelection()
{
	if (bSelectionCompleted) return;
	if (!IsValid(TargetLocalPlayer)) return;
	bSelectionCompleted = true;
	
	OnSelectionCancelled();
	OnProfileSelectionCancelled.Broadcast(TargetLocalPlayer);
}
