#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "InGameUI.generated.h"

class UPanelWidget;
class UPlayerCard;

UCLASS()
class PROJECTESTIVAL_API UInGameUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game HUD")
	TSubclassOf<UPlayerCard> PlayerCardClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game HUD")
	float PlayerCardPadding = 20.0f;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PlayerCardsPanel;
	
private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPlayerCard>> PlayerCards;
	
	void InitializePlayerCards();
	
	UFUNCTION()
	void OnFloorStarted(int32 Floor);
};
