#pragma once

#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/UserWidget.h"
#include "PlayerProfileCard.h"
#include "UI/UIBackHandler.h"
#include "CoreMinimal.h"

#include "ProfileSelectionPopup.generated.h"

class UPlayerProfileSubsystem;
class ULocalPlayer;
class UCanvasPanel;
class UScrollBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProfilePopupPlayerEvent, ULocalPlayer*, LocalPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProfileSelected, ULocalPlayer*, LocalPlayer, FGuid, ProfileId);

UCLASS()
class PROJECTESTIVAL_API UProfileSelectionPopup : public UUserWidget, public IUIBackHandler
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
	void ConfirmSelection(const FGuid& ProfileId) const;
	UFUNCTION(BlueprintCallable, Category = "Profile Selection")
	void CompleteSelection(); // Accessible from MainMenuHUD
	void RefreshAvailableProfiles();
	
	// Stop 'back' button from closing popup while in transient state (options open / mandatory popup) 
	virtual bool HandleBack_Implementation(APlayerController* Instigator) override;
	virtual void OnBackPopped_Implementation(APlayerController* Instigator) override;
	
	// Details menu
	UFUNCTION()
	void CloseCardOptions();
	bool IsCardOptionsOpen() const { return IsValid(ActiveOptionsCard); }
	bool HasPendingNameEdit() const { return IsValid(PendingNameCard); }
	bool CancelPendingNameEdit();
	bool CommitPendingNameEntry() const;
	
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
	UPROPERTY(EditDefaultsOnly, Category = "Profile Selection")
	float HorizontalMargin = 20.0f;
private:
	// Object references
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPlayerProfileCard>> SpawnedCards;
	UPROPERTY(Transient)
	TObjectPtr<UPlayerProfileCard> PendingNameCard;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> Options;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionsEntryButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EditButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DeleteButton;
	
	// Cards
	void RebuildProfileCards();
	bool FocusFirstCard();
	UFUNCTION()
	void HandleCardClicked(UPlayerProfileCard* Card);
	
	// Card options
	UFUNCTION()
	void HandleOptionsEntryReleased() const;
	UFUNCTION()
	void HandleCardLongPressed(UPlayerProfileCard* Card);
	UPROPERTY()
	TObjectPtr<UPlayerProfileCard> ActiveOptionsCard;
	UFUNCTION()
	void EditProfile();
	UFUNCTION()
	void DeleteProfile();
	
	// Profile creation
	UPROPERTY(Transient)
	bool bSelectionCompleted = false;
	UFUNCTION()
	void HandleNameCommitted(UPlayerProfileCard* Card, FText EnteredName);
	void BeginCreatingProfile();
	bool bPendingNameFocus = false;
	bool bPendingProfileListRefresh = false;
	
	// Helpers
	UScrollBoxSlot* InsertWidgetAt(UScrollBox* ScrollBox, UWidget* NewWidget, int32 Index) const;
};
