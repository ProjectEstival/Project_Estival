#include "UI/InGame/InGameUI.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Systems/Core/GameModes/DungeonGameMode.h"
#include "Systems/Core/Characters/PlayerBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/PanelSlot.h"
#include "Components/GridSlot.h"
#include "UI/InGame/PlayerCard.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UInGameUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (ADungeonGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ADungeonGameMode>() : nullptr) GameMode->OnFloorStarted.AddUniqueDynamic(this, &UInGameUI::OnFloorStarted);
	
	// Party spawning may finish after the shared base widget is initialized
	if (const UWorld* World = GetWorld())World->GetTimerManager().SetTimerForNextTick(this, &UInGameUI::InitializePlayerCards);
}

void UInGameUI::NativeDestruct()
{
	if (ADungeonGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ADungeonGameMode>() : nullptr)GameMode->OnFloorStarted.RemoveDynamic(this, &UInGameUI::OnFloorStarted);
	
	Super::NativeDestruct();
}

void UInGameUI::OnFloorStarted(const int32)
{
	InitializePlayerCards();
}

void UInGameUI::InitializePlayerCards()
{
	if (!PlayerCardsPanel || !PlayerCardClass) return;
	
	const ADungeonGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ADungeonGameMode>() : nullptr;
	if (!GameMode) return;
	
	const TArray<APlayerBase*> Party = GameMode->GetParty();
	const ULocalPlayerSessionDirectory* Directory = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	TSet<int32> UsedColorSlots;
	if (Directory && GetGameInstance())
	{
		for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
		{
			const int32 ColorSlot = Directory->GetPlayerColorSlot(LocalPlayer);
			if (ColorSlot != INDEX_NONE) UsedColorSlots.Add(ColorSlot);
		}
	}
	
	PlayerCardsPanel->ClearChildren();
	PlayerCards.Reset();
	
	constexpr int32 RequiredCardCount = 4;
	for (int32 PartyIndex = 0; PartyIndex < RequiredCardCount; ++PartyIndex)
	{
		UPlayerCard* Card = CreateWidget<UPlayerCard>(GetOwningPlayer(), PlayerCardClass);
		if (!Card) continue;
		
		APlayerBase* PartyMember = Party.IsValidIndex(PartyIndex) ? Party[PartyIndex] : nullptr;
		int32 ColorSlot = INDEX_NONE;
		FLinearColor PlayerColor = FLinearColor::White;
		if (Directory && PartyMember)
		{
			const APlayerController* PlayerController = Cast<APlayerController>(PartyMember->GetController());
			if (const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr) ColorSlot = Directory->GetPlayerColorSlot(LocalPlayer);
			if (ColorSlot == INDEX_NONE) ColorSlot = Directory->GetNextAvailableColorSlot(UsedColorSlots);
			UsedColorSlots.Add(ColorSlot);
			PlayerColor = Directory->GetPlayerColorForSlot(ColorSlot);
			PartyMember->SetPlayerColor(PlayerColor);
		}
		
		Card->InitializeCard(PartyMember, PartyIndex, PlayerColor);
		if (UPanelSlot* CardSlot = PlayerCardsPanel->AddChild(Card))
		{
			const FMargin CardSlotPadding(PlayerCardPadding);
			if (UHorizontalBoxSlot* HorizontalSlot = Cast<UHorizontalBoxSlot>(CardSlot)) HorizontalSlot->SetPadding(CardSlotPadding);
			else if (UVerticalBoxSlot* VerticalSlot = Cast<UVerticalBoxSlot>(CardSlot)) VerticalSlot->SetPadding(CardSlotPadding);
			else if (UGridSlot* GridSlot = Cast<UGridSlot>(CardSlot)) GridSlot->SetPadding(CardSlotPadding);
			else if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(CardSlot)) OverlaySlot->SetPadding(CardSlotPadding);
		}
		PlayerCards.Add(Card);
	}
}
