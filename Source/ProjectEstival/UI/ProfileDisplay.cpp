#include "UI/ProfileDisplay.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "UI/ProfileDisplayEntry.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

void UProfileDisplay::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	
	BindProfileEvents();
	RefreshDisplay();
}
void UProfileDisplay::BeginDestroy()
{
	UnbindProfileEvents();
	
	Super::BeginDestroy();
}

void UProfileDisplay::BindProfileEvents()
{
	const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UPlayerProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;
	if (BoundProfileSubsystem == Profiles) return;
	
	UnbindProfileEvents();
	if (!IsValid(Profiles)) return;
	
	BoundProfileSubsystem = Profiles;
	BoundProfileSubsystem->OnProfileAssigned.AddUniqueDynamic(this, &UProfileDisplay::HandleProfileAssigned);
	BoundProfileSubsystem->OnProfileReleased.AddUniqueDynamic(this, &UProfileDisplay::HandleProfileChanged);
	BoundProfileSubsystem->OnPlayerDisconnected.AddUniqueDynamic(this, &UProfileDisplay::HandleProfileChanged);
}
void UProfileDisplay::UnbindProfileEvents()
{
	if (!IsValid(BoundProfileSubsystem)) return;
	BoundProfileSubsystem->OnProfileAssigned.RemoveDynamic(this, &UProfileDisplay::HandleProfileAssigned);
	BoundProfileSubsystem->OnProfileReleased.RemoveDynamic(this, &UProfileDisplay::HandleProfileChanged);
	BoundProfileSubsystem->OnPlayerDisconnected.RemoveDynamic(this, &UProfileDisplay::HandleProfileChanged);
	BoundProfileSubsystem = nullptr;
}

void UProfileDisplay::HandleProfileChanged(ULocalPlayer*)
{
	RefreshDisplay();
}
void UProfileDisplay::HandleProfileAssigned(ULocalPlayer*, FGuid)
{
	RefreshDisplay();
}

void UProfileDisplay::RefreshDisplay()
{
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld()) return;
	
	ClearChildren();
	if (!PlayerReadyEntryWidgetClass) return;
	
	const UGameInstance* GI = World->GetGameInstance();
	const ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	if (!GI || !Directory) return;
	
	TArray<ULocalPlayer*> ReadyPlayers;
	for (ULocalPlayer* LocalPlayer : GI->GetLocalPlayers())
	{
		if (IsValid(LocalPlayer) && Directory->GetProfileId(LocalPlayer).IsValid()) ReadyPlayers.Add(LocalPlayer);
	}
	ReadyPlayers.Sort([Directory](const ULocalPlayer& A, const ULocalPlayer& B) { return Directory->GetPlayerColorSlot(&A) < Directory->GetPlayerColorSlot(&B); });
	
	for (ULocalPlayer* LocalPlayer : ReadyPlayers)
	{
		APlayerController* OwningPlayer = LocalPlayer->GetPlayerController(World);
		UProfileDisplayEntry* Entry = OwningPlayer ? CreateWidget<UProfileDisplayEntry>(OwningPlayer, PlayerReadyEntryWidgetClass) : CreateWidget<UProfileDisplayEntry>(GetWorld(), PlayerReadyEntryWidgetClass);
		if (!IsValid(Entry) || !Entry->SetPlayer(LocalPlayer)) continue;
		AddChildToHorizontalBox(Entry);
	}
}
