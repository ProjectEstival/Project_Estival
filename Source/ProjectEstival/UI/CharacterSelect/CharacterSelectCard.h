#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "CharacterSelectCard.generated.h"

class APlayerController;
class UTextBlock;
class UWidget;
class UTexture2D;
class ULoadout;
class UBorder;
class UButton;
class UImage;

USTRUCT(BlueprintType)
struct FCharacterSelectAbilityInfo
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText Name;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UTexture2D> Icon = nullptr;
};

UCLASS()
class PROJECTESTIVAL_API UCharacterSelectCard : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeCard(int32 InCharacterIndex, const FText& InTitle);
	void SetCharacterTitle(const FText& InTitle) const;
	void SetProfile(const FText& InProfileName, const FLinearColor& InProfileColor) const;
	void SetLoadout(const ULoadout* InLoadout);
	void SetAbilities(const TArray<FCharacterSelectAbilityInfo>& InAbilities);
	void SetSelected(bool bInSelected);
	void SetTargeted(bool bInTargeted);
	void SetPlayerAssigned(bool bIsAssigned);
	void FocusAbility(int32 AbilityIndex, APlayerController* PlayerController);
	int32 GetFocusedAbilityIndex() const { return FocusedAbilityIndex; }
	int32 GetCharacterIndex() const { return CharacterIndex; }
	
protected:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor ReadyColor = FLinearColor::Green;
	
private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Background;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> TagBackground;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> TagColor;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> CardBorder;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> TagBorder;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> LoadoutView;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> AIView; // What to show if an NPC will be assigned to this card
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CharacterTitle;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ProfileText;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AbilityButton0;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AbilityButton1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AbilityButton2;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AbilityButton3;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon0;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon2;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> AbilityIcon3;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> AbilityBorder0;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> AbilityBorder1;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> AbilityBorder2;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> AbilityBorder3;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AbilityName;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AbilityDescription;
	UPROPERTY(Transient)
	TArray<FCharacterSelectAbilityInfo> Abilities;
	TArray<FLinearColor> AbilityBorderColors;
	FLinearColor CardColor = FLinearColor::Black;
	FLinearColor LoadoutColor = FLinearColor::White;
	bool bIsSelected = false;
	bool bIsTargeted = false;
	int32 CharacterIndex = INDEX_NONE;
	int32 FocusedAbilityIndex = 0;
	void RefreshFocusedAbilityBorder() const;
	void RefreshAbilityDescription();
	void RefreshBackground() const;
	void RefreshCharacterTitleColor() const;
};
