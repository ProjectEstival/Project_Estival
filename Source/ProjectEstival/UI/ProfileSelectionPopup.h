#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "ProfileSelectionPopup.generated.h"

class UButton;
class UScrollBox;
class ULocalPlayer;
class UPlayerProfileCard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProfilePopupPlayerEvent, ULocalPlayer*, LocalPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProfileSelected, ULocalPlayer*, LocalPlayer, FGuid, ProfileId);

UCLASS()
class PROJECTESTIVAL_API UProfileSelectionPopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Events
	UPROPERTY(BlueprintAssignable, Category = "Profile Selection")
	FOnProfileSelected OnProfileSelected;
	UPROPERTY(BlueprintAssignable, Category = "Profile Selection")
	FProfilePopupPlayerEvent OnProfileSelectionCancelled;
	UPROPERTY(BlueprintAssignable, Category = "Profile Selection")
	FProfilePopupPlayerEvent OnCreateProfileRequested;
	
	// Target player
	void SetTargetLocalPlayer(ULocalPlayer* LocalPlayer);
	ULocalPlayer* GetTargetLocalPlayer() const { return TargetLocalPlayer; }
	
	// Selection flow
	UFUNCTION(BlueprintCallable, Category = "Profile Selection")
	void ConfirmSelection(const FGuid& ProfileId);
	UFUNCTION(BlueprintCallable, Category = "Profile Selection")
	void CancelSelection(); // Optional flow for closing the popup from the UI.
	void CompleteSelection(); // Accessible from MainMenuHUD
	
protected:
	// Popup creation
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	bool bPendingInitialFocus = false;
	
	// Object references
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Title;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ProfileScrollBox;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Profile Selection")
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;
	
	// Cards
	UPROPERTY(EditDefaultsOnly, Category = "Profile Selection")
	TSubclassOf<UPlayerProfileCard> ProfileCardClass;
	
private:
	// Object references
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPlayerProfileCard>> SpawnedCards;
	UPROPERTY(Transient)
	TObjectPtr<UPlayerProfileCard> PendingNameCard;
	
	// Cards
	void RebuildProfileCards();
	bool FocusFirstCard();
	UFUNCTION()
	void HandleCardClicked(UPlayerProfileCard* Card);
	
	// Profile creation
	UPROPERTY(Transient)
	bool bSelectionCompleted = false;
	UFUNCTION()
	void HandleNameCommitted(UPlayerProfileCard* Card, FText EnteredName);
	void BeginCreatingProfile();
	bool bPendingNameFocus = false;
	
	// Helpers
	static UScrollBoxSlot* InsertWidgetAt(UScrollBox* ScrollBox, UWidget* NewWidget, int32 Index);
};
