
#include "UI/ProfileSelectionPopup.h"

#include "Systems/Core/SaveSystem/PlayerProfileSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Components/ScrollBoxSlot.h"
#include "UI/PlayerProfileCard.h"
#include "Components/ScrollBox.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

void UProfileSelectionPopup::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
}
void UProfileSelectionPopup::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (bPendingNameFocus)
	{
		if (!IsInViewport() || !IsValid(PendingNameCard) || !ProfileScrollBox || ProfileScrollBox->GetCachedGeometry().GetLocalSize().IsNearlyZero()) return;
		PendingNameCard->FocusNameEntry();
		ProfileScrollBox->ScrollWidgetIntoView(PendingNameCard, true, EDescendantScrollDestination::Center);
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
		UE_LOG(LogTemp, Warning, TEXT("ProfileSelectionPopup: ProfileScrollBox or ProfileCardClass is not set."));
		return;
	}
	ProfileScrollBox->ClearChildren();
	SpawnedCards.Reset();
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	
	UPlayerProfileSubsystem* ProfileSubsystem = GameInstance->GetSubsystem<UPlayerProfileSubsystem>();
	if (!ProfileSubsystem) return;
	
	// Add all available profiles
	const auto& Profiles = ProfileSubsystem->GetAvailableProfiles();
	for (const auto& Profile : Profiles)
	{
		UPlayerProfileCard* Card = CreateWidget<UPlayerProfileCard>(this, ProfileCardClass);
		if (!Card) continue;
		
		Card->SetProfile(Profile.ProfileId, Profile.ProfileName);
		Card->OnCardClicked.AddDynamic(this, &UProfileSelectionPopup::HandleCardClicked);
		
		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(ProfileScrollBox->AddChild(Card))) ScrollSlot->SetPadding(FMargin(20.0f, 0.0f));
		SpawnedCards.Add(Card);
	}
	
	// Append the create-profile tile last
	UPlayerProfileCard* CreateCard = CreateWidget<UPlayerProfileCard>(this, ProfileCardClass);
	if (CreateCard)
	{
		CreateCard->SetCreateCard();
		CreateCard->OnCardClicked.AddDynamic(this, &UProfileSelectionPopup::HandleCardClicked);
		CreateCard->OnNameCommitted.AddDynamic(this, &UProfileSelectionPopup::HandleNameCommitted);
		
		if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(ProfileScrollBox->AddChild(CreateCard))) ScrollSlot->SetPadding(FMargin(20.0f, 0.0f));
		SpawnedCards.Add(CreateCard);
	}
}
bool UProfileSelectionPopup::FocusFirstCard()
{
	if (!IsValid(TargetLocalPlayer) || SpawnedCards.IsEmpty()) return false;
	
	UPlayerProfileCard* FirstCard = SpawnedCards[0];
	if (!IsValid(FirstCard)) return false;

	// Bring the initial selection into view.
	ProfileScrollBox->ScrollWidgetIntoView(FirstCard, true);

	// Focus through the controller belonging to this local player.
	APlayerController* PlayerController = TargetLocalPlayer->GetPlayerController(GetWorld());
	if (!IsValid(PlayerController)) return false;
	
	FirstCard->FocusCard(PlayerController);
	ProfileScrollBox->ScrollWidgetIntoView(FirstCard, false, EDescendantScrollDestination::Center);
	return true;
}
void UProfileSelectionPopup::HandleCardClicked(UPlayerProfileCard* Card)
{
	if (bSelectionCompleted || !IsValid(Card)) return;
	
	// Don't allow multiple unfinished profile names.
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
	
	// Existing profile selection flow.
	ConfirmSelection(ProfileId);
}

// Selection events
void UProfileSelectionPopup::ConfirmSelection(const FGuid& ProfileId)
{
	if (bSelectionCompleted || !ProfileId.IsValid()) return;
	if (!IsValid(TargetLocalPlayer)) return;
	
	// This is a request. The subsystem may reject it, for example if the profile cannot be loaded or is already assigned.
	// The HUD marks the selection complete only after success.
	OnSelectionConfirmed(ProfileId);
	
	OnProfileSelected.Broadcast(TargetLocalPlayer, ProfileId);
}
// Optional flow for closing the popup. Now handled by the subsystem 
void UProfileSelectionPopup::CancelSelection()
{
	if (bSelectionCompleted || !IsValid(TargetLocalPlayer)) return;
	CompleteSelection();
	
	OnSelectionCancelled();
	OnProfileSelectionCancelled.Broadcast(TargetLocalPlayer);
}
void UProfileSelectionPopup::CompleteSelection()
{
	bSelectionCompleted = true;
}

// New profiles
void UProfileSelectionPopup::BeginCreatingProfile()
{
	if (!ProfileScrollBox || !ProfileCardClass || IsValid(PendingNameCard)) return;
		
	UPlayerProfileCard* NewCard = CreateWidget<UPlayerProfileCard>(this, ProfileCardClass);
	if (!NewCard) return;
	
	NewCard->OnCardClicked.AddDynamic(this, &UProfileSelectionPopup::HandleCardClicked);
	NewCard->OnNameCommitted.AddDynamic(this, &UProfileSelectionPopup::HandleNameCommitted);
	
	// Insert in the second to last slot
	const int32 InsertIndex = ProfileScrollBox->GetChildrenCount() - 1;
	UScrollBoxSlot* NewSlot = InsertWidgetAt(ProfileScrollBox, NewCard, InsertIndex);
	if (!NewSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("InsertChildAt failed."));
		return;
	}
	NewSlot->SetPadding(FMargin(20.0f, 0.0f));
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
	
	// Trim spaces from both ends before validating or saving.
	const FString ProfileName = EnteredName.ToString().TrimStartAndEnd();
	
	// Empty or whitespace-only names are not permitted.
	if (ProfileName.IsEmpty())
	{
		Card->RejectNameEntry();
		bPendingNameFocus = true;
		return;
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	UPlayerProfileSubsystem* ProfileSubsystem = GameInstance ? GameInstance->GetSubsystem<UPlayerProfileSubsystem>() : nullptr;
	if (!ProfileSubsystem)
	{
		Card->RejectNameEntry();
		bPendingNameFocus = true;
		return;
	}
	
	// CreateProfile returns an invalid GUID if creation fails.
	const FGuid NewProfileId = ProfileSubsystem->CreateProfile(ProfileName);
	if (!NewProfileId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ProfileSelectionPopup: Failed to create profile '%s'."), *ProfileName);
		Card->RejectNameEntry();
		bPendingNameFocus = true;
		return;
	}
	
	// Success: turn the editable card into a normal profile card.
	Card->SetProfile(NewProfileId, ProfileName);
	
	PendingNameCard = nullptr;
	bPendingNameFocus = false;
	
	// Focus and center the newly created profile.
	if (IsValid(TargetLocalPlayer))
	{
		if (APlayerController* PlayerController = TargetLocalPlayer->GetPlayerController(GetWorld())) Card->FocusCard(PlayerController);
	}
	
	ProfileScrollBox->ScrollWidgetIntoView(Card, true, EDescendantScrollDestination::Center);
}
UScrollBoxSlot* UProfileSelectionPopup::InsertWidgetAt(UScrollBox* ScrollBox, UWidget* NewWidget, int32 Index)
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
		UPanelSlot* AddedSlot = ScrollBox->AddChild(Widget);
		if (Widget == NewWidget) ResultSlot = Cast<UScrollBoxSlot>(AddedSlot);
	}
	return ResultSlot;
}