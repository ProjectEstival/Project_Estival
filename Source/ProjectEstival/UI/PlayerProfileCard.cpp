#include "UI/PlayerProfileCard.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "GameFramework/PlayerController.h"

void UPlayerProfileCard::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (CardButton) CardButton->OnClicked.AddDynamic(this, &UPlayerProfileCard::HandleCardClicked);
	if (ProfileNameText)
	{
		ProfileNameText->OnTextCommitted.AddDynamic(this, &UPlayerProfileCard::HandleNameCommitted);
		ProfileNameText->SetIsReadOnly(true);
		ProfileNameText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
void UPlayerProfileCard::SetProfile(const FGuid& InProfileId, const FString& InProfileName)
{
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
void UPlayerProfileCard::BeginNameEntry()
{
	bIsCreateCard = false;
	bIsNamingProfile = true;
	ProfileId.Invalidate();
	
	if (CardButton) CardButton->SetVisibility(ESlateVisibility::HitTestInvisible);
	
	if (ProfileNameText)
	{
		ProfileNameText->SetText(FText::GetEmpty());
		ProfileNameText->SetIsReadOnly(false);
		ProfileNameText->SetVisibility(ESlateVisibility::Visible);
		ProfileNameText->SetKeyboardFocus();
	}
}
void UPlayerProfileCard::RejectNameEntry()
{
	if (!bIsNamingProfile || !ProfileNameText) return;
	
	ProfileNameText->SetText(FText::GetEmpty());
	ProfileNameText->SetIsReadOnly(false);
	ProfileNameText->SetVisibility(ESlateVisibility::Visible);
	ProfileNameText->SetKeyboardFocus();
}
void UPlayerProfileCard::FocusCard(APlayerController* PlayerController)
{
	if (IsValid(PlayerController) && IsValid(CardButton) && !bIsNamingProfile) CardButton->SetUserFocus(PlayerController);
}

void UPlayerProfileCard::HandleCardClicked()
{
	if (!bIsNamingProfile) OnCardClicked.Broadcast(this);
}
void UPlayerProfileCard::HandleNameCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (!bIsNamingProfile) return;
	OnNameCommitted.Broadcast(this, Text);
}

void UPlayerProfileCard::FocusNameEntry()
{
	if (bIsNamingProfile && IsValid(ProfileNameText)) ProfileNameText->SetKeyboardFocus();
}