#include "UI/CharacterSelect/CharacterSelectScreen.h"
#include "Systems/Core/GameModes/DungeonGameMode.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "UI/CharacterSelect/CharacterSelectCard.h"
#include "GameFramework/PlayerController.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/PanelSlot.h"
#include "Components/GridSlot.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Systems/Mike/Combat/Loadouts/Loadout.h"
#include "Systems/Mike/Combat/Abilities/AbilityDefinition.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"
#include "Systems/Core/Profiles/PlayerProfileSaveGame.h"
#include "GameplayTagContainer.h"

void UCharacterSelectScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode()))
	{
		OnAllPlayersSelected.AddUniqueDynamic(GameMode, &ADungeonGameMode::HandleAllPlayersSelected);
	}
	if (!CharacterCardsPanel || !CharacterSelectCardClass) return;
	
	CharacterCardsPanel->ClearChildren();
	AvailableCharacters.Reset();
	constexpr int32 CardCount = 4;
	for (int32 Index = 0; Index < CardCount; ++Index)
	{
		if (Characters.IsValidIndex(Index) && Characters[Index].Loadout && Characters[Index].CharacterClass) AvailableCharacters.Add(Index);
		if (UCharacterSelectCard* Card = CreateWidget<UCharacterSelectCard>(GetOwningPlayer(), CharacterSelectCardClass))
		{
			Card->InitializeCard(Index, FText::GetEmpty());
			Card->SetPlayerAssigned(false);
			if (UPanelSlot* AddedCardSlot = CharacterCardsPanel->AddChild(Card))
			{
				const FMargin CardSlotPadding(CharacterCardPadding);
				if (UHorizontalBoxSlot* HorizontalSlot = Cast<UHorizontalBoxSlot>(AddedCardSlot)) HorizontalSlot->SetPadding(CardSlotPadding);
				else if (UVerticalBoxSlot* VerticalSlot = Cast<UVerticalBoxSlot>(AddedCardSlot)) VerticalSlot->SetPadding(CardSlotPadding);
				else if (UGridSlot* GridSlot = Cast<UGridSlot>(AddedCardSlot)) GridSlot->SetPadding(CardSlotPadding);
				else if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(AddedCardSlot)) OverlaySlot->SetPadding(CardSlotPadding);
			}
			CharacterSelectCards.Add(Card);
		}
	}
	const UGameInstance* GI = GetGameInstance();
	ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	const UPlayerProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;
	if (GI && Directory)
	{
		int32 PlayerCardSlot = 0;
		for (ULocalPlayer* Player : GI->GetLocalPlayers())
		{
			if (Directory->GetProfileId(Player).IsValid() && PlayerCardSlot < CharacterSelectCards.Num())
			{
				UCharacterSelectCard* Card = CharacterSelectCards[PlayerCardSlot];
				const UPlayerProfileSaveGame* ProfileSave = Profiles ? Profiles->GetProfile(Player) : nullptr;
				Card->SetProfile(ProfileSave ? FText::FromString(ProfileSave->Profile.ProfileName) : FText::GetEmpty(), Directory->GetPlayerColor(Player));
				FocusedCharacterByPlayer.Add(Player, AvailableCharacters.IsEmpty() ? INDEX_NONE : AvailableCharacters[0]);
				PlayerCardSlotByPlayer.Add(Player, PlayerCardSlot);
				FocusedAbilityByPlayer.Add(Player, 0);
				Card->SetPlayerAssigned(true);
				++PlayerCardSlot;
			}
		}
	}
	RefreshCardStates();
	if (GI && Directory)
	{
		for (const TPair<TObjectPtr<ULocalPlayer>, int32>& Entry : PlayerCardSlotByPlayer)
		{
			if (!CharacterSelectCards.IsValidIndex(Entry.Value)) continue;
			UCharacterSelectCard* Card = CharacterSelectCards[Entry.Value];
			Directory->FocusWidget(Entry.Key, Card);
			Card->FocusAbility(0, Entry.Key->GetPlayerController(GetWorld()));
		}
	}
}

ULocalPlayer* UCharacterSelectScreen::ResolvePlayer(const APlayerController* PlayerController) const
{
	return IsValid(PlayerController) ? PlayerController->GetLocalPlayer() : GetOwningLocalPlayer();
}

bool UCharacterSelectScreen::HandleUIInputAction_Implementation(const EUIInputAction Action, APlayerController* Instigator)
{
	ULocalPlayer* Player = IsValid(Instigator) ? Instigator->GetLocalPlayer() : nullptr;
	if (!IsValid(Player) || !FocusedCharacterByPlayer.Contains(Player)) return false;
	switch (Action)
	{
		case EUIInputAction::Submit:
			return HandlePlayerSubmit(Player);
		
		case EUIInputAction::Back:
			return HandlePlayerBack(Instigator);
		
		case EUIInputAction::PreviousAbility:
			NavigateAbility(Instigator, -1);
			return true;
		
		case EUIInputAction::NextAbility:
			NavigateAbility(Instigator, 1);
			return true;
		
		case EUIInputAction::PreviousCharacter:
			NavigateCharacter(Instigator, -1);
			return true;
		
		case EUIInputAction::NextCharacter:
			NavigateCharacter(Instigator, 1);
			return true;
		
		default:
			return false;
	}
}

void UCharacterSelectScreen::NavigateCharacter(APlayerController* PlayerController, const int32 Direction)
{
	ULocalPlayer* Player = ResolvePlayer(PlayerController);
	if (!IsValid(Player) || ConfirmedCharacterByPlayer.Contains(Player) || AvailableCharacters.IsEmpty()) return;
	const int32 CurrentIndex = FocusedCharacterByPlayer.FindRef(Player);
	int32 Position = AvailableCharacters.IndexOfByKey(CurrentIndex);
	if (Position == INDEX_NONE) Position = 0;
	Position = (Position + (Direction < 0 ? -1 : 1) + AvailableCharacters.Num()) % AvailableCharacters.Num();
	FocusedCharacterByPlayer.Add(Player, AvailableCharacters[Position]);
	RefreshCardStates();
	if (const int32* CardIndex = PlayerCardSlotByPlayer.Find(Player); CardIndex && CharacterSelectCards.IsValidIndex(*CardIndex)) CharacterSelectCards[*CardIndex]->FocusAbility(FocusedAbilityByPlayer.FindRef(Player), Player->GetPlayerController(GetWorld()));
}
void UCharacterSelectScreen::NavigateAbility(APlayerController* PlayerController, const int32 Direction)
{
	ULocalPlayer* Player = ResolvePlayer(PlayerController);
	if (!IsValid(Player)) return;
	const int32 CurrentIndex = FocusedAbilityByPlayer.FindRef(Player);
	const int32 NextIndex = (CurrentIndex + (Direction < 0 ? -1 : 1) + 4) % 4;
	FocusedAbilityByPlayer.Add(Player, NextIndex);
	if (const int32* CardIndex = PlayerCardSlotByPlayer.Find(Player); CardIndex && CharacterSelectCards.IsValidIndex(*CardIndex)) CharacterSelectCards[*CardIndex]->FocusAbility(NextIndex, PlayerController);
}

bool UCharacterSelectScreen::HandlePlayerSubmit(ULocalPlayer* Player)
{
	if (!IsValid(Player) || !PlayerCardSlotByPlayer.Contains(Player)) return false;
	if (const int32* CharacterIndex = FocusedCharacterByPlayer.Find(Player)) ConfirmCharacter(*CharacterIndex, Player->GetPlayerController(GetWorld()));
	return true;
}
void UCharacterSelectScreen::ConfirmCharacter(const int32 CharacterIndex, APlayerController* PlayerController)
{
	ULocalPlayer* Player = ResolvePlayer(PlayerController);
	if (!IsValid(Player) || ConfirmedCharacterByPlayer.Contains(Player) || !AvailableCharacters.Contains(CharacterIndex)) return;
	ConfirmedCharacterByPlayer.Add(Player, CharacterIndex);
	AvailableCharacters.Remove(CharacterIndex);
	for (TPair<TObjectPtr<ULocalPlayer>, int32>& Entry : FocusedCharacterByPlayer)
	{
		if (Entry.Key.Get() != Player && Entry.Value == CharacterIndex) Entry.Value = AvailableCharacters.IsEmpty() ? INDEX_NONE : AvailableCharacters[0];
	}
	FocusedCharacterByPlayer.Add(Player, CharacterIndex);
	RefreshCardStates();
	CompleteIfReady();
}

bool UCharacterSelectScreen::HandlePlayerBack(APlayerController* PlayerController)
{
	ULocalPlayer* Player = ResolvePlayer(PlayerController);
	if (!IsValid(Player)) return false;
	const int32* SelectedIndex = ConfirmedCharacterByPlayer.Find(Player);
	if (!SelectedIndex) return false;
	AvailableCharacters.Add(*SelectedIndex);
	AvailableCharacters.Sort();
	ConfirmedCharacterByPlayer.Remove(Player);
	FocusedCharacterByPlayer.Add(Player, *SelectedIndex);
	RefreshPlayerCard(Player);
	return true;
}
bool UCharacterSelectScreen::HandleBack_Implementation(APlayerController* Instigator)
{
	return HandlePlayerBack(Instigator);
}
void UCharacterSelectScreen::OnBackPopped_Implementation(APlayerController* Instigator)
{
	HandlePlayerBack(Instigator);
}

void UCharacterSelectScreen::RefreshCardStates()
{
	for (const TPair<TObjectPtr<ULocalPlayer>, int32>& Entry : PlayerCardSlotByPlayer)
	{
		RefreshPlayerCard(Entry.Key);
	}
}
void UCharacterSelectScreen::RefreshPlayerCard(ULocalPlayer* Player)
{
	const int32* SlotIndex = PlayerCardSlotByPlayer.Find(Player);
	if (!SlotIndex || !CharacterSelectCards.IsValidIndex(*SlotIndex)) return;
	UCharacterSelectCard* Card = CharacterSelectCards[*SlotIndex];
	if (!IsValid(Card)) return;
	const int32* ConfirmedIndex = ConfirmedCharacterByPlayer.Find(Player);
	const int32* FocusedIndex = FocusedCharacterByPlayer.Find(Player);
	const int32 DisplayIndex = ConfirmedIndex ? *ConfirmedIndex : FocusedIndex ? *FocusedIndex : INDEX_NONE;
	ULoadout* Loadout = Characters.IsValidIndex(DisplayIndex) ? Characters[DisplayIndex].Loadout : nullptr;
	Card->SetLoadout(Loadout);
	TArray<FCharacterSelectAbilityInfo> AbilityInfos;
	static const FName AbilitySlotNames[] = { TEXT("Ability.First"), TEXT("Ability.Second"), TEXT("Ability.Third"), TEXT("Ability.Fourth") };
	for (const FName SlotName : AbilitySlotNames)
	{
		FCharacterSelectAbilityInfo& Info = AbilityInfos.AddDefaulted_GetRef();
		const FGameplayTag SlotTag = FGameplayTag::RequestGameplayTag(SlotName, false);
		if (Loadout && SlotTag.IsValid())
		{
			if (const TObjectPtr<UAbilityDefinition>* Ability = Loadout->Abilities.Find(SlotTag); Ability && IsValid(Ability->Get()))
			{
				Info.Name = Ability->Get()->DisplayName;
				Info.Description = Ability->Get()->Description;
				Info.Icon = Ability->Get()->Icon;
			}
		}
	}
	Card->SetAbilities(AbilityInfos);
	Card->SetSelected(ConfirmedCharacterByPlayer.Contains(Player));
	Card->SetTargeted(DisplayIndex != INDEX_NONE && Characters.IsValidIndex(DisplayIndex));
	Card->SetIsEnabled(true);
}

void UCharacterSelectScreen::CompleteIfReady()
{
	if (bHasCompletedSelection) return;
	const UGameInstance* GI = GetGameInstance();
	ULocalPlayerSessionDirectory* Directory = GI ? GI->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr;
	if (!GI || !Directory) return;
	int32 AssignedProfileCount = 0;
	for (const ULocalPlayer* Player : GI->GetLocalPlayers()) if (Directory->GetProfileId(Player).IsValid()) ++AssignedProfileCount;
	if (ConfirmedCharacterByPlayer.Num() < AssignedProfileCount) return;
	
	TArray<FCharacterSelectPlayerChoice> Choices;
	for (const TPair<TObjectPtr<ULocalPlayer>, int32>& Entry : ConfirmedCharacterByPlayer)
	{
		auto& [LocalPlayer, CharacterTitle, Loadout, CharacterClass] = Choices.AddDefaulted_GetRef();
		LocalPlayer = Entry.Key;
		const FCharacterSelectCharacter* Character = Characters.IsValidIndex(Entry.Value) ? &Characters[Entry.Value] : nullptr;
		Loadout = Character ? Character->Loadout : nullptr;
		CharacterClass = Character ? Character->CharacterClass : nullptr;
		CharacterTitle = Loadout ? Loadout->DisplayName : FText::FromString(FString::Printf(TEXT("Character %d"), Entry.Value + 1));
	}
	TArray<int32> RemainingCharacters = AvailableCharacters;
	while (Choices.Num() < Characters.Num() && !RemainingCharacters.IsEmpty())
	{
		const int32 RandomPosition = FMath::RandRange(0, RemainingCharacters.Num() - 1);
		const int32 CharacterIndex = RemainingCharacters[RandomPosition];
		RemainingCharacters.RemoveAtSwap(RandomPosition);
		const FCharacterSelectCharacter& Character = Characters[CharacterIndex];
		FCharacterSelectPlayerChoice& AIChoice = Choices.AddDefaulted_GetRef();
		AIChoice.LocalPlayer = nullptr;
		AIChoice.Loadout = Character.Loadout;
		AIChoice.CharacterClass = Character.CharacterClass;
		AIChoice.CharacterTitle = Character.Loadout->DisplayName;
	}
	bHasCompletedSelection = true;
	OnAllPlayersSelected.Broadcast(Choices);
}

