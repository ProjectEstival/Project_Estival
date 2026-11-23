#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "PlayerCard.generated.h"

class APlayerBase;
class UTextBlock;
class UBorder;
class UImage;

/** The in-game party card. Gameplay stats can be added here later. */
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
};
