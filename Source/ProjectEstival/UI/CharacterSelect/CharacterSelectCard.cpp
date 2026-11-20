#include "UI/CharacterSelect/CharacterSelectCard.h"

#include "Systems/Mike/Combat/Loadouts/Loadout.h"
#include "GameFramework/PlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"

void UCharacterSelectCard::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	CardColor = Background->GetColorAndOpacity();
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	for (UButton* AbilityButton : { AbilityButton0.Get(), AbilityButton1.Get(), AbilityButton2.Get(), AbilityButton3.Get() })
	{
		if (IsValid(AbilityButton)) AbilityButton->IsFocusable = true;
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	const TArray<UBorder*> Borders = { AbilityBorder0, AbilityBorder1, AbilityBorder2, AbilityBorder3 };
	AbilityBorderColors.Reset(Borders.Num());
	for (const UBorder* Border : Borders)
	{
		AbilityBorderColors.Add(IsValid(Border) ? Border->GetBrushColor() : FLinearColor::White);
	}
	RefreshFocusedAbilityBorder();
}
void UCharacterSelectCard::InitializeCard(const int32 InCharacterIndex, const FText& InTitle)
{
	CharacterIndex = InCharacterIndex;
	SetIsFocusable(true);
	SetCharacterTitle(InTitle);
	SetProfile(FText::GetEmpty(), FLinearColor::White);
	SetSelected(false);
}

void UCharacterSelectCard::SetCharacterTitle(const FText& InTitle) const
{
	if (CharacterTitle) CharacterTitle->SetText(InTitle);
}

void UCharacterSelectCard::SetProfile(const FText& InProfileName, const FLinearColor& InProfileColor) const
{
	if (ProfileText) ProfileText->SetText(InProfileName);
	if (TagColor) TagColor->SetColorAndOpacity(InProfileColor);
	if (CardBorder) CardBorder->SetBrushColor(InProfileColor);
	if (TagBorder) TagBorder->SetBrushColor(InProfileColor);
}

void UCharacterSelectCard::SetLoadout(const ULoadout* InLoadout)
{
	LoadoutColor = InLoadout ? InLoadout->LoadoutColor : FLinearColor::White;
	SetCharacterTitle(InLoadout ? InLoadout->DisplayName : FText::GetEmpty());
	RefreshCharacterTitleColor();
}

void UCharacterSelectCard::SetSelected(const bool bInSelected)
{
	bIsSelected = bInSelected;
	RefreshBackground();
}

void UCharacterSelectCard::SetTargeted(const bool bInTargeted)
{
	bIsTargeted = bInTargeted;
	RefreshCharacterTitleColor();
}

void UCharacterSelectCard::SetPlayerAssigned(const bool bIsAssigned)
{
	if (LoadoutView) LoadoutView->SetVisibility(bIsAssigned ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (AIView) AIView->SetVisibility(bIsAssigned ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void UCharacterSelectCard::RefreshBackground() const
{
	if (!Background) return;
	if (bIsSelected)
	{
		Background->SetColorAndOpacity(ReadyColor);
		TagBackground->SetColorAndOpacity(ReadyColor);
	}
	else
	{
		Background->SetColorAndOpacity(CardColor);
		TagBackground->SetColorAndOpacity(CardColor);
	}
}

void UCharacterSelectCard::RefreshCharacterTitleColor() const
{
	if (CharacterTitle) CharacterTitle->SetColorAndOpacity(FSlateColor(bIsTargeted ? LoadoutColor : FLinearColor::White));
}
void UCharacterSelectCard::SetAbilities(const TArray<FCharacterSelectAbilityInfo>& InAbilities)
{
	Abilities = InAbilities;
	TArray Icons = { AbilityIcon0.Get(), AbilityIcon1.Get(), AbilityIcon2.Get(), AbilityIcon3.Get() };
	for (int32 Index = 0; Index < Icons.Num(); ++Index)
	{
		if (!IsValid(Icons[Index])) continue;
		if (Abilities.IsValidIndex(Index) && Abilities[Index].Icon)
		{
			Icons[Index]->SetBrushFromTexture(Abilities[Index].Icon);
			Icons[Index]->SetVisibility(ESlateVisibility::Visible);
		}
		else Icons[Index]->SetVisibility(ESlateVisibility::Hidden);
	}
	FocusedAbilityIndex = FMath::Clamp(FocusedAbilityIndex, 0, 3);
	RefreshFocusedAbilityBorder();
	RefreshAbilityDescription();
}

void UCharacterSelectCard::FocusAbility(const int32 AbilityIndex, APlayerController* PlayerController)
{
	FocusedAbilityIndex = FMath::Clamp(AbilityIndex, 0, 3);
	UButton* AbilityButton = nullptr;
	switch (FocusedAbilityIndex)
	{
		case 0:
			AbilityButton = AbilityButton0; break;
		case 1:
			AbilityButton = AbilityButton1; break;
		case 2:
			AbilityButton = AbilityButton2; break;
		case 3:
			AbilityButton = AbilityButton3; break;
		default:
			break;
	}
	if (IsValid(PlayerController) && IsValid(AbilityButton)) AbilityButton->SetUserFocus(PlayerController);
	RefreshFocusedAbilityBorder();
	RefreshAbilityDescription();
}

void UCharacterSelectCard::RefreshFocusedAbilityBorder() const
{
	const TArray<UBorder*> Borders = { AbilityBorder0, AbilityBorder1, AbilityBorder2, AbilityBorder3 };
	for (int32 Index = 0; Index < Borders.Num(); ++Index)
	{
		if (!IsValid(Borders[Index])) continue;
		FLinearColor BorderColor = AbilityBorderColors.IsValidIndex(Index) ? AbilityBorderColors[Index] : FLinearColor::White;
		if (Index != FocusedAbilityIndex) BorderColor.A = 0.0f;
		Borders[Index]->SetBrushColor(BorderColor);
		Borders[Index]->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UCharacterSelectCard::RefreshAbilityDescription()
{
	FText AbilityNameText = FText::GetEmpty();
	FText AbilityDescriptionText = FText::GetEmpty();
	if (Abilities.IsValidIndex(FocusedAbilityIndex))
	{
		const FCharacterSelectAbilityInfo& Ability = Abilities[FocusedAbilityIndex];
		AbilityNameText = Ability.Name;
		AbilityDescriptionText = Ability.Description.IsEmpty() ? Ability.Name : Ability.Description;
	}
	if (AbilityName) AbilityName->SetText(AbilityNameText);
	if (AbilityDescription) AbilityDescription->SetText(AbilityDescriptionText);
}
