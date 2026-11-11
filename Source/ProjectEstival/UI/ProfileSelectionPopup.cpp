#include "UI/ProfileSelectionPopup.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Systems/Core/Profiles/PlayerProfileSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "UI/PlayerProfileCard.h"
#include "Components/ScrollBox.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Components/Widget.h"

// Initialization
void UProfileSelectionPopup::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	SetIsFocusable(true);
	if (IsValid(OptionsEntryButton)) OptionsEntryButton->OnReleased.AddDynamic(this, &UProfileSelectionPopup::HandleOptionsEntryReleased);
	if (IsValid(EditButton)) EditButton->OnClicked.AddDynamic(this, &UProfileSelectionPopup::EditProfile);
	if (IsValid(DeleteButton)) DeleteButton->OnClicked.AddDynamic(this, &UProfileSelectionPopup::DeleteProfile);
}
void UProfileSelectionPopup::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bPendingProfileListRefresh && !IsValid(PendingNameCard))
	{
		bPendingProfileListRefresh = false;
		ActiveOptionsCard = nullptr;
		if (IsValid(Options)) Options->SetVisibility(ESlateVisibility::Collapsed);
		RebuildProfileCards();
		bPendingInitialFocus = true;
	}
	
	if (bPendingNameFocus)
	{
		if (!IsInViewport() || !IsValid(PendingNameCard) || !ProfileScrollBox || ProfileScrollBox->GetCachedGeometry().GetLocalSize().IsNearlyZero()) return;
		ProfileScrollBox->ScrollWidgetIntoView(PendingNameCard, true, EDescendantScrollDestination::Center);
		APlayerController* PC = IsValid(TargetLocalPlayer) ? TargetLocalPlayer->GetPlayerController(GetWorld()) : nullptr;
		if (!IsValid(PC)) return;
		PendingNameCard->FocusNameEntry(PC);
		bPendingNameFocus = false;
		return;
	}
	
	if (!bPendingInitialFocus) return;
	if (!IsInViewport()) return;
	if (!ProfileScrollBox || ProfileScrollBox->GetCachedGeometry().GetLocalSize().IsNearlyZero()) return;
	
	if (FocusFirstCard()) bPendingInitialFocus = false;
}
void UProfileSelectionPopup::SetTargetLocalPlayer(ULocalPlayer* LocalPlayer)
{
	TargetLocalPlayer = LocalPlayer;
	bSelectionCompleted = false;
	bPendingInitialFocus = false;
	
	RebuildProfileCards();
	if (IsValid(LocalPlayer)) Title->SetText(FText::Format(FText::FromString("Player {0}"), FText::AsNumber(LocalPlayer->GetLocalPlayerIndex())));
	bPendingInitialFocus = true;
}

// Cards
void UProfileSelectionPopup::RebuildProfileCards()
{
	if (!ProfileScrollBox || !ProfileCardClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ProfileSelection] ProfileScrollBox or ProfileCardClass is not set."));
		return;
	}
	ProfileScrollBox->ClearChildren();
	SpawnedCards.Reset();

	const UGameInstance* GameInstance = GetGameInstance();
	const UPlayerProfileSubsystem* ProfileSubsystem = GameInstance ? GameInstance->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;
	if (!ProfileSubsystem) return;
	
	// Add all available profiles
	for (const auto& Profiles = ProfileSubsystem->GetUnassignedProfiles(); const auto& [ProfileId, ProfileName] : Profiles)
	{
		UPlayerProfileCard* Card = CreateWidget<UPlayerProfileCard>(this, ProfileCardClass);
		if (!Card) continue;
		
		Card->SetProfile(ProfileId, ProfileName);
		Card->OnCardClicked.AddDynamic(this, &UProfileSelectionPopup::HandleCardClicked);
		Card->OnCardLongPressed.AddDynamic(this, &UProfileSelectionPopup::HandleCardLongPressed);
		Card->OnNameCommitted.AddDynamic(this, &UProfileSelectionPopup::HandleNameCommitted);
		
		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(ProfileScrollBox->AddChild(Card))) ScrollSlot->SetPadding(FMargin(HorizontalMargin, 0.0f));
		SpawnedCards.Add(Card);
	}
	
	// Append the create-profile tile last
	if (UPlayerProfileCard* CreateCard = CreateWidget<UPlayerProfileCard>(this, ProfileCardClass))
	{
		CreateCard->SetCreateCard();
		CreateCard->OnCardClicked.AddDynamic(this, &UProfileSelectionPopup::HandleCardClicked);
		CreateCard->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
		
		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(ProfileScrollBox->AddChild(CreateCard))) ScrollSlot->SetPadding(FMargin(HorizontalMargin, 0.0f));
		SpawnedCards.Add(CreateCard);
	}
	if (!SpawnedCards.IsEmpty()) SpawnedCards[0]->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
}
bool UProfileSelectionPopup::FocusFirstCard()
{
	if (!IsValid(TargetLocalPlayer) || SpawnedCards.IsEmpty()) return false;
	
	UPlayerProfileCard* FirstCard = SpawnedCards[0];
	if (!IsValid(FirstCard)) return false;
	
	// Bring the initial selection into view
	ProfileScrollBox->ScrollWidgetIntoView(FirstCard, true);
	
	// Focus through the controller belonging to this local player
	APlayerController* PlayerController = TargetLocalPlayer->GetPlayerController(GetWorld());
	if (!IsValid(PlayerController)) return false;
	
	FirstCard->FocusCard(PlayerController);
	ProfileScrollBox->ScrollWidgetIntoView(FirstCard, false, EDescendantScrollDestination::Center);
	return true;
}
void UProfileSelectionPopup::HandleCardClicked(UPlayerProfileCard* Card)
{
	if (bSelectionCompleted || !IsValid(Card)) return;
	
	// Don't allow multiple unfinished profile names
	if (IsValid(PendingNameCard))
	{
		if (PendingNameCard->IsNamingProfile()) bPendingNameFocus = true;
		return;
	}
	
	if (Card->IsCreateCard())
	{
		BeginCreatingProfile();
		return;
	}
	
	const FGuid ProfileId = Card->GetProfileId();
	if (!ProfileId.IsValid()) return;
	
	// Existing profile selection flow
	ConfirmSelection(ProfileId);
}
void UProfileSelectionPopup::HandleCardLongPressed(UPlayerProfileCard* Card)
{
	if (bSelectionCompleted || !IsValid(Card) || Card->IsCreateCard() || IsValid(PendingNameCard)) return;
	if (IsValid(ActiveOptionsCard)) CloseCardOptions();
	
	ActiveOptionsCard = Card;
	Options->SetVisibility(ESlateVisibility::Visible);
	if (IsValid(TargetLocalPlayer))
	{
		if (APlayerController* PC = TargetLocalPlayer->GetPlayerController(GetWorld()))
		{
			if (ULocalPlayerSessionDirectory* Directory = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr) Directory->FocusWidget(TargetLocalPlayer, OptionsEntryButton);
			else OptionsEntryButton->SetUserFocus(PC);
		}
	}
}
void UProfileSelectionPopup::CloseCardOptions()
{
	if (IsValid(ActiveOptionsCard) && IsValid(TargetLocalPlayer))
	{
		if (APlayerController* PC = TargetLocalPlayer->GetPlayerController(GetWorld())) ActiveOptionsCard->FocusCard(PC);
	}
	ActiveOptionsCard = nullptr;
	if (IsValid(Options)) Options->SetVisibility(ESlateVisibility::Collapsed);
}

// Card options
void UProfileSelectionPopup::EditProfile()
{
	if (!IsValid(ActiveOptionsCard)) return;
	
	ActiveOptionsCard->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
	ActiveOptionsCard->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
	
	ActiveOptionsCard->BeginNameEdit();
	
	PendingNameCard = ActiveOptionsCard;
	bPendingNameFocus = true;
	
	CloseCardOptions();
}
bool UProfileSelectionPopup::CancelPendingNameEdit()
{
	if (!IsValid(PendingNameCard)) return false;
	
	UPlayerProfileCard* CancelledCard = PendingNameCard;
	const bool bWasCreating = !CancelledCard->GetProfileId().IsValid();
	PendingNameCard = nullptr;
	bPendingNameFocus = false;
	
	UPlayerProfileCard* FocusTarget = CancelledCard;
	if (bWasCreating)
	{
		CancelledCard->EndNameEntry();
		if (ProfileScrollBox) ProfileScrollBox->RemoveChild(CancelledCard);
		SpawnedCards.Remove(CancelledCard);
		FocusTarget = SpawnedCards.IsEmpty() ? nullptr : SpawnedCards.Last();
	}
	else
	{
		CancelledCard->CancelNameEdit();
		const int32 CardIndex = SpawnedCards.IndexOfByKey(CancelledCard);
		CancelledCard->SetNavigationRuleBase(EUINavigation::Left, CardIndex == 0 ? EUINavigationRule::Stop : EUINavigationRule::Escape);
		CancelledCard->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Escape);
	}
	
	if (IsValid(TargetLocalPlayer) && IsValid(FocusTarget))
	{
		if (ProfileScrollBox) ProfileScrollBox->ScrollWidgetIntoView(FocusTarget, true, EDescendantScrollDestination::Center);
		if (APlayerController* PC = TargetLocalPlayer->GetPlayerController(GetWorld())) FocusTarget->FocusCard(PC);
	}
	return true;
}
bool UProfileSelectionPopup::CommitPendingNameEntry() const
{
	return IsValid(PendingNameCard) && PendingNameCard->CommitNameEntry();
}
void UProfileSelectionPopup::DeleteProfile()
{
	if (!IsValid(ActiveOptionsCard) || !IsValid(TargetLocalPlayer)) return;
	const FGuid ProfileId = ActiveOptionsCard->GetProfileId();
	if (!ProfileId.IsValid()) return;

	const UGameInstance* GameInstance = GetGameInstance();
	const UPlayerProfileSubsystem* ProfileSubsystem = GameInstance ? GameInstance->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;
	if (!ProfileSubsystem) return;
	
	// Attempt deletion
	if (!ProfileSubsystem->DeleteProfile(ProfileId))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ProfileSelection] Failed to delete profile %s"), *ProfileId.ToString());
		return;
	}
	
	// Close options popup
	ActiveOptionsCard = nullptr;
	if (IsValid(Options)) Options->SetVisibility(ESlateVisibility::Collapsed);
	
	// Rebuild cards from the updated saved profiles
	RebuildProfileCards();
	
	// Restore navigation focus
	if (APlayerController* PC = TargetLocalPlayer->GetPlayerController(GetWorld()))
	{
		if (!SpawnedCards.IsEmpty()) SpawnedCards[0]->FocusCard(PC);
	}
}
void UProfileSelectionPopup::HandleOptionsEntryReleased() const
{
	if (!IsValid(TargetLocalPlayer) || !IsValid(EditButton)) return;
	if (APlayerController* PC = TargetLocalPlayer->GetPlayerController(GetWorld()))
	{
		if (ULocalPlayerSessionDirectory* Directory = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULocalPlayerSessionDirectory>() : nullptr) Directory->FocusWidget(TargetLocalPlayer, EditButton);
		else EditButton->SetUserFocus(PC);
	}
}

// Selection events
void UProfileSelectionPopup::ConfirmSelection(const FGuid& ProfileId) const
{
	if (bSelectionCompleted || !ProfileId.IsValid()) return;
	if (!IsValid(TargetLocalPlayer)) return;
	OnProfileSelected.Broadcast(TargetLocalPlayer, ProfileId);
}
void UProfileSelectionPopup::CancelSelection()
{
	if (bSelectionCompleted || !IsValid(TargetLocalPlayer)) return;
	CompleteSelection();
	OnProfileSelectionCancelled.Broadcast(TargetLocalPlayer);
}
void UProfileSelectionPopup::CompleteSelection()
{
	bSelectionCompleted = true;
}
void UProfileSelectionPopup::RefreshAvailableProfiles()
{
	bPendingProfileListRefresh = true;
}

// New profiles
void UProfileSelectionPopup::BeginCreatingProfile()
{
	if (!ProfileScrollBox || !ProfileCardClass || IsValid(PendingNameCard)) return;
	
	UPlayerProfileCard* NewCard = CreateWidget<UPlayerProfileCard>(this, ProfileCardClass);
	if (!NewCard) return;
	
	NewCard->OnCardClicked.AddDynamic(this, &UProfileSelectionPopup::HandleCardClicked);
	NewCard->OnCardLongPressed.AddDynamic(this, &UProfileSelectionPopup::HandleCardLongPressed);
	NewCard->OnNameCommitted.AddDynamic(this, &UProfileSelectionPopup::HandleNameCommitted);
	NewCard->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
	NewCard->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
	
	// Insert in the second to last slot
	const int32 InsertIndex = ProfileScrollBox->GetChildrenCount() - 1;
	if (const UScrollBoxSlot* NewSlot = InsertWidgetAt(ProfileScrollBox, NewCard, InsertIndex); !NewSlot) return;
	ProfileScrollBox->InvalidateLayoutAndVolatility();
	
	// Keep the array in exactly the same order
	SpawnedCards.Insert(NewCard, InsertIndex);
	PendingNameCard = NewCard;
	NewCard->BeginNameEntry();
	
	bPendingNameFocus = true;
	ProfileScrollBox->ScrollWidgetIntoView(NewCard, true, EDescendantScrollDestination::Center);
}
void UProfileSelectionPopup::HandleNameCommitted(UPlayerProfileCard* Card, FText EnteredName)
{
	if (!IsValid(Card) || Card != PendingNameCard || !Card->IsNamingProfile()) return;
	
	// Trim spaces from both ends before validating or saving
	const FString ProfileName = EnteredName.ToString().TrimStartAndEnd();
	
	// Empty or whitespace-only names are not permitted
	if (ProfileName.IsEmpty())
	{
		Card->RejectNameEntry();
		bPendingNameFocus = true;
		return;
	}
	
	const UGameInstance* GameInstance = GetGameInstance();
	const UPlayerProfileSubsystem* ProfileSubsystem = GameInstance ? GameInstance->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;
	if (!ProfileSubsystem)
	{
		Card->RejectNameEntry();
		bPendingNameFocus = true;
		return;
	}
	
	// Card already associated with profile, must be a rename
	FGuid ProfileId = Card->GetProfileId();  
	if (ProfileId.IsValid())
	{
		if (!ProfileSubsystem->EditProfileName(ProfileId, ProfileName))
		{
			UE_LOG(LogTemp, Warning, TEXT("[ProfileSelection] Failed to edit profile '%s'."), *ProfileName);
			Card->RejectNameEntry();
			bPendingNameFocus = true;
			return;
		}
	}
	else
	{
		// CreateProfile returns an invalid GUID if creation fails
		ProfileId = ProfileSubsystem->CreateProfile(ProfileName);
		if (!ProfileId.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[ProfileSelection] Failed to create profile '%s'."), *ProfileName);
			Card->RejectNameEntry();
			bPendingNameFocus = true;
			return;
		}
	}
	
	// Success: turn the editable card into a normal profile card
	Card->SetProfile(ProfileId, ProfileName);
	
	PendingNameCard = nullptr;
	bPendingNameFocus = false;
	
	// Focus and center the newly created profile
	if (IsValid(TargetLocalPlayer))
	{
		if (APlayerController* PlayerController = TargetLocalPlayer->GetPlayerController(GetWorld())) Card->FocusCard(PlayerController);
	}
	
	ProfileScrollBox->ScrollWidgetIntoView(Card, true, EDescendantScrollDestination::Center);
	
	if (SpawnedCards[0] == Card)
	{
		Card->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
		SpawnedCards.Last()->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Escape);
	}
	else Card->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Escape);
	Card->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Escape);
}
UScrollBoxSlot* UProfileSelectionPopup::InsertWidgetAt(UScrollBox* ScrollBox, UWidget* NewWidget, int32 Index) const
{
	if (!ScrollBox || !NewWidget) return nullptr;
	
	// Store children
	TArray<UWidget*> ContentWidgets = ScrollBox->GetAllChildren();
	
	// Insert new widget to array
	Index = FMath::Clamp(Index, 0, ContentWidgets.Num());
	ContentWidgets.Insert(NewWidget, Index);
	
	// Re-populate scroll box
	ScrollBox->ClearChildren();
	UScrollBoxSlot* ResultSlot = nullptr;
	for (UWidget* Widget : ContentWidgets)
	{
		UScrollBoxSlot* AddedSlot = Cast<UScrollBoxSlot>(ScrollBox->AddChild(Widget));
		AddedSlot->SetPadding(FMargin(HorizontalMargin, 0.0f));
		if (Widget == NewWidget) ResultSlot = AddedSlot;
	}
	return ResultSlot;
}
