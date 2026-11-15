#include "UI/CharacterSelect/CharacterSelectCard.h"

#include "GameFramework/PlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

void UCharacterSelectCard::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	for (UButton* AbilityButton : { AbilityButton0.Get(), AbilityButton1.Get(), AbilityButton2.Get(), AbilityButton3.Get() })
	{
		if (IsValid(AbilityButton)) AbilityButton->IsFocusable = true;
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}
void UCharacterSelectCard::InitializeCard(const int32 InCharacterIndex, const FText& InTitle)
{
	CharacterIndex = InCharacterIndex;
	SetIsFocusable(true);
	SetCharacterTitle(InTitle);
	SetSelected(false);
}

void UCharacterSelectCard::SetCharacterTitle(const FText& InTitle) const
{
	if (CharacterTitle) CharacterTitle->SetText(InTitle);
}
void UCharacterSelectCard::SetSelected(const bool bIsSelected) const
{
	if (Background) Background->SetColorAndOpacity(bIsSelected ? FLinearColor(0.25f, 0.85f, 0.35f, 0.5f) : FLinearColor(0.66f, 0.66f, 0.66f, 1.0f));
}
void UCharacterSelectCard::SetAbilities(const TArray<FCharacterSelectAbilityInfo>& InAbilities)
{
	Abilities = InAbilities;
	const TArray<UImage*> Icons = { AbilityIcon0.Get(), AbilityIcon1.Get(), AbilityIcon2.Get(), AbilityIcon3.Get() };
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
	RefreshAbilityDescription();
}
void UCharacterSelectCard::RefreshAbilityDescription()
{
	if (!AbilityDescription) return;
	if (Abilities.IsValidIndex(FocusedAbilityIndex))
	{
		const FCharacterSelectAbilityInfo& Ability = Abilities[FocusedAbilityIndex];
		AbilityDescription->SetText(Ability.Description.IsEmpty() ? Ability.Name : Ability.Description);
	}
	else AbilityDescription->SetText(FText::GetEmpty());
}
