#pragma once

#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "CoreMinimal.h"

#include "PlayerCard.generated.h"

class UMaterialInstanceDynamic;
class UProgressBar;
class APlayerBase;
class UTextBlock;
class UBorder;
class UImage;

UCLASS()
class PROJECTESTIVAL_API UPlayerCard : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Game HUD")
	void InitializeCard(APlayerBase* InPlayer, int32 InPartyIndex, const FLinearColor& InPlayerColor);
	
	UFUNCTION(BlueprintPure, Category = "Game HUD")
	APlayerBase* GetPlayer() const { return Player; }
	
	UFUNCTION(BlueprintPure, Category = "Game HUD")
	int32 GetPartyIndex() const { return PartyIndex; }
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> CardBorder;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> TagBorder;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TagColor;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ProfileText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CharacterTitle;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
	TObjectPtr<APlayerBase> Player = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
	int32 PartyIndex = INDEX_NONE;
	
	// Stats
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> CorruptionBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Ability0;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Ability1;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Ability2;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Ability3;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Elixir;
	
private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> AbilityMaterials;
	TArray<FGameplayTag> AbilitySlotTags;
	
	void SetAbilityIcon(int32 AbilityIndex, class UTexture2D* Icon);
	void UpdateAbilityCooldowns();
	
	UFUNCTION()
	void OnResourceChanged(FGameplayTag Stat, float NewValue, float OldValue, float MaxValue) const;
};
