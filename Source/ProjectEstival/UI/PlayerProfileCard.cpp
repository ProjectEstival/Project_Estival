#include "UI/PlayerProfileCard.h"

#include "Systems/Core/Profiles/LocalPlayerSessionDirectory.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "GameFramework/PlayerController.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Engine/GameInstance.h"

// Initialization
void UPlayerProfileCard::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (CardButton)
	{
		CardButton->OnPressed.AddDynamic(this, &UPlayerProfileCard::HandleCardPressed);
		CardButton->OnReleased.AddDynamic(this, &UPlayerProfileCard::HandleCardReleased);
	}
	if (ProfileNameText)
	{
		ProfileNameText->OnTextCommitted.AddDynamic(this, &UPlayerProfileCard::HandleNameCommitted);
		ProfileNameText->SetIsReadOnly(true);
		ProfileNameText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
void UPlayerProfileCard::NativeDestruct()
{
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(LongPressTimerHandle);
	RestoreKeyboardTextFocus();
	
	Super::NativeDestruct();
}
void UPlayerProfileCard::SetProfile(const FGuid& InProfileId, const FString& InProfileName)
{
	RestoreKeyboardTextFocus();
	ProfileId = InProfileId;
	bIsCreateCard = false;
	bIsNamingProfile = false;
	
	if (ProfileNameText)
	{
		ProfileNameText->SetText(FText::FromString(InProfileName));
		ProfileNameText->SetIsReadOnly(true);
		ProfileNameText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	
	if (CardButton) CardButton->SetVisibility(ESlateVisibility::Visible);
}
void UPlayerProfileCard::SetCreateCard()
{
	ProfileId.Invalidate();
	bIsCreateCard = true;
	bIsNamingProfile = false;
	
	if (ProfileNameText)
	{
		ProfileNameText->SetText(FText::FromString(TEXT("+")));
		ProfileNameText->SetIsReadOnly(true);
		ProfileNameText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	
	if (CardButton) CardButton->SetVisibility(ESlateVisibility::Visible);
}

// Input
void UPlayerProfileCard::FocusCard(APlayerController* PlayerController) const
{
	if (!IsValid(PlayerController) || !IsValid(CardButton)) return;
	const_cast<UPlayerProfileCard*>(this)->RestoreKeyboardTextFocus();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>())
		{
			Directory->FocusWidget(PlayerController->GetLocalPlayer(), CardButton);
			return;
		}
	}
	CardButton->SetUserFocus(PlayerController);
}
void UPlayerProfileCard::HandleCardPressed()
{
	if (bIsCreateCard || bIsNamingProfile) return;
	bLongPressTriggered = false;
	if (const UWorld* World = GetWorld()) World->GetTimerManager().SetTimer(LongPressTimerHandle, this, &UPlayerProfileCard::HandleLongPressTriggered, LongPressTimerInterval, false);
}
void UPlayerProfileCard::HandleCardReleased()
{
	GetWorld()->GetTimerManager().ClearTimer(LongPressTimerHandle);
	if (bLongPressTriggered)
	{
		bLongPressTriggered = false;
		return;
	}
	HandleCardClicked();
}
void UPlayerProfileCard::HandleLongPressTriggered()
{
	if (bIsCreateCard || bIsNamingProfile) return;
	bLongPressTriggered = true;
	OnCardLongPressed.Broadcast(this);
}
void UPlayerProfileCard::HandleCardClicked()
{
	if (bLongPressTriggered) return;
	if (bIsNamingProfile)
	{
		if (IsValid(ProfileNameText)) HandleNameCommitted(ProfileNameText->GetText(), ETextCommit::OnEnter);
	}
	else OnCardClicked.Broadcast(this);
}

// Naming
void UPlayerProfileCard::BeginNameEntry()
{
	bIsCreateCard = false;
	bIsNamingProfile = true;
	OriginalProfileName = FText::GetEmpty();
	ProfileId.Invalidate();
	
	if (CardButton) CardButton->SetVisibility(ESlateVisibility::HitTestInvisible);
	
	if (ProfileNameText)
	{
		ProfileNameText->SetText(FText::GetEmpty());
		ProfileNameText->SetIsReadOnly(false);
		ProfileNameText->SetVisibility(ESlateVisibility::Visible);
	}
}
void UPlayerProfileCard::BeginNameEdit()
{
	bIsNamingProfile = true;
	if (IsValid(ProfileNameText)) OriginalProfileName = ProfileNameText->GetText();
	if (CardButton) CardButton->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (ProfileNameText)
	{
		ProfileNameText->SetIsReadOnly(false);
		ProfileNameText->SetVisibility(ESlateVisibility::Visible);
	}
}
void UPlayerProfileCard::CancelNameEdit()
{
	if (!bIsNamingProfile || !IsValid(ProfileNameText)) return;
	RestoreKeyboardTextFocus();
	bIsNamingProfile = false;
	ProfileNameText->SetText(OriginalProfileName);
	ProfileNameText->SetIsReadOnly(true);
	ProfileNameText->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (IsValid(CardButton)) CardButton->SetVisibility(ESlateVisibility::Visible);
}
bool UPlayerProfileCard::CommitNameEntry()
{
	if (!bIsNamingProfile || !IsValid(ProfileNameText)) return false;
	HandleNameCommitted(ProfileNameText->GetText(), ETextCommit::OnEnter);
	return true;
}
void UPlayerProfileCard::EndNameEntry()
{
	bIsNamingProfile = false;
	RestoreKeyboardTextFocus();
}
void UPlayerProfileCard::HandleNameCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (!bIsNamingProfile) return;
	OnNameCommitted.Broadcast(this, Text);
}
void UPlayerProfileCard::RejectNameEntry() const
{
	if (!bIsNamingProfile || !ProfileNameText) return;
	
	ProfileNameText->SetIsReadOnly(false);
	ProfileNameText->SetVisibility(ESlateVisibility::Visible);
	if (APlayerController* PC = GetOwningPlayer()) FocusNameEntry(PC);
}
void UPlayerProfileCard::FocusNameEntry(APlayerController* PlayerController) const
{
	if (!bIsNamingProfile || !IsValid(ProfileNameText) || !IsValid(PlayerController)) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>())
		{
			Directory->FocusWidget(PlayerController->GetLocalPlayer(), ProfileNameText);
			ULocalPlayer* KeyboardPlayer = GI->GetLocalPlayerByIndex(0);
			ULocalPlayer* TargetPlayer = PlayerController->GetLocalPlayer();
			if (IsValid(KeyboardPlayer) && KeyboardPlayer != TargetPlayer)
			{
				UPlayerProfileCard* MutableThis = const_cast<UPlayerProfileCard*>(this);
				if (!MutableThis->bKeyboardFocusCaptured)
				{
					const FLocalPlayerSessionRecord* KeyboardRecord = Directory->FindPlayer(KeyboardPlayer);
					MutableThis->KeyboardFocusPlayer = KeyboardPlayer;
					MutableThis->KeyboardPreviousFocusWidget = KeyboardRecord ? KeyboardRecord->FocusedWidget : nullptr;
					MutableThis->bKeyboardFocusCaptured = true;
				}
				Directory->FocusWidget(KeyboardPlayer, ProfileNameText);
			}
			return;
		}
	}
	ProfileNameText->SetUserFocus(PlayerController);
}
void UPlayerProfileCard::RestoreKeyboardTextFocus()
{
	if (!bKeyboardFocusCaptured) return;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (ULocalPlayerSessionDirectory* Directory = GI->GetSubsystem<ULocalPlayerSessionDirectory>())
		{
			if (KeyboardFocusPlayer.IsValid())
			{
				if (KeyboardPreviousFocusWidget.IsValid()) Directory->FocusWidget(KeyboardFocusPlayer.Get(), KeyboardPreviousFocusWidget.Get());
				else if (FSlateApplication::IsInitialized())
				{
					const TSharedPtr<FSlateUser> SlateUser = KeyboardFocusPlayer->GetSlateUser();
					if (SlateUser.IsValid()) FSlateApplication::Get().ClearUserFocus(SlateUser->GetUserIndex());
					Directory->SetFocusedWidget(KeyboardFocusPlayer.Get(), nullptr);
				}
			}
		}
	}
	KeyboardFocusPlayer.Reset();
	KeyboardPreviousFocusWidget.Reset();
	bKeyboardFocusCaptured = false;
}
