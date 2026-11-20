#include "VendorProfileAccess.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"

FVendorProfileAccess FVendorProfileAccess::Find(AActor* Buyer)
{
	FVendorProfileAccess Access;
	if (!Buyer) return Access;

	const APawn* Pawn = Cast<APawn>(Buyer);
	const APlayerController* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : Cast<APlayerController>(Buyer);
	Access.LocalPlayer = Controller ? Controller->GetLocalPlayer() : nullptr;
	if (!Access.LocalPlayer) return Access;

	Access.Profiles = UGameInstance::GetSubsystem<UPlayerProfileSubsystem>(Buyer->GetGameInstance());
	if (Access.Profiles) Access.Save = Access.Profiles->GetProfile(Access.LocalPlayer);
	return Access;
}

void FVendorProfileAccess::Commit() const { if (Profiles && LocalPlayer) Profiles->SaveProfile(LocalPlayer); }