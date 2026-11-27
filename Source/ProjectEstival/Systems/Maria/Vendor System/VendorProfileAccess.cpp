// Fill out your copyright notice in the Description page of Project Settings.

#include "VendorProfileAccess.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"

ULocalPlayer* FVendorProfileAccess::FindLocalPlayer(AActor* Buyer)
{
	if (!Buyer) return nullptr;

	const APawn* Pawn = Cast<APawn>(Buyer);
	const APlayerController* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : Cast<APlayerController>(Buyer);
	return Controller ? Controller->GetLocalPlayer() : nullptr;
}

FVendorProfileAccess FVendorProfileAccess::Find(AActor* Buyer)
{
	FVendorProfileAccess Access;
	Access.LocalPlayer = FindLocalPlayer(Buyer);
	if (!Access.LocalPlayer) return Access;

	Access.Profiles = UGameInstance::GetSubsystem<UPlayerProfileSubsystem>(Buyer->GetGameInstance());
	if (Access.Profiles) Access.Save = Access.Profiles->GetProfile(Access.LocalPlayer);
	return Access;
}

void FVendorProfileAccess::Commit() const { if (Profiles && LocalPlayer) Profiles->SaveProfile(LocalPlayer); }