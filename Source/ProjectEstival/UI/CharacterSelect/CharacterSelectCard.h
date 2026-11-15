#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "CharacterSelectCard.generated.h"

class APlayerController;
class UTextBlock;
class UTexture2D;
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
	void SetAbilities(const TArray<FCharacterSelectAbilityInfo>& InAbilities);
	void SetSelected(bool bIsSelected) const;
	void FocusAbility(int32 AbilityIndex, APlayerController* PlayerController);
	int32 GetFocusedAbilityIndex() const { return FocusedAbilityIndex; }
	int32 GetCharacterIndex() const { return CharacterIndex; }
	
protected:
	virtual void NativeOnInitialized() override;
	
private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Background;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CharacterTitle;
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
	TObjectPtr<UTextBlock> AbilityDescription;
	UPROPERTY(Transient)
	TArray<FCharacterSelectAbilityInfo> Abilities;
	int32 CharacterIndex = INDEX_NONE;
	int32 FocusedAbilityIndex = 0;
	void RefreshAbilityDescription();
};
